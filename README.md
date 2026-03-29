# Cartridge

A chiptune synthesizer plugin emulating the NES 2A03 APU with optional VRC6 expansion.

Available as **VST3**, **AU**, **CLAP**, and **Standalone**.

## Features

### Channels

- **2 Pulse channels** — PolyBLEP anti-aliased, 4 duty cycles (12.5%, 25%, 50%, 75%)
- **Triangle channel** — 32-step wavetable
- **Noise channel** — 15-bit LFSR with long and short modes
- **DPCM channel** — embedded drum samples

### VRC6 Expansion (toggle-able)

- **2 Pulse channels** — 8-level duty cycle
- **1 Sawtooth channel** — accumulator-based

### MIDI Modes

- **Split** — fixed channel-to-voice routing (Ch 1-5 = APU, Ch 5-7 = VRC6, Ch 10 = Noise)
- **Auto** — round-robin voice allocation
- **Mono** — monophonic with last-note priority

### Effects

Bitcrush, Filter (LP/HP/BP), Chorus, Delay, Reverb

### Modulation

- **LFO** — vibrato and tremolo
- **Arpeggiator** — Up/Down/UpDown/Random patterns, configurable rate, octaves, and gate
- **Portamento/Glide**

### Other

- 25 factory presets + user preset save/load
- Velocity sensitivity and pitch bend
- QWERTY-to-MIDI keyboard (standalone)

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `1`–`5` | Toggle APU channels (Pulse 1, Pulse 2, Triangle, Noise, DPCM) |
| `6`–`8` | Toggle VRC6 channels (Pulse 1, Pulse 2, Sawtooth) |
| `Left` / `Right` | Navigate presets |
| `Cmd+V` | Toggle VRC6 expansion |
| `Cmd+Z` / `Cmd+X` | Octave down / up |
| `Cmd+S` | Solo focused channel |
| `Cmd+M` | Mute focused channel |
| `Space` | All notes off (panic) |

## Building from Source

### Prerequisites

- CMake 3.22+
- C++17 compiler
- Git

### Submodules

This project uses [JUCE 8](https://juce.com/) and [clap-juce-extensions](https://github.com/free-audio/clap-juce-extensions) as Git submodules.

### Build

```bash
git clone --recursive https://github.com/smank/cartridge.git
cd cartridge

# Or if already cloned:
git submodule update --init --recursive

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Build artifacts are placed in:

```
build/Cartridge_artefacts/Release/
├── AU/          (macOS only)
├── CLAP/
├── Standalone/
└── VST3/
```

### Platform Support

- **macOS** — AU, VST3, CLAP, Standalone
- **Windows** — VST3, CLAP, Standalone
- **Linux** — VST3, CLAP, Standalone (requires ALSA, X11, FreeType, WebKit2GTK, and GL dev packages)

## Architecture

```
src/
├── dsp/
│   ├── channels/      Channel emulation (Pulse, Triangle, Noise, DPCM, VRC6)
│   ├── components/    APU components (Envelope, Length Counter, Sweep, Linear Counter)
│   ├── effects/       Effects chain (Bitcrush, Filter, Chorus, Delay, Reverb)
│   └── mixing/        Nonlinear NES DAC mixer
├── midi/              Voice manager, Arpeggiator, Note frequency tables
├── ui/                JUCE editor components
├── Parameters.h/cpp   All plugin parameters (APVTS)
├── PresetManager.h/cpp Factory and user presets
├── PluginProcessor.h/cpp
└── PluginEditor.h/cpp
```

## License

[GPL-3.0](LICENSE)
