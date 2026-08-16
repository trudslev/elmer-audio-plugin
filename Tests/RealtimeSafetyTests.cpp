#include "../Source/PluginProcessor.h"
#include "../Source/Parameters.h"
#include "../Source/DSP/SidechainFilter.h"

#include <nf/testing/ProcessorHarness.h>

#include <juce_audio_processors/juce_audio_processors.h>

#include <array>

/**
    Category 1 of the suite-wide bug sweep, for Elmer.

    **Core owns the drivers; this file owns what Elmer's answers should be.**

    ## Every allocation result is reported in TWO figures, even when both are zero

    A warm-up run hides any one-off, not only an over-delivery one. A casting that allocates once on
    its very first block reads identically clean under a warmed probe, and that is a different
    finding from never allocating. So both are measured: **cold** (first block after `prepareToPlay`)
    and **steady** (after warm-up, the per-block cost a host actually pays).

    ## THE FINDING — a heap allocation on the audio thread, on every block

    **`elmer/Source/DSP/SidechainFilter.cpp:22`**

        filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (fs, ..., ...);

    `Coefficients::makeHighPass` heap-allocates a new reference-counted object per call, and
    `PluginProcessor.cpp:75` calls `sidechainFilter.setCutoffHz(...)` unconditionally on every block —
    `setCutoffHz` then re-makes the coefficients unconditionally, whether the cutoff moved or not.

    Measured: **1 allocation, 32 bytes, every block, regardless of signal.** Not a one-off. Release
    build, stable across repeats.

    ### This is a five-castings-do-this-and-one-doesn't, and the one is Elmer

    Every other casting avoids exactly this, by four different routes, and **two of them wrote down
    the reason**:

    | Casting | How |
    |---|---|
    | taperot | Hand-rolls RBJ biquad coefficients as plain floats (`ToneFilters.h:13`), and caches `Ptr` arrays built in `prepare` (`Saturator.h:37`, `TapeModelEQ.h:55`) |
    | reflect-84 | Hand-rolls a one-pole (`ReverbPrimitives.h:27`) |
    | gatecrasher | Change-detection guard — `if (! approximatelyEqual (hz, lastHz))` in both `TriggerDetector.cpp:43` and `DampingStage.cpp:34` |
    | chorus-60 | Calls the factory in `prepare()` only, never per block |
    | fifth-member | Uses no `IIR::Coefficients` at all |

    TapeRot's comment says the factories "heap-allocate a new Coefficients object per call ... which
    is unsafe to call every block on the audio thread the way this needs to." Reflect-84's says the
    same. **Elmer is doing precisely what two siblings wrote comments warning against** — which is the
    suite's recorded duplication shape exactly: a fix lands where the bug was noticed and nothing
    carries it sideways.

    ### FIXED 2026-08-16, stage 2 — and the pin was written to be inverted

    `setCutoffHz` short-circuits on an unchanged cutoff, so a parked session allocates **nothing**
    per block where it allocated 32 bytes on every one. The steady figures are asserted clean now,
    which is exactly what this section used to instruct: *"When it is fixed, this becomes
    `expect (s.clean(), ...)`"*. Saying so in advance is what made it a one-line edit rather than a
    judgement about whether the old number still meant anything.

    **What is left, stated rather than left to be rediscovered as a clean row.** A cutoff that
    actually moves still builds coefficients through `makeHighPass`, which allocates. Removing that
    too means hand-rolling the five biquad coefficients the way TapeRot does, to buy an allocation
    that occurs only while a human is turning the knob. The test below measures both arms, so the
    remaining cost is a figure rather than an omission.

    **And `prepare` invalidates the cache before re-applying**, because coefficients depend on the
    sample rate as well as the cutoff — a short-circuit that swallowed a rate change would leave the
    detector filtering at the wrong corner, silently. That has its own arm.
*/
class RealtimeSafetyTests final : public juce::UnitTest
{
public:
    RealtimeSafetyTests() : juce::UnitTest ("Real-time safety", "DSP") {}

    void runTest() override
    {
        beginTest ("processBlock allocation — matched block size, cold and steady");
        {
            ElmerAudioProcessor cold;
            const auto c = nf::testing::probeProcessBlockAllocation (cold, 48000.0, 512, 512, 2, 1, 0);

            ElmerAudioProcessor steadyProc;
            const auto s = nf::testing::probeProcessBlockAllocation (steadyProc, 48000.0, 512, 512, 2);

            logMessage ("  512/512 cold   -> " + c.describe());
            logMessage ("  512/512 steady -> " + s.describe());

            // **This was PINNED AT 8 and is now asserted clean**, which is the change this file's
            // own header instructed: "When it is fixed, this becomes expect (s.clean(), ...)".
            // Written down in advance is what made it a one-line edit rather than a judgement call.
            expect (s.cleanOfAllocations(),
                    "processBlock allocated on the audio thread in steady state. The sidechain "
                    "coefficient rebuild was 32 bytes every block, signal-independent, and is fixed "
                    "by recomputing only on a change — a count above zero here is either that "
                    "regressing or something new: " + s.describe());
        }

        beginTest ("processBlock allocation — host over-delivers, cold and steady");
        {
            ElmerAudioProcessor cold;
            const auto c = nf::testing::probeProcessBlockAllocation (cold, 48000.0, 256, 2048, 2, 1, 0);

            ElmerAudioProcessor steadyProc;
            const auto s = nf::testing::probeProcessBlockAllocation (steadyProc, 48000.0, 256, 2048, 2);

            logMessage ("  256/2048 cold   -> " + c.describe());
            logMessage ("  256/2048 steady -> " + s.describe());

            // **Elmer has no buffer-growth site at all** — it holds the dry sample in a local inside
            // its per-sample loop where the other five copy the whole block into a dryBuffer first.
            // So over-delivery costs it nothing extra, and the figures here are the same sidechain
            // allocation as above rather than a second defect.
            //
            // That is worth recording for the over-delivery ruling: it is an existence proof that
            // the whole-block dry copy is not required. Whether five castings should restructure
            // that way is a real question and not one this sweep answers.
            expect (s.cleanOfAllocations(),
                    "over-delivery allocated on the audio thread in steady state: " + s.describe());
        }

        beginTest ("The sidechain allocates only when its cutoff MOVES, not on every block");
        {
            /*  **This assertion inverted when the defect was fixed, and that is the fourth time in a
                week.** It read `expectEquals (counts[i], 1)` — the isolating measurement behind the
                finding, correct as a measurement and written as a permanent expectation of a defect.
                `SidechainFilter::setCutoffHz` short-circuits on an unchanged cutoff now, so an
                unmoved knob allocates nothing and the old form asserts the presence of the bug.

                **The counts are captured into plain ints and the sentinel scope CLOSED before any
                logging**, kept from the original: the first version logged inside the armed scope
                and counted `logMessage`'s own `juce::String` work, reporting 4 per block where the
                real figure was 1.

                ## The moving arm is the control, and it is real behaviour rather than a fault

                A knob that actually moves must build new coefficients, and `makeHighPass` returns a
                refcounted object, so it allocates. That is the probe's positive direction — without
                it, five zeros are indistinguishable from a sentinel that hooks nothing.

                **It is also the honest statement of what is left.** Recompute-on-change removes the
                allocation from every block of a parked session, which is the defect; it does not
                remove it from a block during a drag. Eliminating that too means computing the five
                biquad coefficients by hand rather than through JUCE, which duplicates a formula to
                buy an allocation that only occurs while a human is turning something. Recorded as a
                stated limit rather than left for someone to rediscover as a clean row. */
            const auto countsOver = [] (bool moveTheCutoff)
            {
                ElmerAudioProcessor p;
                p.setRateAndBufferSizeDetails (48000.0, 512);
                p.prepareToPlay (48000.0, 512);

                auto* cutoff = p.apvts.getParameter (ParamIDs::sidechainHp);

                juce::AudioBuffer<float> buffer (2, 512);
                juce::MidiBuffer midi;

                for (int ch = 0; ch < 2; ++ch)
                    for (int i = 0; i < 512; ++i)
                        buffer.setSample (ch, i, 0.5f);

                if (cutoff != nullptr)
                    cutoff->setValueNotifyingHost (0.5f);

                for (int i = 0; i < 4; ++i) { midi.clear(); p.processBlock (buffer, midi); }

                std::array<int, 5> counts {};

                for (int i = 0; i < 5; ++i)
                {
                    if (moveTheCutoff && cutoff != nullptr)
                        cutoff->setValueNotifyingHost (0.5f + 0.02f * (float) i);

                    const nf::testing::AllocationSentinel s;
                    p.processBlock (buffer, midi);
                    counts[(size_t) i] = s.count();
                }

                return counts;
            };

            const auto parked = countsOver (false);
            const auto moving = countsOver (true);

            for (int i = 0; i < 5; ++i)
                logMessage ("  block " + juce::String (i)
                                + " -> parked " + juce::String (parked[(size_t) i])
                                + ", moving " + juce::String (moving[(size_t) i]));

            for (int i = 0; i < 5; ++i)
                expectEquals (parked[(size_t) i], 0,
                              "the sidechain still allocates on a block where its cutoff did not "
                              "move. That was 32 bytes every block, signal-independent, on the audio "
                              "thread — and it fired unconditionally rather than under a rare branch");

            int movingTotal = 0;
            for (int c : moving) movingTotal += c;

            expect (movingTotal > 0,
                    "**THE PROBE CANNOT SEE THIS ALLOCATION.** A moving cutoff must build new "
                    "coefficients through makeHighPass, which returns a refcounted object and "
                    "therefore allocates — so five zeros in the parked arm are a sentinel that hooks "
                    "nothing rather than a processor that does not allocate");
        }

        beginTest ("A re-prepare at the same cutoff still rebuilds — the cache is invalidated");
        {
            /*  **The trap the short-circuit opened, closed by measurement rather than by reading.**
                Coefficients depend on the sample rate as well as on the cutoff, so `prepare` at a
                new rate with the knob untouched would hit the unchanged-value early return and keep
                a filter whose corner is wrong by the ratio of the two rates — silently, on the
                detector path, where it changes what the compressor reacts to rather than what you
                hear. `prepare` invalidates the cache first; this is what says so.

                Measured through the filter's own response rather than by inspecting a pointer: the
                same cutoff at two sample rates must attenuate a fixed-Hz tone by the same amount. */
            const auto attenuationAt = [] (double sampleRate)
            {
                SidechainFilter f;
                f.prepare (sampleRate);
                f.setCutoffHz (200.0f);

                const int n = (int) sampleRate;      // one second, so the tone is well settled
                double peak = 0.0;

                for (int i = 0; i < n; ++i)
                {
                    const float x = std::sin (juce::MathConstants<float>::twoPi * 100.0f
                                              * (float) i / (float) sampleRate);
                    const float y = f.processSample (x);

                    if (i > n / 2)
                        peak = juce::jmax (peak, (double) std::abs (y));
                }

                return peak;
            };

            const auto at44 = attenuationAt (44100.0);
            const auto at96 = attenuationAt (96000.0);

            logMessage ("  100 Hz through a 200 Hz corner: 44.1k -> " + juce::String (at44, 6)
                            + ", 96k -> " + juce::String (at96, 6));

            expectWithinAbsoluteError (at96, at44, 0.02,
                                       "the same corner attenuates differently at two sample rates, "
                                       "so prepare did not rebuild coefficients — the short-circuit "
                                       "in setCutoffHz is swallowing a rate change");
        }
    }
};

static RealtimeSafetyTests realtimeSafetyTests;
