#include "ProgramHeader.h"

using namespace ElmerTheme;

ProgramHeader::ProgramHeader (juce::AudioProcessorValueTreeState& s, ProgramManager& p)
    : apvts (s), programs (p)
{
    setBounds (0, 0, (int) Layout::canvasWidth, (int) Layout::canvasHeight);
    setInterceptsMouseClicks (true, false);
}

juce::Rectangle<float> ProgramHeader::saveBounds() const
{
    return { Layout::saveX, Layout::lcdRowY, Layout::headerButtonW, Layout::lcdRowH };
}

juce::Rectangle<float> ProgramHeader::deleteBounds() const
{
    return { Layout::deleteX, Layout::lcdRowY, Layout::headerButtonW, Layout::lcdRowH };
}

bool hitsButton (juce::Rectangle<float> r, juce::Point<float> p) { return r.contains (p); }

void ProgramHeader::mouseDown (const juce::MouseEvent& e)
{
    const auto p = e.position;

    if (hitsButton (saveBounds(), p))
    {
        // SAVE always creates a new Program and never overwrites, so there is no "New" action.
        programs.saveNewUserProgram (programs.getProgramName (programs.getCurrentProgram()));
        repaint();
        return;
    }

    if (hitsButton (deleteBounds(), p) && ! programs.isFactory (programs.getCurrentProgram()))
    {
        programs.deleteUserProgram (programs.getCurrentProgram());
        repaint();
    }
}

void ProgramHeader::showParameter (const juce::String& paramId)
{
    stopTimer();
    editingParam = paramId;
    repaint();
}

void ProgramHeader::releaseParameter()
{
    startTimer (Layout::lcdRevertMs);
}

void ProgramHeader::timerCallback()
{
    stopTimer();
    editingParam = {};
    repaint();
}

void ProgramHeader::setLevels (float inDb, float outDb)
{
    if (std::abs (inDb - inLevelDb) < 0.05f && std::abs (outDb - outLevelDb) < 0.05f)
        return;

    inLevelDb = inDb;
    outLevelDb = outDb;
    repaint();
}

void ProgramHeader::setGainReductionDb (float db)
{
    if (std::abs (db - grDb) < 0.05f)
        return;

    grDb = db;
    repaint();
}

juce::String ProgramHeader::describeParameter (const juce::String& paramId) const
{
    auto* p = apvts.getParameter (paramId);

    if (p == nullptr)
        return {};

    auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (p);
    const float value = ranged != nullptr ? ranged->convertFrom0to1 (p->getValue()) : 0.0f;

    if (paramId == ParamIDs::threshold) return "THRESHOLD " + juce::String (value, 1) + " dB";
    if (paramId == ParamIDs::makeup)    return "MAKEUP +" + juce::String (value, 1) + " dB";
    if (paramId == ParamIDs::iron)      return "IRON " + juce::String (juce::roundToInt (value)) + " %";
    if (paramId == ParamIDs::mix)       return "MIX " + juce::String (juce::roundToInt (value)) + " %";

    if (paramId == ParamIDs::attack)
    {
        const int places = value < 1.0f ? 2 : (value < 10.0f ? 1 : 0);
        return "ATTACK " + juce::String (value, places) + " ms";
    }

    if (paramId == ParamIDs::sidechainHp)
    {
        if (Elmer::Law::hpIsOff (value))
            return "SIDECHAIN HP OFF";

        const float hz = Elmer::Law::hpFrequencyHz (value);
        const int shown = hz < 100.0f ? juce::roundToInt (hz)
                                      : juce::roundToInt (hz / 5.0f) * 5;
        return "SIDECHAIN HP " + juce::String (shown) + " Hz";
    }

    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (p))
    {
        const juce::String name = paramId == ParamIDs::ratio ? "RATIO"
                                : paramId == ParamIDs::knee  ? "KNEE" : "RELEASE";
        // **The value is NOT upper-cased.** The names above are already uppercase literals, so the
        // transform only ever reached the value - and Release's choices carry a unit, so "0.1 s"
        // rendered as "RELEASE 0.1 S". A capital S is a different unit from a lowercase one.
        //
        // Ratio's strings are digits and colons, so the transform was invisible there; Knee's now
        // read as authored ("Soft"), which is the point. If the display should say SOFT, author the
        // choice that way in Parameters.h so the host's automation lane agrees - do not re-case it
        // here, or the two disagree again by exactly this route.
        return name + " " + choice->getCurrentChoiceName();
    }

    return {};
}

juce::String ProgramHeader::currentLcdText() const
{
    if (editingParam.isNotEmpty())
    {
        const auto described = describeParameter (editingParam);

        if (described.isNotEmpty())
            return described;
    }

    return programs.getDisplayName (programs.getCurrentProgram());
}

void ProgramHeader::drawLcdPanel (juce::Graphics& g, juce::Rectangle<float> frame,
                                  const juce::String& text, juce::Justification just,
                                  float textSize)
{
    g.setGradientFill (Paint::vertical (frame, Colour::lcdFrameTop, Colour::lcdFrameBottom));
    g.fillRoundedRectangle (frame, Layout::lcdFrameRadius);

    const auto glass = frame.reduced (Layout::lcdFramePad);
    g.setGradientFill (Paint::vertical (glass, Colour::lcdGlassTop, Colour::lcdGlassBottom));
    g.fillRoundedRectangle (glass, Layout::lcdGlassRadius);

    g.setColour (juce::Colours::white.withAlpha (0.35f));
    g.drawLine (frame.getX() + 2.0f, frame.getY() + 0.5f, frame.getRight() - 2.0f, frame.getY() + 0.5f, 1.0f);

    if (text.isNotEmpty())
    {
        const auto font = Font::mono (textSize);
        const auto area = glass.reduced (Layout::lcdNamePadX, 0.0f);

        // The phosphor bloom, drawn as two soft passes under the glyphs.
        for (auto [dx, alpha] : { std::pair { 1.4f, 0.16f }, std::pair { 0.7f, 0.26f } })
        {
            juce::ignoreUnused (dx);
            Text::drawTracked (g, text, font, Layout::lcdTextTracking, area, just,
                               Colour::phosphor.withAlpha (alpha), false);
        }

        Text::drawTracked (g, text, font, Layout::lcdTextTracking, area, just, Colour::phosphor, false);
    }
}

void ProgramHeader::paint (juce::Graphics& g)
{
    const int index = programs.getCurrentProgram();

    // --- PROGRAM: one continuous glass, with a bank field separated only by a hairline ---------
    const juce::Rectangle<float> frame { Layout::programX, Layout::lcdRowY,
                                         Layout::programW, Layout::lcdRowH };

    g.setGradientFill (Paint::vertical (frame, Colour::lcdFrameTop, Colour::lcdFrameBottom));
    g.fillRoundedRectangle (frame, Layout::lcdFrameRadius);

    const auto glass = frame.reduced (Layout::lcdFramePad);
    g.setGradientFill (Paint::vertical (glass, Colour::lcdGlassTop, Colour::lcdGlassBottom));
    g.fillRoundedRectangle (glass, Layout::lcdGlassRadius);

    g.setColour (juce::Colours::white.withAlpha (0.35f));
    g.drawLine (frame.getX() + 2.0f, frame.getY() + 0.5f, frame.getRight() - 2.0f, frame.getY() + 0.5f, 1.0f);

    const auto bankArea = glass.withWidth (Layout::bankFieldW);
    const auto nameArea = glass.withTrimmedLeft (Layout::bankFieldW).reduced (Layout::lcdNamePadX, 0.0f);
    const auto lcdFont = Font::mono (Layout::lcdTextSize);

    // ONE field that switches its text. Never two labels with one greyed out.
    Text::drawTracked (g, programs.isFactory (index) ? "FACT" : "USER", lcdFont,
                       Layout::lcdTextTracking, bankArea, juce::Justification::centred,
                       Colour::phosphor, false);

    g.setColour (Colour::lcdHairline);
    g.fillRect (bankArea.getRight(), glass.getY() + 1.0f, 1.0f, glass.getHeight() - 2.0f);

    Text::drawTracked (g, currentLcdText(), lcdFont, Layout::lcdTextTracking, nameArea,
                       juce::Justification::left, Colour::phosphor, false);

    // --- SAVE / DELETE -------------------------------------------------------------------------
    const auto drawButton = [&g] (juce::Rectangle<float> r, const juce::String& label, bool enabled)
    {
        g.setGradientFill (Paint::vertical (r, enabled ? Colour::creamTop : Colour::creamOffTop,
                                            enabled ? Colour::creamBottom : Colour::creamOffBottom));
        g.fillRoundedRectangle (r, Layout::lcdGlassRadius);

        g.setColour (juce::Colours::white.withAlpha (enabled ? 0.85f : 0.22f));
        g.drawLine (r.getX() + 1.5f, r.getY() + 0.5f, r.getRight() - 1.5f, r.getY() + 0.5f, 1.0f);

        Text::drawTracked (g, label, Font::mono (Layout::buttonTextSize), Layout::buttonTracking, r,
                           juce::Justification::centred,
                           enabled ? Colour::creamText : Colour::creamOffText, false);
    };

    drawButton (saveBounds(), "SAVE", true);
    drawButton (deleteBounds(), "DELETE", ! programs.isFactory (index));

    // --- IN / OUT ------------------------------------------------------------------------------
    const auto level = [&g, this] (float x, float db)
    {
        const juce::Rectangle<float> r { x, Layout::lcdRowY, Layout::levelBoxW, Layout::lcdRowH };

        g.setGradientFill (Paint::vertical (r, Colour::lcdGlassTop, Colour::lcdMeterBottom));
        g.fillRoundedRectangle (r, Layout::lcdFrameRadius);

        g.setColour (juce::Colours::white.withAlpha (0.32f));
        g.drawLine (r.getX() + 2.0f, r.getBottom() - 0.5f, r.getRight() - 2.0f, r.getBottom() - 0.5f, 1.0f);

        Text::drawTracked (g, juce::String (db, 1), Font::mono (Layout::levelTextSize), 0.0f, r,
                           juce::Justification::centred, Colour::phosphor, false);
    };

    level (Layout::meterInX, inLevelDb);
    level (Layout::meterOutX, outLevelDb);

    // The meter's live GR figure, right-aligned under the meter opposite MOVING COIL. It is a
    // readout, so it belongs here with the other live text rather than in the baked layer.
    Text::drawTracked (g, "GR -" + juce::String (grDb, 1) + " dB",
                       Font::monoBold (Layout::meterSpecSize), Layout::meterSpecTracking,
                       { Layout::meterX, Layout::meterY + Layout::meterH + Layout::meterSpecGap,
                         Layout::meterW, 14.0f },
                       juce::Justification::right, Colour::ink);
}
