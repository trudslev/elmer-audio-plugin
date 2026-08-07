#pragma once

#include "ElmerTheme.h"

/**
    Everything on the panel that never changes, rasterised once into a juce::Image and blitted.

    That covers the fascia and its brushed grain, the side rails and screws, the section boxes, the
    divider, every printed label, the tick rings, the nameplate plinth and the scribble strip. The
    tick rings in particular belong here: they do not rotate with the knob, so re-rendering them
    every frame would be pure waste.

    Baked at `bakeScale`, not 1:1. The fascia's grain is a 4px repeating gradient with 1px stripes;
    blitted 1:1 to a Retina display it resolves to a flat wash and the metal stops reading as metal.
*/
class PanelBackground final : public juce::Component
{
public:
    PanelBackground();

    void paint (juce::Graphics&) override;

private:
    static constexpr int bakeScale = 2;

    void buildImage();

    void paintFascia (juce::Graphics&);
    void paintRailsAndScrews (juce::Graphics&);
    void paintHeaderChrome (juce::Graphics&);
    void paintSections (juce::Graphics&);
    void paintKnobFurniture (juce::Graphics&);
    void paintMeterChrome (juce::Graphics&);
    void paintFooter (juce::Graphics&);

    juce::Image baked;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelBackground)
};
