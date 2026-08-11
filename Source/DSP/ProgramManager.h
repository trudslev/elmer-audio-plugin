#pragma once

#include <vector>

#include "FactoryPrograms.h"

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
    explicit ProgramManager (juce::AudioProcessorValueTreeState&);
    ~ProgramManager() override;

    int getNumPrograms() const;
    int getNumFactoryPrograms() const { return (int) Elmer::factoryPrograms.size(); }
    int getCurrentProgram() const noexcept { return currentIndex; }
    juce::String getProgramName (int index) const;
    juce::String getDisplayName (int index) const;
    bool isFactory (int index) const { return index >= 0 && index < getNumFactoryPrograms(); }
    static bool isInit (int index) noexcept { return index == Elmer::initProgramIndex; }

    void setCurrentProgram (int index);

    /** Restores WHICH Program is showing without touching a single parameter.

        Session restore has already put every value where it belongs; re-applying the Program on top
        would overwrite exactly what was just restored. This also re-takes the clean snapshot, so a
        session that was saved untouched reopens without a dirty asterisk over it. */
    void setCurrentProgramIndexWithoutApplying (int index);

    /** Cancels a queued apply. Essential around setStateInformation: a change requested just before
        the restore would otherwise land just after it and overwrite everything restored. */
    void cancelPendingChange();

    /** Session-state attributes. **These three strings are a contract** - rename one and the
        session still parses while the Program silently reverts to the default, with no error
        anywhere. See ../../CLAUDE.md, "Parameters and saved state". */
    static constexpr const char* sessionSchemaAttribute  = "elmerSessionSchema";
    static constexpr const char* currentProgramAttribute = "elmerCurrentProgram";
    static constexpr int currentSessionSchemaVersion = 1;

    /** Always creates a new Program; never overwrites. Returns its index. */
    int saveNewUserProgram (const juce::String& name);

    /** True once any parameter has moved since the current Program was applied.

        Drives both the display's dirty asterisk and SAVE's enablement, and the spec requires those
        two to agree always - so they read the same flag rather than each deciding for themselves.

        Compared against a snapshot taken from the LIVE APVTS at apply time, not rebuilt from the
        Program's own definition: that keeps applyFactory the single description of what a Program
        sets, with no second copy to drift out of step. */
    bool isModified() const;
    void deleteUserProgram (int index);

    static juce::File getUserProgramDirectory();


    std::function<void()> onProgramChanged;

private:
    void handleAsyncUpdate() override;
    void applyFactory (int index);
    void applyUser (int index);
    void rescanUserPrograms();
    void setParam (const char* id, float actualValue);
    void applyProgramValues (const Elmer::FactoryProgram& p);
    void captureSnapshot();

    static const juce::StringArray& snapshotParamIds();

    juce::AudioProcessorValueTreeState& apvts;
    juce::Array<juce::File> userFiles;
    int currentIndex = Elmer::defaultFactoryProgramIndex;
    std::atomic<int> pendingIndex { -2 };        // -1 is INIT, so "nothing pending" cannot be -1

    // Guarded because setStateInformation can arrive on any thread while the GUI polls the flag on
    // the message thread.
    mutable juce::SpinLock snapshotLock;
    std::vector<float> snapshot;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgramManager)
};
