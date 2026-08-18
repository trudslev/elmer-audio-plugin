#pragma once

#include "PluginProcessor.h"
#include "GUI/ElmerEditorContent.h"

/**
    Thin shell: owns the fixed 1340 x 660 reference canvas and applies one uniform scale transform,
    with the aspect ratio locked. Every child paints in design coordinates and never sees the scale.
*/
class ElmerAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit ElmerAudioProcessorEditor (ElmerAudioProcessor&);
    ~ElmerAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    static constexpr int canvasWidth  = 1340;
    static constexpr int canvasHeight = 660;

    ElmerAudioProcessor& processorRef;
    ElmerEditorContent content;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ElmerAudioProcessorEditor)
};
