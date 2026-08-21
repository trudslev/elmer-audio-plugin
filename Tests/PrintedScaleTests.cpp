#include "Parameters.h"
#include "../Source/GUI/ElmerTheme.h"

#include <nf/PrintedScale.h>

#include <juce_audio_processors/juce_audio_processors.h>

#include <vector>

/**
    BRAND.md makes this a correctness requirement, not a nicety: "Printed scales and actual
    parameter mappings must agree exactly - the pointer sitting on a printed mark must report that
    value. Watch logarithmic controls in particular: evenly spaced marks with unevenly valued steps
    need a log response, or the endpoints will look right while every intermediate mark is wrong."

    That failure mode is specifically what this suite catches. A plain NormalisableRange skew fitted
    to the endpoints passes at 0 and 1 and is wrong everywhere between, which is exactly the bug the
    rule exists to prevent - so every intermediate mark is asserted, not just the ends.

    **Five of the six rings are checked by `nf::printedScaleDefects` now, and one deliberately is
    not.** Core's check takes the parameter's own `NormalisableRange` as the authority, which is the
    whole point of it: a test comparing stored angles with stored angles asserts that somebody
    transcribed a spec consistently and says nothing about whether the ring matches the control.

    That works here for THRESHOLD, IRON, MAKEUP and MIX, whose ranges are plain linear - and, less
    obviously, for **ATTACK, whose range is built from the explicit conversion lambdas in
    `Elmer::Law`**. Because those lambdas ARE the range's convertFrom0to1/convertTo0to1, the range is
    authoritative about the law and core's check applies unchanged. Worth stating because the
    opposite is easy to assume: "the taper lives in a lambda" sounds like the taper is outside the
    parameter, and here it is inside it.

    **SIDECHAIN HP is the one that genuinely sits outside**, and it takes Reflect-84's treatment for
    the same structural reason that casting's whole panel does. Its parameter is a bare
    `NormalisableRange<float>{0, 1}` - it has to be, so the OFF zone can occupy real travel - and the
    Hz taper lives in `Law::hpFrequencyHz`. Its marks print 40..500 against a 0-1 range, so core's
    range check would report every one of them as a value the control cannot reach. What is asserted
    instead is the stronger thing: the numeral printed at a fraction must be what the control reads
    there. `nf::printedScaleDefects` still runs over it for the checks that hold whatever the taper
    is - marks in order, no two sharing a tick, none outside the sweep.

    `Elmer::PrintedScale::Mark` stores `{position01, printedValue}`, which is the inverse of
    `nf::PrintedMark`'s `{value, angleDegrees}`; `toPrintedMarks` below is the whole conversion.
*/
namespace
{
    /** `{position01, printedValue}` -> `{value, angleDegrees}`.

        The tick's angle comes from its stored rotation fraction through the same
        `nf::sweepAngleDegrees` the pointer uses, so the two cannot be computed differently here and
        in the panel. */
    template <typename MarkArray>
    std::vector<nf::PrintedMark> toPrintedMarks (const MarkArray& marks)
    {
        std::vector<nf::PrintedMark> out;
        out.reserve (marks.size());

        for (const auto& m : marks)
            out.push_back ({ m.printedValue, nf::sweepAngleDegrees (m.position01) });

        return out;
    }
}

class PrintedScaleTests final : public juce::UnitTest
{
public:
    PrintedScaleTests() : juce::UnitTest ("Printed scales match the parameter laws", "Parameters") {}

    void runTest() override
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout = Elmer::createParameterLayout();

        // Rebuilding the layout inside a dummy processor is the only way to exercise the real
        // NormalisableRange objects the plugin will use, rather than a copy of the maths.
        struct Dummy final : juce::AudioProcessor
        {
            explicit Dummy (juce::AudioProcessorValueTreeState::ParameterLayout&& l)
                : apvts (*this, nullptr, "ELMER", std::move (l)) {}
            const juce::String getName() const override { return "Dummy"; }
            void prepareToPlay (double, int) override {}
            void releaseResources() override {}
            void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
            juce::AudioProcessorEditor* createEditor() override { return nullptr; }
            bool hasEditor() const override { return false; }
            bool acceptsMidi() const override { return false; }
            bool producesMidi() const override { return false; }
            bool isMidiEffect() const override { return false; }
            double getTailLengthSeconds() const override { return 0.0; }
            int getNumPrograms() override { return 1; }
            int getCurrentProgram() override { return 0; }
            void setCurrentProgram (int) override {}
            const juce::String getProgramName (int) override { return {}; }
            void changeProgramName (int, const juce::String&) override {}
            void getStateInformation (juce::MemoryBlock&) override {}
            void setStateInformation (const void*, int) override {}
            juce::AudioProcessorValueTreeState apvts;
        };

        Dummy dummy { std::move (layout) };

        const auto valueAt = [&dummy] (const char* id, float position01)
        {
            auto* p = dynamic_cast<juce::RangedAudioParameter*> (dummy.apvts.getParameter (id));
            jassert (p != nullptr);
            return p->convertFrom0to1 (position01);
        };

        const auto checkRing = [this, &dummy] (const char* id, const auto& marks,
                                               float toleranceDegrees, const juce::String& ring)
        {
            auto* p = dynamic_cast<juce::RangedAudioParameter*> (dummy.apvts.getParameter (id));
            expect (p != nullptr, ring + ": no such parameter");

            if (p == nullptr)
                return;

            for (const auto& defect : nf::printedScaleDefects (p->getNormalisableRange(),
                                                               toPrintedMarks (marks),
                                                               270.0f, toleranceDegrees))
                expect (false, ring + ": " + defect);
        };

        beginTest ("Every ring agrees with its own parameter's range");
        {
            // 0.5 degrees is core's default and these four are exact to floating point, so the
            // tolerance is doing nothing but absorbing the spec's rounded figures.
            checkRing (ParamIDs::threshold, Elmer::PrintedScale::threshold, 0.5f, "THRESHOLD");
            checkRing (ParamIDs::iron,      Elmer::PrintedScale::iron,      0.5f, "IRON");
            checkRing (ParamIDs::makeup,    Elmer::PrintedScale::makeupDb,  0.5f, "MAKEUP");
            checkRing (ParamIDs::mix,       Elmer::PrintedScale::mix,       0.5f, "MIX");

            // **2.1 degrees on ATTACK, and the figure is measured rather than chosen.** The printed
            // sequence is the rounded form of the law's exact series, so each numeral sits slightly
            // off the tick its own value would compute. Solving f = ln(ms/0.1)/ln(300) for every
            // printed mark and taking the largest |position - f| x 270:
            //
            //     0.1 ->  0.0000 deg      3   -> +0.9975 deg
            //     0.3 -> +1.9950 deg      10  -> -1.9950 deg
            //     1   -> -0.9975 deg      30  ->  0.0000 deg
            //
            // 1.9950 is the worst case, so 2.1 clears it with margin and nothing else. A taper
            // change large enough to matter moves marks by far more than a tenth of a degree - the
            // skew-fitted approximation asserted below misses by whole degrees.
            //
            // **What this check does NOT see on ATTACK, established by causing it rather than by
            // reading core.** `printedScaleDefects` reads `convertTo0to1` and nothing else, so on a
            // range whose two directions are independent lambdas it guards the ring against the
            // INVERSE only. Changing `attackMsFromPosition` alone - 300 -> 250 - leaves
            // `attackPositionFromMs` saying what it always said, and this test passes.
            //
            // That gap is closed by "ATTACK round-trips through its own conversion" below, which is
            // the assertion that the two lambdas are actually inverses. The pair is complete: core's
            // check ties the ring to the inverse, the round-trip ties the inverse to the forward
            // law. Verified both ways - changing both lambdas fails here with "the ring and the
            // taper disagree", changing one fails the round-trip and nine factory Programs with it.
            checkRing (ParamIDs::attack, Elmer::PrintedScale::attackMs, 2.1f, "ATTACK");
        }

        beginTest ("Which rings' FRACTIONS are derived output — and three answers, not one");
        {
            /*  **The question Chorus-60's RATE answered.** `Mark` stores `{position01,
                printedValue}` — a fraction AND a value for the same mark. If `convertTo0to1(value)`
                reproduces the stored fraction, the fraction is derived output that has been
                transcribed, and storing the value alone is strictly stronger: a taper change then
                moves the ring WITH the pointer instead of leaving numerals where it never goes.

                Chorus-60's RATE regenerated its published fractions to six decimals that way, which
                is how the catalogue's figures turned out to be derived output.

                **MEASURED HERE, AND ELMER'S RINGS ARE NOT ALL THE SAME CASE.** Three rings, three
                answers, and only one of them is RATE's:

                  THRESHOLD     0.000000000  the fraction IS derived output — convert it
                  SIDECHAIN HP  0.800000012  structurally cannot — see below
                  ATTACK        0.007388830  could, and it would be WRONG — see below

                **SIDECHAIN HP's parameter stores the knob POSITION, not a frequency**, because the
                control has a dead zone: its first tenth is OFF and its next clamps to 40 Hz, so a
                frequency-valued parameter could not represent where the pointer actually is. So
                `convertTo0to1(140.0f)` asks the range to place a Hz value on a 0–1 parameter and
                gets a clamp. The FRACTION is the primary datum on that ring and the printed value is
                the derived one — the exact inverse of every other ring here.

                **ATTACK's printed values are deliberately not its exact ones.** The panel prints
                0.1 / 0.3 / 1 / 3 / 10 / 30 for an exact series of 0.1 / 0.313 / 0.979 / 3.06 /
                9.58 / 30, which is how real panels are marked. Deriving the angle from the PRINTED
                value would move each tick to where the rounded value sits — 0.0074 of sweep, about
                2 degrees — so the numeral would be exact and the tick would be wrong. The residual
                is the rounding, and it is the reason to leave that ring alone.

                So this is reported per ring rather than converted wholesale. The suite-wide rule
                still holds — a mark's angle should come from what drives the pointer — but on two
                of these three rings the stored fraction IS what drives it. */
            const auto residualFor = [&dummy, this] (const char* id, const auto& marks,
                                                     const juce::String& label)
            {
                auto* p = dummy.apvts.getParameter (id);
                if (p == nullptr) { expect (false, label + ": no parameter"); return; }

                float worst = 0.0f;
                for (const auto& m : marks)
                    worst = juce::jmax (worst,
                                        std::abs (p->convertTo0to1 (m.printedValue) - m.position01));

                logMessage ("  " + label.paddedRight (' ', 14) + "worst residual "
                            + juce::String (worst, 9));
            };

            residualFor (ParamIDs::threshold,   Elmer::PrintedScale::threshold,   "THRESHOLD");
            // Asserted, because THRESHOLD is the one ring where the fraction is provably redundant
            // and a future edit that made it stop being redundant is a real defect.
            for (const auto& m : Elmer::PrintedScale::threshold)
                if (auto* pp = dummy.apvts.getParameter (ParamIDs::threshold))
                    expectWithinAbsoluteError (pp->convertTo0to1 (m.printedValue), m.position01,
                                               1.0e-6f,
                                               "THRESHOLD's stored fraction no longer regenerates "
                                               "from its value, so the two have diverged and the "
                                               "ring may point where the parameter does not");
            residualFor (ParamIDs::sidechainHp, Elmer::PrintedScale::sidechainHp, "SIDECHAIN HP");
            residualFor (ParamIDs::attack,      Elmer::PrintedScale::attackMs,    "ATTACK");
            residualFor (ParamIDs::iron,        Elmer::PrintedScale::iron,        "IRON");
            residualFor (ParamIDs::makeup,      Elmer::PrintedScale::makeupDb,    "MAKEUP");
            residualFor (ParamIDs::mix,         Elmer::PrintedScale::mix,         "MIX");
        }

        beginTest ("THRESHOLD - linear, every printed mark exact");
        for (const auto& m : Elmer::PrintedScale::threshold)
            expectWithinAbsoluteError (valueAt (ParamIDs::threshold, m.position01), m.printedValue, 0.001f,
                          "threshold at " + juce::String (m.position01));

        beginTest ("IRON / MAKEUP / MIX - linear, every printed mark exact");
        for (const auto& m : Elmer::PrintedScale::iron)
            expectWithinAbsoluteError (valueAt (ParamIDs::iron, m.position01), m.printedValue, 0.001f,
                          "iron at " + juce::String (m.position01));
        for (const auto& m : Elmer::PrintedScale::makeupDb)
            expectWithinAbsoluteError (valueAt (ParamIDs::makeup, m.position01), m.printedValue, 0.001f,
                          "makeup at " + juce::String (m.position01));
        for (const auto& m : Elmer::PrintedScale::mix)
            expectWithinAbsoluteError (valueAt (ParamIDs::mix, m.position01), m.printedValue, 0.001f,
                          "mix at " + juce::String (m.position01));

        beginTest ("SIDECHAIN HP - log, every printed mark within 2% of its rounded print");
        expect (Elmer::Law::hpIsOff (0.0f), "position 0 must read OFF");
        expect (Elmer::Law::hpIsOff (0.09f), "the OFF zone runs to 0.10");
        expect (! Elmer::Law::hpIsOff (0.11f), "past 0.10 the filter is engaged");

        for (const auto& m : Elmer::PrintedScale::sidechainHp)
        {
            const float hz = Elmer::Law::hpFrequencyHz (m.position01);
            // 2%: the 0.6 mark is exactly 40*sqrt(12.5) = 141.42 Hz under a printed "140" - the
            // designer rounded to the friendlier number, as hardware panels do. Every other mark
            // lands inside 0.4%.
            expectWithinAbsoluteError (hz, m.printedValue, m.printedValue * 0.02f,
                          "sidechain HP at " + juce::String (m.position01)
                              + " reads " + juce::String (hz, 1) + " Hz");
        }

        beginTest ("SIDECHAIN HP - the structural checks, which hold whatever the taper is");
        {
            // Reflect-84's treatment, and for the same structural reason: this parameter is a plain
            // 0-1 range with the taper in Law::hpFrequencyHz, so its marks print 40..500 against a
            // 0..1 range and core's range check would call every one of them unreachable. Feed it
            // the POSITIONS against the identity range instead - it then asserts only what does not
            // depend on the taper: the marks ascend, no two share a tick, none is drawn outside the
            // sweep. The numeral-versus-law check above is what covers the taper itself.
            const juce::NormalisableRange<float> identity { 0.0f, 1.0f };
            std::vector<nf::PrintedMark> positions;

            for (const auto& m : Elmer::PrintedScale::sidechainHp)
                positions.push_back ({ m.position01, nf::sweepAngleDegrees (m.position01) });

            for (const auto& defect : nf::printedScaleDefects (identity, positions))
                expect (false, "SIDECHAIN HP: " + defect);
        }

        beginTest ("ATTACK - log, every printed mark within 5% of its rounded print");
        // 5% because the printed sequence 0.1/0.3/1/3/10/30 is the conventionally rounded form of
        // the exact series the design's `0.1 * 300^f` produces (0.1/0.313/0.979/3.06/9.58/30). The
        // law is the design's and is kept as-is; see Parameters.h. The tolerance is deliberately
        // tight enough that a skew-fitted approximation still fails.
        for (const auto& m : Elmer::PrintedScale::attackMs)
        {
            const float ms = valueAt (ParamIDs::attack, m.position01);
            expectWithinAbsoluteError (ms, m.printedValue, m.printedValue * 0.05f,
                          "attack at " + juce::String (m.position01)
                              + " reads " + juce::String (ms, 3) + " ms");
        }

        beginTest ("ATTACK - a fitted skew would NOT pass, so the law is really being tested");
        // A NormalisableRange skew chosen to hit both endpoints puts the midpoint at
        // sqrt(0.1 * 30) = 1.73 ms; the true law puts it at 1.73... - so compare somewhere the two
        // genuinely diverge. At position 0.25 the law gives 0.1 * 300^0.25 = 0.416 ms.
        expectWithinAbsoluteError (valueAt (ParamIDs::attack, 0.25f), 0.416f, 0.005f, "attack at 0.25");
        expectWithinAbsoluteError (valueAt (ParamIDs::attack, 0.75f), 7.21f, 0.05f, "attack at 0.75");

        beginTest ("ATTACK round-trips through its own conversion");
        for (float ms : { 0.1f, 0.25f, 1.0f, 4.7f, 12.0f, 30.0f })
            expectWithinAbsoluteError (Elmer::Law::attackMsFromPosition (Elmer::Law::attackPositionFromMs (ms)),
                          ms, ms * 0.001f, "attack round trip at " + juce::String (ms));

        beginTest ("RATIO detents land on their printed positions");
        auto* ratio = dynamic_cast<juce::AudioParameterChoice*> (dummy.apvts.getParameter (ParamIDs::ratio));
        expect (ratio != nullptr);
        expectEquals (ratio->choices.size(), 5);
        for (int i = 0; i < 5; ++i)
        {
            const float pos = Elmer::PrintedScale::detentPosition (i, 5);
            // convertFrom0to1 on a choice parameter yields the index itself, not a 0-1 value.
            expectEquals ((int) std::lround (ratio->convertFrom0to1 (pos)), i,
                          "ratio detent " + juce::String (i));
            expectEquals (ratio->choices[i], Elmer::ratioNames[i],
                          "ratio legend " + juce::String (i));
            expectWithinAbsoluteError (Elmer::ratioValues[(size_t) i],
                          Elmer::ratioNames[i].upToFirstOccurrenceOf (":", false, false).getFloatValue(),
                          0.001f, "ratio value matches its printed legend");
        }

        beginTest ("RELEASE has five positions and AUTO is the last");
        auto* release = dynamic_cast<juce::AudioParameterChoice*> (dummy.apvts.getParameter (ParamIDs::release));
        expect (release != nullptr);
        expectEquals (release->choices.size(), 5);
        expectEquals (release->choices[4], juce::String ("AUTO"));
        expectEquals ((int) Elmer::Release::autoMode, 4);
        // AUTO is a position on the switch, not a separate parameter and not a button.
        expect (dummy.apvts.getParameter ("autoRelease") == nullptr,
                "there must be no separate auto-release parameter");

        beginTest ("Defaults are the brief's, and are not Program 01");
        expectWithinAbsoluteError (valueAt (ParamIDs::threshold, dummy.apvts.getParameter (ParamIDs::threshold)->getDefaultValue()),
                      Elmer::Defaults::threshold, 0.01f, "threshold default");
        expectWithinAbsoluteError (valueAt (ParamIDs::attack, dummy.apvts.getParameter (ParamIDs::attack)->getDefaultValue()),
                      Elmer::Defaults::attackMs, 0.05f, "attack default");
        expect (Elmer::Law::hpIsOff (Elmer::Defaults::sidechainHp), "sidechain HP defaults to OFF");

        /*  **The PRINTED NUMERALS and the UNIT, which nothing here checked until 2026-08-21.**

            Everything above asserts the tick RING, which is looked up by name — `marksFor
            (spec.paramId)` — and was always right. The numerals beside those ticks and the unit in
            the arc gap come from two arrays indexed by KNOB POSITION, and their last two rows were
            ordered MAKEUP then MIX where `Layout::knobs` has been MIX then MAKEUP since its first
            commit.

            So MIX printed **0 / 5 / 10 / 15 / 20** and "dB" on a 0-100 % control, and MAKEUP printed
            **0 / 25 / 50 / 75 / 100** and "%" on a 0-20 dB one. At the printed 10 on MIX the pointer
            was at 50 %; at the printed 50 on MAKEUP it was at 10 dB.

            **Both endpoints agreed**, which is why nothing showed it: the first mark is 0 on both
            and the last mark is the maximum on both, so the ring looked like a ring and only the
            numbers in between were another control's.

            This arm is sourced from **the parameter**, which is outside both tables — the same
            principle as the ring check, one level over. A test comparing the two tables with each
            other would have agreed with itself: each was internally consistent and each comment was
            honest about which row it was.  */
        beginTest ("Every knob's printed numerals and unit belong to ITS parameter");
        {
            namespace L = ElmerTheme::Layout;

            const auto largestNumeral = [] (const std::array<L::Legend, 6>& row)
            {
                float widest = 0.0f;
                for (const auto& lg : row)
                {
                    if (lg.text == nullptr)
                        continue;

                    // RATIO and RELEASE print "20:1" and "AUTO"; a leading number is what is
                    // comparable, and a row with none is skipped by the caller.
                    const auto v = juce::String (lg.text).getFloatValue();
                    widest = juce::jmax (widest, v);
                }
                return widest;
            };

            for (size_t k = 0; k < L::knobs.size(); ++k)
            {
                const auto& spec = L::knobs[k];
                auto* param = dynamic_cast<juce::RangedAudioParameter*> (
                    dummy.apvts.getParameter (spec.paramId));

                if (param == nullptr)
                    continue;

                const juce::String id { spec.paramId };

                /*  **`getText (1.0f)`, not `range.end`.** The first version of this arm compared
                    the numeral against the range's maximum and reported SIDECHAIN HP as broken:
                    that parameter is normalised 0-1 with its law in `Elmer::Law`, so its range
                    maximum is 1.0 while its ring prints 500 Hz. The instrument was wrong, not the
                    panel — and it was wrong in the direction that produces a confident finding,
                    which is why it is recorded here rather than quietly corrected.

                    What the parameter PRINTS at full deflection is the right authority: it is the
                    parameter's own answer, it is outside both positional tables, and it holds
                    whether the law lives in the range or beside it. */
                const float atFullScale = param->getText (1.0f, 0).getFloatValue();
                const float printed = largestNumeral (L::legends[k]);

                // RELEASE's last position is AUTO, which is a word rather than a value, so its ring
                // has no numeric maximum to compare. Excluded BY NAME with the reason, never by a
                // silent zero — a ring that stops being numeric should be a change somebody makes
                // here rather than one that quietly drops out of the check.
                if (id == "release")
                    continue;

                logMessage ("  " + id + ": largest numeral " + juce::String (printed, 1)
                                + ", parameter prints " + param->getText (1.0f, 0)
                                + " at full scale");

                expectWithinAbsoluteError (printed, atFullScale, 0.51f,
                    id + "'s largest printed numeral is not what its parameter reads at full "
                         "deflection — the legend row and the knob are out of step");

                /*  The unit, from the same positional table. Only asserted where the parameter
                    carries a label: SIDECHAIN HP prints its unit inside the value text instead, as
                    the suite's readout rule allows, so an empty label there is correct rather than
                    missing. */
                if (const auto* unit = L::knobArcUnits[k].text; unit != nullptr
                                                             && param->getLabel().isNotEmpty())
                    expectEquals (juce::String (unit), param->getLabel(),
                                  id + "'s printed unit is not its parameter's label");
            }
        }
    }
};

static PrintedScaleTests printedScaleTests;
