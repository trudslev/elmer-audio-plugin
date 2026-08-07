#include "KneeButtons.h"
#include "../Parameters.h"

using namespace ElmerTheme;

KneeButtons::KneeButtons (juce::AudioProcessorValueTreeState& s) : apvts (s)
{
    setSize ((int) Layout::lampW,
             (int) (Layout::lampH * 2.0f + Layout::lampGap));
}

void KneeButtons::mouseDown (const juce::MouseEvent& e)
{
    const bool hard = (float) e.position.y > Layout::lampH + Layout::lampGap * 0.5f;

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

void KneeButtons::drawLamp (juce::Graphics& g, juce::Rectangle<float> r,
                            const juce::String& legend, bool lit)
{
    // The face is identical whether or not this button is selected.
    g.setGradientFill (Paint::vertical (r, Colour::lampFaceTop, Colour::lampFaceBottom));
    g.fillRoundedRectangle (r, Layout::lampRadius);

    g.setColour (juce::Colours::white.withAlpha (0.30f));
    g.drawLine (r.getX() + 1.0f, r.getY() + 0.5f, r.getRight() - 1.0f, r.getY() + 0.5f, 1.0f);

    const auto font = Font::mono (Layout::lampLegendSize);
    const float textWidth = Text::trackedWidth (legend, font, Layout::lampLegendTracking);
    const float total = Layout::lampLedDiameter + Layout::lampContentGap + textWidth;
    const float startX = r.getCentreX() - total * 0.5f;

    const juce::Rectangle<float> led { startX, r.getCentreY() - Layout::lampLedDiameter * 0.5f,
                                       Layout::lampLedDiameter, Layout::lampLedDiameter };

    if (lit)
    {
        // Two-stage halo, sized to the lamp rather than taken literally from the CSS blur radii -
        // drawn as discs at the quoted radius they wash straight over the legend beside them.
        for (auto [radiusScale, alpha] : { std::pair { 3.4f, 0.16f }, std::pair { 1.9f, 0.30f } })
        {
            const float rr = Layout::lampLedDiameter * radiusScale;
            juce::ColourGradient halo { Colour::accent.withAlpha (alpha), led.getCentreX(), led.getCentreY(),
                                        Colour::accent.withAlpha (0.0f),
                                        led.getCentreX() + rr, led.getCentreY(), true };
            g.setGradientFill (halo);
            g.fillEllipse (led.getCentreX() - rr, led.getCentreY() - rr, rr * 2.0f, rr * 2.0f);
        }

        juce::ColourGradient core { Colour::ledOnCore, led.getX() + led.getWidth() * 0.34f,
                                    led.getY() + led.getHeight() * 0.30f,
                                    Colour::ledOnEdge, led.getRight(), led.getBottom(), true };
        core.addColour (0.46, Colour::accent);
        g.setGradientFill (core);
        g.fillEllipse (led);
    }
    else
    {
        juce::ColourGradient off { Colour::ledOffCore, led.getX() + led.getWidth() * 0.34f,
                                   led.getY() + led.getHeight() * 0.30f,
                                   Colour::ledOffEdge, led.getRight(), led.getBottom(), true };
        off.addColour (0.60, Colour::ledOffMid);
        g.setGradientFill (off);
        g.fillEllipse (led);
    }

    Text::drawTracked (g, legend, font, Layout::lampLegendTracking,
                       { startX + Layout::lampLedDiameter + Layout::lampContentGap,
                         r.getY(), textWidth + 4.0f, r.getHeight() },
                       juce::Justification::left,
                       lit ? Colour::lampLegendOn : Colour::lampLegendOff, ! lit);
}

void KneeButtons::paint (juce::Graphics& g)
{
    const bool hard = apvts.getRawParameterValue (ParamIDs::knee)->load() > 0.5f;

    drawLamp (g, { 0.0f, 0.0f, Layout::lampW, Layout::lampH }, "SOFT", ! hard);
    drawLamp (g, { 0.0f, Layout::lampH + Layout::lampGap, Layout::lampW, Layout::lampH }, "HARD", hard);
}
