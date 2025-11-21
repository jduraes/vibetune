# VibeTune TODO and Roadmap

## High Priority

### Hardware/Sound Card Support
- [ ] Test SN76489/PSG playback (pitfall.vgm)
- [ ] Verify dual PSG chip detection
- [ ] Test on additional sound cards:
  - [ ] NABU AY-3-8910
  - [ ] HEATH H8 with Les Bird's MSX Card
  - [ ] MBC PSG module
  - [ ] DUODYNE Sound Module
  - [ ] Z50 LiNC Sound Module
- [ ] Verify SC126 VGM timing with new formula
- [ ] Test on 20MHz Z80 systems

### File Format Support
- [ ] D00 format implementation (currently stubbed out)
- [ ] YM format support
- [ ] Additional VGM chips:
  - [ ] YM2612 (Sega Genesis)
  - [ ] YM2151 (arcade)
  - [ ] Other chip types as needed

## Medium Priority

### Features
- [ ] Loop control option (continuous loop vs single play)
- [ ] Volume control interface
- [ ] Playlist support
- [ ] Display track time/progress during playback

### Documentation
- [ ] Hardware compatibility matrix
- [ ] File format support matrix
- [ ] Performance benchmarks across CPU speeds
- [ ] User guide with examples

## Low Priority

### Code Quality
- [ ] Modularization (see MODULARIZATION_FAILURE_ANALYSIS.md for context)
- [ ] Code comments cleanup
- [ ] Optimize memory usage
- [ ] Performance profiling

### Testing
- [ ] Automated test suite
- [ ] Regression testing framework
- [ ] Test files for all supported formats

## Completed (v0.2.7.25)
- [x] VGM timing calibration for RC2014
- [x] Fix RC2014 crash (heap clear issue)
- [x] Proportional timing formula for all CPU speeds
- [x] AY-3-8910 dual chip support
- [x] OPL2/OPL3 playback
- [x] Dynamic port addressing

## Notes
- VGM timing now uses formula: ((CPU_KHz * 2) / 1024) * 11/16
- Tested on RC2014 @ 7.3728 MHz and SC126 @ 18.432 MHz
- PT3/PT2/MYM formats working correctly
