# VGM Playback Status - v0.2.7.25

## Working Features

### Timing System
- ✅ Proportional timing formula: ((CPU_KHz * 2) / 1024) * 11/16
- ✅ Calibrated for 128 BPM target on RC2014 (7.3728 MHz)
- ✅ Automatically scales for all CPU speeds
- ✅ Tested on RC2014 (Z80) and SC126 (Z180)

### OPL2/OPL3 (YM3812/YMF262)
- ✅ Command parsing (0x5A, 0x5E, 0x5F)
- ✅ Register writes with OPL timing delays
- ✅ Volume boost (carrier operators only, zero attenuation)
- ✅ Dual bank support (OPL3)
- ✅ Proper muting on exit
- ✅ Detection and display

### AY-3-8910
- ✅ Command parsing (0xA0)
- ✅ Single chip support (chip 1)
- ✅ Dual chip support (chip 2, bit 7 in register)
- ✅ Dynamic port addressing via LD DE,(PORTS) and LD HL,(PORTS2)
- ✅ Proper muting on exit (both chips)
- ✅ Detection and display (shows "2x AY-3-8910" for dual)
- ✅ Configurable secondary chip ports (RSEL2/RDAT2 in config table)

### General VGM
- ✅ Header parsing (v1.50+ format)
- ✅ VGM data offset calculation
- ✅ Command stream processing
- ✅ Wait commands (0x61, 0x62, 0x63, 0x70-0x7F)
- ✅ Loop support (0x66 with loop offset)
- ✅ Keyboard abort handling
- ✅ Filename and chip type display

## Not Yet Implemented

### SN76489 (PSG)
- ⏳ Command parsing exists (0x50, 0x30) 
- ⏳ Port writes to PSGREG/PSG2REG constants
- ⏳ Needs testing with actual PSG VGM files
- ⏳ Detection shows "SN76489" but untested

### Other Chips
- ❌ YM2612 (Sega Genesis)
- ❌ YM2151 (arcade)
- ❌ Additional chip types

## Code Structure

### Files
- `src/audio/vgm_player.inc` - Command processor and playback engine
- `src/audio/filetype_config.inc` - VGM detection, init, timing calculation
- `src/ui/messages.inc` - Display strings
- `vibetune.asm` - Main program, config table with PORTS2 support

### Key Functions
- `VGM_PLAY_FRAME` - Process commands until wait
- `VGM_APPLY_DELAY` - Frame timing with calibrated delay loop
- `VGM_MUTE_ALL` - Silence all chips
- `VGM_BOOST_VOLUME` - OPL carrier-only boost

### Port Configuration
- Primary AY: `PORTS` (RSEL/RDAT) - dynamic from config
- Secondary AY: `PORTS2` (RSEL2/RDAT2) - dynamic from config
- OPL3: Hardcoded 0x90-0x93
- PSG: Hardcoded 0x7C/0x7D (RC2014), 0x84/0x8A (SC126)

## Testing Status
- ✅ goneshrt.vgm (OPL2) - Perfect playback, max volume
- ✅ tiger.vgm (2x AY) - Correct playback, shows dual chip
- ✅ penguin.vgm (AY) - Correct tempo (128 BPM), loops properly
- ⏳ pitfall.vgm (PSG) - Need to test

## Known Issues
- None currently (v0.2.7.25)

## Next Steps
1. Test SN76489/PSG playback with pitfall.vgm
2. Verify dual PSG chip detection
3. Test timing on SC126 @ 18.432 MHz
4. Test on 20MHz Z80 systems
5. Document PSG results
