# Checkpoint Guard Hook

## Current Phase

Phase 2 - validation and checkpoint

## Phase Checklist

- [x] Phase 1 - session-baseline hook scaffold
- [x] Phase 2 - validation and checkpoint

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
- Dirty-state comparison is content-aware for unstaged and untracked files and blob-aware for staged files so same-path dirty edits cannot slip through unchanged status labels.