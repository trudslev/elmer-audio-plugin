#pragma once

#include "Parameters.h"
#include "DSP/IronStage.h"
#include "DSP/LevelDetector.h"
#include "DSP/OutputStage.h"
#include "DSP/ProgramManager.h"
#include "DSP/SidechainFilter.h"

#include <juce_audio_processors/juce_audio_processors.h>
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

    int getNumPrograms() override { return programs.getNumPrograms(); }
    int getCurrentProgram() override { return programs.getCurrentProgram(); }
    void setCurrentProgram (int index) override { programs.setCurrentProgram (index); }
    const juce::String getProgramName (int index) override { return programs.getProgramName (index); }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

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
