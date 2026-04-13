#include "NoiseChannel.h"
#include <algorithm>

namespace cart {

void NoiseChannel::reset()
{
    phase  = 0.0;
    lfsr   = 1;
    gateOn = false;
    env.reset();
    length.reset();
}

void NoiseChannel::setSampleRate (double sr)
{
    sampleRate = sr;
}

void NoiseChannel::setPeriodIndex (int index, bool ntsc)
{
    index = std::clamp (index, 0, 15);
    cpuClock = ntsc ? CPU_CLOCK_NTSC : CPU_CLOCK_PAL;
    uint16_t period = ntsc ? NOISE_PERIOD_NTSC[static_cast<size_t> (index)]
                           : NOISE_PERIOD_PAL[static_cast<size_t> (index)];

    // Noise clocks at cpuClock / period rate
    double noiseFreq = cpuClock / static_cast<double> (period);
    phaseInc = noiseFreq / sampleRate;
}

void NoiseChannel::setEnabled (bool en)
{
    length.setEnabled (en);
    if (!en)
        gateOn = false;
}

void NoiseChannel::noteOn()
{
    gateOn = true;
    lfsr   = 1;
    env.start();
    length.load (0x1F);
}

void NoiseChannel::noteOff()
{
    gateOn = false;
}

void NoiseChannel::clockQuarterFrame()
{
    env.clock();
}

void NoiseChannel::clockHalfFrame()
{
    length.clock();
}

void NoiseChannel::clockLfsr()
{
    // Bit 0 XOR bit 1 (long mode) or bit 0 XOR bit 6 (short mode)
    uint16_t bit = useShortMode
        ? ((lfsr & 1) ^ ((lfsr >> 6) & 1))
        : ((lfsr & 1) ^ ((lfsr >> 1) & 1));

    lfsr = static_cast<uint16_t> ((lfsr >> 1) | (bit << 14));
}

bool NoiseChannel::isActive() const
{
    return gateOn && length.isActive();
}

uint8_t NoiseChannel::output() const
{
    if (!isActive())
        return 0;

    // Output is 0 when bit 0 of LFSR is set (inverted logic)
    return (lfsr & 1) ? 0 : env.output();
}

float NoiseChannel::process()
{
    if (!isActive() || phaseInc <= 0.0)
        return 0.0f;

    // Read current LFSR state
    float sample = (lfsr & 1) ? 0.0f : 1.0f;
    float vol = static_cast<float> (env.output()) / 15.0f;

    // Advance phase accumulator, clock LFSR when phase wraps
    phase += phaseInc;
    while (phase >= 1.0)
    {
        phase -= 1.0;
        clockLfsr();
    }

    return sample * vol + 1e-25f;
}

} // namespace cart
