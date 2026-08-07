#pragma once

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
    bool isFactory (int index) const { return index < getNumFactoryPrograms(); }

    void setCurrentProgram (int index);

    /** Always creates a new Program; never overwrites. Returns its index. */
    int saveNewUserProgram (const juce::String& name);
    void deleteUserProgram (int index);

    static juce::File getUserProgramDirectory();

    std::function<void()> onProgramChanged;

private:
    void handleAsyncUpdate() override;
    void applyFactory (int index);
    void applyUser (int index);
    void rescanUserPrograms();
    void setParam (const char* id, float actualValue);

    juce::AudioProcessorValueTreeState& apvts;
    juce::Array<juce::File> userFiles;
    int currentIndex = Elmer::defaultFactoryProgramIndex;
    std::atomic<int> pendingIndex { -1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgramManager)
};
