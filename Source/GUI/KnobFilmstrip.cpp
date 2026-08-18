#include "KnobFilmstrip.h"

using namespace ElmerTheme;

namespace
{
}

KnobFilmstrip::KnobFilmstrip (Layout::Strip strip, float diameterPx)
    : juce::Slider (juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox),
      whichStrip (strip), diameter (diameterPx)
{
    setMouseDragSensitivity (Layout::knobDragPixels);
    setVelocityBasedMode (false);
    setMouseCursor (juce::MouseCursor::UpDownResizeCursor);

    /*  **The sweep is stated, not inherited.** Elmer's filmstrips are cut at 280 degrees
        (-140..+140), 10 wider than the suite's 270, and that is a deliberate per-casting freedom
        BRAND.md now records rather than an accident. But nothing was telling JUCE: with paint()
        fully overridden and RotaryVerticalDrag in use, the default arc never showed, so the
        divergence sat invisible in the artwork with no code stating it. Anything that later reads
        the Slider's own rotary parameters - a look-and-feel, an accessibility client, a JUCE
        default paint path someone reinstates - would have got 270 and been quietly wrong. */
    /*  **Centred on TWELVE o'clock, and it used to be centred on six.**

        JUCE measures rotary angles clockwise from 12 o'clock, so an arc symmetric about the pointer's
        rest axis is `2*pi +/- sweep/2` — the `360` below. The old form was `180 +/- sweep/2`, which
        is symmetric about 6 o'clock: the same SPAN, pointing the opposite way. That is why it
        survived, and why a test asserting the span would have passed it.

        **Nothing on the panel showed it**, which is the other half. `paint()` is fully overridden,
        so the Slider's own rotary parameters were unread — and are still unread by the drawing,
        which computes its angle from `knobSweepStartDeg`. They are set because a future reader who
        reinstates any default paint path would otherwise get JUCE's 270 and be quietly wrong.
    */
    setRotaryParameters (juce::degreesToRadians (360.0f - Layout::knobSweepDegrees * 0.5f),
                         juce::degreesToRadians (360.0f + Layout::knobSweepDegrees * 0.5f),
                         true);
}

void KnobFilmstrip::mouseDown (const juce::MouseEvent& e)
{
    // Sensitivity has to be settled BEFORE Slider::mouseDown records its drag anchor: JUCE measures
    // the drag from that anchor and scales by the current sensitivity, so changing it part-way
    // through a drag rescales the distance already travelled and the value jumps.
    setMouseDragSensitivity (e.mods.isShiftDown() ? Layout::knobFineDragPixels
                                                  : Layout::knobDragPixels);

    juce::Slider::mouseDown (e);
}


void KnobFilmstrip::setCentrePosition (juce::Point<float> centre)
{
    const float half = diameter * 0.5f;
    setBounds (juce::Rectangle<float> (centre.x - half, centre.y - half, diameter, diameter)
                   .getSmallestIntegerContainer());
}

void KnobFilmstrip::paint (juce::Graphics& g)
{
    /*  **CODE-DRAWN AND CACHED — §3.1 retires the 128-frame filmstrips.**

        **The cache is keyed on the DEVICE SCALE, not the component size.** A Retina display and a
        scaled editor both change how many physical pixels the knob covers while its logical bounds
        stay put, so a cache keyed on `getWidth()` serves a layer drawn for the wrong resolution and
        looks merely soft — which is the failure mode that survives review.

        **`setBufferedToImage` on the whole component is the trap this avoids.** It compiles,
        profiles identically, and re-renders on every repaint — and a knob repaints on every frame
        of a drag, so it is a cache that caches nothing while looking like one. `staticLayerBuilds`
        exists so a test can tell the two apart, and `KnobRenderTests` asserts it in BOTH
        directions: a counter that never increments passes a one-directional arm exactly as a
        working cache does. */
    const auto bounds = getLocalBounds().toFloat();

    const float deviceScale = g.getInternalContext().getPhysicalPixelScaleFactor();
    const int wantedW = juce::roundToInt (bounds.getWidth() * deviceScale);
    const int wantedH = juce::roundToInt (bounds.getHeight() * deviceScale);

    if (staticLayer.isNull() || staticLayer.getWidth() != wantedW
        || staticLayer.getHeight() != wantedH)
    {
        staticLayer = juce::Image (juce::Image::ARGB, juce::jmax (1, wantedW),
                                    juce::jmax (1, wantedH), true);
        ++staticLayerBuilds;

        juce::Graphics ig { staticLayer };
        ig.addTransform (juce::AffineTransform::scale (deviceScale));
        ElmerTheme::Layout::paintKnobStatic (ig, bounds.withZeroOrigin(), whichStrip);
    }

    g.drawImageTransformed (staticLayer, juce::AffineTransform::scale (1.0f / deviceScale));

    /*  The pointer, live. Its proportion comes from the Slider's own `valueToProportionOfLength`,
        which carries the parameter's taper — so it lands on the printed marks, which are placed
        from that same law. */
    ElmerTheme::Layout::paintKnobPointer (g, bounds.getCentre(), diameter,
                                          (float) valueToProportionOfLength (getValue()));
}
