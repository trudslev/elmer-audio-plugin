#pragma once

#include <juce_dsp/juce_dsp.h>

/**
    MAKEUP and MIX.

    Mix is a true parallel blend. There is no lookahead anywhere in this plugin, so the wet path is
    sample-aligned with the dry by construction and blending cannot comb. If lookahead is ever
    added, the dry tap must be delayed to match - noting it here because that is exactly the kind of
    change that introduces comb filtering silently, months later, in a plugin whose whole job is to
    sound transparent.

    **Both are SMOOTHED per sample, and both used to step at the block boundary.** Measured by
    sweeping each parameter once per block and comparing the worst step at a boundary against the
    worst step inside one: MAKEUP came out **x25.74** and MIX **x3.17** against a static control at
    x1. They are applied as a per-sample gain with nothing damping them, where the six detector
    parameters reach the output through the envelope follower's own ballistics and need no smoothing
    of their own - which is why only three of Elmer's nine are smoothed here and the rest are
    deliberately left alone.

    A host's automation resolution IS its buffer size, so a parameter written once per block and
    applied flat is a staircase at the block rate. That is audible on a gain in a way it is not on a
    time constant.
*/
class OutputStage
{
public:
    /** The initial values are arguments rather than a later `setX`, because
        `SmoothedValue::reset (rate, seconds)` sets the RAMP LENGTH and snaps the value to whatever
        target it last held - zero on a constructed object. Passing them in is what stops the first
        block after every prepare gliding up from nowhere, which is the defect this suite has found
        eleven times in three castings under the name "unguarded reset". */
    void prepare (double sampleRate, float initialMakeupDb, float initialMix01) noexcept;
    void reset() noexcept;

    void setMakeupDb (float db) noexcept;
    void setMix (float mix01) noexcept { mix.setTargetValue (mix01); }

    /** wet has already been through gain reduction and Iron; dry is the untouched input.

        Not `const` any more: advancing a smoother is a state change, and hiding that behind a const
        method would let a caller reasonably believe it could be called twice for one sample. */
    float processSample (float dry, float wet) noexcept
    {
        const float g = makeupGain.getNextValue();
        const float m = mix.getNextValue();

        return dry * (1.0f - m) + wet * g * m;
    }

private:
    static constexpr double smoothingSeconds = 0.02;

    juce::SmoothedValue<float> makeupGain { 1.0f };
    juce::SmoothedValue<float> mix { 1.0f };
};
