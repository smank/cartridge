#pragma once

#include <cstdint>

namespace cart {

/// Nonlinear DAC mixer using precomputed lookup tables.
class ApuMixer
{
public:
    ApuMixer();

    /// Mix all 5 channel outputs into a single float sample [-1, 1].
    /// pulse1/pulse2: 0–15, triangle: 0–15, noise: 0–15, dpcm: 0–127
    float mix (uint8_t pulse1, uint8_t pulse2,
               uint8_t triangle, uint8_t noise, uint8_t dpcm) const;

    /// Mix float channel outputs [-1, 1] using nonlinear DAC tables.
    /// Internally scales to integer ranges before applying lookup tables.
    float mixFloat (float pulse1, float pulse2,
                    float triangle, float noise, float dpcm) const;

    /// Per-channel nonlinear outputs for stereo panning.
    /// Each channel is individually scaled through the NES DAC curve.
    /// out[0]=pulse1, out[1]=pulse2, out[2]=tri, out[3]=noise, out[4]=dpcm
    void mixIndividual (float pulse1, float pulse2,
                        float triangle, float noise, float dpcm,
                        float out[5]) const;

    /// Per-channel mix levels (0.0–1.0)
    void setPulseMix (float level)     { pulseMixLevel = level; }
    void setTriangleMix (float level)  { triangleMixLevel = level; }
    void setNoiseMix (float level)     { noiseMixLevel = level; }
    void setDpcmMix (float level)      { dpcmMixLevel = level; }

    void setPulse1Mix (float level)    { pulse1MixLevel = level; }
    void setPulse2Mix (float level)    { pulse2MixLevel = level; }

private:
    float pulseMixLevel    = 1.0f;
    float pulse1MixLevel   = 1.0f;
    float pulse2MixLevel   = 1.0f;
    float triangleMixLevel = 1.0f;
    float noiseMixLevel    = 1.0f;
    float dpcmMixLevel     = 1.0f;
};

} // namespace cart
