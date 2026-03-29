#include "Vrc6SawChannel.h"
#include <cmath>
#include <algorithm>

namespace cart {

void Vrc6SawChannel::reset()
{
    phase       = 0.0;
    accumulator = 0;
    step        = 0;
    gateOn      = false;
}

void Vrc6SawChannel::setSampleRate (double sr)
{
    sampleRate = sr;
    phaseInc   = static_cast<double> (frequency) / sampleRate;
}

void Vrc6SawChannel::setFrequency (float freqHz)
{
    frequency = freqHz;
    // VRC6 saw divides by 14 (7 accumulator steps * 2 clocks each)
    phaseInc  = static_cast<double> (frequency) / sampleRate;
}

void Vrc6SawChannel::noteOn (float freqHz)
{
    setFrequency (freqHz);
    accumulator = 0;
    step        = 0;
    gateOn      = true;
}

void Vrc6SawChannel::noteOff()
{
    gateOn = false;
}

float Vrc6SawChannel::process()
{
    if (!isActive() || phaseInc <= 0.0)
        return 0.0f;

    // Advance phase first to avoid first-sample click after noteOn
    // Each full cycle = 14 internal clocks
    phase += phaseInc * 14.0;
    while (phase >= 1.0)
    {
        phase -= 1.0;

        // The VRC6 saw clocks in pairs: on even clocks nothing, on odd clocks add rate
        // After 6 additions (step 13), reset accumulator on the 7th
        step++;
        if (step >= 14)
        {
            step = 0;
            accumulator = 0;
        }
        else if ((step & 1) != 0)
        {
            // Add rate on odd clocks (steps 1, 3, 5, 7, 9, 11 → 6 additions)
            accumulator = (accumulator + rate) & 0xFF;
            // Intentionally allow overflow for distortion when rate > 42
        }
    }

    // Output is top 5 bits of 8-bit accumulator, normalized to [-1, 1]
    return (static_cast<float> (accumulator >> 3) / 15.5f) - 1.0f;
}

} // namespace cart
