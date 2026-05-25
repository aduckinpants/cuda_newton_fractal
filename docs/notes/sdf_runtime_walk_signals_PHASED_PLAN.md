# Runtime Walk SDF Signal Consumption

## Current Phase

Complete - runtime-walk headless SDF report sampling now uses the source-neutral SDF signal helper created by the prior slice. This plan remains closure evidence for the bounded sdf_runtime_walk_signals slice; live Color Pipeline SDF rows, viewport overlay work, new fractal types, renderer replacement, authored-pack UI, and physical mouse automation remain deferred.

## Phase Checklist

- [x] Phase 1 - create and lock this checked-in plan/contract
- [x] Phase 2 - add RED native proof that runtime-walk reports carry SDF signal fields, not only raw signed distance and inside flags
- [x] Phase 3 - route runtime-walk tick and reference SDF sampling through SampleSdfFieldSignals(...)
- [x] Phase 4 - prove runtime-walk headless, Lens SDF, flashlight, runtime publish, hostile audit, and diff/code-quality rails
- [x] Phase 5 - checkpoint, receipts, push, and clean-tree closeout

## Explicit User Asks

- [done] Continue into the next best measured seam after hostile review.
- [done] Keep SDF substrate work modular and avoid a renderer/viewport monolith.
- [done] Preserve existing Lens SDF and Color Pipeline behavior while widening report consumers.
- [done] Do not use OS mouse automation.

## Scope

In scope:

- Add runtime-walk headless report fields for SDF `inside_outside`, `boundary_band`, `normal_angle_radians`, and `curvature_estimate`.
- Route runtime-walk reference and tick SDF sampling through `SampleSdfFieldSignals(...)`.
- Keep existing report fields (`samples_signed_px`, `samples_inside`, saddle metrics, residual metrics) behavior-compatible.
- Update native tests and build helper surfaces to prove the helper is used.

Out of scope:

- Live Color Pipeline SDF source rows.
- Viewport overlay productization.
- New `FractalType` registration.
- Renderer replacement or passing SDF textures into CUDA render kernels.
- Authored SDF pack UI.
- Physical mouse automation.

## Proof Ledger

- Start authority: previous slice `sdf_field_signal_consumption` is closed on `master` at `b1fb392` and introduced `SdfFieldSignalSample`, `SampleSdfFieldSignals(...)`, and flashlight probe/report usage.
- Starting repo authority: `runtime_walk_headless.cpp` duplicated raw `SampleSignedDistanceSdfField(...)` calls for tick and reference report sampling before this slice; the repaired state routes those samples through `SampleSdfFieldSignals(...)`.
- Boundary authority: live Color Pipeline SDF rendering remains intentionally deferred because the current CUDA color path has no SDF field input surface.
- RED evidence: sdf_runtime_walk_signals_red failed before implementation because runtime_walk_report.json did not include the new SDF signal report fields.
- Native runtime-walk rail: sdf_runtime_walk_signals_test_runtime_walk_headless passed with test_runtime_walk_headless: passed=48 failed=0, including proof that tick samples and reference trace both carry samples_inside_outside, samples_boundary_band, samples_normal_angle_radians, and samples_curvature_estimate.
- Lens SDF preservation rail: sdf_runtime_walk_signals_test_lens_sdf passed.
- Flashlight preservation rail: sdf_runtime_walk_signals_test_flashlight_probe passed with test_flashlight_probe: 21 passed, 0 failed.
- Runtime publish rail: sdf_runtime_walk_signals_runtime_publish passed and published D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe; a follow-up failure-pattern scan found no compiler, fatal, link, unresolved, or failed markers.
- Color Pipeline preservation rail: sdf_runtime_walk_signals_color_pipeline_owner passed the focused Color Pipeline owner helper suite.
- Code-quality rail: sdf_runtime_walk_signals_code_quality passed baseline at score 96/100.
- Diff hygiene rail: sdf_runtime_walk_signals_diff_check passed.

## Hostile Audit

- Status: complete
- Required posture: assume the report still bypasses the SDF signal helper, the test only proves compilation, or the slice accidentally disturbs Lens SDF, flashlight, Color Pipeline, runtime publish, or renderer behavior until focused proof disproves it.

## Audit Passes

- [done] Pass 1 - re-read runtime-walk report output as if it still bypasses the SDF signal helper; found the original RED was too weak because it checked for scalar field names rather than proving both tick and reference report arrays.
- [done] Pass 2 - re-read tests/build scripts as if they only prove helper compilation; repaired the native test to require each new samples_* SDF signal key at least twice in the emitted report and added sdf_field_signal.cpp to the focused/full runtime-walk helper build.
- [done] Pass 3 - re-read the repaired state through runtime-walk, Lens SDF, flashlight, runtime publish plus log scan, Color Pipeline owner, code-quality, and diff-check rails; no additional real defect found in the touched seams and deferred live renderer/Color Pipeline SDF work stayed untouched.

## Audit Findings

- [done] Real defect found: the first RED could have accepted a report that exposed only one side of the runtime-walk payload because it looked for scalar-style field names. The repaired test now proves both tick sdf_samples and reference_trace carry the new source-neutral SDF signal arrays.
- [done] Real defect found: the first focused runtime-walk build script path would not link the new helper source for this test. The focused and full test_runtime_walk_headless compile commands now include ui_app/src/sdf_field_signal.cpp.
- [done] Clean re-read evidence: the repaired state passed runtime-walk, Lens SDF, flashlight, runtime publish plus explicit failure-pattern scan, Color Pipeline owner, code-quality baseline, and diff hygiene; no viewport renderer, live Color Pipeline SDF row, new fractal type, or physical mouse automation work was introduced.

## Action Hostile Review

- Action ID: sdf-runtime-walk-signals-start
- Suspected failure mode: runtime-walk reports kept a parallel raw signed-distance sampling path, so the new helper only covered flashlight and downstream consumers could diverge again.
- Correct owner/action: runtime-walk report SDF sampling now goes through SampleSdfFieldSignals(...) and exposes the same source-neutral signal fields in the report JSON.
- Proof surface: focused native `test_runtime_walk_headless`, focused Lens SDF and flashlight preservation rails, runtime publish, Color Pipeline preservation rail, contract validation, plan sync, hostile audit, code-quality baseline, and diff hygiene.
- Blocked action: Color Pipeline live renderer integration, viewport overlay, new fractal type, authored-pack UI, or physical mouse automation.
