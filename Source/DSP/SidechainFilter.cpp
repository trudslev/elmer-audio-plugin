#include "SidechainFilter.h"

void SidechainFilter::prepare (double sampleRate) noexcept
{
    fs = sampleRate;
    filter.prepare ({ sampleRate, 512, 1 });
    setCutoffHz (cutoff);
    reset();
}

void SidechainFilter::reset() noexcept
{
    filter.reset();
}

void SidechainFilter::setCutoffHz (float hz) noexcept
{
    bypassed = hz <= 0.0f;
    cutoff = hz;

    if (! bypassed)
        filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (
            fs, juce::jlimit (20.0f, (float) (fs * 0.45), hz), juce::MathConstants<float>::sqrt2 * 0.5f);
}

float SidechainFilter::processSample (float x) noexcept
{
    return bypassed ? x : filter.processSample (x);
}
