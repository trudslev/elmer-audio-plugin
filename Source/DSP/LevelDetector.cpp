#include "LevelDetector.h"

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

    constexpr float floorDb = -120.0f;

    inline float toDb (float linear) noexcept
    {
        return linear > 1.0e-6f ? 20.0f * std::log10 (linear) : floorDb;
    }
}

void LevelDetector::prepare (double sampleRate) noexcept
{
    fs = sampleRate;
    autoRelease.prepare (sampleRate);
    setAttackMs (10.0f);
    setRelease (fixedReleaseSeconds, autoMode);
    reset();
}

void LevelDetector::reset() noexcept
{
    currentGrDb = 0.0f;
    autoRelease.reset();
}

void LevelDetector::setAttackMs (float ms) noexcept
{
    attackCoeff = onePoleCoeff (ms * 0.001f, fs);
}

void LevelDetector::setRelease (float seconds, bool useAuto) noexcept
{
    autoMode = useAuto;
    fixedReleaseSeconds = seconds;
    fixedReleaseCoeff = onePoleCoeff (seconds, fs);
}

float LevelDetector::releaseCoeffFor (float seconds) const noexcept
{
    return onePoleCoeff (seconds, fs);
}

float LevelDetector::processSample (float detectorSample) noexcept
{
    const float targetGr = gainComputer.gainReductionDbFor (toDb (std::abs (detectorSample)));

    // Attack when gain reduction is increasing, release when it is recovering. AUTO recomputes its
    // coefficient every sample because the release time is itself a moving target.
    if (targetGr > currentGrDb)
    {
        currentGrDb += (targetGr - currentGrDb) * attackCoeff;
    }
    else
    {
        const float coeff = autoMode ? releaseCoeffFor (autoRelease.nextReleaseSeconds (currentGrDb))
                                     : fixedReleaseCoeff;
        currentGrDb += (targetGr - currentGrDb) * coeff;
    }

    // Keep the tracker moving during attack too, or a passage that never fully releases would never
    // build any sustain and AUTO would behave like a fixed fast release.
    if (autoMode && targetGr > currentGrDb)
        autoRelease.nextReleaseSeconds (currentGrDb);

    return currentGrDb;
}
