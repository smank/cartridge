#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "dsp/DpcmSamples.h"
#include <cmath>

CartridgeProcessor::CartridgeProcessor()
    : AudioProcessor (BusesProperties()
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", cart::createParameterLayout())
{
    voiceManager.setApu (&apu);

    // Cache parameter pointers
    masterVolumeParam  = apvts.getRawParameterValue (cart::ParamIDs::MasterVolume);
    masterTuneParam    = apvts.getRawParameterValue (cart::ParamIDs::MasterTune);
    regionParam        = apvts.getRawParameterValue (cart::ParamIDs::Region);

    p1EnabledParam     = apvts.getRawParameterValue (cart::ParamIDs::P1Enabled);
    p1DutyParam        = apvts.getRawParameterValue (cart::ParamIDs::P1Duty);
    p1VolumeParam      = apvts.getRawParameterValue (cart::ParamIDs::P1Volume);
    p1ConstVolParam    = apvts.getRawParameterValue (cart::ParamIDs::P1ConstVol);
    p1EnvLoopParam     = apvts.getRawParameterValue (cart::ParamIDs::P1EnvLoop);
    p1SweepEnableParam = apvts.getRawParameterValue (cart::ParamIDs::P1SweepEnable);
    p1SweepPeriodParam = apvts.getRawParameterValue (cart::ParamIDs::P1SweepPeriod);
    p1SweepNegateParam = apvts.getRawParameterValue (cart::ParamIDs::P1SweepNegate);
    p1SweepShiftParam  = apvts.getRawParameterValue (cart::ParamIDs::P1SweepShift);
    p1MixParam         = apvts.getRawParameterValue (cart::ParamIDs::P1Mix);

    p2EnabledParam     = apvts.getRawParameterValue (cart::ParamIDs::P2Enabled);
    p2DutyParam        = apvts.getRawParameterValue (cart::ParamIDs::P2Duty);
    p2VolumeParam      = apvts.getRawParameterValue (cart::ParamIDs::P2Volume);
    p2ConstVolParam    = apvts.getRawParameterValue (cart::ParamIDs::P2ConstVol);
    p2EnvLoopParam     = apvts.getRawParameterValue (cart::ParamIDs::P2EnvLoop);
    p2SweepEnableParam = apvts.getRawParameterValue (cart::ParamIDs::P2SweepEnable);
    p2SweepPeriodParam = apvts.getRawParameterValue (cart::ParamIDs::P2SweepPeriod);
    p2SweepNegateParam = apvts.getRawParameterValue (cart::ParamIDs::P2SweepNegate);
    p2SweepShiftParam  = apvts.getRawParameterValue (cart::ParamIDs::P2SweepShift);
    p2MixParam         = apvts.getRawParameterValue (cart::ParamIDs::P2Mix);

    triEnabledParam       = apvts.getRawParameterValue (cart::ParamIDs::TriEnabled);
    triLinearReloadParam   = apvts.getRawParameterValue (cart::ParamIDs::TriLinearReload);
    triLinearControlParam  = apvts.getRawParameterValue (cart::ParamIDs::TriLinearControl);
    triMixParam           = apvts.getRawParameterValue (cart::ParamIDs::TriMix);

    noiseEnabledParam  = apvts.getRawParameterValue (cart::ParamIDs::NoiseEnabled);
    noiseModeParam     = apvts.getRawParameterValue (cart::ParamIDs::NoiseMode);
    noisePeriodParam   = apvts.getRawParameterValue (cart::ParamIDs::NoisePeriod);
    noiseVolumeParam   = apvts.getRawParameterValue (cart::ParamIDs::NoiseVolume);
    noiseConstVolParam = apvts.getRawParameterValue (cart::ParamIDs::NoiseConstVol);
    noiseEnvLoopParam  = apvts.getRawParameterValue (cart::ParamIDs::NoiseEnvLoop);
    noiseMixParam      = apvts.getRawParameterValue (cart::ParamIDs::NoiseMix);

    dpcmEnabledParam   = apvts.getRawParameterValue (cart::ParamIDs::DpcmEnabled);
    dpcmRateParam      = apvts.getRawParameterValue (cart::ParamIDs::DpcmRate);
    dpcmLoopParam      = apvts.getRawParameterValue (cart::ParamIDs::DpcmLoop);
    dpcmMixParam       = apvts.getRawParameterValue (cart::ParamIDs::DpcmMix);
    dpcmSampleParam    = apvts.getRawParameterValue (cart::ParamIDs::DpcmSample);

    vrc6EnabledParam   = apvts.getRawParameterValue (cart::ParamIDs::Vrc6Enabled);
    vrc6P1DutyParam    = apvts.getRawParameterValue (cart::ParamIDs::Vrc6P1Duty);
    vrc6P1VolumeParam  = apvts.getRawParameterValue (cart::ParamIDs::Vrc6P1Volume);
    vrc6P1MixParam     = apvts.getRawParameterValue (cart::ParamIDs::Vrc6P1Mix);
    vrc6P2DutyParam    = apvts.getRawParameterValue (cart::ParamIDs::Vrc6P2Duty);
    vrc6P2VolumeParam  = apvts.getRawParameterValue (cart::ParamIDs::Vrc6P2Volume);
    vrc6P2MixParam     = apvts.getRawParameterValue (cart::ParamIDs::Vrc6P2Mix);
    vrc6SawRateParam   = apvts.getRawParameterValue (cart::ParamIDs::Vrc6SawRate);
    vrc6SawMixParam    = apvts.getRawParameterValue (cart::ParamIDs::Vrc6SawMix);

    velocitySensParam   = apvts.getRawParameterValue (cart::ParamIDs::VelocitySens);
    pitchBendRangeParam = apvts.getRawParameterValue (cart::ParamIDs::PitchBendRange);
    midiModeParam       = apvts.getRawParameterValue (cart::ParamIDs::MidiMode);

    arpEnabledParam  = apvts.getRawParameterValue (cart::ParamIDs::ArpEnabled);
    arpPatternParam  = apvts.getRawParameterValue (cart::ParamIDs::ArpPattern);
    arpRateParam     = apvts.getRawParameterValue (cart::ParamIDs::ArpRate);
    arpOctavesParam  = apvts.getRawParameterValue (cart::ParamIDs::ArpOctaves);
    arpGateParam     = apvts.getRawParameterValue (cart::ParamIDs::ArpGate);

    portaEnabledParam = apvts.getRawParameterValue (cart::ParamIDs::PortaEnabled);
    portaTimeParam    = apvts.getRawParameterValue (cart::ParamIDs::PortaTime);

    lfoEnabledParam       = apvts.getRawParameterValue (cart::ParamIDs::LfoEnabled);
    lfoRateParam           = apvts.getRawParameterValue (cart::ParamIDs::LfoRate);
    lfoVibratoDepthParam   = apvts.getRawParameterValue (cart::ParamIDs::LfoVibratoDepth);
    lfoTremoloDepthParam   = apvts.getRawParameterValue (cart::ParamIDs::LfoTremoloDepth);

    // MIDI CC → parameter mappings
    voiceManager.onControlChange = [this] (int cc, float value01)
    {
        auto setNorm = [&] (const char* paramID, float v)
        {
            if (auto* p = apvts.getParameter (paramID))
                p->setValueNotifyingHost (v);
        };

        auto setNormRange = [&] (const char* paramID, float v)
        {
            if (auto* p = apvts.getParameter (paramID))
                p->setValueNotifyingHost (p->convertTo0to1 (v));
        };

        switch (cc)
        {
            case 1:  setNorm (cart::ParamIDs::LfoVibratoDepth, value01); break;
            case 11: setNorm (cart::ParamIDs::MasterVolume, value01); break;
            case 74: setNormRange (cart::ParamIDs::FltCutoff,
                         20.0f * std::pow (1000.0f, value01)); break;
            case 71: setNormRange (cart::ParamIDs::FltResonance,
                         0.1f + value01 * 4.9f); break;
            case 91: setNorm (cart::ParamIDs::RvMix, value01); break;
            case 93: setNorm (cart::ParamIDs::ChMix, value01); break;
            default: break;
        }
    };
}

CartridgeProcessor::~CartridgeProcessor() = default;

const juce::String CartridgeProcessor::getName() const { return JucePlugin_Name; }
bool CartridgeProcessor::acceptsMidi() const            { return true; }
bool CartridgeProcessor::producesMidi() const           { return false; }
bool CartridgeProcessor::isMidiEffect() const           { return false; }
double CartridgeProcessor::getTailLengthSeconds() const { return 5.0; }
int CartridgeProcessor::getNumPrograms()                { return presetManager.getNumPresets(); }
int CartridgeProcessor::getCurrentProgram()             { return currentPreset; }
void CartridgeProcessor::setCurrentProgram (int index)
{
    currentPreset = index;
    presetManager.applyPreset (index, apvts);
    pendingDspReset.store (true);    // Actual reset deferred to audio thread
}
const juce::String CartridgeProcessor::getProgramName (int index) { return presetManager.getPresetName (index); }
void CartridgeProcessor::changeProgramName (int, const juce::String&) {}

bool CartridgeProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Accept any input layout (we ignore audio input via buffer.clear()).
    // Classification as instrument comes from IS_SYNTH / Vst3Category metadata,
    // not from rejecting input buses — rejecting them breaks bus negotiation
    // in DAWs that have a stale plugin cache.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void CartridgeProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    apu.setSampleRate (sampleRate);
    apu.reset();
    effectsChain.prepare (sampleRate, samplesPerBlock);
    arpeggiator.setSampleRate (sampleRate);
    voiceManager.setSampleRateForPorta (sampleRate);
    lfo.setSampleRate (sampleRate);
    // DC blocker state intentionally preserved — resetting to 0 causes a settling
    // transient.  Member initializers handle the very first startup.
    cachedParams = CachedParams{};   // Force full re-apply after APU reset
    fadeInSamplesRemaining = 1024;   // Suppress startup transient (covers DC blocker settling)
}

void CartridgeProcessor::releaseResources()
{
    effectsChain.reset();
    apu.reset();
}

void CartridgeProcessor::updateDspFromParameters()
{
    // Helper: only update if value changed (exact bit-equality is intentional
    // since atomic parameter values are bit-identical when unchanged)
    auto changed = [] (float& cached, float current) {
        if (std::memcmp (&cached, &current, sizeof (float)) == 0) return false;
        cached = current;
        return true;
    };

    // Global
    float regionVal = *regionParam;
    bool ntsc = static_cast<int> (regionVal) == 0;
    bool regionChanged = changed (cachedParams.region, regionVal);

    if (regionChanged)
        apu.setRegion (ntsc);
    if (changed (cachedParams.masterVolume, *masterVolumeParam))
        apu.setMasterVolume (cachedParams.masterVolume);

    // Voice manager
    if (changed (cachedParams.masterTune, *masterTuneParam))
        voiceManager.setMasterTune (cachedParams.masterTune);
    if (changed (cachedParams.velocitySens, *velocitySensParam))
        voiceManager.setVelocitySensitivity (cachedParams.velocitySens);
    if (changed (cachedParams.pitchBendRange, *pitchBendRangeParam))
        voiceManager.setPitchBendRange (static_cast<int> (cachedParams.pitchBendRange));
    if (changed (cachedParams.midiMode, *midiModeParam))
        voiceManager.setMode (static_cast<cart::MidiMode> (static_cast<int> (cachedParams.midiMode)));

    // Pulse 1
    if (changed (cachedParams.p1Enabled, *p1EnabledParam))
        apu.pulse1().setEnabled (cachedParams.p1Enabled > 0.5f);
    if (changed (cachedParams.p1Duty, *p1DutyParam))
        apu.pulse1().setDuty (static_cast<int> (cachedParams.p1Duty));
    if (changed (cachedParams.p1Volume, *p1VolumeParam))
        apu.pulse1().envelope().setVolume (static_cast<uint8_t> (cachedParams.p1Volume));
    if (changed (cachedParams.p1ConstVol, *p1ConstVolParam))
        apu.pulse1().envelope().setConstantVolume (cachedParams.p1ConstVol > 0.5f);
    if (changed (cachedParams.p1EnvLoop, *p1EnvLoopParam))
        apu.pulse1().envelope().setLoop (cachedParams.p1EnvLoop > 0.5f);
    if (changed (cachedParams.p1SweepEnable, *p1SweepEnableParam))
        apu.pulse1().sweep().setEnabled (cachedParams.p1SweepEnable > 0.5f);
    if (changed (cachedParams.p1SweepPeriod, *p1SweepPeriodParam))
        apu.pulse1().sweep().setPeriod (static_cast<uint8_t> (cachedParams.p1SweepPeriod));
    if (changed (cachedParams.p1SweepNegate, *p1SweepNegateParam))
        apu.pulse1().sweep().setNegate (cachedParams.p1SweepNegate > 0.5f);
    if (changed (cachedParams.p1SweepShift, *p1SweepShiftParam))
        apu.pulse1().sweep().setShift (static_cast<uint8_t> (cachedParams.p1SweepShift));
    if (changed (cachedParams.p1Mix, *p1MixParam))
        apu.mixer().setPulse1Mix (cachedParams.p1Mix);

    // Pulse 2
    if (changed (cachedParams.p2Enabled, *p2EnabledParam))
        apu.pulse2().setEnabled (cachedParams.p2Enabled > 0.5f);
    if (changed (cachedParams.p2Duty, *p2DutyParam))
        apu.pulse2().setDuty (static_cast<int> (cachedParams.p2Duty));
    if (changed (cachedParams.p2Volume, *p2VolumeParam))
        apu.pulse2().envelope().setVolume (static_cast<uint8_t> (cachedParams.p2Volume));
    if (changed (cachedParams.p2ConstVol, *p2ConstVolParam))
        apu.pulse2().envelope().setConstantVolume (cachedParams.p2ConstVol > 0.5f);
    if (changed (cachedParams.p2EnvLoop, *p2EnvLoopParam))
        apu.pulse2().envelope().setLoop (cachedParams.p2EnvLoop > 0.5f);
    if (changed (cachedParams.p2SweepEnable, *p2SweepEnableParam))
        apu.pulse2().sweep().setEnabled (cachedParams.p2SweepEnable > 0.5f);
    if (changed (cachedParams.p2SweepPeriod, *p2SweepPeriodParam))
        apu.pulse2().sweep().setPeriod (static_cast<uint8_t> (cachedParams.p2SweepPeriod));
    if (changed (cachedParams.p2SweepNegate, *p2SweepNegateParam))
        apu.pulse2().sweep().setNegate (cachedParams.p2SweepNegate > 0.5f);
    if (changed (cachedParams.p2SweepShift, *p2SweepShiftParam))
        apu.pulse2().sweep().setShift (static_cast<uint8_t> (cachedParams.p2SweepShift));
    if (changed (cachedParams.p2Mix, *p2MixParam))
        apu.mixer().setPulse2Mix (cachedParams.p2Mix);

    // Triangle
    if (changed (cachedParams.triEnabled, *triEnabledParam))
        apu.triangle().setEnabled (cachedParams.triEnabled > 0.5f);
    if (changed (cachedParams.triLinearReload, *triLinearReloadParam))
        apu.triangle().linearCounter().setReloadValue (static_cast<uint8_t> (cachedParams.triLinearReload));
    if (changed (cachedParams.triLinearControl, *triLinearControlParam))
        apu.triangle().linearCounter().setControl (cachedParams.triLinearControl > 0.5f);
    if (changed (cachedParams.triMix, *triMixParam))
        apu.mixer().setTriangleMix (cachedParams.triMix);

    // Noise
    if (changed (cachedParams.noiseEnabled, *noiseEnabledParam))
        apu.noise().setEnabled (cachedParams.noiseEnabled > 0.5f);
    if (changed (cachedParams.noiseMode, *noiseModeParam))
        apu.noise().setShortMode (static_cast<int> (cachedParams.noiseMode) == 1);

    float noisePeriodVal = *noisePeriodParam;
    if (changed (cachedParams.noisePeriod, noisePeriodVal) || regionChanged)
        apu.noise().setPeriodIndex (static_cast<int> (cachedParams.noisePeriod), ntsc);

    if (changed (cachedParams.noiseVolume, *noiseVolumeParam))
        apu.noise().envelope().setVolume (static_cast<uint8_t> (cachedParams.noiseVolume));
    if (changed (cachedParams.noiseConstVol, *noiseConstVolParam))
        apu.noise().envelope().setConstantVolume (cachedParams.noiseConstVol > 0.5f);
    if (changed (cachedParams.noiseEnvLoop, *noiseEnvLoopParam))
        apu.noise().envelope().setLoop (cachedParams.noiseEnvLoop > 0.5f);
    if (changed (cachedParams.noiseMix, *noiseMixParam))
        apu.mixer().setNoiseMix (cachedParams.noiseMix);

    // DPCM
    if (changed (cachedParams.dpcmEnabled, *dpcmEnabledParam))
        apu.dpcm().setEnabled (cachedParams.dpcmEnabled > 0.5f);

    float dpcmRateVal = *dpcmRateParam;
    if (changed (cachedParams.dpcmRate, dpcmRateVal) || regionChanged)
        apu.dpcm().setRateIndex (static_cast<int> (cachedParams.dpcmRate), ntsc);

    if (changed (cachedParams.dpcmLoop, *dpcmLoopParam))
        apu.dpcm().setLoop (cachedParams.dpcmLoop > 0.5f);
    if (changed (cachedParams.dpcmMix, *dpcmMixParam))
        apu.mixer().setDpcmMix (cachedParams.dpcmMix);

    // Load selected DPCM sample
    if (changed (cachedParams.dpcmSample, *dpcmSampleParam))
        apu.dpcm().loadSample (cart::getDpcmSample (static_cast<int> (cachedParams.dpcmSample)));

    // VRC6 Expansion
    float vrc6Val = *vrc6EnabledParam;
    bool vrc6On = vrc6Val > 0.5f;
    if (changed (cachedParams.vrc6Enabled, vrc6Val))
        apu.setVrc6Enabled (vrc6On);

    if (vrc6On)
    {
        if (changed (cachedParams.vrc6P1Duty, *vrc6P1DutyParam))
            apu.vrc6Pulse1().setDuty (static_cast<int> (cachedParams.vrc6P1Duty));
        if (changed (cachedParams.vrc6P1Volume, *vrc6P1VolumeParam))
            apu.vrc6Pulse1().setVolume (static_cast<uint8_t> (cachedParams.vrc6P1Volume));
        if (changed (cachedParams.vrc6P1Mix, *vrc6P1MixParam))
            apu.setVrc6Pulse1Mix (cachedParams.vrc6P1Mix);
        if (changed (cachedParams.vrc6P2Duty, *vrc6P2DutyParam))
            apu.vrc6Pulse2().setDuty (static_cast<int> (cachedParams.vrc6P2Duty));
        if (changed (cachedParams.vrc6P2Volume, *vrc6P2VolumeParam))
            apu.vrc6Pulse2().setVolume (static_cast<uint8_t> (cachedParams.vrc6P2Volume));
        if (changed (cachedParams.vrc6P2Mix, *vrc6P2MixParam))
            apu.setVrc6Pulse2Mix (cachedParams.vrc6P2Mix);
        if (changed (cachedParams.vrc6SawRate, *vrc6SawRateParam))
            apu.vrc6Saw().setRate (static_cast<uint8_t> (cachedParams.vrc6SawRate));
        if (changed (cachedParams.vrc6SawMix, *vrc6SawMixParam))
            apu.setVrc6SawMix (cachedParams.vrc6SawMix);
    }

    voiceManager.setVrc6Available (vrc6On);
    if (regionChanged)
        voiceManager.setRegion (ntsc);

    // Arpeggiator
    arpeggiator.setEnabled (*arpEnabledParam > 0.5f);
    arpeggiator.setPattern (static_cast<cart::ArpPattern> (static_cast<int> (*arpPatternParam)));
    arpeggiator.setRateHz (*arpRateParam);
    arpeggiator.setOctaves (static_cast<int> (*arpOctavesParam));
    arpeggiator.setGateLength (*arpGateParam);

    // Portamento
    voiceManager.setPortamentoEnabled (*portaEnabledParam > 0.5f);
    voiceManager.setPortamentoTime (*portaTimeParam);

    // LFO
    lfo.setEnabled (*lfoEnabledParam > 0.5f);
    lfo.setRate (*lfoRateParam);
    lfo.setVibratoDepth (*lfoVibratoDepthParam);
    lfo.setTremoloDepth (*lfoTremoloDepthParam);

    // Pass channel enabled state for Auto mode routing
    voiceManager.setChannelEnabled (0, cachedParams.p1Enabled > 0.5f);
    voiceManager.setChannelEnabled (1, cachedParams.p2Enabled > 0.5f);
    voiceManager.setChannelEnabled (2, cachedParams.triEnabled > 0.5f);
    voiceManager.setChannelEnabled (3, cachedParams.noiseEnabled > 0.5f);
    voiceManager.setChannelEnabled (4, cachedParams.dpcmEnabled > 0.5f);
    // VRC6 channels: only route notes to channels with mix > 0
    voiceManager.setChannelEnabled (5, vrc6On && cachedParams.vrc6P1Mix > 0.0f);
    voiceManager.setChannelEnabled (6, vrc6On && cachedParams.vrc6P2Mix > 0.0f);
    voiceManager.setChannelEnabled (7, vrc6On && cachedParams.vrc6SawMix > 0.0f);
}

void CartridgeProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                      juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    // Handle deferred DSP reset (from preset switch / state restore on UI thread)
    if (pendingDspReset.exchange (false))
    {
        effectsChain.reset();
        apu.reset();
        voiceManager.handleAllNotesOff();
        arpeggiator.reset();
        lastArpNote = -1;
        lfo.reset();
        cachedParams = CachedParams{};
        fadeInSamplesRemaining = 1024;   // ~23 ms — covers DC blocker settling
    }

    // Update DSP parameters from APVTS
    updateDspFromParameters();
    effectsChain.updateFromParams (apvts);

    // Inject MIDI events from on-screen keyboard
    keyboardState.processNextMidiBuffer (midiMessages, 0, buffer.getNumSamples(), true);

    // Hold/latch mode: suppress note-offs, release all when toggled off
    bool hold = holdMode.load();
    if (wasHolding && !hold)
    {
        voiceManager.handleAllNotesOff();
        arpeggiator.reset();
        lastArpNote = -1;
    }
    wasHolding = hold;

    // Process MIDI and audio sample-by-sample
    auto midiIterator = midiMessages.cbegin();
    auto midiEnd      = midiMessages.cend();

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        // Process any MIDI events at this sample position
        while (midiIterator != midiEnd)
        {
            const auto metadata = *midiIterator;
            if (metadata.samplePosition > sample)
                break;

            auto msg = metadata.getMessage();

            // In hold mode, suppress note-off messages so notes ring
            if (hold && msg.isNoteOff())
            {
                ++midiIterator;
                continue;
            }

            if (arpeggiator.isEnabled())
            {
                if (msg.isNoteOn())
                    arpeggiator.noteOn (msg.getNoteNumber(), msg.getVelocity());
                else if (msg.isNoteOff())
                    arpeggiator.noteOff (msg.getNoteNumber());
                else
                    voiceManager.processMidiMessage (msg); // pitch bend etc pass through
            }
            else
            {
                voiceManager.processMidiMessage (msg);
            }

            ++midiIterator;
        }

        // Arpeggiator tick
        if (arpeggiator.isEnabled())
        {
            int arpNote = -1;
            auto arpEvent = arpeggiator.process (arpNote);

            if (arpEvent == cart::ArpEvent::NoteOn)
            {
                if (lastArpNote >= 0)
                    voiceManager.processMidiMessage (juce::MidiMessage::noteOff (1, lastArpNote, (juce::uint8) 0));

                voiceManager.processMidiMessage (juce::MidiMessage::noteOn (1, arpNote, arpeggiator.getLastVelocity()));
                lastArpNote = arpNote;
            }
            else if (arpEvent == cart::ArpEvent::GateOff)
            {
                if (lastArpNote >= 0)
                {
                    voiceManager.processMidiMessage (juce::MidiMessage::noteOff (1, lastArpNote, (juce::uint8) 0));
                    lastArpNote = -1;
                }
            }
        }
        else if (lastArpNote >= 0)
        {
            // Arp was just disabled — clean up last note
            voiceManager.processMidiMessage (juce::MidiMessage::noteOff (1, lastArpNote, (juce::uint8) 0));
            lastArpNote = -1;
        }

        // Portamento glide tick
        voiceManager.tickPortamento();

        // LFO modulation
        lfo.tick();
        float pitchMult = lfo.getPitchMultiplier();
        if (pitchMult != 1.0f)
            voiceManager.applyPitchMultiplier (pitchMult);

        // Generate one sample from the APU — write to ch0 only
        float out = apu.process() * lfo.getVolumeMultiplier();

        // DC blocking filter: removes NES DAC offset (~0.5–0.9 at silence)
        // y[n] = x[n] - x[n-1] + R * y[n-1], R=0.995 ≈ 16 Hz cutoff at 44.1kHz
        float dcOut = out - dcBlockX + 0.995f * dcBlockY;
        dcBlockX = out;
        dcBlockY = dcOut;

        buffer.setSample (0, sample, dcOut);
    }

    // Fade-in ramp BEFORE effects — prevents transients from being stored in
    // reverb/delay buffers where they'd leak out after the ramp completes.
    // 1024 total: first 512 hard-muted (covers DC blocker settling), last 512 ramped.
    if (fadeInSamplesRemaining > 0)
    {
        int numSamples = buffer.getNumSamples();
        static constexpr int rampLength = 512;

        float* data = buffer.getWritePointer (0);
        for (int s = 0; s < numSamples && fadeInSamplesRemaining > 0; ++s)
        {
            float gain = 0.0f;
            if (fadeInSamplesRemaining <= rampLength)
                gain = static_cast<float> (rampLength - fadeInSamplesRemaining) / static_cast<float> (rampLength);

            data[s] *= gain;
            --fadeInSamplesRemaining;
        }
    }

    // Block-based effects (handles mono→stereo internally)
    effectsChain.process (buffer);

    // Soft limiter — prevents clipping from stacked effects
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        float* samples = buffer.getWritePointer (ch);
        for (int s = 0; s < buffer.getNumSamples(); ++s)
            samples[s] = std::tanh (samples[s]);
    }
}

juce::AudioProcessorEditor* CartridgeProcessor::createEditor()
{
    return new CartridgeEditor (*this);
}

bool CartridgeProcessor::hasEditor() const { return true; }

void CartridgeProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void CartridgeProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
    {
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
        pendingDspReset.store (true);        // Actual reset deferred to audio thread
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CartridgeProcessor();
}
