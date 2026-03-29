#pragma once

#include <cstdint>
#include "../ApuConstants.h"

namespace cart {

/// VRC6 sawtooth channel.
/// Accumulator-based: adds rate value 6 times, resets on 7th step.
/// Top 5 bits of 8-bit accumulator form the output.
/// Rate > 42 causes overflow distortion (intentional effect).
class Vrc6SawChannel
{
public:
    void reset();
    void setSampleRate (double sr);

    void setFrequency (float freqHz);

    /// Accumulator rate (0–63). Clean max is 42; above causes distortion.
    void setRate (uint8_t r)        { rate = r & 0x3F; }

    void setEnabled (bool en)       { enabled = en; }
    void noteOn (float freqHz);
    void noteOff();

    float process();
    bool isActive() const           { return enabled && gateOn; }

private:
    double   sampleRate   = 44100.0;
    float    frequency    = 440.0f;
    double   phase        = 0.0;
    double   phaseInc     = 0.0;

    uint8_t  rate         = 42;      // accumulator increment (0–63)
    uint8_t  accumulator  = 0;
    int      step         = 0;       // 0–13 (7 pairs of clocks)
    bool     enabled      = true;
    bool     gateOn       = false;
};

} // namespace cart
