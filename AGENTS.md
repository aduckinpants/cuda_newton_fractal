# Viewer-Host Agent Bootstrap

This repo should be worked through its checked-in workflow rails, not chat archaeology.

Treat this file as the concise bootstrap surface for any new agent session.

## Session Start

Do these before making architecture claims or starting broad edits:

1. Run `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8`.
2. Read `C:\code\salticid-cuda\docs\testing_cheat_sheet.md`.
3. Read `AGENT_WORKING_PROTOCOL.md`.
4. Read `spec_intake/_STATUS.md`, `DEFERRED_THREADS.md`, and `KNOWN_ISSUES.md`.
5. Read the last few entries of `HANDOFF_LOG.md`.
6. If you are starting a meaningful work slice, append a pending breadcrumb first:
   - `py -3.14 tools/viewer_host_begin_work_slice.py --intent "<slice>" --profile <native|runtime|catalog|checkpoint|unspecified>`
7. Create or update a detailed, checklisted phased plan in the repo.
   - Prefer the nearest existing plan doc.
   - Otherwise create `docs/notes/<slug>_PHASED_PLAN.md`.
   - Protocol: `docs/PHASED_PLAN_CONTINUITY_PROTOCOL.md`.
8. Read `.github/copilot-instructions.md` after this file, not instead of it.

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

## Mandatory Audit Rule

- After every meaningful code slice, run a distrust-first audit before treating the slice as done.
- Default posture: assume the first implementation is wrong or incomplete until a hostile review proves otherwise.
- Required audit loop:
   1. review the landed diff and the touched seams as if they contain at least one bug
   2. keep digging until a real defect, omission, or workflow mistake is found, or a second and third pass fail to find one
   3. if an audit pass finds a real issue, add the regression first, fix it, revalidate, and repeat the audit on the repaired state
- Do not wait for the user to ask for this review explicitly; it is part of normal slice closure in this repo.
- Record audit findings and follow-up checkpoint commits in `HANDOFF_LOG.md`.

## Public Validation Profiles

Use the public task/profile surface instead of reconstructing command bundles from memory:

- `agent: session bootstrap`
- `verify: profile native`
- `verify: profile runtime`
- `verify: profile catalog`
- `verify: profile checkpoint`

Current profiles:

- `native` — code-quality audit plus `ui_app/build_tests_vsdevcmd.cmd`
- `runtime` — code-quality audit plus `ui_app/build_vsdevcmd.cmd` and the focused probe/session runtime pytest lane
- `catalog` — code-quality audit plus `ui_app/build_vsdevcmd.cmd` and strict catalog smoke
- `checkpoint` — code-quality audit plus native helper tests, runtime publish, and focused probe/session runtime pytest

The VS Code task surface is the canonical profile surface under `verify: profile ...`.

## Build And Tooling Rule

- Prefer the checked-in build scripts and workflow tools over ad hoc shell rituals.
- Use `ui_app/build_tests_vsdevcmd.cmd` and `ui_app/build_vsdevcmd.cmd` instead of hand-assembled compiler commands.
- Prefer task or tool surfaces when they exist for bootstrap, phased-plan sync, and validation profiles.

## Cross-Repo Rule

- `salticid-cuda` is the mainline reference repo from this workspace.
- Do not edit mainline directly from here.
- This repo may invoke mainline workflow helpers when needed, especially `C:\code\salticid-cuda\tools\handoff_append.py`.
- Do not fork core workflow tools into this repo under the same names. Call the mainline helper directly when it supports `--repo-root`, or use a thin repo-specific `viewer_host_*` adapter when local context is required.

## Output Discipline

- Always pass explicit `--out`, `--out-dir`, or `--out-json` paths for generated reports.
- Prefer repo-local `artifacts/` outputs over chat-only summaries.

## VS Code Stability Rule

- Prefer one terminal command at a time for heavy build/test flows.
- If terminal output is likely to be large, redirect to an `artifacts/` log and inspect the file instead of streaming everything through the wrapper.
- Reuse the public task and tool surfaces instead of opening extra ad hoc terminals.

## End Of Slice

Before ending a meaningful work slice:

1. Update the active phased plan, if one exists.
2. Respect the workspace checkpoint guard hook in `.github/hooks/checkpoint_guard.json`; completion/stop is blocked if repo state differs from the session baseline.
3. If the session advanced `HEAD`, write a validation receipt for the current committed state with `py -3.14 tools\viewer_host_write_validation_receipt.py --summary "<what passed>" --command "<validation cmd>" ...`.
4. Append `HANDOFF_LOG.md` with the handoff append helper.
5. Run the matching public validation profile or the equivalent checked-in scripts for the slice.
6. Follow the repo checkpoint discipline from `AGENT_WORKING_PROTOCOL.md`.

Do not treat validated-but-undocumented work as finished.