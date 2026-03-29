#pragma once

#include <cstdint>

namespace cart {

/// APU linear counter (used by triangle channel).
/// Clocked on quarter-frames. When it reaches zero, the triangle is silenced.
class LinearCounter
{
public:
    void reset();

    /// Clock on quarter-frame tick
    void clock();

    /// Set the reload value (7 bits)
    void setReloadValue (uint8_t val)   { reloadValue = val & 0x7F; }

    /// Set the control flag (also halts length counter when set)
    void setControl (bool ctrl)         { controlFlag = ctrl; }

    /// Trigger a reload on the next quarter-frame
    void reload()                       { reloadFlag = true; }

    bool isActive() const               { return counter > 0; }
    uint8_t value() const               { return counter; }

private:
    uint8_t counter      = 0;
    uint8_t reloadValue  = 0;
    bool    controlFlag  = false;
    bool    reloadFlag   = false;
};

} // namespace cart
