#include "DSP/FactoryPrograms.h"
#include "GUI/ElmerTheme.h"

#include <juce_audio_processors/juce_audio_processors.h>

/**
    The PROGRAM display's character budget, asserted rather than trusted.

    The cap on typed names is 19 and it is **not inferable from the layout** - it is 24 minus the
    three characters the two-digit index and its space take, minus the two the dirty marker takes.
    Nothing in the drawing code would fail if the cap drifted: the text would simply run past the
    cell's right edge and be clipped, silently, and only for the longest names in the longest state.

    So this pins all four numbers to each other and to the widest thing that can actually appear.
    Note it deliberately measures the LONGEST FACTORY NAME from the bank rather than a literal, so
    adding a longer Program fails here instead of on someone's panel.
*/
class DisplayBudgetTests final : public juce::UnitTest
{
public:
    DisplayBudgetTests() : juce::UnitTest ("PROGRAM display budget", "GUI") {}

    void runTest() override
    {
        using namespace ElmerTheme::Layout;

        beginTest ("The cap, the index and the marker add up to the budget");
        constexpr int indexPrefix = 3;      // "01 "
        constexpr int dirtyMarker = 2;      // " *"
        expectEquals (indexPrefix + maxUserNameLength + dirtyMarker, lcdCharacterBudget);

        beginTest ("The longest factory Program fits, marker included");
        int longest = 0;
        juce::String longestName;

        for (const auto& fp : Elmer::factoryPrograms)
            if (const juce::String n { fp.name }; n.length() > longest)
            {
                longest = n.length();
                longestName = n;
            }

        expect (indexPrefix + longest + dirtyMarker <= lcdCharacterBudget,
                "\"" + longestName + "\" overruns the name cell once the dirty marker appears");

        beginTest ("A maximum-length user name fits, marker included");
        expect (indexPrefix + maxUserNameLength + dirtyMarker <= lcdCharacterBudget);

        beginTest ("The budget matches the cell the type is actually drawn into");
        // 269px of cell less 2 x 11px padding is 247px of text; 14px IBM Plex Mono at 1.7px
        // tracking advances 10.1px per character. If any of those four change without the budget
        // changing, what can be typed stops matching what can be shown.
        constexpr float advance = 10.1f;
        const float textWidth = lcdNameCellW - 2.0f * lcdNamePadX;
        expect ((int) (textWidth / advance) >= lcdCharacterBudget,
                "the name cell no longer holds " + juce::String (lcdCharacterBudget) + " characters");

        beginTest ("Every cell width, plus its hairlines and frame, is the display's width");
        expectWithinAbsoluteError (2.0f * lcdFrameThickness + lcdBankCellW + lcdCellHairline
                                       + lcdNameCellW + lcdCellHairline + lcdChevronCellW,
                                   programW, 0.01f);

        beginTest ("The header row is centred against the full 112px header block");
        expectWithinAbsoluteError (lcdRowY, contentY + (headerHeight - lcdRowH) * 0.5f, 0.01f);
    }
};

static DisplayBudgetTests displayBudgetTests;
