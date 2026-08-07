juce_add_console_app(ElmerTests PRODUCT_NAME "ElmerTests")

# A new DSP .cpp goes in BOTH this list and target_sources(Elmer ...) in the parent CMakeLists,
# or it silently drops out of test coverage.
target_sources(ElmerTests PRIVATE
    TestMain.cpp
    PrintedScaleTests.cpp
    CompressorTests.cpp
    ../Source/DSP/SidechainFilter.cpp
    ../Source/DSP/LevelDetector.cpp
    ../Source/DSP/GainComputer.cpp
    ../Source/DSP/AutoRelease.cpp
    ../Source/DSP/IronStage.cpp
    ../Source/DSP/OutputStage.cpp)

target_compile_definitions(ElmerTests PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STANDALONE_APPLICATION=1)

target_include_directories(ElmerTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../Source)

target_link_libraries(ElmerTests PRIVATE
    juce::juce_dsp
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_core
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)
