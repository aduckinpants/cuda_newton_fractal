# Coloring Mode Follow-Up Slice 1 - Runtime Split And Legacy Compatibility

## Current Phase

Complete

## Phase Checklist

- [x] Phase 1 - capture the legacy load/default baseline and lock the owner seams
- [x] Phase 2 - add focused RED regressions for synthesized split-color state
- [x] Phase 3 - implement the minimal runtime split and synthesis/default helpers
- [x] Phase 4 - hostile audit the compatibility path and checkpoint the slice

## Explicit User Asks

- [open] Start implementation on the simple local signal/palette/grading path.
- [open] Keep the next coloring work separate from the other polish follow-up threads.
- [open] Avoid jumping straight to a DSL or Salticid runtime dependency.

## Presumption Loop

The likely owner is the runtime state/default/load seam, not the schema or renderer UI surface. This slice should begin in `fractal_types.h`, `fractal_family_rules.h`, and `diagnostics_state_io.cpp`, then stop as soon as old `coloring_mode` states synthesize deterministic split-color state.

Hostile review assumes the first bug risk is silent drift between legacy `coloring_mode`, family defaults, and any new internal fields. Write the narrowest regression that proves the old state still loads and the split fields are synthesized atomically before broadening the runtime model.

## Presumption Evidence

- Owner Proof: `KernelParams` currently owns only `coloring_mode` plus grading scalars, `DefaultColoringModeForFractal(...)` centralizes family defaults, and `LoadDiagnosticsStateFile(...)` still treats `coloring_mode` as the only color-selection field.
- RED Witness: the first targeted native-helper run failed because `ColorSignal`, `ColorPalette`, `ColorGradingPreset`, `ColorPipelineSelection`, `DefaultColorPipelineForFractal(...)`, and `KernelParams::color_pipeline` did not exist yet.
- Fix Proof: those runtime types and synthesis helpers now exist, and legacy diagnostics state load synthesizes `color_pipeline` from either the parsed legacy mode or the family default path.
- Hostile Review Pass 1: the first green build exposed a real omission in `fractal_derived_fields.cpp`: `ApplyFractalPresetDefaults(...)` kept resetting `coloring_mode` without refreshing the new internal `color_pipeline` field, so a focused derived-fields regression and repair landed immediately.
- Hostile Review Pass 2: diagnostics capture still writes only legacy `coloring_mode`, and a new characterization regression now proves the internal `color_pipeline` field stays runtime-only during slice 1. A follow-up grep confirmed there are still no runtime consumers of `color_pipeline` outside the synchronization seams.

## Proof Ledger

- Manual RED: current owner reads showed no internal split-color state at all; the runtime still exposed only `coloring_mode` and grading scalars.
- Checked-in regression RED: `ui_app\build_tests_vsdevcmd.cmd` first failed on the new family-rules and diagnostics-state regressions because the split-color types, helpers, and `KernelParams::color_pipeline` field were missing.
- First GREEN: after the minimal runtime pass landed in `fractal_types.h`, `fractal_family_rules.h`, and `diagnostics_state_io.cpp`, the same native helper rail passed.
- Post-green hostile finding: a second audit regression in `ui_app/tests/test_fractal_derived_fields.cpp` then proved preset/default application left `color_pipeline` stale even though `coloring_mode` was reset correctly. Repairing `ui_app/src/fractal_derived_fields.cpp` to centralize the paired default assignment made the same rail pass again.
- Second audit guard GREEN: `ui_app/tests/test_finding_archive_actions.cpp` now proves diagnostics capture preserves legacy `coloring_mode` authority and does not serialize `color_pipeline`, and the native helper rail stayed green after adding that guard.

## Notes

- Expected owner files:
  - `ui_app/src/fractal_types.h`
  - `ui_app/src/fractal_family_rules.h`
  - `ui_app/src/fractal_derived_fields.cpp`
  - `ui_app/src/diagnostics_capture.cpp`
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/tests/test_fractal_derived_fields.cpp`
  - `ui_app/tests/test_diagnostics_state_io.cpp`
  - `ui_app/tests/test_fractal_family_rules.cpp`
  - `ui_app/tests/test_finding_archive_actions.cpp`
- Non-goals:
  - do not change the schema/UI surface here
  - do not change the renderer dispatch or shipped palettes here
  - do not change the saved-state authority from `coloring_mode` yet
- Validation target:
  - focused state-load/default regressions first
  - the native helper rail after the first green
- Closure snapshot:
  - contract validation stays green at `artifacts/validation/coloring_mode_followup_slice1_runtime_split_contract.json`
  - plan sync stays green for the coloring overview and slice plans
  - code quality passed at `artifacts/coloring_mode_slice1_code_quality.json`
  - final native helper proof passed at `artifacts/coloring_slice1_final_native.log`

## Resume Point

Slice 1 is complete on `feature/coloring-mode-foundation`. The next step is checkpoint closure, merge back into `feature/coloring-mode-integration`, and then start slice 2 on its own feature branch.