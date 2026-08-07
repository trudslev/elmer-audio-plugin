#include "IronStage.h"

#include <algorithm>
#include <cmath>

float IronStage::processSample (float x) const noexcept
{
    if (amount <= 0.0f)
        return x;

    const float a = std::clamp (amount, 0.0f, 1.0f);

    // Drive rises with the control; the divide keeps small-signal gain at unity for any drive.
    const float drive = 1.0f + a * 4.0f;
    const float driven = std::tanh (x * drive) / drive;

    // A trace of second harmonic. Real iron is not symmetric, and the asymmetry is most of what
    // separates "transformer" from "clipper" by ear. Kept small and scaled by the control so IRON
    // at 0 is genuinely transparent.
    const float even = 0.06f * a * (driven * driven - 0.5f * x * x);

    return x * (1.0f - a) + (driven + even) * a;
}
