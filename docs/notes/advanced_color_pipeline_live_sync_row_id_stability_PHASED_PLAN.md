# Advanced Color Pipeline Live Sync Row Id Stability

## Current Phase

Complete - checkpoint commit `c88ac90` and machine proof receipts closed the live-sync row-id stability slice. The programmable draft no longer rebuilds on unchanged live sync, and this plan remains historical closure evidence only.

## Phase Checklist

- [x] Phase 0 - add focused failing coverage for unchanged live-sync row-id churn in the advanced color pipeline window state
- [x] Phase 1 - stop unchanged live sync from rebuilding the draft while preserving real external live adoption paths
- [x] Phase 2 - validate with focused helper coverage, native helpers, runtime publish, staged validate-ui, runtime pytest, contract validation, and phased-plan sync

## Explicit User Asks

- [done] Explain why the previous fixes still wasted time.
- [done] Fix the real D: runtime slider failure instead of another helper-only surrogate.
- [done] Add coverage that catches this drag-reset path before another handoff.

## Presumption Loop

The controlling seam is now `SyncColorPipelineWindowFromLiveState(...)` in `ui_app/src/color_pipeline_window.h`. The falsifiable local hypothesis is that after a supported slider edit applies live, the next frame sees `HasColorPipelineDraftEdits(...) == false` and unconditionally rebuilds the draft from `live_snapshot`, even when the live snapshot did not actually change externally. That rebuild assigns fresh row ids and can break the active drag path in the real viewer. The cheapest discriminating check is a focused regression that two consecutive syncs against the same live params must preserve row ids instead of reassigning them.

## Presumption Evidence

- The user reports the D: runtime sliders are still broken in the same way after seam-only fixes.
- `RenderColorPipelineWindow(...)` calls `SyncColorPipelineWindowFromLiveState(...)` at the start of every frame.
- `SyncColorPipelineWindowFromLiveState(...)` currently adopts into the draft whenever `!draftHasEdits`, even if the newly built live snapshot is identical to the existing one.
- `ResetColorPipelineDraftFromLiveState(...)` copies `live_snapshot.lanes` into the draft and reassigns row ids through `EnsureColorPipelineLaneRowsInitialized(...)`.

## Proof Ledger

- Completed: focused regression now proves unchanged supported live sync preserves row ids instead of rebuilding the programmable draft between frames.
- Completed: `SyncColorPipelineWindowFromLiveState(...)` now gates draft adoption on actual live snapshot changes, so unchanged live sync preserves row ids while external live changes still adopt correctly.
- Completed: focused native helpers, code quality audit (97/100), runtime publish, staged `fractal_ui.exe --validate-ui`, and runtime pytest (`68 passed`) are green after the live-sync stability fix.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_pipeline_live_sync_row_id_stability_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_pipeline_live_sync_row_id_stability.contract.json`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals:
  - do not add another drag timing workaround
  - do not widen the advanced color pipeline feature surface
  - do not reintroduce an apply checkbox or button

## Resume Point

Closed. Do not resume from this slice's old checkpoint chores. Re-enter later advanced-color work from `docs/notes/advanced_color_feature_restart_inventory.md`, `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`, and `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md` instead.
