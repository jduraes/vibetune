# VibeTune Agent Instructions

## Project Context

VibeTune is a RomWBW CP/M music player (`vtune.com`) for PT2/PT3/MYM files on real Z80-family hardware (SC126 / RCZ180 EB, RC2014 / RCZ80, MSX, Coleco). It is written in Z80 assembly (TASM) and is modeled after RomWBW's `tune.com`.

Key source files:
- `vibetune.asm` — startup, CLI parsing, file load, playback loop, hardware detection, TurboSound logic, MYM depack engine (`MYM_INIT`/`MYM_EXTRACT`/`MYM_READBITS`, MYMPlay 0.4 port)
- `pt3bulba.inc` / `pt3bulba_shim.inc` — Bulba PT2/PT3 player engine
- `timing.inc` — `WAITQ`, HBIOS timer probe, QDLY calibration, SC126 forced-delay handling
- `uian.inc` — ANSI/VT100 playlist UI (tiles, metadata, loop status) for `vtune -list`
- `vtunecfg.asm` — display-mode config utility
- `vtversion.inc` — version and build date strings
- `HANDOVER.md` — live session handoff and state log

## Build Command

```cmd
cmd /c Build.cmd
```

This invokes TASM twice (`tasm -t80 -g3 -fFF -dWBW vibetune.asm vtune.com vtune.lst` and
`tasm -t80 -g3 -fFF -dWBW vtunecfg.asm vtunecfg.com vtunecfg.lst`) and copies both
resulting binaries into `../RomWBW/Binary/Apps/`.

## Permanent Rules

### 1. Version and date must advance on every compile of `vibetune.com`

Every time `vibetune.com` is built, increment the **z** component of `VT_VERSION` in `vtversion.inc` and set `VT_BUILD_DATE` to the current system date in **dd-Mmm-yyyy** format (e.g. `17-Jun-2026`).

- Version format is `x.y.z` (e.g. `0.0.154`).
- Increment **z** on every build.
- Increment **y** only after a successful test and an explicit user instruction to do so.
- Increment **x** only on explicit user instruction.
- Do this **before** running `Build.cmd` so the emitted binary reports the new version.

### 2. Update documentation with new findings and steps taken

After any material change — bug fix, feature addition, behavior verification, or build — append a concise entry to `HANDOVER.md` under `## Session Delta` describing:
- What changed
- Why it changed
- Version number
- Any verification performed or outstanding

Keep `HANDOVER.md` as the single live handoff document; do not create new dated root handoffs.

### 3. Do not reintroduce abandoned approaches

- Do not reintroduce the clean-room engine from `archive/cleanroom/`.
- Do not emit `.DS` reservations in the loaded `.COM` image region (they shift load addresses).
- Do not scan command-tail buffers unbounded past terminators.
- Do not change the tuned timing paths (QDLY bias, `TS_ADJTIM`, SC126 scale, empirical trims) without hardware verification evidence.

### 4. Prefer minimal changes

Make the smallest change that achieves the goal. Follow the existing assembly style, register conventions, and comment density. Avoid refactoring unrelated code.

### 5. SIMH runs only on explicit user request

Do not launch SIMH test runs for every build — they are time/credit expensive. Build and static-check changes, then let the user decide whether a SIMH (or hardware) run is needed. Only run SIMH when the user explicitly asks for it.

### 6. Desk-check register flow before building

For any edit touching register conventions (PUSH/POP pairing, EX/EXX swaps, flags across calls), trace the register state on paper before building. This class of bug is desk-checkable — a test run is not a substitute (see v0.0.180: wrong register pushed in `PRINT_HARDWARE_CONFIG`, caught by the user, not by testing).
