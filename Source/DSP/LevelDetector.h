#pragma once

#include "AutoRelease.h"
#include "GainComputer.h"

/**
    The single stereo-linked detector.

    ONE detector, fed a mono sum of both channels, producing ONE gain-reduction value applied
    identically to left and right. That is the entire premise of a bus compressor: the stereo image
    cannot shift because there is nothing to shift it. There is no link control and no second
    detector — do not add either.

    Smoothing is applied in the dB domain to the gain reduction itself, not to the input envelope.
    That is what makes ATTACK and RELEASE mean what the panel prints: the attack time is the time to
    reach 1 − 1/e of the *gain change*, which is the thing a listener hears and the thing the
    printed scale claims. Smoothing the input envelope first and computing gain from it makes the
    realised times depend on how far over threshold the signal is.
*/
class LevelDetector
{
public:
    void prepare (double sampleRate) noexcept;
    void reset() noexcept;

    void setAttackMs (float ms) noexcept;
    /** Fixed release in seconds. Pass useAuto = true for the switch's fifth position, where the
        time comes from AutoRelease instead and this value is ignored. */
    void setRelease (float seconds, bool useAuto) noexcept;

    GainComputer& computer() noexcept { return gainComputer; }

    /** One sample of the (already high-passed) mono detector signal in, gain reduction in dB out. */
    float processSample (float detectorSample) noexcept;

    float getGainReductionDb() const noexcept { return currentGrDb; }

private:
    float releaseCoeffFor (float seconds) const noexcept;

    GainComputer gainComputer;
    AutoRelease autoRelease;

    double fs = 44100.0;
    float attackCoeff = 1.0f;
    float fixedReleaseCoeff = 1.0f;
    float fixedReleaseSeconds = 0.3f;
    bool  autoMode = false;

    float currentGrDb = 0.0f;
};
