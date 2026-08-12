#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <array>
#include <cmath>

/**
    Elmer's nine parameters: IDs, ranges, laws and defaults, in one place.

    Every control on this panel is always live. There are no mutually exclusive selectors that make
    one parameter irrelevant, so unlike Fifth Member there is no active-path filtering here and every
    Program stores all nine values. Do not port that machinery across.

    The two logarithmic laws are transcribed from `design/GUI-SPEC.md` rather than approximated with a
    juce::NormalisableRange skew, because BRAND.md makes the printed scale a correctness requirement:
    "the pointer sitting on a printed mark must report that value". `PrintedScale` below carries every
    mark the panel prints, and `Tests/PrintedScaleTests.cpp` asserts the mapping against it.
*/
namespace ParamIDs
{
    inline constexpr auto threshold   = "threshold";
    inline constexpr auto ratio       = "ratio";
    inline constexpr auto knee        = "knee";
    inline constexpr auto sidechainHp = "sidechainHp";
    inline constexpr auto attack      = "attack";
    inline constexpr auto release     = "release";
    inline constexpr auto iron        = "iron";
    inline constexpr auto makeup      = "makeup";
    inline constexpr auto mix         = "mix";
}

namespace Elmer
{

//==============================================================================
enum class Knee   { soft = 0, hard };
enum class Ratio  { r1_5 = 0, r2, r4, r10, r20 };
enum class Release{ r0_1 = 0, r0_3, r0_6, r1_2, autoMode };

inline constexpr std::array<float, 5> ratioValues { { 1.5f, 2.0f, 4.0f, 10.0f, 20.0f } };

/** The four fixed release detents in seconds. The fifth position is AUTO and has no fixed time -
    it is a program-dependent dual time constant, not a preset rate. */
inline constexpr std::array<float, 4> releaseSeconds { { 0.1f, 0.3f, 0.6f, 1.2f } };

inline const juce::StringArray ratioNames   { "1.5:1", "2:1", "4:1", "10:1", "20:1" };
inline const juce::StringArray kneeNames    { "Soft", "Hard" };
inline const juce::StringArray releaseNames { "0.1 s", "0.3 s", "0.6 s", "1.2 s", "AUTO" };

//==============================================================================
namespace Law
{
    /** SIDECHAIN HP, from the design: `f < 0.10` is OFF, otherwise
        `40 * 12.5^clamp((f - 0.2)/0.8, 0, 1)` Hz. Returns 0 for OFF.

        This parameter stores the KNOB POSITION, not the frequency, and that is deliberate. The
        control has a dead zone - its first tenth is OFF and its next tenth is clamped to 40 Hz - so
        a frequency-valued parameter could not represent where the pointer actually is. Storing Hz
        would collapse the whole OFF zone onto one position and the knob could never rest inside it.
        Every other continuous control here stores real units; this one cannot. */
    inline float hpFrequencyHz (float position01) noexcept
    {
        if (position01 < 0.10f)
            return 0.0f;

        const float t = juce::jlimit (0.0f, 1.0f, (position01 - 0.2f) / 0.8f);
        return 40.0f * std::pow (12.5f, t);
    }

    inline bool hpIsOff (float position01) noexcept  { return position01 < 0.10f; }

    /** The inverse, for the factory table, which stores readable frequencies rather than positions.
        0 Hz means OFF and maps to position 0. */
    inline float hpPositionForHz (float hz) noexcept
    {
        if (hz <= 0.0f)
            return 0.0f;

        return juce::jlimit (0.2f, 1.0f,
                             0.2f + 0.8f * std::log (hz / 40.0f) / std::log (12.5f));
    }

    /** ATTACK: `0.1 * 300^f` ms, spanning 0.1 to 30 ms. */
    inline float attackMsFromPosition (float position01) noexcept
    {
        return 0.1f * std::pow (300.0f, juce::jlimit (0.0f, 1.0f, position01));
    }

    inline float attackPositionFromMs (float ms) noexcept
    {
        return std::log (juce::jlimit (0.1f, 30.0f, ms) / 0.1f) / std::log (300.0f);
    }
}

//==============================================================================
/** Every value the panel prints around a knob, with the normalised position of the tick it sits on.

    Tick counts are chosen so a mark falls on every printed value: the 11-mark ring (`scale-lg`)
    prints on every second mark, giving 0, .2, .4, .6, .8, 1; the 9-mark ring (`scale-sm`) likewise
    gives 0, .25, .5, .75, 1; RELEASE's 5-mark ring prints on all five.

    ATTACK is the one place where print and law do not land on the same number to full precision.
    `0.1 * 300^f` puts the 0.4 mark at 0.979 ms under a printed "1", and the 0.8 mark at 9.59 ms
    under a printed "10" - about 4 %. The printed sequence 0.1/0.3/1/3/10/30 is the conventionally
    rounded form of the exact geometric series 0.1/0.313/0.979/3.06/9.58/30, which is how real
    hardware is marked. The law is the design's and is kept; the test allows 5 % on this control and
    demands exactness on the rest. */
namespace PrintedScale
{
    struct Mark { float position01; float printedValue; };

    inline constexpr std::array<Mark, 6> threshold { {
        { 0.0f, -40.0f }, { 0.2f, -30.0f }, { 0.4f, -20.0f },
        { 0.6f, -10.0f }, { 0.8f, 0.0f },   { 1.0f, 10.0f } } };

    /** Hz. The first mark is OFF and is checked separately - it has no numeric value. */
    inline constexpr std::array<Mark, 5> sidechainHp { {
        { 0.2f, 40.0f }, { 0.4f, 75.0f }, { 0.6f, 140.0f },
        { 0.8f, 265.0f }, { 1.0f, 500.0f } } };

    inline constexpr std::array<Mark, 6> attackMs { {
        { 0.0f, 0.1f }, { 0.2f, 0.3f }, { 0.4f, 1.0f },
        { 0.6f, 3.0f }, { 0.8f, 10.0f }, { 1.0f, 30.0f } } };

    inline constexpr std::array<Mark, 5> iron { {
        { 0.0f, 0.0f }, { 0.25f, 25.0f }, { 0.5f, 50.0f }, { 0.75f, 75.0f }, { 1.0f, 100.0f } } };

    inline constexpr std::array<Mark, 5> makeupDb { {
        { 0.0f, 0.0f }, { 0.25f, 5.0f }, { 0.5f, 10.0f }, { 0.75f, 15.0f }, { 1.0f, 20.0f } } };

    inline constexpr std::array<Mark, 5> mix { {
        { 0.0f, 0.0f }, { 0.25f, 25.0f }, { 0.5f, 50.0f }, { 0.75f, 75.0f }, { 1.0f, 100.0f } } };

    /** RATIO and RELEASE are detented: the printed value at index i is simply position i/(n-1). */
    inline constexpr float detentPosition (int index, int count) noexcept
    {
        return count > 1 ? (float) index / (float) (count - 1) : 0.0f;
    }
}

//==============================================================================
namespace Defaults
{
    inline constexpr float threshold   = -12.0f;   // dB
    inline constexpr int   ratio       = 2;        // 4:1
    inline constexpr int   knee        = 0;        // Soft
    inline constexpr float sidechainHp = 0.0f;     // OFF
    inline constexpr float attackMs    = 10.0f;
    inline constexpr int   release     = 1;        // 0.3 s
    inline constexpr float iron        = 20.0f;    // %
    inline constexpr float makeup      = 0.0f;     // dB
    inline constexpr float mix         = 100.0f;   // %
}

//==============================================================================
/** Builds the APVTS layout.

    These defaults govern the host's default state and double-click-to-reset. They are NOT the state
    on instantiation - that is factory Program 01 UNDER PRESSURE, applied by ProgramManager. The two
    are different things and both are intentional. */
inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ParamIDs::threshold, 1 }, "Threshold",
        NormalisableRange<float> { -40.0f, 10.0f, 0.1f }, Defaults::threshold,
        AudioParameterFloatAttributes().withLabel ("dB")));

    layout.add (std::make_unique<AudioParameterChoice> (
        ParameterID { ParamIDs::ratio, 1 }, "Ratio", ratioNames, Defaults::ratio));

    layout.add (std::make_unique<AudioParameterChoice> (
        ParameterID { ParamIDs::knee, 1 }, "Knee", kneeNames, Defaults::knee));

    // Stores the knob position; renders as OFF or a frequency. See Law::hpFrequencyHz.
    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ParamIDs::sidechainHp, 1 }, "Sidechain HP",
        NormalisableRange<float> { 0.0f, 1.0f }, Defaults::sidechainHp,
        AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float v, int)
            {
                if (Law::hpIsOff (v))
                    return String ("OFF");

                const float hz = Law::hpFrequencyHz (v);
                return hz < 100.0f ? String (roundToInt (hz)) + " Hz"
                                   : String (roundToInt (hz / 5.0f) * 5) + " Hz";
            })));

    // Real units with the design's law as explicit conversions, so the value a host shows and the
    // value the DSP uses are the same number as the panel's print.
    // **The formatter is not decoration - without it this parameter prints at SEVEN decimal
    // places.** Its NormalisableRange is built from conversion lambdas and therefore carries no
    // interval, and JUCE leaves numDecimalPlacesToDisplay at 7 for a zero interval, so a host's
    // automation lane and any generic editor read "4.7381272 ms". The panel did not, because
    // ProgramHeader used to format this by hand - which made the defect invisible here while it was
    // fully visible in every host. Gatecrasher hit the same thing and fixed it; TapeRot shipped it;
    // this casting hid it. nf::readoutDefects is what now fails a build over it.
    //
    // Decimals vary with magnitude because the range spans 0.1 to 30: two places below 1 ms, one
    // below 10, none above. A fixed count is either useless at the bottom or noise at the top.
    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ParamIDs::attack, 1 }, "Attack",
        NormalisableRange<float> { 0.1f, 30.0f,
            [] (float, float, float t) { return Law::attackMsFromPosition (t); },
            [] (float, float, float v) { return Law::attackPositionFromMs (v); },
            [] (float lo, float hi, float v) { return jlimit (lo, hi, v); } },
        Defaults::attackMs,
        AudioParameterFloatAttributes().withLabel ("ms")
            .withStringFromValueFunction ([] (float v, int)
            {
                // **The >= 10 ms case is roundToInt, NOT String(v, 0).** juce::String(double, int)
                // only sets a formatting flag when the decimal count is greater than zero; at
                // exactly 0 it falls through to std::ostream's default, which is six significant
                // digits with trailing zeros stripped - so String(16.0191f, 0) is "16.0191", not
                // "16". Written the wrong way here first and caught by ReadoutConformanceTests
                // within the minute, which is the entire argument for that test existing.
                if (v < 1.0f)  return String (v, 2);
                if (v < 10.0f) return String (v, 1);

                return String (roundToInt (v));
            })));

    layout.add (std::make_unique<AudioParameterChoice> (
        ParameterID { ParamIDs::release, 1 }, "Release", releaseNames, Defaults::release));

    // **Whole percent, and NOT via String(v, 0).** juce::String(double, int) only sets a formatting
    // flag when the decimal count is greater than zero; at exactly 0 it falls through to
    // std::ostream's default, which is six significant digits with trailing zeros stripped - so
    // "0 decimal places" renders 33.333332 as "33.3333" and only looks right while the value
    // happens to be integral. roundToInt is what actually rounds. Gatecrasher's Parameters.h
    // carries the same note beside the same lambda.
    //
    // Whole rather than one place because the printed ring is legended in whole percent, and the
    // readout should read back what the scale says.
    const auto wholePercent = [] (float v, int) { return String (roundToInt (v)); };

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ParamIDs::iron, 1 }, "Iron",
        NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, Defaults::iron,
        AudioParameterFloatAttributes().withLabel ("%")
            .withStringFromValueFunction (wholePercent)));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ParamIDs::makeup, 1 }, "Makeup",
        NormalisableRange<float> { 0.0f, 20.0f, 0.1f }, Defaults::makeup,
        AudioParameterFloatAttributes().withLabel ("dB")));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ParamIDs::mix, 1 }, "Mix",
        NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, Defaults::mix,
        AudioParameterFloatAttributes().withLabel ("%")
            .withStringFromValueFunction (wholePercent)));

    return layout;
}

} // namespace Elmer
