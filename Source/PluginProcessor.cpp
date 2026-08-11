#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

ElmerAudioProcessor::ElmerAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "ELMER", Elmer::createParameterLayout())
{
    // Cached once. Calling getRawParameterValue per block is a map lookup per parameter per block.
    thresholdParam   = apvts.getRawParameterValue (ParamIDs::threshold);
    ratioParam       = apvts.getRawParameterValue (ParamIDs::ratio);
    kneeParam        = apvts.getRawParameterValue (ParamIDs::knee);
    sidechainHpParam = apvts.getRawParameterValue (ParamIDs::sidechainHp);
    attackParam      = apvts.getRawParameterValue (ParamIDs::attack);
    releaseParam     = apvts.getRawParameterValue (ParamIDs::release);
    ironParam        = apvts.getRawParameterValue (ParamIDs::iron);
    makeupParam      = apvts.getRawParameterValue (ParamIDs::makeup);
    mixParam         = apvts.getRawParameterValue (ParamIDs::mix);
}

void ElmerAudioProcessor::prepareToPlay (double sampleRate, int)
{
    sidechainFilter.prepare (sampleRate);
    detector.prepare (sampleRate);
    ironStage.reset();

    gainReductionDb.store (0.0f, std::memory_order_relaxed);
    inputLevelDb.store (-100.0f, std::memory_order_relaxed);
    outputLevelDb.store (-100.0f, std::memory_order_relaxed);

    // ~300 ms meter integration is done in the GUI; these two only smooth the numeric IN/OUT
    // readouts so they are legible rather than flickering.
    meterCoeff = 1.0f - std::exp (-1.0f / (0.15f * (float) sampleRate));
}

bool ElmerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();

    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainInputChannelSet() == out;
}

void ElmerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = juce::jmin (buffer.getNumChannels(), getTotalNumInputChannels());

    for (int ch = numChannels; ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);

    if (numChannels == 0 || numSamples == 0)
        return;

    // --- parameters, converted once per block ------------------------------------------------
    const int ratioIndex   = juce::jlimit (0, 4, (int) std::lround (ratioParam->load()));
    const int releaseIndex = juce::jlimit (0, 4, (int) std::lround (releaseParam->load()));
    const bool hardKnee    = kneeParam->load() > 0.5f;
    const bool autoRelease = releaseIndex == (int) Elmer::Release::autoMode;

    detector.computer().setThresholdDb (thresholdParam->load());
    detector.computer().setRatio (Elmer::ratioValues[(size_t) ratioIndex]);
    detector.computer().setKneeWidthDb (hardKnee ? 0.0f : GainComputer::softKneeWidthDb);
    detector.setAttackMs (attackParam->load());
    detector.setRelease (autoRelease ? 0.3f : Elmer::releaseSeconds[(size_t) releaseIndex], autoRelease);

    sidechainFilter.setCutoffHz (Elmer::Law::hpFrequencyHz (sidechainHpParam->load()));

    ironStage.setAmount (ironParam->load() * 0.01f);
    outputStage.setMakeupDb (makeupParam->load());
    outputStage.setMix (mixParam->load() * 0.01f);

    auto* left  = buffer.getWritePointer (0);
    auto* right = numChannels > 1 ? buffer.getWritePointer (1) : nullptr;

    float peakGr = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        const float dryL = left[i];
        const float dryR = right != nullptr ? right[i] : dryL;

        // ONE detector, fed a mono sum. Both channels get the same gain, so the stereo image cannot
        // move. This is the whole premise of a bus compressor - do not make it per-channel.
        const float detectorIn = sidechainFilter.processSample ((dryL + dryR) * 0.5f);
        const float grDb = detector.processSample (detectorIn);
        const float gain = std::pow (10.0f, -grDb / 20.0f);

        peakGr = juce::jmax (peakGr, grDb);

        const float wetL = ironStage.processSample (dryL * gain);
        left[i] = outputStage.processSample (dryL, wetL);

        if (right != nullptr)
        {
            const float wetR = ironStage.processSample (dryR * gain);
            right[i] = outputStage.processSample (dryR, wetR);
        }

        inSmoothed  += (std::abs (dryL) - inSmoothed) * meterCoeff;
        outSmoothed += (std::abs (left[i]) - outSmoothed) * meterCoeff;
    }

    const auto toDb = [] (float linear)
    {
        return linear > 1.0e-5f ? 20.0f * std::log10 (linear) : -99.9f;
    };

    gainReductionDb.store (peakGr, std::memory_order_relaxed);
    inputLevelDb.store (toDb (inSmoothed), std::memory_order_relaxed);
    outputLevelDb.store (toDb (outSmoothed), std::memory_order_relaxed);
}

juce::AudioProcessorEditor* ElmerAudioProcessor::createEditor()
{
    return new ElmerAudioProcessorEditor (*this);
}

void ElmerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto xml = apvts.copyState().createXml();

    if (xml == nullptr)
        return;

    // WHICH Program is showing is session state in its own right. Without it a reopened session
    // restored every value correctly and then labelled them "01 UNDER PRESSURE" whatever had
    // actually been selected - and, because the clean snapshot was never re-taken either, hung a
    // dirty asterisk over a session nobody had touched.
    xml->setAttribute (ProgramManager::sessionSchemaAttribute,
                       ProgramManager::currentSessionSchemaVersion);
    xml->setAttribute (ProgramManager::currentProgramAttribute, programs.getCurrentProgram());

    copyXmlToBinary (*xml, destData);
}

void ElmerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);

    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
        return;

    // Read, not merely written: restoring an older layout otherwise leaves surviving IDs at their
    // saved values while new ones fall back to defaults, a silent hybrid nothing reports.
    const int savedSchema = xml->getIntAttribute (ProgramManager::sessionSchemaAttribute, 0);

    programs.cancelPendingChange();

    if (savedSchema != ProgramManager::currentSessionSchemaVersion)
    {
        // Pre-schema sessions (no attribute at all) carry values but no Program identity. Applying
        // the default is honest: the alternative is naming a Program that was never recorded.
        programs.setCurrentProgram (Elmer::defaultFactoryProgramIndex);
        return;
    }

    apvts.replaceState (juce::ValueTree::fromXml (*xml));

    programs.setCurrentProgramIndexWithoutApplying (
        xml->getIntAttribute (ProgramManager::currentProgramAttribute,
                              Elmer::defaultFactoryProgramIndex));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ElmerAudioProcessor();
}
