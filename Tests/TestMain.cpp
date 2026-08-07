#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

int main (int, char**)
{
    // Without a MessageManager, AsyncUpdater::triggerAsyncUpdate() silently clears its own pending
    // flag - every Program test would then pass while proving nothing. Gatecrasher and Fifth Member
    // both paid for this.
    juce::ScopedJuceInitialiser_GUI juceInit;

    juce::UnitTestRunner runner;
    runner.setAssertOnFailure (false);
    runner.runAllTests();

    int failures = 0;

    for (int i = 0; i < runner.getNumResults(); ++i)
        failures += runner.getResult (i)->failures;

    if (failures > 0)
        std::cerr << "\n*** " << failures << " test failure(s)\n";

    return failures > 0 ? 1 : 0;
}
