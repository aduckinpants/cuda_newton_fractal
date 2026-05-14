# Advanced Color Root Palette Shape Interactivity

## Current Phase

Complete - checkpoint commit `cc3d785` and machine proof receipts closed the root-palette Shape interactivity detour. This plan remains historical closure evidence only and must not be read as live pre-closeout restart authority.

## Phase Checklist

- [x] Phase 1 - add focused native and published-runtime regressions proving root-palette advanced tuples currently ignore Shape in the renderer
- [x] Phase 2 - land the minimal runtime repair so root-classic and joy basin tuples honor Shape without regressing the legacy identity look
- [x] Phase 3 - run bounded validation rails, hostile-audit the seam, checkpoint the slice, and resume the paused advanced-color foundation plan

## Explicit User Asks

- [done] Fix the actual product behavior where Explaino and Explaino Joy open into an advanced root-palette tuple whose Shape controls do nothing.
- [done] Treat runtime interactivity as the real requirement instead of declaring catalog rows done because the editor can select them.
- [done] Keep the fix bounded and test-backed instead of papering over it with more editor-only explanation text.

## Presumption Loop

The controlling defect is in the basin renderer path, not in the advanced color window state: when the live tuple is `root_index + root_classic_palette` or `root_index + joy_root_palette`, `kernel_render(...)` still colors through the explicit `root_basin` and `joy_basins` branches using the raw nearest-root index and never applies `color_shape`. The cheapest disconfirming check is renderer-scoped and executable: a native CUDA regression should show identical rendered frames before and after a non-identity Shape edit on those tuples in the current code.

## Presumption Evidence

- `ui_app/src/color_pipeline_window.h` already imports and applies the root-classic and joy-root advanced tuples as supported live selections.
- `ui_app/src/fractal_family_rules.h` maps those tuples directly back to `ColoringMode::root_basin` and `ColoringMode::joy_basins`.
- `ui_app/src/fractal_renderer.cu` still handles those explicit basin modes with hardcoded root-index branches and never consults `color_shape`.
- `ui_app/src/escape_time_coloring.h` already owns the shared Shape math, so the minimal fix should reuse that seam instead of inventing a second Shape implementation.
- `ui_app/tests/test_fractal_sample_device.cu` is the cheapest focused proof surface for the full renderer path because it already hashes frame changes through `RenderFractalCUDA(...)`.

## Proof Ledger

- Read-only finding: the advanced editor is not lying about the live tuple; the renderer is bypassing Shape for explicit root-basin and joy-basin branches.
- Landed: `ui_app/tests/test_fractal_sample_device.cu` now proves the real renderer path changes rendered frames when Explaino `root_classic` and Explaino Joy `joy` tuples switch Shape from `identity` to `offset_scale`.
- Landed: `tests/test_fractal_runtime_explaino_escape_variants.py` now proves the published runtime changes the frame hash for the same root-classic and joy Shape edit.
- Landed: `ui_app/src/escape_time_coloring.h` now resolves real basin root samples and exposes a shared helper that converts the nearest-root index into a Shape-adjusted palette index.
- Landed: `ui_app/src/fractal_renderer.cu` now feeds the root-basin and joy-basin palette branches through that Shape-adjusted root index while preserving the existing identity-path formulas.
- Validated: `artifacts/root_palette_shape_native_green.log` is green for `ui_app/build_tests_vsdevcmd.cmd` after the new renderer regression landed.
- Validated: `artifacts/root_palette_shape_runtime_publish.log` republished the runtime cleanly, and `artifacts/root_palette_shape_runtime_pytest.log` reports the new published-runtime root-palette Shape regression passing.
- Audit: `artifacts/root_palette_runtime_audit_pytest.log` reports both the legacy `root_classic` versus `joy` distinction proof and the new Shape proof passing together against the published runtime.
- Validated: `artifacts/code_quality_report.json` stayed at the `97/100` baseline for the slice.
- Validated: `artifacts/root_palette_runtime_probe_lane.log` reports the canonical published-runtime probe/session lane passing with `68 passed` and no skips.

## Notes

- Expected owner files for this pass:
  - `docs/contracts/advanced_color_root_palette_shape_interactivity.contract.json`
  - `docs/notes/advanced_color_root_palette_shape_interactivity_PHASED_PLAN.md`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/src/fractal_renderer.cu`
  - `ui_app/tests/test_fractal_sample_device.cu`
  - `tests/test_fractal_runtime_explaino_escape_variants.py`
- Non-goals for this pass:
  - do not redesign the advanced color window again
  - do not add fake per-palette parameter sliders to `root_classic_palette` or `joy_root_palette`
  - do not widen Phase 5 grading work in the same slice

## Resume Point

Closed. Do not resume from this slice's old checkpoint chores. Re-enter later advanced-color work from `docs/notes/advanced_color_feature_restart_inventory.md`, `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`, and `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md` instead.
