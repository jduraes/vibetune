31-May-2026

# VibeTune Implementation Plan

This document captures the working implementation plan and current milestone status for VibeTune.

## 1. Objectives

1.1 Rebuild VibeTune as a clean-room successor to tune.com for RomWBW.
1.2 Reuse RomWBW build infrastructure and shared include files via relative references.
1.3 Deliver in small milestones with testable checkpoints.
1.4 Keep initial playback scope to PTx and MYM. VGM remains deferred.
1.5 Keep configuration authoring outside vtune.com via vtunecfg.com.

## 2. Permanent Rules

2.1 Versioning is agent-managed, not build-managed.
2.2 Start at v0.0.1.
2.3 Increment z on every compile/build.
2.4 Increment y only after successful test plus commit.
2.5 Increment x only on explicit user instruction.
2.6 Banner always includes current OS date in dd-Mmm-yyyy format.
2.7 Banner format: VibeTune Player for RomWBW v{version}, {date}.
2.8 Use RomWBW tools/includes through relative paths where practical.
2.9 Do not implement -config in vtune.com; vtunecfg.com owns config writing.

## 3. Milestones and Status

3.1 Milestone 0 - Build bootstrap
3.1.1 Status: Complete.
3.1.2 Evidence: Build.cmd compiles and copies vtune.com through RomWBW toolchain.

3.2 Milestone 0A - Playback skeleton documentation gate
3.2.1 Status: Complete.
3.2.2 Evidence: PlaybackEngineSkeleton.md in place and maintained.

3.3 Milestone 1 - Display bootstrap
3.3.1 Status: Complete.
3.3.2 Implemented: DISP_MODE, startup display flow, deterministic fallback behavior.

3.4 Milestone 1A - External display config contract
3.4.1 Status: Complete.
3.4.2 Implemented: VTUNE.CFG binary contract with magic 0xA5 and mode byte.

3.5 Milestone 2 - Single-file CLI and parser/classification
3.5.1 Status: Complete.
3.5.2 Implemented: single-argument parse, quoted token handling, extension classification, file preflight.

3.6 Milestone 3 - Track scan/list foundation
3.6.1 Status: Complete.
3.6.2 Implemented: -list, deterministic scan order, selection marker infrastructure.

3.7 Milestone 4 - Playback architecture foundation
3.7.1 Status: Complete.
3.7.2 Implemented: PLAY_STATE, PSG abstraction, routed PSG writes, MUSIC_BUF lifecycle.

3.8 Milestone 5 - Initial engine slices
3.8.1 Status: In progress.
3.8.2 Implemented: loader, engine init dispatch, proof-of-pipeline PTX tick, MYM frame-write stub path.
3.8.3 Remaining: real PTx and MYM decode/playback engines.

3.9 Milestone 6 - Input and loop control
3.9.1 Status: Complete and stabilized.
3.9.2 Implemented: non-echo input polling, Q/q, Esc, Ctrl-C quit, Space pause/resume, N/P navigation, L loop mode with status messages.

3.10 Milestone 7 - vtunecfg.com utility
3.10.1 Status: Complete for baseline feature set.
3.10.2 Implemented: mode write, show/status, verify roundtrip, VTUNE.CFG authoring.

3.11 Milestone 8 - TurboSound support track
3.11.1 Status: Preparation complete, decode integration pending.
3.11.2 Implemented:
1. Routed output architecture with AY and TurboSound modes.
2. Automatic PT3 TurboSound detection from loaded file internals.
3. Startup telemetry line for detected audio mode.
3.11.3 Remaining: connect dual-chip routing to real PT3 decode output.

3.12 Milestone 9 - Hardware detection and port routing
3.12.1 Status: Complete and validated on hardware.
3.12.2 Implemented:
1. Three-tier detection: CLI stub, HBIOS query, Tune-compatible probe fallback, MSX default.
2. Dynamic PSG_REG_PORT / PSG_DATA_PORT (and PSG2 pair for TurboSound routing).
3. Startup hardware telemetry before parse.
3.12.3 Evidence (31-May-2026): SC126 EB ($68/$60); RC2014 MSX card ($A0/$A1).
3.12.4 Remaining: CLI override flags (-msx, -rc, -eb, -coleco) still stubbed.

3.13 Deferred - VGM
3.13.1 Status: Deferred by scope decision.

## 4. SIMH and Test Workflow State

4.1 SIMH launch scripts now enforce matching ROM/profile assumptions.
4.2 Sync-SIMH-Content.ps1 copies vtune/vtunecfg/tunes and rebuilds images.
4.3 Current known interactive boot path for ZPM3 slice in this profile is 3.4.

## 5. Current Repository State

5.1 Core runtime: vibetune.asm.
5.2 Config utility: vtunecfg.asm.
5.3 Validation ledger and parser/spec docs are maintained in workspace root.
5.4 Current observed runtime version at last update: 0.0.42.

## 6. Next Milestone Execution Plan (M5 PT3 decode, clean-room)

6.1 Phase A — Quark timing scaffold in pt3engine.inc. **Complete.**
6.2 Phase B — Position pointers, channel BSS, note table, AY flush. **Complete.**
6.3 Phase C — Pattern-line decoder, position/pattern bind, play tick. **Complete (v0.0.42).**
6.4 Phase D — Hardware audible smoke on `rl2wof.pt3` and `rl2wofts.pt3`; update ledger decode-smoke columns (user-reported).
6.5 Phase E — TurboSound dual-chip routing from the same decode output path where required.
6.6 Keep MYM decode completion as follow-up after PT3 baseline audio.
6.7 Do not regress M9 hardware lines or M8 mode telemetry validated on SC126/RC2014.
6.8 Do not copy or include tune.com / Bulba / RomWBW VibeTune player sources (see ProjectGoals.md).
