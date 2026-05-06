# Advanced Color Pipeline Slice 6 - Phase And Bands Parameters

## Current Phase

Phase 4 complete - the first real phase/bands parameter seam is live, audited, and republished

## Phase Checklist

- [x] Phase 1 - capture the preview-only parameter gap and add focused RED regressions for phase and bands owner fields
- [x] Phase 2 - add runtime owner fields for the bounded phase and iteration-bands parameter set
- [x] Phase 3 - wire advanced-window apply and live snapshot import for the supported parameter paths
- [x] Phase 4 - drive pixels, persistence, and validation for the first real parameterized pipelines

## Explicit User Asks

- [open] Continue from slice 5 instead of stopping at review.
- [open] Land the working feature through the next phased slices rather than staying overly cautious.
- [open] Keep the simple fixed three-slot editor where slot choice determines the controls shown beneath it.
- [open] Avoid collapsing back into three alias dropdowns with fake knobs.

## Presumption Loop

The nearest owner seam is the isolated phase and iteration-band color math in `ui_app/src/escape_time_coloring.h`, plus the advanced-window lane parameter state already present in `ui_app/src/color_pipeline_window.h`. The current behavioral gap is that the window can now apply the live signal/palette/grading tuple, but the parameter controls for those functions are still disabled because there are no runtime owner fields behind them.

The falsifiable local hypothesis is that a small set of new `KernelParams` fields can own the current phase and iteration-bands parameter paths, that the advanced window can import/apply those fields for just the supported functions, and that the existing color helper seams can consume them without needing a broad renderer redesign. The cheapest disconfirming checks are: a RED in `ui_app/tests/test_escape_time_coloring.cpp` that fails until phase and band helper output changes when the new owner fields change, and a RED in `ui_app/tests/test_schema_binding.cpp` that fails until the advanced window applies/imports those same supported parameter values.

## Presumption Evidence

- Color Proof: `ui_app/src/escape_time_coloring.h` already isolates `phase` and `iteration_bands` color math behind small helper functions instead of spreading that logic across the whole renderer.
- Basin Proof: `ui_app/src/fractal_renderer.cu` still contains the Newton/Explaino basin-family `phase` and `iteration_bands` branches directly, so this slice must touch that narrow call site too or the new parameters would only affect escape-time families.
- Window Proof: `ui_app/src/color_pipeline_window.h` already owns per-function parameter state and live-snapshot sync, so this slice can stay local instead of inventing a new parameter editor model.
- Authority Proof: `KernelParams` in `ui_app/src/fractal_types.h` is still the one runtime color authority, and slice 5 already proved the advanced window can apply back into it.
- Persistence Proof: `ui_app/src/diagnostics_state_io.cpp` and `ui_app/src/diagnostics_capture.cpp` already persist color runtime fields, so adding a small number of optional parameter fields extends an existing seam instead of creating a new save/load model.

## Proof Ledger

- Manual anchor: read `ui_app/src/fractal_renderer.cu`, `ui_app/src/escape_time_coloring.h`, `ui_app/src/color_pipeline_window.h`, `ui_app/src/fractal_types.h`, and the focused color/window tests to lock the narrow slice.
- GREEN: focused regressions now cover phase/bands helper output changes, advanced-window live apply/resync, runtime reset defaults, runtime-walk synthesized state output, diagnostics load, and diagnostics capture persistence for the supported owner fields.
- GREEN: bounded owner fields, window import/apply mapping, persistence, and helper-math wiring landed only for the supported phase/bands paths.
- Audit finding closed: diagnostics capture initially failed the slice's promised round-trip seam because it still omitted the new phase/bands fields from saved `state.json`; a saved-state regression now covers that seam and the writer is repaired.
- Audit/validation complete: native helper tests passed on the repaired state, `tools/code_quality_audit.py --check-baseline` passed, runtime publish refreshed `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`, runtime pytest passed `68/68`, and deployed `fractal_ui.exe --validate-ui` passed.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_pipeline_slice6_phase_bands_params_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_pipeline_slice6_phase_bands_params.contract.json`
  - `ui_app/src/fractal_types.h`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/src/fractal_renderer.cu`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/src/diagnostics_capture.cpp`
  - `ui_app/src/runtime_walk_bootstrap.cpp`
  - `ui_app/src/fractal_derived_fields.cpp`
  - `ui_app/tests/test_escape_time_coloring.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
  - `ui_app/tests/test_finding_archive_actions.cpp`
  - `ui_app/tests/test_diagnostics_state_io.cpp`
  - `ui_app/tests/test_runtime_walk_bootstrap.cpp`
  - `ui_app/tests/test_runtime_reset.cpp`
- Non-goals for this slice:
  - do not generalize every currently advertised parameter path
  - do not make grade-lane parameters live yet where they conflict with the existing post-color grading controls
  - do not widen into a DSL, custom catalogs, or a general graph model
  - do not change the public schema in the main Controls panel
- Exit criteria:
  - the advanced window can apply and import real parameter values for the supported phase and iteration-bands functions
  - the supported parameter values affect pixels through the existing color helper seams
  - the supported values persist through diagnostics capture/load and reset to deterministic defaults
  - unsupported parameter paths remain clearly preview-only instead of pretending to be live

## Resume Point

Slice 6 is ready for plan/contract validation and checkpoint closure.