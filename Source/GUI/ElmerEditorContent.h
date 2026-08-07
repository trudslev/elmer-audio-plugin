#pragma once

#include "ElmerTheme.h"
#include "GainReductionMeter.h"
#include "KneeButtons.h"
#include "KnobFilmstrip.h"
#include "PanelBackground.h"
#include "ProgramHeader.h"

class ElmerAudioProcessor;

/**
    The fixed 1120 x 776 reference canvas. Every child paints in design coordinates; the editor
    shell applies one uniform scale transform above this and nothing here ever sees it.
*/
class ElmerEditorContent final : public juce::Component,
                                 private juce::Timer
{
public:
    explicit ElmerEditorContent (ElmerAudioProcessor&);
    ~ElmerEditorContent() override;

private:
    void timerCallback() override;

    ElmerAudioProcessor& processorRef;

    PanelBackground background;
    GainReductionMeter meter;
    KneeButtons kneeButtons;
    ProgramHeader header;

    std::vector<std::unique_ptr<KnobFilmstrip>> knobs;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;

    double lastTimeSeconds = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ElmerEditorContent)
};
