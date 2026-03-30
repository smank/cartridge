#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters.h"
#include "PresetManager.h"
#include "dsp/Apu.h"
#include "dsp/effects/EffectsChain.h"
#include "midi/MidiVoiceManager.h"
#include "midi/Arpeggiator.h"
#include "dsp/Lfo.h"

class CartridgeProcessor : public juce::AudioProcessor
{
public:
    CartridgeProcessor();
    ~CartridgeProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getApvts() { return apvts; }
    juce::MidiKeyboardState& getKeyboardState()   { return keyboardState; }
    cart::PresetManager& getPresetManager()         { return presetManager; }

    void setHoldMode (bool on) { holdMode.store (on); }
    bool getHoldMode() const   { return holdMode.load(); }

private:
    void updateDspFromParameters();

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState            keyboardState;
    cart::PresetManager                 presetManager;
    int                                currentPreset = 0;

    cart::Apu           apu;
    cart::MidiVoiceManager voiceManager;
    cart::EffectsChain     effectsChain;
    cart::Arpeggiator      arpeggiator;
    cart::Lfo              lfo;
    int                    lastArpNote = -1;
    int                    fadeInSamplesRemaining = 0;
    std::atomic<bool>      holdMode { false };
    bool                   wasHolding = false;
    std::atomic<bool>      pendingDspReset { false };  // Defers reset to audio thread

    // DC blocking filter state (removes NES DAC offset)
    float dcBlockX = 0.0f;   // previous input
    float dcBlockY = 0.0f;   // previous output

    // Cached parameter pointers for real-time access
    std::atomic<float>* masterVolumeParam  = nullptr;
    std::atomic<float>* masterTuneParam    = nullptr;
    std::atomic<float>* regionParam        = nullptr;

    std::atomic<float>* p1EnabledParam     = nullptr;
    std::atomic<float>* p1DutyParam        = nullptr;
    std::atomic<float>* p1VolumeParam      = nullptr;
    std::atomic<float>* p1ConstVolParam    = nullptr;
    std::atomic<float>* p1EnvLoopParam     = nullptr;
    std::atomic<float>* p1SweepEnableParam = nullptr;
    std::atomic<float>* p1SweepPeriodParam = nullptr;
    std::atomic<float>* p1SweepNegateParam = nullptr;
    std::atomic<float>* p1SweepShiftParam  = nullptr;
    std::atomic<float>* p1MixParam         = nullptr;

    std::atomic<float>* p2EnabledParam     = nullptr;
    std::atomic<float>* p2DutyParam        = nullptr;
    std::atomic<float>* p2VolumeParam      = nullptr;
    std::atomic<float>* p2ConstVolParam    = nullptr;
    std::atomic<float>* p2EnvLoopParam     = nullptr;
    std::atomic<float>* p2SweepEnableParam = nullptr;
    std::atomic<float>* p2SweepPeriodParam = nullptr;
    std::atomic<float>* p2SweepNegateParam = nullptr;
    std::atomic<float>* p2SweepShiftParam  = nullptr;
    std::atomic<float>* p2MixParam         = nullptr;

    std::atomic<float>* triEnabledParam       = nullptr;
    std::atomic<float>* triLinearReloadParam   = nullptr;
    std::atomic<float>* triLinearControlParam  = nullptr;
    std::atomic<float>* triMixParam           = nullptr;

    std::atomic<float>* noiseEnabledParam  = nullptr;
    std::atomic<float>* noiseModeParam     = nullptr;
    std::atomic<float>* noisePeriodParam   = nullptr;
    std::atomic<float>* noiseVolumeParam   = nullptr;
    std::atomic<float>* noiseConstVolParam = nullptr;
    std::atomic<float>* noiseEnvLoopParam  = nullptr;
    std::atomic<float>* noiseMixParam      = nullptr;

    std::atomic<float>* dpcmEnabledParam   = nullptr;
    std::atomic<float>* dpcmRateParam      = nullptr;
    std::atomic<float>* dpcmLoopParam      = nullptr;
    std::atomic<float>* dpcmMixParam       = nullptr;
    std::atomic<float>* dpcmSampleParam    = nullptr;

    std::atomic<float>* vrc6EnabledParam   = nullptr;
    std::atomic<float>* vrc6P1DutyParam    = nullptr;
    std::atomic<float>* vrc6P1VolumeParam  = nullptr;
    std::atomic<float>* vrc6P1MixParam     = nullptr;
    std::atomic<float>* vrc6P2DutyParam    = nullptr;
    std::atomic<float>* vrc6P2VolumeParam  = nullptr;
    std::atomic<float>* vrc6P2MixParam     = nullptr;
    std::atomic<float>* vrc6SawRateParam   = nullptr;
    std::atomic<float>* vrc6SawMixParam    = nullptr;

    std::atomic<float>* velocitySensParam  = nullptr;
    std::atomic<float>* pitchBendRangeParam = nullptr;
    std::atomic<float>* midiModeParam      = nullptr;

    std::atomic<float>* arpEnabledParam  = nullptr;
    std::atomic<float>* arpPatternParam  = nullptr;
    std::atomic<float>* arpRateParam     = nullptr;
    std::atomic<float>* arpOctavesParam  = nullptr;
    std::atomic<float>* arpGateParam     = nullptr;

    std::atomic<float>* portaEnabledParam = nullptr;
    std::atomic<float>* portaTimeParam    = nullptr;

    std::atomic<float>* lfoEnabledParam       = nullptr;
    std::atomic<float>* lfoRateParam           = nullptr;
    std::atomic<float>* lfoVibratoDepthParam   = nullptr;
    std::atomic<float>* lfoTremoloDepthParam   = nullptr;

    // Cached previous values for change detection
    struct CachedParams
    {
        float masterVolume = -1.0f;
        float masterTune = -1.0f;
        float region = -1.0f;
        float midiMode = -1.0f;
        float velocitySens = -1.0f;
        float pitchBendRange = -1.0f;

        float p1Enabled = -1.0f, p1Duty = -1.0f, p1Volume = -1.0f;
        float p1ConstVol = -1.0f, p1EnvLoop = -1.0f;
        float p1SweepEnable = -1.0f, p1SweepPeriod = -1.0f;
        float p1SweepNegate = -1.0f, p1SweepShift = -1.0f, p1Mix = -1.0f;

        float p2Enabled = -1.0f, p2Duty = -1.0f, p2Volume = -1.0f;
        float p2ConstVol = -1.0f, p2EnvLoop = -1.0f;
        float p2SweepEnable = -1.0f, p2SweepPeriod = -1.0f;
        float p2SweepNegate = -1.0f, p2SweepShift = -1.0f, p2Mix = -1.0f;

        float triEnabled = -1.0f, triLinearReload = -1.0f;
        float triLinearControl = -1.0f, triMix = -1.0f;

        float noiseEnabled = -1.0f, noiseMode = -1.0f, noisePeriod = -1.0f;
        float noiseVolume = -1.0f, noiseConstVol = -1.0f;
        float noiseEnvLoop = -1.0f, noiseMix = -1.0f;

        float dpcmEnabled = -1.0f, dpcmRate = -1.0f;
        float dpcmLoop = -1.0f, dpcmMix = -1.0f, dpcmSample = -1.0f;

        float vrc6Enabled = -1.0f;
        float vrc6P1Duty = -1.0f, vrc6P1Volume = -1.0f, vrc6P1Mix = -1.0f;
        float vrc6P2Duty = -1.0f, vrc6P2Volume = -1.0f, vrc6P2Mix = -1.0f;
        float vrc6SawRate = -1.0f, vrc6SawMix = -1.0f;
    } cachedParams;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CartridgeProcessor)
};
