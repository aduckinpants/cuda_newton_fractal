# Advanced Color Pipeline Slice 5 - Live Apply

## Current Phase

Phase 4 complete - the advanced window now applies the live split-color tuple and the published runtime is refreshed

## Phase Checklist

- [x] Phase 1 - capture the current draft-only gap and add focused RED regressions for live apply
- [x] Phase 2 - apply the selected advanced signal/palette/grading tuple back into live `KernelParams`
- [x] Phase 3 - resync the advanced window UX around one shared live authority and remove the draft-only lie
- [x] Phase 4 - hostile-audit the live-apply boundary, validate, publish, and checkpoint the slice

## Explicit User Asks

- [done] Start implementation on the next advanced color-pipeline slice.
- [done] Stop being overly cautious and land the working feature across the next phased slices.
- [open] Keep the simple fixed three-slot editor where each selection controls which tuning surface appears beneath it.
- [open] Keep the main Color panel simple rather than turning the public surface back into three alias dropdowns.

## Presumption Loop

The nearest owner seam is still `ui_app/src/color_pipeline_window.h`, because the advanced window already owns the lane selections, the live snapshot, and the draft/live divergence messaging. The current behavioral gap is no longer architectural uncertainty; it is that the window explicitly says draft-only even though the selected lane `function_id` values already match the exact runtime enum ids used by the split-color authority.

The falsifiable local hypothesis is that the advanced window can build an exact `ColorPipelineSelection` from its three selected lane ids, validate that tuple through `TryLegacyColoringModeForPipeline(...)` plus `IsColorPipelineAllowedForFractal(...)`, then write the result into `KernelParams::color_pipeline` and `KernelParams::coloring_mode` without introducing a second authority or new persistence layer. The cheapest disconfirming check is a RED in `ui_app/tests/test_schema_binding.cpp` that fails until applying a diverged draft mutates live `KernelParams`, clears the divergence after resync, and preserves the same legality guarantees as the existing main-panel path.

## Presumption Evidence

- Owner Proof: slice 4 already owns advanced-window lane state, live snapshot sync, and divergence reporting entirely inside `ui_app/src/color_pipeline_window.h`.
- Enum Proof: the lane `function_id` strings are the exact ids exposed by `ColorSignalId(...)`, `ColorPaletteId(...)`, and `ColorGradingPresetId(...)`, so live apply can stay fail-closed without guessed mappings.
- Runtime Proof: `KernelParams::color_pipeline` and `KernelParams::coloring_mode` remain the one real runtime color authority in `ui_app/src/fractal_types.h`.
- Persistence Proof: `ui_app/src/diagnostics_state_io.cpp` and `ui_app/src/diagnostics_capture.cpp` already load/save the split-color tuple once it is present in `KernelParams`, so this slice does not need a separate draft persistence model just to land live apply.

## Proof Ledger

- Manual anchor: read `ui_app/src/color_pipeline_window.h`, `ui_app/src/main.cpp`, `ui_app/src/schema_binding.cpp`, and `ui_app/tests/test_schema_binding.cpp` to lock the narrow live-apply seam.
- RED/GREEN complete: `ui_app/tests/test_schema_binding.cpp` now proves illegal draft tuples fail closed, legal tuples apply back into `KernelParams`, and the live snapshot resync clears divergence to the applied tuple.
- GREEN complete: `ui_app/src/color_pipeline_window.h` now builds exact tuples from the selected lane ids, applies them back into live `KernelParams`, updates the window summary to an honest live-editor message, and disables preview-only parameter knobs instead of leaving fake live controls enabled.
- Wiring complete: `ui_app/src/main.cpp` now passes `dirty` into the advanced window render path so successful apply clicks trigger a rerender in the live viewer.
- Audit/validation complete: focused `test_schema_binding` compile/run passed, `tools/code_quality_audit.py --check-baseline` passed, native helper tests passed, runtime publish refreshed `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`, runtime pytest lane passed `68/68`, and `fractal_ui.exe --validate-ui` passed.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_pipeline_slice5_live_apply_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_pipeline_slice5_live_apply.contract.json`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/main.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals for this slice:
  - do not add new runtime owner fields for per-function parameter knobs yet
  - do not widen into renderer math changes for the currently unwired parameter controls
  - do not add a separate advanced draft save/load payload
  - do not change the public schema surface in the main Controls panel
- Exit criteria:
  - the advanced window can apply its selected signal/palette/grading tuple back into live `KernelParams`
  - the main panel and advanced window stay synchronized through the shared runtime color authority
  - the advanced window no longer advertises itself as draft-only for slot selection
  - focused headless tests prove live apply, post-apply resync, and fail-closed invalid tuple handling

## Resume Point

Slice 5 is ready for checkpoint closure. The next bounded slice is slice 6: add the first real parameterized pipelines so the currently preview-only per-function tuning controls begin to own real runtime fields and affect pixels for a small curated subset of functions.