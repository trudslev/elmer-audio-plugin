#include "../Source/GUI/KnobFilmstrip.h"
#include "../Source/GUI/ElmerTheme.h"

#include <juce_gui_basics/juce_gui_basics.h>

/**
    §3.1's code-drawn knob, and specifically its cache.

    **A cache that rebuilds every frame is indistinguishable from one that works, by looking at the
    panel.** Both draw the right knob; the difference is only in what a drag costs, and a knob
    repaints on every frame of one. That is exactly what `setBufferedToImage` does on a Slider — it
    compiles, profiles identically, and caches nothing.

    **So every arm here counts rebuilds, and it counts them in BOTH directions.** A counter that
    never increments satisfies "rebuilt once over thirty renders" precisely as well as a working
    cache does, so the second arm drives the things that MUST invalidate and asserts the count
    rises.
*/
class ElmerKnobRenderTests final : public juce::UnitTest
{
public:
    ElmerKnobRenderTests() : juce::UnitTest ("Knob rendering", "GUI") {}

    static void render (KnobFilmstrip& k, juce::Image& target)
    {
        juce::Graphics g { target };
        k.paint (g);
    }

    void runTest() override
    {
        using namespace ElmerTheme::Layout;

        beginTest ("Built once, reused across every value change");
        {
            KnobFilmstrip knob { Strip::detect, knobLarge };
            knob.setBounds (0, 0, (int) knobLarge, (int) knobLarge);
            knob.setRange (0.0, 1.0);

            juce::Image target { juce::Image::ARGB, (int) knobLarge, (int) knobLarge, true };

            render (knob, target);
            expectEquals (knob.staticLayerBuildCount(), 1,
                          "the first paint built no static layer at all");

            for (int i = 0; i < 30; ++i)
            {
                knob.setValue ((double) i / 29.0, juce::dontSendNotification);
                render (knob, target);
            }

            logMessage ("  30 value changes -> " + juce::String (knob.staticLayerBuildCount())
                        + " build(s)");

            expectEquals (knob.staticLayerBuildCount(), 1,
                          "the static layer rebuilt while only the pointer moved — a cache in name "
                          "only, and a drag pays for it every frame");
        }

        beginTest ("SHOWN ABLE TO FAIL — the counter rises when a rebuild is genuinely required");
        {
            /*  Without this, the arm above passes on a counter that never increments. The size
                change is what a device-scale change looks like to the cache, which is the key it
                is actually on. */
            KnobFilmstrip knob { Strip::output, knobSmall };
            knob.setBounds (0, 0, (int) knobSmall, (int) knobSmall);

            juce::Image small { juce::Image::ARGB, (int) knobSmall, (int) knobSmall, true };
            render (knob, small);
            const int baseline = knob.staticLayerBuildCount();

            juce::Image big { juce::Image::ARGB, (int) knobSmall * 2, (int) knobSmall * 2, true };
            knob.setBounds (0, 0, (int) knobSmall * 2, (int) knobSmall * 2);
            render (knob, big);

            logMessage ("  after a size change -> " + juce::String (knob.staticLayerBuildCount())
                        + " build(s), was " + juce::String (baseline));

            expectGreaterThan (knob.staticLayerBuildCount(), baseline,
                               "a resized knob reused a layer drawn at the old size, so the counter "
                               "cannot distinguish a working cache from one that never rebuilds");
        }

        beginTest ("The pointer follows the parameter's taper, not a linear value");
        {
            KnobFilmstrip knob { Strip::timing, knobLarge };
            knob.setNormalisableRange ({ 0.1, 30.0, 0.0, 0.3 });
            knob.setValue (1.0, juce::dontSendNotification);

            const float travel = (float) knob.valueToProportionOfLength (knob.getValue());
            logMessage ("  ATTACK 1.0 ms sits at travel " + juce::String (travel, 6));

            expect (travel > 0.05f && travel < 0.95f,
                    "1 ms on a skewed 0.1-30 range collapsed to an endpoint, so the pointer is "
                    "reading a linear value rather than the parameter's own travel");
        }
    }
};

static ElmerKnobRenderTests elmerKnobRenderTests;
