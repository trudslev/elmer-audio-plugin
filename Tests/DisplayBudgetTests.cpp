#include "DSP/FactoryPrograms.h"
#include "DSP/ProgramManager.h"
#include "GUI/ElmerTheme.h"

#include <juce_audio_processors/juce_audio_processors.h>

/**
    The PROGRAM display's character budget, asserted rather than trusted.

    The cap on typed names is `Layout::maxUserNameLength` and it is **not inferable from the
    layout** - it is the 24-character budget minus the two the dirty marker takes. (It said 19 here
    for a cap of 22 until 2026-08-12: the assertions below were written against the constant, so
    they passed throughout and only the prose lied. That is exactly why a green build preserves this
    class of error - name the constant, do not restate its value.)
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

        constexpr int indexPrefix = 3;      // "01 ", FACTORY Programs only
        constexpr int dirtyMarker = 2;      // " *"
        constexpr int cursorCell  = 1;      // the naming field's block cursor

        // **Three cases, because the display has three shapes.** The single
        // prefix + cap + marker == budget identity this replaced encoded the old model where USER
        // names carried the "NN " prefix too; with the prefix gone it would read 3 + 22 + 2 = 27
        // against a 24-character cell and fail. Each case is still a relationship between
        // independent constants, which is what catches one moving without the other.

        beginTest ("Factory: the longest name plus its NN prefix and the marker fits");
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

        beginTest ("User: a maximum-length name fits, with NO prefix");
        expect (maxUserNameLength + dirtyMarker <= lcdCharacterBudget);

        beginTest ("Naming: a maximum-length name plus the cursor fits");
        expect (maxUserNameLength + cursorCell <= lcdCharacterBudget);

        beginTest ("The cap is the budget less whichever of the marker and the cursor is larger");
        expectEquals (maxUserNameLength, lcdCharacterBudget - juce::jmax (dirtyMarker, cursorCell));

        // **The store's copy must equal the display's, and this is the binding.** ProgramManager
        // enforces the cap on every save path - not only on the keystrokes ProgramHeader filters -
        // but it cannot include a GUI header to alias this constant, so the two are pinned here
        // instead. A test that fails on divergence is stronger than a comment that does not.
        expectEquals (ProgramManager::maxProgramNameLength, maxUserNameLength);

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
