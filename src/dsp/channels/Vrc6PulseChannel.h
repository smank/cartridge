#pragma once

#include <cstdint>
#include "../ApuConstants.h"

namespace cart {

/// VRC6 pulse channel.
/// 8-level duty cycle (0–7), 4-bit direct volume, 12-bit period.
/// No hardware envelope or sweep — simpler than 2A03 pulse.
class Vrc6PulseChannel
{
public:
    void reset();
    void setSampleRate (double sr);

    void setFrequency (float freqHz);

    /// Set duty cycle (0–7): 0=1/16 (6.25%), 1=2/16 (12.5%) ... 7=8/16 (50%)
    void setDuty (int duty);

    /// Direct volume (0–15)
    void setVolume (uint8_t vol)    { volume = vol & 0x0F; }

    /// Digital mode: when true, outputs constant volume (ignores duty)
    void setDigitalMode (bool dm)   { digitalMode = dm; }

    void setEnabled (bool en)       { enabled = en; }
    void noteOn (float freqHz);
    void noteOff();

    float process();
    bool isActive() const           { return enabled && gateOn; }

private:
    float polyBlep (float t) const;

    double sampleRate  = 44100.0;
    float  frequency   = 440.0f;
    double phase       = 0.0;
    double phaseInc    = 0.0;

    int     dutyLevel  = 4;      // 0–7
    float   dutyRatio  = 0.3125f; // (dutyLevel+1)/16
    uint8_t volume     = 15;
    bool    digitalMode = false;
    bool    enabled    = true;
    bool    gateOn     = false;
};

} // namespace cart
