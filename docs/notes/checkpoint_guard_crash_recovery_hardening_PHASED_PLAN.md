# Checkpoint Guard Crash-Recovery Hardening

## Current Phase

Phase 4 in progress - add path-scoped checkpoint wrapper support so the clean-side recovery commit can land without absorbing unrelated carryover from the stranded zoom/root slice

## Phase Checklist

- [x] Phase 1 - add focused RED coverage for dirty repo plus missing baseline crash-recovery states and for the explicit recovery helper
- [x] Phase 2 - implement shared recovery-adoption logic, the repo-local crash recovery helper, and the actionable prompt/session-start recovery messaging
- [x] Phase 3 - update workflow docs, land the durable operator playbook, rerun the targeted validators, and append the closing handoff entry
- [ ] Phase 4 - teach the checkpoint wrapper to commit only the recovery slice paths so clean-side workflow commits do not swallow unrelated carryover

## Explicit User Asks

- [done] Implement the agreed hook crash-recovery hardening plan instead of stopping at review.
- [done] Preserve fail-closed policy while replacing the dirty-session crash dead-end with an explicit recovery lane.
- [done] Leave a detailed Markdown playbook in the repo so this failure mode is easier to diagnose and fix later.
- [done] Make the result portable enough to point other similar repos at the same recovery pattern.
- [done] Append a normal `HANDOFF_LOG.md` closeout entry stating that an external agent repaired the hook lockout and summarizing the landed workflow changes.
- [open] Commit the clean-side recovery tools/docs first without dragging the unrelated mid-slice runtime carryover into the same checkpoint.

## Presumption Loop

The local hypothesis is that the lockout is not a generic VS Code problem; it is a specific repo workflow gap where `UserPromptSubmit` sees a dirty repo, the current session has no baseline file, and the hook has no explicit recovery adoption seam. The cheapest disconfirming path is targeted guard tests plus a repo-local recovery helper that records the stranded snapshot, classifies the dirty paths against the active contract scope, and only allows resume after explicit operator adoption. If those tests or the validator surfaces still permit silent dirty resume, stale adoption reuse, or warning-only recovery, the slice is not done.

## Presumption Evidence

- The dirty-prompt hook previously blocked on `baseline is None` plus dirty repo state without offering a self-heal path for a post-crash session.
- `tools/viewer_host_checkpoint_guard.py` already owned the session-baseline and receipt logic, so the correct fix seam was shared baseline-resolution logic rather than a second workflow state machine.
- `tools.viewer_host_contract_state.file_path_is_in_contract_scope(...)` already existed, which makes crash reports immediately useful for distinguishing in-scope stranded files from contract drift.
- The repo already carries workflow-doc validation surfaces in `tests/test_agent_workflow_tools.py`, so the recovery command can be pinned into the checked-in protocol instead of living only in chat.

## Proof Ledger

- RED/GREEN: `tests/test_viewer_host_checkpoint_guard.py` now covers missing-baseline dirty blocking, stale adoption blocking, successful dirty adoption resume, and one-shot adoption artifact consumption.
- RED/GREEN: `tests/test_viewer_host_recover_crash_state.py` now covers recovery-report generation, in-scope versus out-of-scope dirty-path classification, and `--adopt-current-state` artifact writing.
- GREEN: `tools/viewer_host_recover_crash_state.py` writes durable reports under `artifacts/hooks/viewer_host_checkpoint_guard/recovery/` and records the exact recovery reason, current HEAD/branch, active contract context, handoff context, and receipt state.
- GREEN: `tools/viewer_host_checkpoint_dirty_prompt_guard.py` now turns the missing-baseline dirty case into an actionable recovery denial and permits resume only after a matching explicit recovery-adoption artifact materializes a new session baseline.
- GREEN: `tools/viewer_host_checkpoint_guard.py` now shares baseline-resolution logic across `SessionStart` and `UserPromptSubmit`, consumes the one-shot adoption artifact after use, and tells the operator when the dirty snapshot changed and the helper must be rerun.
- RED/GREEN: `tests/test_agent_workflow_tools.py` now covers path-scoped checkpoint commits, automatic `HANDOFF_LOG.md` inclusion for scoped commits, and rejection of out-of-contract scoped paths.
- GREEN: `tools/viewer_host_checkpoint_slice.py` now accepts repeated `--path` arguments for `commit`, auto-includes `HANDOFF_LOG.md` in scoped commits, and rejects scoped pathspecs that fall outside the locked contract.

## Hostile Audit

- Status: done
- Required posture: assume the first recovery lane still leaves at least one unsafe resume, stale artifact, documentation drift path, or mixed-slice checkpoint foot-gun until the targeted tests and doc assertions prove otherwise.

## Audit Passes

- [done] Pass 1 - added the missing-baseline and stale-adoption regressions first, then drove the shared baseline-resolution and prompt-guard changes green.
- [done] Pass 2 - audited the helper flow and found a real defect: writing the durable recovery report changed the live snapshot and initially broke `--adopt-current-state`; repaired the helper to ignore the freshly written report path when comparing the diagnosed snapshot against the current one.
- [done] Pass 3 - audited the workflow docs and final validation outputs, then appended the external-agent handoff summary after the repaired state proved cleanly.
- [done] Pass 4 - audited the scoped checkpoint wrapper so it cannot silently stage unrelated dirty carryover or skip the hostile-audit gate.

## Audit Findings

- [done] Real defect found and repaired: the first helper implementation diagnosed the snapshot, wrote the report, then rejected explicit adoption because the new report file itself made the repo look changed. The helper now compares against a normalized snapshot that ignores the just-written report artifact.
- [done] No additional repaired-state defect found across the workflow docs, targeted validators, or handoff closeout after the final audit pass.
- [done] Real defect found and repaired: the first scoped checkpoint implementation would have omitted the newly appended `HANDOFF_LOG.md` entry from path-scoped commits; the wrapper now adds `HANDOFF_LOG.md` automatically whenever `commit --path ...` is used.
- [done] Real defect found and repaired: the first scoped checkpoint implementation would have allowed arbitrary `--path` values outside the locked contract scope; the wrapper now rejects any scoped checkpoint path that is not in-contract before running handoff append or git mutation.

## Notes

- Expected owner files for this slice:
  - `docs/contracts/checkpoint_guard_crash_recovery_hardening.contract.json`
  - `docs/notes/checkpoint_guard_crash_recovery_hardening_PHASED_PLAN.md`
  - `docs/notes/checkpoint_guard_crash_recovery_playbook.md`
  - `tools/viewer_host_checkpoint_guard.py`
  - `tools/viewer_host_checkpoint_dirty_prompt_guard.py`
  - `tools/viewer_host_checkpoint_slice.py`
  - `tools/viewer_host_recover_crash_state.py`
  - `tools/viewer_host_session_bootstrap.py`
  - `tests/test_viewer_host_checkpoint_guard.py`
  - `tests/test_viewer_host_recover_crash_state.py`
  - `tests/test_agent_workflow_tools.py`
  - `AGENTS.md`
  - `AGENT_WORKING_PROTOCOL.md`
  - `AGENT_TERMINAL_PROTOCOL.md`
  - `.github/copilot-instructions.md`
  - `HANDOFF_LOG.md`
- Non-goals for this slice:
  - do not add a generic hook-bypass switch
  - do not relax `PreToolUse`, `PostToolUse`, or `Stop` closure semantics
  - do not repair the stranded advanced-color runtime work itself here

## Resume Point

Add the focused checkpoint-wrapper regression first, then teach `tools/viewer_host_checkpoint_slice.py commit` to honor explicit path arguments so the recovery slice can checkpoint cleanly while the unrelated zoom/root carryover remains dirty.
