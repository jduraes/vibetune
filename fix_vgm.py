#!/usr/bin/env python3
"""Apply VGM timing, volume, and key-off fixes."""

# Fix 1: Slow down by 12.5%
with open('src/audio/filetype_config.inc', 'r') as f:
    lines = f.readlines()

# Find and replace timing calculation (around line 151-156)
for i in range(len(lines)):
    if '\tLD\tA,L\t\t\t; Get result' in lines[i] and i > 140:
        # Replace next 5 lines
        lines[i] = '\tLD\tA,L\t\t\t; Get result\n'
        lines[i+1] = '\t; Add 12.5% to slow down\n'
        lines[i+2] = '\tLD\tB,A\n'
        lines[i+3] = '\tSRL\tA\n'
        lines[i+4] = '\tSRL\tA\n'
        lines[i+5] = '\tSRL\tA\t\t\t; A = original/8\n'
        lines.insert(i+6, '\tADD\tA,B\t\t\t; Add back = 112.5%\n')
        break

# Add VGM_MUTE_ALL call before playback
for i in range(len(lines)):
    if '\t; Initialize VGM playback' in lines[i]:
        lines.insert(i+1, '\tCALL\tVGM_MUTE_ALL\t\t; Reset all sound hardware first\n')
        break

with open('src/audio/filetype_config.inc', 'w') as f:
    f.writelines(lines)

print("✓ Fixed timing and added reset call")

# Fix 2 & 3: Add OPL delays, key-off, and volume boost
with open('src/audio/vgm_player.inc', 'r') as f:
    lines = f.readlines()

# Add OPL delays after address writes
for i in range(len(lines)):
    if 'OUT\t(OPL3ADDR' in lines[i]:
        lines.insert(i+1, '\tPUSH\tBC\n')
        lines.insert(i+2, '\tLD\tB,3\n')
        lines.insert(i+3, '\tDJNZ\t$\n')
        lines.insert(i+4, '\tPOP\tBC\n')

# Add key-off loop before register clear in VGM_MUTE_ALL
for i in range(len(lines)):
    if '\t; Clear OPL3 registers' in lines[i]:
        keyoff = [
            '\t; Turn off all OPL keys first\n',
            '\tLD\tC,0B0H\n',
            'VGM_OPL_KEYOFF:\n',
            '\tLD\tA,C\n',
            '\tOUT\t(OPL3ADDR1),A\n',
            '\tXOR\tA\n',
            '\tOUT\t(OPL3DATA1),A\n',
            '\tLD\tA,C\n',
            '\tOUT\t(OPL3ADDR2),A\n',
            '\tXOR\tA\n',
            '\tOUT\t(OPL3DATA2),A\n',
            '\tINC\tC\n',
            '\tLD\tA,C\n',
            '\tCP\t0B9H\n',
            '\tJR\tNZ,VGM_OPL_KEYOFF\n',
            '\t\n'
        ]
        for j, line in enumerate(keyoff):
            lines.insert(i+j, line)
        break

with open('src/audio/vgm_player.inc', 'w') as f:
    f.writelines(lines)

print("✓ Added OPL delays and key-off")
print("\nNote: Volume boost not added - needs more careful implementation")
print("Test with current fixes first")
