# Cartridge

Chiptune synthesizer plugin emulating the NES 2A03 APU with optional VRC6 expansion. Available as VST3, AU, CLAP, and Standalone.

## Features

- **5 APU channels** — 2 Pulse (PolyBLEP, 4 duty cycles), Triangle (32-step), Noise (15-bit LFSR, long/short), DPCM (embedded samples)
- **VRC6 expansion** — 2 Pulse (8-level duty), 1 Sawtooth (accumulator-based), toggleable
- **3 MIDI modes** — Split (fixed channel routing), Auto (round-robin), Mono
- **Effects** — Bitcrush, Filter (LP/HP/BP), Chorus, Delay, Reverb
- **Modulation** — LFO (vibrato/tremolo), Arpeggiator (Up/Down/UpDown/Random), Portamento
- **25 factory presets** + user preset save/load
- Velocity sensitivity, pitch bend, QWERTY-to-MIDI keyboard

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `1`–`5` | Toggle APU channels (Pulse 1/2, Triangle, Noise, DPCM) |
| `6`–`8` | Toggle VRC6 channels (Pulse 1/2, Sawtooth) |
| `Left` / `Right` | Navigate presets |
| `Cmd+V` | Toggle VRC6 expansion |
| `Cmd+Z` / `Cmd+X` | Octave down / up |
| `Cmd+S` | Solo channel |
| `Cmd+M` | Mute channel |
| `Space` | All notes off (panic) |

## Building from Source

Requires CMake 3.22+, a C++17 compiler, and Git. Uses [JUCE 8](https://juce.com/) and [clap-juce-extensions](https://github.com/free-audio/clap-juce-extensions) as submodules.

```bash
git clone --recursive https://github.com/smank/cartridge.git
cd cartridge
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Artifacts land in `build/Cartridge_artefacts/Release/{AU,CLAP,Standalone,VST3}`.

**macOS** — AU, VST3, CLAP, Standalone
**Windows** — VST3, CLAP, Standalone
**Linux** — VST3, CLAP, Standalone (requires ALSA, X11, FreeType, WebKit2GTK, GL dev packages)

## Architecture

```
src/
├── dsp/
│   ├── channels/      Pulse, Triangle, Noise, DPCM, VRC6
│   ├── components/    Envelope, Length Counter, Sweep, Linear Counter
│   ├── effects/       Bitcrush, Filter, Chorus, Delay, Reverb
│   └── mixing/        Nonlinear NES DAC mixer
├── midi/              Voice manager, Arpeggiator
└── ui/                Editor components
```

## License

[GPL-3.0](LICENSE)
