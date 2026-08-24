# TurboSound module (dual-AVR) support — design and plan

Branch: `turbosound-module`

## Goal

Support **two TurboSound hardware topologies**, user-selectable:

1. **Dual card** (existing): two real AY/YM chips on two port pairs
   (current `TS_DUALHW` path, e.g. $A0/$A1 + $A2/$A3). Unchanged.
2. **Single card + dual-AVR TurboSound module** (new): one port pair
   (MSX standard $A0/$A1); the two emulated chips are selected by writing
   the latch codes **0xFF (chip 0) / 0xFE (chip 1)** to the RSEL port,
   exactly like a ZX TurboSound. Selection is sticky until changed.

## Why a mode, not auto-detection

The AVR module never drives the data bus (Hi-Z on reads by design — see the
RCA session `2026-08-24-turbosound-avr-latency-rca`). Port readback probing
sees a floating bus, so reliable software auto-detect of module-vs-real-chip
is not possible. The user tells VibeTune which topology is installed:

- `vtunecfg.com` menu option (persistent, VTUNE.CFG), and/or
- a CLI switch for one-shot override.

## Architecture impact (small by design)

The existing TS engine already does the hard part: two Bulba contexts
(`TS_LOAD_CTX1/2`, `TS_SAVE_CTX1/2`) played per tick by `TS_PLAYQUARK`, with
`TS_SETPORTS1/2` pointing the port globals at each chip. For the module:

- `TS_SETPORTS1` → set port pair to the *same* $A0/$A1, then
  `TSMOD_SELECT(0xFF)` (latch RSEL with 0xFF).
- `TS_SETPORTS2` → same port pair, then `TSMOD_SELECT(0xFE)`.

So module support = a topology flag + a select routine called at the
setports points + mute/init adjustments. No engine changes.

### New/changed pieces

| Piece | Change |
|---|---|
| `AUDIO_OUT_MODE` | keep; topology is orthogonal (`TS_TOPOLOGY` var: 0=dual-card, 1=module) |
| `TSMOD_SELECT` | new: latch 0xFF/0xFE to RSEL, with the same slow-I/O wrapping as `PSG_WRITE_REG` |
| `TS_PLAYQUARK` | branch on `TS_TOPOLOGY`: module → `TSMOD_SETPORTS1/2` (same pair + select code) |
| `TS_INIT`, `TS_MUTE_BOTH_HARDWARE` | module: select chip, mute, select other chip, mute — one port pair |
| Detection (`TS_PORTS_SETUP`) | module skips dual-pair probing; single pair only |
| `vtunecfg.asm` | new menu item: TS hardware = auto / dual-card / module |
| VTUNE.CFG | extend with byte 5 = TS topology (0=auto, 1=dual-card, 2=module); missing byte → auto (current behavior) |

## Timing constraints from the AVR side (hard requirements)

Established in the RCA session:

- Strobes must be ≥ ~380ns: RomWBW `Z180_IOWAIT=3` covers this system-wide
  (already flashed on the SC126).
- The AVR services each latch/write in an ISR (~1µs); back-to-back OUTs must
  leave it room. The select-code latch is an extra OUT per chip switch —
  two extra per tick — negligible at 50Hz frame rate, but each OUT must keep
  the slow-I/O delay (`PSG_IO_SLOW_ENTER/EXIT` or the module-equivalent).
- After the 0xFF/0xFE select latch, the *next* latch is the register number —
  no extra settling needed beyond the standard inter-access delay.

## vtunecfg config byte (proposal)

```
byte 0: magic = 0xA5
byte 1: DISP_MODE
byte 2: flags (bit0 ANSI colour)
byte 3: rows
byte 4: cols
byte 5: TS topology (0=auto, 1=dual-card, 2=single-card module)  [NEW]
```

Old 5-byte CFG files simply leave topology = auto. vtunecfg writes 6 bytes.

## Work items

1. [x] `tsmodule.inc`: `TSMOD_SELECT` / `TSMOD_MUTE` (done, v0.0.193)
2. [x] vibetune.asm: `TS_TOPOLOGY`, CFG byte-5 load, branches in TS_SETPORTS1/2/TS_MUTE/TS_PORTS_SETUP (done, v0.0.193)
3. [x] CLI switch `-tsm` (done, v0.0.193)
4. [x] vtunecfg.asm: TS hardware prompt + byte-5 load/save (done, v0.0.193)
5. [ ] Hardware test on SC126: `vtune RL2WOFTS.PT3` on the module (both AVRs),
      regression: dual-card unchanged
6. [ ] README.md systems table update

## Out of scope

- 3-ch AVR firmware (needs a board revision; see the RCA session's
  `firmware/3CH-DESIGN.md`). Module stays 2-ch; A/B fold-down mix.
