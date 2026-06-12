31-May-2026

# VibeTune Playback Engine Skeleton

This document defines the playback architecture contract and current implementation checkpoint. It is a design guide, not a source transplant.

## 1. Core Principles

1.1 Keep engine decode logic separate from UI, input polling, and command handling.
1.2 Keep shared runtime responsibilities centralized once.
1.3 Keep output hardware routing behind a unified PSG abstraction.
1.4 Keep format-specific state explicit and isolated.
1.5 Prefer deterministic failure handling over silent undefined behavior.

## 2. Shared Playback Architecture

2.1 Pipeline
2.1.1 Parse and classify input.
2.1.2 Preflight file access and structural checks.
2.1.3 Run engine initialization for selected format.
2.1.4 Load music payload to buffer.
2.1.5 Auto-select output mode (AY or TurboSound) from loaded data.
2.1.6 Enter runtime loop for tick, input dispatch, and state transitions.

2.2 Shared runtime responsibilities
2.2.1 PLAY_STATE lifecycle (stopped, playing, paused).
2.2.2 Loop state lifecycle (off, track, playlist).
2.2.3 Navigation and track reload path.
2.2.4 PSG silence policy on stop/pause/exit.
2.2.5 User-visible state telemetry messages.

2.3 Current contract surface
2.3.1 engine_init: validates and binds runtime state.
2.3.2 engine_step: advances playback state and emits register writes.
2.3.3 engine_stop/mute: ensures predictable quiet state.
2.3.4 engine_query: completion/loop/error metadata (future extension point).

## 3. PTx Engine Position

3.1 Implemented now
3.1.1 Variant family detection (Vortex versus ProTracker headers).
3.1.2 Runtime state binding and init readiness checks.
3.1.3 Tick call path through shared dispatcher.

3.2 Not implemented yet
3.2.1 Real pattern/sample/ornament decode.
3.2.2 Real channel mix, envelope, and mixer behavior.
3.2.3 True end-of-track and loop-point decode semantics.

3.3 Immediate next step
3.3.1 Replace proof-of-pipeline tick with first clean-room PT3 decode slice in pt3engine.inc.
3.3.2 Do not import tune.com / Bulba player source; implement original logic against Tunes corpus behavior.
3.3.3 Decode acceptance: rl2wof.pt3 (AY single-chip) and rl2wofts.pt3 (TurboSound) must both pass audible hardware smoke.

## 4. MYM Engine Position

4.1 Implemented now
4.1.1 Header gate and init state capture.
4.1.2 Tick dispatch entry.
4.1.3 Frame-write scaffold path through shared routed PSG output.

4.2 Not implemented yet
4.2.1 Complete MYM decompression/decode pipeline.
4.2.2 Robust stream-end and loop semantics.

## 5. TurboSound Positioning

5.1 TurboSound is an output-routing mode, not a separate parser UX mode.
5.2 Activation is automatic from loaded PT3 content.
5.3 No manual runtime toggle is exposed.
5.4 Startup can report detected mode as informational telemetry.
5.5 Decode logic remains format-centric; routing remains output-centric.

## 6. Format Scope Boundary

6.1 In scope: .pt2, .pt3, .mym.
6.2 Out of scope for current track: VGM playback.
6.3 AY as used here refers to PSG hardware path, not AY file format scope.

## 7. Memory and Timing Strategy

7.1 Keep engine state blocks small and contiguous.
7.2 Keep UI/navigation state separate from decode state.
7.3 Keep pacing in shared loop logic, not embedded in format decode internals.
7.4 Keep hardware routing writes behind a shared routine.

## 8. Failure Handling Policy

8.1 Any validation/init failure exits before playback loop entry.
8.2 Runtime stop paths must leave PSG silent.
8.3 Missing configuration must not block startup.
8.4 Unknown or invalid files must fail with explicit message paths.

## 9. Current Checkpoint Summary

9.1 Shared runtime infrastructure is stable.
9.2 Input/control paths are stabilized from Milestone 6.
9.3 TurboSound auto-detection and status telemetry are present.
9.4 M9 hardware detection and dynamic port routing validated on SC126 (EB) and RC2014 (MSX card).
9.5 Real PT3 quark decode remains the unresolved milestone gate for audible playback.
9.6 Current PTX_ENGINE_TICK is proof-of-pipeline only (not music); replace with clean-room PT3 decode per ImplementationPlan.md section 6 and ProjectGoals.md.

## 10. Clean-Room Decode Constraint

10.1 Player decode is original VibeTune code, not a port of tune.com or Bulba PTxPlay.
10.2 Reference material informs format semantics and acceptance tests only.
10.3 Target outcome: smaller, focused runtime suitable for RomWBW CP/M constraints.
10.4 Decode acceptance pair: rl2wof.pt3 (AY) and rl2wofts.pt3 (TurboSound auto-detected).
