# Workflow CLI Friction Closure

## Current Phase

Phase 4 - improve build and validation visibility

## Phase Checklist

- [x] Phase 1 - clarify prompt and closure wording
- [x] Phase 2 - repair handoff helper checkpoint-id flow
- [x] Phase 3 - audit deterministic closure enforcement seam
- [ ] Phase 4 - improve build and validation visibility
- [ ] Phase 5 - reduce runtime pytest ambiguity
- [ ] Phase 6 - harden helper defaults and carryover ritual

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
- Validation:
  - `py -3.14 -m pytest tests/test_viewer_host_handoff_append.py tests/test_agent_workflow_tools.py -q`
  - `py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q`
  - `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`