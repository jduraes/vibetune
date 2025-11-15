# VibeTune Project Rules (WARP.md)

**Last Updated**: 2025-11-15  
**Current Version**: 0.2.6.27

## 📋 Development Rules

### 1. Versioning System (CRITICAL)
**Format**: x.y.z.a where:
- **x**: Major version (breaking changes)
- **y**: Minor version (new features, backwards compatible) 
- **z**: Patch version (bug fixes and small improvements)
- **a**: Build number (increments with EVERY compilation after ANY change)

**RULE**: The 4th digit (a) MUST be incremented with every single change that results in compilation.

**AUTOMATIC VERSION INCREMENT RULE**: The build system MUST automatically increment the version number every time a compilation happens and the test is successful.

**AUTOMATIC MAME DEPLOYMENT RULE**: The build system MUST automatically copy the latest builds to MAME after successful testing so that it can be tested in MAME immediately.

**HUMAN VERIFICATION AND COMMIT RULE**: Upon positive confirmation that everything works in MAME (by copy-pasting successful execution output), automatically commit to the project and update all documentation.

**Current Version Tracking**:
- All files must maintain version consistency
- VERSION file, messages.inc, README.md, Makefile, PROJECT_STATUS.md must all match
- Build system automatically updates date AND version number
- Version increment happens automatically on successful build+test
- MAME deployment happens automatically after successful build+test
- Upon MAME verification, commit and documentation update happens automatically

### 2. File Naming Conventions
**CP/M/DOS 8.3 Compatibility**:
- Favor letters over symbols in file names when space permits
- Use symbols if there is space remaining
- Lowercase letters are acceptable, especially if derived from already lowercase values
- **DO NOT** force uppercase when there's no need

### 3. Architecture Rules
**Modular Design Principles**:
- Maintain separation of concerns through src/ structure
- Use established extension points for new formats
- Preserve backward compatibility with existing formats (PT2/PT3/MYM)
- Test each modification thoroughly

### 4. Build and Testing Rules
**Every Change Must**:
1. Compile successfully (`make vibetune.com`)
2. Pass emulator test (`make test`)  
3. Increment version number (x.y.z.a format)
4. Update all version references consistently

### 5. Wayne Warthen Integration Patterns
**When adding new audio formats**:
- Follow Wayne's file extension dispatch pattern
- Use modular DETECT_FILE_TYPE and CONFIGURE_FILE_PLAYBACK functions
- Implement shared heap memory management approach
- Maintain clean library attribution (credit original authors)

## 🎯 Current Status
- **Phase**: Wayne's Learnings Applied - VGM Extension Complete
- **Architecture**: Fully modularized with clear extension points
- **Formats Supported**: PT2, PT3, MYM, VGM (placeholder)
- **Binary Size**: ~5156 bytes (optimized)

## 📝 Change Log Tracking
Each version increment must be documented with:
- What changed
- Files modified
- Binary size impact
- Testing verification

## 🔄 Complete Development Workflow

### Step 1: Make Changes
- Edit source code, add features, fix bugs

### Step 2: Build and Auto-Deploy  
```bash
make build
```
- Builds executable
- Runs emulator tests
- Auto-increments version (x.y.z.a)
- Deploys to MAME automatically

### Step 3: MAME Verification
- Test in MAME CP/M system
- Verify functionality works correctly
- Copy-paste successful execution output

### Step 4: Commit After Verification
```bash
make commit  
```
- Updates documentation timestamps
- Adds changelog entry
- Creates git commit with detailed message
- Creates git tag for version

## ⚠️ Critical Reminders
- **NEVER** commit arbitrary version increments
- **ALWAYS** maintain version consistency across all files
- **TEST** every change before committing
- **DOCUMENT** every version increment with reasoning
- **VERIFY** in MAME before final commit

---

**This file tracks project-specific development rules and must be updated when rules change.**