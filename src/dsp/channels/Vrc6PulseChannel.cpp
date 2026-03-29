#include "Vrc6PulseChannel.h"
#include <cmath>
#include <algorithm>

namespace cart {

void Vrc6PulseChannel::reset()
{
    phase   = 0.0;
    gateOn  = false;
}

void Vrc6PulseChannel::setSampleRate (double sr)
{
    sampleRate = sr;
    phaseInc   = static_cast<double> (frequency) / sampleRate;
}

void Vrc6PulseChannel::setFrequency (float freqHz)
{
    frequency = freqHz;
    phaseInc  = static_cast<double> (frequency) / sampleRate;
}

void Vrc6PulseChannel::setDuty (int duty)
{
    dutyLevel = std::clamp (duty, 0, 7);
    dutyRatio = static_cast<float> (dutyLevel + 1) / 16.0f;
}

void Vrc6PulseChannel::noteOn (float freqHz)
{
    setFrequency (freqHz);
    gateOn = true;
}

void Vrc6PulseChannel::noteOff()
{
    gateOn = false;
}

float Vrc6PulseChannel::polyBlep (float t) const
{
    float dt = static_cast<float> (phaseInc);
    if (dt <= 0.0f)
        return 0.0f;

    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0f;
    }
    else if (t > 1.0f - dt)
    {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }

    return 0.0f;
}

float Vrc6PulseChannel::process()
{
    if (!isActive() || phaseInc <= 0.0)
        return 0.0f;

    float vol = static_cast<float> (volume) / 15.0f;

    // Digital mode: constant output at volume level
    if (digitalMode)
    {
        phase += phaseInc;
        if (phase >= 1.0) phase -= 1.0;
        return vol;
    }

    float p = static_cast<float> (phase);

    // Naive pulse
    float sample = (p < dutyRatio) ? 1.0f : -1.0f;

    // PolyBLEP at rising edge (phase = 0)
    sample += polyBlep (p);

    // PolyBLEP at falling edge (phase = dutyRatio)
    float distFromDuty = p - dutyRatio;
    if (distFromDuty < 0.0f)
        distFromDuty += 1.0f;
    sample -= polyBlep (distFromDuty);

    phase += phaseInc;
    if (phase >= 1.0)
        phase -= 1.0;

    return sample * vol;
}

} // namespace cart
