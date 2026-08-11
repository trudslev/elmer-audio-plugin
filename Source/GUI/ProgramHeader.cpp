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
        programs.deleteUserProgram (programs.getCurrentProgramId());
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
    // Only a User Program can be deleted. INIT and an unresolved id are not stored things, and a
    // Factory Program is read-only. Always live while naming, as CANCEL.
    return namingMode || programs.getCurrentProgramId().bank == ProgramBank::user;
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
    // once the dirty asterisk is added - see Layout::maxUserNameLength. User names carry no index
    // any more, which is where the three extra characters came from.
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
    menu.setLookAndFeel (&menuLookAndFeel);

    const auto current = programs.getCurrentProgramId();

    // **Row IDs are positions in THIS menu, not Program indices.** PopupMenu needs an int per row
    // and reserves 0 for "dismissed"; the callback maps the row back to the ProgramId it was built
    // from, so no Program is addressed by a bank position here.
    menuRows = programs.listPrograms();

    bool factoryHeaderDone = false;
    bool userHeaderDone = false;

    for (size_t i = 0; i < menuRows.size(); ++i)
    {
        const auto& id = menuRows[i];

        // INIT first, unnumbered and outside both banks, with a divider beneath it.
        if (id.bank == ProgramBank::factory && ! std::exchange (factoryHeaderDone, true))
        {
            menu.addSeparator();
            menu.addSectionHeader ("FACTORY");
        }

        // The USER section is absent entirely when empty - header and divider included - rather
        // than showing an empty heading. This differs from Reflect-84, which keeps the header and
        // prints a placeholder row.
        if (id.bank == ProgramBank::user && ! std::exchange (userHeaderDone, true))
        {
            menu.addSeparator();
            menu.addSectionHeader ("USER");
        }

        menu.addItem ((int) i + 1, programs.displayLabelFor (id), true, id == current);
    }

    // The list takes the display's OUTER width - frame edges included - so the two share a left and
    // a right edge and read as one instrument. Measured 403..764 in design/screenshots/
    // panel-menu-open.png, which is the display's border box, not its 355px of glass. The prose's
    // "left: 0, width: 100%" would resolve against the padding box in CSS and give 355; the render
    // is the artefact, so it wins.
    const auto rail = displayBounds().getSmallestIntegerContainer();

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
        const juce::Rectangle<int> anchor { rail.getX(), menuAnchorY() - 1, rail.getWidth(), 1 };

        options = options.withTargetScreenArea (localAreaToGlobal (anchor))
                         .withParentComponent (menuParent)
                         .withMinimumWidth (rail.getWidth());
    }
    else
    {
        options = options.withTargetScreenArea (localAreaToGlobal (rail))
                         .withMinimumWidth (rail.getWidth());
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

                            const auto row = (size_t) (result - 1);

                            if (row < safeThis->menuRows.size())
                                safeThis->programs.requestProgramChange (safeThis->menuRows[row]);
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

    const auto id = programs.getCurrentProgramId();

    // An identifier the session named but the bank no longer has: the VALUES are correct and
    // untouched, only the name is unknown, so the panel says so rather than pretending. No dirty
    // asterisk either - there is no baseline to differ from.
    if (id.bank == ProgramBank::unresolved)
        return id.displayName + "?";

    // The asterisk and SAVE's enablement read the SAME predicate, so the panel cannot show a lit
    // SAVE over an unmarked name or the reverse. Only Factory Programs carry a number, computed
    // from their bank position at paint time.
    return programs.displayLabelFor (id) + (programs.isModified() ? " *" : "");
}

void ProgramHeader::paint (juce::Graphics& g)
{
    const auto currentId = programs.getCurrentProgramId();

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
    // **An em-dash where the Program is in neither bank** - INIT, or an unresolved identifier.
    const bool onInit = currentId.bank == ProgramBank::init
                         || currentId.bank == ProgramBank::unresolved;

    const auto bankText = namingMode ? juce::String ("NAME")
                                     : (onInit ? Text::emDash()
                                               : juce::String (currentId.bank == ProgramBank::user
                                                                    ? "USER" : "FACT"));

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

    // --- SAVE / STORE and DELETE / CANCEL ------------------------------------------------------
    /*  A split-legend annunciator cap: one body, two windows, each independently lamped. Nothing
        here branches on "enabled" - the body and both window grounds are identical in every state,
        including the state that used to be disabled. A dark legend is a function with nothing to
        do, not a control that has been switched off. */
    const auto drawLegendWindow = [&g] (juce::Rectangle<float> window, const juce::String& label,
                                        bool lit, bool roundTop)
    {
        // The ground is the same lit or unlit. Drawing it as a full rounded rect and then squaring
        // off the inner edge keeps the two windows sharing one continuous surface, so a halo can
        // cross the join.
        g.setGradientFill (Paint::vertical (window, Colour::windowTop, Colour::windowBottom));
        g.fillRoundedRectangle (window, Layout::windowRadius);
        g.fillRect (roundTop ? window.withTrimmedTop (Layout::windowRadius)
                             : window.withTrimmedBottom (Layout::windowRadius));

        const auto font = Font::mono (Layout::buttonTextSize);

        if (lit)
        {
            /*  The spec's three-layer text-shadow (3px / 7px / 13px, warm and widening). JUCE has
                no text-shadow and no cheap blur for a string, so each layer is the same tracked
                text drawn at eight points around a circle - overlapping copies sum to a halo,
                which is what a blur of that radius gives at 10px type.

                The colours warm as they widen, exactly as the spec's do: a white core over an
                amber spill is what an incandescent bulb behind a legend looks like. Alphas are
                tuned against the render, not lifted from it - eight overlapping copies at alpha a
                reach 1-(1-a)^8 where they coincide. */
            struct Halo { float radius, alpha; juce::Colour colour; };
            const Halo halo[] {
                { 6.5f, 0.030f, juce::Colour (0xFFFFC46E) },
                { 3.5f, 0.055f, juce::Colour (0xFFFFD696) },
                { 1.0f, 0.110f, Colour::legendLit }
            };

            for (const auto& layer : halo)
                for (int i = 0; i < 8; ++i)
                {
                    const float angle = juce::MathConstants<float>::twoPi * (float) i / 8.0f;

                    Text::drawTracked (g, label, font, Layout::buttonTracking,
                                       window.translated (std::cos (angle) * layer.radius,
                                                          std::sin (angle) * layer.radius),
                                       juce::Justification::centred,
                                       layer.colour.withAlpha (layer.alpha), false);
                }
        }

        Text::drawTracked (g, label, font, Layout::buttonTracking, window,
                           juce::Justification::centred,
                           lit ? Colour::legendLit : Colour::legendUnlit, false);
    };

    const auto drawButton = [&g, &drawLegendWindow] (juce::Rectangle<float> r,
                                                     const juce::String& topLabel,
                                                     const juce::String& bottomLabel,
                                                     bool topLit, bool bottomLit)
    {
        g.setGradientFill (Paint::vertical (r, Colour::capTop, Colour::capBottom));
        g.fillRoundedRectangle (r, Layout::lcdGlassRadius);

        // 0 1px 0 rgba(255,255,255,.34) - the machined top edge of the collar.
        g.setColour (juce::Colours::white.withAlpha (0.34f));
        g.drawLine (r.getX() + 1.5f, r.getY() + 0.5f, r.getRight() - 1.5f, r.getY() + 0.5f, 1.0f);

        auto lens = r.reduced (Layout::capPadding);
        const juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (lens.getSmallestIntegerContainer());

        drawLegendWindow (lens.removeFromTop (Layout::legendWindowH), topLabel, topLit, true);
        drawLegendWindow (lens.removeFromTop (Layout::legendWindowH), bottomLabel, bottomLit, false);
    };

    /*  GUI-SPEC's "which legend is live" table, in full:

        | Condition                        | SAVE | STORE | DELETE | CANCEL |
        | Factory or INIT, unmodified      | dark | dark  | dark   | dark   |
        | Factory or INIT, edited          | LIT  | dark  | dark   | dark   |
        | User Program, unmodified         | dark | dark  | LIT    | dark   |
        | User Program, edited             | LIT  | dark  | LIT    | dark   |
        | Naming                           | dark | LIT   | dark   | LIT    |

        saveEnabled() and deleteEnabled() already report both regions live while naming, so the
        naming row is what decides which of each button's two legends that liveness belongs to. */
    drawButton (saveBounds(),   "SAVE",   "STORE",  ! namingMode && saveEnabled(),   namingMode);
    drawButton (deleteBounds(), "DELETE", "CANCEL", ! namingMode && deleteEnabled(), namingMode);

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
