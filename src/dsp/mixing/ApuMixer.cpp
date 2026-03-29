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

} // namespace cart
