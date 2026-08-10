#pragma once

#include "../Parameters.h"

#include <array>

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
    { "INIT", 10.0f, 0, 0, 0.0f, 10.0f, 1, 0.0f, 0.0f, 100.0f };

inline constexpr std::array<FactoryProgram, 16> factoryPrograms { {
    { "UNDER PRESSURE",     -14.0f, 1, 0,  75.0f, 10.0f,  4,  20.0f,  2.5f, 100.0f },
    { "ART OF GLUE",        -16.0f, 2, 0,  75.0f, 30.0f,  4,  25.0f,  3.5f, 100.0f },
    { "MINNEAPOLIS SQUEEZE",-18.0f, 2, 1, 140.0f,  3.0f,  1,  30.0f,  4.0f, 100.0f },
    { "BLUE TUESDAY",       -20.0f, 2, 1, 140.0f, 10.0f,  0,  35.0f,  5.0f, 100.0f },
    { "SHEFFIELD STEEL",    -18.0f, 2, 0,  75.0f, 10.0f,  1,  45.0f,  4.5f, 100.0f },
    { "JERSEY BUS",         -16.0f, 1, 0,  75.0f, 30.0f,  2,  40.0f,  3.0f, 100.0f },
    { "HAMMER DOWN",        -20.0f, 2, 1, 140.0f,  3.0f,  1,  40.0f,  5.0f, 100.0f },
    { "QUEENS SMASH",       -30.0f, 3, 1,   0.0f,  0.1f,  0,  55.0f,  8.0f,  40.0f },
    { "HALFWAY THERE",      -18.0f, 2, 0,  40.0f, 10.0f,  1,  30.0f,  4.0f, 100.0f },
    { "KILIMANJARO",        -16.0f, 1, 0,  40.0f, 30.0f,  4,  20.0f,  3.0f, 100.0f },
    { "WEST END",           -20.0f, 2, 1,  75.0f,  1.0f,  1,  25.0f,  5.0f, 100.0f },
    { "BITE THE DUST",      -18.0f, 2, 0,  75.0f,  3.0f,  0,  35.0f,  4.0f, 100.0f },
    { "DANCES ON THE SAND", -16.0f, 1, 0,  40.0f, 10.0f,  1,  25.0f,  3.0f, 100.0f },
    { "PASADENA",           -18.0f, 2, 1, 140.0f,  3.0f,  1,  45.0f,  4.0f, 100.0f },
    { "DON'T FORGET",       -12.0f, 0, 0,  75.0f, 30.0f,  4,  15.0f,  2.0f, 100.0f },
    { "PANCAKE",            -38.0f, 4, 1,   0.0f,  0.1f,  0, 100.0f, 15.0f, 100.0f } } };

/** QUEENS SMASH is the only Program with Mix below 100 - the New York parallel setting, where a
    heavily crushed signal sits under the dry. If a second one ever appears below 100, it should be
    for the same deliberate reason. */
inline constexpr int parallelProgramIndex = 7;

} // namespace Elmer
