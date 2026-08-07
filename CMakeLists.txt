cmake_minimum_required(VERSION 3.24)

# LANGUAGES needs C as well as CXX: JUCE and the fetched VST3 SDK bundle .c sources. Xcode and
# Visual Studio tolerate that without an explicit enable_language(C); Linux's single-config
# generators do not, and fail at generate time with "CMAKE_C_COMPILE_OBJECT not set".
#
# The two macOS cache entries MUST come before project(), which creates them itself - after that a
# plain set(... CACHE ...) is a silent no-op and the build quietly inherits the host architecture.
# TapeRot shipped an arm64-only .pkg once for exactly this reason.
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Universal: Apple Silicon + Intel")
endif()

project(Elmer VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.14
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

# The panel is bitmap-composited: the fascia is drawn in code, and these production assets sit on
# top of it. There is deliberately no dressed-panel render in here - Gatecrasher shipped one as its
# background and spent a release chasing bugs caused by every live control sitting over a baked copy
# of itself. design/screenshots/panel.png is an acceptance target, not a runtime asset.
juce_add_binary_data(ElmerBinaryData
    SOURCES
        design/assets/knob-detect-128.png
        design/assets/knob-timing-128.png
        design/assets/knob-output-128.png
        design/assets/meter-face.png
        design/assets/meter-needle.png
        design/assets/scale-lg.png
        design/assets/scale-sm.png
        design/assets/scale-5.png
        design/fonts/archivo-black/ArchivoBlack-Regular.ttf
        design/fonts/barlow-condensed/BarlowCondensed-SemiBold.ttf
        design/fonts/ibm-plex-mono/IBMPlexMono-Regular.ttf
        design/fonts/ibm-plex-mono/IBMPlexMono-Medium.ttf
        design/fonts/ibm-plex-mono/IBMPlexMono-SemiBold.ttf
        design/fonts/permanent-marker/PermanentMarker-Regular.ttf)

# AU is Apple-only. VST3_COPY_DIR/AU_COPY_DIR are set only on Apple - leaving them unset elsewhere
# lets JUCE apply its own correct per-OS default rather than a macOS path built on $ENV{HOME}.
if(APPLE)
    set(ELMER_FORMATS AU VST3 Standalone)
    set(ELMER_EXTRA_ARGS
        VST3_COPY_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/VST3"
        AU_COPY_DIR   "$ENV{HOME}/Library/Audio/Plug-Ins/Components")
else()
    set(ELMER_FORMATS VST3 Standalone)
    set(ELMER_EXTRA_ARGS "")
endif()

juce_add_plugin(Elmer
    PRODUCT_NAME "Elmer"
    COMPANY_NAME "Neon Foundry"
    BUNDLE_ID "com.neonfoundry.elmer"
    PLUGIN_MANUFACTURER_CODE Nfdy
    PLUGIN_CODE Gl87
    FORMATS ${ELMER_FORMATS}
    VST3_CATEGORIES Fx Dynamics
    AU_MAIN_TYPE kAudioUnitType_Effect
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    # JUCE derives every platform icon from these two. The macOS .icns takes one entry per file at
    # the largest exact size of 16/32/64/128/256/512/1024 that fits, so both must be EXACTLY square
    # at a valid size. Both are generated at CONFIGURE time and the PNGs are not configure
    # dependencies - after changing the artwork, re-run cmake -B build or the old icon ships.
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/design/assets/icons/elmer-icon-1024.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/design/assets/icons/elmer-icon-256.png"
    ${ELMER_EXTRA_ARGS})

target_sources(Elmer PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(Elmer PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_compile_definitions(Elmer PUBLIC
    NF_VERSION_SHORT="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

target_link_libraries(Elmer
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        ElmerBinaryData
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

add_subdirectory(Tests)
