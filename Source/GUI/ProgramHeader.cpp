#include "ProgramHeader.h"

using namespace ElmerTheme;

ProgramHeader::ProgramHeader (juce::AudioProcessorValueTreeState& s, ProgramManager& p)
    : apvts (s), programs (p)
{
    setBounds (0, 0, (int) Layout::canvasWidth, (int) Layout::canvasHeight);
    setInterceptsMouseClicks (true, false);
    setWantsKeyboardFocus (true);       // the display becomes a text field during naming
}

juce::Rectangle<float> ProgramHeader::saveBounds() const
{
    return { Layout::saveX, Layout::lcdRowY, Layout::headerButtonW, Layout::lcdRowH };
}

juce::Rectangle<float> ProgramHeader::deleteBounds() const
{
    return { Layout::deleteX, Layout::lcdRowY, Layout::headerButtonW, Layout::lcdRowH };
}

juce::Rectangle<float> ProgramHeader::displayBounds() const
{
    return { Layout::programX, Layout::lcdRowY, Layout::programW, Layout::lcdRowH };
}

juce::Rectangle<float> ProgramHeader::glassBounds() const
{
    return displayBounds().reduced (Layout::lcdFrameThickness);
}

/** The three cells, left to right, with a 1px hairline in each join. They are stated as explicit
    widths rather than as fractions of the glass because the NAME cell's width is what the character
    budget is computed from - 269 less 2 x 11px padding is 247px of text, 24 characters at 10.1px.
    A proportional split would let a change in the glass width silently change how many characters
    fit, and the cap on typed names would no longer match. */
juce::Rectangle<float> ProgramHeader::bankCellBounds() const
{
    return glassBounds().withWidth (Layout::lcdBankCellW);
}

juce::Rectangle<float> ProgramHeader::nameCellBounds() const
{
    return glassBounds().withTrimmedLeft (Layout::lcdBankCellW + Layout::lcdCellHairline)
                        .withWidth (Layout::lcdNameCellW);
}

juce::Rectangle<float> ProgramHeader::chevronCellBounds() const
{
    auto d = glassBounds();
    return d.removeFromRight (Layout::lcdChevronCellW);
}

bool hitsButton (juce::Rectangle<float> r, juce::Point<float> p) { return r.contains (p); }

void ProgramHeader::mouseDown (const juce::MouseEvent& e)
{
    const auto p = e.position;

    if (namingMode)
    {
        // The two buttons already in the row do confirm and cancel - no new controls appear and the
        // row's geometry does not move. The display is a field, not a menu trigger, so a click on
        // the glass does nothing rather than opening the list over a half-typed name.
        if (hitsButton (saveBounds(), p))        commitNaming();
        else if (hitsButton (deleteBounds(), p)) cancelNaming();
        return;
    }

    // Clicking anywhere in the display opens the list - the name cell and the chevron cell alike,
    // per the spec. The chevron is an affordance marking the display as a selector, not a button of
    // its own, so treating it as a separate target would make the larger area feel dead.
    if (hitsButton (displayBounds(), p))
    {
        showProgramMenu();
        return;
    }

    if (hitsButton (saveBounds(), p) && saveEnabled())
    {
        // SAVE never overwrites, so it must ask for a name rather than storing one silently. It
        // used to store immediately under the CURRENT Program's name, which made pressing it twice
        // produce two Programs indistinguishable in the list.
        enterNamingMode();
        return;
    }

    if (hitsButton (deleteBounds(), p) && deleteEnabled())
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

bool ProgramHeader::saveEnabled() const
{
    // Always live while naming, as STORE. Otherwise gated on the Program being dirty: with nothing
    // moved there is nothing to store, and the spec requires this to agree with the display's
    // asterisk always - so both read ProgramManager::isModified rather than deciding separately.
    return namingMode || programs.isModified();
}

bool ProgramHeader::deleteEnabled() const
{
    // Two states only. Enabled for a User Program, or as CANCEL while naming. Disabled for every
    // Factory Program AND for INIT - INIT is not a stored thing, so there is nothing to delete.
    const int index = programs.getCurrentProgram();
    return namingMode || (! ProgramManager::isInit (index) && ! programs.isFactory (index));
}

void ProgramHeader::enterNamingMode()
{
    namingMode = true;
    typedName.clear();
    grabKeyboardFocus();
    caret.start();
    repaint();
}

void ProgramHeader::commitNaming()
{
    // Empty falls back to UNTITLED INSIDE the manager rather than here, so no future caller can
    // write a nameless file.
    //
    // saveNewUserProgram selects the new Program itself and re-takes the clean snapshot; it is
    // deliberately NOT followed by setCurrentProgram, which would asynchronously re-apply values
    // that are already live and briefly hand the parameters back to the audio thread for nothing.
    const auto name = typedName;

    namingMode = false;
    typedName.clear();
    caret.stop();

    programs.saveNewUserProgram (name);
    repaint();
}

void ProgramHeader::cancelNaming()
{
    // A mode exit and nothing else. **It must never touch a parameter** - whatever the user had
    // tweaked before pressing SAVE has to survive cancelling, and the displayed Program was never
    // written to while naming, so leaving the mode is all the revert that is needed.
    namingMode = false;
    typedName.clear();
    caret.stop();
    repaint();
}

bool ProgramHeader::keyPressed (const juce::KeyPress& key)
{
    if (! namingMode)
        return false;

    if (key == juce::KeyPress::returnKey)  { commitNaming(); return true; }
    if (key == juce::KeyPress::escapeKey)  { cancelNaming(); return true; }

    if (key == juce::KeyPress::backspaceKey)
    {
        typedName = typedName.dropLastCharacters (1);
        repaint();
        return true;
    }

    const auto c = key.getTextCharacter();

    // Forced uppercase, and hard-capped at 19 so what CAN be typed matches what can be displayed
    // once the index and the dirty asterisk are added - see Layout::maxUserNameLength.
    if (c >= 32 && c != 127 && typedName.length() < Layout::maxUserNameLength)
    {
        typedName += juce::String::charToString (c).toUpperCase();
        repaint();
        return true;
    }

    return true;    // swallow everything else while naming; the field owns the keyboard
}

void ProgramHeader::focusLost (FocusChangeType)
{
    if (namingMode)
        cancelNaming();
}

void ProgramHeader::paintChevron (juce::Graphics& g) const
{
    const auto cell = chevronCellBounds();
    const float left = cell.getCentreX() - Layout::chevronW * 0.5f;
    const float top  = cell.getCentreY() - Layout::chevronH * 0.5f;

    // Mirrored about the mark's own centre line rather than rotated, so the apex stays on one
    // vertical axis and it reads as flipping in place instead of sliding sideways.
    const float outerY = menuOpen ? top + Layout::chevronH * 0.6f : top + Layout::chevronH * 0.2f;
    const float apexY  = menuOpen ? top + Layout::chevronH * 0.2f : top + Layout::chevronH * 0.8f;

    juce::Path chevron;
    chevron.startNewSubPath (left, outerY);
    chevron.lineTo (left + Layout::chevronW * 0.5f, apexY);
    chevron.lineTo (left + Layout::chevronW, outerY);

    g.setColour (Colour::phosphor);
    g.strokePath (chevron, { Layout::chevronStroke, juce::PathStrokeType::curved,
                             juce::PathStrokeType::rounded });
}

void ProgramHeader::showProgramMenu()
{
    juce::PopupMenu menu;
    menu.setLookAndFeel (&getLookAndFeel());

    const int current = programs.getCurrentProgram();
    const int factoryCount = programs.getNumFactoryPrograms();
    const int total = programs.getNumPrograms();

    // INIT first, unnumbered and outside both banks, with a divider beneath it. Its item ID is not
    // index+1 like the rest - that would be 0, which PopupMenu reserves for "dismissed" - so it
    // carries its own sentinel and is translated back on selection.
    constexpr int initMenuId = 9999;
    menu.addItem (initMenuId, "INIT", true, ProgramManager::isInit (current));
    menu.addSeparator();

    menu.addSectionHeader ("FACTORY");

    for (int i = 0; i < factoryCount; ++i)
        menu.addItem (i + 1, juce::String (i + 1).paddedLeft ('0', 2) + " " + programs.getProgramName (i),
                      true, i == current);

    // The USER section is absent entirely when empty - header and divider included - rather than
    // showing an empty heading. The spec is explicit, and it differs from Reflect-84, which keeps
    // the header and prints a placeholder row.
    if (total > factoryCount)
    {
        menu.addSeparator();
        menu.addSectionHeader ("USER");

        for (int i = factoryCount; i < total; ++i)
            menu.addItem (i + 1, juce::String (i + 1).paddedLeft ('0', 2) + " " + programs.getProgramName (i),
                          true, i == current);
    }

    const auto glass = displayBounds().reduced (Layout::lcdFrameThickness).getSmallestIntegerContainer();

    auto options = juce::PopupMenu::Options()
                       .withTargetComponent (this)
                       .withMaximumNumColumns (1);

    if (menuParent != nullptr)
    {
        // A 1px anchor on the glass's lower edge, NOT the display rect: with a parent component JUCE
        // first does constrainedWithin(parentArea), which would slide the whole display down into
        // the host and open the list a display-height too low. 1px and not zero, because a
        // zero-height rectangle is isEmpty() and that drops the list into the sideways placement
        // meant for submenus. See ../../CLAUDE.md, "The Program dropdown".
        const juce::Rectangle<int> anchor { glass.getX(), menuAnchorY() - 1, glass.getWidth(), 1 };

        options = options.withTargetScreenArea (localAreaToGlobal (anchor))
                         .withParentComponent (menuParent)
                         .withMinimumWidth (glass.getWidth());
    }
    else
    {
        options = options.withTargetScreenArea (localAreaToGlobal (glass))
                         .withMinimumWidth (glass.getWidth());
    }

    menuOpen = true;
    repaint();

    menu.showMenuAsync (options,
                        [safeThis = juce::Component::SafePointer<ProgramHeader> (this)] (int result)
                        {
                            if (safeThis == nullptr)
                                return;

                            // Cleared here rather than on selection: JUCE runs this on a dismissal
                            // too, so clicking away cannot leave the chevron stuck inverted.
                            safeThis->menuOpen = false;
                            safeThis->repaint();

                            if (result == 0)
                                return;

                            safeThis->programs.setCurrentProgram (result == initMenuId
                                                                      ? Elmer::initProgramIndex
                                                                      : result - 1);
                        });
}

juce::String ProgramHeader::currentLcdText() const
{
    if (editingParam.isNotEmpty())
    {
        const auto described = describeParameter (editingParam);

        if (described.isNotEmpty())
            return described;
    }

    const int index = programs.getCurrentProgram();

    // The asterisk and SAVE's enablement read the SAME predicate, so the panel cannot show a lit
    // SAVE over an unmarked name or the reverse. Budget: 3 ("01 ") + 19 (name) + 2 (" *") = 24, the
    // lcdCharacterBudget the name cell is sized for - see Layout::maxUserNameLength.
    return programs.getDisplayName (index) + (programs.isModified() ? " *" : "");
}

void ProgramHeader::paint (juce::Graphics& g)
{
    const int index = programs.getCurrentProgram();

    // --- PROGRAM: one continuous glass, with a bank field separated only by a hairline ---------
    const juce::Rectangle<float> frame { Layout::programX, Layout::lcdRowY,
                                         Layout::programW, Layout::lcdRowH };

    g.setGradientFill (Paint::vertical (frame, Colour::lcdFrameTop, Colour::lcdFrameBottom));
    g.fillRoundedRectangle (frame, Layout::lcdFrameRadius);

    const auto glass = glassBounds();
    g.setGradientFill (Paint::vertical (glass, Colour::lcdGlassTop, Colour::lcdGlassBottom));
    g.fillRoundedRectangle (glass, Layout::lcdGlassRadius);

    g.setColour (juce::Colours::white.withAlpha (0.35f));
    g.drawLine (frame.getX() + 2.0f, frame.getY() + 0.5f, frame.getRight() - 2.0f, frame.getY() + 0.5f, 1.0f);

    const auto bankArea = bankCellBounds();
    const auto nameCell = nameCellBounds();
    const auto nameArea = nameCell.reduced (Layout::lcdNamePadX, 0.0f);
    const auto lcdFont = Font::mono (Layout::lcdTextSize);

    // ONE field that switches its text. Never two labels with one greyed out.
    //
    // On INIT it reads an em-dash at 42% phosphor with no glow: INIT sits outside both banks, so
    // printing FACT or USER there would name it twice and name it wrongly.
    const bool onInit = ProgramManager::isInit (index);

    const auto bankText = namingMode ? juce::String ("NAME")
                                     : (onInit ? Text::emDash()
                                               : juce::String (programs.isFactory (index) ? "FACT" : "USER"));

    Text::drawTracked (g, bankText, lcdFont, Layout::lcdTextTracking, bankArea,
                       juce::Justification::centred,
                       (onInit && ! namingMode) ? Colour::phosphor.withAlpha (0.42f) : Colour::phosphor,
                       false);

    g.setColour (Colour::lcdHairline);
    g.fillRect (bankArea.getRight(), glass.getY() + 1.0f, Layout::lcdCellHairline, glass.getHeight() - 2.0f);
    g.fillRect (nameCell.getRight(), glass.getY() + 1.0f, Layout::lcdCellHairline, glass.getHeight() - 2.0f);

    if (namingMode)
    {
        // CENTRED, not left-aligned like the program name. The field appears in place of a name that
        // is itself centred in this cell, so left-aligning it would make the text visibly jump
        // sideways at the moment SAVE is pressed - a flinch that reads as a bug.
        //
        // The cursor's cell is in the string on BOTH phases - a space when dark - so the centring is
        // computed over a constant width. Appending the block only when lit, which is what the
        // left-aligned castings do safely, would walk the whole name half a character sideways at
        // every blink. A space is the right stand-in because this face is monospaced, so the two
        // advance identically.
        Text::drawTracked (g, typedName + (caret.visible ? Text::blockCursor() : juce::String (" ")),
                           lcdFont, Layout::lcdTextTracking, nameArea,
                           juce::Justification::centred, Colour::phosphor, false);
    }
    else
    {
        // CENTRED, matching the naming field. Names run 11 to 22 characters; left-aligning left a
        // ragged gap before the chevron, and it is also why the dirty asterisk shifts the text half
        // a character rather than appearing in dead space.
        Text::drawTracked (g, currentLcdText(), lcdFont, Layout::lcdTextTracking, nameArea,
                           juce::Justification::centred, Colour::phosphor, false);
    }

    // The chevron is hidden while naming: the display is a text field then, not a menu trigger, and
    // a mark saying "there is a list here" over a half-typed name invites a click that is refused.
    if (! namingMode)
        paintChevron (g);

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

    drawButton (saveBounds(), namingMode ? "STORE" : "SAVE", saveEnabled());
    drawButton (deleteBounds(), namingMode ? "CANCEL" : "DELETE", deleteEnabled());

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
