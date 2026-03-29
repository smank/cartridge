#include "EnvelopeGenerator.h"

namespace cart {

void EnvelopeGenerator::reset()
{
    startFlag    = false;
    decayLevel   = 0;
    dividerCount = 0;
}

void EnvelopeGenerator::start()
{
    startFlag = true;
}

void EnvelopeGenerator::clock()
{
    if (startFlag)
    {
        startFlag    = false;
        decayLevel   = 15;
        dividerCount = volume;
        return;
    }

    if (dividerCount > 0)
    {
        --dividerCount;
    }
    else
    {
        dividerCount = volume;

        if (decayLevel > 0)
            --decayLevel;
        else if (loopFlag)
            decayLevel = 15;
    }
}

uint8_t EnvelopeGenerator::output() const
{
    return constantVolume ? volume : decayLevel;
}

} // namespace cart
