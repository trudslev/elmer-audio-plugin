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

project(Elmer VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.14
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

# ---------------------------------------------------------------------------------------------
# neon-foundry-core: shared BEHAVIOUR across the suite, pinned by tag exactly like JUCE above.
#
# **This is not a dependency on a sibling casting.** Depending on one of those is still forbidden -
# it couples two things that must ship on their own schedules. This is a versioned library that no
# casting owns, so a change to it forces nothing on anyone until they move their own pin, which is
# the same relationship every casting already has with JUCE. The root CLAUDE.md draws that
# distinction under "Structure".
#
# **Declared AFTER JUCE, and that ordering is load-bearing.** Core links juce::juce_core and does
# not fetch its own when consumed; it fails with an explicit message if JUCE is not already
# available. Two JUCE trees in one build link two juce_core builds into one binary and surface as
# duplicate symbols a long way from the cause.
#
FetchContent_Declare(NeonFoundryCore
    GIT_REPOSITORY https://github.com/trudslev/neon-foundry-core.git
    GIT_TAG v1.0.0
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(NeonFoundryCore)


# The panel is bitmap-composited: the fascia is drawn in code, and these production assets sit on
# top of it. There is deliberately no dressed-panel render in here - Gatecrasher shipped one as its
# background and spent a release chasing bugs caused by every live control sitting over a baked copy
# of itself. design/screenshots/panel.png is an acceptance target, not a runtime asset.
juce_add_binary_data(ElmerBinaryData
    SOURCES
        # The three 128-frame knob sheets are RETIRED — §3.1 draws the skirt, cap, pointer, arc
        # and ticks in code, and bundle 3's opening makes the meter face and needle the only
        # bitmaps on this panel. They stay in design/assets/ as the reference the code was drawn
        # against; what is removed is embedding them in a binary nothing reads them from.
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

# Single source of truth for the two identity strings that appear BOTH in the bundle metadata and
# in ProgramManager's user-Programs path. ProgramManager cannot read JUCE's JucePlugin_* macros -
# those exist only in the plugin target's generated header, and that file is also compiled into the
# Tests console app - so it used to carry hand-written copies. CHORUS-60's did not stay in sync:
# COMPANY_NAME became "Neon Foundry" while the copy still said "Tanis", quietly pointing saved
# Programs at a dead directory. CMake hands both targets the same values, so drift is a build error.
set(ELMER_COMPANY_NAME "Neon Foundry")
set(ELMER_PRODUCT_NAME "Elmer")

juce_add_plugin(Elmer
    PRODUCT_NAME "${ELMER_PRODUCT_NAME}"
    COMPANY_NAME "${ELMER_COMPANY_NAME}"
    BUNDLE_ID "com.neonfoundry.elmer"
    # **Without this the Standalone is silent and the meters look broken.** An app whose Info.plist
    # carries no NSMicrophoneUsageDescription is never granted capture access on macOS - the OS does
    # not prompt and does not error, the input stream simply delivers zeros. On an effect that reads
    # as dead metering: IN sits at its floor and OUT follows, because there is nothing to process.
    # The meters are wired correctly; they have nothing to show.
    #
    # Plugin formats are unaffected - the host owns the input there - so this only surfaces when
    # someone opens the Standalone to listen.
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "ELMER needs audio input so the Standalone can compress what you play into it."
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
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/design/icons/elmer-icon-1024.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/design/icons/elmer-icon-256.png"
    ${ELMER_EXTRA_ARGS})

target_sources(Elmer PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/DSP/SidechainFilter.cpp
    Source/DSP/LevelDetector.cpp
    Source/DSP/GainComputer.cpp
    Source/DSP/AutoRelease.cpp
    Source/DSP/IronStage.cpp
    Source/DSP/OutputStage.cpp
    Source/DSP/ProgramManager.cpp
    Source/GUI/KnobFilmstrip.cpp
    Source/GUI/GainReductionMeter.cpp
    Source/GUI/KneeButtons.cpp
    Source/GUI/PanelBackground.cpp
    Source/GUI/ProgramHeader.cpp
    Source/GUI/ElmerEditorContent.cpp)

target_compile_definitions(Elmer PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_compile_definitions(Elmer PUBLIC
    NF_VERSION_SHORT="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
    NF_COMPANY_NAME="${ELMER_COMPANY_NAME}"
    NF_PRODUCT_NAME="${ELMER_PRODUCT_NAME}")

target_link_libraries(Elmer
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        ElmerBinaryData
        # ProgramManager.cpp is compiled into both this target and the tests, and calls into core,
        # so both link it.
        nf::core
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

add_subdirectory(Tests)
