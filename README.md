# VibeTune

[12-Sep-2026]

RomWBW CP/M music player for real Z80-family hardware. `vtune.com` plays PT2
and PT3 (AY-3-8910 / YM2149), with an ANSI playlist UI and TurboSound (two
cards or a dual-AVR module). Default builds omit MYM; `Build.cmd MYM` adds it.
`vtunecfg.com` writes `VTUNE.CFG` (display, AY card, TurboSound topology).

Current release: **v0.1.0**.

## Features

- **Formats:** PT2/PT3 from the file extension (MYM only in a `-dMYM` build).
  PT3 title/author shown while playing.
- **Playlist (`-list`):** up to 128 tracks on the current drive/user. Arrows
  and WASD move the marker; the file reloads after a short settle so holding
  a key does not thrash the disk. `-loop` loops the playlist in `-list`, or
  the single file otherwise. `l`/`L` toggle track/playlist loop at runtime.
- **ANSI UI:** tiled multi-column list, cursor keys, play/pause and loop
  footer. Plain terminals get a scrolling list. Same key path on CP/M 2.2,
  CP/M 3, ZPM3 and ZDOS.
- **Config:** `vtunecfg` sets terminal type, colour, screen size (up to
  150×50), primary AY, TurboSound mode, and optional second card. CFG is the
  default; CLI switches override it.
- **Timing:** HBIOS timer when present; otherwise a calibrated delay loop
  (`-delay` forces delay mode).

## Sound hardware

Auto-detect uses the HBIOS platform ID and port probing. CLI / CFG override:

| Switch    | Card                         | Ports (reg/data) |
|-----------|------------------------------|------------------|
| (auto)    | HBIOS-reported AY            | platform default |
| `-msx`    | MSX standard                 | `$A0 / $A1`      |
| `-rc`     | RC2014 standard              | `$D8 / $D0`      |
| `-coleco` | Coleco                       | `$50 / $51`      |
| `-eb`     | Ed Brindley (EB) module      | `$68 / $60`      |

On Z80 (RC2014 / RCZ80) the EB module is `$D8/$D0`. On Z180 (SC126 / RCZ180)
those ports sit in the internal I/O window and are rejected; use `-eb` or
MSX/Coleco. Tested on SC126 and RC2014.

Rev 5 EB cards use a YM2149; Rev 6.x can host a dual-AVR TurboSound module
(`docs/TurboSound-AVR-EB-Rev6.md`).

## TurboSound

Packed dual-module PT3 files tick two AY contexts.

**Dual-card:** two chips on two port pairs. Set both cards in `vtunecfg`
(RomWBW does not enumerate a second AY).

**Dual-AVR module (`-tsm` / CFG module):** one port pair; chips selected by
latching `0xFF` / `0xFE` on the register port. Must be selected explicitly —
auto never infers the module from Hi-Z (write-only AY clones look the same).
A TS file on a single chip plays chip 1 only (UI: Single-Card).

Single-chip files are unchanged. See `docs/TurboSound-Module.md`.

## Usage

```
VTUNE [switches] file[.pt3|.pt2]
Valid switches: -help -credits -list -loop -delay -msx -rc -coleco -eb -tsm
```

(`file[.mym]` only in a MYM build.)

Playlist keys: `Esc` quit, `Space` pause, `Enter` play the selection (when
paused; loads it if you have been browsing), `N`/`P` next/previous,
`WASD`/arrows navigate, `R` redraw (mute + full UI refresh, no disk),
`l`/`L` loop track/playlist, `DEL` ×3 delete the selected file.

```
VTUNECFG                      interactive setup (term, colour, size, AY, TS)
VTUNECFG plain|vt100|ansi     one-shot terminal type (0|1|2 also accepted)
VTUNECFG show                 print current settings
VTUNECFG verify               write/read/compare CFG round-trip
```

## Building

```
Build.cmd           default (no MYM)
Build.cmd MYM       include MYMPlay
make                uz80as via RomWBW Tools/Makefile.inc
```

Keep sources dual-assembler clean (`#IF`/`#ENDIF`, explicit `(IX+0)`).
Outputs copy to `../RomWBW/Binary/Apps/`.

Key sources: `vibetune.asm`, `ansiui.inc`, `pt3bulba.inc` / `pt3shim.inc`,
`timing.inc`, `tsmodule.inc`, `mymeng.inc` (optional), `vtunecfg.asm`,
`vtver.inc`.

## Docs

| Doc | Topic |
|-----|--------|
| `docs/TurboSound-Module.md` | Dual-card vs dual-AVR module |
| `docs/TurboSound-AVR-EB-Rev6.md` | AVR module on EB Rev6 |
| `docs/SC126-Forced-Delay.md` | SC126 forced-delay timing |
| `docs/SIMH-Testing.md` | SIMH / telnet tests |
| `docs/PT3FormatSpec.md` | PT3 format notes |

## Attribution and license

GNU GPL v3 (or later) — see `LICENSE`.

- **VibeTune** — Copyright (C) 2026, Joao Miguel Duraes (fackie)
- Derived from **RomWBW `tune.com`** — Copyright (C) 2026, Wayne Warthen
- **PTxPlayer** — Copyright (C) 2004-2007, S.V.Bulba
- **MYMPlay 0.4** — Marq/Lieves!Tuore (MYM builds only)
