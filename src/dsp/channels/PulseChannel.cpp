#include "PulseChannel.h"
#include <cmath>
#include <algorithm>

namespace cart {

PulseChannel::PulseChannel (int channelIdx)
{
    sweepUnit.setChannel (channelIdx);
}

void PulseChannel::reset()
{
    phase      = 0.0;
    phaseInc   = 0.0;
    gateOn     = false;
    env.reset();
    length.reset();
    sweepUnit.reset();
}

void PulseChannel::setSampleRate (double sr)
{
    sampleRate = sr;
    phaseInc   = static_cast<double> (frequency) / sampleRate;
}

void PulseChannel::setFrequency (float freqHz)
{
    frequency = freqHz;
    phaseInc  = static_cast<double> (frequency) / sampleRate;

    // Keep timerPeriod in sync so sweep muting check works correctly
    if (frequency > 0.0f)
    {
        double p = CPU_CLOCK_NTSC / (16.0 * frequency) - 1.0;
        timerPeriod = static_cast<uint16_t> (std::clamp (p, 0.0, 2047.0));
    }
}

void PulseChannel::setTimerPeriod (uint16_t period, double cpuClock)
{
    timerPeriod = period;
    if (period > 0)
    {
        frequency = static_cast<float> (cpuClock / (16.0 * (period + 1)));
        phaseInc  = static_cast<double> (frequency) / sampleRate;
    }
}

void PulseChannel::setDuty (int idx)
{
    dutyIndex = std::clamp (idx, 0, 3);
    dutyRatio = DUTY_RATIOS[static_cast<size_t> (dutyIndex)];
}

void PulseChannel::setEnabled (bool en)
{
    length.setEnabled (en);
    if (!en)
        gateOn = false;
}

void PulseChannel::noteOn()
{
    gateOn = true;
    env.start();
    length.load (0x1F);   // Max length
    sweepUnit.reload();
}

void PulseChannel::noteOff()
{
    gateOn = false;
}

void PulseChannel::clockQuarterFrame()
{
    env.clock();
}

void PulseChannel::clockHalfFrame()
{
    length.clock();
    sweepUnit.clock (timerPeriod);
}

bool PulseChannel::isActive() const
{
    return gateOn && length.isActive();
}

uint8_t PulseChannel::output() const
{
    if (!isActive())
        return 0;

    if (sweepUnit.isMuting (timerPeriod))
        return 0;

    // Determine if we're in the high or low part of the duty cycle
    bool high = (phase < static_cast<double> (dutyRatio));
    return high ? env.output() : 0;
}

float PulseChannel::polyBlep (float t) const
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

float PulseChannel::process()
{
    if (!isActive() || phaseInc <= 0.0)
        return 0.0f;

    if (sweepUnit.isMuting (timerPeriod))
        return 0.0f;

    float p = static_cast<float> (phase);
    float vol = static_cast<float> (env.output()) / 15.0f;

    // Naive pulse: high if phase < dutyRatio
    float sample = (p < dutyRatio) ? 1.0f : -1.0f;

    // PolyBLEP correction at rising edge (phase = 0)
    sample += polyBlep (p);

    // PolyBLEP correction at falling edge (phase = dutyRatio)
    float distFromDuty = p - dutyRatio;
    if (distFromDuty < 0.0f)
        distFromDuty += 1.0f;
    sample -= polyBlep (distFromDuty);

    // Advance phase
    phase += phaseInc;
    if (phase >= 1.0)
        phase -= 1.0;

    return sample * vol;
}

} // namespace cart
