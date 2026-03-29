#pragma once

#include <cstdint>
#include "../ApuConstants.h"
#include "../components/EnvelopeGenerator.h"
#include "../components/LengthCounter.h"

namespace cart {

/// Noise channel. 15-bit LFSR with long and short modes.
class NoiseChannel
{
public:
    void reset();
    void setSampleRate (double sr);

    /// Set noise period index (0–15)
    void setPeriodIndex (int index, bool ntsc = true);

    /// Set mode: false = long (32767 steps), true = short (93 steps)
    void setShortMode (bool shortMode)  { useShortMode = shortMode; }

    EnvelopeGenerator& envelope()       { return env; }
    LengthCounter&     lengthCounter()  { return length; }

    void clockQuarterFrame();
    void clockHalfFrame();

    float process();

    uint8_t output() const;
    bool isActive() const;

    void setEnabled (bool en);
    void noteOn();
    void noteOff();

private:
    void clockLfsr();

    double   sampleRate   = 44100.0;
    double   phase        = 0.0;
    double   phaseInc     = 0.0;
    double   cpuClock     = CPU_CLOCK_NTSC;

    uint16_t lfsr         = 1;       // 15-bit, initial value
    bool     useShortMode = false;
    bool     gateOn       = false;

    EnvelopeGenerator env;
    LengthCounter     length;
};

} // namespace cart
