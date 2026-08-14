#include "../Source/PluginProcessor.h"
#include "../Source/Parameters.h"

#include <nf/testing/ProcessorHarness.h>

#include <juce_audio_processors/juce_audio_processors.h>

/**
    Category 5 — automation and latency.

    ## The declared-latency check, and the known case it is introduced against

    Five of the six castings declare no latency at all. **A declaration of zero is a claim like any
    other**, and the only one of the six that can validate the instrument is TapeRot: it is the sole
    casting that declares any (`setLatencySamples (saturator.getLatencySamples())`), its Saturator
    oversamples, and category 4 independently measured a ~20 ms dead window at the start of every
    render which is that latency showing up in a level profile.

    So TapeRot is the known case, stated before the set is run: **the instrument must report a
    non-zero figure there and it must agree with `getLatencySamples()`.** If it reports zero for
    TapeRot, a zero anywhere else means nothing — which is the failure mode five "no latency" claims
    would otherwise sail through.

    ## What agreement means

    An impulse must emerge at exactly the declared latency. Emerging LATER than declared is undeclared
    delay — a host aligns by the declaration, so the plugin's output arrives late and every other
    track is early against it. Emerging EARLIER is a declaration that over-compensates, which pulls
    the plugin ahead. Both are reported; neither is assumed to be the interesting one.
*/
class AutomationLatencyTests final : public juce::UnitTest
{
public:
    AutomationLatencyTests() : juce::UnitTest ("Automation and latency", "DSP") {}

    void runTest() override
    {
        beginTest ("MIX at 50% does not comb — the claim in OutputStage.h, tested by impulse");
        {
            // `OutputStage.h:6-10` states there is no lookahead anywhere, so the wet path is
            // sample-aligned with the dry BY CONSTRUCTION and blending cannot comb. The plan's brief
            // said the opposite — that a delay-compensated dry path was claimed and should be
            // verified. The header is the one making a checkable claim, so it is the one tested, and
            // the answer comes from the measurement rather than from either document.
            //
            // The test: with the compressor not compressing, wet and dry carry the SAME signal, so
            // Mix 50% must be flat. If the two paths are misaligned by N samples the output is
            // 0.5*(x[n] + x[n-N]), which is a comb — deep periodic notches, unmistakable.
            //
            // KNOWN CASE, named before the run: Mix at 100% is wet only, one path, so it CANNOT
            // comb whatever the alignment. If the 100% arm shows the same spread as the 50% arm,
            // the instrument is reading something other than blending and proves nothing.
            const std::vector<double> probes { 200.0, 400.0, 800.0, 1600.0, 3200.0, 6400.0, 12000.0 };

            const auto spreadAtMix = [&probes, this] (float mixNormalised)
            {
                ElmerAudioProcessor p;

                // Threshold at its top and ratio at its bottom: the compressor passes signal
                // through without acting, so any deviation from flat is the BLEND, not gain
                // reduction tracking the sweep.
                const auto set = [&p] (const char* id, float v)
                {
                    if (auto* param = p.apvts.getParameter (id))
                        param->setValueNotifyingHost (v);
                };

                set (ParamIDs::threshold, 1.0f);
                set (ParamIDs::ratio, 0.0f);
                set (ParamIDs::makeup, 0.0f);
                set (ParamIDs::iron, 0.0f);
                set (ParamIDs::mix, mixNormalised);

                const auto rows = nf::testing::measureProcessorMagnitudeResponse (p, 48000.0, 512, probes);

                double lo = 1.0e9, hi = -1.0e9;
                juce::String row;

                for (const auto& r : rows)
                {
                    lo = juce::jmin (lo, r.gainDb);
                    hi = juce::jmax (hi, r.gainDb);
                    row += juce::String (r.gainDb, 2).paddedLeft (' ', 9);
                }

                logMessage ("  MIX " + juce::String (mixNormalised * 100.0f, 0) + "% ->" + row
                                + "   spread " + juce::String (hi - lo, 3) + " dB");

                return hi - lo;
            };

            logMessage ("  probe (Hz)              200      400      800     1600     3200     6400    12000");

            // **A KNOWN COMB, because the wet-only control only proves the instrument reads flat on
            // something that cannot comb — not that it would SHOW one.** A negative from a detector
            // never shown able to fire is the exact shape this sweep keeps finding. So: 0.5*(x[n] +
            // x[n-8]) built by hand, which is what a misaligned blend physically is. At 48 kHz an
            // 8-sample comb notches at 3 kHz and again at 9 kHz, both inside the probe set.
            {
                std::array<float, 16> history {};
                int writeIndex = 0;

                const auto combRows = nf::testing::measureMagnitudeResponse (
                    [&history, &writeIndex] (float x)
                    {
                        const int readIndex = (writeIndex + (int) history.size() - 8) % (int) history.size();
                        const float delayed = history[(size_t) readIndex];
                        history[(size_t) writeIndex] = x;
                        writeIndex = (writeIndex + 1) % (int) history.size();
                        return 0.5f * (x + delayed);
                    },
                    [&history, &writeIndex] { history.fill (0.0f); writeIndex = 0; },
                    48000.0, probes);

                double lo = 1.0e9, hi = -1.0e9;
                juce::String row;

                for (const auto& r : combRows)
                {
                    lo = juce::jmin (lo, r.gainDb);
                    hi = juce::jmax (hi, r.gainDb);
                    row += juce::String (r.gainDb, 2).paddedLeft (' ', 9);
                }

                logMessage ("  known comb ->" + row + "   spread " + juce::String (hi - lo, 3) + " dB");

                expectGreaterThan (hi - lo, 3.0,
                                   "a hand-built 8-sample comb did not register as one, so this "
                                   "instrument cannot detect combing and the Elmer result below "
                                   "means nothing: spread " + juce::String (hi - lo, 3) + " dB");
            }

            const auto wetOnly = spreadAtMix (1.0f);      // the control: one path, cannot comb
            const auto blended = spreadAtMix (0.5f);      // the arm under test

            logMessage ("  => " + juce::String (blended > wetOnly + 3.0
                            ? "MIX 50% COMBS: the blend adds " + juce::String (blended - wetOnly, 2)
                                  + " dB of spread the wet-only path does not have"
                            : "no comb — the header's claim holds, and the brief's premise was wrong"));

            // Reported against the control rather than in absolute terms: this compressor is not
            // flat to begin with, so an absolute spread says nothing about alignment.
            expectLessThan (blended, wetOnly + 3.0,
                            "MIX at 50% introduced " + juce::String (blended - wetOnly, 2)
                                + " dB of extra spread over wet-only, which is comb filtering — so "
                                  "OutputStage.h's claim that the paths are sample-aligned by "
                                  "construction is wrong.");
        }

        beginTest ("Zipper — EVERY parameter swept once per block, not just MAKEUP");
        {
            // **MAKEUP at x25.74 was one sample of a property the whole casting may have.** Elmer
            // uses juce::SmoothedValue nowhere — TapeRot has it in 9 files, Fifth Member 3,
            // Reflect-84 3, Chorus-60 2, Gatecrasher 1, Elmer 0 — so the question is not whether one
            // parameter needs smoothing but whether the omission is casting-wide. Those are
            // different rulings: one is an edit, the other is a policy.
            //
            // A steady 200 Hz sine in, one parameter swept once per block, and |y[n] - y[n-1]|
            // compared AT block boundaries against everywhere else. An unsmoothed parameter steps at
            // every boundary and nowhere else.
            //
            // KNOWN CASE: Reflect-84's OUTPUT TRIM is smoothed and carries the identical test at
            // x0.95. It serves this whole set, not just the MAKEUP row — the instrument is the same
            // one, and it has already been shown able to come back clean on a smoothed parameter.
            // The per-row static control is the second: with the parameter held still there is
            // nothing to zipper, so any excess there is the instrument's own.
            constexpr double fs = 48000.0;
            constexpr int blockSize = 256;

            const auto boundaryRatio = [&] (const char* id, bool sweep)
            {
                ElmerAudioProcessor p;

                nf::testing::RenderSpec warmSpec;
                warmSpec.blockSize = blockSize;
                warmSpec.numBlocks = 8;
                nf::testing::render (p, warmSpec);

                nf::testing::RenderSpec spec;
                spec.sampleRate = fs;
                spec.blockSize = blockSize;
                spec.numBlocks = 48;

                auto* param = id != nullptr ? p.apvts.getParameter (id) : nullptr;

                spec.fillInput = [&param, sweep] (juce::AudioBuffer<float>& buffer, int blockIndex)
                {
                    if (sweep && param != nullptr)
                        param->setValueNotifyingHost ((blockIndex % 2) == 0 ? 0.15f : 0.85f);

                    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                        for (int i = 0; i < buffer.getNumSamples(); ++i)
                        {
                            const double n = blockIndex * buffer.getNumSamples() + i;
                            buffer.setSample (ch, i, 0.5f * (float) std::sin (2.0 * juce::MathConstants<double>::pi
                                                                              * 200.0 * n / fs));
                        }
                };

                const auto out = nf::testing::render (p, spec);

                double worstBoundary = 0.0, worstInterior = 0.0;

                for (size_t i = 1; i < out[0].size(); ++i)
                {
                    const double step = std::abs ((double) out[0][i] - out[0][i - 1]);

                    if ((i % (size_t) blockSize) == 0) worstBoundary = juce::jmax (worstBoundary, step);
                    else                               worstInterior = juce::jmax (worstInterior, step);
                }

                return worstInterior > 0.0 ? worstBoundary / worstInterior : 0.0;
            };

            const auto still = boundaryRatio (nullptr, false);
            logMessage ("  static control (nothing swept) -> ratio x" + juce::String (still, 2));

            expectLessThan (still, 3.0,
                            "the static control showed boundary excess with nothing swept, so this "
                            "instrument does not isolate smoothing and no row below means anything");

            struct Row { const char* id; const char* label; };
            const Row rows[] = {
                { ParamIDs::makeup,      "MAKEUP" },      { ParamIDs::threshold,   "THRESHOLD" },
                { ParamIDs::ratio,       "RATIO" },       { ParamIDs::attack,      "ATTACK" },
                { ParamIDs::release,     "RELEASE" },     { ParamIDs::sidechainHp, "SIDECHAIN HP" },
                { ParamIDs::knee,        "KNEE" },        { ParamIDs::mix,         "MIX" },
                { ParamIDs::iron,        "IRON" }
            };

            juce::StringArray zippering, clean;

            for (const auto& row : rows)
            {
                const auto ratio = boundaryRatio (row.id, true);
                const bool zips = ratio > 3.0;

                logMessage ("  " + juce::String (row.label).paddedRight (' ', 16)
                                + "ratio x" + juce::String (ratio, 2) + (zips ? "   ZIPPERS" : ""));

                (zips ? zippering : clean).add (row.label);
            }

            logMessage ("  zippering -> " + (zippering.isEmpty() ? juce::String ("none")
                                                                 : zippering.joinIntoString (", ")));
            logMessage ("  clean     -> " + (clean.isEmpty() ? juce::String ("none")
                                                             : clean.joinIntoString (", ")));

            // **Reported as a set, and asserted as a set.** The finding is the shape of the column,
            // not any one row: a casting with no smoothing anywhere should zipper on every
            // continuous parameter, and if it zippers on only one that is a different fact needing
            // a different explanation.
            // **MEASURED: three of nine, and the split is not arbitrary.** MAKEUP x25.74,
            // IRON x10.65, MIX x3.17 zipper; THRESHOLD, RATIO, ATTACK, RELEASE, SIDECHAIN HP and
            // KNEE do not, all at x0.82 or below.
            //
            // The three that zipper are the three applied as a per-sample GAIN with nothing between
            // the parameter and the output. The six that do not are all DETECTOR parameters — they
            // feed the envelope follower, whose own attack and release ballistics smooth them by
            // construction. A compressor gets that smoothing free and does not need SmoothedValue
            // for it.
            //
            // **So "Elmer uses SmoothedValue nowhere" is true and does NOT imply a casting-wide
            // defect.** The casting-wide hypothesis is refuted with an explanation rather than
            // merely contradicted, which is what makes the narrower ruling safe to act on: smooth
            // the three gain-like parameters, and leave the detector six alone because something
            // already smooths them.
            expect (zippering.isEmpty(),
                    "these parameters step at block boundaries with nothing damping them: "
                        + zippering.joinIntoString (", ")
                        + ". All three are applied as a per-sample gain; the detector parameters are "
                          "smoothed by the envelope follower's own ballistics and do not zipper.");
        }

        beginTest ("MIX at 50% does not comb — the claim in OutputStage.h, tested by impulse");
        {
            // `OutputStage.h:6-10` states there is no lookahead anywhere, so the wet path is
            // sample-aligned with the dry BY CONSTRUCTION and blending cannot comb. The plan's brief
            // said the opposite — that a delay-compensated dry path was claimed and should be
            // verified. The header is the one making a checkable claim, so it is the one tested, and
            // the answer comes from the measurement rather than from either document.
            //
            // The test: with the compressor not compressing, wet and dry carry the SAME signal, so
            // Mix 50% must be flat. If the two paths are misaligned by N samples the output is
            // 0.5*(x[n] + x[n-N]), which is a comb — deep periodic notches, unmistakable.
            //
            // KNOWN CASE, named before the run: Mix at 100% is wet only, one path, so it CANNOT
            // comb whatever the alignment. If the 100% arm shows the same spread as the 50% arm,
            // the instrument is reading something other than blending and proves nothing.
            const std::vector<double> probes { 200.0, 400.0, 800.0, 1600.0, 3200.0, 6400.0, 12000.0 };

            const auto spreadAtMix = [&probes, this] (float mixNormalised)
            {
                ElmerAudioProcessor p;

                // Threshold at its top and ratio at its bottom: the compressor passes signal
                // through without acting, so any deviation from flat is the BLEND, not gain
                // reduction tracking the sweep.
                const auto set = [&p] (const char* id, float v)
                {
                    if (auto* param = p.apvts.getParameter (id))
                        param->setValueNotifyingHost (v);
                };

                set (ParamIDs::threshold, 1.0f);
                set (ParamIDs::ratio, 0.0f);
                set (ParamIDs::makeup, 0.0f);
                set (ParamIDs::iron, 0.0f);
                set (ParamIDs::mix, mixNormalised);

                const auto rows = nf::testing::measureProcessorMagnitudeResponse (p, 48000.0, 512, probes);

                double lo = 1.0e9, hi = -1.0e9;
                juce::String row;

                for (const auto& r : rows)
                {
                    lo = juce::jmin (lo, r.gainDb);
                    hi = juce::jmax (hi, r.gainDb);
                    row += juce::String (r.gainDb, 2).paddedLeft (' ', 9);
                }

                logMessage ("  MIX " + juce::String (mixNormalised * 100.0f, 0) + "% ->" + row
                                + "   spread " + juce::String (hi - lo, 3) + " dB");

                return hi - lo;
            };

            logMessage ("  probe (Hz)              200      400      800     1600     3200     6400    12000");

            // **A KNOWN COMB, because the wet-only control only proves the instrument reads flat on
            // something that cannot comb — not that it would SHOW one.** A negative from a detector
            // never shown able to fire is the exact shape this sweep keeps finding. So: 0.5*(x[n] +
            // x[n-8]) built by hand, which is what a misaligned blend physically is. At 48 kHz an
            // 8-sample comb notches at 3 kHz and again at 9 kHz, both inside the probe set.
            {
                std::array<float, 16> history {};
                int writeIndex = 0;

                const auto combRows = nf::testing::measureMagnitudeResponse (
                    [&history, &writeIndex] (float x)
                    {
                        const int readIndex = (writeIndex + (int) history.size() - 8) % (int) history.size();
                        const float delayed = history[(size_t) readIndex];
                        history[(size_t) writeIndex] = x;
                        writeIndex = (writeIndex + 1) % (int) history.size();
                        return 0.5f * (x + delayed);
                    },
                    [&history, &writeIndex] { history.fill (0.0f); writeIndex = 0; },
                    48000.0, probes);

                double lo = 1.0e9, hi = -1.0e9;
                juce::String row;

                for (const auto& r : combRows)
                {
                    lo = juce::jmin (lo, r.gainDb);
                    hi = juce::jmax (hi, r.gainDb);
                    row += juce::String (r.gainDb, 2).paddedLeft (' ', 9);
                }

                logMessage ("  known comb ->" + row + "   spread " + juce::String (hi - lo, 3) + " dB");

                expectGreaterThan (hi - lo, 3.0,
                                   "a hand-built 8-sample comb did not register as one, so this "
                                   "instrument cannot detect combing and the Elmer result below "
                                   "means nothing: spread " + juce::String (hi - lo, 3) + " dB");
            }

            const auto wetOnly = spreadAtMix (1.0f);      // the control: one path, cannot comb
            const auto blended = spreadAtMix (0.5f);      // the arm under test

            logMessage ("  => " + juce::String (blended > wetOnly + 3.0
                            ? "MIX 50% COMBS: the blend adds " + juce::String (blended - wetOnly, 2)
                                  + " dB of spread the wet-only path does not have"
                            : "no comb — the header's claim holds, and the brief's premise was wrong"));

            // Reported against the control rather than in absolute terms: this compressor is not
            // flat to begin with, so an absolute spread says nothing about alignment.
            expectLessThan (blended, wetOnly + 3.0,
                            "MIX at 50% introduced " + juce::String (blended - wetOnly, 2)
                                + " dB of extra spread over wet-only, which is comb filtering — so "
                                  "OutputStage.h's claim that the paths are sample-aligned by "
                                  "construction is wrong.");
        }

        beginTest ("Zipper — a gain parameter swept once per block");
        {
            // **Elmer uses juce::SmoothedValue NOWHERE.** TapeRot has it in 9 files, Fifth Member 3,
            // Reflect-84 3, Chorus-60 2, Gatecrasher 1, Elmer 0 — the one-of-six shape the audit
            // kept finding. Whether it matters is a measurement: a compressor whose output gain is
            // ridden may or may not zipper, and the way to know is to ride it.
            //
            // ## The instrument
            //
            // A steady sine in, so any discontinuity is the plugin's and not the input's. The gain
            // parameter is swept once per block. An unsmoothed gain then steps at every block
            // boundary, so the test compares |y[n] - y[n-1]| AT boundaries against the same figure
            // everywhere else. A smoothed gain shows no excess; an unsmoothed one shows a spike
            // exactly at the boundaries and nowhere else.
            //
            // ## Known case, named before the run
            //
            // **Reflect-84's OUTPUT TRIM is smoothed** (trimSmoothed, PluginProcessor.cpp:52) and
            // carries the same test. It must come back with no boundary excess. If it does not, the
            // instrument is reading something other than smoothing and Elmer's figure means nothing.
            // The static arm below is the second control: with the parameter held still there is
            // nothing to zipper, so any excess there is the instrument's own.
            constexpr double fs = 48000.0;
            constexpr int blockSize = 256;

            const auto boundaryExcess = [&] (const char* label, bool sweep)
            {
                ElmerAudioProcessor p;

                nf::testing::RenderSpec warmSpec;
                warmSpec.blockSize = blockSize;
                warmSpec.numBlocks = 8;
                nf::testing::render (p, warmSpec);

                nf::testing::RenderSpec spec;
                spec.sampleRate = fs;
                spec.blockSize = blockSize;
                spec.numBlocks = 48;

                auto* param = p.apvts.getParameter (ParamIDs::makeup);

                spec.fillInput = [&param, sweep] (juce::AudioBuffer<float>& buffer, int blockIndex)
                {
                    if (sweep && param != nullptr)
                        param->setValueNotifyingHost ((blockIndex % 2) == 0 ? 0.15f : 0.85f);

                    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                        for (int i = 0; i < buffer.getNumSamples(); ++i)
                        {
                            const double n = blockIndex * buffer.getNumSamples() + i;
                            buffer.setSample (ch, i, 0.5f * (float) std::sin (2.0 * juce::MathConstants<double>::pi
                                                                              * 200.0 * n / fs));
                        }
                };

                const auto out = nf::testing::render (p, spec);

                double worstBoundary = 0.0, worstInterior = 0.0;

                for (size_t i = 1; i < out[0].size(); ++i)
                {
                    const double step = std::abs ((double) out[0][i] - out[0][i - 1]);

                    if ((i % (size_t) blockSize) == 0)
                        worstBoundary = juce::jmax (worstBoundary, step);
                    else
                        worstInterior = juce::jmax (worstInterior, step);
                }

                const double ratio = worstInterior > 0.0 ? worstBoundary / worstInterior : 0.0;

                logMessage ("  " + juce::String (label).paddedRight (' ', 22)
                                + "boundary " + juce::String (worstBoundary, 6)
                                + ", interior " + juce::String (worstInterior, 6)
                                + ", ratio x" + juce::String (ratio, 2));

                return ratio;
            };

            const auto still = boundaryExcess ("parameter held still", false);
            const auto swept = boundaryExcess ("parameter swept", true);

            logMessage (juce::String ("  => ") + (swept > 3.0 && still < 3.0
                            ? "ZIPPER: a per-block step reaches the output undamped"
                            : still >= 3.0 ? "the STATIC control shows boundary excess too — the "
                                             "instrument is not isolating smoothing and proves nothing"
                                           : "no zipper: the step is damped before it reaches the output"));

            expectLessThan (still, 3.0,
                            "the static control showed boundary excess with nothing being swept, so "
                            "this instrument does not isolate smoothing");
        }

        beginTest ("Declared latency against an impulse");
        {
            ElmerAudioProcessor processor;

            nf::testing::RenderSpec spec;
            spec.blockSize = 512;
            spec.numBlocks = 32;

            // An impulse, not the default noise: the question is WHERE energy first emerges.
            // **`measureImpulseLatency` cannot be used on a casting that GENERATES**, and TapeRot
            // is one. Its noise bed and hum are above any sensible detection threshold at every
            // sample, so "the first output above threshold" is sample 0 whatever the latency is —
            // the warmed run reported exactly that, 0 against a declared 4, and it would have read
            // as a 4-sample over-declaration.
            //
            // So measure DIFFERENTIALLY: render the impulse, render silence, subtract. Everything
            // the plugin generates on its own is deterministic and seeded, so it cancels exactly,
            // and what remains is the impulse's own response. On a casting that generates nothing
            // the silent render is zero and this reduces to the original measurement.
            //
            // (This belongs in core beside measureImpulseLatency rather than in six copies. It is
            // here because moving it costs a tag move and six repins mid-category; the six copies
            // are generated from one template, so they are identical by construction rather than by
            // discipline. Recorded so it is moved when the harness is next touched.)
            const auto renderWith = [&] (bool withImpulse)
            {
                ElmerAudioProcessor p;

                nf::testing::RenderSpec warmSpec;
                warmSpec.blockSize = spec.blockSize;
                warmSpec.numBlocks = 8;
                nf::testing::render (p, warmSpec);      // spend any first-run state — see category 3

                auto s = spec;
                s.fillInput = [withImpulse] (juce::AudioBuffer<float>& buffer, int blockIndex)
                {
                    buffer.clear();

                    if (withImpulse && blockIndex == 0)
                        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                            buffer.setSample (ch, 0, 1.0f);
                };

                return nf::testing::render (p, s);
            };

            const auto withImpulse = renderWith (true);
            const auto silent      = renderWith (false);

            int measured = -1;
            double impulsePeak = 0.0;

            for (size_t i = 0; i < withImpulse[0].size() && i < silent[0].size(); ++i)
            {
                const double d = std::abs ((double) withImpulse[0][i] - silent[0][i]);
                impulsePeak = juce::jmax (impulsePeak, d);

                if (measured < 0 && d > 1.0e-4)
                    measured = (int) i;
            }

            ElmerAudioProcessor reference;
            reference.setRateAndBufferSizeDetails (spec.sampleRate, spec.blockSize);
            reference.prepareToPlay (spec.sampleRate, spec.blockSize);
            const int declared = reference.getLatencySamples();

            logMessage ("  impulse response peak " + juce::String (impulsePeak, 6)
                            + " — if that is 0 the two renders are identical and nothing was measured");

            logMessage ("  declared " + juce::String (declared) + " samples, impulse emerged at "
                            + juce::String (measured)
                            + (measured < 0 ? "  (NOTHING EMERGED)" : ""));

            if (measured >= 0 && declared >= 0)
                logMessage ("  difference -> " + juce::String (measured - declared)
                                + " samples ("
                                + juce::String ((measured - declared) * 1000.0 / spec.sampleRate, 3)
                                + " ms)");

            expect (impulsePeak > 1.0e-4,
                    "the impulse produced no measurable response at all, so the latency figure "
                    "below is not a measurement of anything");

            expect (measured >= 0,
                    "no impulse emerged at all within " + juce::String (spec.blockSize * spec.numBlocks)
                        + " samples, so this casting produced nothing to measure latency from");

            // **A tolerance, and it is deliberately tight.** Latency is an integer contract with the
            // host; a few samples of disagreement is still a few samples of misalignment on every
            // track in the session. The band exists only for a first output sample that is genuinely
            // tiny rather than exactly zero.
            expectWithinAbsoluteError (measured, declared, 8,
                                       "the impulse did not emerge where the declared latency says "
                                       "it would. A host aligns by the declaration, so this is "
                                       "session-wide misalignment, not a local artefact.");
        }
    }
};

static AutomationLatencyTests automationLatencyTests;
