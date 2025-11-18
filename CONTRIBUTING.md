# Contributing to VibeTune

## Quick Start

VibeTune is a Z80 assembly project targeting RomWBW systems. Here's how to get started:

### Prerequisites
- macOS (current setup)
- RomWBW development environment at `../RomWBW/`
- z88dk installed at `~/z88dk/`
- MAME with FATDISK mounted at `/Volumes/FATDISK` (for testing)

### Development Workflow

1. **Build**: `make vibetune.com`
2. **Test**: `make test` (runs z88dk-ticks emulator)
3. **Deploy**: `make deploy` (copies to MAME disk)
4. **All-in-one**: `make release`

### Code Style

VibeTune follows CP/M/DOS 8.3 filename conventions:
- Use letters over symbols in filenames when possible
- Lowercase letters are acceptable
- Avoid forcing uppercase unless needed

### Current Architecture (v0.2.0)

**Modular Structure**:
- `vibetune.asm` - Main program logic (streamlined)
- `src/audio/filetype_config.inc` - Format-specific audio configuration
- `src/io/filetype_detection.inc` - File type identification
- `src/io/filetypes.inc` - File type constants
- `src/hardware/constants.inc` - Hardware definitions
- `src/ui/messages.inc` - User interface messages

### Extension Points

The modular architecture provides clear extension points for:
- **New file formats** - Add detection logic and configuration
- **Hardware support** - Extend constants and detection
- **UI enhancements** - Modify message handling and display

### Testing Strategy

**Current**: Basic emulator validation with z88dk-ticks
**Planned**: Unit tests, integration tests, module validation

### Version Management

- **Semantic versioning**: major.minor.patch
- **Build dates**: Automatic injection in DD-MMM-YYYY format
- **Git tags**: All releases tagged (e.g., v0.1.0)

### Key Files

- `vibetune.asm` - Main source code
- `Makefile` - Build system with testing/deployment
- `VERSION` - Current version number
- `CHANGELOG.md` - Release history
- `PROJECT_STATUS.md` - Current development phase

### Making Changes

1. Create feature branch from master
2. Make changes with appropriate comments
3. Test with `make test`
4. Update CHANGELOG.md if needed
5. Commit with descriptive messages
6. Ensure `make release` works before PR

### Immediate Contribution Opportunities

1. **Additional audio formats** (VGM, D00, SID, MOD) - Use established extension points
2. **Enhanced hardware support** (OPL, FM synthesis) - Extend hardware constants
3. **User interface improvements** (real-time controls, better info display)
4. **Advanced features** (playlists, looping, volume control)
5. **Performance optimization** (faster file loading, better timing)

### Development Guidelines

- **Small, safe changes** - Test each modification thoroughly
- **Preserve compatibility** - Maintain existing functionality
- **Follow module boundaries** - Respect the established architecture
- **Document extensions** - Update relevant .md files

The project has completed Phase 2 (Modularization) and is ready for format extensions and feature enhancements.
