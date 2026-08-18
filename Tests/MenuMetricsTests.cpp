#include "../Source/GUI/ElmerMenuLookAndFeel.h"
#include "../Source/GUI/ElmerTheme.h"

#include <nf/MenuMetrics.h>

#include <juce_gui_basics/juce_gui_basics.h>

/**
    The Program list's caption — **and this casting is where the suite's figure came from.**

    Elmer is the only casting whose caption height was designer-authored rather than inherited from
    JUCE's `rowHeight + rowHeight / 2`, so its **19** is the measurement the other five were ruled
    against. A base figure with no test in the repo it came from is exactly the shape this suite
    keeps finding, and this file is that test.

    **The question it was written to answer**, raised after Chorus-60's type pass found three menu
    sites passing a CSS px straight to a font builder that wanted a JUCE height — its caption
    rendering at 9.76 effective px, under the functional floor: *was Elmer's 19 measured as CSS px,
    or as a JUCE height?* If the latter, the ruling's base is wrong and every casting that adopted
    it inherited the error, the same way the catalogue's rotation fractions turned out to be derived
    output rather than authored input.

    It was **CSS px**, and that is asserted here rather than read: `ElmerTheme::Font::of` builds
    through `withPointHeight`, and the arms below measure the line box that produces.
*/
class ElmerMenuMetricsTests final : public juce::UnitTest
{
public:
    ElmerMenuMetricsTests() : juce::UnitTest ("Menu metrics", "GUI") {}

    void runTest() override
    {
        const auto captionFont = ElmerTheme::Font::mono (ElmerMenuLookAndFeel::captionCssPx());
        const auto m = ElmerMenuLookAndFeel::publishedMetrics();

        beginTest ("The caption type is CSS px, not a JUCE height — the ruling's base figure");
        {
            /*  A CSS `font-size` is an **em** size; `juce::Font::withHeight` sets ascent + descent,
                a typeface-specific multiple of it. So the two constructions differ by the face's own
                line-box ratio, and the tell is which side of that ratio the measured height sits on.

                IBM Plex Mono's line box is ~1.30 em. Built as CSS px, a nominal 9 measures **~11.7**;
                built as a JUCE height it would measure **9.0** and the glyphs would render at 6.9
                effective px — well under BRAND.md's ~9-10 functional floor, which is precisely what
                Chorus-60's dropdown was doing. */
            const float nominal = ElmerMenuLookAndFeel::captionCssPx();
            const float measured = captionFont.getHeight();
            const float ratio = measured / nominal;

            logMessage ("  caption nominal " + juce::String (nominal, 1) + " CSS px -> line box "
                        + juce::String (measured, 3) + " px  (ratio " + juce::String (ratio, 4) + ")");

            expectGreaterThan (ratio, 1.15f,
                               "the caption's line box is at or below its nominal size, which is "
                               "what a CSS px passed to withHeight() produces — the type is "
                               "rendering small and the suite's 19 was measured against it");
            expectLessThan (ratio, 1.45f,
                            "the line box is far larger than IBM Plex Mono's ~1.30 em, so this is "
                            "not the face the figure was measured on");
        }

        beginTest ("19 is the CONSTRUCTION's answer, not a literal that happens to agree");
        {
            /*  **This is the arm the casting did not have, and the reason it matters here more than
                anywhere else.** `headerHeight` is a literal 19; `nf::captionHeight` computes padding
                plus the font's own measured line box. While they agree, nothing can tell a
                designer-authored figure from a coincidence — and this figure is the one five other
                castings were ruled against.

                Elmer's spec authors the caption as `padding: 3px 12px 4px` at `font-size: 9px` with
                **no line-height**, so the middle term is the face's natural line box rather than a
                declared one. */
            const int constructed = nf::captionHeight (captionFont, 3, 4);

            logMessage ("  3 + " + juce::String (captionFont.getHeight(), 3) + " + 4 = "
                        + juce::String (constructed) + "   literal " + juce::String (m.sectionHeaderHeight));

            expectEquals (m.sectionHeaderHeight, constructed,
                          "the published caption height and the one its own font produces have "
                          "diverged — five castings were ruled against this figure");
        }

        beginTest ("The caption is SHORTER than a row, which is the ruling");
        {
            // JUCE's default is rowHeight + rowHeight / 2 — a caption half again taller than a row.
            // Elmer is the casting that chose otherwise and the reason the suite pins it.
            logMessage ("  caption " + juce::String (m.sectionHeaderHeight)
                        + "px, row " + juce::String (m.rowHeight) + "px");

            expectLessThan (m.sectionHeaderHeight, m.rowHeight,
                            "the caption is at least as tall as a row, which is JUCE's default and "
                            "the thing this casting's spec rejected");
        }
    }
};

static ElmerMenuMetricsTests elmerMenuMetricsTests;
