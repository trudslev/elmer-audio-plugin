#include "OutputStage.h"

#include <cmath>

namespace
{
    float makeupDbToGain (float db) noexcept
    {
        return std::pow (10.0f, db / 20.0f);
    }
}

void OutputStage::prepare (double sampleRate, float initialMakeupDb, float initialMix01) noexcept
{
    makeupGain.reset (sampleRate, smoothingSeconds);
    mix.reset (sampleRate, smoothingSeconds);

    /*  **`reset (rate, seconds)` did NOT set these, and that is the whole point of the two lines
        below.** It is `setCurrentAndTargetValue (this->target)` internally, so it snaps to whatever
        target the smoother last held - zero on a constructed object, the previous session's value on
        a re-prepared one. It sets the ramp LENGTH.

        And the value written has to come from the CALLER, not from `getTargetValue()`. TapeRot has
        seven sites doing the latter, which is exactly what `reset` already did: a guard that reads
        the stale target back and writes it in, and reads as guarded, which is worse than nothing
        being there. */
    makeupGain.setCurrentAndTargetValue (makeupDbToGain (initialMakeupDb));
    mix.setCurrentAndTargetValue (initialMix01);
}

/*  **Deliberately empty, and the empty body is the decision.**

    RULED: a reset owes a cleared TAIL, not a rewound control. These two smoothers hold where a
    gain currently sits, which is control state rather than anything that decays - a host locating
    the transport has no business moving MAKEUP.

    The tempting body is `setCurrentAndTargetValue (getTargetValue())` on each, to kill an in-flight
    ramp. Two things against it: it moves a control on a transport event, and it is character for
    character the no-op guard this suite has found seven times in TapeRot, where it follows
    `reset (rate, seconds)` and undoes nothing. A reader meeting it here would have to work out which
    of the two it is. Empty says what it means. */
void OutputStage::reset() noexcept
{
}

void OutputStage::setMakeupDb (float db) noexcept
{
    makeupGain.setTargetValue (makeupDbToGain (db));
}
