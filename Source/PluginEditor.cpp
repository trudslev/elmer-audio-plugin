#include "PluginEditor.h"

ElmerAudioProcessorEditor::ElmerAudioProcessorEditor (ElmerAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setResizable (true, true);

    // 0.5x - 2x, per BRAND.md: the scaling range has to be a genuine accessibility lever, not a
    // token 10-15%.
    if (auto* constrainer = getConstrainer())
    {
        constrainer->setFixedAspectRatio ((double) canvasWidth / (double) canvasHeight);
        constrainer->setSizeLimits (canvasWidth / 2, canvasHeight / 2,
                                    canvasWidth * 2, canvasHeight * 2);
    }

    setSize (canvasWidth, canvasHeight);
}

void ElmerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFFA9A294));
}

void ElmerAudioProcessorEditor::resized()
{
}
