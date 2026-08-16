#pragma once

#include <vector>

#include "FactoryPrograms.h"

#include <nf/ParameterSnapshot.h>
#include <nf/UserProgramStore.h>

#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>

/**
    Factory and User Program banks, following Gatecrasher's architecture directly.

    - SAVE always creates a NEW named Program and never overwrites, even when a User Program is
      loaded. There is therefore no separate "New Program" action anywhere in the interface.
    - DELETE works only on User Programs and is gated at both the button and here in the model.
    - One .elmerprogram XML file per User Program, in a per-OS directory, sorted by filename.
    - A host can call setCurrentProgram from a non-message thread, so the actual apply is deferred
      through an AsyncUpdater.

    Unlike Fifth Member, every Program stores all nine parameters: Elmer has no mutually exclusive
    selectors, so there is no active path to filter and nothing persists across a Program change.
*/
class ProgramManager final : private juce::AsyncUpdater
{
public:
    /** @param userDirectoryOverride  where User Programs live. Defaults to the real per-OS
                                      location; a test passes a temporary directory so it never
                                      writes into the user's own Programs folder. */
    explicit ProgramManager (juce::AudioProcessorValueTreeState&,
                             juce::File userDirectoryOverride = {});
    ~ProgramManager() override;

    /** The Factory bank's size - what the host is told, and it never changes. */
    int getNumPrograms() const noexcept { return getNumFactoryPrograms(); }

    ProgramId getCurrentProgramId() const;
    static ProgramId factoryIdAt (int factoryPosition);
    static ProgramId initId();
    static int factoryPositionOf (const juce::String& slug);

    /** The Factory position of the current Program, or 0 when it is INIT, a User Program or
        unresolved - none of which the host's list contains. */
    int getCurrentFactoryPosition() const;

    ProgramId resolve (ProgramBank bank, const juce::String& id, const juce::String& displayName) const;
    std::vector<ProgramId> listPrograms() const;

    /** **What the LCD and the dropdown print - a label, not a key.** Only Factory Programs get the
        two-digit number, computed from their bank position at paint time. */
    juce::String displayLabelFor (const ProgramId& id) const;

    /** Applies a Program by identity. Safe from any thread - defers through the AsyncUpdater. */
    void requestProgramChange (const ProgramId& id);

    /*  **The pending-program handshake, and it is public so a test can reach it.**

        These two functions ARE the critical section: everything between taking `pendingLock` and
        releasing it happens inside them, and nothing else touches `pendingProgram`. An allocation
        sentinel is not lock-aware, so a probe around `requestProgramChange` cannot distinguish heap
        work under the lock from heap work beside it — the totals are identical either way. Arming
        it around a function that is exactly the locked region is the only honest way to assert the
        property, and that is worth the two names on this class.

        See their definitions for what moved out of the lock and why 0.12 us was never the argument. */
    ProgramId exchangePendingProgram (ProgramId incoming);
    bool takePendingProgram (ProgramId& out);

    int getNumFactoryPrograms() const { return (int) Elmer::factoryPrograms.size(); }
    juce::String getProgramName (int index) const;


    /** Restores WHICH Program is showing without touching a single parameter.

        Session restore has already put every value where it belongs; re-applying the Program on top
        would overwrite exactly what was just restored. This also re-takes the clean snapshot, so a
        session that was saved untouched reopens without a dirty asterisk over it. */
    void setCurrentProgramWithoutApplying (const ProgramId& id);

    /** Cancels a queued apply. Essential around setStateInformation: a change requested just before
        the restore would otherwise land just after it and overwrite everything restored. */
    void cancelPendingChange();

    /** Session-state attributes. **These three strings are a contract** - rename one and the
        session still parses while the Program silently reverts to the default, with no error
        anywhere. See ../../CLAUDE.md, "Parameters and saved state". */
    static constexpr const char* sessionSchemaAttribute  = "elmerSessionSchema";
    static constexpr const char* currentProgramAttribute = "elmerCurrentProgram";
    static constexpr int currentSessionSchemaVersion = 2;

    /** The schema at which the session stopped storing a positional index and started storing bank
        + identifier. */
    static constexpr int identitySchemaVersion = 2;

    /** **The oldest session schema whose values can still be interpreted, pinned to a literal.**
        Elmer has only ever had one, and the identity bump is purely additive.

        A literal on purpose: the gate read `!= currentSessionSchemaVersion`, which is correct
        exactly once - this bump would otherwise have discarded every existing session's parameter
        values over a change that alters no parameter's meaning. */
    static constexpr int oldestReadableSessionSchema = 1;

    /** The identity attributes, joining the contract above. `...ProgramName` is DISPLAY ONLY. */
    static constexpr const char* programBankAttribute = "elmerProgramBank";
    static constexpr const char* programIdAttribute   = "elmerProgramId";
    static constexpr const char* programNameAttribute = "elmerProgramName";

    /** Three outcomes, deliberately distinct: too old to interpret, too new to know about, usable. */
    enum class SchemaVerdict { tooOld, tooNew, readable };

    static SchemaVerdict classifySessionSchema (int savedSchema) noexcept
    {
        if (savedSchema < oldestReadableSessionSchema) return SchemaVerdict::tooOld;
        if (savedSchema > currentSessionSchemaVersion) return SchemaVerdict::tooNew;

        return SchemaVerdict::readable;
    }

    static juce::String bankAttributeValue (ProgramBank bank)
    {
        switch (bank)
        {
            case ProgramBank::init:       return "init";
            case ProgramBank::factory:    return "factory";
            case ProgramBank::user:       return "user";
            case ProgramBank::unresolved: return "unresolved";
        }

        return "factory";
    }

    static ProgramBank bankFromAttribute (const juce::String& value)
    {
        if (value == "init")       return ProgramBank::init;
        if (value == "user")       return ProgramBank::user;
        if (value == "unresolved") return ProgramBank::unresolved;

        return ProgramBank::factory;
    }

    /** Always creates a new Program; never overwrites. Selects and returns it. */
    ProgramId saveNewUserProgram (const juce::String& name);

    /** True once any parameter has moved since the current Program was applied.

        Drives both the display's dirty asterisk and SAVE's enablement, and the spec requires those
        two to agree always - so they read the same flag rather than each deciding for themselves.

        Compared against a snapshot taken from the LIVE APVTS at apply time, not rebuilt from the
        Program's own definition: that keeps applyFactory the single description of what a Program
        sets, with no second copy to drift out of step. */
    bool isModified() const;
    void deleteUserProgram (const ProgramId& id);

    /** Where this instance stores User Programs, and the real per-OS location regardless of it. */
    juce::File getUserProgramDirectory() const;
    static juce::File getDefaultUserProgramDirectory();

    /** **The store's cap, and it MUST equal `Layout::maxUserNameLength`.**

        22 = the 24-character cell less the dirty marker. The derivation is a display fact and lives
        with the display, in ElmerTheme.h - this class cannot include a GUI header, so the two are
        bound by `Tests/DisplayBudgetTests.cpp` asserting them equal rather than by an alias. A test
        that fails on divergence is stronger than a comment that does not.

        It is stated here at all because the cap used to be applied by the keystroke filter ALONE,
        so any programmatic save bypassed it - and a name longer than the cell is one the panel
        cannot show. */
    static constexpr int maxProgramNameLength = 22;


    std::function<void()> onProgramChanged;

private:
    void handleAsyncUpdate() override;
    bool applyUserFile (const juce::File& file);
    void setParam (const char* id, float actualValue);
    void applyProgramValues (const Elmer::FactoryProgram& p);
    void applyProgram (const ProgramId& id);
    void setCurrentId (const ProgramId& id);
    void captureSnapshot();

    juce::AudioProcessorValueTreeState& apvts;

    /** The User bank on disk. Scanning, sorting, naming, the collision check, save and delete are
        core's; WHAT a Program contains - all nine parameters, since Elmer has no selectors to
        filter on - stays here. */
    nf::UserProgramStore store;
    mutable juce::SpinLock currentIdLock;
    ProgramId currentId;

    // "Nothing pending" is its own flag now rather than a sentinel value.
    juce::SpinLock pendingLock;

    bool hasPendingProgram = false;
    ProgramId pendingProgram;

    // The SpinLock that used to sit here is core's now: setStateInformation carries no thread
    // guarantee and the GUI polls the flag on the message thread, and four of the six castings had
    // that unguarded. See nf/ParameterSnapshot.h.
    nf::ParameterSnapshot snapshot;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgramManager)
};
