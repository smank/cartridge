#pragma once

#include <array>
#include <cstdint>
#include <cmath>

namespace cart {

// ─── Clock Rates ────────────────────────────────────────────────────────────
inline constexpr double CPU_CLOCK_NTSC = 1789772.5;
inline constexpr double CPU_CLOCK_PAL  = 1662607.0;

// Frame counter rates (approximate)
inline constexpr double FRAME_COUNTER_RATE_NTSC = 240.0;   // quarter-frame
inline constexpr double FRAME_COUNTER_RATE_PAL  = 200.0;

// ─── Pulse Duty Cycle Sequences ─────────────────────────────────────────────
// Each duty cycle is 8 steps; 1 = high, 0 = low
// Index: 0=12.5%, 1=25%, 2=50%, 3=75%
inline constexpr std::array<std::array<uint8_t, 8>, 4> DUTY_TABLE = {{
    {{ 0, 0, 0, 0, 0, 0, 0, 1 }},   // 12.5%
    {{ 0, 0, 0, 0, 0, 0, 1, 1 }},   // 25%
    {{ 0, 0, 0, 0, 1, 1, 1, 1 }},   // 50%
    {{ 1, 1, 1, 1, 1, 1, 0, 0 }},   // 75% (inverted 25%)
}};

// Duty ratios as floats for PolyBLEP
inline constexpr std::array<float, 4> DUTY_RATIOS = {{ 0.125f, 0.25f, 0.5f, 0.75f }};

// ─── Triangle Waveform ──────────────────────────────────────────────────────
// 32-step sequence: 15, 14, 13 ... 1, 0, 0, 1, 2 ... 14, 15
inline constexpr std::array<uint8_t, 32> TRIANGLE_SEQUENCE = {{
    15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
     0,  1,  2,  3,  4,  5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
}};

// ─── Length Counter Lookup Table ────────────────────────────────────────────
// Indexed by the 5-bit length counter load value
inline constexpr std::array<uint8_t, 32> LENGTH_TABLE = {{
    10, 254, 20,  2, 40,  4, 80,  6,
    160,   8, 60, 10, 14, 12, 26, 14,
    12,  16, 24, 18, 48, 20, 96, 22,
    192,  24, 72, 26, 16, 28, 32, 30
}};

// ─── Noise Period Lookup Table (NTSC) ───────────────────────────────────────
// Timer period values for the noise channel
inline constexpr std::array<uint16_t, 16> NOISE_PERIOD_NTSC = {{
    4, 8, 16, 32, 64, 96, 128, 160,
    202, 254, 380, 508, 762, 1016, 2034, 4068
}};

inline constexpr std::array<uint16_t, 16> NOISE_PERIOD_PAL = {{
    4, 8, 14, 30, 60, 88, 118, 148,
    188, 236, 354, 472, 708,  944, 1890, 3778
}};

// ─── DPCM Rate Table (NTSC) ────────────────────────────────────────────────
// CPU cycles per DPCM sample
inline constexpr std::array<uint16_t, 16> DPCM_RATE_NTSC = {{
    428, 380, 340, 320, 286, 254, 226, 214,
    190, 160, 142, 128, 106,  84,  72,  54
}};

inline constexpr std::array<uint16_t, 16> DPCM_RATE_PAL = {{
    398, 354, 316, 298, 276, 236, 210, 198,
    176, 148, 132, 118,  98,  78,  66,  50
}};

// ─── Mixer Lookup Tables ────────────────────────────────────────────────────
// Precomputed nonlinear DAC mixing tables.
// pulse_out = 95.52 / (8128.0 / n + 100.0)  where n = pulse1 + pulse2 (0–30)
// tnd_out   = 163.67 / (24329.0 / n + 100.0) where n = 3*tri + 2*noise + dpcm (0–202)

struct MixerTables
{
    std::array<float, 31>  pulseTable {};
    std::array<float, 203> tndTable {};

    constexpr MixerTables()
    {
        pulseTable[0] = 0.0f;
        for (int n = 1; n <= 30; ++n)
            pulseTable[static_cast<size_t>(n)] = static_cast<float>(95.52 / (8128.0 / n + 100.0));

        tndTable[0] = 0.0f;
        for (int n = 1; n <= 202; ++n)
            tndTable[static_cast<size_t>(n)] = static_cast<float>(163.67 / (24329.0 / n + 100.0));
    }
};

inline const MixerTables& getMixerTables()
{
    static const MixerTables tables;
    return tables;
}

} // namespace cart
