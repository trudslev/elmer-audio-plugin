#include "KneeSwitch.h"
#include "../Parameters.h"

using namespace ElmerTheme;

KneeSwitch::KneeSwitch (juce::AudioProcessorValueTreeState& s) : apvts (s)
{
    /*  One pixel taller than the shoe. `box-shadow: 0 1px 0 rgba(255,255,255,.28)` on the
        container is an OUTER shadow offset down with no blur, so it lands on the row BELOW the
        shoe's foot — outside a component sized to the shoe exactly, where it would instead
        lighten the foot itself and read as a bevel the part does not have. */
    setSize ((int) Layout::kneeShoeW, (int) Layout::kneeShoeH + 1);
}

void KneeSwitch::mouseDown (const juce::MouseEvent& e)
{
    // A shoe is a POSITION, not a toggle: pressing the half that is already live must not flip it.
    // The old lamp pair had the same property and it is easy to lose in a rewrite, because a single
    // two-state control invites `setValue (! current)`.
    const bool hard = e.position.x > Layout::kneeShoeW * 0.5f;

    if (auto* p = apvts.getParameter (ParamIDs::knee))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (hard ? 1.0f : 0.0f);
        p->endChangeGesture();
    }

    if (onInteraction != nullptr)
        onInteraction();

    repaint();
}

void KneeSwitch::paintHalf (juce::Graphics& g, juce::Rectangle<float> half, bool live) const
{
    /*  The two faces, from §3's own declarations:

            live: linear-gradient(180deg,#dcd6c6,#bdb6a4)
                  inset 0 1px 0 rgba(255,255,255,.55), inset 0 -2px 4px rgba(40,34,26,.18)
            idle: linear-gradient(180deg,#413b31,#2e2921)
                  inset 0 2px 5px rgba(0,0,0,.5)

        The halves sit inside a container with `overflow:hidden` and a 3 px radius, so their own
        corners are square and the container's rounded clip is what shapes the ends. Drawing each
        half rounded instead would put a notch in the middle of the shoe.  */
    g.setGradientFill (Paint::vertical (half,
                                        live ? Colour::shoeLiveTop    : Colour::shoeIdleTop,
                                        live ? Colour::shoeLiveBottom : Colour::shoeIdleBottom));
    g.fillRect (half);

    if (live)
    {
        // `inset 0 1px 0 rgba(255,255,255,.55)` — a 1 px lit edge along the top, no blur.
        g.setColour (juce::Colours::white.withAlpha (0.55f));
        g.fillRect (half.getX(), half.getY(), half.getWidth(), 1.0f);

        // `inset 0 -2px 4px rgba(40,34,26,.18)` — a soft seat at the foot.
        g.setGradientFill ({ juce::Colour (0xFF28221A).withAlpha (0.0f),
                             half.getX(), half.getBottom() - 6.0f,
                             juce::Colour (0xFF28221A).withAlpha (0.18f),
                             half.getX(), half.getBottom(), false });
        g.fillRect (half);
    }
    else
    {
        // `inset 0 2px 5px rgba(0,0,0,.5)` — the recess, strongest under the top edge.
        g.setGradientFill ({ juce::Colours::black.withAlpha (0.50f), half.getX(), half.getY(),
                             juce::Colours::black.withAlpha (0.0f),
                             half.getX(), half.getY() + 7.0f, false });
        g.fillRect (half);
    }
}

void KneeSwitch::paint (juce::Graphics& g)
{
    const bool hard = apvts.getRawParameterValue (ParamIDs::knee)->load() > 0.5f;

    const auto body = getLocalBounds().toFloat().withHeight (Layout::kneeShoeH);
    const float half = body.getWidth() * 0.5f;

    // The container's own OUTER shadow, `0 1px 0 rgba(255,255,255,.28)` — a 1 px lit line under the
    // foot, so the shoe reads as sitting proud of the fascia rather than cut into it.
    g.setColour (juce::Colours::white.withAlpha (0.28f));
    g.fillRect (body.getX(), body.getBottom(), body.getWidth(), 1.0f);

    // `overflow:hidden` on a `border-radius:3px` container: the halves are square and this is what
    // rounds the shoe's four corners.
    juce::Graphics::ScopedSaveState saved (g);
    juce::Path shell;
    shell.addRoundedRectangle (body, Layout::kneeShoeRadius);
    g.reduceClipRegion (shell);

    paintHalf (g, body.withWidth (half),               ! hard);
    paintHalf (g, body.withTrimmedLeft (half),           hard);

    /*  The container's own overlay, drawn over both halves so the seam between them runs under one
        continuous frame: `inset 0 0 0 1px #6d6759` then `inset 0 2px 6px rgba(40,34,26,.42)`.
        CSS paints the first-listed inset on top, so the recess goes down first. */
    {
        juce::ColourGradient recess { juce::Colour (0xFF28221A).withAlpha (0.42f),
                                      body.getX(), body.getY(),
                                      juce::Colour (0xFF28221A).withAlpha (0.0f),
                                      body.getX(), body.getY() + 8.0f, false };
        g.setGradientFill (recess);
        g.fillRect (body);
    }

    g.setColour (Colour::shoeRing);
    g.drawRoundedRectangle (body.reduced (0.5f), Layout::kneeShoeRadius, 1.0f);
}
