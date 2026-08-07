#include "PluginEditor.h"

ElmerAudioProcessorEditor::ElmerAudioProcessorEditor (ElmerAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), content (p)
{
    addAndMakeVisible (content);

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
    g.fillAll (juce::Colour (0xFF22201D));
}

void ElmerAudioProcessorEditor::resized()
{
    // One uniform transform for the whole panel. Nothing below this line knows about scaling; every
    // child lays itself out in design coordinates against the fixed canvas.
    const float scale = (float) getWidth() / (float) canvasWidth;
    content.setTransform (juce::AffineTransform::scale (scale));
    content.setBounds (0, 0, canvasWidth, canvasHeight);
}
