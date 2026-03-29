#pragma once

#include <cstdint>
#include "../ApuConstants.h"
#include "../components/EnvelopeGenerator.h"
#include "../components/LengthCounter.h"
#include "../components/SweepUnit.h"

namespace cart {

/// Pulse wave channel with PolyBLEP anti-aliasing.
class PulseChannel
{
public:
    explicit PulseChannel (int channelIndex);

    void reset();
    void setSampleRate (double sr);

    /// Set frequency in Hz (from MIDI note)
    void setFrequency (float freqHz);

    /// Set timer period directly
    void setTimerPeriod (uint16_t period, double cpuClock);

    /// Set duty cycle index (0–3)
    void setDuty (int dutyIndex);

    // Component access
    EnvelopeGenerator& envelope()       { return env; }
    LengthCounter&     lengthCounter()  { return length; }
    SweepUnit&         sweep()          { return sweepUnit; }

    /// Clock on quarter-frame
    void clockQuarterFrame();

    /// Clock on half-frame
    void clockHalfFrame();

    /// Generate one sample
    float process();

    /// Is the channel producing sound?
    bool isActive() const;

    /// Current 4-bit output (0–15)
    uint8_t output() const;

    void setEnabled (bool en);
    void noteOn();
    void noteOff();

private:
    float polyBlep (float t) const;

    double sampleRate = 44100.0;
    float  frequency  = 440.0f;
    double phase      = 0.0;
    double phaseInc   = 0.0;
    int    dutyIndex  = 1;       // 25% default
    float  dutyRatio  = 0.25f;

    bool   gateOn     = false;

    EnvelopeGenerator env;
    LengthCounter     length;
    SweepUnit         sweepUnit;
    uint16_t          timerPeriod = 0;
};

} // namespace cart
