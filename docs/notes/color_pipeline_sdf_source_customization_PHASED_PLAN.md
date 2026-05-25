# Color Pipeline SDF Source Customization

## Current Phase

Closed - SDF Source row customization is implemented, validated, audited, and ready for closeout receipts/rearward review.

## Phase Checklist

- [x] Phase 1 - create this checked-in plan/contract and open the slice
- [x] Phase 2 - repair stale SDF roadmap/status truth after viewport overlay shipped
- [x] Phase 3 - add RED native/runtime proof for hidden SDF row params and missing Source-section downsample alias
- [x] Phase 4 - expose existing SDF source params and add boundary width as a live Source-row param
- [x] Phase 5 - add shared SDF Field Downsample alias in the Color Pipeline Source section and default-collapse inactive Lens panel
- [x] Phase 6 - publish runtime and prove no-mouse Source-row customization, downsample alias authority, Capture Finding parity, and viewport overlay preservation
- [x] Phase 7 - hostile audit, receipts, rearward review, push, and clean-tree closeout

## Explicit User Asks

- [done] Make SDF Source row parameters adjustable instead of selecting a source with no visible knobs.
- [done] Add useful customization for SDF sources without broad Color Pipeline redesign.
- [done] Move field-resolution UX closer to the SDF Source section while preserving one shared `lens.downsample` authority.
- [done] Leave the legacy Lens path intact and reduce inactive Lens panel noise.
- [done] Keep authored-pack UI, SDF-native fractal lanes, and broader renderer/Color Pipeline redesign deferred.

## Scope

In scope:

- Repair roadmap/status text that still marks shipped viewport overlay work as deferred.
- Make all five SDF Source rows render their already-live Scale, Bias, and Blend Weight controls.
- Add `signal.boundary_width_px` only for `sdf_boundary_band`, defaulting to current `2.0` field-pixel behavior.
- Add a Color Pipeline Source-section `SDF Field Downsample` alias bound to the existing Lens downsample authority.
- Default-collapse the legacy Lens panel when Lens visualization and SDF overlay are both off.
- Add focused native and no-mouse runtime proof.

Out of scope:

- New SDF-native selectable fractal lanes.
- Authored SDF pack UI/live viewport integration.
- Dynamic renderer replacement or CUDA renderer redesign.
- A second persisted SDF field-resolution setting.
- Physical cursor or OS mouse automation.

## Proof Ledger

- Start authority: current branch `codex/color-pipeline-sdf-source-rows` at `d0af8b6` was clean and rearward review returned `status=ok`.
- RED proof: `test_color_pipeline_core`, `test_color_pipeline_window`, `test_color_pipeline_sdf_postprocess`, and `test_diagnostics_state_io` failed before implementation on the missing SDF source params, missing `signal.boundary_width_px`, missing Source-section downsample alias, and missing state IO support.
- Roadmap truth sync landed in `spec_intake/_STATUS.md`, `DEFERRED_THREADS.md`, and `docs/notes/sdf_field_pack_near_term_TODO.md`: normal viewport SDF overlay is now marked shipped; SDF Source customization is the active follow-up; authored-pack UI/live viewport integration and SDF-native lanes remain deferred.
- Native Color Pipeline proof passed: `ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core` reported `passed=152 failed=0`.
- Native Color Pipeline window proof passed: `ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_window` reported `passed=177 failed=0`.
- Native SDF postprocess proof passed: `ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_sdf_postprocess` reported `passed=20 failed=0`.
- Native diagnostics state IO proof passed: `ui_app/build_tests_vsdevcmd.cmd test_diagnostics_state_io` reported `all passed`.
- Schema/binding preservation proof passed: `ui_app/build_tests_vsdevcmd.cmd test_ui_schema` and `test_schema_binding` reported `all passed`.
- Fractal type/default preservation proof passed: `ui_app/build_tests_vsdevcmd.cmd test_fractal_types` reported `passed=134 failed=0`.
- Color/Lens preservation proof passed: `ui_app/build_tests_vsdevcmd.cmd test_escape_time_coloring` and `test_lens_sdf` reported `all passed`.
- Runtime publish passed twice after code changes; final publish command `ui_app/build_vsdevcmd.cmd` staged `D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe`.
- Published no-mouse runtime proof passed: `py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py tests/test_fractal_runtime_sdf_viewport_overlay.py` reported `5 passed` after the runtime visibility/audit repair.
- Runtime witness: the published viewer exposes Scale, Bias, and Blend Weight controls for all five SDF Source rows, exposes `signal.boundary_width_px` for `sdf_boundary_band`, applies Scale/Bias/Boundary Width edits through visible controls with frame-hash changes, and applies `color_pipeline.source.sdf_field.downsample.primary` to the shared Lens downsample authority without enabling Lens visualization.
- Hygiene proof passed: contract validation, phased-plan sync, hostile-audit validation, code-quality baseline, and `git diff --check` are green.

## Hostile Audit

- Status: done
- Required posture: assume SDF controls are still hidden, visible controls do not reach live runtime state, boundary width is serialized but ignored, downsample alias forks state, Lens panel collapse hides needed controls, Capture Finding parity regresses, or viewport overlay proof regresses until focused proof disproves each risk.

## Audit Passes

- [done] Pass 1 - re-read roadmap/status truth and repair stale shipped/deferred text.
- [done] Pass 2 - re-read SDF Source descriptor/render/apply/postprocess/state IO seams for hidden or non-live controls.
- [done] Pass 3 - re-read shared Lens downsample alias and Lens panel collapse behavior for duplicate authority or hidden control regressions.
- [done] Pass 4 - re-run preservation rails for SDF source rows, Capture Finding parity, and viewport overlay.

## Audit Findings

- [done] Real planning truth defect found and repaired: SDF roadmap/status docs still called normal viewport SDF overlay deferred after it shipped.
- [done] Real implementation defect found and repaired: SDF Source descriptors had params, but `IsLiveColorPipelineParamPath` hid SDF row params from the Color Pipeline window, leaving visible rows with no tunable controls.
- [done] Real implementation gap found and repaired: `sdf_boundary_band` used the fixed `SdfFieldSignalConfig` default and had no serialized/runtime-owned boundary width field.
- [done] Real runtime UX/harness regression found and repaired: default-collapsing the inactive Lens panel hid no-mouse overlay controls; automation/reporting now keeps the Lens panel open for Lens control proof while normal inactive UI remains collapsed.
- [done] Real proof gap found and repaired: the first runtime proof only checked boundary-band controls; the repaired runtime test loads all five SDF Source rows in one published viewer and checks Scale/Bias/Blend Weight visibility for each.
- [done] Clean re-read: no SDF-native selectable fractal lane, authored-pack UI/live viewport integration, renderer replacement, physical mouse automation, or second persisted downsample authority was added.

## Action Hostile Review

- Action ID: color-pipeline-sdf-source-customization-closeout
- Suspected failure mode: SDF Source descriptors could appear complete while the window hides params, the postprocess ignores boundary width, or the Source-section downsample alias forks Lens state.
- Correct owner/action: Color Pipeline Source descriptor/render/apply/state IO, Lens SDF postprocess, and the shared Lens downsample UI alias moved together under the existing Lens authority.
- Proof surface: focused Color Pipeline native rails, state IO rail, runtime publish, no-mouse published-runtime proof, Capture Finding parity, viewport overlay preservation, contract validation, phased-plan sync, hostile-audit validation, code-quality baseline, and diff hygiene.
- Blocked action: SDF-native fractal lane, authored-pack UI/live viewport integration, renderer replacement, physical mouse automation, or broad Color Pipeline redesign.
