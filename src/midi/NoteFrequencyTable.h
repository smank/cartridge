#pragma once

#include <cmath>
#include <cstdint>

namespace cart {

/// Convert MIDI note number (0–127) to frequency in Hz.
/// A4 (note 69) = 440 Hz. Supports fractional notes for pitch bend.
inline float midiNoteToFrequency (float midiNote)
{
    return 440.0f * std::pow (2.0f, (midiNote - 69.0f) / 12.0f);
}

/// Convert MIDI note to timer period for pulse channels.
/// period = cpuClock / (16 * frequency) - 1
inline uint16_t midiNoteToTimerPeriod (int midiNote, double cpuClock)
{
    float freq = midiNoteToFrequency (static_cast<float> (midiNote));
    if (freq <= 0.0f)
        return 0;

    double period = cpuClock / (16.0 * freq) - 1.0;
    if (period < 0.0)
        return 0;
    if (period > 0x7FF)
        return 0x7FF;

    return static_cast<uint16_t> (period + 0.5);
}

/// Convert MIDI note to timer period for triangle channel.
/// Triangle period = cpuClock / (32 * frequency) - 1
inline uint16_t midiNoteToTrianglePeriod (int midiNote, double cpuClock)
{
    float freq = midiNoteToFrequency (static_cast<float> (midiNote));
    if (freq <= 0.0f)
        return 0;

    double period = cpuClock / (32.0 * freq) - 1.0;
    if (period < 0.0)
        return 0;
    if (period > 0x7FF)
        return 0x7FF;

    return static_cast<uint16_t> (period + 0.5);
}

} // namespace cart
