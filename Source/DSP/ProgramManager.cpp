#include "ProgramManager.h"

using namespace Elmer;

namespace
{
    constexpr const char* fileExtension = ".elmerprogram";
    constexpr int stateSchemaVersion = 1;

    const std::array<const char*, 9> allParamIds { {
        ParamIDs::threshold, ParamIDs::ratio, ParamIDs::knee, ParamIDs::sidechainHp,
        ParamIDs::attack, ParamIDs::release, ParamIDs::iron, ParamIDs::makeup, ParamIDs::mix } };
}

ProgramManager::ProgramManager (juce::AudioProcessorValueTreeState& s) : apvts (s)
{
    rescanUserPrograms();
    applyFactory (defaultFactoryProgramIndex);
}

ProgramManager::~ProgramManager()
{
    cancelPendingUpdate();
}

juce::File ProgramManager::getUserProgramDirectory()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("Neon Foundry")
               .getChildFile ("Elmer")
               .getChildFile ("Programs");
}

void ProgramManager::rescanUserPrograms()
{
    userFiles.clear();
    auto dir = getUserProgramDirectory();

    if (dir.isDirectory())
        for (const auto& f : dir.findChildFiles (juce::File::findFiles, false,
                                                 "*" + juce::String (fileExtension)))
            userFiles.add (f);

    userFiles.sort();
}

int ProgramManager::getNumPrograms() const
{
    return getNumFactoryPrograms() + userFiles.size();
}

juce::String ProgramManager::getProgramName (int index) const
{
    if (isFactory (index))
        return juce::isPositiveAndBelow (index, getNumFactoryPrograms())
                   ? juce::String (factoryPrograms[(size_t) index].name) : juce::String();

    const int userIndex = index - getNumFactoryPrograms();
    return juce::isPositiveAndBelow (userIndex, userFiles.size())
               ? userFiles[userIndex].getFileNameWithoutExtension() : juce::String();
}

juce::String ProgramManager::getDisplayName (int index) const
{
    // The LCD shows a two-digit number and the name, e.g. "01 UNDER PRESSURE".
    return juce::String (index + 1).paddedLeft ('0', 2) + " " + getProgramName (index);
}

void ProgramManager::setCurrentProgram (int index)
{
    if (! juce::isPositiveAndBelow (index, getNumPrograms()))
        return;

    pendingIndex.store (index);
    triggerAsyncUpdate();
}

void ProgramManager::handleAsyncUpdate()
{
    const int index = pendingIndex.exchange (-1);

    if (index < 0)
        return;

    currentIndex = index;

    if (isFactory (index))
        applyFactory (index);
    else
        applyUser (index);

    if (onProgramChanged != nullptr)
        onProgramChanged();
}

void ProgramManager::setParam (const char* id, float actualValue)
{
    if (auto* p = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (id)))
        p->setValueNotifyingHost (p->convertTo0to1 (actualValue));
}

void ProgramManager::applyFactory (int index)
{
    const auto& fp = factoryPrograms[(size_t) index];

    setParam (ParamIDs::threshold,   fp.thresholdDb);
    setParam (ParamIDs::ratio,       (float) fp.ratioIndex);
    setParam (ParamIDs::knee,        (float) fp.kneeIndex);
    setParam (ParamIDs::sidechainHp, Law::hpPositionForHz (fp.sidechainHpHz));
    setParam (ParamIDs::attack,      fp.attackMs);
    setParam (ParamIDs::release,     (float) fp.releaseIndex);
    setParam (ParamIDs::iron,        fp.ironPercent);
    setParam (ParamIDs::makeup,      fp.makeupDb);
    setParam (ParamIDs::mix,         fp.mixPercent);
}

void ProgramManager::applyUser (int index)
{
    const int userIndex = index - getNumFactoryPrograms();

    if (! juce::isPositiveAndBelow (userIndex, userFiles.size()))
        return;

    if (auto xml = juce::XmlDocument::parse (userFiles[userIndex]))
    {
        // Read the schema version rather than only writing it - Reflect-84's improvement. A newer
        // file is skipped whole rather than half-applied.
        if (xml->getIntAttribute ("schema", 1) > stateSchemaVersion)
            return;

        for (const auto* id : allParamIds)
            if (xml->hasAttribute (id))
                setParam (id, (float) xml->getDoubleAttribute (id));
    }
}

int ProgramManager::saveNewUserProgram (const juce::String& name)
{
    auto dir = getUserProgramDirectory();
    dir.createDirectory();

    const auto safe = juce::File::createLegalFileName (name.isEmpty() ? "UNTITLED" : name);

    // A name collision creates a distinct file rather than silently overwriting - Reflect-84's fix,
    // and the reason SAVE can promise never to overwrite.
    auto file = dir.getChildFile (safe + fileExtension).getNonexistentSibling();

    juce::XmlElement xml { "ElmerProgram" };
    xml.setAttribute ("schema", stateSchemaVersion);

    for (const auto* id : allParamIds)
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (id)))
            xml.setAttribute (id, (double) p->convertFrom0to1 (p->getValue()));

    xml.writeTo (file);
    rescanUserPrograms();

    for (int i = 0; i < userFiles.size(); ++i)
    {
        if (userFiles[i] == file)
        {
            currentIndex = getNumFactoryPrograms() + i;

            if (onProgramChanged != nullptr)
                onProgramChanged();

            return currentIndex;
        }
    }

    return currentIndex;
}

void ProgramManager::deleteUserProgram (int index)
{
    if (isFactory (index))
        return;                                   // gated in the model, not only at the button

    const int userIndex = index - getNumFactoryPrograms();

    if (! juce::isPositiveAndBelow (userIndex, userFiles.size()))
        return;

    userFiles[userIndex].deleteFile();
    rescanUserPrograms();

    currentIndex = defaultFactoryProgramIndex;
    applyFactory (currentIndex);

    if (onProgramChanged != nullptr)
        onProgramChanged();
}
