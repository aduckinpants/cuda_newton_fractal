# Workflow CLI Failure Observability Stop-Line

## Current Phase

Phase 3 complete - the bounded workflow repair and its follow-up contract-proof evidence fix are both validated, and the slice is ready for the final checkpoint and receipt closure

## Phase Checklist

- [x] Phase 1 - lock the failure-observability gap with focused RED tests and authority setup
- [x] Phase 2 - harden the logged-command summary and task wiring with the smallest truthful repair
- [x] Phase 3 - rerun focused workflow validation, hostile-audit the repaired slice, and checkpoint it cleanly

## Explicit User Asks

- [done] Make repo-local workflow failures report truthfully instead of looking hung or silent.
- [done] Keep this stop-line bounded to workflow tooling; do not blend the paused grading implementation back into this slice.
- [done] Preserve the normal repo discipline: strict TDD, hostile review, validation rails, and clean checkpoint closure.

## Presumption Loop

The local hypothesis is that the misleading behavior is controlled by the operator-facing logged-command seam rather than the build scripts themselves: `tools/viewer_host_run_logged_command.py` prints only a terse tail when a command exits nonzero, and `.vscode/tasks.json` still wraps the build batch files with `cmd /c` even though the wrapper already normalizes direct batch launches. The cheapest disconfirming checks are a focused regression in `tests/test_viewer_host_run_logged_command.py` for explicit nonzero-exit failure classification and task-surface assertions in `tests/test_agent_workflow_tools.py` proving the validation tasks use direct batch commands.

## Presumption Evidence

- `tools/viewer_host_run_logged_command.py` currently prints `label`, `cwd`, `command`, `log`, `exit`, and a short tail, but it does not emit an explicit failure-state line for nonzero exits.
- `tests/test_viewer_host_run_logged_command.py` currently proves exit-code preservation and tail visibility, but it does not lock any explicit failure summary contract.
- `.vscode/tasks.json` still routes `verify: native helper tests` and `verify: runtime publish` through `cmd /c ui_app\build_*_vsdevcmd.cmd` even though the wrapper already resolves relative `.cmd` paths directly.
- `docs/notes/terminal_summary_workflow_repair_PHASED_PLAN.md` already records that the direct logged-command wrapper exists and was proven on real commands, so this slice should harden that existing seam instead of inventing a new runner.

## Proof Ledger

- Read-only finding: the live mismatch is local to the repo-owned wrapper/task surface; it does not require reopening the paused grading slice or redesigning the checkpoint workflow.
- Read-only finding: the smallest falsifiable first move is a RED test on `viewer_host_run_logged_command.py`'s failure summary, paired with a task-surface regression that rejects `cmd /c` for the wrapped build tasks.
- Green: `tests/test_viewer_host_run_logged_command.py` now locks an explicit `result=failure` summary for child-process nonzero exits and a distinct `result=launch-failure` summary when the command never launches.
- Green: `tests/test_agent_workflow_tools.py` now proves the wrapped native-helper and runtime-publish tasks launch the batch files directly through `tools/viewer_host_run_logged_command.py` instead of adding `cmd /c` indirection.
- Green: `py -3.14 -m pytest tests/test_viewer_host_run_logged_command.py tests/test_agent_workflow_tools.py -q --junitxml artifacts/pytest/workflow_cli_failure_observability_stopline.junit.xml`
- Green: `py -3.14 -m pytest tests/test_viewer_host_run_logged_command.py -q --junitxml artifacts/pytest/test_viewer_host_run_logged_command.junit.xml`
- Green: `py -3.14 -m pytest tests/test_agent_workflow_tools.py -q --junitxml artifacts/pytest/test_agent_workflow_tools.junit.xml`
- Green: `py -3.14 -m pytest tests/test_viewer_host_contract_proof.py -q --junitxml artifacts/pytest/test_viewer_host_contract_proof.junit.xml`
- Green: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/workflow_cli_failure_observability_stopline.contract.json --out-json artifacts/validation/workflow_cli_failure_observability_stopline_contract.json`
- Green: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py docs/notes/workflow_cli_failure_observability_stopline_PHASED_PLAN.md docs/notes/workflow_cli_friction_closure_PHASED_PLAN.md`
- Green: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- Repair proof: `tools/viewer_host_contract_proof.py` now recognizes the exact `tests/test_viewer_host_run_logged_command.py` pytest validation command, so the workflow stop-line contract can use parseable per-suite JUnit evidence instead of the earlier unrecognized combined command.

## Hostile Audit

- Status: complete
- Required posture: assume the first repair still leaves at least one misleading failure path until focused workflow tests and a hostile diff review prove otherwise.

## Audit Passes

- [done] Pass 1 - reviewed the first wrapper/task repair, found that launch failures were still collapsed into the same generic result as ordinary child-process failures, added the regression first, and repaired the wrapper summary.
- [done] Pass 2 - reran the targeted workflow validations on the repaired state and rechecked the wrapper/task seams; no additional real defect was found.
- [done] Pass 3 - reaudited the continuity surfaces around the active plan, parent workflow-friction plan, and checked-in contract; no additional stale blocker or proof mismatch remained after the repair.
- [done] Pass 4 - repaired the contract-proof evidence mismatch, reran the exact required validation commands, and re-audited closure on the repaired proof chain.

## Audit Findings

- [done] Real defect found and repaired: the first repair still reported missing executables with the same generic `result=failure` summary used for child-process nonzero exits even though the wrapper knew the command never launched. The wrapper now emits `result=launch-failure`, and the focused missing-executable regression locks that distinction.
- [done] No additional real defect found in the repaired wrapper/task/continuity seams after rerunning the focused workflow suite and rechecking the checked-in plan/contract state.
- [done] Real closure defect found and repaired after the first checkpoint attempt: the contract-proof evidence registry could not parse the stop-line slice's original combined pytest command or the `tests/test_viewer_host_run_logged_command.py` proof file, so receipt writing still failed even though the behavioral validations were green. The slice now uses exact per-suite pytest commands with canonical JUnit artifacts, and `tools/viewer_host_contract_proof.py` recognizes the new logged-command suite explicitly.

## Notes

- Expected owner files for this slice:
  - `docs/contracts/workflow_cli_failure_observability_stopline.contract.json`
  - `docs/notes/workflow_cli_failure_observability_stopline_PHASED_PLAN.md`
  - `docs/notes/workflow_cli_friction_closure_PHASED_PLAN.md`
  - `.vscode/tasks.json`
  - `tools/viewer_host_run_logged_command.py`
  - `tools/call_vsdevcmd.cmd`
  - `tests/test_viewer_host_run_logged_command.py`
  - `tests/test_agent_workflow_tools.py`
  - `AGENT_TERMINAL_PROTOCOL.md`
  - `AGENT_WORKING_PROTOCOL.md`
- Non-goals for this slice:
  - do not repair the paused advanced-color grading runtime slice here
  - do not redesign the broader checkpoint/receipt/contract stack
  - do not hide toolchain failures behind fallback behavior or warning-only prose

## Resume Point

Append the follow-up closing handoff for `ck:80e05a72`, checkpoint the repaired proof-seam follow-up, write receipts for the newest clean `HEAD`, and then return to `docs/notes/workflow_cli_friction_closure_PHASED_PLAN.md` for the final overall hostile workflow review.