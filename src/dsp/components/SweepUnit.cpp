#include "SweepUnit.h"

namespace cart {

void SweepUnit::reset()
{
    reloadFlag = false;
    divider    = 0;
}

uint16_t SweepUnit::targetPeriod (uint16_t currentPeriod) const
{
    uint16_t change = currentPeriod >> shift;

    if (negate)
    {
        // Pulse 1 uses one's complement (subtracts change + 1)
        // Pulse 2 uses two's complement (subtracts change)
        if (channel == 0)
            return currentPeriod - change - 1;
        else
            return currentPeriod - change;
    }

    return currentPeriod + change;
}

bool SweepUnit::isMuting (uint16_t currentPeriod) const
{
    return currentPeriod < 8 || targetPeriod (currentPeriod) > 0x7FF;
}

bool SweepUnit::clock (uint16_t& timerPeriod)
{
    bool periodChanged = false;

    if (reloadFlag)
    {
        if (divider == 0 && enabled && shift > 0 && !isMuting (timerPeriod))
        {
            timerPeriod = targetPeriod (timerPeriod);
            periodChanged = true;
        }
        divider    = period;
        reloadFlag = false;
    }
    else if (divider > 0)
    {
        --divider;
    }
    else
    {
        divider = period;

        if (enabled && shift > 0 && !isMuting (timerPeriod))
        {
            timerPeriod = targetPeriod (timerPeriod);
            periodChanged = true;
        }
    }

    return periodChanged;
}

} // namespace cart
