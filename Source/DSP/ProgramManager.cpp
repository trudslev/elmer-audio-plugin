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
    applyProgram (factoryIdAt (defaultFactoryProgramIndex));

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
    // **macOS needs the "Application Support" segment added by hand, and only macOS.** JUCE's
    // userApplicationDataDirectory is `~/Library` there - NOT `~/Library/Application Support` -
    // while it is `%APPDATA%` on Windows and `~/.config` on Linux, both of which are already the
    // right root. JUCE's own PropertiesFile appends the segment the same way, for the same reason.
    //
    // This was got wrong once in exactly the plausible direction: the note here used to claim JUCE
    // resolved the segment for us, and that hard-coding it would be wrong on two platforms out of
    // three. The first half was false, and the second half only argues for the `#if` - it is one
    // platform's extra segment, not a shared literal path. Programs landed directly in
    // `~/Library/<Company>/` for a while, which is not where application data goes on macOS and is
    // not a folder anything else writes into.
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
    auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);

   #if JUCE_MAC
    dir = dir.getChildFile ("Application Support");
   #endif

    return dir
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

    // **The displayed name, case-insensitively.** userFiles.sort() used juce::File::operator<,
    // which compares the FULL PATH via compareFilenames - and that ignores case on macOS/Windows
    // but respects it on Linux, so this casting alone sorted differently by OS. It also compared
    // the extension, which sorts "AB C" before "AB" (space 0x20 precedes dot 0x2E).
    std::sort (userFiles.begin(), userFiles.end(),
               [] (const juce::File& a, const juce::File& b)
               {
                   return a.getFileNameWithoutExtension()
                           .compareIgnoreCase (b.getFileNameWithoutExtension()) < 0;
               });
}

//==============================================================================
// Identity. Nothing below addresses a Program by position except the deliberate crossings.

ProgramId ProgramManager::factoryIdAt (int factoryPosition)
{
    const auto& p = Elmer::factoryPrograms[(size_t) factoryPosition];
    return { ProgramBank::factory, p.slug, p.name };
}

ProgramId ProgramManager::initId()
{
    return { ProgramBank::init, Elmer::initProgram.slug, Elmer::initProgram.name };
}

int ProgramManager::factoryPositionOf (const juce::String& slug)
{
    for (size_t i = 0; i < Elmer::factoryPrograms.size(); ++i)
        if (slug == Elmer::factoryPrograms[i].slug)
            return (int) i;

    return -1;
}

ProgramId ProgramManager::getCurrentProgramId() const
{
    const juce::SpinLock::ScopedLockType lock (currentIdLock);
    return currentId;
}

void ProgramManager::setCurrentId (const ProgramId& id)
{
    const juce::SpinLock::ScopedLockType lock (currentIdLock);
    currentId = id;
}

int ProgramManager::getCurrentFactoryPosition() const
{
    const auto id = getCurrentProgramId();

    if (id.bank == ProgramBank::factory)
        if (const int pos = factoryPositionOf (id.id); pos >= 0)
            return pos;

    return 0;
}

ProgramId ProgramManager::resolve (ProgramBank bank, const juce::String& id,
                                    const juce::String& displayName) const
{
    if (bank == ProgramBank::init && id == Elmer::initProgram.slug)
        return initId();

    if (bank == ProgramBank::factory)
        if (const int pos = factoryPositionOf (id); pos >= 0)
            return factoryIdAt (pos);

    if (bank == ProgramBank::user)
        for (const auto& f : userFiles)
            if (f.getFileNameWithoutExtension() == id)
                return { ProgramBank::user, id, id };

    // **Degrade honestly.** The restored values are correct and stay put; only the name is unknown.
    return { ProgramBank::unresolved, id, displayName.isNotEmpty() ? displayName : id };
}

std::vector<ProgramId> ProgramManager::listPrograms() const
{
    std::vector<ProgramId> out;
    out.reserve (1 + Elmer::factoryPrograms.size() + (size_t) userFiles.size());

    out.push_back (initId());

    for (size_t i = 0; i < Elmer::factoryPrograms.size(); ++i)
        out.push_back (factoryIdAt ((int) i));

    for (const auto& f : userFiles)
    {
        const auto stem = f.getFileNameWithoutExtension();
        out.push_back ({ ProgramBank::user, stem, stem });
    }

    return out;
}

juce::String ProgramManager::displayLabelFor (const ProgramId& id) const
{
    if (id.bank == ProgramBank::factory)
        if (const int pos = factoryPositionOf (id.id); pos >= 0)
            return juce::String (pos + 1).paddedLeft ('0', 2) + " " + id.displayName;

    return id.displayName;
}

juce::File ProgramManager::userProgramFile (const juce::String& stem) const
{
    for (const auto& f : userFiles)
        if (f.getFileNameWithoutExtension() == stem)
            return f;

    return {};
}

void ProgramManager::requestProgramChange (const ProgramId& id)
{
    {
        const juce::SpinLock::ScopedLockType lock (pendingLock);
        pendingProgram = id;
        hasPendingProgram = true;
    }

    triggerAsyncUpdate();
}

juce::String ProgramManager::getProgramName (int factoryPosition) const
{
    // Raw, unnumbered - what the HOST's list wants, since a host renders its own numbering.
    return juce::isPositiveAndBelow (factoryPosition, getNumFactoryPrograms())
               ? juce::String (factoryPrograms[(size_t) factoryPosition].name)
               : juce::String();
}

void ProgramManager::setCurrentProgramWithoutApplying (const ProgramId& id)
{
    setCurrentId (id);
    captureSnapshot();

    if (onProgramChanged != nullptr)
        onProgramChanged();
}

void ProgramManager::cancelPendingChange()
{
    {
        const juce::SpinLock::ScopedLockType lock (pendingLock);
        hasPendingProgram = false;
    }

    cancelPendingUpdate();
}

void ProgramManager::handleAsyncUpdate()
{
    ProgramId id;

    {
        const juce::SpinLock::ScopedLockType lock (pendingLock);

        if (! hasPendingProgram)
            return;

        id = pendingProgram;
        hasPendingProgram = false;
    }

    applyProgram (id);
}

void ProgramManager::applyProgram (const ProgramId& id)
{
    if (id.bank == ProgramBank::init)
    {
        // The slug is checked, not just the bank: an id claiming to be INIT with some other
        // identifier names nothing, and applying INIT anyway would be the same "land on whatever is
        // nearby" failure this model exists to prevent.
        if (id.id != initProgram.slug)
            return;

        applyProgramValues (initProgram);
    }
    else if (id.bank == ProgramBank::factory)
    {
        const int pos = factoryPositionOf (id.id);

        if (pos < 0)
            return;

        applyProgramValues (factoryPrograms[(size_t) pos]);
    }
    else if (id.bank == ProgramBank::user)
    {
        const auto file = userProgramFile (id.id);

        if (file == juce::File())
            return;

        if (! applyUserFile (file))
            return;
    }

    // Unresolved falls through: the values are whatever the session restored and stay exactly as
    // they are. Only the identity is recorded, so the panel can say it does not know the name.
    setCurrentId (id);

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

bool ProgramManager::applyUserFile (const juce::File& file)
{
    auto xml = juce::XmlDocument::parse (file);

    if (xml == nullptr)
        return false;

    // Read the schema version rather than only writing it. A file this build cannot read is skipped
    // WHOLE rather than half-applied - a Program that loads two-thirds of itself is worse than one
    // that refuses, because nothing reports it.
    if (xml->getIntAttribute ("schema", 1) > stateSchemaVersion)
        return false;

    for (const auto* id : allParamIds)
        if (xml->hasAttribute (id))
            setParam (id, (float) xml->getDoubleAttribute (id));

    return true;
}

ProgramId ProgramManager::saveNewUserProgram (const juce::String& name)
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
            setCurrentId ({ ProgramBank::user, file.getFileNameWithoutExtension(),
                            file.getFileNameWithoutExtension() });

            // The snapshot is re-taken against what was just written, so SAVE goes dark and the
            // asterisk clears the instant the Program exists. Without this the panel keeps claiming
            // unsaved changes against the PREVIOUS Program's baseline, immediately after saving.
            captureSnapshot();

            if (onProgramChanged != nullptr)
                onProgramChanged();

            return getCurrentProgramId();
        }
    }

    return getCurrentProgramId();
}

void ProgramManager::deleteUserProgram (const ProgramId& id)
{
    // Gated on the BANK, which is stronger than the old index range: an id from any other bank
    // simply cannot address a file. Gated in the model, not only at the button.
    if (id.bank != ProgramBank::user)
        return;

    const auto file = userProgramFile (id.id);

    if (file == juce::File())
        return;

    file.deleteFile();
    rescanUserPrograms();

    // Deliberately NOT the unresolved state: deleting from the panel is unambiguous intent.
    applyProgram (factoryIdAt (defaultFactoryProgramIndex));
}
