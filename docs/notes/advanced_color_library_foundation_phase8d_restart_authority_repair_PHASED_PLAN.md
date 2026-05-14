# Advanced Color Library Foundation Phase 8D - Restart Authority Repair

## Current Phase

Complete - checkpoint commit `560cbd1` and machine proof receipts closed the bounded restart-authority repair slice. This plan remains historical continuity evidence only and must not be read as live pre-closeout restart authority.

## Phase Checklist

- [x] Phase 1 - open and lock this workflow-only slice; re-read handoffs, current code, and current tests to prove weighted blend, basin-default, and `neutral_finish` are already closed while `tone_map_finish` remains unshipped
- [x] Phase 2 - repair only the stale restart-authority surfaces in `docs/notes/advanced_color_feature_restart_inventory.md`, `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`, and `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md`
- [x] Phase 3 - hostile-review the repaired state with targeted repo searches, run contract/plan/audit validators, and prove the repaired state with a clean re-audit before checkpoint closure

## Explicit User Asks

- [done] Treat the stale next-lane guidance as a critical continuity bug, not wording polish.
- [done] Open a new bounded workflow/continuity slice before any restart-authority mutation and do not mutate under the closed phase8c contract.
- [done] Fix only the stale restart-authority surfaces needed to make the next feature lane truthful again.
- [done] Keep feature work, tooling work, manual archive work, and unrelated continuity churn out of scope.
- [done] Close cleanly with commit, receipts, hostile review, and a clean worktree.

## Presumption Loop

The controlling risk is stale restart authority, not missing implementation. A docs-only slice is justified only if it replaces the false weighted-blend-next routing with the actual next unshipped feature-row authority, keeps weighted blend/basin-default/neutral_finish recorded as already closed, and leaves no contradictory current-tense restart text behind in the checked-in authority surfaces.

## Presumption Evidence

- `HANDOFF_LOG.md` records `ck:5972173a` closing generic Source weighted blend, `ck:47bd4450` closing basin-default grading lane retention, and `ck:a0ce2d03` closing `neutral_finish`.
- `ui_app/src/color_pipeline_core.h`, `ui_app/src/color_pipeline_window.h`, and the grading tests now ship runtime-backed `basin_default` and `neutral_finish`.
- Repo searches still show `tone_map_finish` absent from the shipped runtime-backed grading catalog while `grade.glow` remains explicitly deferred.
- The repaired authority docs now route the next unopened feature row to `tone_map_finish`, while the weighted-blend startup packet and older handoff entries remain only as dated historical records rather than current launch authority.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` reports branch `feature/advanced-color-pipeline-draft-editor-reframe`, `HEAD=a087faf`, clean tree, and the still-active closed phase8c contract.
- Bootstrap: `py -3.14 tools/viewer_host_repo_status.py` reports `staged=none | unstaged=none | untracked=none`.
- Handoff proof: `HANDOFF_LOG.md` lines for `ck:5972173a`, `ck:47bd4450`, and `ck:a0ce2d03` already close weighted blend, basin-default, and `neutral_finish`.
- Owner proof: current runtime-backed grading ids include `contrast_lift`, `phase_finish`, `band_finish`, `basin_default`, and `neutral_finish`; `tone_map_finish` is still absent from the shipped grading catalog/runtime searches.
- Repair landed: `docs/notes/advanced_color_feature_restart_inventory.md`, `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`, and `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md` now all record weighted blend, basin-default, and `neutral_finish` as already closed and route the next unopened feature row to `tone_map_finish`.
- Supporting continuity repair landed: the restart-inventory/foundation continuity plans, the phase8c proof-ladder plan, and the weighted-blend startup packet now describe weighted blend as historical launch material rather than current next-lane authority.
- Hostile search evidence: targeted `rg` sweeps now find current next-lane routing only to `tone_map_finish`; remaining weighted-blend-next matches are confined to dated `HANDOFF_LOG.md` entries or explicitly historical closed-plan text.
- Validation complete: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/advanced_color_library_foundation_phase8d_restart_authority_repair.contract.json --out-json artifacts/validation/advanced_color_library_foundation_phase8d_restart_authority_repair_contract.json` returned `ok=true`.
- Validation complete: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` returned `OK (6 plan(s), source=dirty)` after the continuity plan repairs.
- Validation complete: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/advanced_color_library_foundation_phase8d_restart_authority_repair_PHASED_PLAN.md --out-json artifacts/validation/advanced_color_library_foundation_phase8d_restart_authority_repair_hostile_audit.json` returned `ok=true`.

## Hostile Audit

- Status: complete
- Required posture: assume the repaired docs are still lying until every checked-in restart-authority surface stops routing future work to weighted blend and the remaining next-lane text matches the actually closed slices.

## Audit Passes

- [done] Pass 1 - verify the closed weighted-blend, basin-default, and `neutral_finish` checkpoints against current code/tests instead of trusting prior summaries.
- [done] Pass 2 - re-read the repaired restart-authority docs and search the repo for any remaining checked-in claim that weighted blend is still the next unshipped feature row.
- [done] Pass 3 - validate the finished docs-only slice, then re-read the repaired state; the final targeted repo searches left only dated `HANDOFF_LOG.md` history and explicitly historical closed-plan text, and no additional real defect found in the repaired restart-authority surfaces.

## Audit Findings

- [done] Real continuity defect found: the three checked-in restart-authority surfaces still routed future agents to weighted blend even though `ck:5972173a`, `ck:47bd4450`, and `ck:a0ce2d03` had already closed weighted blend, basin-default, and `neutral_finish`.
- [done] Real stale-state defect found during hostile reread: the new phase8d plan itself still described the pre-repair weighted-blend-next state in present tense after the authority docs were fixed, so the plan had to be updated before validator closeout.
- [done] No additional real defect found after the clean re-read: the repaired authority docs all point to `tone_map_finish`, `tone_map_finish` still has no shipped runtime-backed code/test surface, and remaining weighted-blend-next hits are confined to dated handoff history or explicitly historical closed-plan text.

## Notes

- Expected owner files for this bounded slice:
  - `docs/contracts/advanced_color_library_foundation_phase8d_restart_authority_repair.contract.json`
  - `docs/notes/advanced_color_library_foundation_phase8d_restart_authority_repair_PHASED_PLAN.md`
  - `docs/notes/advanced_color_feature_restart_inventory.md`
  - `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`
  - `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md`
  - `HANDOFF_LOG.md`
- Non-goals for this slice:
  - do not implement `tone_map_finish`
  - do not implement `grade.glow`
  - do not implement `balance_void_grade`, ExplainO-BalanceVoid, or ExplainO-all
  - do not reopen manual archive work
  - do not widen into proof-ladder tooling or runtime code changes

## Resume Point

Closed. Do not resume from this slice's old checkpoint chores. Re-enter later advanced-color work from `docs/notes/advanced_color_feature_restart_inventory.md`, `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`, and `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md` instead.
