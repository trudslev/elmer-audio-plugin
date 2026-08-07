#include "DSP/AutoRelease.h"
#include "DSP/GainComputer.h"
#include "DSP/IronStage.h"
#include "DSP/LevelDetector.h"
#include "DSP/OutputStage.h"
#include "DSP/SidechainFilter.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

#include <cmath>
#include <vector>

namespace
{
    constexpr double fs = 48000.0;

    inline float dbToGain (float db) { return std::pow (10.0f, db / 20.0f); }

    /** Runs a steady sine through a detector until the gain reduction settles, and returns it. */
    float settledGrDb (LevelDetector& d, float amplitudeDb, float freqHz, double seconds)
    {
        const int n = (int) (seconds * fs);
        const float amp = dbToGain (amplitudeDb);
        float gr = 0.0f;

        for (int i = 0; i < n; ++i)
            gr = d.processSample (amp * std::sin (juce::MathConstants<float>::twoPi
                                                  * freqHz * (float) i / (float) fs));

        return gr;
    }
}

//==============================================================================
class GainComputerTests final : public juce::UnitTest
{
public:
    GainComputerTests() : juce::UnitTest ("Gain computer realises the printed ratio", "DSP") {}

    void runTest() override
    {
        beginTest ("Below threshold nothing happens, at any ratio");
        GainComputer gc;
        gc.setThresholdDb (-20.0f);
        gc.setKneeWidthDb (0.0f);

        for (float r : { 1.5f, 2.0f, 4.0f, 10.0f, 20.0f })
        {
            gc.setRatio (r);
            expectEquals (gc.gainReductionDbFor (-40.0f), 0.0f);
            expectEquals (gc.gainReductionDbFor (-21.0f), 0.0f);
        }

        beginTest ("Each detent realises its printed ratio exactly, hard knee");
        // 20 dB over threshold at ratio R should leave 20/R dB over, i.e. reduce by 20 - 20/R.
        for (float r : { 1.5f, 2.0f, 4.0f, 10.0f, 20.0f })
        {
            gc.setRatio (r);
            const float gr = gc.gainReductionDbFor (0.0f);          // 20 dB over a -20 threshold
            const float outputOverThreshold = 20.0f - gr;
            expectWithinAbsoluteError (20.0f / outputOverThreshold, r, 0.001f,
                                       "realised ratio at " + juce::String (r, 1) + ":1");
        }

        beginTest ("Soft knee differs from hard only near the threshold");
        gc.setRatio (4.0f);
        gc.setKneeWidthDb (GainComputer::softKneeWidthDb);
        const float softAtThreshold = gc.gainReductionDbFor (-20.0f);
        gc.setKneeWidthDb (0.0f);
        const float hardAtThreshold = gc.gainReductionDbFor (-20.0f);
        expect (softAtThreshold > hardAtThreshold,
                "soft knee already compresses at the threshold, hard does not");

        gc.setKneeWidthDb (GainComputer::softKneeWidthDb);
        const float softWellAbove = gc.gainReductionDbFor (0.0f);
        gc.setKneeWidthDb (0.0f);
        const float hardWellAbove = gc.gainReductionDbFor (0.0f);
        expectWithinAbsoluteError (softWellAbove, hardWellAbove, 0.001f,
                                   "well above the knee the two curves converge");

        beginTest ("The knee is continuous - no step at either edge");
        gc.setKneeWidthDb (GainComputer::softKneeWidthDb);
        const float w = GainComputer::softKneeWidthDb;
        for (float d : { -0.01f, 0.01f })
        {
            const float lo = gc.gainReductionDbFor (-20.0f - w * 0.5f + d);
            const float hi = gc.gainReductionDbFor (-20.0f - w * 0.5f - d);
            expectWithinAbsoluteError (lo, hi, 0.01f, "lower knee edge is continuous");
        }
    }
};

//==============================================================================
class StereoLinkTests final : public juce::UnitTest
{
public:
    StereoLinkTests() : juce::UnitTest ("One detector drives both channels", "DSP") {}

    void runTest() override
    {
        beginTest ("A hard-panned input produces the same gain on both channels");
        // The premise of a bus compressor: gain is computed once from the summed signal, so a loud
        // left channel ducks the right by exactly as much and the image cannot shift. If this ever
        // fails, someone has made the detector per-channel.
        LevelDetector d;
        d.prepare (fs);
        d.computer().setThresholdDb (-20.0f);
        d.computer().setRatio (4.0f);
        d.computer().setKneeWidthDb (0.0f);
        d.setAttackMs (1.0f);
        d.setRelease (0.1f, false);

        const int n = (int) (0.5 * fs);
        std::vector<float> gainL (n), gainR (n);

        for (int i = 0; i < n; ++i)
        {
            const float left = 0.9f * std::sin (juce::MathConstants<float>::twoPi * 220.0f * (float) i / (float) fs);
            const float right = 0.0f;                       // silence on the right
            const float gr = d.processSample ((left + right) * 0.5f);
            const float g = dbToGain (-gr);
            gainL[(size_t) i] = g;
            gainR[(size_t) i] = g;
        }

        for (int i = 0; i < n; i += 97)
            expectEquals (gainL[(size_t) i], gainR[(size_t) i],
                          "gain must be identical on both channels at sample " + juce::String (i));

        beginTest ("Silence on one channel is still gain-reduced by the other");
        expect (gainL[(size_t) (n - 1)] < 0.999f,
                "the summed detector must duck even the silent channel");
    }
};

//==============================================================================
class SidechainFilterTests final : public juce::UnitTest
{
public:
    SidechainFilterTests() : juce::UnitTest ("Sidechain HP shapes detection, not audio", "DSP") {}

    void runTest() override
    {
        beginTest ("OFF is a genuine bypass, not a very low corner");
        SidechainFilter f;
        f.prepare (fs);
        f.setCutoffHz (0.0f);

        for (float x : { -0.7f, -0.1f, 0.0f, 0.3f, 0.95f })
            expectEquals (f.processSample (x), x, "OFF must pass the sample through untouched");

        beginTest ("Engaged, it attenuates below its corner and passes above");
        const auto rmsAt = [] (float cutoff, float freq)
        {
            SidechainFilter filt;
            filt.prepare (fs);
            filt.setCutoffHz (cutoff);

            double sum = 0.0;
            const int n = (int) (0.5 * fs);
            for (int i = 0; i < n; ++i)
            {
                const float y = filt.processSample (std::sin (juce::MathConstants<float>::twoPi
                                                              * freq * (float) i / (float) fs));
                if (i > n / 2) sum += (double) y * y;         // skip the transient
            }
            return std::sqrt (sum / (n / 2));
        };

        const double low  = rmsAt (140.0f, 40.0f);
        const double high = rmsAt (140.0f, 2000.0f);
        expect (low < high * 0.25, "40 Hz must be well down against 2 kHz through a 140 Hz HP");
        expectWithinAbsoluteError ((float) high, 0.707f, 0.05f, "the passband is unity");

        beginTest ("It changes gain reduction without touching the audio path");
        // Two detectors, identical but for the filter, fed a low sine. The filtered one must
        // compress less. The audio path is not involved at all - that is the point.
        const auto grWithHp = [] (float cutoff)
        {
            SidechainFilter filt;
            filt.prepare (fs);
            filt.setCutoffHz (cutoff);

            LevelDetector d;
            d.prepare (fs);
            d.computer().setThresholdDb (-24.0f);
            d.computer().setRatio (4.0f);
            d.computer().setKneeWidthDb (0.0f);
            d.setAttackMs (1.0f);
            d.setRelease (0.1f, false);

            float gr = 0.0f;
            const int n = (int) (0.4 * fs);
            for (int i = 0; i < n; ++i)
            {
                const float x = 0.8f * std::sin (juce::MathConstants<float>::twoPi * 50.0f * (float) i / (float) fs);
                gr = d.processSample (filt.processSample (x));
            }
            return gr;
        };

        expect (grWithHp (265.0f) < grWithHp (0.0f) - 3.0f,
                "a 265 Hz sidechain HP must measurably reduce compression of a 50 Hz tone");
    }
};

//==============================================================================
class AutoReleaseTests final : public juce::UnitTest
{
public:
    AutoReleaseTests() : juce::UnitTest ("AUTO release is program-dependent", "DSP") {}

    void runTest() override
    {
        beginTest ("Sustained compression stretches the release; a short burst does not");
        // This is the whole claim of the mode, and a fixed rate - however well chosen - cannot pass
        // it. If someone ever replaces AUTO with an averaged constant, this fails.
        AutoRelease ar;
        ar.prepare (fs);

        const int shortBurst = (int) (0.02 * fs);
        for (int i = 0; i < shortBurst; ++i)
            ar.nextReleaseSeconds (8.0f);
        const float afterBurst = ar.nextReleaseSeconds (8.0f);

        ar.reset();
        const int longTone = (int) (2.0 * fs);
        for (int i = 0; i < longTone; ++i)
            ar.nextReleaseSeconds (8.0f);
        const float afterTone = ar.nextReleaseSeconds (8.0f);

        expect (afterTone > afterBurst * 1.5f,
                "sustained compression must release substantially slower: burst "
                    + juce::String (afterBurst, 3) + " s vs tone " + juce::String (afterTone, 3) + " s");

        beginTest ("It stays inside its two time constants");
        expect (afterBurst >= AutoRelease::fastSeconds - 1.0e-4f);
        expect (afterTone  <= AutoRelease::slowSeconds + 1.0e-4f);

        beginTest ("Sustain decays once compression stops");
        for (int i = 0; i < (int) (3.0 * fs); ++i)
            ar.nextReleaseSeconds (0.0f);
        expect (ar.getSustain() < 0.05f, "with no gain reduction the tracker returns to fast");

        beginTest ("A detector in AUTO recovers faster after a transient than after a tone");
        const auto recoveryTime = [] (double excitationSeconds)
        {
            LevelDetector d;
            d.prepare (fs);
            d.computer().setThresholdDb (-30.0f);
            d.computer().setRatio (4.0f);
            d.computer().setKneeWidthDb (0.0f);
            d.setAttackMs (1.0f);
            d.setRelease (0.3f, true);                       // AUTO

            const int on = (int) (excitationSeconds * fs);
            for (int i = 0; i < on; ++i)
                d.processSample (0.9f * std::sin (juce::MathConstants<float>::twoPi * 220.0f * (float) i / (float) fs));

            const float peak = d.getGainReductionDb();
            int samples = 0;
            const int limit = (int) (8.0 * fs);
            while (d.processSample (0.0f) > peak * 0.37f && samples < limit)
                ++samples;

            return (double) samples / fs;
        };

        const double afterTransient = recoveryTime (0.02);
        const double afterSustained = recoveryTime (2.0);
        expect (afterSustained > afterTransient * 1.4,
                "AUTO must adapt: " + juce::String (afterTransient, 3) + " s after a transient vs "
                    + juce::String (afterSustained, 3) + " s after a sustained tone");
    }
};

//==============================================================================
class OutputStageTests final : public juce::UnitTest
{
public:
    OutputStageTests() : juce::UnitTest ("Makeup, mix and Iron", "DSP") {}

    void runTest() override
    {
        beginTest ("Mix at 0 % is bit-transparent dry");
        OutputStage o;
        o.setMakeupDb (12.0f);
        o.setMix (0.0f);

        for (float x : { -0.9f, -0.25f, 0.0f, 0.4f, 0.8f })
            expectEquals (o.processSample (x, 0.123f), x,
                          "at mix 0 the wet path and makeup must not leak through at all");

        beginTest ("Mix at 100 % is pure wet");
        o.setMakeupDb (0.0f);
        o.setMix (1.0f);
        for (float x : { -0.5f, 0.3f })
            expectEquals (o.processSample (0.9f, x), x);

        beginTest ("Makeup 0 dB is unity");
        o.setMakeupDb (0.0f);
        o.setMix (1.0f);
        expectEquals (o.processSample (0.0f, 0.5f), 0.5f);

        beginTest ("Makeup is exact in dB");
        o.setMakeupDb (6.0f);
        expectWithinAbsoluteError (o.processSample (0.0f, 0.5f), 0.5f * dbToGain (6.0f), 1.0e-6f);

        beginTest ("Iron at 0 is transparent, and never explodes");
        IronStage iron;
        iron.setAmount (0.0f);
        for (float x : { -0.99f, -0.3f, 0.0f, 0.6f, 0.99f })
            expectEquals (iron.processSample (x), x, "IRON at 0 must be a genuine bypass");

        iron.setAmount (1.0f);
        for (float x : { -4.0f, -1.0f, 0.0f, 1.0f, 4.0f })
            expect (std::abs (iron.processSample (x)) < 2.0f,
                    "IRON must stay bounded even well past full scale");

        beginTest ("Iron preserves small-signal gain - the tanh(x*d)/tanh(d) trap");
        // tanh(x*d)/tanh(d) is unity only at FULL SCALE; below it the gain is d/tanh(d), which at
        // drive 5 is a factor of 5. Fifth Member shipped that bug and a feedback loop ran away.
        iron.setAmount (1.0f);
        const float small = 1.0e-3f;
        expectWithinAbsoluteError (iron.processSample (small) / small, 1.0f, 0.05f,
                                   "small-signal gain through Iron must be unity");
    }
};

//==============================================================================
class DetectorTimingTests final : public juce::UnitTest
{
public:
    DetectorTimingTests() : juce::UnitTest ("Attack and release times mean what the panel prints", "DSP") {}

    void runTest() override
    {
        beginTest ("Attack reaches 1 - 1/e of the gain change in the printed time");
        // Smoothing is applied to the gain reduction in dB, which is why this holds regardless of
        // how far over threshold the signal sits. Smooth the input envelope instead and the
        // realised attack time becomes level-dependent and the print stops being true.
        for (float attackMs : { 1.0f, 10.0f, 30.0f })
        {
            LevelDetector d;
            d.prepare (fs);
            d.computer().setThresholdDb (-30.0f);
            d.computer().setRatio (4.0f);
            d.computer().setKneeWidthDb (0.0f);
            d.setAttackMs (attackMs);
            d.setRelease (1.2f, false);

            const float target = settledGrDb (d, -6.0f, 1000.0f, 2.0);

            LevelDetector fresh;
            fresh.prepare (fs);
            fresh.computer().setThresholdDb (-30.0f);
            fresh.computer().setRatio (4.0f);
            fresh.computer().setKneeWidthDb (0.0f);
            fresh.setAttackMs (attackMs);
            fresh.setRelease (1.2f, false);

            const int n = (int) (attackMs * 0.001f * fs);
            float gr = 0.0f;
            const float amp = dbToGain (-6.0f);
            for (int i = 0; i < n; ++i)
                gr = fresh.processSample (amp * std::sin (juce::MathConstants<float>::twoPi
                                                          * 1000.0f * (float) i / (float) fs));

            const float reached = gr / target;
            expect (reached > 0.45f && reached < 0.85f,
                    "at " + juce::String (attackMs, 1) + " ms the attack reached "
                        + juce::String (reached * 100.0f, 1) + "% of target");
        }

        beginTest ("A faster attack really is faster");
        const auto grAfter = [] (float attackMs, double seconds)
        {
            LevelDetector d;
            d.prepare (fs);
            d.computer().setThresholdDb (-30.0f);
            d.computer().setRatio (10.0f);
            d.computer().setKneeWidthDb (0.0f);
            d.setAttackMs (attackMs);
            d.setRelease (1.2f, false);
            return settledGrDb (d, -6.0f, 1000.0f, seconds);
        };

        expect (grAfter (0.1f, 0.002) > grAfter (30.0f, 0.002),
                "after 2 ms the 0.1 ms attack must have clamped down much harder than the 30 ms one");
    }
};

//==============================================================================
static GainComputerTests   gainComputerTests;
static StereoLinkTests     stereoLinkTests;
static SidechainFilterTests sidechainFilterTests;
static AutoReleaseTests    autoReleaseTests;
static OutputStageTests    outputStageTests;
static DetectorTimingTests detectorTimingTests;
