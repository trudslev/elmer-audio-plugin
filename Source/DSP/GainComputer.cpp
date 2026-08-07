#include "GainComputer.h"

#include <algorithm>

float GainComputer::gainReductionDbFor (float inputDb) const noexcept
{
    const float over = inputDb - threshold;

    // Above the knee: the straight compressed segment. Output = T + over/ratio, so the reduction is
    // the part of `over` the ratio throws away.
    if (knee <= 0.0f || over >= knee * 0.5f)
        return over > 0.0f ? over * (1.0f - 1.0f / ratio) : 0.0f;

    // Below the knee: untouched.
    if (over <= -knee * 0.5f)
        return 0.0f;

    // Inside the knee: the standard quadratic bridge. Its value and its first derivative both match
    // the two straight segments at the knee edges, so the curve has no corner - which is the whole
    // point of a soft knee and is what a listener actually hears as "gentle".
    const float x = over + knee * 0.5f;
    return (1.0f - 1.0f / ratio) * (x * x) / (2.0f * knee);
}
