#pragma once

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

    /** Takes the display over with a parameter's name and value. */
    void showParameter (const juce::String& paramId);
    /** Starts the 1200 ms countdown back to the program name. */
    void releaseParameter();

    /** The component the Program list lays out inside; its bounds fix the list's top edge and stop
        it outgrowing the panel. See ../../CLAUDE.md, "The Program dropdown". */
    void setMenuParent (juce::Component* parent) noexcept { menuParent = parent; }

    static int menuAnchorY() noexcept
    {
        return (int) std::floor (ElmerTheme::Layout::lcdRowY + ElmerTheme::Layout::menuTopOffset);
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
    void drawLcdPanel (juce::Graphics&, juce::Rectangle<float>, const juce::String& text,
                       juce::Justification, float textSize);
    juce::Rectangle<float> saveBounds() const;
    juce::Rectangle<float> deleteBounds() const;
    juce::Rectangle<float> displayBounds() const;
    juce::Rectangle<float> chevronCellBounds() const;
    void showProgramMenu();
    void paintChevron (juce::Graphics&) const;

    juce::Component* menuParent = nullptr;
    bool menuOpen = false;

    juce::AudioProcessorValueTreeState& apvts;
    ProgramManager& programs;

    juce::String editingParam;
    float inLevelDb = -99.9f;
    float outLevelDb = -99.9f;
    float grDb = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgramHeader)
};
