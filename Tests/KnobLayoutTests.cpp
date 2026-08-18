#include "../Source/GUI/ElmerTheme.h"

#include <juce_gui_basics/juce_gui_basics.h>

/**
    §3's knob layout, and specifically its **two stated exceptions**.

    Both are correct and both look like transcription slips to anyone checking the pitch arithmetic
    later. That is the failure this file exists to prevent, and it is not hypothetical: Chorus-60's
    knob row had three of five stay put while two moved, and the two that moved read as slips
    precisely because the three that did not made the row look uniform.

    **So the exceptions are asserted with their reasons in the failure messages** — a future reader
    who "corrects" either one gets told why it is not a correction.
*/
class KnobLayoutTests final : public juce::UnitTest
{
public:
    KnobLayoutTests() : juce::UnitTest ("Knob layout", "GUI") {}

    static const ElmerTheme::Layout::KnobSpec& byId (const char* id)
    {
        for (const auto& k : ElmerTheme::Layout::knobs)
            if (juce::String (k.paramId) == id)
                return k;
        jassertfalse;
        return ElmerTheme::Layout::knobs[0];
    }

    void runTest() override
    {
        using namespace ElmerTheme::Layout;

        beginTest ("Two diameter classes and ONLY two");
        {
            /*  §3 is explicit that the diameters were never a third value and that position was
                doing it. A third size appearing here is the defect that rule names. */
            std::set<float> sizes;
            for (const auto& k : knobs)
                sizes.insert (k.knobSize);

            for (float d : sizes)
                logMessage ("  diameter in use: " + juce::String (d, 1));

            expectEquals ((int) sizes.size(), 2,
                          "a third knob diameter has appeared — §3 allows Ø76 and Ø56 and says the "
                          "diameters were never a third value");
            expect (sizes.count (knobLarge) == 1 && sizes.count (knobSmall) == 1,
                    "the two classes are no longer 76 and 56");
        }

        beginTest ("EXCEPTION 1 — RATIO's cell is 226, and the CAPS would confirm 217");
        {
            /*  The reason this needs an arm rather than a comment: at the nominal 134 pitch the two
                Ø76 CAPS clear each other by 58 px, so a check on the controls says 217 is fine. What
                collides is the printed scale, which is wider than the control. */
            const auto& thr = byId ("threshold");
            const auto& rat = byId ("ratio");

            const float nominalPitch = 134.0f;
            const float nominal = thr.areaCentre.x + nominalPitch;
            const float capGap = (rat.areaCentre.x - thr.areaCentre.x) - knobLarge;
            const float capGapAtNominal = nominalPitch - knobLarge;

            logMessage ("  RATIO at " + juce::String (rat.areaCentre.x, 0)
                        + ", nominal would be " + juce::String (nominal, 0));
            logMessage ("  cap gap: " + juce::String (capGap, 0) + " px here, "
                        + juce::String (capGapAtNominal, 0) + " px at the nominal pitch");

            expectEquals (rat.areaCentre.x, 226.0f,
                          "RATIO moved off 226. At the nominal 217 the two Ø76 NUMERAL RINGS "
                          "collide — THRESHOLD's widest numeral reaches 148 and RATIO's would "
                          "start at 150. The caps clear at either figure, so checking the controls "
                          "confirms the wrong one");

            expectGreaterThan (capGapAtNominal, 0.0f,
                               "the caps overlap at the nominal pitch, which would make this a "
                               "control-clearance problem rather than a scale one — re-read §3");
        }

        beginTest ("EXCEPTION 2 — MAKEUP is centred at 1174, not on a cell centre");
        {
            /*  §3: a knob alone in its row is placed BY ITS CLASS. Ø76 centres on the column; Ø56
                sits on a cell centre, because a lone Ø56 between the two cell centres reads as a
                third diameter. MAKEUP is the only knob alone in its row. */
            const auto& mk = byId ("makeup");

            expectEquals (mk.knobSize, knobLarge,
                          "MAKEUP is no longer the large class, which changes which placement rule "
                          "applies to it — a lone Ø56 must sit on a cell centre, not be centred");

            expectEquals (mk.areaCentre.x, makeupCentre,
                          "MAKEUP moved off the column centre. On a cell centre at this row height "
                          "its numerals land on IRON's or MIX's legend above it: its numeral band "
                          "is 480-491 against those legends' bottom at 496, and it clears only "
                          "because 1174 sits in the 41 px gap between 1107 and 1241");

            expect (mk.areaCentre.x != cellRightA && mk.areaCentre.x != cellRightB,
                    "MAKEUP has been moved onto a cell centre");

            // The clearance that makes the row height legal, stated as the arithmetic §3 gives.
            logMessage ("  MAKEUP at " + juce::String (mk.areaCentre.x, 0) + ", between cells "
                        + juce::String (cellRightA, 0) + " and " + juce::String (cellRightB, 0)
                        + " (gap " + juce::String (cellRightB - cellRightA, 0) + ")");
        }

        beginTest ("Registration is a Ø76 box for BOTH classes");
        {
            /*  §3.1: unit and legend register on a Ø76 box whatever the knob's own size, so the
                offset is (76 - d) / 2. This panel held the label baseline and missed the PIVOT by
                exactly 10 px in both bands — which is this offset on the small class, and a defect
                that inspecting the labels would have declared clean. */
            expectEquals (knobRegistrationOffset (knobLarge), 0.0f);
            expectEquals (knobRegistrationOffset (knobSmall), 10.0f,
                          "the small class's registration offset is not 10, which is the exact "
                          "amount this panel's Ø56 pivots were out by");

            /*  The property the box actually buys: both classes' legends land on ONE baseline
                relative to the row centre. The offset is measured from the knob's own box top, so
                the absolute figure is `cy - r + legendTop(d)` — and that must not depend on d. */
            const float largeFromRow = knobLegendTopFor (knobLarge) - knobLarge * 0.5f;
            const float smallFromRow = knobLegendTopFor (knobSmall) - knobSmall * 0.5f;

            logMessage ("  legend baseline from row centre: Ø76 " + juce::String (largeFromRow, 1)
                        + ", Ø56 " + juce::String (smallFromRow, 1));

            expectEquals (largeFromRow, smallFromRow,
                          "the two classes' legends no longer land on one baseline, which is the "
                          "whole purpose of registering both on a Ø76 box");

            // Same for the unit line, which shares the registration.
            expectEquals (knobUnitTopFor (knobLarge) - knobLarge * 0.5f,
                          knobUnitTopFor (knobSmall) - knobSmall * 0.5f,
                          "the two classes' unit lines no longer share a baseline");
        }

        beginTest ("The two tick kinds share an INNER end — the property, not the lengths");
        {
            /*  **This asserts the shared inner end rather than the two lengths, and the reason is a
                mistake already made.** A commit called Elmer's tick construction the OPPOSITE of
                Chorus-60's — that casting's ring is `tickInner = r + 8` drawn to
                `tickInner + length`, which is also a shared inner end differing at the outer. The
                two are the same shape and the claim was wrong.

                What is inverted is how the two SPECS express it. Elmer's states the outer length
                and the ink, so the inner end is derived (14 − 9 = 5). Chorus-60's states the inner
                gap and the length, so the outer end is derived (8 + 9 = 17). **Reading one as if it
                were the other puts every tick out by the difference and draws a perfectly plausible
                ring** — it would compile, run, and look like a ring.

                So the lengths are the figures that differ legitimately between castings and tell
                you nothing. The shared inner end is what both constructions have in common, and it
                is what a mis-transcription between them destroys. */
            const float numberedInner = knobNumberedTickLength - knobNumberedTickInk;
            const float plainInner    = knobPlainTickLength - knobPlainTickInk;

            logMessage ("  numbered: inked " + juce::String (numberedInner, 1) + " -> "
                        + juce::String (knobNumberedTickLength, 1));
            logMessage ("  plain:    inked " + juce::String (plainInner, 1) + " -> "
                        + juce::String (knobPlainTickLength, 1));

            expectEquals (numberedInner, plainInner,
                          "the two tick kinds no longer start at one radius. §3.1 gives them a "
                          "shared inner end and different outer ones; if the inner ends have "
                          "diverged, a length or an ink figure has been read from the wrong spec");

            expectEquals (knobTickInnerRadiusGap, numberedInner,
                          "the derived inner gap and the one the lengths imply disagree");

            expectGreaterThan (knobNumberedTickLength, knobPlainTickLength,
                               "a numbered tick is no longer the longer of the two, so the ring "
                               "reads its emphasis backwards");
        }

        beginTest ("The sweep is 280 degrees from -140, not the suite's 270");
        {
            expectEquals (knobSweepSpanDeg, 280.0f);
            expectEquals (knobSweepStartDeg, -140.0f);
            expectEquals (knobSweepStartDeg + knobSweepSpanDeg, 140.0f,
                          "the sweep is no longer symmetric about twelve o'clock");
        }
    }
};

static KnobLayoutTests knobLayoutTests;
