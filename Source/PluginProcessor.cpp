#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <nf/BlockChunking.h>

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

    /*  **The three smoothed controls are prepared with their CURRENT parameter values.**

        `SmoothedValue::reset (rate, seconds)` sets the ramp length and snaps the value to whatever
        target it last held, which is zero on a constructed object - so a stage prepared without
        being told where its control sits glides up from nowhere across the first block of an
        instance's first playback. That is the "unguarded reset" shape this suite has found eleven
        times in three castings, and passing the values in is what makes it unexpressible here
        rather than something a later edit has to remember.

        Read off the live parameters rather than from a default: a session restore writes the APVTS
        before the host prepares, so these are the values the first block should already be at. */
    ironStage.prepare (sampleRate, ironParam->load() * 0.01f);
    outputStage.prepare (sampleRate, makeupParam->load(), mixParam->load() * 0.01f);

    gainReductionDb.store (0.0f, std::memory_order_relaxed);
    inputLevelDb.store (-100.0f, std::memory_order_relaxed);
    outputLevelDb.store (-100.0f, std::memory_order_relaxed);

    // ~300 ms meter integration is done in the GUI; these two only smooth the numeric IN/OUT
    // readouts so they are legible rather than flickering.
    meterCoeff = 1.0f - std::exp (-1.0f / (0.15f * (float) sampleRate));
}

//==============================================================================
/** A host's reset - a transport locate, a buffer clear - propagated to the DSP.

    **JUCE's base implementation is a no-op, and none of the six castings overrode it**, so until
    stage 1c a host asking every plugin in the session to clear itself was answered by nothing
    anywhere. Measured tails surviving a reset: Gatecrasher 0.679, Chorus-60 0.429, Reflect-84 0.111.

    Routed to the same per-stage `reset()` calls `prepareToPlay` already makes, and deliberately NOT
    to `prepareToPlay` itself: re-preparing would also re-run whatever a prepare re-arms, and this
    suite has a measured example of that being audible.
*/
void ElmerAudioProcessor::reset()
{
    sidechainFilter.reset();
    detector.reset();
    ironStage.reset();
    outputStage.reset();
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

    float peakGr = 0.0f;

    // **The over-delivery policy, and Elmer is the casting where it removes nothing.** The other
    // five grow a scratch buffer when a host sends more samples than it declared; this one holds
    // its dry sample in a local inside the per-sample loop and has no scratch buffer to grow. It
    // still calls the wrapper, because the policy is the deliverable rather than five ports of a
    // fix — and because that makes this the ONE casting where a before/after separates the two
    // effects the others conflate. Everywhere else the allocation goes AND the loop arrives; here
    // only the loop arrives, so anything that moves is the wrapper itself.
    //
    // ScopedNoDenormals and the unused-channel clear stay outside it deliberately: the guard is
    // scoped, so once per call is both correct and cheaper than once per span.
    nf::processInChunks (buffer, getBlockSize(), [&] (juce::AudioBuffer<float>& span)
    {
    auto* left  = span.getWritePointer (0);
    auto* right = numChannels > 1 ? span.getWritePointer (1) : nullptr;

    for (int i = 0; i < span.getNumSamples(); ++i)
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
    });

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
    // **The bank, the identifier, and the full parameter state.** The values make the session sound
    // right; the identity only decides what the panel CALLS them.
    const auto id = programs.getCurrentProgramId();
    xml->setAttribute (ProgramManager::programBankAttribute, ProgramManager::bankAttributeValue (id.bank));
    xml->setAttribute (ProgramManager::programIdAttribute, id.id);
    xml->setAttribute (ProgramManager::programNameAttribute, id.displayName);

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

    // **Two branches, both pinned to literals.** Too old (including pre-schema sessions with no
    // attribute at all, which carry values but no Program identity): applying the default is honest,
    // since the alternative is naming a Program that was never recorded. Too new: written by a later
    // build, and reading it with today's assumptions would give plausible wrong values.
    //
    // This replaced `savedSchema != currentSessionSchemaVersion`, which was correct exactly once.
    if (ProgramManager::classifySessionSchema (savedSchema) != ProgramManager::SchemaVerdict::readable)
    {
        programs.requestProgramChange (ProgramManager::factoryIdAt (Elmer::defaultFactoryProgramIndex));
        return;
    }

    apvts.replaceState (juce::ValueTree::fromXml (*xml));

    ProgramId restored;

    if (savedSchema >= ProgramManager::identitySchemaVersion)
    {
        restored = programs.resolve (
            ProgramManager::bankFromAttribute (
                xml->getStringAttribute (ProgramManager::programBankAttribute)),
            xml->getStringAttribute (ProgramManager::programIdAttribute),
            xml->getStringAttribute (ProgramManager::programNameAttribute));
    }
    else
    {
        // Schema 1 stored a position. Map it through the CURRENT bank.
        const int savedIndex = xml->getIntAttribute (ProgramManager::currentProgramAttribute,
                                                      Elmer::defaultFactoryProgramIndex);

        if (savedIndex == -1)
            restored = ProgramManager::initId();
        else if (juce::isPositiveAndBelow (savedIndex, (int) Elmer::factoryPrograms.size()))
            restored = ProgramManager::factoryIdAt (savedIndex);
        else
            restored = ProgramManager::factoryIdAt (Elmer::defaultFactoryProgramIndex);
    }

    programs.setCurrentProgramWithoutApplying (restored);

    // **Armed AFTER replaceState**, or the restore's own writes would disarm it.
    userEdits.armRestore();
}

void ElmerAudioProcessor::setCurrentProgram (int index)
{
    if (! juce::isPositiveAndBelow (index, programs.getNumPrograms()))
        return;

    // The stale-replay guard, disarmed by this call whether or not it is honoured.
    if (userEdits.consumeRestore() && index == getCurrentProgram())
        return;

    programs.requestProgramChange (ProgramManager::factoryIdAt (index));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ElmerAudioProcessor();
}
