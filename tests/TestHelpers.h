#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters.h"
#include <vector>
#include <cmath>
#include <numeric>

namespace test {

/// Process N samples from any channel that has a process() method returning float.
template <typename Channel>
inline std::vector<float> processN (Channel& ch, int n)
{
    std::vector<float> out;
    out.reserve (static_cast<size_t> (n));
    for (int i = 0; i < n; ++i)
        out.push_back (ch.process());
    return out;
}

/// Returns true if any sample has absolute value > threshold.
inline bool hasEnergy (const std::vector<float>& samples, float threshold = 1e-6f)
{
    for (auto s : samples)
        if (std::abs (s) > threshold)
            return true;
    return false;
}

/// Root mean square of a sample buffer.
inline float rms (const std::vector<float>& samples)
{
    if (samples.empty()) return 0.0f;
    float sumSq = 0.0f;
    for (auto s : samples)
        sumSq += s * s;
    return std::sqrt (sumSq / static_cast<float> (samples.size()));
}

/// Count zero crossings in a sample buffer (for rough frequency estimation).
inline int zeroCrossings (const std::vector<float>& samples)
{
    int count = 0;
    for (size_t i = 1; i < samples.size(); ++i)
    {
        if ((samples[i - 1] >= 0.0f && samples[i] < 0.0f) ||
            (samples[i - 1] < 0.0f && samples[i] >= 0.0f))
            ++count;
    }
    return count;
}

/// Minimal AudioProcessor stub for creating an APVTS in tests.
class StubProcessor : public juce::AudioProcessor
{
public:
    StubProcessor() : juce::AudioProcessor (BusesProperties()
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)) {}

    const juce::String getName() const override { return "StubProcessor"; }
    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
    double getTailLengthSeconds() const override { return 0.0; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}
};

/// Create an APVTS using the real Cartridge parameter layout.
inline std::unique_ptr<juce::AudioProcessorValueTreeState> makeTestApvts (juce::AudioProcessor& proc)
{
    return std::make_unique<juce::AudioProcessorValueTreeState> (
        proc, nullptr, "Parameters", cart::createParameterLayout());
}

} // namespace test
