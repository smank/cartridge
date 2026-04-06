#include "Apu.h"

namespace cart {

Apu::Apu()
{
    setRegion (true);
}

void Apu::reset()
{
    pulse1Ch.reset();
    pulse2Ch.reset();
    triangleCh.reset();
    noiseCh.reset();
    dpcmCh.reset();
    vrc6Pulse1Ch.reset();
    vrc6Pulse2Ch.reset();
    vrc6SawCh.reset();

    framePhase = 0.0;
    frameStep  = 0;
}

void Apu::setSampleRate (double sr)
{
    sampleRate = sr;
    framePhaseInc = (isNtsc ? FRAME_COUNTER_RATE_NTSC : FRAME_COUNTER_RATE_PAL) / sampleRate;

    pulse1Ch.setSampleRate (sr);
    pulse2Ch.setSampleRate (sr);
    triangleCh.setSampleRate (sr);
    noiseCh.setSampleRate (sr);
    dpcmCh.setSampleRate (sr);
    vrc6Pulse1Ch.setSampleRate (sr);
    vrc6Pulse2Ch.setSampleRate (sr);
    vrc6SawCh.setSampleRate (sr);
}

void Apu::setRegion (bool ntsc)
{
    isNtsc = ntsc;
    if (sampleRate > 0.0)
        framePhaseInc = (ntsc ? FRAME_COUNTER_RATE_NTSC : FRAME_COUNTER_RATE_PAL) / sampleRate;
}

void Apu::clockFrameCounter()
{
    // Quarter-frame: always (envelope, linear counter)
    pulse1Ch.clockQuarterFrame();
    pulse2Ch.clockQuarterFrame();
    triangleCh.clockQuarterFrame();
    noiseCh.clockQuarterFrame();

    // Half-frame: steps 1 and 3 (length counter, sweep)
    if (frameStep == 1 || frameStep == 3)
    {
        pulse1Ch.clockHalfFrame();
        pulse2Ch.clockHalfFrame();
        triangleCh.clockHalfFrame();
        noiseCh.clockHalfFrame();
    }

    frameStep = (frameStep + 1) % 4;
}

float Apu::process()
{
    // Clock the frame counter
    framePhase += framePhaseInc;
    while (framePhase >= 1.0)
    {
        framePhase -= 1.0;
        clockFrameCounter();
    }

    // Process base APU channels
    float p1  = pulse1Ch.process();
    float p2  = pulse2Ch.process();
    float tri = triangleCh.process();
    float noi = noiseCh.process();
    float dpc = dpcmCh.process();

    // Base APU mix using nonlinear DAC lookup tables
    float output = apuMixer.mixFloat (p1, p2, tri, noi, dpc);

    // VRC6 expansion channels
    if (vrc6Enabled)
    {
        float v6p1 = vrc6Pulse1Ch.process() * vrc6Pulse1Mix;
        float v6p2 = vrc6Pulse2Ch.process() * vrc6Pulse2Mix;
        float v6sw = vrc6SawCh.process()    * vrc6SawMix;

        // VRC6 channels mixed at similar level to base APU pulses
        output += (v6p1 * 0.20f + v6p2 * 0.20f + v6sw * 0.22f);
    }

    return output * masterVolume;
}

void Apu::processIndividual (float out[8])
{
    // Clock the frame counter
    framePhase += framePhaseInc;
    while (framePhase >= 1.0)
    {
        framePhase -= 1.0;
        clockFrameCounter();
    }

    // Process base APU channels
    float p1  = pulse1Ch.process();
    float p2  = pulse2Ch.process();
    float tri = triangleCh.process();
    float noi = noiseCh.process();
    float dpc = dpcmCh.process();

    // Per-channel nonlinear DAC
    float baseOut[5];
    apuMixer.mixIndividual (p1, p2, tri, noi, dpc, baseOut);

    out[0] = baseOut[0] * masterVolume;
    out[1] = baseOut[1] * masterVolume;
    out[2] = baseOut[2] * masterVolume;
    out[3] = baseOut[3] * masterVolume;
    out[4] = baseOut[4] * masterVolume;

    // VRC6 expansion channels
    if (vrc6Enabled)
    {
        out[5] = vrc6Pulse1Ch.process() * vrc6Pulse1Mix * 0.20f * masterVolume;
        out[6] = vrc6Pulse2Ch.process() * vrc6Pulse2Mix * 0.20f * masterVolume;
        out[7] = vrc6SawCh.process()    * vrc6SawMix    * 0.22f * masterVolume;
    }
    else
    {
        out[5] = 0.0f;
        out[6] = 0.0f;
        out[7] = 0.0f;
    }
}

} // namespace cart
