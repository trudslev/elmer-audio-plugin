juce_add_console_app(ElmerTests PRODUCT_NAME "ElmerTests")

# A new DSP .cpp goes in BOTH this list and target_sources(Elmer ...) in the parent CMakeLists,
# or it silently drops out of test coverage.
target_sources(ElmerTests PRIVATE
    TestMain.cpp
    PrintedScaleTests.cpp
    CompressorTests.cpp
    FactoryProgramTests.cpp
    DisplayBudgetTests.cpp
    ../Source/DSP/SidechainFilter.cpp
    ../Source/DSP/LevelDetector.cpp
    ../Source/DSP/GainComputer.cpp
    ../Source/DSP/AutoRelease.cpp
    ../Source/DSP/IronStage.cpp
    ../Source/DSP/OutputStage.cpp
    ../Source/DSP/ProgramManager.cpp)

# The same identity macros the plugin target gets. This target compiles ProgramManager.cpp
# directly rather than linking the plugin, so without them the user-Programs path would not
# compile here - and that is the point of taking them from CMake rather than hand-copying.
target_compile_definitions(ElmerTests PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STANDALONE_APPLICATION=1
    NF_COMPANY_NAME="${ELMER_COMPANY_NAME}"
    NF_PRODUCT_NAME="${ELMER_PRODUCT_NAME}")

target_include_directories(ElmerTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../Source)

# ElmerBinaryData, because DisplayBudgetTests asserts against ElmerTheme.h and the theme header
# includes BinaryData.h for the embedded faces. Linking the real generated header rather than
# stubbing it is the point: a token the tests check has to be the token the panel draws with.
target_link_libraries(ElmerTests PRIVATE
    ElmerBinaryData
    juce::juce_dsp
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_core
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)
