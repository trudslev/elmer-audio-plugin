#pragma once

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <BinaryData.h>

#include <array>

/**
    Elmer's design tokens: every colour, coordinate, size and typographic constant.

    All coordinates are ABSOLUTE against the 1340 x 660 canvas. The prototype expresses layout as
    CSS flexbox, which cannot be transcribed directly, so the nested boxes were resolved by hand and
    then checked against `design/screenshots/panel.png`: all eight knob centres, the header divider
    and the top row's origin land on the measured render exactly.

    The panel is bitmap-composited - the fascia is drawn in code and the production assets sit on
    top. There is deliberately no dressed-panel render in BinaryData: Gatecrasher used one as its
    background and every live control ended up sitting over a baked copy of itself.
*/
#include <nf/HeaderPart.h>
#include <nf/ParameterReadout.h>

namespace ElmerTheme
{

//==============================================================================
namespace Colour
{
    // --- fascia --------------------------------------------------------------
    // #a9a294 is not a taste choice: it is set by BRAND.md's 7:1 legibility floor. On the original
    // #8b8579 even pure black ink tops out at 5.7:1, so the panel could not reach the floor by
    // darkening text - the fascia itself had to come up. Every bitmap in design/assets was
    // re-rendered against this value. If it changes again, they must be re-rendered too.
    inline const juce::Colour fascia         { 0xFFA9A294 };
    inline const juce::Colour railDark       { 0xFF847E73 };
    inline const juce::Colour railLight      { 0xFFB4AE9F };
    inline const juce::Colour railEdge       { 0xFF98917F };
    inline const juce::Colour screwLight     { 0xFFC9C3B6 };
    inline const juce::Colour screwDark      { 0xFF5C574E };

    // --- ink -----------------------------------------------------------------
    // One value for all functional text.
    // contrast: 7.57:1 vs fascia [functional]
    inline const juce::Colour ink            { 0xFF0F0F0C };
    inline const juce::Colour wordmarkInk    { 0xFF24231F };
    inline const juce::Colour markerInk      { 0xFF2B2A26 };
    /** §6's meter sub-caption — flavour, not functional, and the only ink on this panel BRAND.md
        lets sit below the 7:1 bar.
        // contrast: 5.78:1 vs fascia [flavour] */
    inline const juce::Colour inkFlavour     { 0xFF2D2B24 };

    // --- panels --------------------------------------------------------------
    inline const juce::Colour dividerLeft    { juce::Colour::fromRGBA (60, 54, 44, 115) };  // .45
    inline const juce::Colour dividerRight   { juce::Colour::fromRGBA (60, 54, 44, 46) };   // .18
    inline const juce::Colour plinthTop      { 0xFFB4AD9A };
    inline const juce::Colour plinthBottom   { 0xFF9B9488 };

    // --- LCD -----------------------------------------------------------------
    inline const juce::Colour lcdFrameTop    { 0xFF26241F };
    inline const juce::Colour lcdFrameBottom { 0xFF3A372F };
    inline const juce::Colour lcdGlassTop    { 0xFF1B1A16 };
    inline const juce::Colour lcdGlassBottom { 0xFF242219 };
    inline const juce::Colour lcdMeterBottom { 0xFF262419 };
    inline const juce::Colour phosphor       { 0xFFE6DCAE };
    inline const juce::Colour phosphorGlow   { juce::Colour::fromRGBA (214, 196, 124, 115) };
    inline const juce::Colour lcdHairline    { juce::Colour::fromRGBA (214, 196, 124, 56) };  // .22

    // --- Program buttons: split-legend annunciator caps -----------------------
    /** **One body, two independently-lamped windows, stacked.** The resting function on top, what
        the button becomes while naming beneath it. Neither button relabels and neither wears a
        disabled face; only the lamps change.

        The cream cap is gone and should not return. It was a pale face carrying dark text with a
        separate greyed-out disabled treatment, and both halves of that were wrong: a printed panel
        legend cannot rewrite itself, and no rack unit greys a button out - its lamp goes out. The
        retired values were #F0E9D3 -> #D6CDB2 with #302C24 ink, and a disabled face of
        #A5A094 -> #8F8A7E whose #6F6A5F label measured **1.56:1**. */
    inline const juce::Colour capTop         { 0xFF57503F };
    inline const juce::Colour capBottom      { 0xFF211F19 };

    /** **The lens body never changes - only the characters light.** Both windows render this same
        ground lit or unlit; what differs is the legend and the halo the glowing characters throw
        onto the ground around them.

        An earlier revision warmed the whole lit window, and it read as a filled rectangle lighting
        up rather than as a bulb behind a legend - which is precisely the failure BRAND.md names
        when it says the lamp lights the letters, not the button. */
    inline const juce::Colour windowTop      { 0xFF2A2822 };
    inline const juce::Colour windowBottom   { 0xFF1E1C17 };

    // contrast: 4.30-5.00:1 vs windowTop,windowBottom [state]
    inline const juce::Colour legendUnlit    { 0xFF8F8A7C };
    /** Warm white - incandescent, not any of Elmer's three function-group colours, and NOT the
        panel accent #F3D021, which stays reserved for the KNEE lamp.
        // contrast: 13.10-15.10:1 vs windowTop,windowBottom [functional] */
    inline const juce::Colour legendLit      { 0xFFFFEFD0 };

    // --- KNEE lamp -----------------------------------------------------------
    /** **The lit face is DARKER than the unlit one, and that is the fix, not a mistake.**

        This is Elmer's only lit indicator, and it used to be the least legible label on the panel:
        one face served both states, so the selected legend drew #FFF6C9 on mid-grey at
        **2.28-3.17:1** while the unselected one sat at 4.94-5.86. The engaged state was harder to
        read than the disengaged one. No runtime change could fix it - lifting #FFF6C9 off that
        grey is not possible - so it needed the face itself to darken when lit, which is what the
        2026-08-11 handoff delivered. */
    inline const juce::Colour lampFaceTop    { 0xFFA9A496 };
    inline const juce::Colour lampFaceBottom { 0xFF8E8A7D };
    // contrast: 6.90-4.90:1 vs lampFaceTop,lampFaceBottom [functional]
    inline const juce::Colour lampLegendOff  { 0xFF1D1C17 };
    inline const juce::Colour lampFaceLitTop    { 0xFF46402F };
    inline const juce::Colour lampFaceLitBottom { 0xFF322D21 };
    // contrast: 9.50-12.60:1 vs lampFaceLitTop,lampFaceLitBottom [functional]
    inline const juce::Colour lampLegendOn   { 0xFFFFF6C9 };

    /** The one accent, per BRAND.md, and the ONLY lit indicator anywhere on this panel. Reserved
        for the selected KNEE dot. Orange and red belong to siblings and are deliberately avoided;
        gain reduction is not a fault, so the meter has no red zone either. */
    inline const juce::Colour accent         { 0xFFF3D021 };
    inline const juce::Colour ledOnCore      { 0xFFFFFDF0 };
    inline const juce::Colour ledOnEdge      { 0xFF8A7108 };
    inline const juce::Colour ledOffCore     { 0xFF7D7466 };
    inline const juce::Colour ledOffMid      { 0xFF43403A };
    inline const juce::Colour ledOffEdge     { 0xFF2A2825 };

    // --- scribble strip ------------------------------------------------------
    inline const juce::Colour tapeTop        { 0xFFEFE9D6 };
    inline const juce::Colour tapeMid        { 0xFFDED7C0 };
    inline const juce::Colour tapeBottom     { 0xFFE6DFCA };
}

//==============================================================================
namespace Font
{
    /** Function-local statics: created once, lazily, thread-safely. JUCE's binary-data name
        mangling STRIPS non-alphanumerics, so IBMPlexMono-SemiBold becomes IBMPlexMonoSemiBold_ttf. */
    inline juce::Typeface::Ptr archivoBlack()
    {
        static const juce::Typeface::Ptr t = juce::Typeface::createSystemTypefaceFor (
            BinaryData::Archivo_ExpandedBold_ttf, (size_t) BinaryData::Archivo_ExpandedBold_ttfSize);
        return t;
    }

    inline juce::Typeface::Ptr barlowSemiBold()
    {
        static const juce::Typeface::Ptr t = juce::Typeface::createSystemTypefaceFor (
            BinaryData::BarlowCondensedSemiBold_ttf, (size_t) BinaryData::BarlowCondensedSemiBold_ttfSize);
        return t;
    }

    inline juce::Typeface::Ptr monoRegular()
    {
        static const juce::Typeface::Ptr t = juce::Typeface::createSystemTypefaceFor (
            BinaryData::IBMPlexMonoRegular_ttf, (size_t) BinaryData::IBMPlexMonoRegular_ttfSize);
        return t;
    }

    inline juce::Typeface::Ptr monoMedium()
    {
        static const juce::Typeface::Ptr t = juce::Typeface::createSystemTypefaceFor (
            BinaryData::IBMPlexMonoMedium_ttf, (size_t) BinaryData::IBMPlexMonoMedium_ttfSize);
        return t;
    }

    inline juce::Typeface::Ptr monoSemiBold()
    {
        static const juce::Typeface::Ptr t = juce::Typeface::createSystemTypefaceFor (
            BinaryData::IBMPlexMonoSemiBold_ttf, (size_t) BinaryData::IBMPlexMonoSemiBold_ttfSize);
        return t;
    }

    inline juce::Typeface::Ptr permanentMarker()
    {
        static const juce::Typeface::Ptr t = juce::Typeface::createSystemTypefaceFor (
            BinaryData::PermanentMarkerRegular_ttf, (size_t) BinaryData::PermanentMarkerRegular_ttfSize);
        return t;
    }

    /** Builds a font whose em size equals the design's CSS px value.

        This is what `font-size: 12px` means, and it is NOT juce::Font::withHeight(), which sets
        ascent+descent - a typeface-specific multiple of the em, so a CSS px passed straight to
        withHeight() renders visibly small. Gatecrasher needed a measured calibration constant for
        this; JUCE 8's withPointHeight() expresses it directly. */
    inline juce::Font of (juce::Typeface::Ptr face, float cssPx)
    {
        return juce::Font (juce::FontOptions (face).withPointHeight (cssPx));
    }

    inline juce::Font wordmark (float px) { return of (archivoBlack(), px); }
    inline juce::Font label (float px)    { return of (barlowSemiBold(), px); }
    inline juce::Font mono (float px)     { return of (monoRegular(), px); }
    inline juce::Font monoMed (float px)  { return of (monoMedium(), px); }
    inline juce::Font monoBold (float px) { return of (monoSemiBold(), px); }
    inline juce::Font marker (float px)   { return of (permanentMarker(), px); }
}

//==============================================================================
namespace Text
{
    /** U+2014, built from a codepoint: juce::String's const char* constructor decodes as LATIN-1,
        so a UTF-8 literal would render as stray glyphs. */
    inline juce::String emDash() { return juce::String::charToString ((juce::juce_wchar) 0x2014); }

    /** The naming cursor is a **block**, U+2588, blinking at 1 s / 50 % duty - the house form in
        every casting, not this panel's own invention. The spec asks for a native text caret; the
        suite convention outranks it, and a thin bar would make Elmer the only casting whose naming
        field looks like a web form rather than a piece of hardware.

        Built from the codepoint: juce::String's const char* constructor decodes Latin-1, so a UTF-8
        literal renders as three stray glyphs. */
    inline juce::String blockCursor()
    {
        return juce::String::charToString ((juce::juce_wchar) 0x2588);
    }

    inline juce::String middleDot()
    {
        // Built from the codepoint: juce::String's const char* constructor decodes Latin-1, not
        // UTF-8, so a "\xc2\xb7" literal renders as a stray A-circumflex on the panel.
        return juce::String::charToString ((juce::juce_wchar) 0x00B7);
    }

    inline float trackedWidth (const juce::String& text, const juce::Font& font, float tracking)
    {
        float width = 0.0f;

        for (int i = 0; i < text.length(); ++i)
        {
            width += juce::GlyphArrangement::getStringWidth (font, juce::String::charToString (text[i]));

            if (i < text.length() - 1)
                width += tracking;
        }

        return width;
    }

    /** juce::Font has no absolute-pixel letter-spacing, so tracked text is drawn glyph by glyph.
        Every tracking figure in this design is quoted in px, not em - unlike Fifth Member's. */
    inline void drawTracked (juce::Graphics& g, const juce::String& text, const juce::Font& font,
                             float tracking, juce::Rectangle<float> area,
                             juce::Justification justification, juce::Colour colour,
                             bool withRelief = true)
    {
        const float total = trackedWidth (text, font, tracking);
        float startX = area.getX();

        if (justification.testFlags (juce::Justification::horizontallyCentred))
            startX = area.getCentreX() - total * 0.5f;
        else if (justification.testFlags (juce::Justification::right))
            startX = area.getRight() - total;

        // Every dark panel label carries a 1px light relief edge beneath it. It is a material cue -
        // ink sitting in a moulded surface - and it also lifts the glyph off the brushed grain.
        const int passes = withRelief ? 2 : 1;

        for (int pass = 0; pass < passes; ++pass)
        {
            const bool relief = withRelief && pass == 0;
            g.setFont (font);
            g.setColour (relief ? juce::Colours::white.withAlpha (0.32f) : colour);

            float x = startX;

            for (int i = 0; i < text.length(); ++i)
            {
                const auto ch = juce::String::charToString (text[i]);
                const float w = juce::GlyphArrangement::getStringWidth (font, ch);

                g.drawText (ch, juce::Rectangle<float> (x, area.getY() + (relief ? 1.0f : 0.0f),
                                                        w + 1.0f, area.getHeight()),
                            juce::Justification::centredLeft, false);
                x += w + tracking;
            }
        }
    }
}

//==============================================================================
namespace Layout
{
    /*  **1340 x 660, AND THIS IS THE ONLY CASTING WHOSE HEIGHT DROPS.** Call 1 brought +220 of
        width (1120 -> 1340); the height comes DOWN, 776 -> 660, because the four-section layout at
        full width no longer needs the depth.

        **That inverts the usual check.** Everywhere else in the suite a body figure moved down the
        panel; here anything positioned from the BOTTOM moves UP by 116, while anything positioned
        from the top does not move at all. So a figure that looks wrong is right and vice versa, and
        the constants below were checked ONE AT A TIME rather than by pattern — which is the lesson
        Chorus-60's knob row taught, where three of five stayed put and the two that moved read as
        transcription slips when the row was checked as a group. */
    inline constexpr float canvasWidth  = 1340.0f;
    inline constexpr float canvasHeight = 660.0f;
    inline constexpr float cornerRadius = 5.0f;

    /*  **The chassis rails and screws, moved rather than retired — and neither prototype has
        them.** Grepped both: the superseded `Elmer.dc.html` and the delivered
        `Elmer GL-87 Panel.dc.html` contain no screw, no rail and no circular-highlight gradient.
        They are this build's own chassis metaphor in both revisions, not a delivered element.

        That makes their fate a design question rather than a refactor's, so they keep their
        relationship to the panel: 11 px in from each corner. The two BOTTOM screws are exactly the
        case this canvas inverts — they were 765 against a 776 panel and are **649** against a 660
        one, which is 116 px UP where every other casting's move went down. */
    inline constexpr float railWidth = 15.0f;
    inline constexpr float screwDiameter = 6.0f;
    inline constexpr float screwInset = 11.0f;
    inline constexpr std::array<juce::Point<float>, 4> screwCentres { {
        { 8.0f, screwInset }, { canvasWidth - 8.0f, screwInset },
        { 8.0f, canvasHeight - screwInset }, { canvasWidth - 8.0f, canvasHeight - screwInset } } };

    /*  **`contentBottom` is DELETED, not moved, and the rule says which.** It was 756 — the old
        canvas less its 20 px inset — and it had exactly **one** appearance in this repo: its own
        declaration. A value nothing reads cannot be checked by anything and drifts silently, which
        root CLAUDE.md rules is a derive-or-delete, never a keep-and-correct. Nothing derives from
        it, so it goes.

        The other three inset figures are read (`contentX` 8 sites, `contentRight` 3, `contentY` 5)
        and are left standing for the body pass, which re-lays them against §1's body origin at
        y 120 and its dividers at x 16..1324. */
    inline constexpr float contentX = 22.0f;
    inline constexpr float contentY = 20.0f;
    inline constexpr float contentRight = 1098.0f;
    inline constexpr float contentWidth = contentRight - contentX;   // 1076

    // --- header (112 tall, 13px right padding) --------------------------------
    inline constexpr float headerHeight = 112.0f;
    inline constexpr float headerBottom = contentY + headerHeight;   // 132

    inline constexpr float nameplateX = 37.0f;      // contentX + 15px column padding
    inline constexpr float nameplateY = 22.0f;      // contentY + 2px column padding
    inline constexpr float wordmarkSize = 31.0f;
    inline constexpr float wordmarkTracking = 6.0f;
    inline constexpr float plinthPadLeft = 18.0f;
    inline constexpr float plinthPadRight = 20.0f;
    inline constexpr float plinthPadTop = 7.0f;
    inline constexpr float plinthPadBottom = 9.0f;
    inline constexpr float plinthRadius = 3.0f;

    inline constexpr float taglineSize = 14.0f;
    inline constexpr float taglineTracking = 3.6f;
    inline constexpr float modelLineSize = 10.5f;
    inline constexpr float modelLineTracking = 2.2f;

    inline constexpr float captionSize = 9.5f;      // PROGRAM / IN / OUT
    inline constexpr float captionTracking = 3.0f;

    /** The caption row, measured off the reference render rather than derived: cap ink lands at
        y 45..52 there, so the 12px box that centres it starts at 42.5. The captions sit 5px above
        the row and rise with it - the 34px band moved the row up 2px, and the captions follow,
        which is why this is not a constant anyone should pin independently.

        They used to sit at contentY, which was right for a row that began at 37 and is 24px too
        high for one that begins at 59. */
    inline constexpr float captionY = (float) nf::HeaderGeometry::captionY;

    /** **The header row: 34px tall, at y 59.** Every element in it - display, SAVE, DELETE, IN and
        OUT - is the same height and shares this Y, so the band reads as one instrument; the display
        is not allowed to be the odd one out.

        **34 is the suite's figure, not this panel's.** BRAND.md fixes the header part height at
        34px in every casting - not a proportion of the panel, because the castings are
        differently-sized units rather than scales of one design, and a manufacturer uses the same
        physical part across a product line. It came up from 30 with that ruling.

        59 is not a free choice either: the row is centred against the FULL 112px header block,
        which starts at the 20px content inset, so (112 - 34) / 2 + 20 = 59. Centring on the
        wordmark plate instead - which is what the old 37 amounted to - left the display sitting
        high against a left column running well below it. `DisplayBudgetTests` asserts that
        relationship rather than the literal, so the row cannot gain height without moving its Y:
        that half-update is exactly what would otherwise ship.

        **Widths are unchanged by the 34px change** - the display stays 361 at x 403..764, the
        buttons 62, the meters 74 - so the character budget is untouched at 24. The height went
        into the buttons, which now carry two stacked legends each.

        The display was 364 x 38 and held 21 characters. At 361 with 14px type it holds 24: the
        weight comes off and three characters come back. */
    /*  **THE BAND IS `nf::HeaderGeometry` NOW, AND THESE ARE ALIASES — one edit with the canvas,
        because they cannot be separated.**

        The shared band puts the LCD at 357..998 and the meter wells out to 1302. This casting's
        canvas was 1120 wide, so aliasing before the canvas move would have placed SAVE, DELETE and
        both meters past the right edge of the panel; moving the canvas first would have left the
        header cluster bunched into the left two-thirds of a 1340 panel. Neither is a state worth
        committing, and neither would have failed a build.

        **Every figure moved, and by different amounts** — which is why they are aliased in one go
        rather than nudged: the LCD's x by -46, its width by +280, SAVE by +235 and 0 width, DELETE
        by +236 and +8, the meter wells by +236 and +246 with widths -10. A row where every member
        moves differently is exactly the row a diff cannot check. */
    inline constexpr float lcdRowY = (float) nf::HeaderGeometry::bandY;
    inline constexpr float lcdRowH = (float) nf::HeaderGeometry::bandH;
    inline constexpr float programX = (float) nf::HeaderGeometry::lcdX;

    /** 361, not the 359 the cell widths alone give: 56 + 269 + 28 is 353, plus TWO 1px hairlines
        between the three cells, plus the 3px frame on each side. Measured 403..764 in the reference
        render, which agrees exactly. */
    /*  **641, and this is the width the first pass MISSED.** `programX` was aliased and this was
        not, so the display started on core's x and ended on this casting's own 361 — a rect with one
        edge moved and one edge not, which a diff cannot see and a capture shows immediately. It was
        found exactly that way: the band measured 360..714.5 against core's 357..998.

        Its cell widths (56 / 269 / 28) are this casting's and are re-laid by the LCD pass; what is
        shared is the outer box. */
    inline constexpr float programW = (float) nf::HeaderGeometry::lcdW;

    /** The display's three cells, inside the 3px metal frame. Bank 56 + name 269 + chevron 28.
        The name cell's 11px horizontal padding leaves 247px of text, which at IBM Plex Mono 14px
        with 1.7px tracking (10.1px per character) is the 24-character budget the spec cites. */
    inline constexpr float lcdFrameThickness = 3.0f;
    inline constexpr float lcdBankCellW = 56.0f;
    /*  **549, ABSORBING THE NEW OUTER WIDTH — and the budget and the cap are deliberately NOT
        touched in this commit.**

        The display's outer box is the shared part's 641 now. Its cells are still this casting's, so
        the name cell takes the difference: 641 = 3 frame + 56 bank + 1 + **549** + 1 + 28 chevron +
        3 frame. That figure is forced by the arm below rather than chosen — it is the only value
        that keeps the cells summing to the box.

        **It was typed as 533 first and the arm rejected it at 625 against 641** — a sixteen-pixel
        subtraction slip caught in seconds by the one assertion here whose whole job is that the
        parts add up. That is the argument for the arm existing rather than for computing carefully.

        **What is NOT done here is the LCD's own pass, and the reason is §11.** The part makes the
        LCD cell *with its budget and cap* shared — 49 and 47 against `nf::LcdCell::nameAreaW` —
        and §11 gates adopting them on the casting holding Share Tech Mono. The face is delivered
        (`design/fonts/ShareTechMono-Regular.ttf`) but is **not in BinaryData**, and this casting's
        LCD is still IBM Plex Mono at 14 px. Adopting a budget measured on a face the build does not
        embed is adopting a measurement nobody can reproduce, which is the thing §11 forbids.

        **A cap may never shrink, so it is the one figure that cannot be corrected afterwards.**
        Growing the cell while leaving the cap at 22 is safe in the only direction that matters: 533
        px holds far more than 24 characters, so nothing a user has typed becomes untypable. The
        rise from 22 to 47 — §11 names it as the largest in the suite — belongs in the commit that
        embeds the face and re-measures the advance. */
    inline constexpr float lcdNameCellW = 549.0f;
    inline constexpr float lcdChevronCellW = 28.0f;

    /** The chevron, drawn as a stroked path in an 11 x 7 box so it renders identically whatever the
        platform's font fallback does - and because it FLIPS with menu state, which a baked glyph
        could not. */
    inline constexpr float chevronW = 11.0f;
    inline constexpr float chevronH = 7.0f;
    inline constexpr float chevronStroke = 1.6f;

    /** The menu hangs from the GLASS's lower edge, not the frame's outside: the 34px display less
        two 3px frame edges is 28px of glass, plus 4px to clear its inner shadow. Rose with the
        band; it is 34 - 2 x 3 + 4 and not a free number. */
    inline constexpr float menuTopOffset = 32.0f;

    /** **22 = 24 - 2, and the three characters it gained are the "NN " prefix that user names no
        longer carry.**

        Only FACTORY Programs are numbered now: User Programs sort alphabetically, so a number would
        change whenever one was saved. The cell still holds 24, and the dirty marker " *" still takes
        2 - the naming cursor takes 1, so the marker is the binding constraint.

        The factory side is unchanged and still the tighter case: "03 MINNEAPOLIS SQUEEZE *" is
        3 + 19 + 2 = 24 and fills the cell precisely. That is why the budget test is three separate
        cases rather than one formula - the prefix applies to one bank only. */
    inline constexpr int maxUserNameLength = 22;
    inline constexpr int lcdCharacterBudget = 24;
    inline constexpr float lcdFrameRadius = 3.0f;
    inline constexpr float lcdGlassRadius = 2.0f;
    inline constexpr float lcdCellHairline = 1.0f;

    /** 14px IBM Plex Mono at 1.7px tracking advances 10.1px per character. That figure is what the
        24-character budget is computed from, so changing either without re-deriving the cap silently
        breaks the guarantee that what can be typed can be shown. */
    inline constexpr float lcdTextSize = 14.0f;
    inline constexpr float lcdTextTracking = 1.7f;
    inline constexpr float lcdNamePadX = 11.0f;

    /** The Program buttons. **SAVE and DELETE are no longer the same width** — the part gives
        SAVE 62 and DELETE 70, because DELETE/CANCEL is the longer legend pair. This casting had
        both at 62, so `headerButtonW` splits in two. */
    inline constexpr float saveX = (float) nf::HeaderGeometry::saveX;
    inline constexpr float saveW = (float) nf::HeaderGeometry::saveW;
    inline constexpr float deleteX = (float) nf::HeaderGeometry::deleteX;
    inline constexpr float deleteW = (float) nf::HeaderGeometry::deleteW;
    inline constexpr float headerButtonW = saveW;   // retained: existing call sites read this
    /** 10px is BRAND.md's floor for functional text and **both** legends are functional, so
        neither is set smaller than the other to make the pair fit. Tracking came in from 2.0 to
        1.4 because two stacked legends at 2.0 crowd DELETE/CANCEL against the 62px cap. */
    inline constexpr float buttonTextSize = 10.0f;
    inline constexpr float buttonTracking = 1.4f;

    /** The cap's 2px padding, then two windows of 15px each: 2 + 15 + 15 + 2 = 34. The windows
        share an edge with no gap, deliberately - the ground is continuous so a lit legend's halo
        can spill into the neighbouring legend's half, which is what a real bulb behind a split
        lens does. **No rib, divider, bezel or lamp is drawn as its own element**: BRAND.md forbids
        it, and the two legends separate by their own leading plus the difference in kind between a
        luminous and a matte legend. */
    inline constexpr float capPadding = 2.0f;
    inline constexpr float legendWindowH = 15.0f;
    inline constexpr float windowRadius = 2.0f;

    inline constexpr float meterInX = (float) nf::HeaderGeometry::inWellX;
    inline constexpr float meterOutX = (float) nf::HeaderGeometry::outWellX;
    inline constexpr float meterWellW = (float) nf::HeaderGeometry::meterWellW;
    /** **64, from the part.** Same miss as `programW`: `meterInX` and `meterOutX` were aliased
        while the width stayed at this casting's 74, so both wells measured 1164..1236.5 and
        1238..1310.5 where the part gives 1164..1228 and 1238..1302 — the second overrunning the
        band's own right edge at 1302. */
    inline constexpr float levelBoxW = meterWellW;
    inline constexpr float levelTextSize = 14.0f;

    // --- divider --------------------------------------------------------------
    /*  **§2's BODY IS THREE COLUMNS — and this replaces the FOUR-SECTION table of one day earlier.**

        Bundle 2 gave a meter + DETECTOR row over a TIMING + OUTPUT row, divided at x 500 / y 386 /
        x 700. Bundle 3 replaces it with **DETECTOR left, the meter centred in its own column,
        TIMING over OUTPUT right**, so that left-to-right order carries the signal path: detection,
        then timing, then output.

        **Both dividers run the FULL band and are a matched pair**, y 156 → 630. They previously
        stopped at each column's own last row, which left the left one 74 px short — that existed to
        clear the scribble strip in the bottom-left corner, and the strip has moved to the centre
        column, so nothing is down there to collide with.

        **Each divider sits MID-GUTTER, not at a fixed inset from either side**, and that is why
        they are transcribed rather than derived: §2 states DETECTOR's ink ends at 293 and the well
        starts at 355, so the rule takes **324** with 31 px of air either way. A figure computed from
        a column edge would land somewhere else and look entirely reasonable. */
    inline constexpr float bodyBandTop = 156.0f, bodyBandBottom = 630.0f;
    inline constexpr float dividerInk = 0.5f;   // rgba(255,255,255,.5)
    inline constexpr float dividerLeftX = 324.0f;
    inline constexpr float dividerRightX = 1010.0f;

    /*  §2's three column headings, Barlow Condensed 600 at 12 / 15 / .28 em, each CENTRED over the
        region it names. OUTPUT is the one that moved axis rather than position: it was a bottom-row
        section beside TIMING and is now stacked BELOW it in the same right column. */
    inline constexpr float sectionHeadingCssPx = 12.0f, sectionHeadingTrackingEm = 0.28f;
    inline constexpr float sectionHeadingTracking = sectionHeadingCssPx * sectionHeadingTrackingEm;
    inline constexpr float sectionHeadingLineBox = 15.0f;

    struct SectionHeading { const char* text; float x, y, w; };
    inline constexpr std::array<SectionHeading, 3> sectionHeadings { {
        { "DETECTOR",   16.0f, 156.0f, 294.0f },
        { "TIMING",   1040.0f, 156.0f, 268.0f },
        { "OUTPUT",   1040.0f, 339.0f, 268.0f } } };

    /*  §4's two meter rows. **Each carries a LEFT and a RIGHT string**, which is the pairing that
        went wrong yesterday: the caption and the stereo note were drawn from two different sites
        and printed twice. They are one table now, so a move takes both ends with it.

        The live `GR −x.x dB` is the right half of the caption row and is drawn by `ProgramHeader`
        with the other live text — it is the only one of the four that changes. */
    inline constexpr float meterLabelRowY = 179.0f;
    inline constexpr float meterCaptionRowY = 457.0f;
    inline constexpr float meterRowCssPx = 11.0f, meterRowTrackingEm = 0.14f;
    inline constexpr float meterRowTracking = meterRowCssPx * meterRowTrackingEm;
    inline constexpr float meterRowLineBox = 14.0f;

    /*  §5: the strip is **centred under the meter** now, not in the bottom-left corner, and §5 is
        explicit that it was never retired — it existed in the 1120 x 776 prototype's footer and
        fell out of a re-layout unrecorded. Scaled 1.5x from 19 px this round, with padding and
        tracking scaled alongside so the tape grows with the type rather than the type outgrowing
        the tape.

        (396, 566) is "about 45 px left of the centre column's centre line", which §5 says in words —
        so it is a stated position and not a centring to compute. */
    inline constexpr float scribbleX = 396.0f, scribbleY = 566.0f;

    /*  §5: the serial and the version consolidate into ONE right-aligned line. This casting drew
        two — a left `GL-87 · SN 0871` and a right `v1.0` — which was yesterday's fix for the run
        that crossed OUTPUT's scale. One line supersedes it. */
    inline constexpr float footerLineX = 924.0f, footerLineY = 640.0f, footerLineW = 400.0f;

    inline constexpr float dividerY = 140.0f;       // SUPERSEDED, retired with paintSections

    // --- top row --------------------------------------------------------------
    inline constexpr float topRowY = 155.0f;        // dividerY + 1 + 14
    inline constexpr float topRowH = 352.0f;

    inline constexpr float sectionRadius = 4.0f;
    inline constexpr float sectionPadTop = 14.0f;
    inline constexpr float sectionPadX = 16.0f;
    inline constexpr float sectionHeaderSize = 11.5f;
    inline constexpr float sectionHeaderTracking = 3.4f;
    inline constexpr float controlLabelSize = 11.5f;
    inline constexpr float controlLabelTracking = 2.4f;
    /** A control label sits 15px below its knob area. */
    inline constexpr float controlLabelGap = 15.0f;

    inline constexpr float detectionX = 22.0f;
    inline constexpr float detectionW = 340.0f;

    // The meter column is flex:1 after a 20px gap, and the 588px meter is centred inside it.
    /*  **The meter well, at the delivered prototype's box: (60, 176), 396 x 159.** It was
        (446, 213) at 588 x 236 — the previous canvas's, where the meter sat right of centre. §2
        puts GAIN REDUCTION METER at (60, 150) with the well beneath it, so the whole instrument
        moves to the left column and shrinks by a third. */
    /*  **BUNDLE 3's well: 630 x 254 at (355, 199).** It was (60, 176) at 396 x 159 for one day —
        bundle 2's centre column — and (446, 213) at 588 x 236 before that. §4 states it outright
        now rather than leaving it to a prototype, which is why this is a transcription rather than
        a measurement off markup.

        **The aspect is locked by the cut face at 2.4854:1** — §4 says width and height are one
        figure, not two — so a future change to either must come from a re-cut rather than from
        here. 630 / 253.5 = 2.4852, which is that ratio to four places. */
    inline constexpr float meterX = 355.0f;
    inline constexpr float meterW = 630.0f;
    inline constexpr float meterH = 254.0f;
    inline constexpr float meterY = 199.0f;
    inline constexpr float meterRadius = 4.0f;
    inline constexpr float meterSpecSize = 11.5f;
    inline constexpr float meterSpecTracking = 1.8f;
    inline constexpr float meterSpecGap = 9.0f;

    // --- bottom row -----------------------------------------------------------
    inline constexpr float bottomRowY = 523.0f;     // topRowY + topRowH + 16
    inline constexpr float bottomRowH = 190.0f;

    inline constexpr float timingX = 22.0f;
    inline constexpr float timingW = 340.0f;
    inline constexpr float characterX = 445.0f;
    inline constexpr float characterW = 230.0f;
    inline constexpr float outputX = 758.0f;
    inline constexpr float outputW = 340.0f;

    /*  --- footer -------------------------------------------------------------------------
        **725 -> 634, and this is a bottom-derived figure that moved UP.** It sat 51 px above the
        old 776 panel bottom; the delivered prototype puts both footer strings on **y 634**, which
        is 26 above a 660 panel. So it is not the old figure shifted by the canvas delta — it is a
        measured position from the new cut, and checking it as "776 - 51 -> 660 - 51 = 609" would
        have been wrong by 25. Bottom-derived does not mean bottom-relative. */
    inline constexpr float footerY = 634.0f;
    /** Both ends, from the delivered prototype. One full-width run is what put `SN 0871` through
        OUTPUT's scale — see PanelBackground's note. */
    inline constexpr float footerLeftX = 26.0f;
    inline constexpr float footerRightX = 914.0f;
    inline constexpr float footerLineBox = 13.0f;
    inline constexpr float footerTextSize = 11.5f;
    inline constexpr float footerTracking = 1.8f;
    /*  **28.5, scaled 1.5x from the original 19 px this round — and this read 21.**

        §5 states the scale explicitly and states that padding and tracking scale WITH the type, so
        the tape grows proportionally rather than the type outgrowing it. 21 was neither the
        original nor the scaled figure; it is what this build had drifted to.

        §5 also records that the strip **was never retired** — it existed in the 1120 x 776
        prototype's footer and fell out of a re-layout unrecorded, which is why bundle 3 restates it
        rather than introducing it. */
    inline constexpr float scribbleSize = 28.5f;
    inline constexpr float scribbleRotationDegrees = -2.4f;

    /** The tape's padding, from the prototype's `padding: 9px 30px 11px 32px` - **asymmetric on
        both axes**, which is why these are four numbers rather than a reduced() call. Together with
        the 0.5px letter-spacing below they are what sizes the tape: the strip was 29.5px narrower
        than the render because it used 42px of total horizontal padding against the specified 62,
        and drew the marker text with no tracking at all. */
    // §5's padding, 10.5 / 39 / 13.5 / 42 as top / right / bottom / left.
    inline constexpr float scribblePadLeft   = 42.0f;
    inline constexpr float scribblePadRight  = 39.0f;
    inline constexpr float scribblePadTop    = 10.5f;
    inline constexpr float scribblePadBottom = 13.5f;
    inline constexpr float scribbleTracking  = 0.5f;

    //==========================================================================
    /** Knobs are 128-frame vertical filmstrips. Frame = round(value01 * 127), source Y offset
        = -frame * frameSize. Rotation -140 to +140 is BAKED INTO the strip; nothing rotates at
        runtime, which is the whole reason for using filmstrips - the specular highlight travelling
        across a coloured cap is what the colour-coding is for, and a code-drawn gradient flattens
        it. */
    inline constexpr int filmstripFrames = 128;
    inline constexpr int filmstripFrameSize = 96;

    enum class Strip { detect, timing, output };
    enum class Ring  { large11, small9, five };

    struct Legend { float left, top; const char* text; };

    struct KnobSpec
    {
        const char* paramId;
        const char* label;
        juce::Point<float> areaCentre;   // centre of the scale area, absolute
        float areaSize;                  // 112 on THRESHOLD, 100 elsewhere
        float ringSize;                  // the tick-ring bitmap's drawn width
        float knobSize;                  // the filmstrip's drawn diameter
        Strip strip;
        Ring  ring;
        float labelY;                    // absolute y of the control label's line box
    };

    /** Legends are drawn at their literal offsets from the scale area's top-left, exactly as the
        design places them: each is a 40px-wide centred box, so the glyph centre is
        (left + 20, top + 6).

        DO NOT recompute these from an angle and a radius. Most are formula-clean - THRESHOLD sits
        at a constant 62px and SIDECHAIN HP at 54px - but RATIO's run 54px at the top, 59.5 at the
        sides and 60.3 at the bottom because its wider labels were pushed out by eye. RELEASE's
        "0.6s" sits at top=-8 where RATIO's "4:1" sits at -10 on an identical ring, and IRON's "25"
        at left=-21 where RATIO's "2:1" is at -26. Those two-pixel differences are the hand-tuning,
        and a formula erases every one of them. */
    inline constexpr float legendBoxWidth = 40.0f;
    inline constexpr float legendLineHeight = 12.0f;
    inline constexpr float legendSize = 10.0f;
    inline constexpr float tickRingOpacity = 0.62f;   // printed ink, not drawn UI

    inline const std::array<std::array<Legend, 6>, 8> legends { {
        // THRESHOLD
        { { { -4, 98, "-40" }, { -26, 44, "-30" }, { 7, -5, "-20" },
            { 65, -5, "-10" }, { 98, 44, "0" }, { 76, 98, "+10" } } },
        // RATIO - five legends; the sixth slot is unused
        { { { -9, 90, "1.5:1" }, { -26, 24, "2:1" }, { 30, -10, "4:1" },
            { 86, 24, "10:1" }, { 69, 90, "20:1" }, { 0, 0, nullptr } } },
        // SIDECHAIN HP
        { { { -5, 85, "OFF" }, { -24, 40, "40" }, { 5, -4, "75" },
            { 55, -4, "140" }, { 86, 40, "265" }, { 65, 85, "500" } } },
        // ATTACK
        { { { -5, 87, "0.1" }, { -26, 40, "0.3" }, { 5, -4, "1" },
            { 55, -4, "3" }, { 84, 40, "10" }, { 65, 87, "30" } } },
        // RELEASE
        { { { -9, 90, "0.1s" }, { -26, 24, "0.3s" }, { 30, -8, "0.6s" },
            { 86, 24, "1.2s" }, { 69, 90, "AUTO" }, { 0, 0, nullptr } } },
        // IRON
        { { { -5, 85, "0" }, { -21, 26, "25" }, { 30, -10, "50" },
            { 81, 26, "75" }, { 65, 85, "100" }, { 0, 0, nullptr } } },
        // MAKEUP
        { { { -5, 85, "0" }, { -21, 26, "5" }, { 30, -10, "10" },
            { 81, 26, "15" }, { 65, 85, "20" }, { 0, 0, nullptr } } },
        // MIX
        { { { -5, 85, "0" }, { -21, 26, "25" }, { 30, -10, "50" },
            { 81, 26, "75" }, { 65, 85, "100" }, { 0, 0, nullptr } } } } };

    /** Every knob, in the order the `legends` table above uses.

        The area centres were resolved from the CSS flexbox and then confirmed against
        `design/screenshots/panel.png` - all eight land on the measured cap centres. The knob and
        its scale area are concentric, so this one point positions the filmstrip, the tick ring and
        every legend. */
    /*  **§3'S EIGHT KNOBS. TWO DIAMETER CLASSES AND ONLY TWO — Ø76 and Ø56.**

        Row centres are **243 · 383 · 418 · 546** and cell centres **83 / 226** left, **1107 / 1241**
        right. Everything below follows from those except two entries, and BOTH of those look like
        transcription slips to anyone checking the pitch arithmetic later. That is not hypothetical:
        Chorus-60's row had three of five knobs stay put while MIX and OUTPUT TRIM moved, and the two
        that moved read as slips precisely because the three that did not made the row look uniform.
        So each exception carries its reason at the entry rather than in a table somewhere.

        **EXCEPTION 1 — RATIO's cell is 226, NOT the nominal 217.** Row 1 carries two Ø76 knobs, and
        at the nominal 134 pitch their NUMERAL RINGS collide: THRESHOLD's widest numeral reaches 148
        and RATIO's would start at 150. At 226 they clear by 11. The knobs themselves never touch at
        either figure — Ø76 at 134 apart has 58 px of air — so checking the CAPS would confirm 217
        and be wrong. What collides is the printed scale, which is wider than the control.

        **EXCEPTION 2 — MAKEUP is centred at 1174, not on a cell centre.** §3's rule is that a knob
        alone in its row is placed BY ITS CLASS: Ø76 centres on the column, Ø56 sits on a cell
        centre. Centring is only safe for the larger class, because a lone Ø56 parked between the two
        cell centres reads as a **third diameter**, intermediate between the classes flanking it —
        the diameters were never a third value, position was doing it.

        MAKEUP's row also sits closer to the row above than the cell arithmetic allows, and it works
        ONLY because it is centred: its numeral band (480–491) and top ticks (494) overlap the
        vertical band of IRON and MIX's legends (bottom 496) — but not their INK, because those sit
        on 1107 and 1241 while MAKEUP's top numeral sits at 1174, in the 41 px gap between them.
        **Moved onto a cell centre at this height its numerals would land on the legend above it.**
        So the centring and the row height hold each other up; neither is safe to change alone. */
    inline constexpr float knobLarge = 76.0f, knobSmall = 56.0f;
    inline constexpr float rowOne = 243.0f, rowSidechain = 383.0f;
    inline constexpr float rowIronMix = 418.0f, rowMakeup = 546.0f;
    inline constexpr float cellLeftA = 83.0f, cellLeftB = 226.0f;
    inline constexpr float cellRightA = 1107.0f, cellRightB = 1241.0f;
    inline constexpr float makeupCentre = 1174.0f;

    inline const std::array<KnobSpec, 8> knobs { {
        { "threshold",   "THRESHOLD",    { cellLeftA,  rowOne },       112.0f, 108.0f, knobLarge,
          Strip::detect, Ring::large11, 0.0f },
        // 226, not 217 — see EXCEPTION 1 above. The numeral rings collide at the nominal pitch;
        // the caps do not, so a check on the caps confirms the wrong figure.
        { "ratio",       "RATIO",        { cellLeftB,  rowOne },       112.0f, 108.0f, knobLarge,
          Strip::detect, Ring::large11, 0.0f },
        { "sidechainHp", "SIDECHAIN HP", { 163.0f,     rowSidechain }, 100.0f,  96.0f, knobSmall,
          Strip::detect, Ring::large11, 0.0f },
        { "attack",      "ATTACK",       { cellRightA, rowOne },       100.0f,  96.0f, knobSmall,
          Strip::timing, Ring::large11, 0.0f },
        { "release",     "RELEASE",      { cellRightB, rowOne },       112.0f, 108.0f, knobLarge,
          Strip::timing, Ring::large11, 0.0f },
        { "iron",        "IRON",         { cellRightA, rowIronMix },   100.0f,  96.0f, knobSmall,
          Strip::output, Ring::large11, 0.0f },
        { "mix",         "MIX",          { cellRightB, rowIronMix },   100.0f,  96.0f, knobSmall,
          Strip::output, Ring::large11, 0.0f },
        // 1174 — centred on the column, NOT on a cell centre. See EXCEPTION 2 above: it is the
        // only knob alone in its row, it is Ø76, and its row height depends on the centring.
        { "makeup",      "MAKEUP",       { makeupCentre, rowMakeup },  112.0f, 108.0f, knobLarge,
          Strip::output, Ring::large11, 0.0f },
    } };

    /*  §3.1's registration. **Unit and legend are positioned off a Ø76 box for EVERY class**, so the
        offset is `(76 − d) / 2 = 38 − r` — which is zero on the large class and 10 on the small one.
        Pivots register on the row's Y and both legend lines in a band land on one baseline.

        This panel was drawn before that was understood: it held the label baseline and missed the
        pivot by exactly 10 px in both bands, because its Ø56 cells were bottom-aligned. That is why
        the labels were right and the centres were not — a defect that inspecting the labels would
        have declared clean. */
    constexpr float knobRegistrationBox = 76.0f;
    constexpr float knobRegistrationOffset (float diameter)
    {
        return (knobRegistrationBox - diameter) * 0.5f;
    }

    constexpr float knobUnitTopFor (float diameter)
    {
        return diameter + 12.0f + knobRegistrationOffset (diameter);
    }
    constexpr float knobLegendTopFor (float diameter)
    {
        return diameter + 25.0f + knobRegistrationOffset (diameter);
    }

    /*  §3.1's sweep — **280°, start −140°**, which is per-casting freedom rather than the suite's
        270. It is why this casting's mark angles cannot be copied from another's table. */
    constexpr float knobSweepStartDeg = -140.0f, knobSweepSpanDeg = 280.0f;

    /*  §3.1's CODE-DRAWN CONSTRUCTION. The 128-frame filmstrips retire with it — bundle 3's opening
        makes the meter face and needle the ONLY bitmaps on this panel.

        **What the filmstrips bought and what they cost.** Their comment argued that baking the
        rotation kept the specular travelling across a coloured cap, and that a code-drawn gradient
        flattens it. That was true of a gradient rotating WITH the knob. §3.1 fixes the specular to
        the panel — `circle at 34% 24%` on the cap, the skirt's conic from a constant 200° — so
        nothing about the highlight depends on the value, and the argument for baking 128 frames per
        control goes with it. It is also what makes a static-layer cache possible at all. */
    inline const std::array<std::pair<float, juce::Colour>, 7> knobSkirtStops { {
        { 0.00f, juce::Colour (0xFFE8E3DA) }, { 0.18f, juce::Colour (0xFFB6AFA5) },
        { 0.34f, juce::Colour (0xFFEFEAE1) }, { 0.52f, juce::Colour (0xFFA8A199) },
        { 0.70f, juce::Colour (0xFFE6E1D8) }, { 0.86f, juce::Colour (0xFFB0A9A0) },
        { 1.00f, juce::Colour (0xFFE8E3DA) } } };
    constexpr float knobSkirtFromDeg = 200.0f;

    /** §2's function-group coding: highlight, base and shade per group. Colour is organisation, not
        information — every legend reads without it. */
    struct CapStops { juce::Colour hi, base, lo; };
    inline const CapStops capDetect { juce::Colour (0xFFEE5C9C), juce::Colour (0xFFD5257A),
                                      juce::Colour (0xFF8E1152) };
    inline const CapStops capTiming { juce::Colour (0xFF4FC79C), juce::Colour (0xFF17825F),
                                      juce::Colour (0xFF0C6247) };
    inline const CapStops capOutput { juce::Colour (0xFF6E9CE8), juce::Colour (0xFF3A6FD0),
                                      juce::Colour (0xFF1E4189) };

    inline const CapStops& capStopsFor (Strip strip)
    {
        switch (strip)
        {
            case Strip::timing: return capTiming;
            case Strip::output: return capOutput;
            case Strip::detect:
            default:            return capDetect;
        }
    }

    constexpr float knobCapInset = 6.0f;
    constexpr float knobCapHighlightX = 0.34f, knobCapHighlightY = 0.24f, knobCapMidStop = 0.52f;

    /** §3.1: `3 x (r - 7)`, `#f6f1e6`, corner radius 1.5. */
    constexpr float knobPointerWidth = 3.0f, knobPointerInset = 7.0f, knobPointerRadius = 1.5f;
    inline const juce::Colour knobPointerInk { 0xFFF6F1E6 };

    /** §3.1: O d + 12, a 1.4 px ring over 0.7778 turn — which is the 280 degree sweep as a
        fraction, so the arc and the pointer cannot disagree about the span. */
    constexpr float knobArcRadiusGap = 6.0f, knobArcThickness = 1.4f;
    inline const juce::Colour knobArcInk { 0x4D16150F };   // rgba(22,21,15,.30)

    /*  §3.1's ticks. **Both share an INNER end and differ at the outer**: a numbered tick runs to
        r + 14 inked 9, a plain one to r + 10 inked 5, so both are inked from **r + 5** outward.

        **A PREVIOUS COMMIT CALLED THIS THE OPPOSITE OF CHORUS-60'S AND THAT WAS WRONG.** That
        casting's ring is `tickInner = r + 8` drawn to `tickInner + length` — also a shared inner
        end, also differing at the outer. The two constructions are the same shape.

        What is genuinely inverted is how the two SPECS express it. Elmer's states the outer length
        and the ink, so the inner end is derived (14 − 9 = 5). Chorus-60's states the inner gap and
        the length, so the outer end is derived (8 + 9 = 17). Reading one as if it were the other
        puts every tick out by the difference and produces a perfectly plausible ring.

        So the property worth asserting is the **shared inner end**, not the two lengths: it is what
        both constructions have in common and what a mis-transcription between them destroys. The
        lengths differ between the castings legitimately and tell you nothing. */
    constexpr float knobNumberedTickLength = 14.0f, knobNumberedTickInk = 9.0f;
    constexpr float knobNumberedTickWidth = 2.0f;
    constexpr float knobPlainTickLength = 10.0f, knobPlainTickInk = 5.0f;
    constexpr float knobPlainTickWidth = 1.5f;
    inline const juce::Colour knobTickInk { 0xFF16150F };   // 7.47:1 on fascia

    constexpr float knobNumeralCssPx = 11.0f, knobNumeralLineBox = 11.0f;

    /** Derived, not transcribed: both tick kinds are inked from here outward. */
    constexpr float knobTickInnerRadiusGap = knobNumberedTickLength - knobNumberedTickInk;   // 5

    /*  §3.1's ring: the sweep arc, the ticks and the numerals. **They were baked ring bitmaps** —
        one per ring size, drawn by `PanelBackground` — and are drawn here now.

        The arc's 0.7778 turn is the 280° sweep as a fraction, so it is computed from the sweep
        rather than restated: the arc and the pointer cannot disagree about the span. */
    inline void paintKnobRing (juce::Graphics& g, juce::Point<float> centre, float diameter,
                               const std::vector<std::pair<float, juce::String>>& marks)
    {
        const float r = diameter * 0.5f;

        {
            const float arcR = r + knobArcRadiusGap;
            juce::Path arc;
            arc.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f,
                               juce::degreesToRadians (knobSweepStartDeg),
                               juce::degreesToRadians (knobSweepStartDeg + knobSweepSpanDeg), true);
            g.setColour (knobArcInk);
            g.strokePath (arc, juce::PathStrokeType (knobArcThickness));
        }

        const float inner = r + knobTickInnerRadiusGap;

        for (const auto& [value01, printed] : marks)
        {
            const bool numbered = printed.isNotEmpty();
            const float outer = inner + (numbered ? knobNumberedTickInk : knobPlainTickInk);
            const float width = numbered ? knobNumberedTickWidth : knobPlainTickWidth;
            const float angle = knobSweepStartDeg + value01 * knobSweepSpanDeg;

            const auto dir = juce::Point<float> (std::sin (juce::degreesToRadians (angle)),
                                                 -std::cos (juce::degreesToRadians (angle)));

            g.setColour (knobTickInk);
            g.drawLine ({ centre + dir * inner, centre + dir * outer }, width);
        }
    }

    /*  **THE STATIC LAYER — everything that does not move with the value.** Drawn at the
        component's own origin so the cached image is position-independent. */
    inline void paintKnobStatic (juce::Graphics& g, juce::Rectangle<float> area, Strip strip)
    {
        const auto centre = area.getCentre();
        const float r = juce::jmin (area.getWidth(), area.getHeight()) * 0.5f;

        /*  The machined skirt. JUCE has no conic gradient, so it is swept as thin wedges — 360 of
            them, one per degree, which is well under a pixel of arc at these diameters. Sweeping it
            is what a conic IS; approximating it with a linear or radial fill would lose the turned
            banding entirely, which is the one thing this skirt is for. */
        {
            juce::Graphics::ScopedSaveState saved (g);
            juce::Path clip;
            clip.addEllipse (centre.x - r, centre.y - r, r * 2.0f, r * 2.0f);
            g.reduceClipRegion (clip);

            for (int i = 0; i < 360; ++i)
            {
                const float t = (float) i / 360.0f;

                juce::Colour c = knobSkirtStops.back().second;
                for (size_t k = 1; k < knobSkirtStops.size(); ++k)
                    if (t <= knobSkirtStops[k].first)
                    {
                        const auto& a = knobSkirtStops[k - 1];
                        const auto& b = knobSkirtStops[k];
                        const float f = (t - a.first) / juce::jmax (1.0e-6f, b.first - a.first);
                        c = a.second.interpolatedWith (b.second, f);
                        break;
                    }

                juce::Path wedge;
                wedge.startNewSubPath (centre);
                const float a0 = juce::degreesToRadians (knobSkirtFromDeg + t * 360.0f);
                wedge.addArc (centre.x - r, centre.y - r, r * 2.0f, r * 2.0f, a0,
                              a0 + juce::degreesToRadians (1.2f));
                wedge.closeSubPath();

                g.setColour (c);
                g.fillPath (wedge);
            }
        }

        const auto cap = area.reduced (knobCapInset);
        const auto& stops = capStopsFor (strip);

        juce::ColourGradient capFill { stops.hi,
                                       cap.getX() + cap.getWidth() * knobCapHighlightX,
                                       cap.getY() + cap.getHeight() * knobCapHighlightY,
                                       stops.lo, cap.getRight(), cap.getBottom(), true };
        capFill.addColour (knobCapMidStop, stops.base);
        g.setGradientFill (capFill);
        g.fillEllipse (cap);
    }

    /** §3.1's pointer. **The only part that moves with the value**, and therefore the only part
        outside the cache. */
    inline void paintKnobPointer (juce::Graphics& g, juce::Point<float> centre, float diameter,
                                  float value01)
    {
        const float r = diameter * 0.5f;
        const float length = r - knobPointerInset;
        const float angle = knobSweepStartDeg
                          + juce::jlimit (0.0f, 1.0f, value01) * knobSweepSpanDeg;

        juce::Path pointer;
        pointer.addRoundedRectangle (-knobPointerWidth * 0.5f, -length,
                                      knobPointerWidth, length, knobPointerRadius);

        g.setColour (knobPointerInk);
        g.fillPath (pointer, juce::AffineTransform::rotation (juce::degreesToRadians (angle))
                                 .translated (centre.x, centre.y));
    }

    /** **Units live in the ARC GAP, not on the control name.** They sit on the bottom legend row,
        between the minimum and maximum numerals, in the same 10px legend type at 0.6px tracking -
        so a unit reads as part of the printed scale it qualifies rather than as part of the
        control's name. "ATTACK - ms" is gone.

        Literal offsets transcribed from the prototype, like `legends` above and for the same
        reason: THRESHOLD's is left 36 on a 112px area and the rest are left 30 on a 100px area,
        which is the box centred in both cases, but ATTACK's top is 87 where its neighbours' is 85.
        A formula would erase that.

        **Six controls carry a unit and three do not**, decided from the parameter definitions
        rather than from what the labels happened to print: RATIO prints ratios (`4:1`), RELEASE's
        values carry their own suffixes (`0.6s`, `AUTO`), and KNEE has no scale at all. No unit is
        invented for consistency's sake and none is dropped for the sake of the old label text. */
    inline constexpr float unitTracking = 0.6f;

    inline const std::array<Legend, 8> knobArcUnits { {
        { 36, 98, "dB" },       // THRESHOLD
        { 0, 0, nullptr },      // RATIO - prints ratios
        { 30, 85, "Hz" },       // SIDECHAIN HP
        { 30, 87, "ms" },       // ATTACK
        { 0, 0, nullptr },      // RELEASE - values carry their own suffixes
        { 30, 85, "%" },        // IRON
        { 30, 85, "dB" },       // MAKEUP
        { 30, 85, "%" } } };    // MIX

    /** The KNEE column: 140px wide, centred at the same x as RATIO, in row 2 of DETECTION.

        **The label sits BELOW the buttons**, like every other control name on the panel, and shares
        SIDECHAIN HP's label line - it used to sit above them, which made KNEE the only control on
        the panel named from the top.

        Note that `design/screenshots/panel.png` still shows the label above: the render is stale on
        this one element, and `design/Elmer.dc.html` and the handoff prose both agree it goes below
        (`padding-top: 38px`, buttons, then `margin-top: 15px`). Raised with the designers; do not
        "correct" this back to the render.

        The buttons start at 399 rather than the prototype's literal 400 so that 63px of buttons
        plus the 15px label margin lands the label on SIDECHAIN HP's 477 exactly. The prototype's
        own arithmetic (38 + 63 + 15 = 116 against the knob column's 100 + 15 = 115) puts it 1px
        lower; a shared baseline is the property that is visible, so it wins over the literal
        padding figure. */
    inline constexpr juce::Point<float> kneeButtonsTopLeft { 232.0f, 399.0f };
    inline constexpr float kneeLabelY = 477.0f;

    // --- KNEE lamp buttons ----------------------------------------------------
    inline constexpr float lampW = 74.0f;
    inline constexpr float lampH = 28.0f;
    inline constexpr float lampRadius = 2.0f;
    inline constexpr float lampGap = 7.0f;
    inline constexpr float lampLedDiameter = 6.0f;
    inline constexpr float lampContentGap = 7.0f;
    inline constexpr float lampLegendSize = 9.5f;
    inline constexpr float lampLegendTracking = 1.6f;

    // --- meter mapping --------------------------------------------------------
    /** Face source is 1000 x 402 with the needle pivot at (500, 500) - BELOW the visible face, by
        design. The needle sprite is 60 x 510 with its own pivot at (30, 499). At the 588px display
        width the scale factor is 0.588, putting the pivot at (294, 294) inside the meter body. */
    /*  **ALL FOUR SPRITE CONSTANTS DESCRIBED A SUPERSEDED ASSET PAIR, AND THAT IS WHY THE NEEDLE
        RESTED OUTSIDE THE FACE.**

        Measured off the delivered files rather than read from anywhere: `meter-face.png` is
        **1188 x 478** and `meter-needle.png` is **71 x 607**. The constants said 1000 and 60.

        A wrong source width is not a wrong size — it is a wrong SCALE, and the scale multiplies the
        pivot offset. At 588/1000 the needle's pivot offset came out (17.6, 293.4) where the real
        asset needs (35.5, 592.9) x scale, so the sprite hung roughly 55 px below where its pivot
        said it was and swung out of the well. **The angle law was never wrong**: the prototype's
        `63 - (gr/20)*126` is exactly what this build already computed, which is why the symptom
        read as a pivot fault and not a value fault.

        The needle's source pivot is derived from the prototype's own placement rather than guessed:
        it draws the sprite at (-11.9, -197.6) from the pivot at a drawn size of 23.8 x 202.3, so in
        source pixels the pivot sits at (11.9/23.8 x 71, 197.6/202.3 x 607) = (35.5, 592.9).

        **The meter pivot is 0.5 x the face WIDTH in both axes**, which the prototype states in
        words — "pivot at 0.5 x face width below the top edge" — and which this build already had
        right. It is worth keeping as an expression: on a 159-tall well the y is 198, comfortably
        below the well's own bottom, and a reader checking it against the HEIGHT would move it. */
    inline constexpr float meterFaceSourceW = 1188.0f;
    inline constexpr float meterFaceSourceH = 478.0f;
    inline constexpr float meterScale = meterW / meterFaceSourceW;
    inline constexpr juce::Point<float> meterPivot { meterW * 0.5f, meterW * 0.5f };
    /*  **RE-MEASURED FOR BUNDLE 3, WHICH REDREW THE NEEDLE — and these were corrected against the
        PREVIOUS file less than a day earlier.**

        The needle was 71 x 607 and is now **18 x 525**: a much finer pointer, not a rescale. So the
        constants fixed on 2026-08-18 were already describing a file that no longer exists, which is
        the same defect they were fixing one revision earlier. The face keeps its 1188 x 478
        dimensions and changed content, so its two figures still hold — checking only the one that
        looked suspect would have missed the needle.

        Derived from §4 rather than guessed: the needle draws at 9.55 x 278.4 placed at
        (-4.77, -273.2) from the pivot, so in source pixels the pivot sits at
        (4.77/9.55 x 18, 273.2/278.4 x 525) = **(8.99, 515.2)**. The implied scale, 9.55/18 =
        0.5306, agrees with the face's 630/1188 = 0.5303 — an independent check that the pair is cut
        to one scale. */
    inline constexpr float needleSourceW = 18.0f;
    inline constexpr juce::Point<float> needleSourcePivot { 8.99f, 515.2f };

    /*  **§Meter's glass sheen, taken from the prototype rather than approximated.** It is
        `linear-gradient(118deg, rgba(255,255,255,.10) 0 22%, rgba(255,255,255,0) 40%)` — a band
        along the upper-left that is OUT by 40 % of the gradient line, leaving the rest of the glass
        clear.

        The build had a gradient from the box's top-left corner to (0.42 w, 0.78 h) holding .10 all
        the way to 55 %, which is a different direction and more than twice the reach: a wash over
        the whole face at constant alpha, which reads as flat cream rather than as light. Same class
        as the knob specular that needed the gradient squashed rather than the path — a plausible
        construction giving the wrong falloff.

        CSS measures the angle clockwise from "to top", so 118 deg runs right and slightly down:
        direction (sin 118, -cos 118) = (0.883, 0.469) with y downward. The gradient line's length
        is |w sin A| + |h cos A|, and it is centred on the box, which is what puts its start OUTSIDE
        the top-left corner. */
    constexpr float meterSheenAngleDeg = 118.0f;
    constexpr float meterSheenAlpha = 0.10f;
    constexpr float meterSheenHoldStop = 0.22f, meterSheenOutStop = 0.40f;

    /** angle = 63 - (GR / 20) * 126 degrees, clamped 0..20 dB. 0 dB parks the needle at the far
        RIGHT and it swings LEFT as the compressor works - the scale is gain reduction, not level. */
    inline constexpr float meterAngleAtZero = 63.0f;
    inline constexpr float meterAngleSpan = 126.0f;
    inline constexpr float meterFullScaleDb = 20.0f;

    /** 300 ms VU-style integration, run at 60 fps in the GUI from the DSP's gain-reduction value.
        The panel is marked MOVING COIL - 300 ms BALLISTIC and that should be true, so the needle
        swings and settles like a physical movement rather than tracking the detector instantly. */
    inline constexpr float meterAttackMs = 300.0f;
    inline constexpr float meterReleaseSeconds = 0.30f;
    inline constexpr float meterDamping = 0.55f;
    inline constexpr int   animationHz = 60;

    /** **The readout revert lives in core now - `nf::ReadoutFormat::revertMs`, 900 ms.**

        It was 1200 here. The suite ran 800 / 900 / 1100 / 1200 under three different constant
        names and two mechanisms, and no spec anywhere justified any of them; 900 is what three
        castings already had. `ProgramHeader::readoutFormat()` is where this panel states its
        readout spelling, and the delay comes with it rather than being a separate number here that
        nothing binds to the others.

        Left as a comment rather than deleted silently, because a reader looking for the old
        constant should find out where it went rather than conclude the revert was removed. */

    /** 190px of vertical drag spans the full range, and 760 while Shift is held.

        **Both are suite figures, not this casting's.** Six castings had six drag feels - JUCE's
        untouched 250 in two, plus 200, 180 and 190 - so the same hand got a different response
        from each, which nothing about any casting's identity argues for. Elmer was already on the
        plurality; the Shift fine mode is what it gained, ported from Reflect-84 because a player
        who learns it on one casting expects it on the next. */
    inline constexpr int knobDragPixels = 190;
    inline constexpr int knobFineDragPixels = 760;

    /** **280 degrees, and Elmer is the only casting that is not 270.**

        BRAND.md records this as explicit per-casting freedom rather than an unstated exception -
        the figure is baked into the filmstrips at -140..+140 (see filmstripFrames), so it is a
        property of the artwork and changing it means re-cutting three strips.

        It is declared here because nothing was telling JUCE. With paint() fully overridden the
        default 270 arc never rendered, so the divergence sat invisible in the artwork with no code
        stating it - and anything later reading the Slider's own rotary parameters would have got
        270 and been quietly wrong. KnobFilmstrip's constructor passes it to setRotaryParameters. */
    inline constexpr float knobSweepDegrees = 280.0f;

    /** **How this panel spells the LCD parameter readout.**

        A presentation decision, so it lives with the other presentation constants rather than in
        ProgramHeader - and that placement is load-bearing for the test: ProgramHeader.h reaches the
        processor, whose JucePlugin_* macros exist only in the plugin target, so a test reading the
        format from there cannot link. The test must read the SHIPPING format rather than a copy, or
        it asserts against itself. All six castings state it in the same place for that reason.
    */
    /** This casting's spelling of the readout: `NAME VALUE UNIT`, **no colon**.

        That is the one legitimate divergence in the suite - `design/GUI-SPEC.md` asks for it - and
        stating it here rather than hand-writing the join is what keeps it a choice instead of
        drift. The value is left in the case its parameter authored: a capital S is a different unit
        from a lowercase one, which is reasoning this casting's source had right first and which now
        lives beside the flag in core.

        The revert is core's 900 ms, where this panel used to carry 1200 - four values under three
        constant names across six castings, with no spec justifying any of them. */
    inline nf::ReadoutFormat readoutFormat()
    {
        nf::ReadoutFormat f;
        f.separatorColon = false;
        f.nameCharacterBudget = lcdCharacterBudget;
        return f;
    }
}

//==============================================================================
namespace Paint
{
    inline juce::ColourGradient vertical (juce::Rectangle<float> r, juce::Colour top, juce::Colour bottom)
    {
        return { top, r.getX(), r.getY(), bottom, r.getX(), r.getBottom(), false };
    }

    /** CSS interpolates translucent gradient stops in PREMULTIPLIED space and JUCE does not, so a
        single gradient from a light translucent colour to a dark one carries a mid grey at partial
        alpha and lightens the middle of the box. Draw the two as separate passes instead. */
    inline void translucentVertical (juce::Graphics& g, juce::Rectangle<float> r, float radius,
                                     juce::Colour top, juce::Colour bottom)
    {
        g.setGradientFill (vertical (r, top, top.withAlpha (0.0f)));
        g.fillRoundedRectangle (r, radius);
        g.setGradientFill (vertical (r, bottom.withAlpha (0.0f), bottom));
        g.fillRoundedRectangle (r, radius);
    }
}

} // namespace ElmerTheme
