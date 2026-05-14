# Advanced Color Pipeline Slice 4 - Live Draft Sync

## Current Phase

Complete - checkpoint commit `8c72364` and machine proof receipts closed the live-draft-sync slice. This plan remains historical closure evidence only and must not be read as live pre-closeout restart authority.

## Phase Checklist

- [x] Phase 1 - capture the current disconnected draft window behavior and add focused RED regressions for live-model sync
- [x] Phase 2 - import the live `KernelParams` color pipeline into the advanced window as a draft baseline without mutating runtime state
- [x] Phase 3 - expose draft-vs-live divergence inside the window while keeping the main Color panel as the sole live authority
- [x] Phase 4 - hostile-audit the live-sync boundary, validate, publish, and checkpoint the slice

## Explicit User Asks

- [done] Move forward to the next advanced color-pipeline slice.
- [done] Keep the restored main Color panel as the live authority while the advanced window becomes more real.
- [done] Continue building toward the fixed three-segment combinator editor where each selector determines the tuning controls shown beneath it.
- [done] Do not widen this slice into renderer dispatch or a second live runtime authority.

## Presumption Loop

The nearest owner seam is the draft-only advanced window in `ui_app/src/color_pipeline_window.h`, the live color authority in `KernelParams::color_pipeline` / `KernelParams::coloring_mode`, and the existing `test_schema_binding.cpp` headless ImGui harness. The next slice should stay local by binding the window to the live runtime color model as an imported draft baseline while leaving all live mutation on the existing main Color panel path.

The falsifiable local hypothesis is that `ColorPipelineWindowState` can carry an explicit live snapshot plus a draft copy derived from the real runtime color pipeline, and that `RenderColorPipelineWindow(...)` can sync that snapshot from `KernelParams` and `FractalType` without writing anything back. The cheapest disconfirming check is a RED in `test_schema_binding.cpp` that fails until a live `phase` pipeline imports exact lane ids into the draft model and later draft edits diverge from live state without mutating `KernelParams`.

## Presumption Evidence

- Owner Proof: slice 3 already owns all advanced-window state inside `ColorPipelineWindowState`, and `main.cpp` already calls `RenderColorPipelineWindow(...)` once per frame.
- Runtime Proof: `KernelParams` still owns the authoritative `coloring_mode` plus `color_pipeline` pair, and `fractal_family_rules.h` already defines the exact runtime-supported combinations.
- Enum Proof: `enum_id_utils.h` already exposes exact ids for `ColorSignal`, `ColorPalette`, `ColorGradingPreset`, and `ColoringMode`, so live sync can use explicit ids rather than inferred names.
- Boundary Proof: a read-only import plus draft/live comparison preserves the current rule that the advanced window is not yet a second live authority.

## Proof Ledger

- Manual anchor: read `ui_app/src/color_pipeline_window.h`, `ui_app/src/fractal_family_rules.h`, `ui_app/src/enum_id_utils.h`, `ui_app/src/main.cpp`, and `ui_app/tests/test_schema_binding.cpp` to lock the narrow slice.
- RED/GREEN complete: `ui_app/tests/test_schema_binding.cpp` now proves exact live `phase` import, draft/live divergence, non-mutation of `KernelParams`, live-snapshot refresh after runtime changes, and reset-to-live recovery.
- GREEN complete: `ui_app/src/color_pipeline_window.h` now builds a validated live snapshot from `KernelParams` + `FractalType`, imports exact runtime ids into the draft lanes, preserves diverged drafts across later live changes, and exposes reset-to-live plus live summary messaging in the window.
- Wiring complete: `ui_app/src/main.cpp` now calls the live-sync `RenderColorPipelineWindow(&colorPipelineWindow, view.fractal_type, &params)` overload so the advanced window mirrors the live runtime model each frame.
- Audit/validation complete: focused `test_schema_binding` compile/run passed, `tools/code_quality_audit.py --check-baseline` passed, native helper tests passed, runtime publish refreshed `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`, runtime pytest lane passed `68/68`, and `fractal_ui.exe --validate-ui` passed.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_pipeline_slice4_live_draft_sync_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_pipeline_slice4_live_draft_sync.contract.json`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/main.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals for this slice:
  - do not apply advanced draft edits back into `KernelParams`
  - do not add renderer dispatch for draft lane parameters
  - do not change the public schema or the Controls-window entry button
  - do not add save/load persistence for the advanced draft yet
- Exit criteria:
  - the advanced window imports the exact live runtime color pipeline into its three draft lanes
  - the window can show when the draft differs from live and can reset back to live
  - draft edits do not mutate `KernelParams`
  - focused headless tests prove live import and draft/live divergence behavior

## Resume Point

Closed. Do not resume from this slice's old checkpoint chores. Re-enter later advanced-color work from `docs/notes/advanced_color_feature_restart_inventory.md`, `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`, and `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md` instead.
