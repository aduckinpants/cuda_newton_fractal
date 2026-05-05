# UI Polish Slice 3 - Render Resolution And Pacing

## Current Phase

Phase 1 - capture the resolution/pacing baseline on the dedicated slice-3 branch

## Phase Checklist

- [ ] Phase 1 - capture the resolution/pacing baseline on the dedicated slice-3 branch
- [ ] Phase 2 - define the target default resolution and steady-state pacing policy
- [ ] Phase 3 - implement the runtime, UI, and focused test updates
- [ ] Phase 4 - hostile audit the resulting quality/performance behavior and checkpoint the slice

## Explicit User Asks

- [open] Raise the starting render resolution from the current low default.
- [open] Treat render-resolution and pacing cleanup as its own feature slice.
- [done] Merge slice 2 into the integration branch before starting slice 3.

## Presumption Loop

The likely problem is a mismatch between startup defaults, reset defaults, and the interaction-preview pacing policy. The slice should begin at the current owner files for runtime reset, viewport sizing, and render pacing, then move outward only if a focused regression shows the real decision point lives elsewhere.

Hostile review assumes the current low-resolution startup behavior is real, but the fix is not just "make the numbers bigger." First lock the mismatch between startup resolution, reset resolution, and pacing behavior with focused tests.

## Presumption Evidence

- Owner Proof: current UI review found a mismatch between `ui_app/src/runtime_reset.cpp`, `ui_app/src/viewer_render_pacing.cpp`, and the startup window/render behavior in `ui_app/src/main.cpp`.
- Fix Proof: `feature/ui-polish-color-authority` is now merged into `feature/ui-polish-integration`, and `feature/ui-polish-resolution-pacing` is the active branch for the bounded slice-3 follow-up.
- RED Witness: pending.
- Hostile Review Pass 1: pending.
- Hostile Review Pass 2: pending.

## Proof Ledger

- Manual RED: pending.
- Checked-in regression RED: pending.
- First GREEN: pending.
- Post-green hostile finding: pending.

## Notes

- Expected owner files:
  - `ui_app/src/runtime_reset.cpp`
  - `ui_app/src/viewer_render_pacing.cpp`
  - `ui_app/src/main.cpp`
  - `ui/fractal_binding_surface_v1.ui_schema.json`
  - `ui_app/tests/test_runtime_reset.cpp`
  - `ui_app/tests/test_viewer_render_pacing.cpp`
  - `ui_app/tests/test_ui_schema.cpp`
- Non-goals:
  - do not redesign color-mode authority here
  - do not mix this slice with the schema-domain cleanup except where validation proves a shared control surface is required
- Validation target:
  - focused tests for defaults and pacing behavior
  - relevant native helper or runtime validation before checkpoint

## Resume Point

The dedicated slice-3 branch and contract are now in place. Capture the current startup, reset, and interaction-preview behavior first, then write the smallest failing regression that proves the resolution/pacing mismatch.