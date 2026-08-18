#include "KnobFilmstrip.h"

using namespace ElmerTheme;

namespace
{
    const juce::Image& cachedStrip (ElmerTheme::Layout::Strip s)
    {
        static const juce::Image detect = juce::ImageCache::getFromMemory (
            BinaryData::knobdetect128_png, BinaryData::knobdetect128_pngSize);
        static const juce::Image timing = juce::ImageCache::getFromMemory (
            BinaryData::knobtiming128_png, BinaryData::knobtiming128_pngSize);
        static const juce::Image output = juce::ImageCache::getFromMemory (
            BinaryData::knoboutput128_png, BinaryData::knoboutput128_pngSize);

        switch (s)
        {
            case ElmerTheme::Layout::Strip::timing: return timing;
            case ElmerTheme::Layout::Strip::output: return output;
            case ElmerTheme::Layout::Strip::detect:
            default:                                return detect;
        }
    }
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

        **Nothing on the panel showed it**, which is the other half. `paint()` is fully overridden and
        draws a baked filmstrip frame chosen by `valueToProportionOfLength`, so the Slider's own
        rotary parameters reach no pixel here. They reach an accessibility client, a look-and-feel,
        and any JUCE default paint path someone later reinstates — all of which would have been
        handed an arc pointing at the floor.

        `printedScaleDefects` cannot catch it either: it checks a ring's marks against the
        parameter's range, and this is neither a mark nor a range. */
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

const juce::Image& KnobFilmstrip::stripImage() const
{
    return cachedStrip (whichStrip);
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
