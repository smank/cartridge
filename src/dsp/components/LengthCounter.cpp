#include "LengthCounter.h"

namespace cart {

void LengthCounter::reset()
{
    counter  = 0;
    haltFlag = false;
    enabled  = false;
}

void LengthCounter::clock()
{
    if (counter > 0 && !haltFlag)
        --counter;
}

void LengthCounter::load (uint8_t index)
{
    if (enabled)
        counter = LENGTH_TABLE[index & 0x1F];
}

} // namespace cart
