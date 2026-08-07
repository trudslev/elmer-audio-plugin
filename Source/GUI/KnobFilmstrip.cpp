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
