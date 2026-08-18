#include "GainReductionMeter.h"

#include <cmath>

using namespace ElmerTheme;

GainReductionMeter::GainReductionMeter()
{
    setInterceptsMouseClicks (false, false);
    face   = juce::ImageCache::getFromMemory (BinaryData::meterface_png, BinaryData::meterface_pngSize);
    needle = juce::ImageCache::getFromMemory (BinaryData::meterneedle_png, BinaryData::meterneedle_pngSize);
}

bool GainReductionMeter::updateBallistics (float targetGainReductionDb, float deltaSeconds)
{
    const float target = juce::jlimit (0.0f, Layout::meterFullScaleDb, targetGainReductionDb);
    const float previous = displayed;

    // A real moving coil has mass: it accelerates toward the target and overshoots slightly rather
    // than easing exponentially into it. The damping term is what stops it oscillating - drop it
    // and the needle rings like a bell on every transient.
    const float attackCoeff  = 1.0f - std::exp (-deltaSeconds * 1000.0f / Layout::meterAttackMs);
    const float releaseCoeff = 1.0f - std::exp (-deltaSeconds / Layout::meterReleaseSeconds);
    const float coeff = target > displayed ? attackCoeff : releaseCoeff;

    velocity += (target - displayed) * coeff;
    velocity *= Layout::meterDamping;
    displayed += velocity;
    displayed = juce::jlimit (0.0f, Layout::meterFullScaleDb, displayed);

    return std::abs (displayed - previous) > 0.002f;
}

void GainReductionMeter::paint (juce::Graphics& g)
{
    const juce::Rectangle<float> body { 0.0f, 0.0f, Layout::meterW, Layout::meterH };

    juce::Graphics::ScopedSaveState state { g };
    {
        juce::Path clip;
        clip.addRoundedRectangle (body, Layout::meterRadius);
        g.reduceClipRegion (clip);
    }

    g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);

    if (face.isValid())
        g.drawImage (face, body, juce::RectanglePlacement::stretchToFit, false);

    if (needle.isValid())
    {
        const float angle = Layout::meterAngleAtZero
                          - (displayed / Layout::meterFullScaleDb) * Layout::meterAngleSpan;

        // Written as an explicit four-step chain rather than scale().translated().rotated(): move
        // the sprite's own pivot to the origin, scale, rotate about the origin, then place it on
        // the meter's pivot. Composition order on the compact form is easy to get subtly wrong and
        // the failure looks like a needle on the right axis at the wrong place along it.
        const auto transform =
            juce::AffineTransform::translation (-Layout::needleSourcePivot.x,
                                                -Layout::needleSourcePivot.y)
                .scaled (Layout::meterScale)
                .rotated (juce::degreesToRadians (angle))
                .translated (Layout::meterPivot.x, Layout::meterPivot.y);

        g.drawImageTransformed (needle, transform, false);
    }

    /*  The glass sheen, built as the prototype's `linear-gradient(118deg, …)` rather than
        approximated — see `Layout::meterSheenAngleDeg`. A CSS linear-gradient is centred on its box
        and its line runs |w sin A| + |h cos A|, so both ends sit outside the box; computing it that
        way is what makes the band land along the upper-left instead of washing the whole face. */
    {
        const float a = juce::degreesToRadians (Layout::meterSheenAngleDeg);
        const juce::Point<float> dir { std::sin (a), -std::cos (a) };
        const float lineLength = std::abs (body.getWidth() * dir.x)
                               + std::abs (body.getHeight() * dir.y);
        const auto centre = body.getCentre();
        const auto start = centre - dir * (lineLength * 0.5f);
        const auto end   = centre + dir * (lineLength * 0.5f);

        const auto lit = juce::Colours::white.withAlpha (Layout::meterSheenAlpha);

        juce::ColourGradient sheen { lit, start, juce::Colours::transparentWhite, end, false };
        sheen.addColour (Layout::meterSheenHoldStop, lit);
        sheen.addColour (Layout::meterSheenOutStop, juce::Colours::transparentWhite);
        g.setGradientFill (sheen);
        g.fillRect (body);
    }
}
