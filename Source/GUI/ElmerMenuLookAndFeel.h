#pragma once

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
class ElmerMenuLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    ElmerMenuLookAndFeel()
    {
        // The few places JUCE paints without asking us first, most visibly the shadow's backdrop.
        setColour (juce::PopupMenu::backgroundColourId, ground);
        setColour (juce::PopupMenu::textColourId, rowInk);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, phosphor.withAlpha (0.13f));
        setColour (juce::PopupMenu::highlightedTextColourId, rowInkBright);
    }

    juce::Font getPopupMenuFont() override
    {
        return ElmerTheme::Font::mono (itemTextSize);
    }

    /** 4px above the first row and below the last, per the spec's `padding: 4px 0`. */
    int getPopupMenuBorderSize() override { return verticalPadding; }

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

    void getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator,
                                    int standardMenuItemHeight, int& idealWidth,
                                    int& idealHeight) override
    {
        if (isSeparator)
        {
            idealWidth = 50;
            idealHeight = separatorHeight;
            return;
        }

        idealWidth = (int) std::ceil (ElmerTheme::Text::trackedWidth (text, getPopupMenuFont(), tracking))
                     + padLeft + padRight;

        // The row height is the spec's 22 and is NOT allowed to grow to the platform's standard
        // item height: seventeen rows have to fit the panel without scrolling, and JUCE's standard
        // on macOS is taller than 22.
        juce::ignoreUnused (standardMenuItemHeight);
        idealHeight = rowHeight;
    }

    /** **19px, not JUCE's default.** LookAndFeel_V2 sizes a section header at the item height plus
        half again - 33 against our 22 - which pushed everything below FACTORY 14px down the list and
        put the whole bank out of step with the render. The spec's header is `padding: 3px 12px 4px`
        around a 9px line: 3 + 12 + 4.

        Overridden on the WithOptions form because that is the one V2 actually calls; the older
        two-argument variant delegates to it. */
    void getIdealPopupMenuSectionHeaderSizeWithOptions (const juce::String& text,
                                                        int standardMenuItemHeight,
                                                        int& idealWidth, int& idealHeight,
                                                        const juce::PopupMenu::Options& options) override
    {
        juce::ignoreUnused (standardMenuItemHeight, options);

        idealWidth = (int) std::ceil (ElmerTheme::Text::trackedWidth (
                         text, ElmerTheme::Font::mono (headerTextSize), headerTracking))
                     + padLeft + padRight;
        idealHeight = headerHeight;
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
        ElmerTheme::Text::drawTracked (g, sectionName.toUpperCase(),
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
    static constexpr int   rowHeight       = 22;
    static constexpr int   headerHeight     = 19;   // 3px top padding + a 9px line + 4px bottom
    static constexpr int   separatorHeight = 9;
    static constexpr int   verticalPadding = 4;

    // 10px of padding on the left, of which the current row spends 2 on its marker bar - so the
    // text never moves between a selected row and its neighbours.
    static constexpr int   padLeft  = 10;
    static constexpr int   padRight = 12;
};
