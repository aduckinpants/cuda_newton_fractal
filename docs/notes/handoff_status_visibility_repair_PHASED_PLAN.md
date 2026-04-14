# Handoff Status Visibility Repair

## Current Phase

Complete - handoff status visibility repair checkpointed

## Phase Checklist

- [x] Phase 1 - prove the hidden staged-state visibility gap
- [x] Phase 2 - surface deterministic repo status after handoff append
- [x] Phase 3 - validate and checkpoint the workflow repair

## Notes

- Trigger:
  - the local handoff wrapper delegates to the mainline helper, which can auto-rotate `HANDOFF_LOG.md` and stage both the trimmed log and the new archive file.
  - that staged state is easy to miss when closing a slice, which makes repo state feel under-reported even though the helper did real index work.
- Scope:
  - repo-local `tools/viewer_host_append_handoff.py`
  - focused Python tests for repo-status reporting
  - minimal workflow documentation if needed
- Completion snapshot:
  - added `tools/viewer_host_repo_status.py`, a deterministic repo-status helper built on `git diff` plus `git ls-files --others --exclude-standard`, so repo-local workflow surfaces no longer depend on plain `git status` when checking dirty state
  - rewired `tools/viewer_host_begin_work_slice.py` and `tools/viewer_host_session_bootstrap.py` to use the shared helper for dirty-state detection, which makes them agree with the hidden-file case that plain `git status` was missing in this workspace
  - added explicit staged/unstaged/untracked repo-status capture and summary formatting to the local handoff wrapper so post-rotation archive staging is surfaced immediately after the helper runs
  - extended `tests/test_viewer_host_handoff_append.py` with a temporary-repo proof that the wrapper status logic distinguishes staged, unstaged, and untracked paths, plus a deterministic summary-format regression
  - updated `AGENT_WORKING_PROTOCOL.md` so the append-handoff workflow note documents the new status summary behavior
- Validation:
  - `py -3.14 -m pytest tests/test_viewer_host_handoff_append.py -q`
  - `py -3.14 -m pytest tests/test_agent_workflow_tools.py -q`
  - `py -3.14 tools/viewer_host_repo_status.py`
  - `py -3.14 tools/viewer_host_begin_work_slice.py --intent "workflow friction dry-run validation" --profile checkpoint --dry-run`
  - `py -3.14 tools/viewer_host_session_bootstrap.py --tail-handoff 1`
- Hostile audit note:
  - kept the fix repo-local because the surprising implicit staging behavior lives in the read-only mainline helper; the local wrapper now makes that staged state explicit instead of silently depending on operators to notice it later
- Exit criteria:
  - the local wrapper prints a deterministic repo-status summary after it runs
  - the summary surfaces staged, unstaged, and untracked paths explicitly
  - focused tests prove the summary logic against a temporary git repo
