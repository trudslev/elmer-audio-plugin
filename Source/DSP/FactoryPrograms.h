#pragma once

#include <juce_core/juce_core.h>

#include <nf/ProgramId.h>

#include "../Parameters.h"

#include <array>

/** **Program identity comes from core, and these aliases are the whole of the local surface.**

    `nf::ProgramBank` and `nf::ProgramId` say what six identical copies used to say separately:
    INIT is its own bank rather than a magic index, identity is a permanent slug for a Factory
    Program and the filename stem for a User one, and `displayName` is carried for presentation
    only - a factory slug is not presentable, "under-pressure?" in the LCD would read as a
    rendering fault - and is deliberately outside `operator==`, so a corrected typo in the bank
    cannot make a Program stop equalling itself.

    Aliased rather than used qualified at every call site, because the unqualified names are what
    the panel, the header and the tests already read. */
using ProgramBank = nf::ProgramBank;
using ProgramId   = nf::ProgramId;

namespace Elmer
{

/**
    The 16 factory Programs.

    Stored in human-readable units - dB, Hz, ms, percent - and converted to parameter positions on
    apply, so this table can be read against the brief and against the panel without decoding
    normalised values. `sidechainHpHz == 0` means OFF.

    Every Program carries all nine values. Elmer has no mutually exclusive selectors, so unlike
    Fifth Member there is no active-path filtering and no zero-fill invariant here: a zero in this
    table is a real zero, not an absent field.

    These are starting values - directionally correct per each Program's intent rather than tuned by
    ear. A by-ear pass is expected once the build is audible.
*/
struct FactoryProgram
{
    /** **The permanent identity, fixed at creation and never changed again.** `name` is a label the
        designers may revise; `slug` may not be, because it is what a saved session stores. */
    const char* slug;

    const char* name;
    float thresholdDb;
    int   ratioIndex;      // into ratioNames
    int   kneeIndex;       // 0 soft, 1 hard
    float sidechainHpHz;   // 0 = OFF
    float attackMs;
    int   releaseIndex;    // 4 = AUTO
    float ironPercent;
    float makeupDb;
    float mixPercent;
};

inline constexpr int defaultFactoryProgramIndex = 0;

/** INIT's index. Negative on purpose: INIT sits OUTSIDE both banks rather than being Program 00, so
    it cannot be addressed by a bank index without pushing every Factory number up by one. Factory
    still starts at 01, which is what the display and the menu both print.

    The host never sees it - getNumPrograms() counts Factory + User only, so the DAW's own Program
    list is unchanged and INIT is reachable from the panel alone. That matches what it is: a place to
    start from, not a stored sound worth recalling by automation. */
inline constexpr int initProgramIndex = -1;

/** A blank canvas: the compressor present but not engaging, so raising any single control
    immediately shows what that control does.

    Threshold is above anything a bus will reach, ratio is the gentlest available, and everything
    that gives Elmer its character - Iron, sidechain filtering, makeup - sits at zero. **Mix stays at
    100 %**, which is not a character setting here: Elmer is serial, and Mix is its
    parallel-compression control. At anything less the compressor would be partly bypassed rather
    than idle, and the first knob the user moved would appear weaker than it is. */
inline constexpr FactoryProgram initProgram
    { "init", "INIT", 10.0f, 0, 0, 0.0f, 10.0f, 1, 0.0f, 0.0f, 100.0f };

inline constexpr std::array<FactoryProgram, 16> factoryPrograms { {
    { "under-pressure", "UNDER PRESSURE",     -14.0f, 1, 0,  75.0f, 10.0f,  4,  20.0f,  2.5f, 100.0f },
    { "art-of-glue", "ART OF GLUE",        -16.0f, 2, 0,  75.0f, 30.0f,  4,  25.0f,  3.5f, 100.0f },
    { "minneapolis-squeeze", "MINNEAPOLIS SQUEEZE",-18.0f, 2, 1, 140.0f,  3.0f,  1,  30.0f,  4.0f, 100.0f },
    { "blue-tuesday", "BLUE TUESDAY",       -20.0f, 2, 1, 140.0f, 10.0f,  0,  35.0f,  5.0f, 100.0f },
    { "sheffield-steel", "SHEFFIELD STEEL",    -18.0f, 2, 0,  75.0f, 10.0f,  1,  45.0f,  4.5f, 100.0f },
    { "jersey-bus", "JERSEY BUS",         -16.0f, 1, 0,  75.0f, 30.0f,  2,  40.0f,  3.0f, 100.0f },
    { "hammer-down", "HAMMER DOWN",        -20.0f, 2, 1, 140.0f,  3.0f,  1,  40.0f,  5.0f, 100.0f },
    { "queens-smash", "QUEENS SMASH",       -30.0f, 3, 1,   0.0f,  0.1f,  0,  55.0f,  8.0f,  40.0f },
    { "halfway-there", "HALFWAY THERE",      -18.0f, 2, 0,  40.0f, 10.0f,  1,  30.0f,  4.0f, 100.0f },
    { "kilimanjaro", "KILIMANJARO",        -16.0f, 1, 0,  40.0f, 30.0f,  4,  20.0f,  3.0f, 100.0f },
    { "west-end", "WEST END",           -20.0f, 2, 1,  75.0f,  1.0f,  1,  25.0f,  5.0f, 100.0f },
    { "bite-the-dust", "BITE THE DUST",      -18.0f, 2, 0,  75.0f,  3.0f,  0,  35.0f,  4.0f, 100.0f },
    { "dances-on-the-sand", "DANCES ON THE SAND", -16.0f, 1, 0,  40.0f, 10.0f,  1,  25.0f,  3.0f, 100.0f },
    { "pasadena", "PASADENA",           -18.0f, 2, 1, 140.0f,  3.0f,  1,  45.0f,  4.0f, 100.0f },
    { "dont-forget", "DON'T FORGET",       -12.0f, 0, 0,  75.0f, 30.0f,  4,  15.0f,  2.0f, 100.0f },
    { "pancake", "PANCAKE",            -38.0f, 4, 1,   0.0f,  0.1f,  0, 100.0f, 15.0f, 100.0f } } };

/** QUEENS SMASH is the only Program with Mix below 100 - the New York parallel setting, where a
    heavily crushed signal sits under the dry. If a second one ever appears below 100, it should be
    for the same deliberate reason. */
inline constexpr int parallelProgramIndex = 7;

} // namespace Elmer
