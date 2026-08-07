#pragma once

/**
    The static compression curve: input level in dB to gain reduction in dB.

    Feed-forward, deliberately. Gain is computed from the INPUT level, not the output, which is what
    makes the curve predictable and — the reason it matters here — what makes the printed RATIO marks
    tell the truth. A feedback topology's effective ratio drifts with level, so a panel marked 4:1
    would only be 4:1 at one input level and the print would be lying. `Tests/GainComputerTests.cpp`
    measures the realised ratio against each detent.

    Stateless: no memory, no sample rate. All the time behaviour lives in LevelDetector.
*/
class GainComputer
{
public:
    /** Soft knee width in dB, straddling the threshold. Hard knee is 0. */
    static constexpr float softKneeWidthDb = 6.0f;

    void setThresholdDb (float t) noexcept { threshold = t; }
    void setRatio (float r) noexcept       { ratio = r; }
    void setKneeWidthDb (float w) noexcept { knee = w; }

    /** Returns gain reduction in dB as a POSITIVE number (0 = no compression). */
    float gainReductionDbFor (float inputDb) const noexcept;

private:
    float threshold = -12.0f;
    float ratio = 4.0f;
    float knee = softKneeWidthDb;
};
