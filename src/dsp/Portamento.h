#pragma once
#include <cmath>

namespace cart {

class Portamento
{
public:
    void setSampleRate (double sr) { sampleRate = sr; updateCoeff(); }
    void setEnabled (bool en) { enabled = en; }
    void setTime (float seconds) { glideTime = seconds; updateCoeff(); }

    /// Set new target frequency. Returns current (gliding) frequency.
    void setTarget (float freq)
    {
        if (!enabled || currentFreq <= 0.0f)
        {
            currentFreq = freq;
            targetFreq = freq;
            return;
        }
        targetFreq = freq;
    }

    /// Advance one sample, returns current frequency
    float process()
    {
        if (!enabled || std::abs (targetFreq - currentFreq) < 0.001f)
            return currentFreq;

        currentFreq += (targetFreq - currentFreq) * coeff;

        // Snap when close enough
        if (std::abs (targetFreq - currentFreq) < 0.01f)
            currentFreq = targetFreq;

        return currentFreq;
    }

    float getCurrentFreq() const { return currentFreq; }
    bool isGliding() const { return enabled && std::abs (targetFreq - currentFreq) >= 0.001f; }

    void reset()
    {
        currentFreq = 0.0f;
        targetFreq = 0.0f;
    }

private:
    void updateCoeff()
    {
        if (sampleRate > 0.0 && glideTime > 0.0f)
            coeff = 1.0f - std::exp (-1.0f / (static_cast<float> (sampleRate) * glideTime));
        else
            coeff = 1.0f;
    }

    bool enabled = false;
    float glideTime = 0.1f;
    double sampleRate = 44100.0;
    float coeff = 1.0f;
    float currentFreq = 0.0f;
    float targetFreq = 0.0f;
};

} // namespace cart
