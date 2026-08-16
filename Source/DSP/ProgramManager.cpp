#include "ProgramManager.h"

#include <nf/UserProgramDirectory.h>

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

ProgramManager::ProgramManager (juce::AudioProcessorValueTreeState& s,
                                juce::File userDirectoryOverride)
    : apvts (s),
      store (nf::userProgramDirectory (NF_COMPANY_NAME, NF_PRODUCT_NAME, userDirectoryOverride),
             fileExtension,
             maxProgramNameLength)
{
    store.refresh();
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

void ProgramManager::captureSnapshot()
{
    snapshot.capture (apvts.processor);
}

bool ProgramManager::isModified() const
{
    // **Every parameter, with no exclusion list.** Elmer has no momentary triggers - unlike TapeRot,
    // which must exclude STOP/FILTER/FAIL or holding one lights SAVE - and no mutually exclusive
    // selectors, so the compared set is simply everything.
    return snapshot.differsFrom (apvts.processor);
}


juce::File ProgramManager::getUserProgramDirectory() const
{
    return store.getDirectory();
}

juce::File ProgramManager::getDefaultUserProgramDirectory()
{
    // The per-OS resolution, the "Application Support" segment macOS alone needs, and the reason
    // ~/Library/Audio/Presets is the wrong answer are all in nf/UserProgramDirectory.h now. That
    // reasoning was carried in six near-identical comment blocks, and the one time it was wrong it
    // was wrong in all six at once.
    //
    // **There is deliberately no migration from the old location**, and that is a decision rather
    // than an omission. One was written and then removed: nothing in this suite has shipped at a
    // released version, so no installed build has ever written a Program to the old path for anyone
    // but us, and our own were disposable. Migration code guarding a case that cannot occur is dead
    // weight that still costs a directory probe on every rescan. If a version ever ships and the
    // path changes AGAIN, that is when this becomes necessary.
    return nf::userProgramDirectory (NF_COMPANY_NAME, NF_PRODUCT_NAME);
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
        if (store.fileFor (id) != juce::File())
            return { ProgramBank::user, id, id };

    // **Degrade honestly.** The restored values are correct and stay put; only the name is unknown.
    return { ProgramBank::unresolved, id, displayName.isNotEmpty() ? displayName : id };
}

std::vector<ProgramId> ProgramManager::listPrograms() const
{
    std::vector<ProgramId> out;
    out.reserve (1 + Elmer::factoryPrograms.size() + (size_t) store.getFiles().size());

    out.push_back (initId());

    for (size_t i = 0; i < Elmer::factoryPrograms.size(); ++i)
        out.push_back (factoryIdAt ((int) i));

    for (const auto& f : store.getFiles())
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



/*  **The critical section is a SWAP now, and it used to be two assignments.**

    A `juce::String` copy is a refcount increment and reads as safe. The ASSIGNMENT is the other
    half: it releases whatever the target held first, and a refcount reaching zero calls `free()`.
    So `pendingProgram = id` and `id = pendingProgram` each did heap work, and both were inside the
    lock — on a path VST3 can deliver **on the audio thread**, since a program change is an
    automatable parameter there.

    **Measured at 0.12 us worst case against a 10,667 us block budget**, so this was never a dropout
    risk and is not sold as one. It is negligible because a refcount release happens to be cheap,
    not because anything guarantees the path stays heap-free — and the next person to add a field to
    `ProgramId` has no reason to think about it.

    The copy and the destruction both move OUT of the lock: `exchangePendingProgram` takes its
    argument by value, so the caller's copy is made in the caller's frame, and returns the previous
    program by value, so its release happens in the caller's frame too. What is left between the
    lock and the unlock is a pointer exchange.

    **Named functions rather than inline blocks because that is what makes it testable.** An
    allocation sentinel is not lock-aware, so a probe around `requestProgramChange` sees the same
    total either way — the change is WHERE the work happens, not whether it happens. Arming the
    sentinel around a function that IS the critical section is the only honest way to assert it. */
ProgramId ProgramManager::exchangePendingProgram (ProgramId incoming)
{
    const juce::SpinLock::ScopedLockType lock (pendingLock);

    std::swap (pendingProgram, incoming);
    hasPendingProgram = true;

    return incoming;   // the PREVIOUS pending program; it is released in the caller's frame
}

bool ProgramManager::takePendingProgram (ProgramId& out)
{
    const juce::SpinLock::ScopedLockType lock (pendingLock);

    if (! hasPendingProgram)
        return false;

    // `out` is empty on entry, so this is a pointer exchange and nothing is released here.
    std::swap (out, pendingProgram);
    hasPendingProgram = false;

    return true;
}

void ProgramManager::requestProgramChange (const ProgramId& id)
{
    // The copy is made HERE, in this frame: copying a ProgramId is two refcount increments, and an
    // increment never frees. The previous pending program comes back and is released here too.
    const ProgramId previous = exchangePendingProgram (id);
    juce::ignoreUnused (previous);

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

    if (! takePendingProgram (id))
        return;

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
        const auto file = store.fileFor (id.id);

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
    // **What a Program CONTAINS stays here** - all nine parameters, since Elmer has no mutually
    // exclusive selectors to filter on. Core owns naming, the collision check and the write, and
    // takes finished XML.
    juce::XmlElement xml { "ElmerProgram" };
    xml.setAttribute ("schema", stateSchemaVersion);

    for (const auto* id : allParamIds)
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (id)))
            xml.setAttribute (id, (double) p->convertFrom0to1 (p->getValue()));

    // **The empty-name fallback is `TAKE n` now, not `UNTITLED`.** The suite had five different
    // ones across six castings; TAKE n is the one that is better rather than merely different,
    // since consecutive empty saves give TAKE 3, TAKE 4 instead of leaning on getNonexistentSibling
    // for "UNTITLED (2)". Upper-casing and the 22-character cap also apply on every path now - they
    // lived in ProgramHeader's keystroke filter alone, so any programmatic save bypassed both.
    const auto file = store.save (name, xml);

    if (file == juce::File())
        return getCurrentProgramId();   // the write failed; stay on the Program already showing

    // **The stem comes off the file core returned, not off the requested name.** A collision takes
    // the next free sibling, so taking it from the request would point the panel at the first file
    // while the values came from the second.
    const auto stem = file.getFileNameWithoutExtension();
    setCurrentId ({ ProgramBank::user, stem, stem });

    // The snapshot is re-taken against what was just written, so SAVE goes dark and the asterisk
    // clears the instant the Program exists. Without this the panel keeps claiming unsaved changes
    // against the PREVIOUS Program's baseline, immediately after saving.
    captureSnapshot();

    if (onProgramChanged != nullptr)
        onProgramChanged();

    return getCurrentProgramId();
}

void ProgramManager::deleteUserProgram (const ProgramId& id)
{
    // Gated on the BANK, which is stronger than the old index range: an id from any other bank
    // simply cannot address a file. Gated in the model, not only at the button.
    if (id.bank != ProgramBank::user)
        return;

    if (! store.remove (id.id))
        return;

    // Deliberately NOT the unresolved state: deleting from the panel is unambiguous intent.
    applyProgram (factoryIdAt (defaultFactoryProgramIndex));
}
