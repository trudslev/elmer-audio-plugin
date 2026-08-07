#include "Parameters.h"

#include <juce_audio_processors/juce_audio_processors.h>

/**
    BRAND.md makes this a correctness requirement, not a nicety: "Printed scales and actual
    parameter mappings must agree exactly - the pointer sitting on a printed mark must report that
    value. Watch logarithmic controls in particular: evenly spaced marks with unevenly valued steps
    need a log response, or the endpoints will look right while every intermediate mark is wrong."

    That failure mode is specifically what this suite catches. A plain NormalisableRange skew fitted
    to the endpoints passes at 0 and 1 and is wrong everywhere between, which is exactly the bug the
    rule exists to prevent - so every intermediate mark is asserted, not just the ends.
*/
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
    }
};

static PrintedScaleTests printedScaleTests;
