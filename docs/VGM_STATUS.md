# VGM Playback Status - v0.2.7.3

## Working Features

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
- ✅ Dynamic timing calibration for 18.432 MHz CPU
- ✅ Keyboard abort handling
- ✅ adplay-style info display (filename, chip type)

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
- `src/audio/filetype_config.inc` - VGM detection, init, main loop
- `src/ui/messages.inc` - Display strings
- `vibetune.asm` - Config table with PORTS2 support

### Key Functions
- `VGM_PLAY_FRAME` - Process commands until wait
- `VGM_APPLY_DELAY` - Frame timing with dynamic calibration
- `VGM_MUTE_ALL` - Silence all chips
- `VGM_BOOST_VOLUME` - OPL carrier-only boost

### Port Configuration
- Primary AY: `PORTS` (RSEL/RDAT) - dynamic from config
- Secondary AY: `PORTS2` (RSEL2/RDAT2) - dynamic from config
- OPL3: Hardcoded 0x90-0x93
- PSG: Hardcoded 0xFF/0xFB

## Testing Status
- ✅ goneshrt.vgm (OPL2) - Perfect playback, max volume
- ✅ tiger.vgm (2x AY) - Plays correctly, shows dual chip
- ✅ penguin.vgm (AY) - Plays correctly, loops properly
- ⏳ pitfall.vgm (PSG) - Need to test

## Next Steps
1. Test SN76489/PSG playback
2. Verify PSG port configuration
3. Add dual PSG chip detection
4. Document PSG in status
