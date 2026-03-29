#pragma once

#include <cstdint>

namespace cart {

/// APU envelope generator.
/// Clocked on quarter-frames (~240 Hz). Produces a 4-bit volume (0–15).
/// In constant-volume mode, outputs the volume register directly.
/// In decay mode, counts down from 15 to 0, optionally looping.
class EnvelopeGenerator
{
public:
    void reset();

    /// Call on quarter-frame tick
    void clock();

    /// Set parameters (typically from register writes or parameter updates)
    void setConstantVolume (bool constant)    { constantVolume = constant; }
    void setVolume (uint8_t vol)              { volume = vol & 0x0F; }
    void setLoop (bool loop)                  { loopFlag = loop; }

    /// Trigger start of new envelope
    void start();

    /// Current 4-bit output volume
    uint8_t output() const;

private:
    bool     constantVolume = true;
    uint8_t  volume         = 15;      // Also serves as envelope period
    bool     loopFlag       = false;
    bool     startFlag      = false;
    uint8_t  decayLevel     = 0;
    uint8_t  dividerCount   = 0;
};

} // namespace cart
