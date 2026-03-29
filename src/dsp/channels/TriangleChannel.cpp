#include "TriangleChannel.h"
#include <cmath>

namespace cart {

void TriangleChannel::reset()
{
    phase       = 0.0;
    phaseInc    = 0.0;
    sequencePos = 0;
    gateOn      = false;
    length.reset();
    linear.reset();
}

void TriangleChannel::setSampleRate (double sr)
{
    sampleRate = sr;
    phaseInc   = static_cast<double> (frequency) / sampleRate;
}

void TriangleChannel::setFrequency (float freqHz)
{
    frequency = freqHz;
    phaseInc  = static_cast<double> (frequency) / sampleRate;
}

void TriangleChannel::setTimerPeriod (uint16_t period, double cpuClock)
{
    if (period > 0)
    {
        // Triangle clocks at CPU/32 (half the rate of pulse)
        frequency = static_cast<float> (cpuClock / (32.0 * (period + 1)));
        phaseInc  = static_cast<double> (frequency) / sampleRate;
    }
}

void TriangleChannel::setEnabled (bool en)
{
    length.setEnabled (en);
    if (!en)
        gateOn = false;
}

void TriangleChannel::noteOn()
{
    gateOn = true;
    length.load (0x1F);
    linear.reload();
    // Immediately prime the linear counter so we don't wait for a quarter-frame
    linear.clock();
}

void TriangleChannel::noteOff()
{
    gateOn = false;
}

void TriangleChannel::clockQuarterFrame()
{
    linear.clock();
}

void TriangleChannel::clockHalfFrame()
{
    length.clock();
}

bool TriangleChannel::isActive() const
{
    return gateOn && length.isActive() && linear.isActive();
}

uint8_t TriangleChannel::output() const
{
    if (!isActive())
        return 0;

    return TRIANGLE_SEQUENCE[static_cast<size_t> (sequencePos)];
}

float TriangleChannel::process()
{
    if (!isActive() || phaseInc <= 0.0)
        return 0.0f;

    // Advance phase and update sequence position FIRST
    phase += phaseInc;
    while (phase >= 1.0)
    {
        phase -= 1.0;
    }

    // Map phase to sequence position (0–31)
    sequencePos = static_cast<int> (phase * 32.0) % 32;

    // Read current step from the 32-step triangle sequence
    uint8_t step = TRIANGLE_SEQUENCE[static_cast<size_t> (sequencePos)];
    float sample = (static_cast<float> (step) / 7.5f) - 1.0f;  // Normalize to [-1, 1]

    return sample;
}

} // namespace cart
