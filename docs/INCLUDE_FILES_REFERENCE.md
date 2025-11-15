# VibeTune Include Files Reference

## Overview
This document provides complete documentation of all include files in VibeTune v0.2.6, their locations, contents, and purposes after the successful modularization completed on 2025-10-07.

## Directory Structure

```
src/
├── audio/
│   └── filetype_config.inc     # Audio format configurations
├── cli/
│   └── cli.inc                 # Command-line argument processing
├── hardware/
│   └── constants.inc           # Hardware and system constants
├── io/
│   ├── filetype_detection.inc  # File type detection logic
│   └── filetypes.inc           # File type constant definitions
├── system/
│   ├── cpm.inc                 # CP/M system interface
│   └── hbios.inc               # HBIOS function definitions
├── ui/
│   └── messages.inc            # User interface messages
├── utils/
│   ├── printing.inc            # Print utility functions
│   ├── strings.inc             # String manipulation utilities
│   └── timing.inc              # Timer and delay functions
└── tune.inc                    # Application-specific macros
```

---

## Include Files Documentation

### System Interface Files (`src/system/`)

#### `src/system/hbios.inc`
- **Purpose**: HBIOS (RomWBW Hardware BIOS) function definitions and constants
- **Size**: 661 bytes
- **Contents**:
  - HBIOS function codes (BF_SYSVER, BF_SYSGET, etc.)
  - System identification constants
  - Hardware interface definitions
- **Used by**: Main application for system calls and hardware detection
- **Dependencies**: None

#### `src/system/cpm.inc`  
- **Purpose**: CP/M system interface constants and vectors
- **Size**: 145 bytes
- **Contents**:
  - CP/M BDOS function vectors
  - System restart vectors
  - FCB (File Control Block) definitions
  - Command line argument locations
- **Used by**: File operations, program termination, system interface
- **Dependencies**: None

### Utility Functions (`src/utils/`)

#### `src/utils/printing.inc`
- **Purpose**: Print utility functions for character and string output
- **Size**: 2,663 bytes  
- **Contents**:
  - PRTCHR: Print single character
  - PRTSTR: Print null-terminated string
  - PRTHEX: Print hexadecimal values
  - CRLF: Print carriage return/line feed
  - Error handling and register preservation
- **Used by**: All output operations throughout the application
- **Dependencies**: CP/M BDOS calls

#### `src/utils/strings.inc`
- **Purpose**: String manipulation and search utilities
- **Size**: 444 bytes
- **Contents**:
  - STRINDEX: Search for substring within string
  - String comparison functions
  - String length utilities
- **Used by**: File extension parsing, command-line processing
- **Dependencies**: None

#### `src/utils/timing.inc`
- **Purpose**: Timer and delay functions for audio playback timing
- **Size**: 1,707 bytes
- **Contents**:
  - WAITQ: Wait for quark play time
  - Hardware timer support detection
  - CPU speed-calibrated delay loops
  - PROBETIMER: Detect available timing methods
- **Used by**: Audio playback engines for precise timing
- **Dependencies**: HBIOS for timer detection

### Command-Line Interface (`src/cli/`)

#### `src/cli/cli.inc`
- **Purpose**: Command-line argument parsing and validation
- **Size**: 2,465 bytes
- **Contents**:
  - CLI_ABRT_IF_OPT_FIRST: Check for help/usage options
  - CLI_PORTS: Parse port selection options (-msx, -rc)
  - CLI_HAVE_HBIOS_SWITCH: Parse --hbios option
  - CLI_HAVE_DELAY_SWITCH: Parse -delay option
  - CLI_OCTAVE_ADJST: Parse octave adjustment (+tn/-tn)
  - CLI_ABRT_UNSUPPFILTYP: Validate file type support
- **Used by**: Main application startup for option processing
- **Dependencies**: String utilities, error messages

### User Interface (`src/ui/`)

#### `src/ui/messages.inc`
- **Purpose**: All user interface text messages and strings
- **Size**: 1,348 bytes
- **Contents**:
  - MSGBAN: Application banner with version
  - MSGUSE: Usage instructions and copyright
  - Error messages (MSGBIO, MSGPLT, MSGHW, etc.)
  - Status messages (MSGTIM, MSGDLY, MSGPLY, MSGEND)
  - Song information messages (MSGSONGNAME, MSGARTIST)
  - Special messages (MSGUNSUP)
- **Used by**: All user-facing output and error reporting
- **Dependencies**: HBIOS version constants (RMJ, RMN)

### Audio System (`src/audio/`)

#### `src/audio/filetype_config.inc`
- **Purpose**: Audio file format-specific configurations
- **Size**: 3,919 bytes
- **Contents**:
  - Load address configurations for different file types
  - Timing parameters for PT2, PT3, MYM formats
  - Audio engine initialization parameters
- **Used by**: Audio file loading and player initialization
- **Dependencies**: File type constants

### Input/Output (`src/io/`)

#### `src/io/filetypes.inc`
- **Purpose**: File type constant definitions
- **Size**: 675 bytes
- **Contents**:
  - TYPPT2, TYPPT3, TYPMYM: File type identification values
  - PORTS_AUTO, PORTS_MSX, PORTS_RC: Audio port selection modes
  - HEAPEND: Memory management constant
- **Used by**: File type detection, audio system configuration
- **Dependencies**: None

#### `src/io/filetype_detection.inc`
- **Purpose**: File type detection logic based on extensions
- **Size**: 2,173 bytes  
- **Contents**:
  - Extension parsing (.PT2, .PT3, .MYM)
  - File type validation and identification
  - Default extension handling
- **Used by**: File loading process
- **Dependencies**: File type constants, string utilities

### Hardware Interface (`src/hardware/`)

#### `src/hardware/constants.inc`
- **Purpose**: Hardware-specific constants and configuration strings
- **Size**: 1,341 bytes
- **Contents**:
  - CPU control constants (SBCV2004, CPUFAMZ180)
  - Conditional assembly constants (_ZX, _MSX, _WBW, HBIOS)
  - Player configuration (CurPosCounter, ACBBAC, LoopChecker, Id)
  - Hardware description strings (HWSTR_* for all supported platforms)
- **Used by**: Hardware detection, platform-specific configuration
- **Dependencies**: None

### Application Core (`src/`)

#### `src/tune.inc`
- **Purpose**: Application-specific macros and definitions
- **Size**: 150 bytes
- **Contents**:
  - ISHBIOS: Macro for HBIOS detection
  - PRTSTRDE: Macro for string printing
  - PRTCRLF: Macro for line formatting
  - ERRWITHMSG: Macro for error handling with messages
- **Used by**: Main application code for common operations
- **Dependencies**: HBIOS detection, message system

---

## Include Dependencies Graph

```
vibetune.asm (main)
├── ../RomWBW/Source/ver.inc (external)
├── src/system/hbios.inc
├── src/system/cpm.inc  
├── src/tune.inc
├── src/utils/timing.inc
├── src/utils/strings.inc
├── src/cli/cli.inc
├── src/utils/printing.inc
└── src/ui/messages.inc

Additional modular files (currently unused but available):
├── src/io/filetypes.inc
├── src/hardware/constants.inc  
├── src/io/filetype_detection.inc
└── src/audio/filetype_config.inc
```

## Usage Patterns

### Build Process
The main `vibetune.asm` file includes all necessary modules in the following order:
1. **External dependencies**: RomWBW version information
2. **System interfaces**: HBIOS and CP/M definitions
3. **Application core**: Tune-specific macros
4. **Utility functions**: Timing, strings, CLI, printing (loaded later)
5. **User interface**: Messages (loaded later)

### Modularization Strategy
- **System files**: Isolated CP/M and HBIOS interfaces
- **Utilities**: Reusable functions grouped by purpose
- **Business logic**: CLI, audio, and I/O grouped by domain
- **Presentation**: UI messages separated from logic
- **Configuration**: Hardware and file type constants modularized

## Size Analysis

| Category | Files | Total Size | Percentage |
|----------|--------|------------|------------|
| **Utilities** | 3 | 4,814 bytes | 37.8% |
| **Audio** | 1 | 3,919 bytes | 30.8% |
| **CLI** | 1 | 2,465 bytes | 19.4% |
| **I/O** | 2 | 2,848 bytes | 22.4% |
| **Hardware** | 1 | 1,341 bytes | 10.5% |
| **UI** | 1 | 1,348 bytes | 10.6% |
| **System** | 2 | 806 bytes | 6.3% |
| **Core** | 1 | 150 bytes | 1.2% |
| **TOTAL** | **12** | **12,737 bytes** | **100%** |

## Future Expansion

The modular structure supports easy addition of:
- New audio formats (add to `src/audio/`)
- Additional utility functions (add to `src/utils/`)
- Hardware platform support (extend `src/hardware/`)
- Enhanced CLI features (extend `src/cli/`)
- Internationalization (extend `src/ui/`)

## Maintenance Guidelines

1. **Single Responsibility**: Each include file has one clear purpose
2. **Dependency Minimization**: Avoid circular dependencies
3. **Testing**: Test after each include file modification
4. **Documentation**: Update this reference when adding new includes
5. **Naming**: Use descriptive names that indicate file purpose
6. **Organization**: Group related functionality in appropriate directories

---

*Generated for VibeTune v0.2.6 - Complete Include File Reference*  
*Last Updated: 2025-10-07*