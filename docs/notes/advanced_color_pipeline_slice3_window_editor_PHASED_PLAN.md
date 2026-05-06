# Advanced Color Pipeline Slice 3 - Descriptor Window Editor

## Current Phase

Complete - slice 3 descriptor window editor is implemented, audited, validated, and published

## Phase Checklist

- [x] Phase 1 - capture the placeholder advanced window behavior and add focused RED regressions for a three-lane editor surface
- [x] Phase 2 - replace the placeholder body with a fixed three-lane descriptor-driven editor that reuses the shared row chrome
- [x] Phase 3 - keep the advanced window draft-only while proving lane selection swaps the parameter surface without touching renderer authority
- [x] Phase 4 - hostile-audit the draft editor boundary, validate, publish, and checkpoint the slice

## Explicit User Asks

- [done] Continue the advanced color-pipeline work after slice 2.
- [done] Preserve the restored main Color panel while the advanced window grows into the real editor surface.
- [done] Move toward the desired fixed three-segment combinator editor where each selector drives the tuning controls that appear beneath it.
- [done] Do not widen this slice into renderer integration or a second live runtime authority.

## Presumption Loop

The nearest owner seam is now the checked-in placeholder advanced window in `ui_app/src/color_pipeline_window.h` plus the existing headless ImGui harness in `ui_app/tests/test_schema_binding.cpp`. The next slice should stay local by replacing the placeholder text body with a fixed three-lane editor that uses descriptor metadata to decide which parameter controls render for the selected function in each lane.

The falsifiable local hypothesis is that a header-only window helper can own three fixed lane states, each with a selected function id and descriptor-derived parameter values, while reusing `ImGuiStackEditorRowChrome(...)` from slice 1 and avoiding any renderer/state authority changes. The cheapest disconfirming check is a RED in `test_schema_binding.cpp` that fails until a lane function selection swaps the parameter surface and the open window still renders through the shared row helper.

## Presumption Evidence

- Owner Proof: slice 2 already routes the `Color Pipeline...` button into `ColorPipelineWindowState` and `RenderColorPipelineWindow(...)`, so the placeholder window helper is the direct control seam.
- Metadata Proof: `FunctionParamDescriptor` already provides the descriptor vocabulary needed for labels, numeric bounds, defaults, enum options, and help text without inventing a second metadata format.
- Row-Chrome Proof: `imgui_stack_editor.h` already exposes stable row ids and generic tree/header chrome that fits a fixed three-lane editor without pulling domain semantics into the shared helper.
- Boundary Proof: keeping all lane selections inside `ColorPipelineWindowState` preserves the current rule that the advanced path is draft-only and not yet a second runtime authority.

## Proof Ledger

- Manual anchor: read `ui_app/src/color_pipeline_window.h`, `ui_app/src/function_descriptor.h`, `ui_app/src/imgui_stack_editor.h`, and the existing `test_schema_binding.cpp` ImGui harness to lock the narrow slice.
- RED/GREEN: `test_schema_binding.cpp` now proves the advanced window initializes exactly three fixed lanes with stable row ids and deterministic default functions.
- RED/GREEN: `test_schema_binding.cpp` now proves a lane function change swaps the descriptor-backed parameter surface and that unknown lane function ids raise an explicit validation message instead of silently falling back.
- GREEN: `ui_app/src/color_pipeline_window.h` now renders a fixed three-lane draft editor through `ImGuiStackEditorRowChrome(...)` with selector combos and descriptor-driven numeric/bool parameter controls.
- Audit repair: the hostile review found that invalid lane function ids were being skipped silently and that same-frame descriptor refresh needed to follow combo changes; both were repaired before widening validation.
- Boundary proof: the advanced window still keeps all selections inside `ColorPipelineWindowState`, shows draft-only copy, and does not touch `KernelParams` or renderer dispatch.
- Validation: focused `test_schema_binding` compile/run passed; `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json` passed; native helper tests passed; runtime publish passed and refreshed `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`; the runtime pytest lane passed `68 passed`; published `fractal_ui.exe --validate-ui` produced no output.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_pipeline_slice3_window_editor_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_pipeline_slice3_window_editor.contract.json`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals for this slice:
  - do not apply advanced window edits back into `KernelParams`
  - do not add renderer dispatch for the advanced window yet
  - do not change the public schema or the Controls-window button entry
  - do not add dynamic lane counts; keep the editor fixed at three segments
- Exit criteria:
  - the advanced window renders three fixed lane rows with stable ids and selected function labels
  - switching a lane selector swaps the bound tuning controls derived from descriptor metadata
  - the window explicitly remains draft-only and does not become a second live authority
  - focused headless tests prove the lane initialization and function-switch behavior

## Resume Point

Slice 3 is closed. The next follow-up slice can decide whether to bind this draft editor to a real color-pipeline model or keep expanding descriptor richness while the main Color panel stays the live authority.