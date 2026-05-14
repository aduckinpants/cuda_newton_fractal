# Advanced Color Pipeline Shipped Draft Label Truth Repair

## Current Phase

Complete - shipped-vs-draft support-text truth now matches the current shipped advanced-color authority in the checked-in editor support surfaces, and the bounded native proof plus hostile audit completed cleanly on this head.

## Phase Checklist

- [x] Phase 1 - add focused REDs that prove shipped Source or Grading candidates still classify as draft-only or unsupported where current repo authority says they are shipped, and prove unsupported candidates still remain draft-only.
- [x] Phase 2 - repair the support-state and label path so shipped rows and supported tuples stop reading as draft-only without widening unsupported tuples or reopening non-goals.
- [x] Phase 3 - rerun the phase8c native rails, confirm no extra runtime witness is needed for this bounded message-only truth repair, complete hostile audit, and leave the committed plan free of stale closeout text.

## Explicit User Asks

- [done] Repair false `("draft only")` labeling for shipped advanced-color pipeline rows and tuples.
- [done] Treat weighted Source composition, `basin_default`, `neutral_finish`, `tone_map_finish`, `grade_glow`, and `balance_void_grade` as shipped authority where the repo already says they are shipped.
- [done] Keep unsupported rows or tuples truthfully draft-only; do not remove draft labeling globally or promote unsupported tuples to shipped.
- [done] Make editor candidate labels, apply-state messages, and live bridge truth agree.
- [done] Stay out of manual archive work, historical `234919_563__explaino_inertial` archaeology, ExplainO-BalanceVoid, ExplainO-all, hooks, crash recovery, anti-lie tooling, and unrelated UI polish.

## Proof Ledger

- Bootstrap on 2026-05-14 proved this repo is on `feature/advanced-color-pipeline-draft-editor-reframe` at clean `HEAD=5c26051`, with closed prior contract `advanced_color_pipeline_live_drag_runtime_restore` still locked and therefore unable to authorize new mutation.
- Launch authority reread before this slice: `docs/notes/advanced_color_feature_restart_inventory.md`, `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`, `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md`, `docs/notes/advanced_color_library_foundation_oracle_and_inventory.md`, `docs/notes/advanced_color_library_foundation_phase8g_balance_void_grade_owner_proof_PHASED_PLAN.md`, `docs/notes/advanced_color_pipeline_active_drag_and_palette_grading_preserve_PHASED_PLAN.md`, `docs/notes/advanced_color_pipeline_live_drag_runtime_restore_PHASED_PLAN.md`, and `docs/notes/advanced_color_closed_slice_truth_repair_PHASED_PLAN.md` all agree that weighted Source stacking plus `basin_default`, `neutral_finish`, `tone_map_finish`, `grade_glow`, and `balance_void_grade` sit inside the current shipped boundary.
- `ui_app/src/color_pipeline_window.h` appends `(draft only)` whenever `DescribeColorPipelineCandidateApplyState(...)` returns anything other than `can_apply` or `matches_live`, so the slice first had to prove whether a shipped supported candidate still fell into that branch on current HEAD.
- The checked-in RED now lives in `ui_app/tests/test_schema_binding.cpp`: supported `neutral_finish`, `tone_map_finish`, `grade_glow`, and `balance_void_grade` candidate switches on a shipped smooth-escape tuple must stay `can_apply`; shipped `basin_default` on an aligned root-basin tuple must stay `matches_live`; the shipped two-row weighted Source stack must stay `can_apply`; and unsupported Grading guidance must explicitly list `balance_void_grade` inside the shipped Grading boundary.
- The first repro narrowed the defect truthfully: the supported shipped candidate-state assertions already stayed green on current HEAD, so the live bug was not a deeper support-state branch in those covered contexts. The actual failing RED was the stale unsupported Grading guidance, which still omitted `balance_void_grade` from the shipped stack list.
- The repair is bounded to `ui_app/src/color_pipeline_window.h`: the unsupported Grading guidance now lists `balance_void_grade`, and the summary banner no longer lies that the live apply bridge only supports one enabled Source row and one enabled Palette row after weighted Source and Palette stack shipping.
- Validation on the repaired state is currently green through the phase8c native ladder:
  - `cmd /c ui_appuild_tests_vsdevcmd.cmd advanced_color_grading_red`
  - `cmd /c ui_appuild_tests_vsdevcmd.cmd advanced_color_grading_owner`

## Hostile Audit

- Status: complete
- Required posture: assume the bug is deeper than one label string until the candidate-state seam, apply-state messages, shipped support lists, and any required runtime witness all agree on the same repaired head.

## Audit Passes

- [x] Pass 1 - add the first REDs and confirm a real shipped-vs-draft defect on current HEAD. Result: the supported shipped candidate-state assertions stayed green in the covered smooth-escape, root-basin, and weighted Source seams, but the unsupported Grading guidance failed because it still omitted `balance_void_grade` from the shipped stack list.
- [x] Pass 2 - clean re-read of the repaired support-state and label seams after green owner rails. Result: the repaired `balance_void_grade` support text stayed aligned with the shipped Grading boundary, the stale one-row Source / Palette summary banner was removed, and no additional shipped candidate omission or unsupported false promotion surfaced.
- [x] Pass 3 - clean re-read of the repaired diff, validator outputs, and stale-support grep after the native green rails. Result: no additional real defect, stale shipped-boundary wording, or scope-widening claim surfaced in the repaired state.

## Audit Findings

- [x] Real defect found: `TryBuildColorPipelineSelectionFromDraft(...)` still emitted an outdated Grading support list that excluded shipped `balance_void_grade`, even though the runtime-backed support filter already included it.
- [x] Real UI truth defect found: the summary banner still claimed the live apply bridge only supported one enabled Source row and one enabled Palette row after weighted Source and Palette stack shipping closed.
- [x] Narrower-than-feared scope: the first shipped-candidate REDs proved the covered supported candidate branches were already classifying correctly, so this slice did not need deeper support-state logic changes for those contexts.
- [x] Clean re-read: after the support-text repair and native green rails, no additional real defect or hidden scope widening surfaced in the shipped candidate-state seams covered by this slice.
