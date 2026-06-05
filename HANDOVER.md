04-Jun-2026 — Build under test: **v0.0.73**

# VibeTune Handover (v0.0.73)

Short summary

- **Build:** v0.0.73 (04-Jun-2026). **Fix:** `PT3_BIND_PATTERN` stack leak (leftover `PUSH AF` → instant return to CCP). v0.0.72: prompt return + exit whine after classification. **Retest `rl2wof` on SC126.**
- **Hardware:** RCZ180 EB module ($68/$60).

What changed in v0.0.65–0.0.67

- PSG path aligned with `tune.com` (ROUT bulk write, Z180 slow I/O $C0, ROUT-only mute).
- Deferred AY detect until after successful parse (fixes usage-only pops).
- Exit mute only when `PSG_TOUCHED` (playback actually used the chip).

Immediate testing checklist (what to run and what to report)

- Build (if you want to rebuild locally):

```powershell
.\Build.cmd
```

- Run the structural validation harness:

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File .\Validate-StructureChecks.ps1
```

- Run VibeTune on the target (RomWBW / SIMH / hardware). At the CP/M prompt run:

```
vtune.com rl2wof
vtune.com rl2wofts
```

Record these observations for each tune:

- **Audible music:** Yes / No
- **Pops on start/stop:** None / Few / Many
- **App behavior:** Stays until Esc? Quiet on exit? Any error text?
- **Engine banner lines:** Copy the startup lines that show `Classification`, `Engine init`, and `Audio mode`.

-If music is still absent

- For audio verification: run `vtune.com` on real hardware (RCZ180 or equivalent) and capture a short video or serial log of the CP/M session showing `vtune.com` startup and the observations — SIMH does not emulate AY audio and cannot provide trustworthy sound output.
- For decoding/logging only: you may still run under SIMH (`Run-SIMH-VibeTune.cmd`) to capture a CP/M transcript and `vtune.lst` (assembly listing / runtime logs); attach those when reporting to help with decode trace and timing analysis.
- Next code actions (on my side): enable a small PT3-decode trace output (tick-level) and a guarded AY-register dump to help locate whether notes are being scheduled or suppressed.

Next engineer steps

- **Validated 01-Jun v0.0.67 (RCZ180):** `vtune.com` — silent. `vtune.com rl2wof` — no whine; pops on play/pause/exit; stays in loop until Esc; no audible tune yet.
- Priority A: PT3 decode port (Bulba/tune.com parity) for audible `rl2wof`.
- **Done (v0.0.68):** Mixer-only pause — user confirms no pause/resume pops.
- **Do not retry** quiet exit without full ROUT — v0.0.69 failed on hardware (whine after exit).
- Priority C: Triage remaining envelope/special-command decode cases that can silence channels.

Relevant files to attach when reporting

- `vtune.com` (build artifact)
- `vtune.lst` (assembly listing in build output)
- Captured CP/M transcript / SIMH log
- `TunesValidationLedger.md` entries around v0.0.59–v0.0.60

Contacts and context

- I'm tracking progress in the repo (commit messages + `TunesValidationLedger.md`). If you'd like, I can:
	- run the trace-build and attach results, or
	- instrument a lightweight in-app debug flag that prints PT3 decode events to the console while the tune runs.

---
Updated: 01-Jun-2026 — concise handover for v0.0.60
