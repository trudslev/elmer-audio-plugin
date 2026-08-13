#pragma once

#include "Parameters.h"
#include "DSP/IronStage.h"
#include "DSP/LevelDetector.h"
#include "DSP/OutputStage.h"
#include "DSP/ProgramManager.h"
#include "DSP/SidechainFilter.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <nf/UserEditGate.h>
#include <atomic>

/**
    Elmer - stereo-linked VCA bus compressor, model GL-87.

    Plain stereo in/out. There is no external sidechain bus: SIDECHAIN HP is a filter on the
    detector path, not a key input, so nothing needs routing in from the host.

    Signal chain, in order:

        in -+-------------------- dry ------------------------------+
            |                                                        |
            +-> SidechainFilter -> LevelDetector -> gain reduction   |
            |        (detector path only)              |             |
            +-> x gain <------------------------------+              |
                   |                                                 |
                   +-> IronStage -> [makeup] -> OutputStage mix <----+
*/
class ElmerAudioProcessor final : public juce::AudioProcessor
{
public:
    ElmerAudioProcessor();
    ~ElmerAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    /** **The host adapter - the ONLY place a Program is addressed by position.**

        **The list is the Factory bank and nothing else.** juce_AudioProcessor.h documents
        getNumPrograms as "The value returned must be valid as soon as this object is created, and
        must not change over its lifetime"; a count including User Programs changed on every save.
        JUCE's VST3 wrapper builds the automatable Program parameter ONCE from this value, so a
        Program saved afterwards was unreachable from the host. Excluding INIT too means host index
        n IS Factory Program n+1.

        **Accepted divergence.** getCurrentProgram answers 0 while a User Program is loaded, so a
        host's menu shows a Factory name while the panel shows the user's Program. Sound and panel
        are both correct; only the host's own menu is wrong. */
    int getNumPrograms() override { return programs.getNumPrograms(); }
    int getCurrentProgram() override { return programs.getCurrentFactoryPosition(); }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override { return programs.getProgramName (index); }
    /** Deliberately a no-op: with Factory-only exposure nothing on the host's list can be renamed.
        Implementing it would be a back door into the Factory bank. */
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    /** **Guards a host replaying a stale program index over a just-restored session.** Armed by
        setStateInformation, consumed by the next setCurrentProgram (which ignores it only when the
        index matches what getCurrentProgram already reports — the shape of a replay), disarmed by
        the first USER-originated edit. **Automation must not disarm it**: a host may write
        automation on load before replaying, and that would reopen the hole.

        Public because the editor hands it to `nf::connectUserEdit` for every control, which is the
        point of it living in core: Reflect-84 once shipped this guard with zero call sites for its
        disarm, and coupling the disarm to the LCD hand-off is what makes that omission
        inexpressible. See nf/UserEditGate.h. */
    nf::UserEditGate userEdits;

    juce::AudioProcessorValueTreeState apvts;

    ProgramManager programs { apvts };

    /** Gain reduction in dB, for the meter. Relaxed - it is a display value, and a torn read costs
        one frame of a needle already integrating over 300 ms. */
    float getGainReductionDb() const noexcept { return gainReductionDb.load (std::memory_order_relaxed); }
    float getInputLevelDb()  const noexcept { return inputLevelDb.load (std::memory_order_relaxed); }
    float getOutputLevelDb() const noexcept { return outputLevelDb.load (std::memory_order_relaxed); }

private:
    SidechainFilter sidechainFilter;
    LevelDetector detector;
    IronStage ironStage;
    OutputStage outputStage;

    std::atomic<float>* thresholdParam   = nullptr;
    std::atomic<float>* ratioParam       = nullptr;
    std::atomic<float>* kneeParam        = nullptr;
    std::atomic<float>* sidechainHpParam = nullptr;
    std::atomic<float>* attackParam      = nullptr;
    std::atomic<float>* releaseParam     = nullptr;
    std::atomic<float>* ironParam        = nullptr;
    std::atomic<float>* makeupParam      = nullptr;
    std::atomic<float>* mixParam         = nullptr;

    std::atomic<float> gainReductionDb { 0.0f };
    std::atomic<float> inputLevelDb  { -100.0f };
    std::atomic<float> outputLevelDb { -100.0f };

    float meterCoeff = 0.0f;
    float inSmoothed = 0.0f;
    float outSmoothed = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ElmerAudioProcessor)
};
