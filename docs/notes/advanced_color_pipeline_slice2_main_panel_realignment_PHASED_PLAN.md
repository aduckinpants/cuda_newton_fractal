# Advanced Color Pipeline Slice 2 - Main Panel Realignment

## Current Phase

Complete - slice 2 main-panel realignment is implemented, audited, validated, and published

## Phase Checklist

- [x] Phase 1 - capture the current split public color surface and add focused RED regressions
- [x] Phase 2 - restore the legacy coloring-mode combo as the primary public control surface
- [x] Phase 3 - add the advanced Color Pipeline entry button and persistent placeholder window state
- [x] Phase 4 - hostile-audit the control-surface boundary, validate, publish, and checkpoint the slice

## Explicit User Asks

- [done] Start implementation on slice 2.
- [done] Keep the public Color panel on one obvious legacy path plus the existing grading controls during the transition.
- [done] Add an advanced Color Pipeline entry point rather than exposing three alias dropdowns as the public UX.
- [done] Do not widen this slice into renderer integration.

## Presumption Loop

The nearest owner seam is the checked-in schema plus the Controls-window render path in `ui/fractal_binding_surface_v1.ui_schema.json` and `ui_app/src/main.cpp`, not the renderer, diagnostics persistence, or `schema_binding.cpp` action validation. This slice should begin by proving the public schema currently exposes split alias controls, then restore a single `coloring_mode` combo in the Color panel, keep the grading sliders, and inject an adjacent `Color Pipeline...` entry button that opens a small persistent placeholder window state.

The first falsifiable hypothesis is local: a header-only `color_pipeline_window.h` stub plus existing schema and ImGui test surfaces should be enough to realign the public panel without touching renderer/state authority or widening build-script churn. The cheapest disconfirming check is a RED in `test_ui_schema.cpp` for the restored legacy surface plus a RED in `test_schema_binding.cpp` for the placeholder window state/render seam.

## Presumption Evidence

- Owner Proof: the current checked-in schema still exposes `color_signal`, `color_palette`, and `color_grading` in the Color panel, while `main.cpp` already owns a special inline button seam next to `load_state`.
- Boundary Proof: `BindingContext` and the split internal state still need to exist for compatibility/state-load reasons, but the public control-surface decision can change without touching diagnostics or renderer code.
- Test Proof: `test_ui_schema.cpp` already asserts the public checked-in schema shape, and `test_schema_binding.cpp` already hosts a headless ImGui harness that can cover a header-only placeholder advanced-window seam without build-script changes.
- Non-goal Proof: renderer integration remains deferred because the current action is strictly public-panel realignment and placeholder entry wiring.

## Proof Ledger

- Manual anchor: read `ui/fractal_binding_surface_v1.ui_schema.json`, `ui_app/src/main.cpp`, `ui_app/tests/test_ui_schema.cpp`, and `ui_app/tests/test_schema_binding.cpp` to lock the public-panel seam and the narrow placeholder-window seam.
- RED/GREEN: `test_ui_schema.cpp` now proves the checked-in public schema exposes one `coloring_mode` combo, keeps `color_grading`, and removes public `color_signal` / `color_palette` controls.
- RED/GREEN: `test_schema_binding.cpp` now proves the placeholder `ColorPipelineWindow` seam opens and renders, and that public `coloring_mode` plus `color_grading` edits still land on coherent runtime-supported pipelines.
- GREEN: `ui/fractal_binding_surface_v1.ui_schema.json` now restores the legacy `coloring_mode` combo, keeps `color_grading`, and removes the public split alias controls.
- GREEN: `ui_app/src/main.cpp` now injects a `Color Pipeline...` entry next to the legacy coloring-mode combo and opens the header-only placeholder `ColorPipelineWindowState` seam without renderer changes.
- Audit: the hostile review checked that `color_grading` still routes through `ApplySelectedColorPipeline(...)` rather than becoming a second unsynchronized authority, then added a focused regression for that pairing.
- Validation: focused compile/run of `test_schema_binding` and `test_ui_schema` passed; `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json` passed after a local WinMain line-count repair; native helper tests passed; runtime publish passed and refreshed `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`; the runtime pytest lane passed `68 passed`; published `fractal_ui.exe --validate-ui` produced no output.

## Notes

- Expected owner files:
  - `ui/fractal_binding_surface_v1.ui_schema.json`
  - `ui_app/src/main.cpp`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/tests/test_ui_schema.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals for this slice:
  - do not add the real Color Pipeline lane editor yet
  - do not add renderer authority switching yet
  - do not touch diagnostics save/load semantics yet
  - do not add a new schema action path if the existing inline-button seam is sufficient
- Exit criteria:
  - the main Color panel shows one legacy `coloring_mode` combo plus the existing grading controls
  - the split alias dropdowns are no longer the public operator-facing surface
  - the `Color Pipeline...` entry opens a persistent placeholder window state in the published runtime
  - focused schema and ImGui RED/GREEN proofs exist

## Resume Point

Slice 2 is closed. The next follow-up slice can replace the placeholder `Color Pipeline` window body with the descriptor-driven lane editor while keeping the restored main-panel surface unchanged.