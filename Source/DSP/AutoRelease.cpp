#include "AutoRelease.h"

#include <algorithm>
#include <cmath>

namespace
{
    inline float onePoleCoeff (float seconds, double sampleRate) noexcept
    {
        if (seconds <= 0.0f || sampleRate <= 0.0)
            return 1.0f;

        return 1.0f - std::exp (-1.0f / (seconds * (float) sampleRate));
    }
}

void AutoRelease::prepare (double sampleRate) noexcept
{
    riseCoeff = onePoleCoeff (trackerRiseSeconds, sampleRate);
    fallCoeff = onePoleCoeff (trackerFallSeconds, sampleRate);
    reset();
}

float AutoRelease::nextReleaseSeconds (float currentGainReductionDb) noexcept
{
    const float target = std::clamp (currentGainReductionDb / sustainReferenceDb, 0.0f, 1.0f);

    // Asymmetric on purpose: sustain builds faster than it decays, so a passage that keeps ducking
    // holds the slow release through the gaps between hits instead of resetting on every one.
    const float coeff = target > sustain ? riseCoeff : fallCoeff;
    sustain += (target - sustain) * coeff;

    return fastSeconds + (slowSeconds - fastSeconds) * sustain;
}
