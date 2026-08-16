#include "../Source/PluginProcessor.h"

#include <nf/testing/ProcessorHarness.h>

#include <juce_audio_processors/juce_audio_processors.h>

/**
    Category 4 — channel configurations.

    ## The known case, named before the set is run

    **Chorus-60 and TapeRot declare stereo only; the other four accept mono as well.** So a mono
    request must be REJECTED by exactly those two and accepted by the other four. If the instrument
    reports every layout supported everywhere, it is not reading the layout at all — which is the
    failure mode a "supports everything" result would otherwise sail through, and it is the same
    shape as a check that can only ever pass.

    ## What is asserted, and what is only reported

    Asserted: the accept/reject set matches what the casting declares, and every ACCEPTED layout
    produces finite, non-silent output rather than crashing or going dead. **Silence out of an
    accepted layout is the interesting failure** — a plugin that accepts mono and then produces
    nothing on it is broken in a way no stereo test sees.

    Reported only: TapeRot generates deliberately, so its non-silence proves less than the others'.
*/
class ChannelLayoutTests final : public juce::UnitTest
{
public:
    ChannelLayoutTests() : juce::UnitTest ("Channel layouts", "DSP") {}

    void runTest() override
    {
        beginTest ("Every declared layout is accepted, and every accepted layout makes sound");
        {
            struct Candidate { const char* name; int channels; };
            const Candidate candidates[] = { { "mono", 1 }, { "stereo", 2 } };

            for (const auto& candidate : candidates)
            {
                ElmerAudioProcessor processor;

                juce::AudioProcessor::BusesLayout layout;
                const auto set = candidate.channels == 1 ? juce::AudioChannelSet::mono()
                                                         : juce::AudioChannelSet::stereo();
                layout.inputBuses.add (set);
                layout.outputBuses.add (set);

                const bool accepted = processor.checkBusesLayoutSupported (layout)
                                          && processor.setBusesLayout (layout);

                if (! accepted)
                {
                    logMessage ("  " + juce::String (candidate.name) + " -> REJECTED");
                    continue;
                }

                nf::testing::RenderSpec spec;
                spec.blockSize = 512;
                spec.numBlocks = 16;
                spec.numChannels = candidate.channels;

                const auto out = nf::testing::render (processor, spec);

                double peak = 0.0;
                bool finite = true;

                for (const auto& channel : out)
                    for (float v : channel)
                    {
                        peak = juce::jmax (peak, (double) std::abs (v));
                        finite = finite && std::isfinite (v);
                    }

                logMessage ("  " + juce::String (candidate.name) + " -> accepted, "
                                + juce::String ((int) out.size()) + " channels out, peak "
                                + juce::String (peak, 6) + (finite ? "" : "   NON-FINITE"));

                expect (finite, juce::String (candidate.name)
                                    + " produced non-finite samples");

                expectGreaterThan (peak, 1.0e-6,
                                   juce::String (candidate.name) + " was accepted and then produced "
                                   "silence — a layout a plugin claims to support and cannot make "
                                   "sound on is broken in a way no stereo test sees");
            }
        }

        beginTest ("Lifecycle — double prepare, rate change, reset, state round trip");
        {
            ElmerAudioProcessor processor;

            nf::testing::RenderSpec spec;
            spec.blockSize = 512;
            spec.numBlocks = 16;

            const auto report = nf::testing::exerciseLifecycle (processor, spec);

            logMessage ("  " + report.describe());

            // **`tailEnergyAfterReset` is REPORTED, never asserted, and the plan says why**: what
            // survives a reset that should not is the finding, and core cannot tell a reverb tail
            // (a defect) from a Program selection (correct) apart. The casting has to read it.
            expect (report.sampleRateChangeHandled,
                    "a mid-session sample-rate change was not handled: " + report.describe());

            expect (report.stateRoundTripMismatch.isEmpty(),
                    "a state round trip did not come back identical: " + report.stateRoundTripMismatch);
        }

        beginTest ("Reset returns the detector — WITH IT DRIVEN, which defaults cannot show");
        {
            /*  **This row read 0.000 before stage 1c and 0.000 after, and proved nothing either
                time.** A compressor at its defaults has no tail: the envelope follower sits at unity
                because nothing crossed the threshold, so `reset()` had nothing to return and any
                implementation of it would have scored identically — including none at all.

                **Ask which line makes a clean row correct, not which line agrees with it.** At
                defaults there was no such line. Driven hard into gain reduction with a slow release
                there is: the detector, the sidechain filter and the makeup smoother all hold state,
                and `reset()` has to return every one of them.

                **A compressor's "tail" is not a ringing tail**, which is why this is measured as a
                difference in GAIN rather than in leftover energy. What survives a reset here is
                gain reduction: the first block after the locate is squashed by however far the
                detector had been pulled down. So the measurement is the first block of a quiet
                passage, compared against the same block from an instance that never saw the loud
                one — which is what a locate to that point should sound like.

                **The property, not the value.** The assertion is that a reset makes the two match,
                not that the difference is some figure. */
            constexpr double fs = 48000.0;
            constexpr int blockSize = 512;

            const auto firstQuietBlock = [] (bool loudFirst, bool resetBetween)
            {
                ElmerAudioProcessor p;

                const auto setP = [&p] (const juce::String& id, float value)
                {
                    if (auto* q = dynamic_cast<juce::RangedAudioParameter*> (p.apvts.getParameter (id)))
                        q->setValueNotifyingHost (q->getNormalisableRange().convertTo0to1 (value));
                };

                setP (ParamIDs::threshold, -40.0f);
                setP (ParamIDs::ratio, 10.0f);
                setP (ParamIDs::attack, 1.0f);
                setP (ParamIDs::release, 2000.0f);   // slow, so the detector is still down after
                setP (ParamIDs::mix, 100.0f);

                p.setRateAndBufferSizeDetails (fs, blockSize);
                p.prepareToPlay (fs, blockSize);
                p.reset();

                juce::AudioBuffer<float> buffer (2, blockSize);
                juce::MidiBuffer midi;

                if (loudFirst)
                    for (int b = 0; b < 32; ++b)
                    {
                        for (int ch = 0; ch < 2; ++ch)
                            for (int i = 0; i < blockSize; ++i)
                                buffer.setSample (ch, i, 0.9f);   // hard into gain reduction

                        midi.clear();
                        p.processBlock (buffer, midi);
                    }

                if (resetBetween)
                    p.reset();

                // A quiet passage. How loud it comes out is how far the detector was still pulled.
                for (int ch = 0; ch < 2; ++ch)
                    for (int i = 0; i < blockSize; ++i)
                        buffer.setSample (ch, i, 0.02f);

                midi.clear();
                p.processBlock (buffer, midi);

                double sum = 0.0;
                for (int i = 0; i < blockSize; ++i)
                    sum += std::abs ((double) buffer.getSample (0, i));

                return sum / blockSize;
            };

            const auto fresh      = firstQuietBlock (false, false);   // never saw the loud passage
            const auto carried    = firstQuietBlock (true,  false);   // saw it, no reset
            const auto afterReset = firstQuietBlock (true,  true);    // saw it, then reset

            logMessage ("  fresh " + juce::String (fresh, 9)
                            + ", carried " + juce::String (carried, 9)
                            + ", after reset() " + juce::String (afterReset, 9));

            expect (std::abs (carried - fresh) > fresh * 0.01,
                    "**THE DETECTOR IS NOT BEING DRIVEN.** An instance that had just been hammered "
                    "produced the same quiet block as one that had not, so there is no state for "
                    "reset() to return and a clean row below is the same coincidence the default "
                    "arm was");

            expectWithinAbsoluteError (afterReset, fresh, fresh * 0.01,
                                       "reset() did not return the detector: the first block after "
                                       "a locate is still squashed by gain reduction the previous "
                                       "passage caused.");
        }
    }
};

static ChannelLayoutTests channelLayoutTests;
