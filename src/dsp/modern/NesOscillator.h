#pragma once

#include <cstdint>
#include <cmath>
#include <array>

namespace cart {

/// Waveform types available in the Modern engine.
enum class NesWaveform
{
    Pulse25,     // 25% duty cycle (NES default)
    Pulse50,     // 50% duty cycle (square wave)
    Pulse75,     // 75% duty cycle (inverted 25%)
    Pulse125,    // 12.5% duty cycle (thin)
    Triangle,    // 32-step quantized triangle
    Sawtooth,    // VRC6-style accumulator sawtooth
    Noise        // 15-bit LFSR noise
};

/// Lightweight waveform generator for the Modern polyphonic engine.
/// Extracts just the DSP math from PulseChannel/TriangleChannel/NoiseChannel/Vrc6SawChannel
/// without any NES hardware components (no envelope, length counter, sweep).
class NesOscillator
{
public:
    void setSampleRate (double sr)
    {
        sampleRate = sr;
        updatePhaseInc();
    }

    void setFrequency (float hz)
    {
        frequency = hz;
        updatePhaseInc();
    }

    void setWaveform (NesWaveform w)
    {
        waveform = w;
        updateDutyFromWaveform();
    }

    /// For Sawtooth: accumulator rate (0-63). Clean max is 42; above causes distortion.
    void setSawRate (int r) { sawRate = static_cast<uint8_t> (r & 0x3F); }

    /// For Noise: short mode (93 steps) vs long mode (32767 steps)
    void setShortMode (bool s) { shortMode = s; }

    void noteOn()
    {
        phase = 0.0;

        // Reset noise LFSR
        if (waveform == NesWaveform::Noise)
            lfsr = 1;

        // Reset saw accumulator
        if (waveform == NesWaveform::Sawtooth)
        {
            sawAccumulator = 0;
            sawStep = 0;
        }
    }

    float process()
    {
        if (phaseInc <= 0.0)
            return 0.0f;

        switch (waveform)
        {
            case NesWaveform::Pulse25:
            case NesWaveform::Pulse50:
            case NesWaveform::Pulse75:
            case NesWaveform::Pulse125:
                return processPulse();

            case NesWaveform::Triangle:
                return processTriangle();

            case NesWaveform::Sawtooth:
                return processSawtooth();

            case NesWaveform::Noise:
                return processNoise();
        }

        return 0.0f;
    }

    void reset()
    {
        phase = 0.0;
        lfsr = 1;
        sawAccumulator = 0;
        sawStep = 0;
    }

private:
    // ─── Pulse (PolyBLEP anti-aliased) ────────────────────────────────────
    float processPulse()
    {
        float p = static_cast<float> (phase);

        // Naive pulse: high if phase < dutyRatio
        float sample = (p < dutyRatio) ? 1.0f : -1.0f;

        // PolyBLEP correction at rising edge (phase = 0)
        sample += polyBlep (p);

        // PolyBLEP correction at falling edge (phase = dutyRatio)
        float distFromDuty = p - dutyRatio;
        if (distFromDuty < 0.0f)
            distFromDuty += 1.0f;
        sample -= polyBlep (distFromDuty);

        advancePhase();
        return sample;
    }

    float polyBlep (float t) const
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

    // ─── Triangle (32-step quantized) ─────────────────────────────────────
    float processTriangle()
    {
        advancePhase();

        // Map phase to sequence position (0-31)
        int pos = static_cast<int> (phase * 32.0) % 32;

        // 32-step triangle sequence: 15, 14, 13...1, 0, 0, 1, 2...14, 15
        static constexpr std::array<uint8_t, 32> TRIANGLE_SEQ = {{
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
             0,  1,  2,  3,  4,  5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
        }};

        float sample = (static_cast<float> (TRIANGLE_SEQ[static_cast<size_t> (pos)]) / 7.5f) - 1.0f;
        return sample;
    }

    // ─── Sawtooth (VRC6-style accumulator) ────────────────────────────────
    float processSawtooth()
    {
        // Each full cycle = 14 internal clocks
        phase += phaseInc * 14.0;
        while (phase >= 1.0)
        {
            phase -= 1.0;

            sawStep++;
            if (sawStep >= 14)
            {
                sawStep = 0;
                sawAccumulator = 0;
            }
            else if ((sawStep & 1) != 0)
            {
                // Add rate on odd clocks (steps 1, 3, 5, 7, 9, 11)
                sawAccumulator = (sawAccumulator + sawRate) & 0xFF;
            }
        }

        // Output is top 5 bits of 8-bit accumulator, normalized to [-1, 1]
        return (static_cast<float> (sawAccumulator >> 3) / 15.5f) - 1.0f;
    }

    // ─── Noise (15-bit LFSR) ──────────────────────────────────────────────
    float processNoise()
    {
        // Read current LFSR state
        float sample = (lfsr & 1) ? 0.0f : 1.0f;

        // Advance phase accumulator, clock LFSR when phase wraps
        phase += phaseInc;
        while (phase >= 1.0)
        {
            phase -= 1.0;
            clockLfsr();
        }

        // Normalize to [-1, 1]
        return sample * 2.0f - 1.0f;
    }

    void clockLfsr()
    {
        uint16_t bit = shortMode
            ? ((lfsr & 1) ^ ((lfsr >> 6) & 1))
            : ((lfsr & 1) ^ ((lfsr >> 1) & 1));

        lfsr = static_cast<uint16_t> ((lfsr >> 1) | (bit << 14));
    }

    // ─── Common ───────────────────────────────────────────────────────────
    void advancePhase()
    {
        phase += phaseInc;
        if (phase >= 1.0)
            phase -= 1.0;
    }

    void updatePhaseInc()
    {
        if (sampleRate > 0.0 && frequency > 0.0f)
            phaseInc = static_cast<double> (frequency) / sampleRate;
        else
            phaseInc = 0.0;
    }

    void updateDutyFromWaveform()
    {
        switch (waveform)
        {
            case NesWaveform::Pulse125:  dutyRatio = 0.125f; break;
            case NesWaveform::Pulse25:   dutyRatio = 0.25f;  break;
            case NesWaveform::Pulse50:   dutyRatio = 0.5f;   break;
            case NesWaveform::Pulse75:   dutyRatio = 0.75f;  break;
            case NesWaveform::Triangle:
            case NesWaveform::Sawtooth:
            case NesWaveform::Noise:     break;
        }
    }

    NesWaveform waveform   = NesWaveform::Pulse50;
    double      sampleRate = 44100.0;
    float       frequency  = 440.0f;
    double      phase      = 0.0;
    double      phaseInc   = 0.0;
    float       dutyRatio  = 0.5f;

    // Noise state
    uint16_t    lfsr       = 1;
    bool        shortMode  = false;

    // Sawtooth state
    uint8_t     sawRate    = 42;
    uint8_t     sawAccumulator = 0;
    int         sawStep    = 0;
};

} // namespace cart
