#pragma once

#include <juce_dsp/juce_dsp.h>

/**
    The SIDECHAIN HP, 40-500 Hz, second order.

    It sits on the DETECTOR PATH ONLY and never in the audio path. It changes what the compressor
    reacts to, not what you hear - filtering the audio would make a bus compressor a tone control,
    which is not what the control is for. There is exactly one instance, because there is exactly
    one detector.

    OFF genuinely bypasses rather than setting a very low corner: a 20 Hz high-pass still moves
    phase and still attenuates the sub content a kick actually has, so "OFF" has to mean off.
*/
class SidechainFilter
{
public:
    void prepare (double sampleRate) noexcept;
    void reset() noexcept;

    /** Pass 0 (or anything <= 0) for OFF. */
    void setCutoffHz (float hz) noexcept;

    float processSample (float x) noexcept;

private:
    /** A value no cutoff can take, so `prepare` can invalidate the cache — see its comment. The
        parameter's range starts at 40 Hz and anything <= 0 means OFF. */
    static constexpr float noCachedCutoff = -1.0f;

    juce::dsp::IIR::Filter<float> filter;
    double fs = 44100.0;
    float cutoff = noCachedCutoff;
    bool bypassed = true;
};
