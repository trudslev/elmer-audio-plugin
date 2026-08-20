juce_add_console_app(ElmerTests PRODUCT_NAME "ElmerTests")

# A new DSP .cpp goes in BOTH this list and target_sources(Elmer ...) in the parent CMakeLists,
# or it silently drops out of test coverage.
target_sources(ElmerTests PRIVATE
    RealtimeSafetyTests.cpp
    CpuBaselineTests.cpp
    NumericalRobustnessTests.cpp
    InvarianceTests.cpp
    AutomationLatencyTests.cpp
    ChannelLayoutTests.cpp
    PendingProgramLockTests.cpp
    TestMain.cpp
    KnobRenderTests.cpp
    KnobLayoutTests.cpp
    MenuMetricsTests.cpp
    PrintedScaleTests.cpp
    ReadoutConformanceTests.cpp
    EditorWiringTests.cpp
    CompressorTests.cpp
    FactoryProgramTests.cpp
    DisplayBudgetTests.cpp
    ../Source/DSP/SidechainFilter.cpp
    ../Source/DSP/LevelDetector.cpp
    ../Source/DSP/GainComputer.cpp
    ../Source/DSP/AutoRelease.cpp
    ../Source/DSP/IronStage.cpp
    ../Source/DSP/OutputStage.cpp
    ../Source/DSP/ProgramManager.cpp
    # **The REAL processor and the REAL editor, not stand-ins.** Everything above this line is what
    # core does not own at the DSP layer; everything below it is the wiring and the panel, which no
    # test could reach while this target compiled DSP alone.
    #
    # That gap was not theoretical. Reflect-84 shipped the stale-replay guard with ZERO call sites
    # for its disarm, and no suite could have seen it: the defect lived in an editor .cpp that was
    # not compiled into anything a test could run.
    ../Source/PluginProcessor.cpp
    ../Source/PluginEditor.cpp
    ../Source/GUI/ElmerEditorContent.cpp
    ../Source/GUI/ProgramHeader.cpp
    ../Source/GUI/PanelBackground.cpp
    ../Source/GUI/KnobFilmstrip.cpp
    RotaryArcTests.cpp
    ../Source/GUI/KneeButtons.cpp
    ../Source/GUI/GainReductionMeter.cpp)

# The same identity macros the plugin target gets. This target compiles ProgramManager.cpp
# directly rather than linking the plugin, so without them the user-Programs path would not
# compile here - and that is the point of taking them from CMake rather than hand-copying.
#
# The JucePlugin_* pair and NF_VERSION_SHORT arrived with the processor and panel sources: a console
# app gets none of the plugin target's generated definitions, and PluginProcessor uses
# JucePlugin_Name for getName() while PanelBackground prints the version stamp. Inherited from the
# same parent variables the plugin target uses, so there is nothing to re-sync — a hand-synced copy
# in CHORUS-60 once drifted to a dead company name and quietly pointed saved Programs at a directory
# nothing reads.
# **The core commit this casting was pinned at, resolved at CONFIGURE time and baked into the CPU
# test's provenance.** The tag moves while core is pre-release, so `v1.0.0` in a baseline file would
# record nothing — the whole value of the provenance block is that a figure can be attributed to one
# state of six castings and one state of core, which is what makes "one sitting, all six, one machine
# state" checkable after the fact rather than a promise.
execute_process(
    COMMAND git -C "${neonfoundrycore_SOURCE_DIR}" rev-parse --short HEAD
    OUTPUT_VARIABLE NF_CORE_PIN_COMMIT_VALUE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)
if(NOT NF_CORE_PIN_COMMIT_VALUE)
    set(NF_CORE_PIN_COMMIT_VALUE "unknown")
endif()

target_compile_definitions(ElmerTests PRIVATE
    NF_CORE_PIN_COMMIT="${NF_CORE_PIN_COMMIT_VALUE}"
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STANDALONE_APPLICATION=1
    NF_COMPANY_NAME="${ELMER_COMPANY_NAME}"
    NF_PRODUCT_NAME="${ELMER_PRODUCT_NAME}"
    JucePlugin_Name="${ELMER_PRODUCT_NAME}"
    JucePlugin_Manufacturer="${ELMER_COMPANY_NAME}"
    NF_VERSION_SHORT="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

target_include_directories(ElmerTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../Source)

# ElmerBinaryData, because DisplayBudgetTests asserts against ElmerTheme.h and the theme header
# includes BinaryData.h for the embedded faces. Linking the real generated header rather than
# stubbing it is the point: a token the tests check has to be the token the panel draws with.
target_link_libraries(ElmerTests PRIVATE
    # The bug sweep's measurement drivers. TEST target only — never the plugin, which must not
    # carry an operator new override. See nf/testing/ProcessorHarness.h.
    nf::testing
    ElmerBinaryData
    # ProgramManager.cpp is compiled into both targets and calls into core, so both link it.
    nf::core
    juce::juce_dsp
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_core
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)
