# UI Polish Slice 2 - Color Mode Authority

## Current Phase

Phase 4 complete - one public color-mode control now owns the family-filtered UI surface

## Phase Checklist

- [x] Phase 1 - wait for Phase 0 closeout and capture the color-authority baseline
- [x] Phase 2 - define the target user-facing color-mode model and authority seam
- [x] Phase 3 - implement the schema, runtime, and focused test updates
- [x] Phase 4 - hostile audit the resulting color-mode UX and checkpoint the slice

## Explicit User Asks

- [done] Improve how the color mode is done.
- [done] Keep the UI polish work separated into durable feature slices.

## Presumption Loop

The current problem is likely an authority split: duplicated UI controls and hard-coded runtime branches make the color-mode surface harder to reason about than it needs to be. The slice should start from the existing public schema/runtime owner files and only invent new structure if a focused regression proves the current authority model cannot be cleaned up incrementally.

Hostile review assumes the current duplication is real and user-visible. Lock that duplication or mismatch with a focused regression before reshaping the control model.

## Presumption Evidence

- Owner Proof: current UI review found duplicated coloring-mode controls in the schema and hard-coded mode branches across `ui_app/src/fractal_types.h`, `ui_app/src/escape_time_coloring.h`, and `ui_app/src/fractal_renderer.cu`.
- Workflow Proof: `feature/ui-polish-color-authority` now exists as the dedicated branch for this slice, and slice 1 is already merged into `feature/ui-polish-integration`, so the color-authority work no longer needs to wait on Phase 0 or slice 1.
- RED Witness: the live schema currently exposes two separate public controls, `coloring_mode_newton` and `coloring_mode_escape`, but both bind to the same runtime path `fractal.params.coloring_mode`, which means the public authority surface is duplicated even before any runtime branching is considered.
- Fix Proof: the runtime already centralizes family-aware coloring legality and defaults in `ui_app/src/fractal_family_rules.h` via `IsColoringModeAllowedForFractal(...)` and `DefaultColoringModeForFractal(...)`, so the likely repair is to collapse the duplicated public control surface around those existing rules instead of inventing another authority layer.
- Hostile Review Pass 1: the smallest viable fix did not require a new schema-option predicate system. The color-authority seam now filters one unified `coloring_mode` option set at render time using the existing family rules instead of creating another source of truth.
- Hostile Review Pass 2: published-runtime proof confirmed the shipped schema now exposes only the unified `coloring_mode` control, while the runtime still validates and renders against the existing family-aware enum authority.

## Proof Ledger

- Manual RED: baseline inventory on `feature/ui-polish-color-authority` confirmed that the schema exposes two different `Coloring Mode` combos with different option lists even though they mutate the same `fractal.params.coloring_mode` enum.
- Checked-in regression RED: `cmd /c ui_app\build_tests_vsdevcmd.cmd` failed first with the missing `ResolveVisibleEnumOptions(...)` seam after the new test locked family-filtered visible color-mode options in `ui_app/tests/test_schema_binding.cpp`, and the schema regression in `ui_app/tests/test_ui_schema.cpp` demanded a single public `coloring_mode` control instead of duplicated `coloring_mode_newton` / `coloring_mode_escape` controls.
- First GREEN: `cmd /c ui_app\build_tests_vsdevcmd.cmd` passed after adding `ResolveVisibleEnumOptions(...)`, wiring the enum combo renderer through the existing `IsColoringModeAllowedForFractal(...)` rules, and collapsing the live schema to one public `coloring_mode` combo.
- Post-green hostile finding: the slice still needed viewer-first proof, so `verify: runtime publish`, `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe --validate-ui`, a deployed-schema check for only the unified `coloring_mode` id, `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json`, and `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` were rerun before checkpointing.

## Notes

- Branch owner:
  - active branch: `feature/ui-polish-color-authority`
  - integration base: `feature/ui-polish-integration`
- Expected owner files:
  - `ui/fractal_binding_surface_v1.ui_schema.json`
  - `ui_app/src/fractal_types.h`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/src/fractal_renderer.cu`
  - `ui_app/src/schema_binding.cpp`
- Non-goals:
  - do not broaden this slice into fully programmable color-authority experiments
  - do not change render-resolution defaults or pacing policy here
- Validation target:
  - focused tests for control visibility, enum authority, or color-branch selection
  - relevant native or runtime validation before checkpoint

## Resume Point

Slice 2 is complete on `feature/ui-polish-color-authority`. The next UI polish step is slice 3 on render resolution and pacing, not more color-authority churn.