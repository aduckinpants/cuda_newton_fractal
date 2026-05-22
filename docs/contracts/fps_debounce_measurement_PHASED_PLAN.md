# FPS Debounce Measurement Repair

## Current Phase

Phase 4 complete - FPS debounce activation is measurement-gated: moderate fast/f32 interaction stays full resolution, slow f64 interaction still enters preview, and runtime proof is green.

## Phase Checklist

- [x] Phase 0 - open a dedicated checked-in plan/contract for FPS debounce measurement and lock the active slice to it.
- [x] Phase 1 - measure current f32/low-cost interaction and prove whether preview downscale is activating without an FPS problem.
- [x] Phase 2 - measure current f64/high-cost interaction and prove whether preview downscale activates when frames are actually slow.
- [x] Phase 3 - repair the smallest pacing/reporting seam that explains the measured contradiction: harmful when not needed, ineffective when needed.
- [x] Phase 4 - validate native pacing/report rails, publish runtime, run no-mouse runtime proof, hostile-audit, checkpoint, receipts, push, and clean tree.

## Explicit User Asks

- [done] Return to the debounce/FPS issue immediately after the native-helper measurement slice.
- [done] No more guessing; measure before changing behavior.
- [done] Fix the regression where preview/debounce is too aggressive in common fast f32 situations.
- [done] Fix the failure where preview/debounce does nothing in slow f64/high-resolution situations.
- [done] Preserve capture finding quality, color pipeline behavior, fractal parameters, and renderer semantics unless the measured pacing defect strictly requires a narrow touch.
- [done] Use no physical mouse automation and avoid repeated viewer open/close loops.

## Proof Ledger

- Starting point: this slice starts after `9f1b73c`, which measured the full native helper as slow but passing and added JSON timing support for focused/full helper commands.
- RED native proof: `ui_app/build_tests_vsdevcmd.cmd test_viewer_render_pacing` failed before the fix with `Expected moderately slow full-resolution frames to stay full resolution until the FPS loss is material`.
- RED runtime proof: `tests/test_fractal_runtime_resolution_pacing.py::test_default_target_fast_f32_stays_full_but_slow_f64_enters_preview_no_mouse` failed before the fix because `render_pacing_preview_active` was `true` for the fast/f32 default-target path.
- Measured failing fast/f32 path before fix: 4096x4096 fast Multibrot, `max_iter=868`, default 30 FPS target, baseline `54.979ms`, center edit downscaled to preview scale `0.778647608613` at `3189x3189`.
- Measured slow/f64 path before fix: 2048x1536 standard Multibrot, baseline about `1630ms`, center edit entered preview at scale `0.5` and `1024x768`.
- Landed: `ViewerRenderPacingConfig::step_down_hysteresis` default changed from `1.10` to `2.0`, so preview starts after a material full-resolution miss instead of just above the nominal target frame time.
- Landed: native pacing test now locks the moderate-f32 case (`55ms` at 30 FPS target) to stay full resolution, while the existing `90ms` slow case still enters preview.
- Landed: runtime proof now uses one persistent viewer to prove default-target fast/f32 stays full resolution and slow/f64 still enters preview, with no physical mouse input.
- Measured green runtime proof after fix: fast/f32 baseline `59.089ms`, center edit `54.655ms`, preview inactive, full `4096x4096`; slow/f64 baseline `1613.371ms`, center edit preview active at scale `0.5`, rendered `1024x768` from target `2048x1536`.
- Validated: `py -3.14 tools/viewer_host_run_logged_command.py --label fps_debounce_native_pacing --log artifacts/logs/fps_debounce_native_pacing.log --out-json artifacts/validation/fps_debounce_native_pacing.json --timeout-seconds 180 -- ui_app/build_tests_vsdevcmd.cmd test_viewer_render_pacing` passed.
- Validated: `py -3.14 tools/viewer_host_run_logged_command.py --label fps_debounce_native_report --log artifacts/logs/fps_debounce_native_report.log --out-json artifacts/validation/fps_debounce_native_report.json --timeout-seconds 180 -- ui_app/build_tests_vsdevcmd.cmd test_viewer_ui_automation_report` passed.
- Validated: `py -3.14 tools/viewer_host_run_logged_command.py --label fps_debounce_publish_runtime --log artifacts/logs/fps_debounce_publish_runtime.log --out-json artifacts/validation/fps_debounce_publish_runtime.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_vsdevcmd.cmd` passed and published `D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe`.
- Validated: `py -3.14 -m pytest tests/test_fractal_runtime_resolution_pacing.py -q --junitxml artifacts/pytest/test_fractal_runtime_resolution_pacing.junit.xml` passed with `3 passed`.
- Validated: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/fps_debounce_measurement_code_quality.json` passed with score `97/100` and baseline passed.
- Validated: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/fps_debounce_measurement.contract.json --out-json artifacts/validation/fps_debounce_measurement_contract.json` passed.
- Validated: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Validated: `git diff --check` passed with only the existing `HANDOFF_LOG.md` line-ending warning.

## Hostile Audit

- Status: complete
- Did I actually measure fast f32 and slow f64 behavior before changing pacing?
- Did I distinguish unknown timing, fast timing, slow full-resolution timing, and active-preview timing?
- Did I avoid making f32/common interaction worse to help f64 slow paths?
- Did I prove slow f64/high-resolution interaction gets material relief rather than only flipping a flag?
- Did I preserve one full-quality settle render after debounce?
- Did I avoid physical mouse automation and repeated viewer open/close loops?
- Did I leave capture finding, color pipeline, and fractal math behavior untouched unless a measured pacing seam required it?

## Audit Passes

- [done] Pass 1: reviewed the first measurements as if the report fields were lying; the RED matched both native policy and published-runtime dimensions, so the report was discriminating.
- [done] Pass 2: reviewed the pacing diff as if it fixed only the test harness; the only behavior change is the activation threshold, and the published runtime proof shows actual rendered dimensions.
- [done] Pass 3: clean re-read after the repair found no additional real defect; capture finding, color pipeline, fractal math, and OS mouse automation seams were untouched.

## Audit Findings

- [done] Finding repaired: preview activation was too close to the nominal target frame time, causing a measured 4096x4096 f32 Multibrot interaction around `55ms` to downscale even though the current workload remained usable at full resolution.
- [done] Finding guarded: slow f64/high-resolution interaction still gets material relief after the threshold change; the runtime proof shows `2048x1536` downscales to `1024x768` at preview scale `0.5`.
- [done] Finding guarded: the runtime proof uses one persistent viewer process and in-process set-value/load-state commands, not physical mouse automation or repeated viewer open/close loops.
- [done] Clean re-audit: no additional real defect found in capture finding, color pipeline, fractal math, runtime publish, or pacing report fields after the repair.

## Measurement Requirements

- Fast path witness: low-cost f32 interaction should not visibly downscale without measured slow-frame evidence. Satisfied by `test_default_target_fast_f32_stays_full_but_slow_f64_enters_preview_no_mouse`.
- Slow path witness: high-resolution f64 interaction should downscale quickly when measured full-resolution frames exceed budget. Satisfied by the same runtime test and the existing camera-center slow proof.
- Recovery witness: active preview should not pump resolution upward until scaled timing is safely under budget, then should recover gradually. Preserved by `test_viewer_render_pacing`.
- Settle witness: after interaction debounce, the viewer should render one full-quality frame. Preserved by `test_viewer_render_pacing` and the existing runtime settle assertion.
- Harness witness: all runtime proof must use in-process set-value/camera automation, not OS mouse input. Satisfied by `tests/test_fractal_runtime_resolution_pacing.py`.
