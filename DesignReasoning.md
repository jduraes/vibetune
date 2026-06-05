31-May-2026

# VibeTune Design Reasoning

This document records the rationale for current architecture and workflow decisions.

## 1. Why Bootstrap First

1.1 A standalone buildable artifact proved the workspace can evolve independently while reusing RomWBW infrastructure.
1.2 Early banner/version/date enforcement validated permanent release discipline before feature growth.

## 2. Why Display and Config Were Split Early

2.1 Runtime display policy must remain deterministic and stable.
2.2 vtune.com consumes configuration; vtunecfg.com authors configuration.
2.3 This boundary prevents configuration UX from bloating playback control paths.

## 3. Why Parser and Validation Were Staged

3.1 Single-file command path reduced initial state-space and made failures diagnosable.
3.2 Extension classification, preflight access, structural checks, and engine init were split into explicit stages.
3.3 Stage separation enabled clean error messaging and prevented unsafe playback entry.

## 4. Why Non-Echo Input Was Required

4.1 Echoing console input during playback made runtime UX noisy and misleading.
4.2 Direct console read without echo keeps display coherent while preserving responsive controls.
4.3 Ctrl-C handling was restored explicitly to keep expected CP/M quit behavior.

## 5. Why Loop and Pause Telemetry Is Explicit

5.1 Runtime controls are easier to trust when each transition is visible.
5.2 State messages for pause/play and loop mode reduce ambiguity during interactive testing.

## 6. Why TurboSound Is Auto-Detected, Not User-Toggled

6.1 User requirement is file-driven behavior, not a manual runtime switch.
6.2 PT3 mode decision now derives from module internals after load.
6.3 Startup mode reporting is informational only, preserving zero-config behavior.
6.4 Output routing remains centralized so dual-chip behavior does not fork parser or UI logic.

## 7. Why SIMH Workflow Needed Hardening

7.1 Prior ROM/profile mismatch produced non-actionable hangs and misleading failures.
7.2 Launch scripts now enforce known-good profile assumptions.
7.3 Content sync plus image rebuild is owned by VibeTune Sync-SIMH-Content.ps1 (stages binaries/tunes to RomWBW Apps, rebuilds hd1k images).

## 8. Why M9 Hardware Detection Was Required

8.1 PSG ports vary by platform and sound-module type; hardcoded addresses break multi-target RomWBW use.
8.2 HBIOS sound query is the primary path on RomWBW; probing is fallback only.
8.3 Validated profiles: SC126 EB ($68/$60), RC2014 MSX card ($A0/$A1).

## 9. Why Real Decode Remains the Critical Next Step

9.1 Current engine slices prove control-flow plumbing but do not yet implement complete PT3 or MYM decode.
9.2 Routing and mode detection are now in place so decode work can land without redesigning control infrastructure.
9.3 Decode must be implemented clean-room: understand reference behavior, write original lightweight VibeTune logic (ProjectGoals.md).
9.4 The next milestone should prioritize audible, structured playback over adding more UX surface area.

## 10. Current Decision Summary

10.1 Keep PTx and MYM as active scope.
10.2 Keep VGM deferred.
10.3 Keep vtunecfg as separate utility.
10.4 Keep TurboSound activation automatic from file content.
10.5 Keep clean-room decode policy: no legacy player source transplant.
10.6 Keep milestone discipline and ledger-backed evidence updates for every material change.
