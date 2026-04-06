#include "EffectsChain.h"
#include "../../Parameters.h"

namespace cart {

void EffectsChain::prepare (double sampleRate, int maxBlockSize)
{
    bitCrush.prepare (sampleRate, maxBlockSize);
    filter.prepare (sampleRate, maxBlockSize);
    chorus.prepare (sampleRate, maxBlockSize);
    delay.prepare (sampleRate, maxBlockSize);
    reverb.prepare (sampleRate, maxBlockSize);
}

void EffectsChain::reset()
{
    bitCrush.reset();
    filter.reset();
    chorus.reset();
    delay.reset();
    reverb.reset();
}

void EffectsChain::updateFromParams (juce::AudioProcessorValueTreeState& apvts)
{
    auto get = [&] (const char* id) { return apvts.getRawParameterValue (id)->load(); };

    bitCrush.setEnabled    (get (ParamIDs::BcEnabled) > 0.5f);
    bitCrush.setBitDepth   (get (ParamIDs::BcBitDepth));
    bitCrush.setRateReduce (get (ParamIDs::BcRateReduce));
    bitCrush.setMix        (get (ParamIDs::BcMix));

    filter.setEnabled   (get (ParamIDs::FltEnabled) > 0.5f);
    filter.setType      (static_cast<int> (get (ParamIDs::FltType)));
    filter.setCutoff    (get (ParamIDs::FltCutoff));
    filter.setResonance (get (ParamIDs::FltResonance));

    chorus.setEnabled (get (ParamIDs::ChEnabled) > 0.5f);
    chorus.setRate    (get (ParamIDs::ChRate));
    chorus.setDepth   (get (ParamIDs::ChDepth));
    chorus.setMix     (get (ParamIDs::ChMix));

    delay.setEnabled  (get (ParamIDs::DlEnabled) > 0.5f);
    delay.setTime     (get (ParamIDs::DlTime));
    delay.setFeedback (get (ParamIDs::DlFeedback));
    delay.setMix      (get (ParamIDs::DlMix));

    reverb.setEnabled (get (ParamIDs::RvEnabled) > 0.5f);
    reverb.setSize    (get (ParamIDs::RvSize));
    reverb.setDamping (get (ParamIDs::RvDamping));
    reverb.setWidth   (get (ParamIDs::RvWidth));
    reverb.setMix     (get (ParamIDs::RvMix));
}

void EffectsChain::process (juce::AudioBuffer<float>& buffer)
{
    // 1. Mono effects on ch0 (and ch1 if stereo input)
    bitCrush.process (buffer);
    filter.process (buffer);

    // 2. Copy ch0 → ch1 (mono→stereo) — skip when input is already stereo
    if (!stereoInput && buffer.getNumChannels() >= 2)
    {
        buffer.copyFrom (1, 0, buffer, 0, 0, buffer.getNumSamples());
    }

    // 3. Stereo effects
    chorus.process (buffer);
    delay.process (buffer);
    reverb.process (buffer);
}

} // namespace cart
