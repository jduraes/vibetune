31-May-2026

# VibeTune Non-Hardware and SIMH Testing

## 1. Purpose

1.1 Validate CLI, parser, startup, and integration behavior without relying only on physical hardware cycles.
1.2 Keep a reproducible SIMH workflow for milestone smoke checks.

## 2. Preconditions

2.1 Build from workspace root:
2.1.1 cmd /c Build.cmd
2.2 Ensure sibling RomWBW tree exists at ..\RomWBW.
2.3 Ensure SIMH binaries exist under ..\RomWBW\Tools\simh.

## 3. Launch Paths

3.1 Preferred internal script:
3.1.1 cmd /c Run-SIMH-VibeTune.cmd
3.2 External profile script:
3.2.1 cmd /c Run-SIMH-VibeTune-External.cmd
3.3 Both flows assume content sync and image/profile consistency.

## 4. Content Sync

4.1 Use Sync-SIMH-Content.ps1 before validation sessions when binaries or tune corpus changed.
4.2 Sync scope includes:
4.2.1 vtune.com
4.2.2 vtunecfg.com
4.2.3 Tunes corpus files (.pt2/.pt3/.mym)
4.3 Sync rebuilds target RomWBW disk images used by SIMH test paths.

## 5. Boot Notes

5.1 In current profile assumptions, ZPM3 interactive boot path is 3.4.
5.2 ROM/profile mismatch can appear as hang-like behavior; use provided launch scripts to avoid drift.

## 6. Suggested Smoke Tests

6.1 Parser and usage
6.1.1 VTUNE
6.1.2 VTUNE rl2wof
6.1.3 VTUNE rl2wofts
6.1.4 VTUNE rl2woftz

6.2 List mode
6.2.1 VTUNE -LIST
6.2.2 Verify clean return to CCP after list output.

6.3 Config utility
6.3.1 VTUNECFG show
6.3.2 VTUNECFG plain
6.3.3 VTUNECFG verify

6.4 Runtime controls
6.4.1 While in loop, test Space, L, N, P, Esc, Ctrl-C.

## 7. Expected Current Milestone Output

7.1 Startup should show classification and engine-init readiness.
7.2 Startup should also show audio mode telemetry.
7.3 rl2wof expected mode line: AY (single-chip).
7.4 rl2wofts expected mode line: TurboSound (auto-detected).
7.5 Audible decode parity is not yet the acceptance criterion for this milestone stage.
7.6 When decode lands, gate on both: rl2wof (AY audible) and rl2wofts (TurboSound audible).

## 8. Host Harness

8.1 Structural gate harness:
8.1.1 pwsh -NoProfile -File .\Validate-StructureChecks.ps1
8.2 Use harness before SIMH runs to quickly catch structural regressions.

## 9. Recording Results

9.1 Record startup and control-path observations in TunesValidationLedger.md.
9.2 Treat unexpected startup-mode changes as regressions until explained.
9.3 Physical hardware validation (31-May-2026, v0.0.38) supersedes SIMH for M9; ledger section 5.8 holds evidence.
9.4 Canonical pre-SIMH sync remains Sync-SIMH-Content.ps1 in this workspace (stages Apps, rebuilds hd1k_cpm22 / hd1k_zpm3 / hd1k_combo).
