# Advanced Color Explaino Runtime Pipeline Bugfix

## Current Phase

Phase 3 in progress - the Explaino programmable basin renderer repair and its focused proofs are green; update continuity surfaces and resolve slice closure cleanly

## Phase Checklist

- [x] Phase 1 - add focused native and published-runtime regressions proving Explaino supported programmable color tuples actually affect rendered output
- [x] Phase 2 - land the minimal basin-family renderer repair so supported non-basin advanced color tuples route through the programmable pipeline instead of legacy hardcoded colors
- [ ] Phase 3 - run the bounded validation rails, hostile-audit the repaired seam, checkpoint the detour, and hand back to the planned advanced-color library work

## Explicit User Asks

- [open] Stop treating the Explaino Color Pipeline as landed while the published runtime still shows dead sliders.
- [open] Verify the real renderer/runtime path with tests instead of relying on control-layer assumptions.
- [open] Fix the actual `/D:` runtime behavior where Explaino opens on a supported `smooth_escape / identity / heatmap` tuple but the advanced color sliders do nothing.
- [open] Keep the slice professional and reusable rather than adding another UI-only patch.

## Presumption Loop

The controlling defect is in the basin-family renderer dispatch, not the window state: Explaino reaches the advanced color window with a supported mirrored tuple, but `kernel_render(...)` still uses the basin-family hardcoded color formulas for `smooth_escape`, `phase`, `iteration_bands`, and `iteration_count` instead of the programmable `color_pipeline` authority. The cheapest disconfirming checks are renderer-scoped and executable: a native CUDA regression should show that changing Explaino programmable owner fields leaves rendered pixels unchanged on the current code, and a published-runtime `--load-state-json` capture test should show the same frame hash before the fix.

## Presumption Evidence

- `ui_app/src/color_pipeline_window.h` already imports, mirrors, and applies supported Explaino tuples into live `KernelParams`, including `smooth_escape / heatmap` and `smooth_escape / explaino_cmap`.
- `ui_app/src/fractal_family_rules.h` explicitly allows basin-coloring families to use mirrored programmable tuples and maps supported non-basin tuples back to `ColoringMode::smooth_escape`.
- `ui_app/src/fractal_renderer.cu` still branches on `SupportsBasinColoring(ft)` first, then handles `smooth_escape`, `phase`, `iteration_bands`, and `iteration_count` with legacy hardcoded colors inside the basin-family branch instead of calling the programmable color path.
- `ui_app/tests/test_escape_time_coloring.cpp` already proves the programmable signal/palette/shape owner fields for escape-time families, which explains why the control layer looked healthy while the published Explaino runtime stayed dead.
- `tests/test_fractal_runtime_explaino_escape_variants.py` already has headless capture/hash machinery that can prove whether a loaded Explaino state changes rendered output in the published runtime.

## Proof Ledger

- Read-only finding: the dead Explaino slider report reproduces the exact gap between the advanced color UI state and the basin-family renderer path.
- Read-only finding: the next useful proof surfaces are `ui_app/tests/test_fractal_sample_device.cu` for the native renderer seam and `tests/test_fractal_runtime_explaino_escape_variants.py` for the published runtime capture seam.
- Landed: `ui_app/tests/test_fractal_sample_device.cu` now proves that Explaino `smooth_escape` rendering changes when the programmable heatmap cycle-scale and `explaino_cmap` seed-phase owner fields change.
- Landed: `tests/test_fractal_runtime_explaino_escape_variants.py` now proves the published runtime changes the Explaino frame hash for the same loaded-state programmable owner changes.
- Landed: `ui_app/src/escape_time_coloring.h` and `ui_app/src/fractal_renderer.cu` now route basin-family non-basin advanced color tuples through a programmable basin-color path while leaving explicit `root_basin` and `joy_basins` handling separate.
- Validated: `py -3.14 -m pytest tests/test_fractal_runtime_explaino_escape_variants.py -q -k programmable_color_pipeline` passes against the rebuilt published runtime after `ui_app/build_vsdevcmd.cmd`.
- Validated: `D:\salt-fractal\cuda_newton_fractal_clone\build_tests\test_fractal_sample_device.exe` passes with `test_fractal_sample_device: passed=50 failed=0`.
- Audit: `D:\salt-fractal\cuda_newton_fractal_clone\build_tests\test_newton_basin_regression.exe` still passes, confirming the renderer change did not regress explicit `root_basin` coloring.
- Governance: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/advanced_color_explaino_runtime_pipeline_bugfix.contract.json --out-json artifacts/validation/advanced_color_explaino_runtime_pipeline_bugfix_contract.json`, `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`, and `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json` are green.

## Notes

- Expected owner files for this pass:
  - `docs/contracts/advanced_color_explaino_runtime_pipeline_bugfix.contract.json`
  - `docs/notes/advanced_color_explaino_runtime_pipeline_bugfix_PHASED_PLAN.md`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/src/fractal_renderer.cu`
  - `ui_app/tests/test_fractal_sample_device.cu`
  - `tests/test_fractal_runtime_explaino_escape_variants.py`
- Non-goals for this pass:
  - do not reopen the reset/default or legacy-control ownership detour
  - do not widen palette-library work beyond the renderer path needed to make supported Explaino tuples real
  - do not redesign the advanced color UI again unless a regression proves the renderer fix is insufficient
  - do not change `root_basin` or `joy_basins` semantics unless a new regression proves the programmable fix leaks into those explicit basin modes

## Resume Point

Close this renderer bugfix detour first. After the Explaino programmable runtime path is proven and checkpointed, resume the paused advanced-color foundation work from `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`.