#pragma once

#include <nf/MenuMetrics.h>

#include "ElmerTheme.h"

/**
    Dresses the Program dropdown as an extension of the PROGRAM glass.

    The menu is the one part of this panel that cannot be baked: its height depends on how many User
    Programs exist. Left to JUCE's default it renders as a system-grey list belonging to a different
    product entirely - the glass, the phosphor and the mono face all stopping dead at the edge of the
    display, with the list hanging off it like a dialog.

    So everything here is the handoff's dropdown treatment: ground `#16150f`, rows in `#b9ae86`,
    IBM Plex Mono at 12px / 1.4px tracking, and a border that deliberately omits its TOP edge so the
    list meets the glass with no rule between them.

    **The current Program is marked with a 2px left bar, not a tick glyph.** That is the spec's
    choice and it is a good one: a bar reads at a glance straight down the column, and it costs no
    character cell, so every row starts its text at the same x whether it is current or not - the
    bar occupies the 2px that the unselected rows spend on padding.
*/
class ElmerMenuLookAndFeel final : public nf::MenuMetricsLookAndFeel
{
public:
    ElmerMenuLookAndFeel()
        : nf::MenuMetricsLookAndFeel (metrics())
    {
        // The few places JUCE paints without asking us first, most visibly the shadow's backdrop.
        setColour (juce::PopupMenu::backgroundColourId, ground);
        setColour (juce::PopupMenu::textColourId, rowInk);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, phosphor.withAlpha (0.13f));
        setColour (juce::PopupMenu::highlightedTextColourId, rowInkBright);
    }

protected:
    /** **Tracked, not plain.** This panel draws its rows with letter-spacing, so measuring them as
        a plain run would size every row too narrow for its own content. That is the reason text
        measurement is the one hook nf::MenuMetricsLookAndFeel leaves to the casting. */
    float measureMenuItemText (const juce::String& text) override
    {
        return ElmerTheme::Text::trackedWidth (text, getPopupMenuFont(), tracking);
    }

    /** The caption is a smaller face than the rows, so it is measured in its own. */
    float measureSectionHeaderText (const juce::String& text) override
    {
        return ElmerTheme::Text::trackedWidth (text, ElmerTheme::Font::mono (headerTextSize),
                                               headerTracking);
    }

public:
    juce::Font getPopupMenuFont() override
    {
        return ElmerTheme::Font::mono (itemTextSize);
    }

    void drawPopupMenuBackground (juce::Graphics& g, int width, int height) override
    {
        const juce::Rectangle<float> r (0.0f, 0.0f, (float) width, (float) height);

        g.setColour (ground);
        g.fillRoundedRectangle (r, cornerRadius);

        // Left, right and bottom only. **No top border**: the list hangs flush from the glass's
        // lower edge, and a rule along that join would draw a seam exactly where the design wants
        // the two to read as one instrument. Drawn as three strokes rather than a rounded rectangle
        // because a rounded rectangle cannot omit an edge.
        g.setColour (border);
        g.fillRect (r.getX(), r.getY(), 1.0f, r.getHeight() - cornerRadius);
        g.fillRect (r.getRight() - 1.0f, r.getY(), 1.0f, r.getHeight() - cornerRadius);

        juce::Path bottom;
        bottom.startNewSubPath (r.getX() + 0.5f, r.getBottom() - cornerRadius);
        bottom.quadraticTo (r.getX() + 0.5f, r.getBottom() - 0.5f,
                            r.getX() + cornerRadius, r.getBottom() - 0.5f);
        bottom.lineTo (r.getRight() - cornerRadius, r.getBottom() - 0.5f);
        bottom.quadraticTo (r.getRight() - 0.5f, r.getBottom() - 0.5f,
                            r.getRight() - 0.5f, r.getBottom() - cornerRadius);
        g.strokePath (bottom, juce::PathStrokeType (1.0f));
    }



    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                            bool isSeparator, bool isActive, bool isHighlighted,
                            bool isTicked, bool hasSubMenu, const juce::String& text,
                            const juce::String& shortcutKeyText,
                            const juce::Drawable* icon,
                            const juce::Colour* textColourToUse) override
    {
        juce::ignoreUnused (hasSubMenu, shortcutKeyText, icon, textColourToUse);

        const auto r = area.toFloat();

        if (isSeparator)
        {
            g.setColour (divider);
            g.fillRect (r.reduced (dividerInset, 0.0f).withHeight (1.0f).withY (r.getCentreY()));
            return;
        }

        if (isTicked)
        {
            g.setColour (phosphor.withAlpha (0.10f));
            g.fillRect (r);

            g.setColour (ElmerTheme::Colour::phosphor);
            g.fillRect (r.getX(), r.getY(), markerWidth, r.getHeight());
        }

        if (isHighlighted && isActive)
        {
            g.setColour (phosphor.withAlpha (0.13f));
            g.fillRect (r);

            // Repainted over the hover wash so the current Program stays marked while pointed at.
            if (isTicked)
            {
                g.setColour (ElmerTheme::Colour::phosphor);
                g.fillRect (r.getX(), r.getY(), markerWidth, r.getHeight());
            }
        }

        auto ink = isActive ? rowInk : rowInk.withAlpha (0.45f);

        if ((isHighlighted || isTicked) && isActive)
            ink = rowInkBright;

        const auto textArea = r.withTrimmedLeft ((float) padLeft).withTrimmedRight ((float) padRight);

        // The current row carries the display's own phosphor glow, drawn as a soft pass beneath.
        if (isTicked && isActive)
            ElmerTheme::Text::drawTracked (g, text, getPopupMenuFont(), tracking, textArea,
                                           juce::Justification::left,
                                           ElmerTheme::Colour::phosphor.withAlpha (0.22f), false);

        ElmerTheme::Text::drawTracked (g, text, getPopupMenuFont(), tracking, textArea,
                                       juce::Justification::left, ink, false);
    }

    void drawPopupMenuSectionHeader (juce::Graphics& g, const juce::Rectangle<int>& area,
                                     const juce::String& sectionName) override
    {
        // **Drawn as authored, never re-cased.** The caption is authored FACTORY/USER at the
        // addSectionHeader call in ProgramHeader.cpp, per BRAND.md's rule that case belongs at
        // the source. It held a .toUpperCase() here until 2026-08-13. Nothing else reads these
        // two strings today, which is a fact about today rather than a property of the code:
        // the moment a caption comes from data, the site that re-cases it is the site that
        // gets it wrong. Re-arguing the exception each time costs more than the rule.
        ElmerTheme::Text::drawTracked (g, sectionName,
                                       ElmerTheme::Font::mono (headerTextSize), headerTracking,
                                       area.toFloat().withTrimmedLeft ((float) padLeft),
                                       juce::Justification::left, headerInk, false);
    }

private:
    const juce::Colour ground       { 0xFF16150F };
    const juce::Colour rowInk       { 0xFFB9AE86 };
    const juce::Colour rowInkBright { 0xFFF2E9C4 };
    const juce::Colour headerInk    { 0xFF8A8163 };
    const juce::Colour phosphor     { 0xFFD6C47C };   // the tint every wash and rule is mixed from
    const juce::Colour border       { phosphor.withAlpha (0.30f) };
    const juce::Colour divider      { phosphor.withAlpha (0.24f) };

    static constexpr float itemTextSize   = 12.0f;
    static constexpr float tracking       = 1.4f;
    static constexpr float headerTextSize = 9.0f;
    static constexpr float headerTracking = 2.6f;
    static constexpr float cornerRadius   = 3.0f;
    static constexpr float markerWidth    = 2.0f;
    static constexpr float dividerInset   = 10.0f;
    /** **This panel's dropdown sizes, and Elmer is the casting that chose all of them.**

        `sectionHeaderHeight` is **19, not JUCE's default**. `LookAndFeel_V2` sizes a section caption
        at the item height plus half again - 33 against our 22 - which pushed everything below
        FACTORY 14px down the list and put the whole bank out of step with the render. The spec's
        header is `padding: 3px 12px 4px` around a 9px line: 3 + 12 + 4.

        The four castings that took JUCE's default now state it explicitly too, so the suite's
        disagreement is visible rather than implicit. This is the side with a measurement behind it.

        The row height is 22 and never grows to the platform's standard item height: seventeen rows
        have to fit the panel without scrolling, and macOS's standard is taller than 22. That rule
        is nf::MenuMetricsLookAndFeel's now, and it started here. */
public:
    /** Test seams. The caption's size and the published metrics, so `MenuMetricsTests` measures
        what this class actually uses rather than rebuilding its own copy — which is how Chorus-60's
        equivalent came to agree with a defect for a revision. */
    static constexpr float captionCssPx() { return headerTextSize; }
    static nf::MenuMetrics publishedMetrics() { return metrics(); }

private:
    static nf::MenuMetrics metrics()
    {
        nf::MenuMetrics m;
        m.rowHeight = rowHeight;
        m.sectionHeaderHeight = headerHeight;
        m.separatorHeight = separatorHeight;
        m.borderSize = verticalPadding;      // 4px above the first row and below the last
        m.leadingColumn = 0;                 // the current row is marked by a bar in its padding
        m.horizontalPadding = padLeft + padRight;
        return m;
    }

    static constexpr int   rowHeight       = 22;

    /*  **19, AND THE ARITHMETIC THIS COMMENT USED TO STATE WAS WRONG WHILE THE ANSWER WAS RIGHT.**

        It read "3px top padding + a 9px line + 4px bottom", citing the spec's
        `padding: 3px 12px 4px`, and summed **3 + 12 + 4**. In that CSS shorthand the 12 is the
        LEFT AND RIGHT padding — the horizontal axis. The vertical sum is 3 + <line box> + 4, and
        the prototype declares no `line-height` at all, so the middle term is IBM Plex Mono's own
        natural line box at 9 CSS px: **11.7**.

        3 + 11.7 + 4 = 18.7, which rounds to 19. **The figure is correct and its stated derivation
        was a coincidence** — it took a number from the wrong axis that happens to sit 0.3 px from
        the right one.

        That matters more here than it would anywhere else, because **this casting is where the
        suite's caption figure came from**: five other castings were ruled against Elmer's 19. A
        wrong derivation under a right number is the shape that survives review, and it survived
        here with no test in the repo the number came from. `Tests/MenuMetricsTests.cpp` is that
        test now, and it asserts the CONSTRUCTION rather than the literal.

        **The type itself is genuinely CSS px** — `ElmerTheme::Font::of` builds through
        `withPointHeight` — so the base figure is sound and nothing inherited an error. That was the
        open question and it is answered by measurement, not by reading this comment. */
    static constexpr int   headerHeight     = 19;
    static constexpr int   separatorHeight = 9;
    static constexpr int   verticalPadding = 4;

    // 10px of padding on the left, of which the current row spends 2 on its marker bar - so the
    // text never moves between a selected row and its neighbours.
    static constexpr int   padLeft  = 10;
    static constexpr int   padRight = 12;
};
