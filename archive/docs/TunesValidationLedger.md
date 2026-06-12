31-May-2026

# Tunes Validation Ledger

This ledger defines expected behavior and recorded evidence for the current Tunes corpus.

## 1. Purpose

1.1 Track file-by-file expected parser and startup behavior.
1.2 Detect regressions quickly after parser/runtime changes.
1.3 Distinguish structural acceptance from real playback decode readiness.

## 2. Status Columns

2.1 Expected Engine: extension-based routing target.
2.2 Structural Validation: stage gate before playback loop.
2.3 Startup Smoke: startup path evidence in current milestone stage.
2.4 Decode Smoke: real audible decode status (currently pending globally).
2.5 Notes: compatibility or milestone remarks.

## 3. Corpus Expectations

| File | Expected Engine | Structural Validation | Startup Smoke | Decode Smoke | Notes |
|---|---|---|---|---|---|
| Attack.pt3 | PTx | Pass | Pending | Pending | Valid PT3 corpus file |
| Backup.pt3 | PTx | Pass | Pending | Pending | Valid PT3 corpus file |
| BadMice.pt3 | PTx | Pass | Pending | Pending | Valid PT3 corpus file |
| Demo.mym | MYM | Pass | Pending | Pending | Valid MYM corpus file |
| Demo1.mym | MYM | Pass | Pending | Pending | Valid MYM corpus file |
| Demo3.mym | MYM | Pass | Pending | Pending | Valid MYM corpus file |
| Demo3mix.mym | MYM | Pass | Pending | Pending | Valid MYM corpus file |
| Demo4.mym | MYM | Pass | Pending | Pending | Valid MYM corpus file |
| HowRU.pt3 | PTx | Pass | Pending | Pending | Valid PT3 corpus file |
| Iteratn.pt3 | PTx | Pass | Pending | Pending | Valid PT3 corpus file |
| LookBack.pt3 | PTx | Pass | Pending | Pending | Valid PT3 corpus file |
| Louboutn.pt3 | PTx | Pass | Pending | Pending | Valid PT3 corpus file |
| Namida.pt3 | PTx | Pass | Pending | Pending | Valid PT3 corpus file |
| Recoll.pt3 | PTx | Pass | Pending | Pending | Valid PT3 corpus file |
| rl2wof.pt3 | PTx | Pass | Pass | Pending | Startup confirms AY single-chip mode |
| rl2wofts.pt3 | PTx | Pass | Pass | Pending | Startup confirms TurboSound auto-detected |
| Sanxion.pt3 | PTx | Pass | Pending | Pending | Valid PT3 corpus file |
| Synch.pt3 | PTx | Pass | Pending | Pending | Valid PT3 corpus file |
| ToStar.pt3 | PTx | Pass | Pending | Pending | Valid PT3 corpus file |
| Victory.pt3 | PTx | Pass | Pending | Pending | Valid PT3 corpus file |
| Wicked.pt3 | PTx | Pass | Pending | Pending | Valid PT3 corpus file |
| YeOlde.pt3 | PTx | Pass | Pending | Pending | Valid PT3 corpus file |
| Yeovil.pt3 | PTx | Pass | Pending | Pending | Valid PT3 corpus file |

## 4. Update Rules

4.1 Do not remove corpus rows unless files are removed from Tunes.
4.2 Treat unexpected rejection of a listed file as regression candidate.
4.3 Startup Smoke values:
4.3.1 Pending: not yet observed in current stage.
4.3.2 Pass: startup path observed as expected.
4.3.3 Fail: startup path diverged; investigate before changing expectations.
4.4 Decode Smoke values:
4.4.1 Pending: real decoder milestone not yet complete.
4.4.2 Pass: audible decode verified for current build.
4.4.3 Fail: decode smoke failed with reproducible evidence.

## 5. Evidence Log

5.1 31-May-2026: Host harness (Validate-StructureChecks.ps1) passes corpus and rejects negative vectors as expected.
5.2 31-May-2026: Milestone 6 stabilized with non-echo input, Esc/Ctrl-C quit, pause/loop telemetry.
5.3 31-May-2026: vtunecfg baseline utility delivered (write/show/verify for VTUNE.CFG).
5.4 31-May-2026: SIMH workflow hardened (ROM/profile alignment, content sync, image rebuild).
5.5 31-May-2026: PT3 TurboSound detection integrated as automatic file-driven selection.
5.6 31-May-2026: Runtime evidence on target confirms mode distinction:
5.6.1 vtune rl2wof -> Audio mode: AY (single-chip).
5.6.2 vtune rl2wofts -> Audio mode: TurboSound (auto-detected).
5.7 31-May-2026: Decode milestone remains pending; startup evidence does not imply full decode parity.
5.8 31-May-2026: M9 hardware detection validated on physical targets (v0.0.38):
5.8.1 SC126 + RC EB module: hardware line RCZ180 EB ($68/$60); VTUNE usage; VTUNE rl2wof (AY); VTUNE rl2wofts (TurboSound); VTUNE -LIST (3 tracks, clean CCP return).
5.8.2 RC2014 + MSX-addressed sound card: hardware line MSX standard ($A0/$A1); VTUNE rl2wofts (TurboSound); clean exit.
5.9 Decode-smoke remains pending globally until PT3 quark decode produces verified audible playback on both reference tunes: rl2wof.pt3 (AY) and rl2wofts.pt3 (TurboSound).
5.10 1-Jun-2026: v0.0.53 candidate built with PT3 shadow-register initialization and high-bit-normalized key dispatch; Validate-StructureChecks.ps1 passes corpus and invalid vectors.
5.11 1-Jun-2026: v0.0.54 candidate hardens startup/exit quieting via direct-port mute when hardware detection is valid (not fallback-only); Validate-StructureChecks.ps1 passes corpus and invalid vectors.
5.12 1-Jun-2026: v0.0.55 candidate forces AY OUT/IN primitives to 8-bit port addressing (B=0) across write/read/probe paths to avoid platform-sensitive upper-byte port aliasing; Validate-StructureChecks.ps1 passes corpus and invalid vectors.
5.13 1-Jun-2026: v0.0.56 diagnostic build adds ~1 second pauses between hardware-detection attempts (HBIOS and probe candidates) to isolate when pop/whine appears on target hardware; Validate-StructureChecks.ps1 passes corpus and invalid vectors.
5.14 1-Jun-2026: v0.0.57 removes diagnostic delays, adopts tune-informed full 14-register mute on exit/startup silence path, and adds Z180 slow-I/O bracketing around PSG write/read/probe primitives; Validate-StructureChecks.ps1 passes corpus and invalid vectors.
5.15 1-Jun-2026: v0.0.58 corrects PT3 channel-mute amplitudes (use 0x0F quiet), enables dynamic tone mixer gating (instead of fixed 0x3F mute), skips reg13 flush writes when bit7 set, and removes premature channel-A-driven order advance that caused sub-second returns; Validate-StructureChecks.ps1 passes corpus and invalid vectors.
5.16 1-Jun-2026: v0.0.59 hardens PT3 pattern bind logic: out-of-range position pointer is rebased to module pos-list, invalid pattern index bytes are clamped to 0 instead of immediate stop, and pattern-offset clipping now falls back to pattern base pointer; Validate-StructureChecks.ps1 passes corpus and invalid vectors.
5.17 1-Jun-2026: v0.0.60 fixes PT3 pattern-table interpretation for ProTracker path: each pattern index now resolves to a direct 3-word A/B/C stream tuple from the table (base + index*6), with per-stream absolute-offset resolution against module base; Validate-StructureChecks.ps1 passes corpus and invalid vectors.
5.18 1-Jun-2026: v0.0.61 corrects v0.0.60 regression — pattern table is 2 bytes/pattern (pattern base offset); channel A/B/C streams are 3 LE16 words relative to that pattern base (PT3FormatSpec 2.6–2.7). Hardware retest pending for rl2wof/rl2wofts audible playback.
5.19 1-Jun-2026: v0.0.62 replaces zero-all-registers mute (v0.0.57–0.0.61 whine/pops) with safe mute (reg7=$3F, reg8–10=$0F, no tone period writes); PT3 shadow uses $FF periods when muted, skips disabled-channel tone flush, enables special-command dispatch; Validate-StructureChecks.ps1 passes.
5.20 1-Jun-2026: v0.0.63 fixes startup whine on usage-only `vtune.com` (no tune loaded): early Z180 mute before detect, PROBE_AY_QUIET before register-2 probe write, HBIOS detect mutes after port query; probe quiet uses Z180 slow I/O on $68/$D8.
5.21 1-Jun-2026: v0.0.64 fixes playback/exit whine: PT3 mixer starts $3F (enable-only-active), flush writes HW mixer off before periods, mute paths set regs 0–6 to $FF; mute before MAIN_LOOP entry.
5.22 1-Jun-2026: v0.0.65 reworks PSG path from RomWBW `Source/Apps/Tune/tune.asm`: mute via HBIOS BF_SNDRESET when available else zero+ROUT; PT3/MYM output via PSG_ROUT_BLOCK (tune LOUT); Z180_IO_BASE $C0 from CFGTBL; removed pre-detect and post-detect mute storms.
5.23 1-Jun-2026: v0.0.67 defers AY detect until after successful parse (usage-only silent on RCZ180); ROUT-only mute if PSG touched; user-validated: no pops/whine on bare vtune.com; play path no whine, brief pops on entry/pause/exit, no music on rl2wof yet.
5.24 1-Jun-2026: v0.0.68 Space pause uses PSG_MIXER_OFF (reg 7=$3F only). RCZ180: no whine; silent pause/resume; pop on entry and exit only; no music on rl2wof.
5.25 1-Jun-2026: v0.0.69 PSG_EXIT_QUIET reverted after RCZ180 failure (entry pops, whine, sustained post-exit whine). Restored v0.0.68 (ROUT exit mute, mixer-only pause).
