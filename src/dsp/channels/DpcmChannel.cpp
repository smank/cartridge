#include "DpcmChannel.h"
#include <algorithm>

namespace cart {

void DpcmChannel::reset()
{
    phase        = 0.0;
    sampleIndex  = 0;
    bitsRemaining = 0;
    currentByte  = 0;
    outputLevel  = 64;
    playing      = false;
}

void DpcmChannel::setSampleRate (double sr)
{
    sampleRate = sr;
}

void DpcmChannel::loadSample (const std::vector<uint8_t>& data)
{
    sampleData = data;
}

void DpcmChannel::setRateIndex (int index, bool ntsc)
{
    index = std::clamp (index, 0, 15);
    double cpuClock = ntsc ? CPU_CLOCK_NTSC : CPU_CLOCK_PAL;
    uint16_t period = ntsc ? DPCM_RATE_NTSC[static_cast<size_t> (index)]
                           : DPCM_RATE_PAL[static_cast<size_t> (index)];

    // DPCM outputs at cpuClock / period rate (per bit)
    double dpcmFreq = cpuClock / static_cast<double> (period);
    phaseInc = dpcmFreq / sampleRate;
}

void DpcmChannel::setEnabled (bool en)
{
    enabled = en;
    if (!en)
        playing = false;
}

void DpcmChannel::noteOn()
{
    if (sampleData.empty())
        return;

    sampleIndex   = 0;
    bitsRemaining = 0;
    outputLevel   = 64;
    playing       = true;
}

void DpcmChannel::noteOff()
{
    playing = false;
}

float DpcmChannel::process()
{
    if (!playing || sampleData.empty())
        return (static_cast<float> (outputLevel) / 63.5f) - 1.0f;

    // Advance phase, process bits when phase wraps
    phase += phaseInc;
    while (phase >= 1.0)
    {
        phase -= 1.0;

        if (bitsRemaining == 0)
        {
            if (sampleIndex < sampleData.size())
            {
                currentByte = sampleData[sampleIndex++];
                bitsRemaining = 8;
            }
            else if (loopEnabled)
            {
                sampleIndex = 0;
                if (!sampleData.empty())
                {
                    currentByte = sampleData[sampleIndex++];
                    bitsRemaining = 8;
                }
            }
            else
            {
                playing = false;
                break;
            }
        }

        if (bitsRemaining > 0)
        {
            // Delta modulation: bit=1 adds 2, bit=0 subtracts 2
            if (currentByte & 1)
            {
                if (outputLevel <= 125)
                    outputLevel += 2;
            }
            else
            {
                if (outputLevel >= 2)
                    outputLevel -= 2;
            }

            currentByte >>= 1;
            --bitsRemaining;
        }
    }

    // Compute sample AFTER bit processing to reflect current outputLevel
    float sample = (static_cast<float> (outputLevel) / 63.5f) - 1.0f;

    return sample;
}

} // namespace cart
