# Coloring Mode Follow-Up Slice 2 - Control Surface And State Migration

## Current Phase

Phase 4 - hostile audit the resulting UI/state authority and prepare the slice checkpoint

## Phase Checklist

- [x] Phase 1 - wait for slice 1 closeout and capture the current schema/state surface
- [x] Phase 2 - define the split signal/palette/grading control model and legality rules
- [x] Phase 3 - implement the schema, binding, and state migration changes
- [ ] Phase 4 - hostile audit the resulting UI/state authority and checkpoint the slice

## Explicit User Asks

- [open] Keep one obvious public coloring path rather than splintering the UI again.
- [open] Keep the thread separate from non-color follow-ups.
- [open] Stay on the simple viewer-host path rather than a DSL-first design.

## Presumption Loop

This slice should only begin after slice 1 proves the runtime can synthesize split-color state without breaking old `coloring_mode` loads. The likely owner is the schema/binding/state-I/O seam, not a new metadata registry.

Hostile review assumes the main risk is creating a second source of truth for defaults or legality. The UI filtering must still flow from runtime rules, not from standalone schema folklore.

## Presumption Evidence

- Owner Proof: slice 2 landed in the expected schema/binding/state seam: `ui/fractal_binding_surface_v1.ui_schema.json`, `ui_app/src/schema_binding.cpp`, `ui_app/src/diagnostics_state_io.cpp`, and `ui_app/src/diagnostics_capture.cpp`.
- RED Witness: the first state-migration RED proved explicit split fields did not override legacy `coloring_mode`; a second RED then showed split-only state still failed because the loader secretly required the legacy mirror.
- Fix Proof: diagnostics state load now treats `color_signal`/`color_palette`/`color_grading` as the preferred authority, diagnostics capture writes those split fields back out, schema binding now round-trips the split controls through the exact legacy-compatible pipeline selector, and the color panel exposes signal/palette/grading instead of the old single combo.
- Hostile Review Pass 1: the first post-green audit found that split-only state still failed with `Missing or invalid string field: coloring_mode`; the loader now treats legacy `coloring_mode` as fallback-only when the split fields are present.
- Hostile Review Pass 2: the second audit found that diagnostics capture still trusted stale `params.coloring_mode`; the save path now derives the optional legacy mirror from `params.color_pipeline` instead.

## Proof Ledger

- Manual RED: slice 2 started from a schema and binding surface that still exposed only the unified `coloring_mode` combo even after slice 1 introduced the internal split pipeline.
- Checked-in regression RED: `ui_app\build_tests_vsdevcmd.cmd` first failed on the split-field load precedence regression, then on the split-only load regression, then on the schema-binding split-control regression, and finally on the save-authority regression for stale legacy `coloring_mode`.
- First GREEN: the native helper rail now passes with split-field load/save migration, shared split enum ids, legality-aware schema binding, and the split signal/palette/grading controls exposed in the color panel.
- Post-green hostile finding: two real slice-2 defects were found and repaired after the first green pass: the loader's hidden legacy requirement and diagnostics capture's stale legacy mirror.

## Notes

- Expected owner files:
  - `ui/fractal_binding_surface_v1.ui_schema.json`
  - `ui_app/src/schema_binding.cpp`
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/tests/test_ui_schema.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
  - `ui_app/tests/test_diagnostics_state_io.cpp`
- Non-goals:
  - do not reopen per-family duplicated public controls
  - do not widen into Salticid, parser, or AST work

## Resume Point

Slice 2 is functionally green but not yet checkpointed. Next closure step is the slice-2 workflow finish: rerun the checked-in validation rails, append the slice handoff/receipts, then fast-forward the control-surface branch back into the integration branch before starting slice 3 family tuning.