#pragma once

#include <cstdint>
#include "../ApuConstants.h"
#include "../components/LengthCounter.h"
#include "../components/LinearCounter.h"

namespace cart {

/// Triangle wave channel.
/// 32-step quantized waveform (values 15→0→0→15), fixed volume.
class TriangleChannel
{
public:
    void reset();
    void setSampleRate (double sr);

    void setFrequency (float freqHz);
    void setTimerPeriod (uint16_t period, double cpuClock);

    LengthCounter&  lengthCounter()  { return length; }
    LinearCounter&  linearCounter()  { return linear; }

    void clockQuarterFrame();
    void clockHalfFrame();

    /// Generate one sample
    float process();

    /// Current 4-bit output (0–15)
    uint8_t output() const;

    bool isActive() const;

    void setEnabled (bool en);
    void noteOn();
    void noteOff();

private:
    double sampleRate  = 44100.0;
    float  frequency   = 440.0f;
    double phase       = 0.0;
    double phaseInc    = 0.0;
    int    sequencePos = 0;

    bool   gateOn      = false;

    LengthCounter  length;
    LinearCounter  linear;
};

} // namespace cart
