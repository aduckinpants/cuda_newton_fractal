# Advanced Color Pipeline Slice 7 - Full Product Refactor Start

## Current Phase

Phase 4 - widen shipped Source / Shape / Palette execution only when each row family is runtime-real, fail-closed, and deployed-proofed; the bounded offset_scale Shape-authority slice is implemented and closure-validated

## Phase Checklist

- [x] Phase 0 - recover the cumulative spec: latest explicit user wording over time is binding and the current partial shell is not closure
- [x] Phase 1 - add an explicit backend support classifier and remove draft-only rows from the shipped catalog until they are runtime-real
- [x] Phase 2 - add real runtime Shape authority and the first non-Identity shipped Shape row end to end through apply/import/runtime math/persistence
- [x] Phase 3 - checkpoint/receipt closure for the current bounded slice after native/runtime validation and hostile review
- [ ] Phase 4 - widen shipped Source / Shape / Palette execution only when each row family is runtime-real, fail-closed, and deployed-proofed

## Explicit User Asks

- [open] Stop shipping the advanced Color Pipeline window as hard-coded dropdowns with fake controls.
- [open] Rebuild the advanced editor around the discussed Factorio-style schedule metaphor instead of a fixed three-slot panel.
- [open] Model the editor as Source, Shape, and Palette lane stacks with plus-button row insertion and ordered composition.
- [open] Refactor the feature into a professional reusable backend end, not a window-local compromise.
- [open] No shipped draft-only rows or fake live controls: visible catalog rows must be runtime-real or absent.
- [open] Verify with unit plus smoke/integration proof that the feature has actually landed in the published D: runtime.
- [open] Reuse the extracted shared ImGui stack-editor seam instead of copying a second implementation.
- [open] Keep the legacy Color mode and grading controls in the main Color panel during the transition while preserving one runtime authority.

## Presumption Loop

The nearest owner seams are still the advanced-window state and rendering helper in `ui_app/src/color_pipeline_window.h`, the runtime color authority in `ui_app/src/fractal_types.h`, the legacy/pipeline legality seam in `ui_app/src/fractal_family_rules.h`, and the first programmable escape-time runtime path in `ui_app/src/escape_time_coloring.h`. The controlling product defect is now broader than layout: the shipped schedule editor still exposes Shape rows that the runtime cannot execute. That violates the cumulative spec rule that shipped rows must be real or absent.

The falsifiable local hypothesis for the first bounded implementation slice is that the shipped catalog needs an explicit backend support classifier. If the catalog is derived from runtime support instead of hard-coded wishful rows, the current shipped Shape lane should stop surfacing draft-only entries immediately, and the remaining visible entries should all classify as live-backed through the same support seam. The cheapest disconfirming check is a RED in `ui_app/tests/test_schema_binding.cpp` that fails until the shipped Shape selector stops accepting draft-only rows such as `repeat` and `posterize`.

## Presumption Evidence

- Window Proof: `ui_app/src/color_pipeline_window.h` still owns both the shipped lane catalogs and the live-bridge classification, so it is the direct seam for stopping draft-only rows from leaking into the shipped selector.
- Runtime Proof: `ui_app/src/fractal_types.h` and `ui_app/src/fractal_family_rules.h` still only model legacy-compatible live tuples. There is no real runtime Shape authority yet, which is why non-Identity Shape rows remain product debt.
- Escape-Time Proof: `ui_app/src/escape_time_coloring.h` proves the first programmable Source/Palette path is real, which means the catalog-honesty gap is isolated: the Shape selector is broader than the runtime.
- Publish Proof: `ui_app/build_vsdevcmd.cmd` republishes to `D:\salt-fractal\cuda_newton_fractal_clone\runtime\`, and the staged runtime UI validation must run against the active published executable after any shipped-catalog change.

## Proof Ledger

- Existing proof: `ui_app/tests/test_schema_binding.cpp` already proves the schedule editor shape, row-stack operations, and truthful import/apply boundaries for the currently supported live tuples.
- RED/GREEN: `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_escape_time_coloring.cpp`, `ui_app/tests/test_runtime_reset.cpp`, and `ui_app/tests/test_diagnostics_state_io.cpp` now prove the first real `offset_scale` Shape row end to end through shipped catalog exposure, live import/apply, runtime color math, reset defaults, and persisted-state round-trip.
- GREEN: `ui_app/src/fractal_types.h`, `ui_app/src/fractal_derived_fields.cpp`, `ui_app/src/enum_id_utils.h`, `ui_app/src/diagnostics_state_io.cpp`, `ui_app/src/diagnostics_capture.cpp`, `ui_app/src/color_pipeline_window.h`, and `ui_app/src/escape_time_coloring.h` now treat Shape as real runtime authority instead of a fixed Identity placeholder.
- Validation: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json` passed, `cmd /c ui_app\build_tests_vsdevcmd.cmd` passed, `cmd /c ui_app\build_vsdevcmd.cmd` published the staged runtime, staged `fractal_ui.exe --validate-ui` passed, and `py -3.14 tools/viewer_host_runtime_pytest_lane.py` passed with `68 passed`.
- Audit: hostile review found one live-snapshot consistency bug where an unmapped `color_shape` could still leave `draft_import_supported=true`; `ui_app/src/color_pipeline_window.h` now fails that case closed by clearing the bridge-supported flag before returning the snapshot.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_pipeline_slice7_catalog_runtime_binding_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_pipeline_slice7_catalog_runtime_binding.contract.json`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/fractal_types.h`
  - `ui_app/src/fractal_family_rules.h`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/tests/test_schema_binding.cpp`
  - `ui_app/tests/test_escape_time_coloring.cpp`
- Non-goals for this slice:
  - do not widen into a DSL, custom graph model, or schema redesign
  - do not remove the main Color panel as the live operator surface during this transition
  - do not pretend non-runtime-backed Shape recipes already execute in the live runtime
  - do not leave preview-only bait or raw legacy enum identities visible in the shipped catalog once the slice lands
  - do not create a second editable color authority outside `KernelParams`
- Exit criteria:
  - the shipped advanced catalog exposes only runtime-backed advanced function ids; draft-only rows are absent from the shipped selector
  - the advanced window still models Source / Shape / Palette lane stacks with ordered rows and shared row chrome
  - exact supported live recipes still import from and apply back to the live runtime after the catalog-honesty change
  - focused headless tests plus published-runtime smoke proof validate the changed shipped surface before broader runtime-shape work begins

## Resume Point

The first real `offset_scale` Shape row is now landed and validated. The next feature slice is Phase 4: widen shipped Source / Shape / Palette execution only one real row family at a time, with fail-closed live-bridge rules and published-runtime proof for each newly exposed row.