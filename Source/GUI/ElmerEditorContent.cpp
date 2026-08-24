#include "ElmerEditorContent.h"
#include "../PluginProcessor.h"

using namespace ElmerTheme;

ElmerEditorContent::ElmerEditorContent (ElmerAudioProcessor& p)
    : processorRef (p),
      kneeSwitch (p.apvts),
      header (p.apvts, p.programs)
{
    setSize ((int) Layout::canvasWidth, (int) Layout::canvasHeight);

    addAndMakeVisible (background);
    background.setBounds (0, 0, (int) Layout::canvasWidth, (int) Layout::canvasHeight);

    addAndMakeVisible (meter);
    meter.setBounds (juce::Rectangle<float> (Layout::meterX, Layout::meterY,
                                             Layout::meterW, Layout::meterH)
                         .getSmallestIntegerContainer());

    addAndMakeVisible (kneeSwitch);
    kneeSwitch.setTopLeftPosition ((int) Layout::kneeShoeX, (int) Layout::kneeShoeY);
    kneeSwitch.onInteraction = [this]
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
        // The same guard disarms the processor's stale-replay gate, because this is the only place
        // that knows a change came from a PERSON. It deliberately does not fire for automation: a
        // host may write automation on session load before replaying its remembered program index,
        // and disarming there would let that replay land on the restored state. One call rather than
        // two adjacent ones, so the disarm cannot be written without the hand-off — see
        // nf/UserEditGate.h.
        nf::connectUserEdit (*raw, processorRef.userEdits,
                             [this, paramId] { header.showParameter (paramId); });

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

    /*  `ABOUT-PART.md`. §9's materials and §1's five strings are all this casting supplies. */
    {
        constexpr int frameOriginX = 0;   // no rack ears: the frame IS the window

        const nf::AboutMaterials aboutMaterials {
            Colour::aboutGlass, Colour::aboutBody, Colour::aboutDim, Colour::aboutAccent,
            Colour::aboutRing,
            Colour::aboutWellTop, Colour::aboutWellBottom, Colour::aboutWellInk,
            Font::barlowSemiBold(), Font::barlowMedium(), Font::monoRegular(),
            Cursor::help()
        };

        /*  §9.3, as corrected in change set 40: **two licence families, not one.** Permanent Marker
            is **Apache 2.0** by its own name table — nameID 13, © 2010 Font Diner — and §8 asks for
            the faces this casting EMBEDS. Revision 2's line omitted the face entirely while
            Fifth Member's called it OFL: the same error facing opposite ways, one line apart. This
            casting's own §9.2 still reads "all under the SIL Open Font License"; the shared spec is
            both newer and right, and a false licence attribution in a shipped credit is worse than
            a deviation from a document. */
        const nf::AboutContent aboutContent {
            "ELMER", "GL-87",
            NF_VERSION,                 // semver, from PROJECT_VERSION - never a literal
            nf::suiteRelease,           // §1: a separate string, and neither derives from the other
            "github.com/trudslev/elmer-audio-plugin",
            "Barlow Condensed, IBM Plex Mono, Share Tech Mono and Archivo Expanded under the "
            "SIL Open Font License, and Permanent Marker under the Apache License 2.0."
        };

        aboutBox = std::make_unique<nf::AboutBox> (aboutMaterials, aboutContent, frameOriginX);

        /*  §2, revision 3: the tab takes **the face and size this casting's stamp already uses**,
            and the stamp is §5's one consolidated footer line.

            **Three things the promotion corrects, all read off the DELIVERED prototype rather than
            off the build.** `PanelBackground` drew `GL-87 · SN 0871 · v1.0` in IBM Plex Mono
            SemiBold; the delivered `Elmer GL-87 Panel.dc.html` declares
            `GL-87 · SN 0042 · v1.0.0` in **IBM Plex Mono 500** at 10 / 13 / .18 em. §5's own text
            gives the same serial and the same weight.

            **`SN 0871` came from `Elmer.dc.html`, the SUPERSEDED prototype**, which reads
            `GL-87 · CONSOLE MODULE · SN 0871 · v1.0`. That is this casting's own which-artefact-is-
            current failure arriving in a string: the build was drawing yesterday's panel. */
        aboutTab = std::make_unique<nf::AboutTab> (aboutMaterials, Font::monoMedium(),
                                                   juce::String ("GL-87 ") + Text::middleDot()
                                                       + " SN 0042 " + Text::middleDot()
                                                       + " v" + juce::String (NF_VERSION),
                                                   Layout::footerTextSize,
                                                   Layout::footerTrackingEm);
        aboutTab->onClick = [this] { aboutBox->open(); };

        // §2a: the wordmark is the PRIMARY affordance. It draws nothing — PanelBackground already
        // draws the nameplate; this only claims HeaderGeometry's zone, 303 x 84, shared by all six.
        aboutWordmark = std::make_unique<nf::AboutWordmarkHit> (Cursor::help());
        aboutWordmark->onClick = [this] { aboutBox->open(); };

        /*  **Registered LAST, and that is not tidiness.** JUCE paints children in the order they
            were added, so registering these beside their construction puts the tab under the panel
            background — drawn, correct, and invisible in a capture. */
        aboutTab->layoutFor (getHeight(), frameOriginX);
        aboutWordmark->setBounds (nf::AboutWordmarkHit::zone (frameOriginX));
        aboutBox->setBounds (getLocalBounds());
        addAndMakeVisible (*aboutWordmark);
        addAndMakeVisible (*aboutTab);
        addChildComponent (*aboutBox);
    }

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
