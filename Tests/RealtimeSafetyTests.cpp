#include "../Source/PluginProcessor.h"

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

    ### Why this test pins the defect instead of asserting it away

    The sweep fixes nothing; the deliverable is a classified report. But a permanently failing suite
    is not a report, it is a broken build. So the steady figure is **pinned at what it currently is**,
    which fails if it gets worse and does not pretend it is right.

    **When it is fixed, this becomes `expect (s.clean(), ...)`** — the assertion the other five
    castings already carry. Gatecrasher's guard is three lines and is the obvious port.
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

            // **PINNED, NOT PASSED.** One 32-byte allocation per block from SidechainFilter.cpp:22 —
            // see this file's header. The target is zero and the port is Gatecrasher's guard.
            expectEquals (s.allocations, 8,
                          "the per-block allocation count moved. If it went DOWN, the sidechain "
                          "coefficient defect is fixed and this should become expect(s.clean()). If "
                          "it went UP, something new allocates on the audio thread.");
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
            expectEquals (s.allocations, 8, "over-delivery changed the allocation count");
        }

        beginTest ("The sidechain allocation is unconditional, not signal- or change-dependent");
        {
            // The isolating measurement behind the finding. Counts are captured into plain ints and
            // the sentinel scope CLOSED before any logging: the first version of this logged inside
            // the armed scope and counted logMessage's own juce::String work, reporting 4 per block
            // where the real figure is 1 — the same error as arming a probe around render().
            ElmerAudioProcessor p;
            p.setRateAndBufferSizeDetails (48000.0, 512);
            p.prepareToPlay (48000.0, 512);

            juce::AudioBuffer<float> buffer (2, 512);
            juce::MidiBuffer midi;

            for (int ch = 0; ch < 2; ++ch)
                for (int i = 0; i < 512; ++i)
                    buffer.setSample (ch, i, 0.5f);

            for (int i = 0; i < 4; ++i) { midi.clear(); p.processBlock (buffer, midi); }

            std::array<int, 5> counts {};

            for (int i = 0; i < 5; ++i)
            {
                const nf::testing::AllocationSentinel s;
                p.processBlock (buffer, midi);
                counts[(size_t) i] = s.count();
            }

            for (int i = 0; i < 5; ++i)
            {
                logMessage ("  block " + juce::String (i) + " -> " + juce::String (counts[(size_t) i])
                                + " allocation(s)");

                // Identical every block is the point: a converging count would be a warm-up
                // artefact, and a signal-dependent one would be a different defect.
                expectEquals (counts[(size_t) i], 1,
                              "the per-block sidechain coefficient allocation changed shape");
            }
        }
    }
};

static RealtimeSafetyTests realtimeSafetyTests;
