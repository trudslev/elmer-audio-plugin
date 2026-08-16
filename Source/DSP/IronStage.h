#pragma once

#include <juce_dsp/juce_dsp.h>

/**
    IRON - output transformer saturation.

    Applied AFTER gain reduction, because a real output transformer sees the post-VCA signal. Placed
    before, it would feed the detector its own distortion and change the compression itself rather
    than colouring it.

    The gain-staging trap worth knowing, and the reason this uses tanh(x*d)/d rather than the more
    obvious tanh(x*d)/tanh(d): the normalised form is unity only at FULL SCALE. Below that its gain
    is d/tanh(d), which at drive 7 is a factor of 7. TapeRot and Gatecrasher both carry a comment
    about it; Fifth Member shipped the bug and a feedback loop ran away.

    **The amount is SMOOTHED per sample, and it used to step at the block boundary** - measured at
    **x10.65** boundary-to-interior against a static control at x1. It is a drive figure feeding a
    per-sample nonlinearity, so a once-per-block write applied flat is a staircase at the block rate.
*/
class IronStage
{
public:
    /** Initial value as an argument - see OutputStage::prepare for why it is not a later setter. */
    void prepare (double sampleRate, float initialAmount01) noexcept;
    void reset() noexcept;

    /** amount01: the IRON control, 0-1. */
    void setAmount (float amount01) noexcept { amount.setTargetValue (amount01); }

    float processSample (float x) noexcept;

private:
    static constexpr double smoothingSeconds = 0.02;

    juce::SmoothedValue<float> amount { 0.2f };
};
