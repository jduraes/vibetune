# VibeTune

A music player for RomWBW CP/M on real Z80-family hardware. `vtune.com` plays
PT2, PT3 and MYM sound files on AY-3-8910 / YM2149-class sound chips, with a
terminal-aware playlist UI and dual-chip TurboSound support.
`vtunecfg.com` configures the display mode.

## Features

- **Formats:** PT2, PT3 (including Vortex Tracker II variants) and MYM,
  detected from the file extension. PT3 metadata (title/author) shown while
  playing.
- **Playlist mode (`-list`):** scans the current drive/user area for up to
  128 tracks, navigable while playing, with loop-track and loop-playlist
  modes (`-loop`), pause/resume, next/previous, redraw and a guarded
  delete-file flow.
- **ANSI/VT100 UI:** with an ANSI-capable terminal the playlist renders as a
  multi-column tiled view with cursor-key navigation; plain terminals get a
  simple scrolling list. Terminal type (plain/VT100/ANSI), ANSI colour and
  visible screen size (up to 150 x 50) are configured interactively with
  `vtunecfg.com` and persisted in `VTUNE.CFG`.
- **Timing:** uses the HBIOS hardware timer when available, otherwise a
  CPU-calibrated delay loop (`-delay` forces delay mode).

## Supported systems and sound cards

Auto-detection uses the HBIOS platform ID and port probing; CLI switches
override it:

| Switch    | Card / platform                              | Ports (reg/data) |
|-----------|----------------------------------------------|------------------|
| (auto)    | HBIOS-reported AY ports                      | platform default |
| `-msx`    | MSX standard                                 | `$A0 / $A1`      |
| `-rc`     | RC2014 standard                              | `$D8 / $D0`      |
| `-coleco` | Coleco                                       | `$50 / $51`      |
| `-eb`     | Ed Brindley (EB) sound module                | platform variant |

The EB (Ed Brindley) module is addressed `$D8/$D0` on Z80 platforms (RC2014 /
RCZ80) and `$68/$60` on Z180 platforms (SC126 / RCZ180). Tested on SC126 and
RC2014; MSX and Coleco mappings are supported via the switches above.

## TurboSound

Packed dual-module PT3 TurboSound files are detected and played through two
AY port sets (chip 1 + chip 2), with per-chip playback contexts and a
dual-chip mute on exit. Single-chip files play unchanged on either
configuration.

## Usage

```
VTUNE [switches] file[.pt3|.pt2|.mym]
Valid switches: -help -credits -list -loop -delay -msx -rc -coleco -eb
```

Playlist keys: `Esc` quit, `Space` pause, `N/P` next/previous track,
`WASD`/arrow keys navigate, `R` redraw, `l/L` loop track/playlist,
`DEL` x3 delete selected file.

```
VTUNECFG                      interactive display configuration
VTUNECFG plain|vt100|ansi     one-shot terminal type change
VTUNECFG show                 print current settings
```

## Building

```
Build.cmd        (TASM on Windows)
make             (uz80as via RomWBW Tools/Makefile.inc)
```

Both assemblers produce byte-identical binaries; outputs are copied to
`../RomWBW/Binary/Apps/`.

## Attribution and license

VibeTune is free software under the **GNU GPL v3** (or later) — see
`LICENSE`.

- **VibeTune** — Copyright (C) 2026, Joao Miguel Duraes (fackie)
- Derived from **RomWBW `tune.com`** — Copyright (C) 2026, Wayne Warthen,
  GNU GPL v3
- **PTxPlayer** (Universal PT2/PT3 player) — Copyright (C) 2004-2007,
  S.V.Bulba
- **MYMPlay 0.4** — Marq/Lieves!Tuore
