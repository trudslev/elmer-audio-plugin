#include "DSP/FactoryPrograms.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <set>

/**
    The factory bank, checked against the brief rather than against itself.

    Note what is deliberately NOT asserted here: there is no zero-fill invariant. Fifth Member has
    one because its Programs store only their active path, so a value in an inactive slot is a bug.
    Elmer has no mutually exclusive selectors - all nine controls are always live - so every Program
    stores all nine and a zero is a real zero. Porting that invariant here would be wrong.
*/
class FactoryProgramTests final : public juce::UnitTest
{
public:
    FactoryProgramTests() : juce::UnitTest ("Factory Programs", "Programs") {}

    void runTest() override
    {
        using namespace Elmer;

        beginTest ("Sixteen Programs, all uniquely named");
        expectEquals ((int) factoryPrograms.size(), 16);

        std::set<juce::String> names;
        for (const auto& fp : factoryPrograms)
        {
            const juce::String n { fp.name };
            expect (n.isNotEmpty(), "a Program has no name");
            expect (names.insert (n).second, "duplicate Program name: " + n);
        }

        beginTest ("Every value is inside its declared range");
        for (const auto& fp : factoryPrograms)
        {
            const juce::String where { juce::String (fp.name) + ": " };

            expect (fp.thresholdDb >= -40.0f && fp.thresholdDb <= 10.0f, where + "threshold");
            expect (fp.ratioIndex >= 0 && fp.ratioIndex < 5, where + "ratio index");
            expect (fp.kneeIndex == 0 || fp.kneeIndex == 1, where + "knee index");
            expect (fp.releaseIndex >= 0 && fp.releaseIndex < 5, where + "release index");
            expect (fp.attackMs >= 0.1f && fp.attackMs <= 30.0f, where + "attack");
            expect (fp.ironPercent >= 0.0f && fp.ironPercent <= 100.0f, where + "iron");
            expect (fp.makeupDb >= 0.0f && fp.makeupDb <= 20.0f, where + "makeup");
            expect (fp.mixPercent >= 0.0f && fp.mixPercent <= 100.0f, where + "mix");
            expect (fp.sidechainHpHz == 0.0f
                        || (fp.sidechainHpHz >= 40.0f && fp.sidechainHpHz <= 500.0f),
                    where + "sidechain HP must be 0 (OFF) or 40-500 Hz");
        }

        beginTest ("Every sidechain frequency round-trips through the knob position");
        // The factory table stores readable frequencies; the parameter stores knob positions. If
        // that conversion drifts, Programs silently load at the wrong corner.
        for (const auto& fp : factoryPrograms)
        {
            const float pos = Law::hpPositionForHz (fp.sidechainHpHz);

            if (fp.sidechainHpHz == 0.0f)
            {
                expect (Law::hpIsOff (pos), juce::String (fp.name) + ": OFF must stay OFF");
            }
            else
            {
                expect (! Law::hpIsOff (pos), juce::String (fp.name) + ": engaged must stay engaged");
                expectWithinAbsoluteError (Law::hpFrequencyHz (pos), fp.sidechainHpHz,
                                           fp.sidechainHpHz * 0.02f,
                                           juce::String (fp.name) + ": sidechain HP round trip");
            }
        }

        beginTest ("Every attack time round-trips through the knob position");
        for (const auto& fp : factoryPrograms)
            expectWithinAbsoluteError (Law::attackMsFromPosition (Law::attackPositionFromMs (fp.attackMs)),
                                       fp.attackMs, fp.attackMs * 0.01f,
                                       juce::String (fp.name) + ": attack round trip");

        beginTest ("Program 01 UNDER PRESSURE is the default and matches the brief");
        expectEquals (defaultFactoryProgramIndex, 0);
        const auto& first = factoryPrograms[0];
        expectEquals (juce::String (first.name), juce::String ("UNDER PRESSURE"));
        expectWithinAbsoluteError (first.thresholdDb, -14.0f, 0.001f);
        expectEquals (first.ratioIndex, 1);                       // 2:1
        expectEquals (first.kneeIndex, 0);                        // Soft
        expectWithinAbsoluteError (first.sidechainHpHz, 75.0f, 0.001f);
        expectWithinAbsoluteError (first.attackMs, 10.0f, 0.001f);
        expectEquals (first.releaseIndex, 4);                     // AUTO
        expectWithinAbsoluteError (first.ironPercent, 20.0f, 0.001f);
        expectWithinAbsoluteError (first.makeupDb, 2.5f, 0.001f);
        expectWithinAbsoluteError (first.mixPercent, 100.0f, 0.001f);

        beginTest ("QUEENS SMASH is the only Program below 100 % mix");
        // The New York parallel setting. If a second one appears it should be for the same
        // deliberate reason, not because a value was fat-fingered.
        int belowFull = 0;
        for (int i = 0; i < (int) factoryPrograms.size(); ++i)
            if (factoryPrograms[(size_t) i].mixPercent < 100.0f)
            {
                ++belowFull;
                expectEquals (i, parallelProgramIndex,
                              juce::String (factoryPrograms[(size_t) i].name)
                                  + " has mix below 100 - is that intended?");
            }

        expectEquals (belowFull, 1);
        expectEquals (juce::String (factoryPrograms[(size_t) parallelProgramIndex].name),
                      juce::String ("QUEENS SMASH"));

        beginTest ("The bank spans the range it claims to");
        // A bank where every Program sits in the same place is not a bank. These are loose sanity
        // bounds, not tuning targets.
        float minThresh = 0.0f, maxThresh = -100.0f;
        std::set<int> ratios, releases;

        for (const auto& fp : factoryPrograms)
        {
            minThresh = juce::jmin (minThresh, fp.thresholdDb);
            maxThresh = juce::jmax (maxThresh, fp.thresholdDb);
            ratios.insert (fp.ratioIndex);
            releases.insert (fp.releaseIndex);
        }

        expect (maxThresh - minThresh > 20.0f, "threshold spread is too narrow");
        expect (ratios.size() >= 4, "the bank should use most of the ratio detents");
        expect (releases.count (4) > 0, "no Program uses AUTO release");
        expect (releases.size() >= 3, "the bank should use several release settings");
    }
};

static FactoryProgramTests factoryProgramTests;
