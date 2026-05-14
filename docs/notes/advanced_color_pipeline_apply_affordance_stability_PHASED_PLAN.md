# Advanced Color Pipeline Apply Affordance Stability

## Current Phase

Complete - checkpoint commit `e0fa9b5` and machine proof receipts closed the persistent auto-apply checkbox slice. This plan remains historical closure evidence only and must not be read as live pre-closeout restart authority.

## Phase Checklist

- [x] Phase 0 - identify the exact summary-row seam that collapses when the draft matches live
- [x] Phase 1 - replace the visibility-toggling apply affordance with a persistent default-on checkbox and stable summary layout
- [x] Phase 2 - validate the focused UI behavior, publish proof, and close the slice cleanly

## Explicit User Asks

- [done] Stop making the apply control hide/show and move the whole UI vertically.
- [done] Replace the extra apply button workflow with a checkbox.
- [done] Leave the checkbox on by default.
- [done] Do not make the operator repeatedly click an extra button to keep edits flowing.

## Presumption Loop

The controlling seam is the advanced color pipeline summary renderer in `ui_app/src/color_pipeline_window.h`. The local hypothesis is that the vertical jump comes from conditionally removing the apply controls when the draft matches live, which changes the block height more than the slider rows beneath it. The cheapest disconfirming check is the summary function itself: if it conditionally skips the apply-row widgets in the `matches_live` state, the layout jump is explained locally and the fix should be to keep a stable control row rendered at all times.

## Presumption Evidence

- UI ownership: `RenderColorPipelineWindowSummary(...)` owns the current apply status text and the `Apply Supported Recipe` / `Reset Draft From Live` controls.
- Behavior proof: the summary branch currently renders different widget sets for `matches_live` versus other apply states, so the section height is not stable.
- Test seam: `ui_app/tests/test_schema_binding.cpp` already exercises render-time advanced color pipeline window behavior and is the cheapest focused regression surface for this affordance change.

## Proof Ledger

- GREEN: `ui_app/src/color_pipeline_window.h` now keeps the apply-control row visible, replaces the extra apply button with a persistent `Auto-apply supported recipe` checkbox, defaults that checkbox on, and auto-applies supported drafts during render instead of collapsing the layout when the draft matches live.
- GREEN: the same summary seam now reports the actual live-backed Shape row in the live-bridge text instead of hardcoding `identity`.
- GREEN: `ui_app/tests/test_schema_binding.cpp` proves the checkbox defaults on and that supported edits resync during render without a separate apply click.
- Validation: focused schema-binding logged check passed, `cmd /c ui_app\build_tests_vsdevcmd.cmd` passed, `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json` passed, `cmd /c ui_app\build_vsdevcmd.cmd` published the staged runtime, staged `fractal_ui.exe --validate-ui` passed, and `py -3.14 tools/viewer_host_runtime_pytest_lane.py` passed with `68 passed`.
- Audit: distrust-first review found one stale summary defect after the first implementation pass: the live-bridge copy still hardcoded `identity` for Shape. The summary now derives the visible Shape id from the live snapshot lanes before closure.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_pipeline_apply_affordance_stability_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_pipeline_apply_affordance_stability.contract.json`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals:
  - do not widen runtime row support in this slice
  - do not redesign the schedule editor layout beyond stabilizing the apply affordance row
  - do not reintroduce hidden apply controls behind a different toggle

## Resume Point

Closed. Do not resume from this slice's old checkpoint chores. Re-enter later advanced-color work from `docs/notes/advanced_color_feature_restart_inventory.md`, `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`, and `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md` instead.
