# Changelog

All notable changes to VibeTune will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2025-10-05

### Added
- Initial VibeTune release based on RomWBW Tune v3.13 (28-May-2025)
- Rebranded application banner and messages
- New versioning system (semantic versioning)
- Enhanced project structure with git repository
- Comprehensive README.md with feature roadmap
- Support for PT2, PT3, and MYM music file formats
- Hardware auto-detection for various sound chips (AY-3-8910, YM2149, etc.)
- Multi-platform support for RomWBW-compatible systems
- Command-line options for port forcing, timing modes, and octave adjustment

### Changed
- Application name from "Tune Player" to "VibeTune"
- Banner message to reflect version and heritage
- Usage message to show "VIBETUNE" command
- Build system adapted for standalone repository

### Technical Details
- Source forked from `/Users/mduraes/Documents/development/RomWBW/Source/Apps/Tune`
- Maintains full compatibility with original Tune.com functionality
- Uses z88dk-compatible build system
- Successfully tested with z88dk-ticks emulator

## [0.2.0] - 2025-10-05

### Added 
- Modular file type identification system
- File type detection module (`src/io/filetype_detection.inc`)
- File type configuration module (`src/audio/filetype_config.inc`) 
- Clear extension points for adding new file formats (VGM, D00, etc.)
- Improved code organization and maintainability
- Dynamic build date in banner message (DD-MMM-YYYY format)
- Automatic build date injection during compilation
- Enhanced build system with emulator testing
- MAME deployment automation
- Comprehensive make targets (test, deploy, release, help, clean)

### Changed
- Restructured file type handling for better modularity
- Reduced main program complexity through modularization
- Fixed include order dependency issues

### Fixed
- Console I/O functionality after modularization
- Proper function definition ordering

## [Unreleased]

### Planned
- Additional sound file format support (VGM, D00, etc.)
- Enhanced user interface and playback controls
- Multi-sound card management improvements
- Real-time audio controls (volume, tempo, channel muting)
- Playlist and looping functionality
