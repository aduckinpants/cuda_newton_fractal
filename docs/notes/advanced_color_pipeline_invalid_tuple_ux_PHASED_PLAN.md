# Advanced Color Pipeline - Invalid Tuple UX

## Current Phase

Complete - checkpoint commit `ca0f880` and machine proof receipts closed the unsupported-tuple UX slice. This plan remains historical closure evidence only and must not be read as live pre-closeout restart authority.

## Phase Checklist

- [x] Phase 1 - prove the current window presents unsupported tuples like broken actions and add a focused regression for explicit apply-state reporting
- [x] Phase 2 - surface clear draft/apply status in the advanced window and disable apply for unsupported tuples
- [x] Phase 3 - validate, audit, and checkpoint the repaired UX state

## Explicit User Asks

- [done] Make it clear what is and is not supposed to be working right now.
- [done] Stop presenting broken or non-reviewable advanced-window behavior as something to review.
- [done] Keep moving the feature forward instead of asking for review on a misleading partial state.

## Presumption Loop

The nearest owner seam is still `ui_app/src/color_pipeline_window.h`, specifically the summary/apply surface that currently shows `Apply Selected Pipeline` whenever the draft diverges, even when the chosen signal/palette/grade tuple is not one of the legal live runtime combinations.

The falsifiable local hypothesis is that the current UX confusion comes from the window not computing or displaying an explicit draft apply-state before rendering the summary controls. A small pure helper that classifies the current draft as valid-applicable vs unsupported tuple vs family-disallowed should let the window disable Apply, explain the current slice boundary, and remove the "button does nothing" failure mode. The cheapest disconfirming check is a focused headless regression in `ui_app/tests/test_schema_binding.cpp` that fails until an invalid tuple reports non-applicable status while a valid `phase` or `iteration_bands` tuple reports applicable status.

## Presumption Evidence

- Rule Proof: `ui_app/src/fractal_family_rules.h` still only permits the six exact legacy tuples.
- UI Proof: `ui_app/src/color_pipeline_window.h` currently renders `Apply Selected Pipeline` on any divergent draft without first surfacing whether the tuple is actually applicable.
- Failure Proof: `TryBuildColorPipelineSelectionFromDraft(...)` already returns an explicit unsupported-tuple error, so the missing piece is UX surfacing rather than deeper runtime ambiguity.

## Proof Ledger

- GREEN: `ui_app/tests/test_schema_binding.cpp` now proves invalid mixed tuples classify as unsupported before apply while valid phase drafts still classify as applicable.
- GREEN: `ui_app/src/color_pipeline_window.h` now surfaces explicit draft/apply status in the summary, disables Apply for unsupported tuples, and labels unsupported combo choices as `preview only` before selection.
- Audit/validation complete: native helper tests passed after both the summary/apply repair and the combo-label follow-up, `tools/code_quality_audit.py --check-baseline` passed, runtime publish refreshed `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`, and deployed `fractal_ui.exe --validate-ui` passed.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_pipeline_invalid_tuple_ux_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_pipeline_invalid_tuple_ux.contract.json`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals for this slice:
  - do not generalize more tuples into the live runtime yet
  - do not add new runtime owner fields
  - do not change the main Color panel schema surface

## Resume Point

Closed. Do not resume from this slice's old checkpoint chores. Re-enter later advanced-color work from `docs/notes/advanced_color_feature_restart_inventory.md`, `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`, and `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md` instead.
