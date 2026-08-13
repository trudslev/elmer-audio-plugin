#include "../Source/PluginProcessor.h"
#include "../Source/DSP/LevelDetector.h"
#include "../Source/DSP/SidechainFilter.h"

#include <nf/testing/ProcessorHarness.h>

#include <juce_audio_processors/juce_audio_processors.h>

#include <cmath>
#include <limits>

/**
    Category 2 of the suite-wide bug sweep, for Elmer.

    ## A THIRD shape: a control path that modulates rather than sums

    The sweep has met two reasons output-only scanning is insufficient:

      - **Cascade** — TapeRot's eight `DegradationCore` stages. A subnormal can arrive at a stage
        rather than originate in it, and a later stage can scale it back up, so the output reads
        clean while the middle was not.
      - **Parallel join** — Gatecrasher's and Chorus-60's dry buffers. A wet-path subnormal summed
        against an ordinary dry signal disappears into it.

    **Elmer is neither, and it is the one a survey of decaying state would rank last.** Its audio
    path is two stages deep — `ironStage` then `outputStage` — which is the shallowest in the suite.
    But its detector is a decaying path whose value **never reaches the output at all**:

        const float detectorIn = sidechainFilter.processSample ((dryL + dryR) * 0.5f);
        const float grDb       = detector.processSample (detectorIn);
        const float gain       = std::pow (10.0f, -grDb / 20.0f);
        const float wetL       = ironStage.processSample (dryL * gain);

    `grDb` becomes a **gain multiplier**. The detector's attack/release one-poles decay toward zero
    like any tail, and nothing they hold is ever added to the signal — so **output scanning cannot
    see them by construction**, not because the path is deep or because something masks it.

    That is worth naming as its own shape, because **any casting with a control path that modulates
    rather than sums has it**: sidechains, envelope followers, gain computers, level detectors. A
    survey of "where does state decay" finds these last, if at all, because they are not on the
    signal path being surveyed.

    So this file scans the detector's own output, in dB, rather than the processor's.

    ## What the scanner can and cannot say here

    Driven directly, without `ScopedNoDenormals`, so it measures what the detector **produces**. In
    the shipping plugin the flush-to-zero guard is in force for the whole of `processBlock`, so a
    subnormal here is flushed by the hardware before it multiplies anything. A positive result is
    therefore a statement about the detector's arithmetic, not about audible behaviour — which is
    exactly the distinction the guarded output scans cannot draw.
*/
class NumericalRobustnessTests final : public juce::UnitTest
{
public:
    NumericalRobustnessTests() : juce::UnitTest ("Numerical robustness", "DSP") {}

    void runTest() override
    {
        beginTest ("The DETECTOR's own decay — the path the output cannot see");
        {
            LevelDetector detector;
            detector.prepare (48000.0);
            detector.setAttackMs (0.1f);
            detector.setRelease (10.0f, false);      // the longest release: the slowest decay
            detector.computer().setThresholdDb (-40.0f);
            detector.computer().setRatio (10.0f);

            // Hit it hard so there is real gain reduction to decay from.
            for (int i = 0; i < 48000; ++i)
                detector.processSample (1.0f);

            int subnormals = 0, nans = 0, infinities = 0;
            // Sentinel at infinity, not 1.0 — the first version started at 1.0 and so could only
            // ever report <= 1.0. It reported exactly 1.0, which is the sentinel rather than a
            // measurement: a figure that cannot be smaller than its own starting value is not one.
            double smallestNonZero = std::numeric_limits<double>::infinity();
            int samplesToZero = -1;

            // Ten seconds of silence at 48 k. A detector's release is measured in seconds, so a
            // short tail scans the part of the decay where the values are still large.
            for (int i = 0; i < 480000; ++i)
            {
                const float grDb = detector.processSample (0.0f);

                switch (std::fpclassify (grDb))
                {
                    case FP_SUBNORMAL: ++subnormals; break;
                    case FP_NAN:       ++nans;       break;
                    case FP_INFINITE:  ++infinities; break;
                    default: break;
                }

                const double a = std::abs ((double) grDb);

                if (a > 0.0 && a < smallestNonZero)
                    smallestNonZero = a;

                if (a == 0.0 && samplesToZero < 0)
                    samplesToZero = i;
            }

            logMessage ("  detector -> " + juce::String (subnormals) + " subnormal, "
                            + juce::String (nans) + " NaN, " + juce::String (infinities) + " Inf");
            logMessage ("  smallest non-zero |grDb| -> "
                            + (std::isfinite (smallestNonZero) ? juce::String (smallestNonZero, 12)
                                                               : juce::String ("none seen"))
                            + (samplesToZero >= 0
                                   ? ", reached exactly 0 after " + juce::String (samplesToZero) + " samples"
                                   : ", never reached exactly 0 in 10 s"));

            expectEquals (nans, 0, "the detector produced NaN");
            expectEquals (infinities, 0, "the detector produced Inf");

            // Reported rather than asserted zero: in the shipping plugin ScopedNoDenormals flushes
            // these before they multiply anything, so a non-zero count here is a statement about the
            // detector's arithmetic rather than about what a user hears.
            logMessage (subnormals > 0
                            ? "  NOTE: the detector reaches subnormal territory unguarded — flushed "
                              "by ScopedNoDenormals in the plugin, invisible to any output scan"
                            : "  the detector does not reach subnormal territory even unguarded");
        }

        beginTest ("The sidechain filter's own decay — the other half of the control path");
        {
            SidechainFilter filter;
            filter.prepare (48000.0);
            filter.setCutoffHz (500.0f);             // the top of its range, the fastest decay

            for (int i = 0; i < 4800; ++i)
                filter.processSample (1.0f);

            int subnormals = 0;
            bool finite = true;

            for (int i = 0; i < 480000; ++i)
            {
                const float y = filter.processSample (0.0f);

                if (std::fpclassify (y) == FP_SUBNORMAL)
                    ++subnormals;

                if (! std::isfinite (y))
                    finite = false;
            }

            // **FINDING. The whole decay is subnormal.** 478 114 of 480 000 tail samples, measured
            // — the filter's state falls into subnormal territory almost immediately after input
            // stops and stays there for the full ten seconds.
            //
            // In the shipping plugin ScopedNoDenormals flushes these, so this is not audible and not
            // a CPU cost on a platform honouring FTZ. What makes it worth recording is WHERE it is:
            // the sidechain filter is on the control path, feeding the detector, so its value never
            // reaches the output. **No output scan of any depth could have found this** — not
            // because something masked it, but because it is not on the signal path at all.
            //
            // That is the third shape this sweep has met, after "cascade" and "parallel join", and
            // it generalises: any casting with a control path that MODULATES rather than sums has
            // it. Sidechains, envelope followers, gain computers, level detectors.
            //
            // Classification: needs a ruling, measured. The guard covers it today; the exposure is
            // that nothing structural keeps it covered, and the same class is a real cost anywhere
            // FTZ is not in force.
            logMessage ("  sidechain filter -> " + juce::String (subnormals)
                            + " subnormal of 480000 tail samples");

            expectGreaterThan (subnormals, 0,
                               "the sidechain filter no longer reaches subnormal territory. That is "
                               "an improvement, not a failure — update this expectation and the "
                               "comment above rather than reverting whatever changed.");
            expect (finite, "the sidechain filter produced non-finite output");
        }

        beginTest ("The processor's output, for completeness");
        {
            ElmerAudioProcessor processor;

            nf::testing::RenderSpec spec;
            spec.numBlocks = 16;

            const auto report = nf::testing::scanTail (processor, spec, 4000);
            logMessage ("  output -> " + report.describe());

            expectEquals (report.subnormals, 0,
                          "subnormals reached the output: " + report.describe());
            expect (report.clean());
        }
    }
};

static NumericalRobustnessTests numericalRobustnessTests;
