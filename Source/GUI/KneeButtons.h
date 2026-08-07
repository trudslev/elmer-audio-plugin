#pragma once

#include "ElmerTheme.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>

/**
    The two KNEE lamp buttons - SOFT over HARD.

    The selected button's dot is THE ONLY LIT INDICATOR ON THE ENTIRE PANEL. There are no other
    LEDs anywhere; adding one dilutes the signal and breaks the design. AUTO is a position on the
    RELEASE switch, not a button, so it needs no lamp - the pointer is the state.

    The button face never changes. Selection shows two ways only: the LED lights, and the legend
    lights. Everything else - gradient, shadow, border - is identical in both states.
*/
class KneeButtons final : public juce::Component
{
public:
    explicit KneeButtons (juce::AudioProcessorValueTreeState&);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;

    /** Called when the user presses a button, so the header can take the LCD over. */
    std::function<void()> onInteraction;

private:
    void drawLamp (juce::Graphics&, juce::Rectangle<float>, const juce::String& legend, bool lit);

    juce::AudioProcessorValueTreeState& apvts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KneeButtons)
};
