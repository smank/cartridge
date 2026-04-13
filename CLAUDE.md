# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure (Release)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build all targets (Standalone, VST3, AU, CLAP)
cmake --build build --config Release

# Build single target
cmake --build build --config Release --target Cartridge_Standalone

# Configure with tests
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCARTRIDGE_BUILD_TESTS=ON

# Run tests
cd build && ctest --output-on-failure -C Release
# Or directly: ./build/CartridgeTests

# Launch standalone
open build/Cartridge_artefacts/Release/Standalone/Cartridge.app
```

Build artifacts land in `build/Cartridge_artefacts/Release/{AU,CLAP,Standalone,VST3}`.

CMakeLists.txt must use `LANGUAGES C CXX` (not just CXX) — JUCE/clap subprojects need C. `VST3_AUTO_MANIFEST FALSE` is required to suppress manifest generation issues on macOS.

## Architecture

**Cartridge** is a NES 2A03 APU emulation synth plugin (VST3/AU/CLAP/Standalone) built with JUCE 8 + clap-juce-extensions. It does bandlimited synthesis at DAW sample rate, not cycle-accurate emulation.

### Two Engine Modes

- **Classic Engine**: Hardware-accurate 2A03 with 5 channels (2 Pulse, Triangle, Noise, DPCM) + optional VRC6 Konami expansion (2 Pulse, Sawtooth). Each channel is an independent monophonic voice.
- **Modern Engine**: Polyphonic (up to 16 voices) with dual oscillators, ADSR envelope, unison, and portamento.

### Audio Signal Flow (processBlock)

```
MIDI Input → Voice Manager → [Classic: per-channel APU | Modern: voice pool]
  → Step Sequencer modulation (volume/pitch/duty per-sample)
  → LFO modulation (vibrato/tremolo)
  → Per-channel stereo panning (Classic only)
  → DC blocking filter
  → Effects chain: BitCrush → Filter → [stereo upmix] → Chorus → Delay → Reverb
  → Soft limiter (tanh)
```

### Parameter System

All ~190 parameters defined in `src/Parameters.h` (IDs) and `src/Parameters.cpp` (APVTS layout). The processor caches `std::atomic<float>*` pointers for lock-free real-time access. `updateDspFromParameters()` uses change detection (compares against `CachedParams` struct) to avoid unnecessary DSP updates.

**Critical pattern**: After `apu.reset()`, `cachedParams` must also be reset to `CachedParams{}` — otherwise change detection skips re-applying params that didn't change, leaving reset channels silent.

### MIDI Routing

`MidiVoiceManager` (Classic) supports 4 modes:
- **Split**: MIDI Ch 1→Pulse1, Ch 2→Pulse2, Ch 3→Triangle, Ch 10→Noise, Ch 4→DPCM, Ch 5-7→VRC6
- **Auto**: Round-robin across enabled channels with oldest-note stealing
- **Mono**: All notes to Pulse 1
- **Layer**: All enabled channels play every note

`ModernVoiceManager` handles the polyphonic Modern engine with MPE support.

### UI Layout

`CartridgeEditor` stacks components vertically: TopBar → Waveform → ChannelStrips/ModernPanel → ModulationBar → EffectsBar → StatusBar → Keyboard. The FX and Mod bars are accordion-style (mutually exclusive expansion). A minimum channel strip height (240px) is enforced so bottom panels never crush the channel controls. The layout fills the full window bounds — scaling options are 100%, 125%, 150%, 200%. First launch auto-detects the best scale for the display. In macOS fullscreen the content stretches to fill the screen.

### Thread Safety

- Audio thread reads `std::atomic<float>*` from APVTS, writes to `channelActive[8]`, `seqPlaybackStep[8]`, `waveformBuffer`
- UI thread reads those atomics for display, writes `StepSequenceData` with atomic version counter
- Preset changes and state restoration set `pendingDspReset` flag; actual reset is deferred to the audio thread

### Step Sequencer Data

Sequence step data (16 steps x 3 lanes x 8 channels) is stored outside APVTS as plain structs. The UI writes to `stepSequenceData[]`, bumps `sequenceDataVersion`, and the audio thread copies to `stepSeqSnapshot[]` at block boundaries. Serialized as `<StepSeq>` XML elements alongside the APVTS state.

### Preset System

36 factory presets + user presets. `PresetManager` handles save/load/import/export. Preset XML files can contain `<StepSeq>` elements for embedded step sequence data (used by VGM imports). `setCurrentProgram()` loads step seq data from presets and bumps `sequenceDataVersion`. `ABCompare` stores two APVTS snapshots for quick A/B toggling. Presets trigger `pendingDspReset` to cleanly reinitialize DSP state.

### VGM Import

`src/import/` contains a VGM/VGZ parser and instrument extractor. The parser handles gzip decompression, VGM header/command parsing, and GD3 tag extraction. The extractor tracks per-channel APU register state, detects note boundaries (writes to register 3), samples at 60Hz frame intervals, and produces `StepSequenceData` with volume/pitch/duty envelopes. Instruments are saved as user presets with embedded `<StepSeq>` XML via `VgmImporter::importFile()`. The Import button in TopBarComponent routes `.vgm`/`.vgz` files to this path.

## Key Files

- `src/PluginProcessor.cpp` — Central hub: MIDI dispatch, per-sample DSP loop, parameter wiring, state serialization
- `src/Parameters.h` — All parameter ID constants
- `src/midi/MidiVoiceManager.cpp` — Classic engine MIDI→APU routing with note gate callbacks
- `src/dsp/Apu.h` — Master APU controller owning all 8 channels
- `src/dsp/StepSequencer.h` — Phase-accumulator step sequencer (header-only)
- `src/ui/ModulationBarComponent.cpp` — Accordion UI for LFO/Porta/Arp/DPCM/StepSeq
- `src/import/VgmImporter.cpp` — Top-level VGM import: parse, extract, save presets
- `src/import/VgmParser.cpp` — VGM/VGZ file format parser (header, commands, GD3)
- `src/import/VgmInstrumentExtractor.cpp` — NES APU register analysis → StepSequenceData

## Known Gotchas

- `PulseChannel.setFrequency()` must also update `timerPeriod`, otherwise sweep muting check silences channel (timerPeriod=0 < 8)
- Triangle `noteOn()` should immediately call `linear.clock()` after `reload()` to prime the counter
- VRC6 channel routing must gate on `mix > 0` (not just `vrc6On`), otherwise Auto mode routes notes to all 3 VRC6 channels even when preset only wants one
- Release workflow (`.github/workflows/release.yml`) triggers on `v*` tags — do NOT create manual GitHub releases

## Dependencies

All via Git submodules in `libs/`:
- **JUCE 8** — Plugin framework
- **clap-juce-extensions** — CLAP format support
- **Catch2 v3.7.1** — Test framework (fetched via CMake FetchContent, test builds only)
