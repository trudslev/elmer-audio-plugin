#pragma once

#include "ElmerTheme.h"

/**
    A knob drawn from a 128-frame vertical bitmap filmstrip.

    Subclasses juce::Slider purely for its drag-to-value mapping and SliderAttachment compatibility;
    paint() fully replaces the look, so LookAndFeel::drawRotarySlider is never invoked. Same pattern
    as Gatecrasher's KnobFilmstripComponent.

    Filmstrips rather than code-drawn gradients on purpose: the specular highlight travelling across
    a coloured plastic cap as it turns is the entire point of the three-colour grouping, and a
    code-drawn gradient flattens it. Nothing rotates at runtime - the rotation is baked into the
    strip, frame by frame.

    The tick ring and the printed legends are NOT drawn here. They do not rotate, so they live in
    the baked background where they belong and cost nothing per frame.
*/
class KnobFilmstrip final : public juce::Slider
{
public:
    KnobFilmstrip (ElmerTheme::Layout::Strip strip, float diameterPx);

    void paint (juce::Graphics&) override;

    /** Centres the component on a point in design coordinates. */
    void setCentrePosition (juce::Point<float> centre);

private:
    const juce::Image& stripImage() const;

    ElmerTheme::Layout::Strip whichStrip;
    float diameter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KnobFilmstrip)
};
