#pragma once
#include <cmath>

namespace cart {

class Lfo
{
public:
    void setSampleRate (double sr) { sampleRate = sr; updateIncrement(); }
    void setRate (float hz) { rate = hz; updateIncrement(); }
    void setVibratoDepth (float d) { vibratoDepth = d; }  // 0-1, maps to 0-2 semitones
    void setTremoloDepth (float d) { tremoloDepth = d; }  // 0-1, maps to full volume range
    void setEnabled (bool en) { enabled = en; if (!en) phase = 0.0; }

    void reset() { phase = 0.0; }

    /// Get pitch multiplier for current sample (1.0 = no modulation)
    float getPitchMultiplier() const
    {
        if (!enabled || vibratoDepth == 0.0f) return 1.0f;
        float semitones = currentValue * vibratoDepth * 2.0f; // max +/-2 semitones
        return std::pow (2.0f, semitones / 12.0f);
    }

    /// Get volume multiplier for current sample (1.0 = no modulation)
    float getVolumeMultiplier() const
    {
        if (!enabled || tremoloDepth == 0.0f) return 1.0f;
        // Map sine [-1,1] to [1-depth, 1]
        return 1.0f - tremoloDepth * (1.0f - (currentValue + 1.0f) * 0.5f);
    }

    /// Advance one sample
    void tick()
    {
        if (!enabled) return;
        phase += phaseInc;
        if (phase >= 1.0) phase -= 1.0;
        currentValue = std::sin (static_cast<float> (phase * 2.0 * 3.14159265358979323846));
    }

private:
    void updateIncrement()
    {
        if (sampleRate > 0.0)
            phaseInc = static_cast<double> (rate) / sampleRate;
    }

    bool enabled = false;
    float rate = 5.0f;
    float vibratoDepth = 0.0f;
    float tremoloDepth = 0.0f;
    double sampleRate = 44100.0;
    double phase = 0.0;
    double phaseInc = 0.0;
    float currentValue = 0.0f;
};

} // namespace cart
