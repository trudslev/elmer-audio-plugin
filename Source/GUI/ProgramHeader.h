#pragma once

#include "ElmerMenuLookAndFeel.h"
#include "ElmerTheme.h"
#include "../DSP/ProgramManager.h"

/**
    The PROGRAM display, SAVE / DELETE, and the numeric IN / OUT readouts.

    The LCD does double duty. Normally it shows the bank and the program name. While a control is
    being moved it is TAKEN OVER by that parameter's name and value - `THRESHOLD -18.5 dB`,
    `RELEASE AUTO`, `KNEE SOFT` - reverting 1200 ms after release.

    That is the design's deliberate answer to "where do live values go", chosen over a floating
    tooltip because it reuses a display already on the panel and a tooltip has no hardware
    equivalent. It also satisfies BRAND.md's stronger rule that dynamic text belongs inside a screen
    in that screen's own typeface, which a tooltip floating over the fascia does not.

    The FACT/USER indicator is ONE field that switches its text, never two labels with one greyed.
*/
class ProgramHeader final : public juce::Component,
                            private juce::Timer
{
public:
    ProgramHeader (juce::AudioProcessorValueTreeState&, ProgramManager&);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    bool keyPressed (const juce::KeyPress&) override;
    void focusLost (FocusChangeType) override;

    /** Takes the display over with a parameter's name and value. */
    void showParameter (const juce::String& paramId);
    /** Starts the 1200 ms countdown back to the program name. */
    void releaseParameter();

    /** The component the Program list lays out inside; its bounds fix the list's top edge and stop
        it outgrowing the panel. See ../../CLAUDE.md, "The Program dropdown". */
    void setMenuParent (juce::Component* parent) noexcept { menuParent = parent; }

    static int menuAnchorY() noexcept
    {
        // From the GLASS's top, not the frame's: 24px of glass plus 4px to clear its inner shadow.
        return (int) std::floor (ElmerTheme::Layout::lcdRowY + ElmerTheme::Layout::lcdFrameThickness
                                     + ElmerTheme::Layout::menuTopOffset);
    }

    /** NOT the anchor: JUCE clamps a menu to jmax(parentArea.getY() + 1, ...), so a host starting
        exactly at the anchor can only open one pixel below it. */
    static int menuHostTop() noexcept { return menuAnchorY() - 8; }

    void setLevels (float inDb, float outDb);
    void setGainReductionDb (float db);
    void refresh() { repaint(); }

private:
    void timerCallback() override;
    juce::String currentLcdText() const;
    juce::String describeParameter (const juce::String& paramId) const;
    juce::Rectangle<float> saveBounds() const;
    juce::Rectangle<float> deleteBounds() const;
    juce::Rectangle<float> displayBounds() const;
    juce::Rectangle<float> glassBounds() const;
    juce::Rectangle<float> bankCellBounds() const;
    juce::Rectangle<float> nameCellBounds() const;
    juce::Rectangle<float> chevronCellBounds() const;
    void showProgramMenu();
    void paintChevron (juce::Graphics&) const;

    void enterNamingMode();
    void commitNaming();
    void cancelNaming();
    bool saveEnabled() const;
    bool deleteEnabled() const;

    ElmerMenuLookAndFeel menuLookAndFeel;
    juce::Component* menuParent = nullptr;
    bool menuOpen = false;

    // Naming state. typedName is never written back to the Program until commit, and cancelling
    // simply leaves the mode - which is what makes Cancel free: nothing to undo, and any knob the
    // user tweaked before pressing SAVE is still exactly where they left it.
    bool namingMode = false;
    juce::String typedName;

    /** The block cursor's own clock: 500 ms per phase, i.e. the suite's 1 s period at 50 % duty.

        A dedicated Timer rather than the siblings' wall-clock read, because those castings already
        repaint their header at 60 Hz for meters and Elmer's does not - it repaints on change. This
        gets the same cadence without adding a 60 Hz repaint to an otherwise idle panel.

        NOT the header's own Timer, which is a ONE-SHOT revert countdown that stops itself on the
        first tick: sharing it would either kill the blink or keep resurrecting the parameter
        takeover. */
    struct CaretBlinker final : public juce::Timer
    {
        explicit CaretBlinker (ProgramHeader& o) : owner (o) {}
        void timerCallback() override { visible = ! visible; owner.repaint(); }
        void start() { visible = true;  startTimer (500); owner.repaint(); }
        void stop()  { visible = false; stopTimer();      owner.repaint(); }

        ProgramHeader& owner;
        bool visible = false;
    };

    CaretBlinker caret { *this };

    juce::AudioProcessorValueTreeState& apvts;
    ProgramManager& programs;

    juce::String editingParam;
    float inLevelDb = -99.9f;
    float outLevelDb = -99.9f;
    float grDb = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgramHeader)
};
