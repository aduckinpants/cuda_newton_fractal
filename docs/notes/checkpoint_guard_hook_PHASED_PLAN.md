# Checkpoint Guard Hook

## Current Phase

Phase 4 - validation receipt gate

## Phase Checklist

- [x] Phase 1 - session-baseline hook scaffold
- [x] Phase 2 - validation and checkpoint
- [x] Phase 3 - post-tool dirty-state reminders
- [x] Phase 4 - validation receipt gate

## Notes

- Intent: enforce the repo checkpoint/closure invariant with a deterministic workspace hook instead of relying on instruction prose alone.
- Delivered:
  - `.github/hooks/checkpoint_guard.json`
  - `tools/viewer_host_checkpoint_guard.py`
  - `tests/test_viewer_host_checkpoint_guard.py`
- Enforcement surface:
  - capture the session baseline on `SessionStart`
  - deny `task_complete` when repo state differs from the session baseline
  - block session stop when repo state differs from the session baseline
- Phase 3 delivered:
  - added `PostToolUse` wiring to `.github/hooks/checkpoint_guard.json` so the checkpoint guard runs immediately after successful tool calls instead of only at completion boundaries
  - `tools/viewer_host_checkpoint_guard.py` now emits a deterministic checkpoint-debt `systemMessage` when the repo differs from the session baseline after tool use, including the triggering tool name and a truncated changed-path summary
  - focused regression coverage in `tests/test_viewer_host_checkpoint_guard.py` now locks the new `build_posttool_response(...)` behavior and the expanded hook event set
- Phase 4 delivered:
  - `tools/viewer_host_checkpoint_guard.py` now blocks closure when the session advanced `HEAD` but the current committed state lacks a validation receipt, even if the worktree is otherwise clean
  - added `tools/viewer_host_write_validation_receipt.py` as the deterministic helper that writes a head-keyed receipt under `artifacts/hooks/viewer_host_validation_receipts/` only for clean committed states
  - expanded `tests/test_viewer_host_checkpoint_guard.py` to cover missing-receipt denial, post-tool validation-debt reminders, receipt loading, and clean/dirty receipt writes; the focused suite now passes with 16 tests
- Dirty-state comparison is content-aware for unstaged and untracked files and blob-aware for staged files so same-path dirty edits cannot slip through unchanged status labels.