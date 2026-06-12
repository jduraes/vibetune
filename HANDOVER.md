# VibeTune Handover

**Build under test: v0.0.148 (12-Jun-2026)** — *TurboSound delay-mode tempo verified **94 BPM** on RC2014 @ 7.3728 MHz (`rl2wofts`); tune.com-equivalent QDLY path + RC2014 VT-overhead trim.*

## Session Delta (2026-06-11 to 2026-06-12)

- Version bumped through **v0.0.148** in `vtversion.inc`.
- **TurboSound runtime (v0.0.131–132):** full dual-chip path (`TS_INIT` / `TS_PLAYQUARK` / `TS_MUTE`), Coleco chip-2 fallback `$50/$51`, `TS_VERIFY_DUAL_PORTS`, per-chip `VT_/NT_` context save, tune-style `TS_INIT` ordering.
- **TurboSound delay tempo (v0.0.136–148):** see postmortem below — **v0.0.148** lands **94 BPM** on RC2014 delay mode for `rl2wofts`.
- **TurboSound exit (v0.0.134–135):** hardware mute before BDOS; `BDOS` fn 0 return; disable Bulba `LoopChecker` (CHECKLP `POP` corrupted stack); `FLUSH_KEYS` on quit.
- **Repo hygiene:** planning docs moved to `archive/docs/`; active specs/scripts under `docs/` and `scripts/`.

Hardware/behavior validation from this session:

- `vtune rl2wof` on RCZ180 EB and MSX: playback good (single-chip baseline).
- `vtune rl2wofts` on MSX (TurboSound): **playback good (v0.0.132+)**; **clean Esc exit (v0.0.135)**.
- **RC2014 / RCZ80 (delay auto-fallback):** `vtune rl2wofts` — **94 BPM verified (v0.0.148)**; QDLY final `0x683` (1667), trim **986** loops after `TS_ADJTIM`.

Immediate next debugging target:

- Optional: remove `PRINT_TIMING_DEBUG` once tempo stable across more tunes/hardware.
- Single-chip delay-mode speed/sync/fuzz (unchanged; separate from TS tempo fix).

## Goal

RomWBW CP/M player (`vtune.com`) for PT2/PT3/MYM on real hardware (SC126 / RCZ180 EB, ZPM3, F3:), comparable to `tune.com`. Bulba engine; clean-room engine abandoned.

## State

- **TurboSound:** auto-detected dual PT3 (`rl2wofts`); prints both chip port sets; MSX + Coleco fallback when HBIOS has no second device query.
- **PT3 playback:** both ProTracker-family and Vortex-family PT3 files now use the Bulba engine. `ATTACK.PT3` was hardware-verified after removing the old Vortex proof stub.
- **`-list` mode:** bare `vtune -list` now scans the current drive, prints the discovered tracks, selects the first one, and enters playlist playback.
- **`-delay` CLI:** `vtune rl2wof -delay` now enters delay mode again on hardware. The earlier false-positive and always-delay regressions were fixed by restoring tune-like `CLIARGS` substring handling.
- **Remaining delay defect:** delay mode is still too fast, channels drift out of sync, and the output gets fuzzy/noise-like. Leave this untouched for now.
- **Hardware verification summary:**
  - **v0.0.113:** SC126 / RCZ180 EB and RC2014 / RCZ80 startup is clean (no pop).
  - **v0.0.114:** temporary startup debug markers used for isolation were removed.
  - **v0.0.108:**
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

- See `docs/SIMH-Testing.md`. Launch scripts were removed from repo root in this session; use RomWBW tooling or restore from git history if needed.
- Boot **`2.4`** (not `2`, not `3.x`); image **`hd1k_combo.img`**.
- Piped keystrokes into SIMH on Windows unreliable; use interactive boot.

## Known problems (user-reported, still open)

| Issue | Notes |
|-------|--------|
| **Startup pop(s)** | **Fixed (v0.0.113, cleanup v0.0.114):** root causes documented below; validated clean on SC126 and RC2014. |
| **Pause sustain** | **v0.0.108 verified:** Pause cuts all notes cleanly (zeros amplitude regs 8-10 + mixer). Resume continues from exact engine state—no skipped notes. ✓ |
| **Loop toggle (`l`)** | Deferred status OK v0.0.89; host `LOOP_MODE` only at end-of-track. |
| **PT3 metadata** | Need full header print (song name `$1E`, author `$42`, etc.); see `docs/PT3FormatSpec.md`. |
| **Delay mode too fast / fuzzy** | Single-chip delay path still open. **TurboSound dual on RC2014 delay:** fixed v0.0.148 (~94 BPM `rl2wofts`, matches tune.com reference). |
| **`-list` playlist UX** | Core behavior works: scan, first-track autostart, `N`/`P` navigation. Keep an eye on metadata/status refresh when switching tracks. |

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

## Startup pop postmortem (v0.0.109-v0.0.113)

Two separate startup-audio defects existed and were fixed in stages.

1. **Pop at/after hardware probe (first defect):**
  - Probe writes `R2=$AA` to test AY presence (`PROBE_AY_PORTS`).
  - Startup PSG state is unknown, so probe activity could leak audible transitions.
  - Initial mitigation in v0.0.109: call `PSG_MUTE_DIRECT` after detection/probe and again before engine init. This reduced startup pops from two to one.

2. **Pop + temporary high-pitched whine around probe success (second defect):**
  - Isolated using single-character startup markers with per-marker pauses on hardware.
  - Repro showed event between probe success (`E`) and post-detect stage (`2`).
  - Root cause: `PROBE_AY_QUIET` wrote mixer register 7 to `$3F` (mute mixer) but wrote amplitude registers 8/9/10 to `$0F` (max volume), not zero.
  - This left channels armed at max amplitude; later register activity could produce a pop/whine burst until full mute/path initialization completed.

Final fix (v0.0.113):
- Change `PROBE_AY_QUIET` amplitude writes for regs 8/9/10 from `$0F` to `$00`.
- Keep post-detection `PSG_MUTE_DIRECT` for full-register safety cleanup.

Validation:
- SC126 (RCZ180 EB): clean startup, no pops.
- RC2014 (RCZ80): clean startup, no pops.

Cleanup (v0.0.114):
- Removed temporary debug marker output and keypress pauses used during isolation.

## PT3 playback postmortem (v0.0.124)

`ATTACK.PT3` exposed that PTx classification and PTx playback were not using the same capability boundary.

- Detection/classification already recognized both ProTracker-family and Vortex-family PT3 headers.
- Only ProTracker-family PT3 files used the Bulba engine.
- Vortex-family PT3 files were still routed into an old proof/stub path that only poked one PSG register, so valid files classified as Vortex appeared silent.

Final fix (v0.0.124):
- Keep the Vortex/ProTracker variant distinction for display only.
- Route both PT3 header families through the same Bulba init/tick path.

Validation:
- `ATTACK.PT3` now plays on hardware.
- `RL2WOF.PT3` and `RL2WOFTS.PT3` continue to play.

## TurboSound postmortem (v0.0.131–v0.0.135)

`RL2WOFTS.PT3` (packed dual PT3) regressed after Bulba integration. Several independent defects stacked:

1. **Wrong chip-2 Coleco ports** — fallback used swapped RSEL/RDAT vs tune/RomWBW (`$50/$51` standard).
2. **Shared `VT_/NT_` workspace** — `TS_PLAYQUARK` switched channel state but not unpacked tables; caused garbled audio until per-chip `TS_CTX1/2_VTNT` save/restore was added.
3. **False dual-hardware** — `TS_DUALHW` could be set without distinct data ports; gated with `TS_VERIFY_DUAL_PORTS`.
4. **Exit stack corruption** — Bulba `LoopChecker` + `CHECKLP` `POP` (loop disabled) removed return addresses during playback; Esc exit then hit spurious parse errors and skipped mute. Fixed by `LoopChecker=0`, mute-before-BDOS, `BDOS` fn 0 exit, `FLUSH_KEYS`.
5. **Exit mute on chip 2** — `PSG_MUTE_DIRECT` only touched MSX `$A0/$A1`; Coleco chip kept sustaining. Fixed with `TS_MUTE_BOTH_HARDWARE` on configured port pairs.

Validation (MSX, delay mode):
- `vtune rl2wofts` — audible dual-chip playback; Esc → silent, single `Exiting.`, clean CCP return (v0.0.135).

## TurboSound delay tempo postmortem (v0.0.136–v0.0.148)

Target: **~94 BPM** for `RL2WOFTS.PT3` on **RC2014 @ 7.3728 MHz** (auto delay mode; HBIOS timer tick = 0). Reference: RomWBW **`tune.com` v3.2b100** reads ~93 BPM on same hardware.

### Root causes (stacked)

1. **WAITQ zero-guard bug (v0.0.141)** — `LD BC,1` ran *before* `JR NZ`, clobbering QDLY every frame → ~1 loop iteration → ~174 BPM. Fixed to match tune.com: test BC first, `LD BC,1` only when QDLY==0.
2. **QDLY apply stacking (v0.0.138)** — `APPLY_ENGINE_QDLY_ADJ` must always derive from HBIOS base in `QDLY0`, not mutate `(QDLY)` in place (was ~163 BPM when stacked).
3. **Failed scale heuristics (v0.0.136–137)** — `×65/94` / divide-first 16-bit routines had overflow and compare bugs (QDLY→0 re-triggered cartoon speed). Abandoned; tune.com path is **one −185**, then **`TS_ADJTIM` (×97/128)** only.
4. **VibeTune vs tune.com play overhead** — At identical post-`TS_ADJTIM` QDLY **2653 (`0A5D`)**, tune.com ≈93 BPM but VibeTune ≈74.5 BPM after WAITQ fix. Cause: per-quark **432-byte `VT_/NT_` LDIR** in `TS_PLAYQUARK` (required for garbled-audio fix); tune.com `TS_PLAYQUARK` calls `PLAY` without that cost.

### Tempo model (why empirical trim was needed)

Frame time ≈ **play_overhead + QDLY × 40 T-states**. BPM is **not** `∝ 1/QDLY`. Inverse-product retuning (`Q_new = Q × BPM_old / BPM_target`) consistently undershot when speeding up; linear extrapolation from early points overshot when pushed too far.

**RC2014 calibration table** (delay, TS dual, after `TS_ADJTIM` baseline 2653):

| Trim | QDLY (hex) | BPM |
|------|------------|-----|
| 0 | 0A5D | 74.5 |
| 528 | 084D | 84 |
| 755 | 076A | 89 |
| 856 | 0705 | 90 |
| **986** | **0683** | **94** ✓ |
| 1084 | 0621 | 97 |

Final trim **986** = bracket between 856→90 and 1084→97: `856 + (94−90) × (228/7)`.

### Pipeline (v0.0.148, `timing.inc`)

1. `INIT_QDLY_BASE`: HBIOS `$F0` → `CPUKHZ`; `QDLY0 = kHz/2` (7372 → 3686 `0E66`).
2. `APPLY_ENGINE_QDLY_ADJ`: from `QDLY0`, subtract **185** (PTX) → `0DAD`; if PTx + TS dual + delay mode: `TS_ADJTIM` → `0A5D`; subtract **986** → **`0x683` (1667)**.
3. `WAITQ`: timer path when `WMOD≠0`; else delay loop (40 T-states/iter, tune.com zero-guard).
4. **`PRINT_TIMING_DEBUG`** (temporary): prints CPU kHz, QDLY0, post-bias, final QDLY, apply count, flags — remove when broader validation done.

### Long-term fix (not done)

Shrink TS per-quark overhead (optimize or reduce `VT_/NT_` context copies) so tune.com QDLY path alone suffices and CPU-specific trim → 0.

Validation (RC2014, delay auto-fallback):
- `vtune rl2wofts` — **94 BPM on the dot (v0.0.148)**; debug line matches table above.

## Rejected / do not reintroduce

- **Unbounded `$81` scan** when `$0080=0` — false `-DELAY` in page-zero garbage → always delay (v0.0.103).
- **Full page-zero scan `$5C–$FF`** for switches — FCB interior NUL aborts before `$81` (v0.0.101).
- **`DELAYMD_SAVED` sticky restore** across parse — preserves false positives.
- **Setting `DELAYMD` without clearing first** in final delay path.
- **v0.0.90** pop/whine patches — hung after banner on hardware.
- **v0.0.136–137 QDLY scale routines** (`×65/94`, broken 16-bit divide) — use tune.com −185 + `TS_ADJTIM` + measured trim only.
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
- [x] **Hardware v0.0.113:** Startup pop fix validated on SC126 and RC2014; no pops.
- [x] **v0.0.114 cleanup:** Removed startup debug markers/pause prompts used for pop isolation.
- [x] **Hardware v0.0.124:** Vortex-family PT3 playback fixed; `ATTACK.PT3` now plays.
- [x] **Hardware v0.0.124:** `vtune -list` scans and autostarts playlist playback; `N`/`P` navigation works.
- [x] **Hardware v0.0.131–135:** TurboSound `rl2wofts` on MSX — playback and clean Esc exit verified.
- [x] **Hardware v0.0.148:** RC2014 delay-mode TurboSound tempo — **94 BPM** on `rl2wofts` (trim 986, QDLY `0x683`).
- [ ] Optional: strip `PRINT_TIMING_DEBUG` after more hardware/tune coverage.
- [ ] Fix delay-mode speed/sync/fuzz (single-chip, non-TS).
- [ ] Pop / `-list` whine (safer than v0.0.90).

## Key files

| File | Role |
|------|------|
| `vibetune.asm` | Startup, CLI parse, HW detect, load, loop, `-DELAY` logic |
| `pt3bulba.inc` / `pt3bulba_shim.inc` | Bulba player + glue |
| `timing.inc` | `WAITQ`, `PROBETIMER`, `QDLY`/`QDLY0`, `TS_ADJTIM`, TS trim, timing debug print |
| `vtversion.inc` | Version / date |
| `Build.cmd` | Build + copy to RomWBW |
| `HANDOVER.md` | This file |
| `docs/PT3FormatSpec.md`, `docs/SIMH-Testing.md` | Active specs |
| `archive/docs/` | Superseded planning docs (see `README.md`) |
| `scripts/Validate-StructureChecks.ps1` | Format validation harness |

Reference: `..\RomWBW\Source\Apps\Tune\tune.asm`.
