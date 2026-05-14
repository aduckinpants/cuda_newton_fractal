# Advanced Color Pipeline Slider Active Item Smoke

## Current Phase

Complete - checkpoint commit `6c314f5` and machine proof receipts closed the slider-active-item smoke slice. This plan remains historical closure evidence only and must not be read as live pre-closeout restart authority.

## Phase Checklist

- [x] Phase 0 - add focused failing coverage for active slider drag detection in the advanced color pipeline param control seam
- [x] Phase 1 - fix active-item tracking so slider drags suppress end-of-frame apply while preserving live updates
- [x] Phase 2 - validate with focused helper coverage, native helpers, runtime publish, staged validate-ui, runtime pytest, contract validation, and phased-plan sync

## Explicit User Asks

- [done] Add a real smoke test for the slider path.
- [done] Fix the advanced color pipeline sliders so they actually slide.
- [done] Stop shipping turns where the helper tests are green but the slider interaction is still broken.

## Presumption Loop

The likely root cause is local to `RenderColorPipelineParamControl(...)` and `NoteColorPipelineCurrentItemInteraction(...)` in `ui_app/src/color_pipeline_window.h`. Each numeric control renders a slider/drag widget and then an inline numeric input. The current code records item activity only after the inline input, so an active slider drag is not reflected in `ColorPipelineRenderInteractionState::has_active_item`. If that hypothesis is correct, the end-of-frame apply helper still fires during slider drags and can reset the control path. The cheapest discriminating check is a focused regression proving that a slider-side interaction leaves `has_active_item` false today.

## Presumption Evidence

- The user still reports that the sliders do not slide even after the checkbox removal.
- `RenderColorPipelineParamControl(...)` calls `NoteColorPipelineCurrentItemInteraction(...)` only once, after the inline input widget, which makes the last ImGui item the input instead of the slider.
- `ShouldAutoApplySupportedColorPipelineDraft(...)` depends on `has_active_item` to suppress the end-of-frame apply helper during active drags.

## Proof Ledger

- Completed: focused regression coverage now proves slider-side active state survives the combined slider-plus-input seam.
- Completed: numeric slider/drag widgets record interaction state before the inline input overwrites the current-item context, so active drags keep `has_active_item` true and suppress end-of-frame apply.
- Completed: focused native helper coverage, code quality audit (97/100), runtime publish, staged `fractal_ui.exe --validate-ui`, and runtime pytest (`68 passed`) are green after the active-item fix.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_pipeline_slider_active_item_smoke_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_pipeline_slider_active_item_smoke.contract.json`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals:
  - do not add another timer or debounce hack
  - do not widen the color-pipeline feature surface
  - do not reintroduce an apply toggle

## Resume Point

Closed. Do not resume from this slice's old checkpoint chores. Re-enter later advanced-color work from `docs/notes/advanced_color_feature_restart_inventory.md`, `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`, and `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md` instead.
