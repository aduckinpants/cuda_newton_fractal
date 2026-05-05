# UI Polish Slice 2 - Color Mode Authority

## Current Phase

Phase 1 - capture the color-authority baseline from the dedicated slice branch

## Phase Checklist

- [ ] Phase 1 - wait for Phase 0 closeout and capture the color-authority baseline
- [ ] Phase 2 - define the target user-facing color-mode model and authority seam
- [ ] Phase 3 - implement the schema, runtime, and focused test updates
- [ ] Phase 4 - hostile audit the resulting color-mode UX and checkpoint the slice

## Explicit User Asks

- [open] Improve how the color mode is done.
- [open] Keep the UI polish work separated into durable feature slices.

## Presumption Loop

The current problem is likely an authority split: duplicated UI controls and hard-coded runtime branches make the color-mode surface harder to reason about than it needs to be. The slice should start from the existing public schema/runtime owner files and only invent new structure if a focused regression proves the current authority model cannot be cleaned up incrementally.

Hostile review assumes the current duplication is real and user-visible. Lock that duplication or mismatch with a focused regression before reshaping the control model.

## Presumption Evidence

- Owner Proof: current UI review found duplicated coloring-mode controls in the schema and hard-coded mode branches across `ui_app/src/fractal_types.h`, `ui_app/src/escape_time_coloring.h`, and `ui_app/src/fractal_renderer.cu`.
- Workflow Proof: `feature/ui-polish-color-authority` now exists as the dedicated branch for this slice, and slice 1 is already merged into `feature/ui-polish-integration`, so the color-authority work no longer needs to wait on Phase 0 or slice 1.
- RED Witness: the live schema currently exposes two separate public controls, `coloring_mode_newton` and `coloring_mode_escape`, but both bind to the same runtime path `fractal.params.coloring_mode`, which means the public authority surface is duplicated even before any runtime branching is considered.
- Fix Proof: the runtime already centralizes family-aware coloring legality and defaults in `ui_app/src/fractal_family_rules.h` via `IsColoringModeAllowedForFractal(...)` and `DefaultColoringModeForFractal(...)`, so the likely repair is to collapse the duplicated public control surface around those existing rules instead of inventing another authority layer.
- Hostile Review Pass 1: pending.
- Hostile Review Pass 2: pending.

## Proof Ledger

- Manual RED: baseline inventory on `feature/ui-polish-color-authority` confirmed that the schema exposes two different `Coloring Mode` combos with different option lists even though they mutate the same `fractal.params.coloring_mode` enum.
- Checked-in regression RED: pending.
- First GREEN: pending.
- Post-green hostile finding: pending.

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

The baseline inventory is complete enough to name the first hypothesis: the public color-mode authority should be one family-aware control, not two schema controls bound to the same enum path. Next step: write the smallest failing regression that proves the duplicated schema surface or option filtering mismatch, then repair that one seam before widening scope.