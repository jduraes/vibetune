# VibeTune Handover

**Build under test: v0.0.109 (11-Jun-2026)** — *startup pops fixed with pre-mute at probe and engine init.*

## Goal

RomWBW CP/M player (`vtune.com`) for PT2/PT3/MYM on real hardware (SC126 / RCZ180 EB, ZPM3, F3:), comparable to `tune.com`. Bulba engine; clean-room engine abandoned.

## State

- **Play path works** on target: `vtune rl2wof` / `vtune rl2wofts` load, **timer mode**, play audibly. Bare `vtune` → usage, no pop.
- **`-delay` CLI:** `vtune rl2wof -delay` now enters delay mode again on hardware. The earlier false-positive and always-delay regressions were fixed by restoring tune-like `CLIARGS` substring handling.
- **Remaining delay defect:** delay mode is still too fast, channels drift out of sync, and the output gets fuzzy/noise-like. Leave this untouched for now.
- **Hardware verification (v0.0.108):**
  - **SC126 / RCZ180 EB:** `vtune rl2wof` → timer mode; `vtune rl2wof -delay` → delay mode. Confirms explicit `-delay` flag works correctly.
  - **RC2014 / RCZ80:** Auto-fallback to delay mode (no timer hardware). Delay mode playback tested and working.
- Build: `Build.cmd` → TASM `-t80 -g3 -fFF -dWBW` → `vtune.com` → `..\RomWBW\Binary\Apps\vtune.com`.
- Deploy: ZPM3 **ZMD** v1.50, `zmd r`, **Y-Modem** (1024-byte CRC blocks) — not ZMODEM.
- Version: `vtversion.inc` (`v{version}, {date}` banner).

## Engine

- **Bulba PT2/PT3** from RomWBW `tune.asm`: `pt3bulba.inc` + `pt3bulba_shim.inc`. Init `CALL BULBA_START`; tick `CALL BULBA_START+5`.
- **Do not** reintroduce clean-room engine (`archive/cleanroom/`).

## Three fixes that made playback work (do not regress)

1. **Flat `.COM` image** — no `.DS` in emitted region; file ends at `HEAP` (like `tune.com`). `.DS` between code shifts load addresses → silent crash.
2. **`HEAP_CLEAR` starts at `VARS`, not `HEAP`** — preserves parse buffers / `BULBA_PORTS` below `VARS`.
3. **`PROBETIMER` uses `RET Z`** (not `RET NZ`) — nonzero HBIOS tick ⇒ timer mode (`timing.inc`).

## CLI / `-delay` (v0.0.104)

**Model (match tune.com):**

- Filename from CP/M default **FCB (`$5C`)**; tail token walk for trailing switches when FCB holds the name (`PARSE_SWITCH_ONLY`).
- **`-DELAY` only via `APPLY_DELAY_FROM_CMDLINE`:** after banner, **before any BDOS/file I/O** — same timing as tune `CLI_HAVE_DELAY_SWITCH`.
- **`GET_CMD_TAIL_LEN`** → **`STRINDEX_TUNE_LEN`** on `-DELAY` (case-sensitive, tune `STRCMP`); **clear `DELAYMD` then set** if found.
- Other switches (`-list`, `-msx`, …): **`SCAN_CMDLINE_SWITCHES`** with **bounded** tail (`STRINDEX_UP_LEN`); FCB name field for leading switches only.
- Token walk **`APPLY_IF_SWITCH`** remains backup for `-delay` / `-list` in tail.

**Symbols:** `DELAYMD`, `WMOD`, `PROBETIMER`, `PRINT_TIMING_MODE` in `timing.inc` / `vibetune.asm`.

## Hardware / ports / timing

- Ports: HBIOS `BF_SNDQUERY` + probe + CLI (`-msx -rc -coleco -eb`). EB on target: **$68/$60**.
- `-delay` → `DELAYMD=1` → delay loop; else timer when HBIOS timer live.
- **Auto-selection matches `tune.com`:** `PROBETIMER` does a live HBIOS `BF_SYSGET` / `$D0` tick probe. If `DE:HL == 0`, stay in delay mode; if nonzero, use timer mode. This is not a hardcoded SC126/RC2014 split, so RC2014/RCZ80 systems without a live timer should fall back to delay automatically.

## Audio policy (keep)

- **Pause (v0.0.108):** Zeros amplitude registers (8-10) + mixer reg 7 = `$3F` for complete 100% note mute. Resume continues from engine's exact internal position.
- Exit mute via ROUT only when `PSG_TOUCHED`.

## SIMH (non-hardware)

- `Sync-SIMH-Content.ps1 -Force` → `Run-SIMH-VibeTune.cmd` → boot **`2.4`** (not `2`, not `3.x`); image **`hd1k_combo.img`**.
- Piped keystrokes into SIMH on Windows unreliable; use interactive boot. See `docs/SIMH-Testing.md`.

## Known problems (user-reported, still open)

| Issue | Notes |
|-------|--------|
| **Startup pop(s)** | **v0.0.109 fix:** Added PSG_MUTE_DIRECT after hardware probe and before ENGINE_INIT to clean PSG state. Testing on hardware. |
| **Pause sustain** | **v0.0.108 verified:** Pause cuts all notes cleanly (zeros amplitude regs 8-10 + mixer). Resume continues from exact engine state—no skipped notes. ✓ |
| **Loop toggle (`l`)** | Deferred status OK v0.0.89; host `LOOP_MODE` only at end-of-track. |
| **PT3 metadata** | Need full header print (song name `$1E`, author `$42`, etc.); see `docs/PT3FormatSpec.md`. |
| **Delay mode too fast / fuzzy** | `-delay` now selects the right mode, but playback timing is still wrong in delay mode: too fast, channels drift, sound becomes fuzzy/noise-like. |
| **`-list` stub** | Scan + print + exit; exit whine (no `PSG_TOUCHED`); no pick-to-play yet. |

## `-delay` parser history (compressed)

| Build | Result | Cause / note |
|-------|--------|----------------|
| v0.0.91 | Multiple-args error | Token parser rejected trailing `-delay`. |
| v0.0.92–95 | Regressions | CR tail, stack clobber, `B` zeroed, fragile token loop. |
| v0.0.96–99 | Play OK; `-delay` wrong | FCB-first parse; tail scan / token issues. |
| v0.0.100–102 | Timer with `-delay` | Scan missed switch (incl. `$5C-$FF` abort on FCB NULs in v0.0.101). |
| v0.0.103 | **Always delay** | Unbounded scan past empty `$81` into stale buffer; `DELAYMD` never cleared; `DELAYMD_SAVED` sticky. |
| v0.0.104 | **Fix attempt** | Bounded tail only; clear-then-set; one `-DELAY` check after banner; no `DELAYMD_SAVED`. |
| v0.0.105–107 | **Fix verified** | Restored tune-like `CLIARGS` substring handling; hardware now shows timer mode without `-delay` and delay mode with `-delay`. |

## Rejected / do not reintroduce

- **Unbounded `$81` scan** when `$0080=0` — false `-DELAY` in page-zero garbage → always delay (v0.0.103).
- **Full page-zero scan `$5C–$FF`** for switches — FCB interior NUL aborts before `$81` (v0.0.101).
- **`DELAYMD_SAVED` sticky restore** across parse — preserves false positives.
- **Setting `DELAYMD` without clearing first** in final delay path.
- **v0.0.90** pop/whine patches — hung after banner on hardware.
- **Clean-room engine**, `.DS` in emitted COM, `HEAP_CLEAR` from `HEAP`, inverted `PROBETIMER` test.
- **New brittle SIMH automation** (long waits, piped boot) — use existing scripts only.
- **Standalone `hd1k_zpm3.img` attach** — HALT; use combo + `2.4`.

## Decisions

- Reference player: `..\RomWBW\Source\Apps\Tune\tune.asm` (`cli.inc` / `strings.inc` for `-DELAY`).
- Config: `vtunecfg.com` → `VTUNE.CFG`; no `-config` in player.
- TurboSound: auto from file; no user toggle.
- Versioning: `x.y.z`; bump **z** every build, **y** after successful test + commit.
- **This file** is the single live handoff — update in place, no new dated root handoffs.

## Open / verify

- [x] **Hardware v0.0.108:** SC126/RCZ180 EB and RC2014/RCZ80 tested; pause/resume flawless; timer mode on SC126 without `-delay`; auto-fallback to delay mode on RC2014 (no timer hardware); explicit `-delay` flag works.
- [ ] **Hardware v0.0.109:** Startup pops fixed; needs testing to verify pops eliminated.
- [ ] Fix delay-mode speed/sync/fuzz.
- [ ] Pop / `-list` whine (safer than v0.0.90).
- [ ] PT3 metadata; interactive `-list`; TurboSound (`rl2wofts`).

## Key files

| File | Role |
|------|------|
| `vibetune.asm` | Startup, CLI parse, HW detect, load, loop, `-DELAY` logic |
| `pt3bulba.inc` / `pt3bulba_shim.inc` | Bulba player + glue |
| `timing.inc` | `WAITQ`, `PROBETIMER`, `QDLY` |
| `vtversion.inc` | Version / date |
| `Build.cmd` | Build + copy to RomWBW |
| `Run-SIMH-VibeTune.cmd`, `Sync-SIMH-Content.ps1` | SIMH sync + launch |
| `Validate-StructureChecks.ps1` | Format validation harness |
| `HANDOVER.md` | This file |
| `docs/PT3FormatSpec.md`, `docs/SIMH-Testing.md` | Specs |

Reference: `..\RomWBW\Source\Apps\Tune\tune.asm`.
