# Contributing to VibeTune

## Quick Start

VibeTune is a Z80 assembly project targeting RomWBW systems. Here's how to get started:

### Prerequisites
- macOS (current setup)
- RomWBW development environment at `../RomWBW/`
- z88dk installed at `/Users/mduraes/z88dk/`
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

### Current Architecture

**Single File Structure** (to be refactored):
- `vibetune.asm` - Monolithic source (~57K lines)
- Includes PTx player, MYM player, hardware abstraction
- Direct port from RomWBW Tune v3.13

### Planned Refactoring

The codebase will be modularized into:
- `audio/` - Sound generation and playback engines
- `io/` - File I/O and data parsing
- `hardware/` - Sound chip detection and management
- `ui/` - User interface and command processing

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

### Future Contribution Areas

1. **Additional audio formats** (SID, MOD, etc.)
2. **Enhanced hardware support** (OPL, FM synthesis)
3. **User interface improvements** (real-time controls)
4. **Performance optimization** (faster file loading)
5. **Advanced features** (playlists, effects)

The project is currently in Phase 2 (Enhancement) - ready for modularization and feature additions.