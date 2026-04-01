#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace cart {

// Parameter IDs
namespace ParamIDs {
    // Global
    inline constexpr const char* MasterVolume     = "masterVolume";
    inline constexpr const char* MasterTune       = "masterTune";
    inline constexpr const char* Region           = "region";

    // Pulse 1
    inline constexpr const char* P1Enabled        = "p1Enabled";
    inline constexpr const char* P1Duty           = "p1Duty";
    inline constexpr const char* P1Volume         = "p1Volume";
    inline constexpr const char* P1ConstVol       = "p1ConstVol";
    inline constexpr const char* P1EnvLoop        = "p1EnvLoop";
    inline constexpr const char* P1SweepEnable    = "p1SweepEnable";
    inline constexpr const char* P1SweepPeriod    = "p1SweepPeriod";
    inline constexpr const char* P1SweepNegate    = "p1SweepNegate";
    inline constexpr const char* P1SweepShift     = "p1SweepShift";
    inline constexpr const char* P1Mix            = "p1Mix";

    // Pulse 2
    inline constexpr const char* P2Enabled        = "p2Enabled";
    inline constexpr const char* P2Duty           = "p2Duty";
    inline constexpr const char* P2Volume         = "p2Volume";
    inline constexpr const char* P2ConstVol       = "p2ConstVol";
    inline constexpr const char* P2EnvLoop        = "p2EnvLoop";
    inline constexpr const char* P2SweepEnable    = "p2SweepEnable";
    inline constexpr const char* P2SweepPeriod    = "p2SweepPeriod";
    inline constexpr const char* P2SweepNegate    = "p2SweepNegate";
    inline constexpr const char* P2SweepShift     = "p2SweepShift";
    inline constexpr const char* P2Mix            = "p2Mix";

    // Triangle
    inline constexpr const char* TriEnabled       = "triEnabled";
    inline constexpr const char* TriLinearReload   = "triLinearReload";
    inline constexpr const char* TriLinearControl  = "triLinearControl";
    inline constexpr const char* TriMix           = "triMix";

    // Noise
    inline constexpr const char* NoiseEnabled     = "noiseEnabled";
    inline constexpr const char* NoiseMode        = "noiseMode";
    inline constexpr const char* NoisePeriod      = "noisePeriod";
    inline constexpr const char* NoiseVolume      = "noiseVolume";
    inline constexpr const char* NoiseConstVol    = "noiseConstVol";
    inline constexpr const char* NoiseEnvLoop     = "noiseEnvLoop";
    inline constexpr const char* NoiseMix         = "noiseMix";

    // DPCM
    inline constexpr const char* DpcmEnabled      = "dpcmEnabled";
    inline constexpr const char* DpcmRate         = "dpcmRate";
    inline constexpr const char* DpcmLoop         = "dpcmLoop";
    inline constexpr const char* DpcmMix          = "dpcmMix";
    inline constexpr const char* DpcmSample       = "dpcmSample";

    // VRC6 Expansion
    inline constexpr const char* Vrc6Enabled      = "vrc6Enabled";
    inline constexpr const char* Vrc6P1Duty       = "vrc6P1Duty";
    inline constexpr const char* Vrc6P1Volume     = "vrc6P1Volume";
    inline constexpr const char* Vrc6P1Mix        = "vrc6P1Mix";
    inline constexpr const char* Vrc6P2Duty       = "vrc6P2Duty";
    inline constexpr const char* Vrc6P2Volume     = "vrc6P2Volume";
    inline constexpr const char* Vrc6P2Mix        = "vrc6P2Mix";
    inline constexpr const char* Vrc6SawRate      = "vrc6SawRate";
    inline constexpr const char* Vrc6SawMix       = "vrc6SawMix";

    // Per-channel transpose (melodic channels only)
    inline constexpr const char* P1Transpose      = "p1Transpose";
    inline constexpr const char* P2Transpose      = "p2Transpose";
    inline constexpr const char* TriTranspose     = "triTranspose";
    inline constexpr const char* Vrc6P1Transpose  = "vrc6P1Transpose";
    inline constexpr const char* Vrc6P2Transpose  = "vrc6P2Transpose";
    inline constexpr const char* Vrc6SawTranspose = "vrc6SawTranspose";

    // Modern enhancements
    inline constexpr const char* VelocitySens     = "velocitySens";
    inline constexpr const char* PitchBendRange   = "pitchBendRange";
    inline constexpr const char* MidiMode         = "midiMode";

    // Engine Mode
    inline constexpr const char* EngineMode       = "engineMode";

    // Modern Engine
    inline constexpr const char* ModWaveform      = "modWaveform";
    inline constexpr const char* ModVoices        = "modVoices";
    inline constexpr const char* ModAttack        = "modAttack";
    inline constexpr const char* ModDecay         = "modDecay";
    inline constexpr const char* ModSustain       = "modSustain";
    inline constexpr const char* ModRelease       = "modRelease";
    inline constexpr const char* ModUnison        = "modUnison";
    inline constexpr const char* ModDetune        = "modDetune";
    inline constexpr const char* ModPortaEnabled  = "modPortaEnabled";
    inline constexpr const char* ModPortaTime     = "modPortaTime";
    inline constexpr const char* ModVelToFilter   = "modVelToFilter";
    inline constexpr const char* ModVolume        = "modVolume";
    inline constexpr const char* ModOscAEnabled   = "modOscAEnabled";
    inline constexpr const char* ModOscBEnabled   = "modOscBEnabled";
    inline constexpr const char* ModWaveformB     = "modWaveformB";
    inline constexpr const char* ModOscBLevel     = "modOscBLevel";
    inline constexpr const char* ModOscBDetune    = "modOscBDetune";

    // Portamento
    inline constexpr const char* PortaEnabled     = "portaEnabled";
    inline constexpr const char* PortaTime        = "portaTime";

    // LFO
    inline constexpr const char* LfoEnabled       = "lfoEnabled";
    inline constexpr const char* LfoRate           = "lfoRate";
    inline constexpr const char* LfoVibratoDepth   = "lfoVibratoDepth";
    inline constexpr const char* LfoTremoloDepth   = "lfoTremoloDepth";

    // Arpeggiator
    inline constexpr const char* ArpEnabled       = "arpEnabled";
    inline constexpr const char* ArpPattern       = "arpPattern";
    inline constexpr const char* ArpRate          = "arpRate";
    inline constexpr const char* ArpOctaves       = "arpOctaves";
    inline constexpr const char* ArpGate          = "arpGate";

    // Effects — BitCrush
    inline constexpr const char* BcEnabled        = "bcEnabled";
    inline constexpr const char* BcBitDepth       = "bcBitDepth";
    inline constexpr const char* BcRateReduce     = "bcRateReduce";
    inline constexpr const char* BcMix            = "bcMix";

    // Effects — Filter
    inline constexpr const char* FltEnabled       = "fltEnabled";
    inline constexpr const char* FltType          = "fltType";
    inline constexpr const char* FltCutoff        = "fltCutoff";
    inline constexpr const char* FltResonance     = "fltResonance";

    // Effects — Chorus
    inline constexpr const char* ChEnabled        = "chEnabled";
    inline constexpr const char* ChRate           = "chRate";
    inline constexpr const char* ChDepth          = "chDepth";
    inline constexpr const char* ChMix            = "chMix";

    // Effects — Delay
    inline constexpr const char* DlEnabled        = "dlEnabled";
    inline constexpr const char* DlTime           = "dlTime";
    inline constexpr const char* DlFeedback       = "dlFeedback";
    inline constexpr const char* DlMix            = "dlMix";

    // Effects — Reverb
    inline constexpr const char* RvEnabled        = "rvEnabled";
    inline constexpr const char* RvSize           = "rvSize";
    inline constexpr const char* RvDamping        = "rvDamping";
    inline constexpr const char* RvWidth          = "rvWidth";
    inline constexpr const char* RvMix            = "rvMix";
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

} // namespace cart
