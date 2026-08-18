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

    /** Shift gives a 4x fine drag. Suite-wide behaviour rather than this casting's: a player who
        learns Shift on one casting expects it on the next, which makes it behaviour and not
        appearance. */
    void mouseDown (const juce::MouseEvent&) override;

    /** Centres the component on a point in design coordinates. */
    void setCentrePosition (juce::Point<float> centre);

    /** **Test seam: how many times the static layer has been rebuilt.** A cache that silently
        rebuilds every frame is indistinguishable from one that works by looking at the panel —
        both draw the right knob, and the difference is only in what a drag costs. */
    int staticLayerBuildCount() const noexcept { return staticLayerBuilds; }

private:
    juce::Image staticLayer;
    int staticLayerBuilds = 0;

    /*  **RETIRED with the filmstrips and kept only until the last reader goes.** `paint` is fully
        code-drawn now, so nothing calls this; the two 128-frame sheets it reaches are dead weight in
        BinaryData. Removing the declaration, the definition, `cachedStrip` and the sheets themselves
        is one mechanical commit and is deliberately not folded into the drawing change — the same
        reason `KnobFilmstripComponent` kept its name in Chorus-60 for a round. */
    const juce::Image& stripImage() const;

    ElmerTheme::Layout::Strip whichStrip;
    float diameter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KnobFilmstrip)
};
