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
    setRotaryParameters (juce::degreesToRadians (180.0f - Layout::knobSweepDegrees * 0.5f),
                         juce::degreesToRadians (180.0f + Layout::knobSweepDegrees * 0.5f),
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
    const auto& strip = stripImage();

    if (! strip.isValid())
        return;

    const double proportion = valueToProportionOfLength (getValue());
    const int frame = juce::jlimit (0, Layout::filmstripFrames - 1,
                                    juce::roundToInt (proportion * (Layout::filmstripFrames - 1)));

    const int frameSize = Layout::filmstripFrameSize;

    g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
    g.drawImage (strip,
                 0, 0, getWidth(), getHeight(),                 // destination
                 0, frame * frameSize, frameSize, frameSize);   // source frame
}
