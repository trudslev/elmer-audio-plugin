#include "ElmerEditorContent.h"
#include "../PluginProcessor.h"

using namespace ElmerTheme;

ElmerEditorContent::ElmerEditorContent (ElmerAudioProcessor& p)
    : processorRef (p),
      kneeButtons (p.apvts),
      header (p.apvts, p.programs)
{
    setSize ((int) Layout::canvasWidth, (int) Layout::canvasHeight);

    addAndMakeVisible (background);
    background.setBounds (0, 0, (int) Layout::canvasWidth, (int) Layout::canvasHeight);

    addAndMakeVisible (meter);
    meter.setBounds (juce::Rectangle<float> (Layout::meterX, Layout::meterY,
                                             Layout::meterW, Layout::meterH)
                         .getSmallestIntegerContainer());

    addAndMakeVisible (kneeButtons);
    kneeButtons.setTopLeftPosition ((int) Layout::kneeButtonsTopLeft.x,
                                    (int) Layout::kneeButtonsTopLeft.y);
    kneeButtons.onInteraction = [this]
    {
        header.showParameter (ParamIDs::knee);
        header.releaseParameter();
    };

    // Knobs are created once and never recreated - nothing about this panel changes which control
    // sits where, and recreating a Slider drops its attachment and its drag state.
    for (const auto& spec : Layout::knobs)
    {
        auto knob = std::make_unique<KnobFilmstrip> (spec.strip, spec.knobSize);
        knob->setCentrePosition (spec.areaCentre);

        const juce::String paramId { spec.paramId };
        auto* raw = knob.get();
        knob->onDragStart = [this, paramId] { header.showParameter (paramId); };
        knob->onDragEnd   = [this] { header.releaseParameter(); };

        // Only a GRAB takes the display over. Guarding on the drag state matters: a
        // SliderAttachment fires onValueChange when a Program is applied and when the host
        // automates, and without this the LCD latches onto whichever parameter was written last
        // and never shows the program name at all.
        // The same guard disarms the processor's stale-replay flag, because this is the only place
        // that knows a change came from a PERSON. It deliberately does not fire for automation: a
        // host may write automation on session load before replaying its remembered program index,
        // and disarming there would let that replay land on the restored state.
        knob->onValueChange = [this, paramId, raw]
        {
            if (raw->isMouseButtonDown())
            {
                processorRef.noteUserEdit();
                header.showParameter (paramId);
            }
        };

        addAndMakeVisible (*knob);
        attachments.push_back (
            std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
                processorRef.apvts, paramId, *knob));
        knobs.push_back (std::move (knob));
    }

    addAndMakeVisible (header);

    // The list opens inside this, so it can neither move its top edge nor grow past the panel.
    // A SIBLING of header, never a child: header spans the canvas and narrows its hitTest, and JUCE
    // stops searching a component's children once its own hitTest rejects the point.
    const int hostTop = ProgramHeader::menuHostTop();
    menuHost.setBounds (0, hostTop, getWidth(), getHeight() - hostTop);
    menuHost.setInterceptsMouseClicks (false, true);
    addAndMakeVisible (menuHost);
    menuHost.toFront (false);
    header.setMenuParent (&menuHost);
    header.toBack();
    background.toBack();

    processorRef.programs.onProgramChanged = [this] { header.refresh(); };

    startTimerHz (Layout::animationHz);
}

ElmerEditorContent::~ElmerEditorContent()
{
    processorRef.programs.onProgramChanged = nullptr;
}

void ElmerEditorContent::timerCallback()
{
    const double now = juce::Time::getMillisecondCounterHiRes() * 0.001;
    const float dt = lastTimeSeconds > 0.0
                       ? (float) juce::jlimit (0.001, 0.1, now - lastTimeSeconds)
                       : 1.0f / (float) Layout::animationHz;
    lastTimeSeconds = now;

    if (meter.updateBallistics (processorRef.getGainReductionDb(), dt))
        meter.repaint();

    header.setLevels (processorRef.getInputLevelDb(), processorRef.getOutputLevelDb());
    header.setGainReductionDb (meter.getDisplayedGainReductionDb());
}
