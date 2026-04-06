#include "ApuMixer.h"
#include "../ApuConstants.h"
#include <algorithm>

namespace cart {

ApuMixer::ApuMixer() = default;

float ApuMixer::mix (uint8_t pulse1, uint8_t pulse2,
                     uint8_t triangle, uint8_t noise, uint8_t dpcm) const
{
    const auto& tables = getMixerTables();

    // Apply individual mix levels by scaling the channel outputs
    float p1 = static_cast<float> (pulse1)   * pulse1MixLevel;
    float p2 = static_cast<float> (pulse2)   * pulse2MixLevel;
    float tri = static_cast<float> (triangle) * triangleMixLevel;
    float noi = static_cast<float> (noise)    * noiseMixLevel;
    float dpc = static_cast<float> (dpcm)     * dpcmMixLevel;

    // Pulse mix via lookup table
    int pulseIndex = static_cast<int> (p1 + p2);
    pulseIndex = std::clamp (pulseIndex, 0, 30);
    float pulseOut = tables.pulseTable[static_cast<size_t> (pulseIndex)];

    // TND mix via lookup table
    int tndIndex = static_cast<int> (3.0f * tri + 2.0f * noi + dpc);
    tndIndex = std::clamp (tndIndex, 0, 202);
    float tndOut = tables.tndTable[static_cast<size_t> (tndIndex)];

    // Combined output, centered around 0
    float out = pulseOut + tndOut;

    // Normalize: full-scale output is about 1.0, center it
    return (out - 0.5f) * 2.0f;
}

float ApuMixer::mixFloat (float pulse1, float pulse2,
                          float triangle, float noise, float dpcm) const
{
    // Scale float [-1,1] or [0,1] to integer ranges
    // Pulse channels: output is bipolar [-1,1] from PolyBLEP, map to 0-15
    // Triangle: bipolar [-1,1], map to 0-15
    // Noise: [0,1] output * envelope, map to 0-15
    // DPCM: [-1,1], map to 0-127

    auto scaleChannel = [] (float val, float maxVal) -> uint8_t {
        float scaled = (val + 1.0f) * 0.5f * maxVal;
        return static_cast<uint8_t> (std::clamp (scaled, 0.0f, maxVal));
    };

    uint8_t p1  = scaleChannel (pulse1, 15.0f);
    uint8_t p2  = scaleChannel (pulse2, 15.0f);
    uint8_t tri = scaleChannel (triangle, 15.0f);
    // Noise is already [0,1] range (unipolar), scale directly
    uint8_t noi = static_cast<uint8_t> (std::clamp (noise * 15.0f, 0.0f, 15.0f));
    uint8_t dpc = scaleChannel (dpcm, 127.0f);

    return mix (p1, p2, tri, noi, dpc);
}

void ApuMixer::mixIndividual (float pulse1, float pulse2,
                              float triangle, float noise, float dpcm,
                              float out[5]) const
{
    const auto& tables = getMixerTables();

    auto scaleChannel = [] (float val, float maxVal) -> uint8_t {
        float scaled = (val + 1.0f) * 0.5f * maxVal;
        return static_cast<uint8_t> (std::clamp (scaled, 0.0f, maxVal));
    };

    // Scale float to integer ranges and apply mix levels
    float p1f = static_cast<float> (scaleChannel (pulse1, 15.0f)) * pulse1MixLevel;
    float p2f = static_cast<float> (scaleChannel (pulse2, 15.0f)) * pulse2MixLevel;
    float trif = static_cast<float> (scaleChannel (triangle, 15.0f)) * triangleMixLevel;
    float noif = std::clamp (noise * 15.0f, 0.0f, 15.0f) * noiseMixLevel;
    float dpcf = static_cast<float> (scaleChannel (dpcm, 127.0f)) * dpcmMixLevel;

    // Per-channel through DAC curves (each channel solo'd through the table)
    int p1idx = std::clamp (static_cast<int> (p1f), 0, 30);
    out[0] = (tables.pulseTable[static_cast<size_t> (p1idx)] - 0.5f * (p1idx > 0 ? 1.0f : 0.0f)) * 2.0f;

    int p2idx = std::clamp (static_cast<int> (p2f), 0, 30);
    out[1] = (tables.pulseTable[static_cast<size_t> (p2idx)] - 0.5f * (p2idx > 0 ? 1.0f : 0.0f)) * 2.0f;

    // TND channels each go through the tnd curve individually
    int triIdx = std::clamp (static_cast<int> (3.0f * trif), 0, 202);
    out[2] = (tables.tndTable[static_cast<size_t> (triIdx)]) * 2.0f;

    int noiIdx = std::clamp (static_cast<int> (2.0f * noif), 0, 202);
    out[3] = (tables.tndTable[static_cast<size_t> (noiIdx)]) * 2.0f;

    int dpcIdx = std::clamp (static_cast<int> (dpcf), 0, 202);
    out[4] = (tables.tndTable[static_cast<size_t> (dpcIdx)]) * 2.0f;
}

} // namespace cart
