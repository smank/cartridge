#pragma once

#include <juce_dsp/juce_dsp.h>

namespace cart {

class Filter
{
public:
    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    void setEnabled (bool on)          { enabled = on; }
    void setType (int type);           // 0=LP, 1=BP, 2=HP
    void setCutoff (float hz)          { cutoff = hz; updateCoeffs = true; }
    void setResonance (float q)        { resonance = q; updateCoeffs = true; }

    void process (juce::AudioBuffer<float>& buffer);

private:
    bool  enabled   = false;
    float cutoff    = 1000.0f;
    float resonance = 0.707f;
    bool  updateCoeffs = true;

    juce::dsp::StateVariableTPTFilter<float> filter;
};

} // namespace cart
