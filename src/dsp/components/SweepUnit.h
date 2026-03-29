#pragma once

#include <cstdint>

namespace cart {

/// APU sweep unit. Clocked on half-frames (~120 Hz).
/// Adjusts the pulse channel's timer period up or down.
class SweepUnit
{
public:
    /// Which pulse channel (0 or 1) — affects negate behavior
    void setChannel (int ch)              { channel = ch; }

    void reset();

    /// Clock on half-frame tick. Returns true if the period was modified.
    bool clock (uint16_t& timerPeriod);

    /// Trigger a reload of the divider
    void reload()                         { reloadFlag = true; }

    void setEnabled (bool en)             { enabled = en; }
    void setPeriod (uint8_t p)            { period = p; }
    void setNegate (bool neg)             { negate = neg; }
    void setShift (uint8_t s)             { shift = s; }

    /// Calculate the target period (for muting check)
    uint16_t targetPeriod (uint16_t currentPeriod) const;

    /// Is the channel muted by sweep?
    bool isMuting (uint16_t currentPeriod) const;

private:
    int      channel    = 0;
    bool     enabled    = false;
    uint8_t  period     = 0;
    bool     negate     = false;
    uint8_t  shift      = 0;
    bool     reloadFlag = false;
    uint8_t  divider    = 0;
};

} // namespace cart
