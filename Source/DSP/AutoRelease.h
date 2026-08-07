#pragma once

/**
    The RELEASE switch's fifth position: a program-dependent dual time constant, not a preset rate.

    Modelled on the SSL bus compressor's behaviour. Two time constants run in parallel — a fast one
    that lets brief transients recover immediately, and a slow one that takes over when the signal
    stays over threshold. The result is a release that stretches under sustained compression and
    snaps back after a short peak, which is what "program-dependent" actually means and is why AUTO
    glues a mix without pumping on it.

    Implemented as a slow envelope tracking how *sustained* the compression has been, used to
    crossfade between the two release times. A single averaged rate would produce the same figure on
    a spec sheet and none of the behaviour — `Tests/AutoReleaseTests.cpp` asserts that recovery
    after a short burst really is faster than after a long tone, which a fixed rate cannot pass.
*/
class AutoRelease
{
public:
    static constexpr float fastSeconds = 0.08f;
    static constexpr float slowSeconds = 1.20f;

    /** How quickly the sustain tracker itself moves. Long enough that a snare hit does not drag
        the release slow, short enough that a held chord does within a bar. */
    static constexpr float trackerRiseSeconds = 0.30f;
    static constexpr float trackerFallSeconds = 0.80f;

    /** Gain reduction in dB at which compression counts as fully "sustained" for tracking purposes. */
    static constexpr float sustainReferenceDb = 6.0f;

    void prepare (double sampleRate) noexcept;
    void reset() noexcept { sustain = 0.0f; }

    /** Advances the tracker by one sample and returns the release time to use right now. */
    float nextReleaseSeconds (float currentGainReductionDb) noexcept;

    float getSustain() const noexcept { return sustain; }

private:
    float sustain = 0.0f;
    float riseCoeff = 0.0f;
    float fallCoeff = 0.0f;
};
