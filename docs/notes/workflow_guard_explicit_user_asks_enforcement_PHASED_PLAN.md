# Workflow Guard Explicit User-Asks Enforcement

## Current Phase

Phase 3 complete - carryover prompts now hard-block, task completion and stop now deny open explicit user asks in the active phased plan, and the separate paced-loop follow-up slice is seeded on disk

## Phase Checklist

- [x] Phase 1 - add focused guard regressions for carryover prompt blocking and open-explicit-ask completion blocking
- [x] Phase 2 - harden the checkpoint guard and dirty-prompt guard to enforce those workflow rules deterministically
- [x] Phase 3 - rerun the focused workflow tests, seed the separate paced-loop follow-up slice docs, and checkpoint the slice cleanly

## Explicit User Asks

- [done] Make it impossible for the workflow to silently ignore explicit user asks and then claim done anyway.
- [done] Make it impossible to keep acting on new prompts while closure debt from the prior slice still exists.
- [done] Seed the separate bounded paced-loop stop-threshold slice before moving on.

## Presumption Loop

The local hypothesis is that the existing guard stack still leaves two escape hatches: `UserPromptSubmit` only warns when there is dirty carryover or missing receipts, and `task_complete`/`Stop` do not inspect the active phased plan's `## Explicit User Asks` section. The cheapest disconfirming checks are focused Python tests around `build_userprompt_response(...)`, `build_pretool_response(...)`, and `build_stop_response(...)` in the checkpoint guard suite.

## Presumption Evidence

- `tools/viewer_host_checkpoint_dirty_prompt_guard.py` currently returns `continue=True` even when carryover debt exists, which turns a hard repo rule into a warning.
- `tools/viewer_host_checkpoint_guard.py` blocks dirty baselines and missing receipts, but does not read the active phased plan's `## Explicit User Asks` section before allowing `task_complete` or `Stop`.
- The repo's current phased plans already encode explicit user asks with `[open]` markers, so the missing piece is deterministic enforcement rather than a new documentation format.

## Proof Ledger

- Read-only finding: the current guard stack already owns the exact hook seams needed to block both patterns; this does not require a new architecture surface.
- Read-only finding: the separate paced-loop stop-threshold follow-up can be opened now as checked-in plan/contract docs without mixing its bugfix implementation into this workflow-hardening slice.
- Proof: `tests/test_viewer_host_checkpoint_guard.py` now covers hard carryover prompt blocking plus denial of `task_complete` and `Stop` while the active phased plan still has `[open]` explicit user asks.
- Proof: `tools/viewer_host_checkpoint_dirty_prompt_guard.py` now returns `continue=false` when dirty carryover or missing-receipt debt exists, turning the prompt hook from warning-only to a hard stop.
- Proof: the separate paced-loop follow-up slice is seeded at `docs/notes/explaino_sidecar_paced_loop_stop_threshold_PHASED_PLAN.md` and `docs/contracts/explaino_sidecar_paced_loop_stop_threshold.contract.json`.

## Notes

- Expected owner files for this pass:
  - `docs/contracts/workflow_guard_explicit_user_asks_enforcement.contract.json`
  - `docs/notes/workflow_guard_explicit_user_asks_enforcement_PHASED_PLAN.md`
  - `docs/contracts/explaino_sidecar_paced_loop_stop_threshold.contract.json`
  - `docs/notes/explaino_sidecar_paced_loop_stop_threshold_PHASED_PLAN.md`
  - `tools/viewer_host_checkpoint_guard.py`
  - `tools/viewer_host_checkpoint_dirty_prompt_guard.py`
  - `tests/test_viewer_host_checkpoint_guard.py`
- Non-goals for this pass:
  - do not fix the paced-loop runtime bug itself here
  - do not redesign the broader contract schema
  - do not weaken any existing checkpoint, receipt, or contract-proof guard

## Resume Point

Checkpoint this validated workflow-hardening slice cleanly. The next software slice, if resumed, is the separate paced-loop stop-threshold bug documented in the seeded follow-up plan and contract.