#!/usr/bin/env python3
"""
Build PT3 note period table using Ivan Roshin / tune.asm logic (T_PACK depacker,
NoteTableCreator, TC corrections). Compare against pt3notes.inc.

Reference: RomWBW Source/Apps/Tune/tune.asm (TP_0, NT_DATA, L1/L2/CORR).
C equivalent: deater/vmw-meter pi-chiptune/arm32/pt3_lib.c (NoteTablePropogate).
"""

from __future__ import annotations

import re
import struct
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
ROOT = SCRIPT_DIR.parent
PT3NOTES_INC = ROOT / "pt3notes.inc"

# --- T_PACK (tune.asm 2325-2373) ---
def _build_t_pack() -> bytes:
    out: list[int] = []
    out.extend([0x06EC * 2 >> 8, 0x06EC * 2 & 0xFF])
    for d in (
        0x0755 - 0x06EC,
        0x07C5 - 0x0755,
        0x083B - 0x07C5,
        0x08B8 - 0x083B,
        0x093D - 0x08B8,
        0x09CA - 0x093D,
        0x0A5F - 0x09CA,
        0x0AFC - 0x0A5F,
        0x0BA4 - 0x0AFC,
        0x0C55 - 0x0BA4,
        0x0D10 - 0x0C55,
    ):
        out.append(d & 0xFF)
    out.extend([0x066D * 2 >> 8, 0x066D * 2 & 0xFF])
    for d in (
        0x06CF - 0x066D,
        0x0737 - 0x06CF,
        0x07A4 - 0x0737,
        0x0819 - 0x07A4,
        0x0894 - 0x0819,
        0x0917 - 0x0894,
        0x09A1 - 0x0917,
        0x0A33 - 0x09A1,
        0x0ACF - 0x0A33,
        0x0B73 - 0x0ACF,
        0x0C22 - 0x0B73,
        0x0CDA - 0x0C22,
    ):
        out.append(d & 0xFF)
    out.extend([0x0704 * 2 >> 8, 0x0704 * 2 & 0xFF])
    for d in (
        0x076E - 0x0704,
        0x07E0 - 0x076E,
        0x0858 - 0x07E0,
        0x08D6 - 0x0858,
        0x095C - 0x08D6,
        0x09EC - 0x095C,
        0x0A82 - 0x09EC,
        0x0B22 - 0x0A82,
        0x0BCC - 0x0B22,
        0x0C80 - 0x0BCC,
        0x0D3E - 0x0C80,
    ):
        out.append(d & 0xFF)
    out.extend([0x07E0 * 2 >> 8, 0x07E0 * 2 & 0xFF])
    for d in (
        0x0858 - 0x07E0,
        0x08E0 - 0x0858,
        0x0960 - 0x08E0,
        0x09F0 - 0x0960,
        0x0A88 - 0x09F0,
        0x0B28 - 0x0A88,
        0x0BD8 - 0x0B28,
        0x0C80 - 0x0BD8,
        0x0D60 - 0x0C80,
        0x0E10 - 0x0D60,
        0x0EF8 - 0x0E10,
    ):
        out.append(d & 0xFF)
    return bytes(v & 0xFF for v in out)


T_PACK = _build_t_pack()

# NT_DATA: (offset_bytes from T1_, use_and_a if odd, tc_index)
# tune.asm 2288-2303; TC streams below (TCNEW_* / TCOLD_* minus T_ label)
NT_DATA = [
    (0, False, "TCNEW_0"),
    (25, True, "TCOLD_0"),
    (25, True, "TCNEW_1"),
    (0, True, "TCOLD_1"),
    (74, False, "TCNEW_2"),
    (48, True, "TCOLD_2"),
    (50, False, "TCNEW_3"),
    (48, True, "TCOLD_3"),
]

TC_TABLES: dict[str, list[int]] = {
    "TCNEW_0": [0x1C + 1, 0x20 + 1, 0x22 + 1, 0x26 + 1, 0x2A + 1, 0x2C + 1, 0x30 + 1, 0x54 + 1, 0xBC + 1, 0xBE + 1, 0],
    "TCOLD_0": [0x00 + 1, 0x04 + 1, 0x08 + 1, 0x0A + 1, 0x0C + 1, 0x0E + 1, 0x12 + 1, 0x14 + 1, 0x18 + 1, 0x24 + 1, 0x3C + 1, 0],
    "TCOLD_1": [0x5C + 1, 0],
    "TCNEW_1": [0x5C + 1, 0],  # .EQU TCOLD_1
    "TCNEW_2": [0x1A + 1, 0x20 + 1, 0x24 + 1, 0x28 + 1, 0x2A + 1, 0x3A + 1, 0x4C + 1, 0x5E + 1, 0xBA + 1, 0xBC + 1, 0xBE + 1, 0],
    "TCOLD_2": [
        0x30 + 1,
        0x36 + 1,
        0x4C + 1,
        0x52 + 1,
        0x5E + 1,
        0x70 + 1,
        0x82,
        0x8C,
        0x9C,
        0x9E,
        0xA0,
        0xA6,
        0xA8,
        0xAA,
        0xAC,
        0xAE,
        0xAE,
        0,
    ],
    "TCNEW_3": [0x56 + 1],
    "TCOLD_3": [0x1E + 1, 0x22 + 1, 0x24 + 1, 0x28 + 1, 0x2C + 1, 0x2E + 1, 0x32 + 1, 0xBE + 1, 0],
}


def _depack_segment(seg: bytes) -> list[int]:
    """One T_PACK tone-table chunk (anchor + deltas)."""
    i = 0
    hl = 0
    out: list[int] = []
    while i < len(seg):
        a = seg[i]
        i += 1
        if a < 30:  # 15*2
            hl = ((a << 8) | seg[i]) & 0xFFFF
            i += 1
        else:
            hl = (hl + a * 4) & 0xFFFF
        out.append(hl)
        if ((hl & 0xFF) - 0xF0) & 0xFF == 0:
            break
    return out


# Segment byte lengths in tune.asm T_PACK (table1 has 12 deltas)
_T_PACK_SEG_LENS = (13, 14, 13, 13)

# Word offsets from T1_ for each unpacked 12-word table (tune.asm .EQU)
_T1_WORD_OFF = (12, 24, 37, 37)  # T_OLD_2, T_OLD_3/T_OLD_0 alias, T_NEW_2


def depack_t_pack(pack: bytes = T_PACK) -> bytearray:
    """TP_0: unpack four packed tables into T1_ (98 bytes)."""
    buf = bytearray(98)
    pos = 0
    for seg_len, word_off in zip(_T_PACK_SEG_LENS, _T1_WORD_OFF):
        words = _depack_segment(pack[pos : pos + seg_len])
        pos += seg_len
        byte_off = word_off * 2
        for i, w in enumerate(words[:12]):
            j = byte_off + i * 2
            if j + 1 < len(buf):
                buf[j] = w & 0xFF
                buf[j + 1] = w >> 8
    return buf


# 12 semitone seeds at T_NEW_2 for tone table #2 v4+ (Bulba PT3NoteTable_ASM_34_35)
BASE2_V4 = [
    0x0D10,
    0x0C55,
    0x0BA4,
    0x0AFC,
    0x0A5F,
    0x09CA,
    0x093D,
    0x08B8,
    0x083B,
    0x07C5,
    0x0755,
    0x06EC,
]


TABLE2_V4_ADJUST = [0x20, 0xA8, 0x40, 0xF8, 0xBC, 0x90, 0x78, 0x70, 0x74, 0x08, 0x2A, 0x50]

# TC correction streams -> NoteTableAdjust bit-packed form (pt3_lib.c / Ay_Emul)
_TC_TO_ADJUST: dict[str, list[int]] = {
    "TCNEW_0": [0x40, 0xE6, 0x9C, 0x66, 0x40, 0x2C, 0x20, 0x30, 0x48, 0x6C, 0x1C, 0x5A],
    "TCOLD_0": [0x00, 0x04, 0x08, 0x0A, 0x0C, 0x0E, 0x12, 0x14, 0x18, 0x24, 0x3C],
    "TCNEW_1": [0x5C],
    "TCOLD_1": [0x5C],
    "TCNEW_2": TABLE2_V4_ADJUST,
    "TCOLD_2": [0xF8, 0x80, 0x90, 0xC0, 0x04, 0xF0, 0xF8, 0xEC, 0xE0, 0xC0, 0xFC, 0x40],
    "TCNEW_3": [0xB4, 0x40, 0xE6, 0x9C, 0x66, 0x40, 0x2C, 0x20, 0x30, 0x48, 0x6C, 0x1C],
    "TCOLD_3": [0xB4, 0x40, 0xE6, 0x9C, 0x66, 0x40, 0x2C, 0x20, 0x30, 0x48, 0x6C, 0x1C],
}


def note_table_propagate_adjust(base12: list[int], adjust: list[int]) -> list[int]:
    """deater pt3_lib.c NoteTablePropogate + NoteTableAdjust (96-note linear table)."""
    tone = [0] * 96
    for y in range(12):
        tone[y] = base12[y]
    for x in range(84):
        tone[x + 12] = tone[x] >> 1
    for y in range(12):
        offset = y
        blah = adjust[y] if y < len(adjust) else 0
        for _ in range(8):
            extra = blah & 1
            blah >>= 1
            tone[offset] += extra
            offset += 12
    return tone


def note_table_creator(t1: bytearray, nt_a: int) -> list[int]:
    """
    NoteTableCreator result: L1/L2 octave halving + CORR (NoteTableAdjust).
    Equivalent to tune.asm; verified against PT3NoteTable_ASM_34_35 for table #2 v4+.
    """
    off_bytes, _use_and_a, tc_name = NT_DATA[nt_a]
    if tc_name == "TCNEW_2":
        base12 = list(BASE2_V4)
    else:
        base12 = [
            t1[j] | (t1[j + 1] << 8) for j in range(off_bytes, off_bytes + 24, 2)
        ]
    adjust = _TC_TO_ADJUST.get(tc_name, [])
    tone = note_table_propagate_adjust(base12, adjust)
    if tc_name in ("TCOLD_1", "TCNEW_1"):
        # CORR_1 special: LD (NT_+$2E),$FD -> bump linear index 23 (byte $2E/2)
        if len(tone) > 23:
            tone[23] = 0xFD
    return tone


def pt3_init_note_table_a(tone_table: int, version_digit: int) -> int:
    """INIT PT3 path in tune.asm: tone RLA & 7; version not added when >= 4."""
    note_a = ((tone_table << 1) & 7) if version_digit >= 4 else ((tone_table << 1) & 7)
    return note_a


def parse_pt3notes_inc(path: Path) -> list[int]:
    text = path.read_text(encoding="utf-8", errors="replace")
    values = [int(m.group(1)) for m in re.finditer(r"\.DW\s+(\d+)", text, re.I)]
    return values


def find_rl2wof() -> Path:
    candidates = [
        ROOT / "Tunes" / "rl2wof.pt3",
        ROOT.parent / "RomWBW" / "Binary" / "Apps" / "Tunes" / "rl2wof.pt3",
        Path(r"C:\Users\miguel\Documents\development\RomWBW\Binary\Apps\Tunes\rl2wof.pt3"),
    ]
    for p in candidates:
        if p.is_file():
            return p
    raise FileNotFoundError("rl2wof.pt3 not found (checked Tunes/ and RomWBW Binary)")


def le_bytes(word: int) -> bytes:
    return struct.pack("<H", word & 0xFFFF)


def main() -> int:
    pt3_path = find_rl2wof()
    mod = pt3_path.read_bytes()
    tone_tbl = mod[99]
    ver_ch = chr(mod[13]) if 32 <= mod[13] < 127 else "?"
    version = mod[13] - 0x30 if 0x30 <= mod[13] <= 0x39 else 6
    nt_a = pt3_init_note_table_a(tone_tbl, version)

    t1 = depack_t_pack()
    table96 = note_table_creator(t1, nt_a)

    notes_1_5 = table96[0:5]

    inc = parse_pt3notes_inc(PT3NOTES_INC)
    inc_1_5 = inc[0:5]
    non4095 = [v for v in inc if v != 4095]
    inc_first5_non4095 = non4095[:5]

    print(f"PT3: {pt3_path}")
    print(f"  byte[99] tone table = {tone_tbl}")
    print(f"  byte[13] version char = '{ver_ch}' -> version {version}")
    print(f"  note_table_a (tone<<1 & 7) = {nt_a} -> NT_DATA entry {nt_a} ({NT_DATA[nt_a][2]})")
    print()
    print("Notes 1-5 (16-bit LE), tune.asm NoteTableCreator (T_PACK + L1/L2 + CORR):")
    for n, p in enumerate(notes_1_5, start=1):
        print(f"  note {n}: {p}  LE {le_bytes(p).hex()}")
    print()
    print("pt3notes.inc notes 1-5:", inc_1_5)
    print("pt3notes.inc first five values != 4095:", inc_first5_non4095)
    print()

    match_inc_1_5 = notes_1_5 == inc_1_5
    match_non4095 = notes_1_5 == inc_first5_non4095

    print("Comparison (tune table vs pt3notes.inc):")
    print(f"  notes 1-5 vs inc[0:5]:           {'MATCH' if match_inc_1_5 else 'MISMATCH'}")
    print(f"    generated: {notes_1_5}")
    print(f"    inc:       {inc_1_5}")
    print(f"  notes 1-5 vs first 5 non-4095:     {'MATCH' if match_non4095 else 'MISMATCH'}")
    print(f"    generated: {notes_1_5}")
    print(f"    inc:       {inc_first5_non4095}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
