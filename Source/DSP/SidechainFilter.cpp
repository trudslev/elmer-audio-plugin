#include "SidechainFilter.h"

void SidechainFilter::prepare (double sampleRate) noexcept
{
    fs = sampleRate;
    filter.prepare ({ sampleRate, 512, 1 });

    /*  **Invalidate the cache before re-applying, or a re-prepare at the same cutoff builds
        nothing.** `setCutoffHz` short-circuits on an unchanged value now, and the coefficients
        depend on `fs` as well as on `hz` — so a host changing sample rate with the knob untouched
        would leave a filter whose corner is wrong by the ratio of the two rates, silently, on the
        detector path where it colours what the compressor reacts to rather than what you hear.

        `noCachedCutoff` is a value no cutoff can take: the parameter's own range starts at 40 Hz and
        anything <= 0 means OFF, which needs no coefficients either way. */
    const float requested = cutoff;
    cutoff = noCachedCutoff;
    setCutoffHz (requested);

    reset();
}

void SidechainFilter::reset() noexcept
{
    filter.reset();
}

/*  **Recomputes only on a CHANGE, and it used to recompute every block.**

    `makeHighPass` returns a `ReferenceCountedObjectPtr`, so every call heap-allocates a fresh
    coefficients object — measured at **32 bytes per block, signal-independent**, on the audio
    thread. `PluginProcessor` reads the parameter and calls this once per block whether or not the
    knob has moved, which is the ordinary way a control is applied and is why it was invisible.

    An allocation on the audio thread is a page fault and a lock away from a dropout, and this one
    fires unconditionally rather than under some rare branch.

    **Compared as a raw float, deliberately, and the `bypassed` flag is set outside the guard.**
    The parameter arrives already quantised by the host, so equality on the float is exact for an
    unmoved knob rather than approximately true; and a near-equal cutoff that fails the comparison
    costs one allocation rather than a wrong filter. Setting `bypassed` unconditionally keeps OFF
    responsive even on the path that skips the recompute, which is the one thing an early return
    here would break.
*/
void SidechainFilter::setCutoffHz (float hz) noexcept
{
    bypassed = hz <= 0.0f;

    if (hz == cutoff)
        return;

    cutoff = hz;

    if (! bypassed)
        filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (
            fs, juce::jlimit (20.0f, (float) (fs * 0.45), hz), juce::MathConstants<float>::sqrt2 * 0.5f);
}

float SidechainFilter::processSample (float x) noexcept
{
    return bypassed ? x : filter.processSample (x);
}
