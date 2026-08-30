# VibeTune

[2026-08-26]

A music player for RomWBW CP/M on real Z80-family hardware. `vtune.com` plays
PT2 and PT3 sound files on AY-3-8910 / YM2149-class sound chips, with a
terminal-aware playlist UI and dual-chip TurboSound support (two real cards or
a dual-AVR module). Optional MYM support is available via `Build.cmd MYM`
(`mymeng.inc`). `vtunecfg.com` configures display, AY cards, and TurboSound
topology.

Current release: **v0.0.228**.

## Features

- **Formats:** PT2, PT3 (including Vortex Tracker II variants); MYM when built
  with `-dMYM`. Detected from the file extension. PT3 metadata (title/author)
  shown while playing.
- **Playlist mode (`-list`):** scans the current drive/user area for up to
  128 tracks, navigable while playing, with loop-track and loop-playlist
  modes (`-loop`), pause/resume, Enter to play the selection, next/previous,
  redraw and a guarded delete-file flow.
- **ANSI/VT100 UI (`ansiui.inc`):** with an ANSI-capable terminal the playlist
  renders as a multi-column tiled view with cursor-key navigation; plain
  terminals get a simple scrolling list. Status shows input mode (Regular /
  TurboSound), song metadata, hardware line, playlist position, play/pause
  state and loop status.
- **Display config:** terminal type (plain/VT100/ANSI), ANSI colour and
  visible screen size (up to 150×50) via `vtunecfg.com`, persisted in
  `VTUNE.CFG`.
- **Hardware config:** `vtunecfg` also sets primary AY card, TurboSound mode
  (auto / dual-card / dual-AVR module), and optional second card for
  dual-card. CFG is the default; CLI switches override it.
- **Timing:** uses the HBIOS hardware timer when available, otherwise a
  CPU-calibrated delay loop (`-delay` forces delay mode).

## Supported systems and sound cards

Auto-detection uses the HBIOS platform ID and port probing; CLI switches
(and CFG card bytes) override it:

| Switch    | Card / platform                              | Ports (reg/data) |
|-----------|----------------------------------------------|------------------|
| (auto)    | HBIOS-reported AY ports                      | platform default |
| `-msx`    | MSX standard                                 | `$A0 / $A1`      |
| `-rc`     | RC2014 standard                              | `$D8 / $D0`      |
| `-coleco` | Coleco                                       | `$50 / $51`      |
| `-eb`     | Ed Brindley (EB) sound module                | platform variant |

The EB module is addressed `$D8/$D0` on Z80 platforms (RC2014 / RCZ80) and
`$68/$60` on Z180 platforms (SC126 / RCZ180). On Z180, ports that fall inside
the internal I/O window (e.g. RC `$D0`/`$D8`) are rejected with guidance
toward EB or MSX/Coleco as appropriate. Tested on SC126 and RC2014.

Rev 5 EB cards use a real YM2149; Rev 6.x can host a dual-AVR TurboSound
module (see `docs/TurboSound-AVR-EB-Rev6.md`).

## TurboSound

Packed dual-module PT3 TurboSound files use two AY contexts per tick.

**Dual-card:** two real chips on two port pairs. Set both cards in
`vtunecfg` (RomWBW does not enumerate a second AY). CLI cannot alone pick
arbitrary second-card ports.

**Dual-AVR module (`-tsm` / CFG module):** one port pair; chips are selected
by latching `0xFF` / `0xFE` on the register-select port (ZX TurboSound style).
In auto mode, after dual-card probing fails, Hi-Z readback on the play ports
selects module topology without requiring `-tsm`. A TS file on a single
readable AY plays chip 1 only (UI: Single-Card mode).

Single-chip files play unchanged on any of these configurations.

Details: `docs/TurboSound-Module.md`.

## Usage

```
VTUNE [switches] file[.pt3|.pt2|.mym]
Valid switches: -help -credits -list -loop -delay -msx -rc -coleco -eb -tsm
```

Playlist keys: `Esc` quit, `Space` pause, `Enter` play selection (clears
pause), `N`/`P` next/previous track, `WASD`/arrow keys navigate, `R` redraw,
`l`/`L` loop track/playlist, `DEL` ×3 delete selected file.

```
VTUNECFG                      interactive config (AY cards, TS, display)
VTUNECFG plain|vt100|ansi     one-shot terminal type change
VTUNECFG show                 print current settings
VTUNECFG verify               write/read/compare CFG round-trip
```

## Building

```
Build.cmd        (TASM on Windows)
make             (uz80as via RomWBW Tools/Makefile.inc)
```

Both assemblers produce byte-identical binaries when sources stay
dual-assembler clean (`#IF`/`#ENDIF`, explicit `(IX+0)` displacements).
Outputs are copied to `../RomWBW/Binary/Apps/`.

Key sources: `vibetune.asm`, `ansiui.inc`, `pt3bulba.inc` /
`pt3shim.inc`, `timing.inc`, `tsmodule.inc`, `vtunecfg.asm`,
`vtver.inc`.

## Docs

| Doc | Topic |
|-----|--------|
| `docs/TurboSound-Module.md` | Dual-card vs dual-AVR module topology |
| `docs/TurboSound-AVR-EB-Rev6.md` | AVR module on EB Rev6 addressing |
| `docs/SC126-Forced-Delay.md` | SC126 forced-delay timing |
| `docs/SIMH-Testing.md` | SIMH / telnet test workflow |
| `docs/PT3FormatSpec.md` | PT3 format notes |

## Attribution and license

VibeTune is free software under the **GNU GPL v3** (or later) — see
`LICENSE`.

- **VibeTune** — Copyright (C) 2026, Joao Miguel Duraes (fackie)
- Derived from **RomWBW `tune.com`** — Copyright (C) 2026, Wayne Warthen,
  GNU GPL v3
- **PTxPlayer** (Universal PT2/PT3 player) — Copyright (C) 2004-2007,
  S.V.Bulba
- **MYMPlay 0.4** — Marq/Lieves!Tuore
