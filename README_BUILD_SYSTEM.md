# VibeTune Build System - Single Source of Truth

## Overview

The VibeTune build system uses a **single source of truth** approach for version numbers and build dates to ensure consistency across all files.

## Version Management

### Source of Truth
- **VERSION** file contains the current version (e.g., `0.2.6.27`)
- All version references are generated from this single file

### How It Works

1. **VERSION file** - Contains only the version number
2. **build_meta.sh** - Script that reads VERSION and generates assembly include file
3. **src/build_meta.inc** - Auto-generated file with version string (DO NOT EDIT MANUALLY)
4. **vibetune.asm** - Includes build_meta.inc before messages.inc
5. **src/ui/messages.inc** - References VERSION_STR from build_meta.inc

### Build Process

When you run `make vibetune.com`:
1. Makefile calls `build_meta.sh`
2. Script reads current version from VERSION file
3. Script gets current date
4. Script generates `src/build_meta.inc` with:
   ```asm
   VERSION_STR: .DB "VibeTune v0.2.6.27 for RomWBW, 15-Nov-2025",0
   ```
5. Assembler includes this in the binary

### Version Increment

To increment the version:
```bash
make increment-version
```

This will:
- Read current version from VERSION
- Increment the build number (4th digit)
- Update VERSION file
- Update documentation files (README.md, PROJECT_STATUS.md, WARP.md)

### Benefits

✅ **No hardcoded versions** - VERSION file is the only place to change version  
✅ **Auto-updated dates** - Build date is always current  
✅ **No manual sed operations** - Build system handles everything  
✅ **Consistent across all files** - Version appears identically everywhere  
✅ **Git-friendly** - src/build_meta.inc is generated, not committed  

### Files Managed

- `VERSION` - Master version number (committed to git)
- `src/build_meta.inc` - Generated each build (NOT committed to git)
- `build_meta.sh` - Generator script (committed to git)

### .gitignore

Add this line to `.gitignore`:
```
src/build_meta.inc
```

This ensures the auto-generated file isn't committed.

## Build Targets

- `make vibetune.com` - Build with current version
- `make increment-version` - Bump version number
- `make clean` - Remove build artifacts (including build_meta.inc)
- `make build` - Build, test, increment version, deploy
- `make commit` - Commit after hardware verification

## Example Workflow

```bash
# Make code changes
vim src/audio/filetype_config.inc

# Build and test
make vibetune.com

# Test on hardware
# (transfer vibetune.com to RC2014)

# If successful, increment version and commit
make increment-version
make vibetune.com
# Test again on hardware
make commit
```

