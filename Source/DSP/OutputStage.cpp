#include "OutputStage.h"

#include <cmath>

void OutputStage::setMakeupDb (float db) noexcept
{
    makeupGain = std::pow (10.0f, db / 20.0f);
}
