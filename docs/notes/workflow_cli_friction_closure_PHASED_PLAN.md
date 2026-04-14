# Workflow CLI Friction Closure

## Current Phase

Phase 2 - audit deterministic closure enforcement seam

## Phase Checklist

- [x] Phase 1 - clarify prompt and closure wording
- [ ] Phase 2 - audit deterministic closure enforcement seam
- [ ] Phase 3 - improve build and validation visibility
- [ ] Phase 4 - reduce runtime pytest ambiguity
- [ ] Phase 5 - harden helper defaults and carryover ritual

## Notes

- Trigger:
  - repeated workflow friction made the agent appear to ignore user intent or repo invariants, especially around tool-generated prompts, steering interruptions, and validated-but-uncheckpointed carryover
- Scope:
  - repo instruction surfaces for carryover, checkpoint, and receipt semantics
  - prompt-warning helper wording and focused tests
  - later follow-ons for deterministic enforcement, build/test visibility, runtime pytest clarity, and helper defaults
- Phase 1 exit criteria:
  - AGENTS.md, AGENT_WORKING_PROTOCOL.md, and .github/copilot-instructions.md explicitly say tool-generated prompts such as `Start implementation` and steering/reorientation interruptions do not relax closure rules
  - prompt-warning helper text makes it explicit that prompt text is context only, not permission to skip carryover closure or validation receipts
  - focused tests lock the new warning language without changing the current guard logic
- Phase 1 completion snapshot:
  - carryover and prompt-rule wording now explicitly says tool-generated prompts such as `Start implementation` and steering/reorientation interruptions do not relax checkpoint, receipt, or carryover discipline
  - `tools/viewer_host_checkpoint_dirty_prompt_guard.py` now carries the same explicit wording in both dirty-carryover and missing-receipt warnings
  - `tests/test_viewer_host_checkpoint_guard.py` now locks that explicit wording with `Start implementation` as the representative prompt excerpt while preserving the existing guard behavior
- Phase 2 target:
  - reproduce and prove the exact closure-enforcement miss path before changing hook behavior
- Validation:
  - `py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q`
  - `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`