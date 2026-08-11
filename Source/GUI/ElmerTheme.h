#pragma once

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <BinaryData.h>

#include <array>

/**
    Elmer's design tokens: every colour, coordinate, size and typographic constant.

    All coordinates are ABSOLUTE against the 1120 x 776 canvas. The prototype expresses layout as
    CSS flexbox, which cannot be transcribed directly, so the nested boxes were resolved by hand and
    then checked against `design/screenshots/panel.png`: all eight knob centres, the header divider
    and the top row's origin land on the measured render exactly.

    The panel is bitmap-composited - the fascia is drawn in code and the production assets sit on
    top. There is deliberately no dressed-panel render in BinaryData: Gatecrasher used one as its
    background and every live control ended up sitting over a baked copy of itself.
*/
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
    // contrast: 7.9-8.9:1 vs fascia [functional]
    inline const juce::Colour ink            { 0xFF0F0F0C };
    inline const juce::Colour wordmarkInk    { 0xFF24231F };
    inline const juce::Colour markerInk      { 0xFF2B2A26 };

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

    // --- buttons -------------------------------------------------------------
    inline const juce::Colour creamTop       { 0xFFF0E9D3 };
    inline const juce::Colour creamBottom    { 0xFFD6CDB2 };
    inline const juce::Colour creamText      { 0xFF302C24 };
    inline const juce::Colour creamOffTop    { 0xFFA5A094 };
    inline const juce::Colour creamOffBottom { 0xFF8F8A7E };
    inline const juce::Colour creamOffText   { 0xFF6F6A5F };

    inline const juce::Colour lampFaceTop    { 0xFFA9A496 };
    inline const juce::Colour lampFaceBottom { 0xFF8E8A7D };
    inline const juce::Colour lampLegendOff  { 0xFF1D1C17 };
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
            BinaryData::ArchivoBlackRegular_ttf, (size_t) BinaryData::ArchivoBlackRegular_ttfSize);
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
    inline constexpr float canvasWidth  = 1120.0f;
    inline constexpr float canvasHeight = 776.0f;
    inline constexpr float cornerRadius = 5.0f;

    inline constexpr float railWidth = 15.0f;
    inline constexpr float screwDiameter = 6.0f;
    inline constexpr std::array<juce::Point<float>, 4> screwCentres { {
        { 8.0f, 11.0f }, { 1112.0f, 11.0f }, { 8.0f, 765.0f }, { 1112.0f, 765.0f } } };

    // Content inset: 22 left/right, 20 top/bottom.
    inline constexpr float contentX = 22.0f;
    inline constexpr float contentY = 20.0f;
    inline constexpr float contentRight = 1098.0f;
    inline constexpr float contentBottom = 756.0f;
    inline constexpr float contentWidth = contentRight - contentX;   // 1076

    // --- header (112 tall, 13px right padding) --------------------------------
    inline constexpr float headerHeight = 112.0f;
    inline constexpr float headerBottom = contentY + headerHeight;   // 132

    inline constexpr float nameplateX = 37.0f;      // contentX + 15px column padding
    inline constexpr float nameplateY = 22.0f;      // contentY + 2px column padding
    inline constexpr float wordmarkSize = 53.0f;
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
        y 47..54 there, so the 12px box that centres it starts at 44.5. The captions used to sit at
        contentY, which was right for a row that began at 37 and is 24px too high for one that
        begins at 61. */
    inline constexpr float captionY = 45.0f;

    /** **The header row: 30px tall, at y 61.** Every element in it - display, SAVE, DELETE, IN and
        OUT - is the same 30px height and shares this Y, so the band reads as one instrument; the
        display is not allowed to be the odd one out.

        61 is not a free choice: the row is centred against the FULL 112px header block, which
        starts at the 20px content inset, so (112 - 30) / 2 + 20 = 61. Centring on the wordmark
        plate instead - which is what the previous 37 amounted to - left the display sitting high.

        The display was 364 x 38 and held 21 characters. At 361 x 30 with 14px type it holds 24: the
        weight comes off and three characters come back. */
    inline constexpr float lcdRowY = 61.0f;
    inline constexpr float lcdRowH = 30.0f;
    inline constexpr float programX = 403.0f;

    /** 361, not the 359 the cell widths alone give: 56 + 269 + 28 is 353, plus TWO 1px hairlines
        between the three cells, plus the 3px frame on each side. Measured 403..764 in the reference
        render, which agrees exactly. */
    inline constexpr float programW = 361.0f;

    /** The display's three cells, inside the 3px metal frame. Bank 56 + name 269 + chevron 28.
        The name cell's 11px horizontal padding leaves 247px of text, which at IBM Plex Mono 14px
        with 1.7px tracking (10.1px per character) is the 24-character budget the spec cites. */
    inline constexpr float lcdFrameThickness = 3.0f;
    inline constexpr float lcdBankCellW = 56.0f;
    inline constexpr float lcdNameCellW = 269.0f;
    inline constexpr float lcdChevronCellW = 28.0f;

    /** The chevron, drawn as a stroked path in an 11 x 7 box so it renders identically whatever the
        platform's font fallback does - and because it FLIPS with menu state, which a baked glyph
        could not. */
    inline constexpr float chevronW = 11.0f;
    inline constexpr float chevronH = 7.0f;
    inline constexpr float chevronStroke = 1.6f;

    /** The menu hangs from the GLASS's lower edge, not the frame's outside: 30px display less two
        3px frame edges is 24px of glass, plus 4px to clear its inner shadow. */
    inline constexpr float menuTopOffset = 28.0f;

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

    inline constexpr float saveX = 771.0f;
    inline constexpr float deleteX = 840.0f;
    inline constexpr float headerButtonW = 62.0f;
    inline constexpr float buttonTextSize = 10.0f;
    inline constexpr float buttonTracking = 2.0f;

    inline constexpr float meterInX = 928.0f;
    inline constexpr float meterOutX = 1011.0f;
    inline constexpr float levelBoxW = 74.0f;
    inline constexpr float levelTextSize = 14.0f;

    // --- divider --------------------------------------------------------------
    inline constexpr float dividerY = 140.0f;       // headerBottom + 8

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
    inline constexpr float meterX = 446.0f;
    inline constexpr float meterW = 588.0f;
    inline constexpr float meterH = 236.0f;
    inline constexpr float meterY = 213.0f;
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

    // --- footer ---------------------------------------------------------------
    inline constexpr float footerY = 725.0f;
    inline constexpr float footerTextSize = 11.5f;
    inline constexpr float footerTracking = 1.8f;
    inline constexpr float scribbleSize = 21.0f;
    inline constexpr float scribbleRotationDegrees = -2.4f;

    /** The tape's padding, from the prototype's `padding: 9px 30px 11px 32px` - **asymmetric on
        both axes**, which is why these are four numbers rather than a reduced() call. Together with
        the 0.5px letter-spacing below they are what sizes the tape: the strip was 29.5px narrower
        than the render because it used 42px of total horizontal padding against the specified 62,
        and drew the marker text with no tracking at all. */
    inline constexpr float scribblePadLeft   = 32.0f;
    inline constexpr float scribblePadRight  = 30.0f;
    inline constexpr float scribblePadTop    = 9.0f;
    inline constexpr float scribblePadBottom = 11.0f;
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
    inline const std::array<KnobSpec, 8> knobs { {
        { "threshold",   "THRESHOLD",         { 120.0f, 253.0f }, 112.0f, 108.0f, 84.0f,
          Strip::detect, Ring::large11, 324.0f },
        // **RATIO sits 6px lower than THRESHOLD, and its LABEL does not.** Its column carries
        // padding-top: 12px so that a 100px scale area ends level with THRESHOLD's 112px one:
        // 197 + 12 + 100 = 309 against 197 + 112 = 309, and both labels land on 324. Control labels
        // align across a section regardless of ring size, so the smaller ring is pushed down rather
        // than the label pulled up. Measured in panel.png: cap centre 259, against THRESHOLD's 253.
        { "ratio",       "RATIO",             { 269.0f, 259.0f }, 100.0f,  96.0f, 74.0f,
          Strip::detect, Ring::small9,  324.0f },
        { "sidechainHp", "SIDECHAIN HP",      { 120.0f, 412.0f }, 100.0f,  96.0f, 74.0f,
          Strip::detect, Ring::large11, 477.0f },
        { "attack",      "ATTACK",            { 115.0f, 606.0f }, 100.0f,  96.0f, 74.0f,
          Strip::timing, Ring::large11, 671.0f },
        { "release",     "RELEASE",           { 269.0f, 606.0f }, 100.0f,  96.0f, 74.0f,
          Strip::timing, Ring::five,    671.0f },
        { "iron",        "IRON",              { 560.0f, 606.0f }, 100.0f,  96.0f, 74.0f,
          Strip::output, Ring::small9,  671.0f },
        { "makeup",      "MAKEUP",            { 851.0f, 606.0f }, 100.0f,  96.0f, 74.0f,
          Strip::output, Ring::small9,  671.0f },
        { "mix",         "MIX",               { 1005.0f, 606.0f }, 100.0f, 96.0f, 74.0f,
          Strip::output, Ring::small9,  671.0f } } };

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
    inline constexpr float meterFaceSourceW = 1000.0f;
    inline constexpr float meterScale = meterW / meterFaceSourceW;   // 0.588
    inline constexpr juce::Point<float> meterPivot { 294.0f, 294.0f };
    inline constexpr float needleSourceW = 60.0f;
    inline constexpr juce::Point<float> needleSourcePivot { 30.0f, 499.0f };

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

    /** The LCD shows a parameter's name and value while it is being moved, then reverts to the
        program name after this long. The design chose this over a floating tooltip deliberately -
        it reuses a display already on the panel, and a tooltip has no hardware equivalent. */
    inline constexpr int lcdRevertMs = 1200;

    /** 190px of vertical drag spans the full range, per the prototype. */
    inline constexpr int knobDragPixels = 190;
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
