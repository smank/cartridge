#include "LinearCounter.h"

namespace cart {

void LinearCounter::reset()
{
    counter     = 0;
    reloadFlag  = false;
}

void LinearCounter::clock()
{
    if (reloadFlag)
    {
        counter = reloadValue;
    }
    else if (counter > 0)
    {
        --counter;
    }

    if (!controlFlag)
        reloadFlag = false;
}

} // namespace cart
