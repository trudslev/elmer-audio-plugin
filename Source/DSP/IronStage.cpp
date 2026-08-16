#include "IronStage.h"

#include <algorithm>
#include <cmath>

void IronStage::prepare (double sampleRate, float initialAmount01) noexcept
{
    amount.reset (sampleRate, smoothingSeconds);

    // The value comes from the CALLER, never from getTargetValue(): `reset (rate, seconds)` is
    // `setCurrentAndTargetValue (this->target)` internally, so reading the target back and writing
    // it in is exactly what that call already did. See OutputStage::prepare for the full note.
    amount.setCurrentAndTargetValue (std::clamp (initialAmount01, 0.0f, 1.0f));
}

/*  Empty for the same reason OutputStage::reset is - a reset owes a cleared tail, and a drive
    setting is not one. See that function for the ruling. */
void IronStage::reset() noexcept
{
}

float IronStage::processSample (float x) noexcept
{
    // **Advanced every sample, including the transparent path.** An early return above this would
    // leave the smoother parked, so turning IRON down to zero and back up would jump rather than
    // glide - the sample count consumed has to be the same whichever branch the audio takes.
    const float a = std::clamp (amount.getNextValue(), 0.0f, 1.0f);

    if (a <= 0.0f)
        return x;

    // Drive rises with the control; the divide keeps small-signal gain at unity for any drive.
    const float drive = 1.0f + a * 4.0f;
    const float driven = std::tanh (x * drive) / drive;

    // A trace of second harmonic. Real iron is not symmetric, and the asymmetry is most of what
    // separates "transformer" from "clipper" by ear. Kept small and scaled by the control so IRON
    // at 0 is genuinely transparent.
    const float even = 0.06f * a * (driven * driven - 0.5f * x * x);

    return x * (1.0f - a) + (driven + even) * a;
}
