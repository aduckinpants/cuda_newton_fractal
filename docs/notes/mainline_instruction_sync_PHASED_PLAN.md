# Mainline Instruction Sync

## Current Phase

Complete - mainline instruction sync checkpointed

## Phase Checklist

- [x] Phase 1 - audit mainline instruction and hook deltas
- [x] Phase 2 - port applicable viewer-host workflow updates

## Notes

- Intent: inspect the mainline Salticid instruction and hook surfaces, then copy only the changes that fit this viewer-host satellite repo.
- Boundaries:
  - do not copy mainline-only rules that depend on missing tools, tasks, or repo structure
  - prefer repo-local adapters and references to existing viewer-host tooling where possible
  - keep workflow updates explicit about what is enforced by hooks versus what is instruction prose only
- Phase 1 findings:
  - mainline now carries a dedicated `AGENT_TERMINAL_PROTOCOL.md` with concrete terminal-crash avoidance rules
  - mainline adds stronger dirty-carryover guidance through `UserPromptSubmit` and `PreToolUse` checkpoint-carryover hooks
  - mainline `AGENTS.md` and `copilot-instructions.md` now emphasize crash-safe mode and resolving prior dirty slices before unrelated work
  - the mainline bootstrap hook stack is not directly portable because this repo lacks the broader bootstrap-hook tools and different CLI surfaces
- Phase 2 target:
  - add any missing viewer-host instruction text that clearly applies here
  - port lightweight checkpoint-carryover hook behavior if it can be implemented with existing viewer-host tools and repo policy
  - preserve this repo's repo-specific validation, checkpoint, and cross-repo boundaries
- Delivered in Phase 2:
  - added `AGENT_TERMINAL_PROTOCOL.md` as the viewer-host terminal-discipline surface adapted from the mainline rules but pointed at repo-local logged-command and validation flows
  - updated `AGENTS.md`, `AGENT_WORKING_PROTOCOL.md`, and `.github/copilot-instructions.md` with the applicable carryover, crash-safe-mode, and terminal-protocol guidance
  - added a repo-local `UserPromptSubmit` warning hook via `.github/hooks/checkpoint_guard.json` and `tools/viewer_host_checkpoint_dirty_prompt_guard.py`
  - extended `tests/test_viewer_host_checkpoint_guard.py` to prove the new prompt-warning hook wiring and both carryover cases
- Audit repair after the initial Phase 2 implementation:
  - the first prompt-warning hook only warned on dirty baseline drift and missed clean-but-unreceipted committed carryover after `HEAD` advanced
  - extended the hook to warn on missing validation receipts for advanced committed state and added focused regression coverage for that case
- Validation achieved for Phase 2:
  - `py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q`
  - `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- Exit criteria:
  - applicable instruction deltas are copied into the viewer-host repo without importing mainline-only assumptions
  - any new hook wiring is backed by repo-local tools and validated by focused tests
  - the active workflow docs call out what changed and why it applies here