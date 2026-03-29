#pragma once

#include <cstdint>
#include "../ApuConstants.h"

namespace cart {

/// APU length counter. Clocked on half-frames (~120 Hz).
/// When it reaches zero, the channel is silenced.
class LengthCounter
{
public:
    void reset();

    /// Clock on half-frame tick
    void clock();

    /// Load counter using the 5-bit lookup index
    void load (uint8_t index);

    /// Set the halt flag (also serves as envelope loop flag on some channels)
    void setHalt (bool halt)    { haltFlag = halt; }

    /// Enable/disable the counter
    void setEnabled (bool en)   { enabled = en; if (!en) counter = 0; }

    /// Is the channel active (counter > 0)?
    bool isActive() const       { return counter > 0; }

    uint8_t value() const       { return counter; }

private:
    uint8_t counter  = 0;
    bool    haltFlag = false;
    bool    enabled  = false;
};

} // namespace cart
