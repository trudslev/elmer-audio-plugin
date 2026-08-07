#pragma once

/**
    MAKEUP and MIX.

    Mix is a true parallel blend. There is no lookahead anywhere in this plugin, so the wet path is
    sample-aligned with the dry by construction and blending cannot comb. If lookahead is ever
    added, the dry tap must be delayed to match - noting it here because that is exactly the kind of
    change that introduces comb filtering silently, months later, in a plugin whose whole job is to
    sound transparent.
*/
class OutputStage
{
public:
    void setMakeupDb (float db) noexcept;
    void setMix (float mix01) noexcept { mix = mix01; }

    /** wet has already been through gain reduction and Iron; dry is the untouched input. */
    float processSample (float dry, float wet) const noexcept
    {
        return dry * (1.0f - mix) + wet * makeupGain * mix;
    }

private:
    float makeupGain = 1.0f;
    float mix = 1.0f;
};
