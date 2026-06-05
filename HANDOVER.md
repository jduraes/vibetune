# VibeTune Handover

**Build under test: v0.0.87 (05-Jun-2026)** — *first working player.*

## State

- **WORKS on real hardware.** On SC126 / RCZ180 EB (RomWBW HBIOS, ZPM3, drive F
  user 3, AY ports **$68/$60**), `vtune rl2wof` and `vtune rl2wofts` load, report
  `Timing: timer mode`, and **play audibly**, comparable to `tune.com`.
- Banner + usage print correctly on bare `vtune`.
- Build: `Build.cmd` → TASM `-t80 -g3 -fFF -dWBW` → `vtune.com`, then copies to
  `..\RomWBW\Binary\Apps\vtune.com`. Transfer to target via ZMODEM (`zmd r`).
- Version string lives in `vtversion.inc`; banner size tag is `(flat)` to mark
  the flat-image layout (see below).

## Engine

- Uses the **S.V. Bulba PT2/PT3 player** extracted from RomWBW `tune.asm`:
  `pt3bulba.inc` (player) + `pt3bulba_shim.inc` (host glue). Init = `CALL
  BULBA_START`; per-tick = `CALL BULBA_START+5`.
- The **clean-room engine was abandoned** and archived under
  `archive/cleanroom/` with `archive/cleanroom/POSTMORTEM.md`. Do not reintroduce
  it into the build.

## The three fixes that made it work (do not regress)

1. **Flat `.COM` image.** TASM omits `.DS` reserved gaps from the `.com` output;
   any `.DS` *between* code/data shifts every following byte's load address and
   silently crashes startup (no banner). Fix: **no `.DS` in the emitted region.**
   Initialized data (`.DB`/`.DW`) is one contiguous block; then `HEAP .EQU $`;
   then all `.DS` scratch + `VARS` + `MUSIC_BUF`. File ends exactly at `HEAP`,
   like `tune.com`. The shim's `OCTAVEADJ`/`TMP` are `.DB 0` (emitted), not `.DS`.
2. **`HEAP_CLEAR` starts at `VARS`, not `HEAP`.** The parse buffers (`FCB_WORK`,
   `ARG_BUFFER`, …) and `BULBA_PORTS` live below `VARS` and are populated during
   startup; zeroing the whole heap wiped the parsed filename → "unable to read
   input file". Clearing from `VARS` preserves them.
3. **`PROBETIMER` timer test fixed** (`timing.inc`). Was `RET NZ` (inverted),
   which chose delay mode exactly when the HBIOS timer was live. Now `RET Z`:
   nonzero tick count ⇒ timer mode, matching `tune.com`.

## Hardware / ports / timing policy

- Ports come from **RomWBW HBIOS detection** + CLI overrides, not hardcoded EB.
  `DETECT_HARDWARE_CONFIG`: CLI flag → HBIOS `BF_SNDQUERY` → probe → MSX default.
  `BULBA_SYNC_PORTS` copies `PSG_REG_PORT`/`PSG_DATA_PORT` into `BULBA_PORTS`.
- CLI flags: `-msx -rc -coleco -eb -delay -debug -list`.
- Timing: `timing.inc` `PROBETIMER`/`WAITQ`; `-delay` forces the calibrated
  delay loop, otherwise timer mode when a live HBIOS timer is present.

## Audio policy (keep)

- Mixer-only pause (`reg 7 = $3F`) — no pause pops.
- Exit mute via ROUT only when `PSG_TOUCHED`. Never write tone period 0 to mute
  (causes whine). Do not retry "quiet exit without full ROUT".

## Open / next

- [ ] Triage the "many other problems" the user mentioned beyond core playback.
- [ ] Pause/loop state lines (`State: Paused/Playing`, `Loop mode: …`) currently
      echo to console on keypress — review UX vs `tune.com` (which has no pause).
- [ ] Verify TurboSound path (`rl2wofts` reports `Audio mode: TurboSound`).
- [ ] Consolidate older docs (`Handoff-2026-05-3x.md`, design specs) if desired.

## Key files

| File | Role |
|------|------|
| `vibetune.asm` | Main app: startup, HW detect, CLI parse, load, main loop, data+heap |
| `pt3bulba.inc` / `pt3bulba_shim.inc` | Bulba player + host glue |
| `timing.inc` | `WAITQ`, `PROBETIMER` (timer vs delay), `QDLY` |
| `vtversion.inc` | Version / build date / size tag |
| `Build.cmd`, `Clean.cmd`, `Makefile` | Build / clean |
| `scripts/Extract-BulbaPlayer.ps1` | Regenerates `pt3bulba.inc` from `tune.asm` |
| `Run-SIMH-VibeTune*.cmd`, `Sync-SIMH-Content.ps1` | SIMH harness (decode only; no AY audio) |
| `Validate-StructureChecks.ps1` | Structural validation of corpus / invalid vectors |
| `archive/cleanroom/` | Abandoned clean-room engine + `POSTMORTEM.md` |

Reference (working): `..\RomWBW\Source\Apps\Tune\tune.asm`.
