#pragma once

#include "ElmerTheme.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>

/**
    KNEE — a two-position SHOE, the delivered prototype's catalogue part 4B.

    **The shoe carries the state in material, not in ink.** The live half is pale metal and the
    idle half is dark; the SOFT and HARD legends print permanently on the fascia beneath their own
    half, in one weight and one ink, and are drawn by `PanelBackground` with the rest of the
    printed furniture. Nothing about this control re-inks on selection.

    That replaced a pair of lamp buttons — the mechanism the prototype records 4B as withdrawing.
    With the lamp gone this panel has no lit indicator at all, which is the design rather than an
    omission; see the retired `accent` note in `ElmerTheme.h`.

    Click anywhere in the left half for SOFT, the right half for HARD. There is no toggle-on-click:
    a shoe is a position, so clicking the half that is already live is a no-op rather than a flip.
*/
class KneeSwitch final : public juce::Component
{
public:
    explicit KneeSwitch (juce::AudioProcessorValueTreeState&);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;

    /** Called when the user presses the shoe, so the header can take the LCD over. */
    std::function<void()> onInteraction;

private:
    void paintHalf (juce::Graphics&, juce::Rectangle<float>, bool live) const;

    juce::AudioProcessorValueTreeState& apvts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KneeSwitch)
};
