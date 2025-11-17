#!/usr/bin/env python3
"""Trim VGM file to specified byte size while preserving header and adding EOF marker."""

import sys
import struct

def trim_vgm(input_file, output_file, max_size):
    with open(input_file, 'rb') as f:
        data = bytearray(f.read())
    
    if len(data) < 0x40:
        print("Error: File too small to be valid VGM")
        return False
    
    # Check VGM magic
    if data[0:4] != b'Vgm ':
        print("Error: Not a valid VGM file")
        return False
    
    # Get VGM data offset (0x34 is standard, but check header)
    vgm_data_offset_ptr = struct.unpack('<I', data[0x34:0x38])[0]
    if vgm_data_offset_ptr == 0:
        vgm_data_start = 0x40  # Default offset
    else:
        vgm_data_start = 0x34 + vgm_data_offset_ptr
    
    print(f"VGM data starts at: 0x{vgm_data_start:04X}")
    print(f"Original size: {len(data)} bytes")
    
    # Calculate how much data we can keep
    if max_size <= vgm_data_start + 1:
        print(f"Error: max_size too small (need at least {vgm_data_start + 1} bytes)")
        return False
    
    # Trim to max_size - 1 (leave room for EOF marker 0x66)
    trim_point = max_size - 1
    data = data[:trim_point]
    
    # Add EOF marker
    data.append(0x66)
    
    print(f"Trimmed size: {len(data)} bytes")
    
    # Update EOF offset in header (at 0x04)
    eof_offset = len(data) - 4
    data[0x04:0x08] = struct.pack('<I', eof_offset)
    
    # Clear loop offset (at 0x1C) since we're truncating
    data[0x1C:0x20] = struct.pack('<I', 0)
    
    with open(output_file, 'wb') as f:
        f.write(data)
    
    print(f"Wrote {output_file}")
    return True

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print("Usage: trim_vgm.py <input.vgm> <output.vgm> <max_bytes>")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    max_size = int(sys.argv[3])
    
    if trim_vgm(input_file, output_file, max_size):
        print("Success!")
    else:
        sys.exit(1)
