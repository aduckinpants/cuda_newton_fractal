# Seed Refactor Reconciliation

## Verified State

- Current working branch: `working/salt-fractal-sweep-viewer`
- Current validated head before this reconciliation slice: `beb67e0` (`Fix manual capture finding repo-root resolution`)
- Worktree status before reconciliation: clean and already pushed
- Referenced branch exists remotely as `origin/feature/seed-refactor`
- Referenced commits found on that branch:
  - `cbfe6a2`
  - `3180247`
  - `5fe0339`

## Important Git Topology Fact

`origin/feature/seed-refactor` diverges from the initial commit, not from the current working branch tip. A wholesale merge would try to replay a stale branch snapshot that still carries old diagnostics outputs, build artifacts, and outdated repo files.

That means the safe move is not "merge the whole branch and hope." The safe move is:

1. keep the current working branch as the authority for current UI/runtime work
2. inspect `feature/seed-refactor` commit-by-commit
3. backport only the missing seed-control behavior and related docs/tests
4. reject stale artifacts and old UI state files from that branch

## Open Threads

1. Restore the missing seed-control seam from `feature/seed-refactor` without regressing the current UI work.
2. Specifically recover the old `auto_increment_seed` plus `explaino_seed_rate` behavior on top of the extracted current architecture.
3. Decide separately whether `explaino_alive` should remain hidden runtime state or be surfaced again as a user control.
4. Keep saved-state compatibility: old state payloads from the current branch must still load after the backport.
5. Validate with headless tests first, then targeted manual UI review of Explaino seed controls.

## Current Working Decision

For this slice, treat `feature/seed-refactor` as a reference branch, not a merge target. Replay the missing seed-motion behavior surgically onto `working/salt-fractal-sweep-viewer`, then test, review, commit, and push from the current branch.