# VibeTune v0.1.0

VibeTune is an enhanced music player for RomWBW, evolved from the original Tune.com application. This project aims to create a more advanced and feature-rich sound player that supports multiple sound file formats and multiple sound cards.

**Current Version**: v0.1.0 - Based on RomWBW Tune v3.13 (28-May-2025)

## Version Numbering

VibeTune follows semantic versioning (major.minor.patch):
- **Major**: Breaking changes or significant feature additions
- **Minor**: New features, backwards compatible
- **Patch**: Bug fixes and small improvements

## Features

Currently VibeTune supports the same features as the original Tune.com:

- **Sound File Formats**: PT2, PT3, and MYM files
- **Sound Chips**: AY-3-8910, YM2149, and compatible chips
- **Multiple Platforms**: Supports various RomWBW-compatible systems
- **Hardware Detection**: Automatic sound hardware detection and configuration
- **Multiple Sound Modules**: Support for various sound expansion cards

## Usage

```
VIBETUNE <filename>.[PT2|PT3|MYM] [-msx|-rc] [-delay] [--hbios] [+tn|-tn]
```

### Parameters

- `<filename>`: Sound file to play (PT2, PT3, or MYM format)
- `-msx`: Force MSX standard ports (A0H/A1H)
- `-rc`: Force RCBus standard ports (D8H/D0H)
- `-delay`: Force delay mode timing
- `--hbios`: Use HBIOS sound driver
- `+tn/-tn`: Octave adjustment (up/down by n octaves)

## Building

To build VibeTune:

```bash
make vibetune.com
```

## Development Roadmap

Future enhancements planned for VibeTune:

1. **Additional Sound Formats**: Support for more music file formats
2. **Enhanced Sound Card Support**: Better multi-card detection and management
3. **Improved User Interface**: Enhanced playback controls and information display
4. **Advanced Playback Features**: Looping, playlist support, fade effects
5. **Real-time Controls**: Volume control, tempo adjustment, channel muting

## Technical Notes

- Maximum CPU speed: ~8MHz for proper sound chip operation (Z180 can run faster due to I/O wait states)
- Optimal PSG clock: ~1.77MHz (similar to MSX/ZX Spectrum standards)
- Memory requirement: Loads files up to available heap space (typically ~48KB)
- Timer support: Uses hardware timers when available, falls back to CPU delay loops

## Origins

VibeTune is based on the original Tune.com from RomWBW, which incorporates:

- **PTx Player**: Universal PT2/PT3 player by S.V.Bulba
- **MYM Player**: MYM player by Marq/Lieves!Tuore
- **RomWBW Integration**: Hardware abstraction and platform support by Wayne Warthen

## License

GNU GPL v3 (inherited from original Tune.com)

## Contributing

This project is in active development. Contributions and suggestions are welcome!
