#pragma once

/**
    IRON - output transformer saturation.

    Applied AFTER gain reduction, because a real output transformer sees the post-VCA signal. Placed
    before, it would feed the detector its own distortion and change the compression itself rather
    than colouring it.

    The gain-staging trap worth knowing, and the reason this uses tanh(x*d)/d rather than the more
    obvious tanh(x*d)/tanh(d): the normalised form is unity only at FULL SCALE. Below that its gain
    is d/tanh(d), which at drive 7 is a factor of 7. TapeRot and Gatecrasher both carry a comment
    about it; Fifth Member shipped the bug and a feedback loop ran away.
*/
class IronStage
{
public:
    void reset() noexcept {}

    /** amount01: the IRON control, 0-1. */
    void setAmount (float amount01) noexcept { amount = amount01; }

    float processSample (float x) const noexcept;

private:
    float amount = 0.2f;
};
