31-May-2026

This is VibeTune (vtune.com), the spiritual successor to Wayne Warthen's tune.com, included in RomWBW.

## 1. Project lineage and reference trees

1.1 The previous VibeTune (`..\VibeTune-old`) became a bloated mess, full of bugs and unnecessary complexity.
1.2 This workspace started as a faithful copy of `..\RomWBW\Source\Apps\VibeTune`, where VibeTune was originally developed.
1.3 We keep **two reference points** on purpose:
1.3.1 `..\VibeTune-old` — study how things were done; identify good ideas and what became unusable.
1.3.2 `..\RomWBW\Source\Apps\VibeTune` — parallel RomWBW tree; do not treat as code to transplant into this workspace.
1.4 Official working baseline player: `..\RomWBW\Source\Apps\Tune` (tune.com). Less functionality (no TurboSound/dual-chip), but proven playback on RomWBW.

## 2. Clean-room engineering policy (permanent)

2.1 VibeTune is a **new implementation**, not a fork or line-for-line port of tune.com, Bulba PTxPlay, RomWBW VibeTune, or VibeTune-old.
2.2 Read reference code to understand **format behavior**, **compatibility expectations**, and **test baselines** only.
2.3 Do **not** copy, include, translate line-for-line, or structurally mirror legacy player source into this workspace.
2.4 Each decode milestone must produce **original VibeTune code** that is **lighter** (memory and speed) and **more focused** than reference players.
2.5 Compatibility is proven by **corpus behavior** (`Tunes/`, ledger evidence, hardware smoke), not by source similarity.
2.6 Shared RomWBW infrastructure is allowed (toolchain, hbios.inc, cpm.inc, build paths). Player decode logic is not.

## 3. Methodology: reason, document, then implement

3.1 Read and reason the reference code; define **action modules** before coding.
3.2 Document inner workings as **numbered bullet lists** (user edits to remove bloat or unclear items).
3.3 Nest related functionality with sub-numbering. Example shape (numbers illustrative):

    1. Display handling
        1.1 Plain text
        1.2 VT-100
        1.3 ANSI
    2. Config consumption (vtune.com reads; vtunecfg.com writes)
    3. Single-chip PTx playback
    4. Dual-chip TurboSound (file-driven, not user-toggled)
    5. MYM playback
    6. Memory / buffer lifecycle
    7. List mode (scan, selection, navigation)
    8. Keystroke handling and loop modes
    9. Hardware detection and PSG port routing

3.4 Apply the same methodology to tune.com and VibeTune-old where relevant.
3.5 Do **not** adopt reference code structure or accumulated bloat; learn, document, implement minimally.
3.6 Maintain reasoning in `DesignReasoning.md`, milestones in `ImplementationPlan.md`, engine contract in `PlaybackEngineSkeleton.md`.

## 4. RomWBW build and include discipline

4.1 Reference RomWBW includes via **relative paths** (e.g. `../RomWBW/Source/...`) so RomWBW can evolve in parallel without copying stale headers.
4.2 Use RomWBW build methods: workspace `Build.cmd`, and when needed `..\RomWBW\Makefile`, `Build.cmd`, `Clean.cmd`, and `..\RomWBW\Source\Apps\VibeTune` patterns.
4.3 Compiler, linker, and tools already live under `..\RomWBW`; use them.

## 5. Versioning and banner (agent-managed, permanent)

5.1 Format: **x.y.z**, starting at v0.0.1.
5.2 Increment **z** on every compile/build (agent responsibility, not the build script).
5.3 Increment **y** only after successful test **and** commit.
5.4 Increment **x** only on explicit user instruction.
5.5 Banner always includes OS **today's date** as **dd-Mmm-yyyy** (e.g. 31-May-2026).
5.6 Banner format: `VibeTune Player for RomWBW v{version}, {date}`.

## 6. Context handoff discipline

6.1 When agent context exceeds ~50% capacity, compact the conversation and start a fresh processing window.
6.2 Use the **handoff** skill (see user skill at `handoff` / `/handoff` command):
6.2.1 Compact the current conversation into a handoff document for the next agent.
6.2.2 Save to OS temp directory **or** workspace (`HANDOVER.md` / dated handoff).
6.2.3 Include a **suggested skills** section.
6.2.4 Do not duplicate PRDs, plans, ADRs, issues, commits, diffs — reference by path.
6.2.5 Redact secrets and PII.
6.2.6 If the user passes arguments, tailor the handoff to the next session focus.
6.3 **Hardware/runtime reports:** Do not infer playback or exit behavior from console transcripts alone. If a CP/M prompt reappears after `Audio mode:`, ask explicitly (e.g. did you press Esc/Q/Ctrl-C, how long did you wait, was there sound). Treat unexplained exits as unknown until the user confirms.

## 7. Milestone and planning discipline

7.1 Create and maintain a thorough plan with **testable milestones** (`ImplementationPlan.md`).
7.2 Example sequencing already reflected in plan: display mode decision before rich UI; file selection (CLI vs list) before playback; structural validation before decode; hardware ports before PSG I/O.
7.3 Record evidence in `TunesValidationLedger.md`; SIMH workflow in `SIMH-Testing.md`; sync via `Sync-SIMH-Content.ps1`.

## 8. Display and list-mode UX target

8.1 Display modes: plain, VT100, ANSI — configured externally via `vtunecfg.com` / `VTUNE.CFG` (not `-config` in vtune.com).
8.2 ANSI: modern consistent colour theme; selected track visually distinct in list mode.
8.3 **Target** interactive list presentation (not yet fully implemented; current `-list` is informational scan/print):

```
VibeTune Player for RomWBW v0.0.1, 30-May-2026
Keys: Esc=quit, (N)ext, (P)revious, WASD=navigate, (R)edraw (slow), (l/L)oop track/plist

Playing on MSX Standard Ports (A0H/A1H), delay mode

Song name:  backup forever
by: some artist

 ATTACK  .PT3   >BACKUP  .PT3<   BADMICE .PT3    ...
 ...
Looping Status: Off
```

8.4 `>BACKUP.PT3<` style markers and highlight colour denote selection in the **future** full list UI.

## 9. Playback scope and engines

9.1 **In scope now:** PTx (.pt2/.pt3), MYM, AY/PSG output, TurboSound routing when file requires it.
9.2 **Deferred:** VGM (engines exist in tune.com; do not implement in initial track).
9.3 Study tune.com and reference players for logic; plan memory, metadata, and tick pacing; implement **original** lightweight engines (`pt3engine.inc` and successors).
9.4 TurboSound is **automatic from file internals** — no user toggle.
9.5 **Decode acceptance (hardware):** both reference tunes must pass audible smoke:
9.5.1 `rl2wof.pt3` — AY (single-chip) routing.
9.5.2 `rl2wofts.pt3` — TurboSound (auto-detected) routing.

## 10. Configuration split

10.1 `vtune.com` — playback runtime; consumes `VTUNE.CFG`.
10.2 `vtunecfg.com` — display mode authoring (plain / VT100 / ANSI); verify and show modes.
10.3 Do **not** implement `-config` in vtune.com.

## 11. Current implementation snapshot (31-May-2026)

11.1 Runtime: **v0.0.40**; M0–M4, M6, M7, M9 complete; M8 prep complete; M5 decode in progress.
11.2 Hardware validated: SC126 EB ($68/$60); RC2014 MSX card ($A0/$A1).
11.3 Startup smoke pass on rl2wof (AY) and rl2wofts (TurboSound); **decode smoke pending**.
11.4 `-list` today: directory scan + print + clean CCP return — not yet the §8.3 interactive UI.

## 12. Living artifacts (do not replace with vague summaries)

12.1 `ProjectGoals.md` (this file) — permanent rules and methodology.
12.2 `ImplementationPlan.md` — milestones and status.
12.3 `DesignReasoning.md` — why decisions were made.
12.4 `PlaybackEngineSkeleton.md` — engine contract and gaps.
12.5 `ParserBehaviorSpec.md` — CLI, validation, telemetry.
12.6 `TunesValidationLedger.md` — corpus and smoke evidence.
12.7 `SIMH-Testing.md` — non-hardware test workflow.
12.8 `HANDOVER.md` — compact session handoff for the next agent.
