# Advanced Color Pipeline Slice 7 - Runtime Binding And Draft Editor Reframe

## Current Phase

Phase 5 - schedule-editor shell and truthful live bridge are implemented and validated; checkpoint closure is still blocked on carryover dirty state

## Phase Checklist

- [x] Phase 1 - replace legacy enum aliasing with custom advanced-function ids and runtime owner-field bindings for the first escape-time path
- [x] Phase 2 - replace the fixed three-slot editor with a Source / Shape / Palette schedule shell that reuses the shared row-stack helper
- [x] Phase 3 - keep the live bridge honest: import and apply only exact single-row recipes with Identity in Shape while stacked Shape rows remain draft-only
- [x] Phase 4 - validate, republish the staged runtime, and hostile-audit the repaired schedule-editor state
- [ ] Phase 5 - checkpoint and receipt closure after existing carryover dirty state is resolved

## Explicit User Asks

- [open] Stop shipping the advanced Color Pipeline window as hard-coded dropdowns with fake controls.
- [open] Use the discussed new advanced functions instead of exposing the legacy enum list again under another name.
- [open] Verify with unit plus smoke/integration proof that the feature has actually landed in the published D: runtime.
- [open] Rebuild the advanced editor around the discussed Factorio-style schedule metaphor instead of a fixed three-slot panel.
- [open] Model the editor as Source, Shape, and Palette lane stacks with plus-button row insertion and ordered composition.
- [open] Reuse the extracted shared ImGui stack-editor seam instead of copying a second implementation.
- [open] Keep the legacy Color mode and grading controls in the main Color panel during the transition.

## Presumption Loop

The nearest owner seams are the advanced-window state and rendering helper in `ui_app/src/color_pipeline_window.h`, the first programmable escape-time runtime path in `ui_app/src/escape_time_coloring.h`, and the runtime owner fields in `ui_app/src/fractal_types.h`. The controlling product defect was not only fake live knobs; it was also the wrong layout metaphor. A fixed three-slot selector still read like the legacy dropdown surface dressed up with extra chrome instead of the schedule-style editor the user specified.

The falsifiable local hypothesis was that the correct repair had to change both the runtime-binding catalog and the draft-editor state model together: every visible advanced function and parameter path needed an explicit runtime-binding classification, and the window state needed to move from one selected function per lane to ordered Source / Shape / Palette row stacks. The cheapest disconfirming checks were a RED in `ui_app/tests/test_schema_binding.cpp` that failed until the window initialized Source / Shape / Palette starter rows and supported stack operations, plus a RED in `ui_app/tests/test_escape_time_coloring.cpp` that failed until the first smooth-escape / heatmap / contrast-lift path reacted to `color_pipeline` owner fields instead of pure `coloring_mode` aliasing.

## Presumption Evidence

- Window Proof: `ui_app/src/color_pipeline_window.h` now owns explicit row state, lane stacks, summary/apply classification, and the shared ImGui row chrome usage, so it is the direct seam for the schedule-editor shell.
- Runtime Proof: `ui_app/src/fractal_types.h` now owns the smooth-escape, heatmap, and contrast-lift owner fields alongside the earlier phase and iteration-bands fields, making the first programmable escape-time path real in `KernelParams`.
- Escape-Time Proof: `ui_app/src/escape_time_coloring.h` now resolves smooth-escape ramp, heatmap palette, and contrast-lift grading through `params.color_pipeline` owner fields rather than leaving those knobs decorative.
- Persistence Proof: `ui_app/src/diagnostics_state_io.cpp`, `ui_app/src/diagnostics_capture.cpp`, `ui_app/src/runtime_walk_bootstrap.cpp`, and the related reset coverage now round-trip the new owner fields so the advanced runtime path stays single-authority.
- Publish Proof: `ui_app/build_vsdevcmd.cmd` republishes to `D:\salt-fractal\cuda_newton_fractal_clone\runtime\`, and the staged runtime UI validation must run against the active published executable after the editor rewrite.

## Proof Ledger

- RED/GREEN: `ui_app/tests/test_schema_binding.cpp` now proves the advanced window initializes Source / Shape / Palette starter rows, supports row add/move/remove, imports the live programmable tuple during render, and surfaces the specific live-bridge reason for invalid stacks.
- RED/GREEN: `ui_app/tests/test_escape_time_coloring.cpp` now proves the first programmable smooth-escape / heatmap / contrast-lift path reacts to its live owner fields.
- GREEN: `ui_app/src/color_pipeline_window.h` now uses custom advanced function ids, a schedule-style row-stack model, and a truthful live bridge that only imports or applies exact single-row recipes with Identity in Shape.
- Audit repair: hostile review found that invalid schedule stacks were collapsing to a generic preview-only message instead of surfacing the specific bridge failure; the summary/apply path now returns the concrete error text.
- Validation: focused helper compile/run passed, native helper tests passed, runtime publish passed, published `fractal_ui.exe --validate-ui` passed, and `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report_schedule_editor.json` passed.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_pipeline_slice7_catalog_runtime_binding_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_pipeline_slice7_catalog_runtime_binding.contract.json`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/fractal_types.h`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/src/diagnostics_capture.cpp`
  - `ui_app/src/runtime_walk_bootstrap.cpp`
  - `ui_app/src/fractal_derived_fields.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
  - `ui_app/tests/test_escape_time_coloring.cpp`
  - `ui_app/tests/test_diagnostics_state_io.cpp`
  - `ui_app/tests/test_runtime_reset.cpp`
- Non-goals for this slice:
  - do not widen into a DSL, custom graph model, or schema redesign
  - do not remove the main Color panel as the live operator surface during this transition
  - do not pretend stacked Shape recipes or multi-row lane compositions already execute in the live runtime
  - do not leave preview-only bait or raw legacy enum identities visible in the shipped catalog once the slice lands
  - do not create a second editable color authority outside `KernelParams`
- Exit criteria:
  - the shipped advanced catalog exposes custom advanced function ids rather than the raw legacy enum ids
  - the advanced window models Source / Shape / Palette lane stacks with ordered rows and shared row chrome
  - exact supported single-row recipes import from and apply back to the live runtime while unsupported stacks stay explicit draft-only states
  - at least one new advanced signal/palette/grade trio is real end-to-end through `color_pipeline` composition and no longer relies on pure `coloring_mode` aliasing
  - the D: runtime proof path validates the active staged executable and schema explicitly
  - focused headless tests plus published-runtime smoke proof validate the new advanced-function path before broader rails run

## Resume Point

The schedule-editor shell is landed and validated. The next feature slice is fuller runtime composition for non-Identity Shape rows or multi-row lane recipes, but checkpoint/receipt closure has to happen after the current carryover dirty state is resolved.