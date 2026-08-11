#include "ProgramManager.h"

#include <cmath>
#include <vector>

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

    // Construction applies the default synchronously - no host or automation is attached yet - so
    // the clean snapshot has to be taken here too. Without it isModified() has nothing to compare
    // against and SAVE stays dark until the first Program CHANGE rather than the first edit.
    captureSnapshot();
}

ProgramManager::~ProgramManager()
{
    cancelPendingUpdate();
}

const juce::StringArray& ProgramManager::snapshotParamIds()
{
    // Everything a Program stores. Elmer has no momentary triggers, so unlike TapeRot - which must
    // exclude STOP/FILTER/FAIL or holding one lights SAVE - this is simply the full set.
    static const juce::StringArray ids {
        ParamIDs::threshold, ParamIDs::ratio, ParamIDs::knee, ParamIDs::sidechainHp,
        ParamIDs::attack, ParamIDs::release, ParamIDs::iron, ParamIDs::makeup, ParamIDs::mix };
    return ids;
}

void ProgramManager::captureSnapshot()
{
    const auto& ids = snapshotParamIds();
    std::vector<float> fresh;
    fresh.reserve ((size_t) ids.size());

    for (const auto& id : ids)
        if (auto* v = apvts.getRawParameterValue (id))
            fresh.push_back (v->load (std::memory_order_relaxed));
        else
            fresh.push_back (0.0f);

    const juce::SpinLock::ScopedLockType lock (snapshotLock);
    snapshot = std::move (fresh);
}

bool ProgramManager::isModified() const
{
    const auto& ids = snapshotParamIds();

    const juce::SpinLock::ScopedLockType lock (snapshotLock);

    if (snapshot.size() != (size_t) ids.size())
        return false;                   // nothing captured yet - nothing to compare against

    for (int i = 0; i < ids.size(); ++i)
        if (auto* v = apvts.getRawParameterValue (ids[i]))
        {
            // Physical values, so the tolerance suits the widest range here (sidechain HP in Hz).
            // Anything a user can hear moving is far larger than this.
            if (std::abs (v->load (std::memory_order_relaxed) - snapshot[(size_t) i]) > 1.0e-3f)
                return true;
        }

    return false;
}

juce::File ProgramManager::getUserProgramDirectory()
{
    // **Application data, not ~/Library/Audio/Presets.** That directory is Apple's location for the
    // AU PRESET FORMAT - .aupreset files the AU system itself scans, reads and writes. Our user
    // Programs are not those; they are application-owned data in our own format, so they belong
    // where an application keeps its data.
    //
    // The Application Support segment is JUCE's, not ours, and must never be hard-coded:
    // userApplicationDataDirectory resolves to ~/Library/Application Support on macOS, %APPDATA% on
    // Windows and ~/.config on Linux. A shared literal path would be wrong on two of the three.
    //
    // Company and product come from CMake rather than string literals - see the note in
    // CMakeLists.txt for the drift that cost CHORUS-60 a directory.
    //
    // **There is deliberately no migration from ~/Library/Audio/Presets, and that is a decision
    // rather than an omission.** One was written and then removed: nothing in this suite has
    // shipped at a released version, so no installed build has ever written a Program to the old
    // location for anyone but us, and our own were disposable. Migration code guarding a case that
    // cannot occur is dead weight that still costs a directory probe on every rescan.
    //
    // If a version ever ships and the path changes AGAIN, that is when this becomes necessary.
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile (NF_COMPANY_NAME)
               .getChildFile (NF_PRODUCT_NAME)
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
    if (isInit (index))
        return "INIT";

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
    //
    // INIT is UNNUMBERED - here as well as in the menu. Numbering it would put it in a bank, and it
    // is in neither; the arithmetic would also print "00", since its index is -1.
    if (isInit (index))
        return getProgramName (index);

    return juce::String (index + 1).paddedLeft ('0', 2) + " " + getProgramName (index);
}

void ProgramManager::setCurrentProgram (int index)
{
    // INIT is a legal target and is NOT in 0..getNumPrograms(), so it is admitted explicitly rather
    // than by widening the range check - which would also admit every other negative index.
    if (! isInit (index) && ! juce::isPositiveAndBelow (index, getNumPrograms()))
        return;

    pendingIndex.store (index);
    triggerAsyncUpdate();
}

void ProgramManager::setCurrentProgramIndexWithoutApplying (int index)
{
    currentIndex = (isInit (index) || juce::isPositiveAndBelow (index, getNumPrograms()))
                       ? index : Elmer::defaultFactoryProgramIndex;
    captureSnapshot();

    if (onProgramChanged != nullptr)
        onProgramChanged();
}

void ProgramManager::cancelPendingChange()
{
    pendingIndex.store (-2);
    cancelPendingUpdate();
}

void ProgramManager::handleAsyncUpdate()
{
    // -2 is the idle sentinel, not -1: -1 is INIT, and using it here would make selecting INIT
    // indistinguishable from having nothing to apply.
    const int index = pendingIndex.exchange (-2);

    if (index < -1)
        return;

    currentIndex = index;

    if (isInit (index) || isFactory (index))
        applyFactory (index);
    else
        applyUser (index);

    // Taken AFTER the values land, so the Program starts clean by definition.
    captureSnapshot();

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
    applyProgramValues (isInit (index) ? initProgram : factoryPrograms[(size_t) index]);
}

void ProgramManager::applyProgramValues (const FactoryProgram& fp)
{
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

    // Trimmed BEFORE the emptiness test, so a name of nothing but spaces falls back rather than
    // producing a file whose name is invisible in the menu.
    const auto trimmed = name.trim();
    const auto safe = juce::File::createLegalFileName (trimmed.isEmpty() ? "UNTITLED" : trimmed);

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

            // The snapshot is re-taken against what was just written, so SAVE goes dark and the
            // asterisk clears the instant the Program exists. Without this the panel keeps claiming
            // unsaved changes against the PREVIOUS Program's baseline, immediately after saving.
            captureSnapshot();

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
