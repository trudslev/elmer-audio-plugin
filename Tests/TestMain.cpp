#include <juce_audio_processors/juce_audio_processors.h>
#include <nf/UserProgramDirectory.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <iostream>

/*  **JUCE's default logger writes to OutputDebugString on Windows, so every line this suite logs
    has been invisible in Windows CI.**

    `Logger::outputDebugString` is `std::cerr << text` on POSIX and `OutputDebugString (...)` on
    Windows (`juce_win32_Misc.cpp`), which goes to an attached debugger and nowhere else. The target
    is a console app, so this is not a subsystem problem — it is which sink JUCE picked.

    The effect was a Windows test step that printed **8 lines against macOS's 94**: no `Random seed`,
    no `Starting tests in`, and a green job. It passes silently and fails loudly — `TestMain` writes
    its summary to `std::cerr` and returns non-zero — so a real failure would still go red. But
    "green with no evidence" is the state this repository distrusts everywhere else, and it is not
    something a reader can distinguish from a step that never ran the binary at all.

    One logger, every platform, one stream. */
struct ConsoleLogger final : juce::Logger
{
    void logMessage (const juce::String& message) override
    {
        std::cout << message << std::endl;
    }
};

int main (int, char**)
{
    ConsoleLogger consoleLogger;
    juce::Logger::setCurrentLogger (&consoleLogger);

    // Without a MessageManager, AsyncUpdater::triggerAsyncUpdate() silently clears its own pending
    // flag - every Program test would then pass while proving nothing. Gatecrasher and Fifth Member
    // both paid for this.
    juce::ScopedJuceInitialiser_GUI juceInit;

    // **Every suite in this process resolves User Programs under a scratch directory, not the
    // user's real one.** The test harness compiles the shipping AudioProcessor, which builds its
    // ProgramManager from the real per-OS path because that is its job — so without this, any test
    // constructing the processor can reach
    // ~/Library/Application Support/<Company>/<Product>/Programs.
    //
    // A comment saying "do not write there" is a convention, and a convention gets broken silently.
    // It is also the one most likely to be broken by someone doing the right thing: verifying the
    // Program list needs several saved Programs, and building that state by hand is the obvious way
    // to get it. A cleanup glob has already destroyed a Program a user had just saved.
    //
    // Installed before the runner for the same reason ScopedJuceInitialiser_GUI is: it has to be in
    // force before the first line of the first test. See nf/UserProgramDirectory.h.
    const nf::ScopedUserProgramDirectoryOverride programRedirect {
        juce::File::getSpecialLocation (juce::File::tempDirectory)
            .getChildFile ("NeonFoundryTestPrograms")
    };

    juce::UnitTestRunner runner;
    runner.setAssertOnFailure (false);
    runner.runAllTests();

    int failures = 0;

    for (int i = 0; i < runner.getNumResults(); ++i)
        failures += runner.getResult (i)->failures;

    if (failures > 0)
        std::cerr << "\n*** " << failures << " test failure(s)\n";

    juce::Logger::setCurrentLogger (nullptr);   // it is a stack object; do not outlive it

    return failures > 0 ? 1 : 0;
}
