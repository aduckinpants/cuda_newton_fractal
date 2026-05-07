# Advanced Color Pipeline Remove Checkbox Direct Live Controls

## Current Phase

Complete - direct-live supported slider updates landed, the checkbox-owned path is removed, and validation is green for checkpoint closure

## Phase Checklist

- [x] Phase 0 - prove the current checkbox/end-of-frame apply path still owns active slider drags and lock the direct-live expectation in focused coverage
- [x] Phase 1 - remove the checkbox state, apply supported numeric edits directly from the control seam, and keep end-of-frame apply only for non-active structural changes
- [x] Phase 2 - validate the direct-live fix on the focused seam, refresh published runtime proof, and close the slice cleanly

## Explicit User Asks

- [open] Get rid of the apply checkbox.
- [open] Stop the checkbox path from breaking slider behavior.
- [open] Make the color-pipeline sliders work like every other slider in the application.

## Presumption Loop

The controlling seam is still `RenderColorPipelineParamControl(...)` plus the end-of-frame apply block in `RenderColorPipelineWindow(...)` inside `ui_app/src/color_pipeline_window.h`. The falsifiable local hypothesis is that the remaining slider breakage comes from supported numeric edits still mutating only draft state during the widget, then applying the whole draft after the window render. That is not how the working panel sliders behave. The cheapest discriminating check is a focused regression that active slider interaction should suppress the end-of-frame apply helper because supported numeric edits now need a direct-live path instead of the checkbox-owned post-pass.

## Presumption Evidence

- Operator proof: the checkbox path still breaks slider behavior, while the rest of the application has no comparable apply toggle for sliders.
- Code-path proof: `RenderColorPipelineParamControl(...)` still writes only into draft `ColorPipelineParamState`, and `RenderColorPipelineWindow(...)` still applies the supported draft after the window render when the checkbox is enabled.
- Oracle proof: `ui_app/src/schema_binding.cpp` mutates live params directly from the slider control path and marks dirty/interacted during the widget, with no extra apply stage.

## Proof Ledger

- Completed: focused schema-binding coverage now proves active programmable-control drags bypass the end-of-frame apply helper.
- Completed: the checkbox state is removed and supported parameter edits apply live from the control seam while active drags suppress the old post-pass.
- Completed: focused native coverage, code quality audit (97/100), runtime publish, staged `fractal_ui.exe --validate-ui`, and runtime pytest (`68 passed`) are green for the direct-live fix.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_pipeline_remove_checkbox_direct_live_controls_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_pipeline_remove_checkbox_direct_live_controls.contract.json`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals:
  - do not add another timing heuristic
  - do not widen the shipped Source / Shape / Palette catalog
  - do not reintroduce a separate apply toggle under another name

## Resume Point

Run phased-plan sync plus contract validation, write the validation and contract-proof receipts, append the handoff entry against `ck:3cbfed67`, and close the slice on a clean worktree.