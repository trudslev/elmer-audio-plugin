#pragma once

#include "ElmerTheme.h"

/**
    The signature element: an analog moving-coil gain-reduction meter.

    Deliberately not the dark phosphor scope every sibling uses, and deliberately a single needle.
    A bus compressor is stereo-linked - one detector drives both channels, so gain reduction is
    identical left and right. A second needle would imply dual-mono, the opposite of glue. Do not
    add one.

    The scale reads gain reduction, not level: 0 at the far RIGHT, values increasing leftward, and
    there is NO red zone. Red implies a fault and gain reduction is not one.

    The face is a static bitmap; the needle is a separate sprite rotated about a pivot that sits
    below the visible face edge, exactly as a real movement's does.
*/
class GainReductionMeter final : public juce::Component
{
public:
    GainReductionMeter();

    void paint (juce::Graphics&) override;

    /** Feeds the meter one frame's worth of gain reduction, applying the 300 ms VU ballistics.
        Returns true if the needle moved enough to be worth repainting. */
    bool updateBallistics (float targetGainReductionDb, float deltaSeconds);

    float getDisplayedGainReductionDb() const noexcept { return displayed; }

private:
    juce::Image face, needle;
    float displayed = 0.0f;
    float velocity = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainReductionMeter)
};
