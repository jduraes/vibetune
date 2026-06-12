31-May-2026

# VibeTune Parser Behavior Spec

## 1. Scope

1.1 Defines current command-line parse/classify behavior in vtune.com.
1.2 Covers parser, extension resolution, validation stages, and startup status output.
1.3 Covers -list mode behavior and exit policy.
1.4 Does not define full PT3/MYM decode semantics.

## 2. Command-Line Inputs

2.1 Source is CP/M command tail at 0080h.
2.2 Accepts exactly one argument token in current flow.
2.3 Leading/trailing spaces are ignored.
2.4 Quoted token form is accepted.

## 3. Accepted Invocation Modes

3.1 VTUNE -LIST
3.1.1 Case-insensitive switch.
3.1.2 Scans supported files and prints list/count.
3.1.3 Returns to CCP without entering playback load path.

3.2 VTUNE <filename[.ext]>
3.2.1 Supports explicit .PT2/.PT3/.MYM extension.
3.2.2 If extension is missing, probe order is .PT3 then .PT2 then .MYM.
3.2.3 On first existing candidate, ARG buffer is updated in-place and engine is selected.

## 4. Classification Rules

4.1 Case-insensitive extension handling.
4.2 .pt2 and .pt3 classify to PTx engine.
4.3 .mym classifies to MYM engine.
4.4 Unsupported extension returns unsupported-format error.

## 5. Validation Stages

5.1 Stage 1: parse and classify.
5.2 Stage 2: file access preflight.
5.2.1 Build FCB from argument.
5.2.2 Open file, read first record to DMA, close file.
5.3 Stage 2 structural checks.
5.3.1 PTx requires printable leading text and recognized family header prefix.
5.3.2 MYM requires magic 0x92 and bounded second-byte hint.
5.4 Stage 3: engine init binding.
5.4.1 PTx binds family variant (Vortex or ProTracker).
5.4.2 MYM captures hint byte and initializes state pointers.

## 6. Runtime Startup Telemetry

6.1 Banner and hardware configuration line print immediately after detection (before parse).
6.2 Successful file path additionally prints:
6.2.1 Input file
6.2.2 Classification
6.2.3 Variant family for PTx or MYM hint
6.2.4 Engine init readiness
6.2.5 Audio mode status line (after load)
6.3 Audio mode status semantics:
6.2.1 AY (single-chip)
6.2.2 TurboSound (auto-detected)

## 7. Output Mode Selection Rules

7.1 Default mode is AY.
7.2 For PTx files, post-load scan checks for embedded secondary ProTracker header.
7.3 On positive detection, output mode switches to TurboSound automatically.
7.4 No manual CLI or key toggle is used for this selection.

## 8. Error Policy

8.1 Missing argument -> usage output.
8.2 Multiple arguments -> error plus usage.
8.3 Invalid filename form -> filename error.
8.4 File not found -> file-not-found error.
8.5 Read/access problems -> file-read error.
8.6 Structural mismatch -> invalid-data error.
8.7 Engine init failure -> engine-init error.
8.8 All errors return to CCP without entering main playback loop.

## 9. List Mode Behavior

9.1 Scans CP/M directory for .PT2/.PT3/.MYM.
9.2 Deduplicates results.
9.3 Prints deterministic list order by directory slot traversal.
9.4 Restores default DMA (0080h) before exit.
9.5 Skips PSG silence write on list-mode exit to reduce side effects.

## 10. Control Input Behavior in Playback Loop

10.1 Input polling uses non-echo direct console read.
10.2 Quit keys: Q/q, Esc, Ctrl-C.
10.3 After load, `MAIN_LOOP` runs until a quit key or natural end (MYM exhaust / future PT3 end); there is no separate on-screen “now playing” banner — absence of output does not mean an immediate return to the CCP prompt.
10.4 Startup runs hardware detection before parse; **v0.0.46+** calls `PSG_MUTE_ALL` after detection and on every exit so probe/usage paths do not leave the AY squealing.
10.3 Space toggles pause/resume.
10.4 N/P navigate tracks.
10.5 L cycles loop mode (off, track, playlist).

## 11. Known Gaps

11.1 Structural and control-flow behavior are ahead of decode completeness.
11.2 Current PTX and MYM engine ticks are scaffolding and do not yet represent full decoder fidelity.
