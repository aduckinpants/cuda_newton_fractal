# SDF Viewport Overlay Productization

## Current Phase

Closed - normal viewport SDF overlay controls and no-mouse proof are implemented without regressing Color Pipeline SDF rows.

## Phase Checklist

- [x] Phase 1 - create this checked-in plan/contract and open the slice
- [x] Phase 2 - add schema/binding/state support for explicit SDF viewport overlay controls
- [x] Phase 3 - render the Lens SDF overlay in the normal Fractal viewport without changing base fractal pixels or Color Pipeline source-row semantics
- [x] Phase 4 - add native schema/binding/default/reset/report tests for the overlay contract
- [x] Phase 5 - publish runtime and run a no-mouse viewer proof for overlay visibility, Lens Downsample authority, and Color Pipeline preservation
- [x] Phase 6 - hostile audit, receipts, rearward review, push, and clean-tree closeout

## Explicit User Asks

- [done] Continue the interrupted SDF work after the Capture Finding and Lens Downsample bug reports.
- [done] Keep the bug-fix branch behavior intact: SDF Capture Finding parity and Lens Downsample visibility/control authority must not regress.
- [done] Make the next SDF product step tangible in the normal viewport, not only aux windows or reports.
- [done] Do not add SDF-native fractal lanes, authored-pack live viewport integration, renderer replacement, physical mouse automation, or broad Color Pipeline redesign in this slice.

## Scope

In scope:

- Add small explicit Lens/SDF overlay controls to the existing schema and binding surface.
- Reuse the existing Lens SDF field and texture generation path.
- Draw the overlay in the normal Fractal viewport, separate from the aux Lens SDF window.
- Report overlay state through the existing no-mouse automation JSON.
- Add focused native and runtime proof.

Out of scope:

- SDF-native selectable fractal lanes.
- Authored SDF pack UI/live viewport integration.
- Dynamic renderer replacement or new CUDA renderer path.
- Color Pipeline source-row redesign.
- Physical cursor or OS mouse automation.

## Proof Ledger

- Start authority: current branch `codex/color-pipeline-sdf-source-rows` at `c763028` after the SDF roadmap truth sync repair.
- Rearward gate authority: `py -3.14 tools/viewer_host_rearward_review.py --out-json artifacts/validation/sdf_roadmap_truth_sync_rearward_final.json` returned `status=ok` for `c763028043c918c0386c1927d6039f292e2e5aac`.
- Existing product proof to preserve: `tests/test_fractal_runtime_color_pipeline_sdf_rows.py` covers live SDF source rows, Capture Finding parity, and Lens Downsample visibility/control authority.
- Native schema/binding/default/report proof passed: `ui_app/build_tests_vsdevcmd.cmd test_ui_schema`, `test_schema_binding`, `test_fractal_types`, and `test_viewer_ui_automation_report`.
- Native Color Pipeline/Lens preservation proof passed: `ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core`, `test_color_pipeline_window`, `test_color_pipeline_sdf_postprocess`, and `test_lens_sdf`.
- Runtime publish passed: `ui_app/build_vsdevcmd.cmd` staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Runtime no-mouse proof passed: `py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_sdf_viewport_overlay.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py` reported `4 passed`.
- Runtime witness: `tests/test_fractal_runtime_sdf_viewport_overlay.py` proves overlay mode is visible, can be set to `field_debug`, keeps `lens.enabled=false`, computes a valid Lens SDF field, keeps the base rendered frame hash unchanged, and keeps Lens Downsample authoritative after overlay activation.

## Hostile Audit

- Status: done
- Required posture: assume the overlay silently changes base rendered pixels, hides Lens Downsample again, breaks SDF Capture Finding parity, or overclaims SDF-native product readiness until focused tests prove otherwise.

## Audit Passes

- [done] Pass 1 - re-read the overlay diff and touched schema/binding/report/render seams for a real defect.
- [done] Pass 2 - verify no Color Pipeline source-row behavior or Capture Finding parity regressed.
- [done] Pass 3 - verify no SDF-native lane, authored-pack live integration, renderer replacement, or physical mouse automation entered the slice.
- [done] Pass 4 - verify the validation contract records one focused build-test target per command because `build_tests_vsdevcmd.cmd` accepts a single target.

## Audit Findings

- [done] Real implementation risk found and repaired before build: `RenderFractalViewport` used the overlay helper before its definition; forward declarations were added before the focused native compile.
- [done] Real runtime proof defect found and repaired: the initial overlay test expected Lens Downsample while overlay mode was still `off`; the corrected proof enables `field_debug` first, then proves Downsample visibility and authority.
- [done] Real contract defect found and repaired: the validation command list incorrectly grouped multiple focused native targets into one `build_tests_vsdevcmd.cmd` command even though the helper accepts one target.
- [done] Clean re-read: no SDF-native selectable fractal lane, authored-pack live viewport integration, renderer replacement, Color Pipeline source-row redesign, or physical mouse automation was added.
- [done] Preservation proof: the combined runtime lane kept the pre-existing live SDF source-row, Capture Finding parity, and Lens Downsample SDF-row tests green after overlay implementation.