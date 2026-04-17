# Viewer-Host Agent Bootstrap

This repo should be worked through its checked-in workflow rails, not chat archaeology.

Treat this file as the concise bootstrap surface for any new agent session.

## Session Start

Do these before making architecture claims or starting broad edits:

1. Run `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8`.
2. Read `C:\code\salticid-cuda\docs\testing_cheat_sheet.md`.
3. Read `AGENT_WORKING_PROTOCOL.md`.
4. Read `AGENT_TERMINAL_PROTOCOL.md`.
5. Read `spec_intake/_STATUS.md`, `DEFERRED_THREADS.md`, and `KNOWN_ISSUES.md`.
6. Read the last few entries of `HANDOFF_LOG.md`.
7. If you are starting a meaningful work slice, append a session-start breadcrumb first:
   - `py -3.14 tools/viewer_host_begin_work_slice.py --intent "<slice>" --profile <native|runtime|catalog|checkpoint|unspecified>`
8. Create or update a detailed, checklisted phased plan in the repo.
   - Prefer the nearest existing plan doc.
   - Otherwise create `docs/notes/<slug>_PHASED_PLAN.md`.
   - Protocol: `docs/PHASED_PLAN_CONTINUITY_PROTOCOL.md`.
9. Read `.github/copilot-instructions.md` after this file, not instead of it.

The VS Code task surface mirrors these commands:
- `agent: session bootstrap`
- `agent: begin work slice`
- `agent: assert phased plan sync`

## Continuity Rule

- No chat history is assumed durable.
- Every meaningful work slice needs a detailed, checklisted, phased plan in the repo.
- `HANDOFF_LOG.md` is the checkpoint index, not the detailed plan surface.
- A fresh agent should be able to resume from the repo plan, handoff log, and git state alone.

## Checklist Sync Rule

- If you edit a `*_PHASED_PLAN.md` file, update `## Phase Checklist` and `## Current Phase` in the same edit.
- Do not leave a plan saying a later phase is current while earlier phases are still unchecked.
- Do not leave stale blocker text in a phased plan after another checked-in slice already closed that blocker.
- Deterministic check surface: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` or task `agent: assert phased plan sync`.

## Carryover Rule

- If a new user prompt arrives while the repo still differs from the session baseline, treat that as prior-slice closure debt first.
- Do not smear a new request across an uncheckpointed carryover slice.
- Treat prompt text as context only. Tool-generated prompts such as `Start implementation` and abrupt steering/reorientation prompts do not relax checkpoint, receipt, or carryover rules.
- The `UserPromptSubmit` warning in `.github/hooks/checkpoint_guard.json` now surfaces this condition immediately, but the agent is still responsible for resolving the carryover cleanly.

## Mandatory Audit Rule

- After every meaningful code slice, run a distrust-first audit before treating the slice as done.
- Default posture: assume the first implementation is wrong or incomplete until a hostile review proves otherwise.
- Required audit loop:
   1. review the landed diff and the touched seams as if they contain at least one bug
   2. keep digging until a real defect, omission, or workflow mistake is found, or a second and third pass fail to find one
   3. if an audit pass finds a real issue, add the regression first, fix it, revalidate, and repeat the audit on the repaired state
- Do not wait for the user to ask for this review explicitly; it is part of normal slice closure in this repo.
- Record audit findings and follow-up checkpoint commits in `HANDOFF_LOG.md`.

## Explicit Closure Standard

- No slice is done unless all four of these are true:
   1. a real named bug, risk, or bounded behavior gap was identified
   2. a focused proving test or regression covers that gap
   3. the relevant checked-in validation rails are green
   4. the slice is checkpointed in a commit
- Treat any missing item in that chain as not done.

## Public Validation Profiles

Use the public task/profile surface instead of reconstructing command bundles from memory:

- `agent: session bootstrap`
- `verify: profile native`
- `verify: profile runtime`
- `verify: profile catalog`
- `verify: profile checkpoint`

Current profiles:

- `native` — code-quality audit plus `ui_app/build_tests_vsdevcmd.cmd`
- `runtime` — code-quality audit plus `ui_app/build_vsdevcmd.cmd` and the focused probe/session runtime pytest helper lane, which preflights the published runtime and fails ambiguous zero-pass skip-only runs
- `catalog` — code-quality audit plus `ui_app/build_vsdevcmd.cmd` and strict catalog smoke
- `checkpoint` — code-quality audit plus native helper tests, runtime publish, and the same focused probe/session runtime pytest helper lane

The VS Code task surface is the canonical profile surface under `verify: profile ...`.

## Build And Tooling Rule

- Prefer the checked-in build scripts and workflow tools over ad hoc shell rituals.
- Use `ui_app/build_tests_vsdevcmd.cmd` and `ui_app/build_vsdevcmd.cmd` instead of hand-assembled compiler commands.
- Prefer task or tool surfaces when they exist for bootstrap, phased-plan sync, and validation profiles.

## FITS Import Contract

- Treat `Load FITS...` as a foreign-import operator path.
- Default operator contract:
  - required user-facing input: FITS only
  - forbidden normal-path prompts: `state.json`, `finding.json`, request JSON, bundle JSON
  - allowed internal detail: synthesize any repo-native request/session/state artifacts automatically
- If the default FITS-open path asks the operator for repo-native JSON from this repo, that is a bug and a Pit-of-Success violation.
- Review and test FITS import work against the real operator invariant:
  - given only a FITS path, the viewer reaches playback without requesting repo-native JSON inputs.

## Cross-Repo Rule

- `salticid-cuda` is the mainline reference repo from this workspace.
- Do not edit mainline directly from here.
- This repo may invoke mainline workflow helpers when needed, especially `C:\code\salticid-cuda\tools\handoff_append.py`.
- Do not fork core workflow tools into this repo under the same names. Call the mainline helper directly when it supports `--repo-root`, or use a thin repo-specific `viewer_host_*` adapter when local context is required. For checkpoint closure in this repo, prefer `py -3.14 tools\viewer_host_append_handoff.py ...` over calling the mainline helper by hand.

## Output Discipline

- Always pass explicit `--out`, `--out-dir`, or `--out-json` paths for generated reports.
- Prefer repo-local `artifacts/` outputs over chat-only summaries.

## VS Code Stability Rule

- Follow `AGENT_TERMINAL_PROTOCOL.md` for terminal-count limits and crash recovery.
- Prefer one terminal command at a time for heavy build/test flows.
- If terminal output is likely to be large, redirect to an `artifacts/` log and inspect the file instead of streaming everything through the wrapper.
- If a command destabilizes VS Code or the terminal wrapper, switch to crash-safe mode for the rest of the session: one command at a time, no parallel terminal work, and prefer the logged-command wrapper or task surfaces.
- Reuse the public task and tool surfaces instead of opening extra ad hoc terminals.

## End Of Slice

Before ending a meaningful work slice:

1. Update the active phased plan, if one exists.
2. Respect the workspace checkpoint guard hook in `.github/hooks/checkpoint_guard.json`; completion/stop is blocked if repo state differs from the session baseline.
3. Run the matching public validation profile or the equivalent checked-in scripts for the slice.
4. Append `HANDOFF_LOG.md` with `py -3.14 tools\viewer_host_append_handoff.py --commit <checkpoint_id> --score <n> "<message>"` before the final commit; reuse the token printed by `viewer_host_begin_work_slice.py` for the normal flow and keep `--resolve-last-pending` only for legacy pending-entry repair.
5. If the session advanced `HEAD`, write a validation receipt for the current committed state with `py -3.14 tools\viewer_host_write_validation_receipt.py --summary "<what passed>" --command "<validation cmd>" ...` after the final clean commit.
6. Follow the repo checkpoint discipline from `AGENT_WORKING_PROTOCOL.md`.
7. Do not say done unless the explicit closure standard above is satisfied end-to-end.
8. If the current prompt arrived while the repo already differed from the session baseline, resolve that carryover before treating any new request as a fresh slice.

Do not treat validated-but-undocumented work as finished.
