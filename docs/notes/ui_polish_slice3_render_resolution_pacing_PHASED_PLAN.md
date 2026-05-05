# UI Polish Slice 3 - Render Resolution And Pacing

## Current Phase

Phase 4 complete - exploration-first defaults restored and the pacing policy is now explicitly locked for the 2048x1536 baseline

## Phase Checklist

- [x] Phase 1 - capture the resolution/pacing baseline on the dedicated slice-3 branch
- [x] Phase 2 - define the target default resolution and steady-state pacing policy
- [x] Phase 3 - implement the runtime, UI, and focused test updates
- [x] Phase 4 - hostile audit the resulting quality/performance behavior and checkpoint the slice

## Explicit User Asks

- [done] Raise the starting render resolution from the current low default.
- [done] Treat render-resolution and pacing cleanup as its own feature slice.
- [done] Merge slice 2 into the integration branch before starting slice 3.

## Presumption Loop

The likely problem is a mismatch between startup defaults, reset defaults, and the interaction-preview pacing policy. The slice should begin at the current owner files for runtime reset, viewport sizing, and render pacing, then move outward only if a focused regression shows the real decision point lives elsewhere.

Hostile review assumes the current low-resolution startup behavior is real, but the fix is not just "make the numbers bigger." First lock the mismatch between startup resolution, reset resolution, and pacing behavior with focused tests.

## Presumption Evidence

- Owner Proof: current UI review found a mismatch between `ui_app/src/runtime_reset.cpp`, `ui_app/src/viewer_render_pacing.cpp`, and the startup window/render behavior in `ui_app/src/main.cpp`.
- Fix Proof: `feature/ui-polish-color-authority` is already merged into `feature/ui-polish-integration`, and slice 3 is now code-complete on `feature/ui-polish-resolution-pacing` with the restored `2048x1536` default plus explicit 2048-baseline pacing regressions.
- RED Witness: baseline capture showed the live schema, safe-mode schema, `RenderSettings`, and `ResetRuntimeStateForCurrentFractal(...)` had all regressed to `1024x768` even though the repo previously checkpointed `2048x1536` as the aligned exploration-first default.
- Hostile Review Pass 1: restoring the pacing defaults to shared C++ constants initially introduced a real bug: `BuildViewerRenderPacingConfig(...)` started clamping explicit `preview_min_scale` values below `0.5` back up to the default floor. The existing `0.40` regression in `ui_app/tests/test_viewer_render_pacing.cpp` caught that immediately, and the clamp path was repaired before closure.
- Hostile Review Pass 2: after the clamp repair, the native helper rail, runtime publish, deployed `--validate-ui` check, deployed-schema inspection, and code-quality baseline all passed. No further render-resolution or pacing defects surfaced in the touched seam.

## Proof Ledger

- Manual RED: startup/default owner reads showed the active render-resolution default was uniformly `1024x768`, which matches the user's complaint that the viewer still starts too low.
- Checked-in regression RED: `ui_app/tests/test_runtime_reset.cpp` and `ui_app/tests/test_ui_schema.cpp` were updated to expect `2048x1536`, and the native helper rail failed on the fresh runtime render-state assertion before the owner surfaces were repaired.
- First GREEN: restored `2048x1536` at the live schema, safe-mode schema, runtime reset, and `RenderSettings` owner surfaces; `cmd /c ui_app\build_tests_vsdevcmd.cmd` and `cmd /c ui_app\build_vsdevcmd.cmd` are green again, and the deployed schema on `D:` now shows `2048` / `1536` as the published defaults.
- Post-green hostile finding: the first pacing-authority refactor accidentally over-clamped explicit preview floors below the default `0.5`. That defect was repaired locally, then the native helper rail, code-quality audit, runtime publish, and deployed `--validate-ui` checks were rerun before checkpointing.

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

Slice 3 is complete on `feature/ui-polish-resolution-pacing`. The next bounded sprint step is to merge this branch back into `feature/ui-polish-integration` and run the sprint-level integration audit.