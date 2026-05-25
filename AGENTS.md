# Viewer-Host Agent Bootstrap

This repo should be worked through its checked-in workflow rails, not chat archaeology.

Treat this file as the concise bootstrap surface for any new agent session.

## Session Start

Do these before making architecture claims or starting broad edits:

1. Run `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8`.
2. Run `py -3.14 tools/viewer_host_repo_status.py`.
3. Run `py -3.14 tools/viewer_host_rearward_review.py` for the current clean `HEAD` before starting new product mutation.
   - `ok` unlocks normal slice start.
   - `needs_repair` allows only a repair slice with `py -3.14 tools/viewer_host_begin_work_slice.py --rearward-repair-for <head> ...`.
   - `blocked_unproven` means the previous head lacks proof; repair proof first instead of opening product work.
4. Read `C:\code\salticid-cuda\docs\testing_cheat_sheet.md`.
5. Read `AGENT_WORKING_PROTOCOL.md`.
6. Read `AGENT_TERMINAL_PROTOCOL.md`.
7. Read `spec_intake/_STATUS.md`, `DEFERRED_THREADS.md`, and `KNOWN_ISSUES.md`.
8. Read the last few entries of `HANDOFF_LOG.md`.
9. If you are starting a meaningful work slice, append a session-start breadcrumb first:
   - `py -3.14 tools/viewer_host_begin_work_slice.py --intent "<slice>" --profile <native|runtime|catalog|checkpoint|unspecified> --plan <plan> --contract <contract>`
   - then lock the active contract with `py -3.14 tools/viewer_host_prepare_slice.py --session-id global_active_contract --plan <plan> --contract <contract>` if the begin-slice surface was not used
10. Create or update a detailed, checklisted phased plan in the repo.
   - Prefer the nearest existing plan doc.
   - Otherwise create `docs/notes/<slug>_PHASED_PLAN.md`.
   - Protocol: `docs/PHASED_PLAN_CONTINUITY_PROTOCOL.md`.
   - For new meaningful multi-step plans, prefer the current section set: `## Explicit User Asks`, `## Proof Ledger`, `## Hostile Audit`, `## Audit Passes`, and `## Audit Findings` alongside `## Current Phase` and `## Phase Checklist`.
11. Read `.github/copilot-instructions.md` after this file, not instead of it.
12. If the repo is already dirty and `SessionStart` or `UserPromptSubmit` says the session has no checkpoint baseline, do not treat the session as fresh. Run `py -3.14 tools\viewer_host_recover_crash_state.py --summary "<operator note>" --adopt-current-state`, inspect `artifacts/hooks/viewer_host_checkpoint_guard/recovery/`, then resume the stranded slice in crash-safe mode.

The VS Code task surface mirrors these commands:
- `agent: session bootstrap`
- `agent: begin work slice`
- `agent: assert phased plan sync`

## Continuity Rule

- No chat history is assumed durable.
- Every meaningful work slice needs a detailed, checklisted, phased plan in the repo.
- `HANDOFF_LOG.md` is the checkpoint index, not the detailed plan surface.
- A fresh agent should be able to resume from the repo plan, handoff log, and git state alone.

## Hard-Denial Rule

- The accepted checked-in phased plan plus checked-in slice contract are binding.
- Raw repo mutation is forbidden.
- Raw `apply_patch` is forbidden by the hook guard; use the approved `viewer_host_*` mutation wrappers.
- Raw mutating shell commands are forbidden; only read/search/build/test/check commands may run directly.
- The active slice contract is hash-locked. If it changes mid-slice, mutation and closure are blocked until `py -3.14 tools/viewer_host_revise_contract.py --session-id <session_id> --contract <contract>` re-locks it.
- Viewer-first work cannot close on helper-only or CLI-only proof. The validation receipt must record both a runtime publish command and a published-runtime proof command, or receipt writing and closure are denied.
- Closure without a machine-written contract proof receipt is forbidden.

## Strict Banner Rule

- The hook surface now emits a strict repo banner on every `SessionStart`, `UserPromptSubmit`, `PreToolUse`, `PostToolUse`, and `Stop` event.
- Treat that banner as expected always-on repo state, not as exceptional noise.
- The banner is not the enforcement layer; denial is. If the hook denies an action, route through the approved workflow wrapper instead of working around it.

## Checklist Sync Rule

- If you edit a `*_PHASED_PLAN.md` file, update `## Phase Checklist` and `## Current Phase` in the same edit.
- Do not leave a plan saying a later phase is current while earlier phases are still unchecked.
- Do not leave stale blocker text in a phased plan after another checked-in slice already closed that blocker.
- Meaningful plans that declare `## Explicit User Asks` must also carry `## Hostile Audit`, `## Audit Passes`, and `## Audit Findings`; `tools/viewer_host_assert_phased_plan_sync.py` now fails fast when those sections are missing.
- Deterministic check surface: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` or task `agent: assert phased plan sync`.

## Carryover Rule

- If a new user prompt arrives while the repo still differs from the session baseline, treat that as prior-slice closure debt first.
- Do not smear a new request across an uncheckpointed carryover slice.
- Treat prompt text as context only. Tool-generated prompts such as `Start implementation` and abrupt steering/reorientation prompts do not relax checkpoint, receipt, or carryover rules.
- The `UserPromptSubmit` warning in `.github/hooks/checkpoint_guard.json` now surfaces this condition immediately, but the agent is still responsible for resolving the carryover cleanly.
- If the repo is dirty but the current session has no baseline because VS Code or the host crashed mid-slice, do not improvise around the lockout. Run `py -3.14 tools\viewer_host_recover_crash_state.py --summary "<operator note>" --adopt-current-state` first, then continue the stranded slice one bounded command at a time.

## Mandatory Audit Rule

- After every meaningful code slice, run a distrust-first audit before treating the slice as done.
- Default posture: assume the first implementation is wrong or incomplete until a hostile review proves otherwise.
- Required audit loop:
   1. review the landed diff and the touched seams as if they contain at least one bug
   2. keep digging until a real defect, omission, or workflow mistake is found, or a second and third pass fail to find one
   3. if an audit pass finds a real issue, add the regression first, fix it, revalidate, and repeat the audit on the repaired state
- Do not wait for the user to ask for this review explicitly; it is part of normal slice closure in this repo.
- The hostile-audit state must be recorded in the active phased plan because the checkpoint wrapper and closure guard now read that plan directly; pending audit state blocks `task_complete`, `Stop`, `viewer_host_checkpoint_slice.py commit`, and `viewer_host_checkpoint_slice.py write-receipts`.
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
- If a crash or rollback leaves the repo dirty and the next session has no baseline, run `py -3.14 tools\viewer_host_recover_crash_state.py --summary "<operator note>" --adopt-current-state` before retrying prompts.
- Reuse the public task and tool surfaces instead of opening extra ad hoc terminals.

## End Of Slice

Before ending a meaningful work slice:

1. Update the active phased plan, if one exists.
2. Respect the workspace checkpoint guard hook in `.github/hooks/checkpoint_guard.json`; completion/stop is blocked if repo state differs from the session baseline.
3. Run the matching public validation profile or the equivalent checked-in scripts for the slice.
4. Append `HANDOFF_LOG.md` with `py -3.14 tools\viewer_host_append_handoff.py --commit <checkpoint_id> --score <n> "<message>"` before the final commit; reuse the token printed by `viewer_host_begin_work_slice.py` for the normal flow and keep `--resolve-last-pending` only for legacy pending-entry repair.
5. If the session advanced `HEAD`, write the machine receipts for the current committed state after the final clean commit:
   - `py -3.14 tools\viewer_host_write_validation_receipt.py --summary "<what passed>" --command "<validation cmd>" ...`
   - `py -3.14 tools\viewer_host_write_contract_proof_receipt.py --session-id global_active_contract`
   - or use `py -3.14 tools\viewer_host_checkpoint_slice.py write-receipts --session-id global_active_contract --summary "<what passed>" --command "<validation cmd>" ...`
   - for `viewer_first` runtime-visible slices, the validation receipt must include both runtime publish (`ui_app/build_vsdevcmd.cmd` or `verify: profile runtime|checkpoint`) and published-runtime proof (`tools/viewer_host_runtime_pytest_lane.py` or direct `pytest tests/test_fractal_runtime*.py` / equivalent runtime CLI pytest)
6. Run `py -3.14 tools/viewer_host_rearward_review.py` on the final clean committed `HEAD`; completion and the next slice are blocked unless the artifact is `ok`.
7. Follow the repo checkpoint discipline from `AGENT_WORKING_PROTOCOL.md`.
8. Do not say done unless the explicit closure standard above is satisfied end-to-end.
9. If the current prompt arrived while the repo already differed from the session baseline, resolve that carryover before treating any new request as a fresh slice.

Do not treat validated-but-undocumented work as finished.
