# Advanced Color Pipeline Slice 1 - Shared Row Chrome Extraction

## Current Phase

Complete

## Phase Checklist

- [x] Phase 1 - confirm the narrow shared-helper boundary and add a focused RED regression
- [x] Phase 2 - extract the shared row-chrome helper with no FITS/runtime semantics inside it
- [x] Phase 3 - migrate the runtime-walk row editor onto the helper and keep behavior stable
- [x] Phase 4 - hostile-audit the helper boundary, validate, publish, and checkpoint the slice

## Explicit User Asks

- [open] Start implementation.
- [open] Build the advanced color-pipeline work around a separate custom ImGui window rather than the static schema panel.
- [open] Do not make that new window a one-off; extract a reusable row-stack/chrome helper first.
- [open] Be smarter in the initial refactor so the helper saves time long term instead of becoming a framework detour.
- [open] Treat the Salticid ImGui transpiler as design precedent only, not as a direct dependency.

## Presumption Loop

The nearest owner seam is the duplicated row-editor chrome in `ui_app/src/runtime_walk_viewer_imgui.cpp`, not `schema_binding.cpp`, the color renderer, or the Salticid transpiler. This slice should begin by proving that the two runtime-walk row editors can share a narrow helper that owns row ids, row headers, expand/collapse, enable/disable, add/remove, reorder hooks, and validation display while leaving `RuntimeWalkFitsMappingBinding` semantics external.

The first falsifiable hypothesis is local: a header-only helper plus an existing headless ImGui test surface should be enough to extract the row chrome without adding build-script churn or a second descriptor framework. The cheapest disconfirming check is a focused RED in an already-compiled ImGui test file plus the existing runtime-walk tests. If the helper requires moving FITS catalogs, binding semantics, or renderer state into the shared layer, the boundary is wrong and must be tightened before proceeding.

## Presumption Evidence

- Owner Proof: `RenderBindingWorkbench(...)` and `RenderRuntimeWalkLiveBindingWorkbench(...)` in `ui_app/src/runtime_walk_viewer_imgui.cpp` share the same row loop, tree-node chrome, remove flow, and per-row toggle/slider controls even though their catalogs and side effects differ.
- Boundary Proof: `RuntimeWalkFitsMappingBinding` in `ui_app/src/runtime_walk_bootstrap.h` already mixes strings, doubles, flags, and clamps, which means a generic helper must not own domain parameter storage such as `float params[8]`.
- Metadata Proof: `FunctionParamDescriptor` plus `UISchemaOption` and `UISchemaPredicate` already cover the label/range/help/applicability vocabulary we want later, while `RenderControlFromSchema(...)` is still tied to `BindingContext` and the global app state.
- Build Wiring Proof: `ui_app/build_tests_vsdevcmd.cmd` manually lists test translation units, so a header-only helper plus edits in an existing ImGui test file is the narrowest first move.

## Proof Ledger

- Manual anchor: read `ui_app/src/runtime_walk_viewer_imgui.cpp`, `ui_app/src/runtime_walk_bootstrap.h`, `ui_app/src/schema_binding.h`, `ui_app/src/function_descriptor.h`, and the existing ImGui tests to confirm the helper seam and the build constraints.
- Checked-in RED: the first focused `test_schema_binding` rebuild failed on the missing shared-helper API (`EnsureImGuiStackEditorRowId`, `RenderImGuiStackEditorValidationBox`, and `RenderImGuiStackEditorRowChrome`), proving the helper seam did not exist yet.
- GREEN 1: a new header-only `imgui_stack_editor.h` now owns row-id assignment, add-button chrome, row chrome, and validation rendering while staying free of FITS/runtime/color semantics.
- GREEN 2: both runtime-walk row editors now consume the shared row chrome while keeping the runtime-walk-specific parameter widgets in a local helper.
- Validation proof: the focused `test_schema_binding` harness rebuild passed, the helper regression executable passed, the direct `runtime_walk_viewer_imgui.cpp` object compile passed, `ui_app/build_tests_vsdevcmd.cmd` passed through `artifacts/advanced_color_pipeline_slice1_native_helper_tests.log`, and `ui_app/build_vsdevcmd.cmd` published `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe` through `artifacts/advanced_color_pipeline_slice1_runtime_publish.log`.
- Hostile audit pass: a targeted grep of `imgui_stack_editor.h` found no leaked domain words (`fits`, `fractal`, `cuda`, `runtime_walk`, or binding-specific field names), and the duplicated runtime-walk slider/combo widgets collapsed to a single `RenderRuntimeWalkBindingRowFields(...)` implementation.

## Notes

- Expected owner files:
  - `ui_app/src/runtime_walk_viewer_imgui.cpp`
  - `ui_app/src/runtime_walk_bootstrap.h`
  - `ui_app/src/imgui_stack_editor.h`
  - `ui_app/tests/test_schema_binding.cpp`
- Candidate supporting files:
  - `ui_app/src/runtime_walk_viewer_imgui.h`
  - `ui_app/tests/test_runtime_walk_viewer_import.cpp`
- Non-goals for this slice:
  - do not add the color-pipeline window yet
  - do not rewire the main Color panel yet
  - do not touch renderer authority or diagnostics persistence yet
  - do not build a new schema engine or a general plugin framework
- Closure snapshot:
  - contract validation artifact: `artifacts/validation/advanced_color_pipeline_slice1_row_chrome_contract.json`
  - plan sync: green before closure
  - focused helper proof: `artifacts/slice1_row_chrome_red/test_schema_binding_red.exe`
  - native helper rail log: `artifacts/advanced_color_pipeline_slice1_native_helper_tests.log`
  - runtime publish log: `artifacts/advanced_color_pipeline_slice1_runtime_publish.log`

## Resume Point

Slice 1 is complete on `feature/advanced-color-pipeline-row-chrome`. The next step is Slice 2: restore the main Color panel to one legacy `coloring_mode` control plus the existing grading controls, then add the advanced Color Pipeline entry button without letting the advanced path become a second live authority.