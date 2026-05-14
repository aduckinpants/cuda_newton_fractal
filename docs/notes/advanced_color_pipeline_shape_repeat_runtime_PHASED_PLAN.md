# Advanced Color Pipeline Shape Repeat Runtime

## Current Phase

Complete - checkpoint commit `6f2dc2b` and machine proof receipts closed the repeat-runtime widening slice. This plan remains historical closure evidence only and must not be read as live pre-closeout restart authority.

## Phase Checklist

- [x] Phase 0 - capture the slider-runtime lesson in checked-in documentation and add focused RED coverage for the `repeat` Shape row
- [x] Phase 1 - land runtime-owned `repeat` Shape authority end to end through params, color math, import/apply, and shipped catalog exposure
- [x] Phase 2 - validate with focused helper coverage, native helpers, runtime publish, staged validate-ui, runtime pytest, contract validation, and phased-plan sync

## Explicit User Asks

- [done] Learn the slider runtime lesson and document it so the repo stops repeating the same mistake.
- [done] Flesh out and harden the advanced color pipeline tool instead of stopping at the drag fix.
- [done] Expand the real slider-backed function surface with correct properties and runtime-backed behavior.
- [done] Plan toward the full lane-stack backend where multiple functions can eventually work per segment.

## Presumption Loop

The smallest honest next slice after the live-sync row-id stability fix is one more runtime-real Shape row. `repeat` already exists in the advanced window metadata inside `ui_app/src/color_pipeline_window.h`, but it is not runtime-real: there is no `ColorPipelineShape` enum case, no runtime-owned parameter storage in `KernelParams`, no shape math in `ui_app/src/escape_time_coloring.h`, and no import/apply/persistence/reset coverage. The falsifiable hypothesis is that `repeat` can land as the next bounded vertical slice without inventing the future multi-function composition backend. The cheapest disconfirming checks are RED regressions in `ui_app/tests/test_escape_time_coloring.cpp` and `ui_app/tests/test_schema_binding.cpp` that fail until `repeat` is live-backed end to end.

## Presumption Evidence

- `repeat` is already declared in the shipped advanced metadata builders in `ui_app/src/color_pipeline_window.h`, including slider properties for `shape.frequency` and `shape.phase`.
- The shipped runtime currently filters it out because `IsColorPipelineFunctionRuntimeBacked("shape", "repeat")` returns false.
- `escape_time_coloring.h` only executes `identity` and `offset_scale` shape behavior today.
- Existing slice-6 and slice-7 work already established the pattern for landing one runtime-real row family at a time through `KernelParams`, import/apply, persistence, reset defaults, and focused tests.

## Proof Ledger

- Complete: `docs/notes/advanced_color_pipeline_slider_runtime_root_cause_RETROSPECTIVE.md` records the drag-reset root cause and the closure rules that now govern this feature area.
- Complete: focused helper regressions in `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_escape_time_coloring.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp`, and `ui_app/tests/test_runtime_reset.cpp` prove `repeat` is runtime-backed through shipped selector exposure, live apply/import, color math, persistence load, and reset defaults.
- Complete: validation rails passed for this slice: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json`, `cmd /c ui_app\\build_tests_vsdevcmd.cmd`, `cmd /c ui_app\\build_vsdevcmd.cmd`, staged `D:\\salt-fractal\\cuda_newton_fractal_clone\\runtime\\fractal_ui.exe --validate-ui`, and `py -3.14 tools/viewer_host_runtime_pytest_lane.py`.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_pipeline_shape_repeat_runtime_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_pipeline_shape_repeat_runtime.contract.json`
  - `docs/notes/advanced_color_pipeline_slider_runtime_root_cause_RETROSPECTIVE.md`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/fractal_types.h`
  - `ui_app/src/enum_id_utils.h`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/src/fractal_derived_fields.cpp`
  - `ui_app/src/diagnostics_capture.cpp`
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
  - `ui_app/tests/test_escape_time_coloring.cpp`
  - `ui_app/tests/test_diagnostics_state_io.cpp`
  - `ui_app/tests/test_runtime_reset.cpp`
- Non-goals:
  - do not implement multi-function-per-lane composition in this slice
  - do not widen `posterize` in parallel with `repeat`
  - do not leave `repeat` visible in the shipped selector unless the runtime path is fully real
  - do not redesign the metadata layer into a DSL during this slice

## Resume Point

Closed. Do not resume from this slice's old checkpoint chores. Re-enter later advanced-color work from `docs/notes/advanced_color_feature_restart_inventory.md`, `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`, and `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md` instead.
