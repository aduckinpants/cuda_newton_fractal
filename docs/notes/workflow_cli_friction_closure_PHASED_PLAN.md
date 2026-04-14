# Workflow CLI Friction Closure

## Current Phase

Complete

## Phase Checklist

- [x] Phase 1 - clarify prompt and closure wording
- [x] Phase 2 - repair handoff helper checkpoint-id flow
- [x] Phase 3 - audit deterministic closure enforcement seam
- [x] Phase 4 - improve build and validation visibility
- [x] Phase 5 - reduce runtime pytest ambiguity
- [x] Phase 6 - harden helper defaults and carryover ritual

## Notes

- Trigger:
  - repeated workflow friction made the agent appear to ignore user intent or repo invariants, especially around tool-generated prompts, steering interruptions, and validated-but-uncheckpointed carryover
- Scope:
  - repo instruction surfaces for carryover, checkpoint, and receipt semantics
  - prompt-warning helper wording and focused tests
  - checkpoint and handoff helper defaults so final closure does not require a surprise continuity-only follow-up commit
  - later follow-ons for deterministic enforcement, build/test visibility, runtime pytest clarity, and remaining helper defaults
- Phase 1 exit criteria:
  - AGENTS.md, AGENT_WORKING_PROTOCOL.md, and .github/copilot-instructions.md explicitly say tool-generated prompts such as `Start implementation` and steering/reorientation interruptions do not relax closure rules
  - prompt-warning helper text makes it explicit that prompt text is context only, not permission to skip carryover closure or validation receipts
  - focused tests lock the new warning language without changing the current guard logic
- Phase 1 completion snapshot:
  - carryover and prompt-rule wording now explicitly says tool-generated prompts such as `Start implementation` and steering/reorientation interruptions do not relax checkpoint, receipt, or carryover discipline
  - `tools/viewer_host_checkpoint_dirty_prompt_guard.py` now carries the same explicit wording in both dirty-carryover and missing-receipt warnings
  - `tests/test_viewer_host_checkpoint_guard.py` now locks that explicit wording with `Start implementation` as the representative prompt excerpt while preserving the existing guard behavior
- Phase 2 exit criteria:
  - `tools/viewer_host_append_handoff.py` uses one shared generated `ck:` token when resolving the active pending breadcrumb and appending the final checkpoint message without an explicit `--commit`
  - the helper prints that token so the upcoming commit message can carry it, avoiding the previous handoff-follow-up commit pattern
  - repo workflow docs and bootstrap examples describe the checkpoint-id-first closure order instead of the older `--commit <hash>` flow
- Phase 2 completion snapshot:
  - `tools/viewer_host_append_handoff.py` now generates one shared `ck:` token for the resolve-plus-append path, prints it during normal and dry-run usage, and still accepts explicit `--commit` values for legacy repair flows
  - `tests/test_viewer_host_handoff_append.py` locks both token generation and token reuse across resolve and append delegated commands
  - `tools/viewer_host_session_bootstrap.py`, `tests/test_agent_workflow_tools.py`, `AGENT_WORKING_PROTOCOL.md`, and `AGENTS.md` now advertise the checkpoint-id-first closure order instead of the older hash-first pattern
  - the audit follow-up corrected `AGENTS.md` so validation still precedes handoff preparation and receipt writing
- Phase 3 target:
  - reproduce and prove the exact closure-enforcement miss path before changing hook behavior
- Phase 3 completion snapshot:
  - hostile audit confirmed the local guard only recognized `task_complete` through `payload["tool_name"]`, while the host can surface completion via fields such as `recipient_name`
  - `tools/viewer_host_checkpoint_guard.py` now normalizes tool-identification fields before deciding whether `PreToolUse` should run the completion guard, so the dirty-baseline and validation-receipt gates both activate on the real host payload shape
  - `tests/test_viewer_host_checkpoint_guard.py` now includes end-to-end `main()` regressions proving `recipient_name: functions.task_complete` blocks both dirty-state completion and clean-head-without-receipt completion
- Phase 4 completion snapshot:
  - `tools/viewer_host_session_bootstrap.py` now reads the canonical `.vscode/tasks.json` verify profiles and exposes each profile's concrete dependent steps plus artifact outputs instead of only saying to use the `verify: profile ...` tasks
  - the bootstrap report now shows where code-quality JSON, build logs, runtime publish logs, and catalog-smoke outputs land before the operator runs those rails
  - `tests/test_agent_workflow_tools.py` now locks the checkpoint and catalog profile breakdowns and includes an audit-follow-up regression proving the new profile parser respects an explicit repo root instead of only the workspace-global tasks path
- Phase 5 completion snapshot:
  - `.vscode/tasks.json` now routes `verify: runtime probe/session pytest` through `tools/viewer_host_runtime_pytest_lane.py` instead of invoking pytest directly
  - `tools/viewer_host_runtime_pytest_lane.py` now preflights the published runtime metadata, prints the active runtime plus canonical test-file list, and fails zero-pass skip-only runs instead of letting them look like successful runtime validation
  - `tests/test_viewer_host_runtime_pytest_lane.py` locks the helper's summary parsing, missing-runtime preflight, zero-pass failure mode, and normal passing behavior, while `tests/test_agent_workflow_tools.py` locks the task wiring
- Phase 6 exit criteria:
  - `tools/viewer_host_begin_work_slice.py` no longer hardcodes `--commit pending`; it must reuse the checkpoint-id generation flow from `tools/viewer_host_append_handoff.py`
  - begin-work-slice dry-run and normal output surface the generated `ck:` token so the closing handoff can reuse it explicitly with `--commit <checkpoint_id>`
  - `tools/viewer_host_session_bootstrap.py` advertises the explicit checkpoint-id close command as the default next step and keeps `--resolve-last-pending` visible only as a legacy repair path
  - focused workflow tests prove breadcrumb token generation, dry-run output, bootstrap command guidance, and token reuse with the append helper
- Phase 6 completion snapshot:
  - `tools/viewer_host_begin_work_slice.py` now builds a breadcrumb append plan through `tools/viewer_host_append_handoff.py`'s checkpoint-id helpers, so begin-slice breadcrumbs use generated `ck:` tokens instead of the older literal pending commit marker
  - begin-work-slice dry-runs and normal runs now print the generated checkpoint id plus the explicit `viewer_host_append_handoff.py --commit <checkpoint_id>` closure guidance
  - hostile audit found a real mismatch in `tools/viewer_host_session_bootstrap.py`, which still advertised `--resolve-last-pending` as the default close command even after the helper switched to explicit token reuse; the repair now makes bootstrap prefer the explicit checkpoint-id close command and keeps the pending-resolution command as a legacy-repair surface
  - `tests/test_agent_workflow_tools.py` and `tests/test_viewer_host_handoff_append.py` now lock breadcrumb plan generation, dry-run output, bootstrap close-command guidance, and begin-to-end checkpoint token reuse across the workflow helpers
- Documentation refresh snapshot:
  - `AGENTS.md`, `AGENT_WORKING_PROTOCOL.md`, `.github/copilot-instructions.md`, and `docs/PHASED_PLAN_CONTINUITY_PROTOCOL.md` now describe begin-work-slice as a session-start breadcrumb surface instead of a literal pending-breadcrumb flow
  - the workflow docs now advertise `py -3.14 tools\viewer_host_append_handoff.py --commit <checkpoint_id> --score <n> "<message>"` as the default close ritual after `viewer_host_begin_work_slice.py`, while leaving `--resolve-last-pending` documented only as a legacy repair path
  - helper help text now matches the same operator-facing story: `viewer_host_begin_work_slice.py` prints checkpoint-id reuse guidance, `viewer_host_append_handoff.py` prefers explicit checkpoint ids from begin-work-slice, and `viewer_host_write_validation_receipt.py` is described as the post-checkpoint clean-head receipt surface
- Validation:
  - `py -3.14 -m pytest tests/test_viewer_host_handoff_append.py tests/test_agent_workflow_tools.py -q`
  - `py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q`
  - `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`