# Diagnostics Capture Render Stats Repair

## Current Phase

Complete - diagnostics capture render stats now use a 64-bit iteration sum, finding captures export meaningful timing, focused native/runtime proof is green, and closeout is recorded through the checkpoint handoff plus machine receipts.

## Explicit User Asks

- [x] Research the long-standing capture `state.json` stats issue around `last_iters_avg` and `last_render_ms`.
- [x] If the cause is simple, fix it without making the working Magnet or Explaino-all surfaces worse.
- [x] Preserve Color Pipeline behavior and existing in-process automation while touching shared capture/report surfaces.
- [x] Do not regress FPS-sensitive live rendering paths.

## Phase Checklist

- [x] Phase 0 - bootstrap, inspect current repo/capture state, and identify the concrete producer seams before product mutation.
- [x] Phase 1 - open and lock this bounded diagnostics stats contract, then clean up the temporary bridge widening from the closed Magnet contract.
- [x] Phase 2 - repair the stats producers/exporters: prevent 4k iteration-sum overflow and benchmark finding capture renders so exported elapsed time is meaningful.
- [x] Phase 3 - add focused regression tests for 64-bit iteration average math, stats export fields, and finding-capture benchmark routing.
- [x] Phase 4 - run focused native rails plus contract/plan/audit validation, checkpoint, receipts, push, clean tree, and stale-plan reread.

## Proof Ledger

- Bootstrap on 2026-05-19 found branch `master`, `HEAD=3055c88`, clean tree, and active contract `fractal_toolkit_magnet_wave`, which is closed and must not own product mutation for this repair.
- Target capture `D:\salt-fractal\cuda_newton_fractal_clone\findings\manual_capture\2026-05-19\074327_335__magnet\state.json` exports `stats.last_render_ms = 0`, `stats.last_iters_avg = -80`, `resolved_backend = float32`, and `resolved_strategy = direct`.
- Producer seam found: `ui_app/src/fractal_renderer.cu` stores the device iteration sum in a 32-bit `int`, adds every pixel with `atomicAdd(outItersSum, it)`, and divides that signed sum by pixel count for `last_iters_avg`.
- Root cause for negative averages: 4096x4096 captures overflow signed 32-bit once the true average exceeds about 128 iterations; `-80` is consistent with a true average around 176 after 32-bit wrap.
- Separate zero-ms cause: `RenderFractalCUDA` intentionally zeroes `last_render_ms` whenever `render.benchmark` is false, and `BuildFindingArchiveCaptureRender(...)` preserves the interactive render's default `benchmark=false` for manual finding captures.
- Current evidence does not show Magnet returning signed iteration codes; `FractalSampleResult::iterations` is documented as an iteration count and current Magnet renderer tests already expect non-negative stats on small renders.
- Bridge cleanup complete: the temporary additions to `docs/contracts/fractal_toolkit_magnet_wave.contract.json` were removed after this contract was locked, leaving the closed Magnet contract restored.
- Implementation landed: `ui_app/src/fractal_renderer.cu` now accumulates iteration sums in `unsigned long long`, clamps only impossible negative sample counts before accumulation, stores `last_iters_sum` plus `last_pixel_count`, and computes `last_iters_avg` through the 64-bit helper in `ui_app/src/fractal_types.h`.
- Capture export landed: `ui_app/src/diagnostics_capture.cpp` writes `last_iters_sum` and `last_pixel_count` beside `last_iters_avg`, so future `state.json` files can distinguish average, raw sum, and pixel count instead of hiding overflow behind one label.
- Finding capture timing landed: `BuildFindingArchiveCaptureRender(...)` now enables `benchmark=true` only on the 4k finding archive capture render; normal live renders still default to `benchmark=false` and `test_fractal_renderer` continues to assert non-benchmark `last_render_ms == 0`.
- Focused native proof is green: `ui_app\build_tests_vsdevcmd.cmd` completed with `All helper tests passed` in `artifacts/diagnostics_capture_render_stats_build_tests.log`; direct focused reruns are green in `artifacts/diagnostics_capture_render_stats_test_fractal_types.log`, `artifacts/diagnostics_capture_render_stats_test_diagnostics_capture.log`, `artifacts/diagnostics_capture_render_stats_test_finding_archive_actions.log`, and `artifacts/diagnostics_capture_render_stats_test_fractal_renderer.log`.
- Color Pipeline guard stayed green through the full helper-test log: `test_color_pipeline_core: passed=120 failed=0` and `test_color_pipeline_window: passed=155 failed=0`.
- Runtime publish is green: `ui_app\build_vsdevcmd.cmd` completed in `artifacts/diagnostics_capture_render_stats_runtime_build.log` and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published-runtime proof is green: `py -3.14 -m pytest tests/test_fractal_runtime_magnet.py -q --junitxml artifacts/pytest/diagnostics_capture_render_stats_runtime_magnet.junit.xml` passed.
- Direct capture proof is green: `D:\salt-fractal\cuda_newton_fractal_clone\findings\manual_capture\2026-05-19\090120_929__magnet\state.json` records a 4096x4096 Magnet finding capture with `last_render_ms = 78.26534271240234`, `last_iters_avg = 517`, `last_iters_sum = 8676524606`, `last_pixel_count = 16777216`, `resolved_backend = float32`, and `resolved_strategy = direct`; the raw sum exceeds signed 32-bit and no longer wraps negative.
- Contract, plan-sync, hostile-audit, and code-quality validators are green in `artifacts/validation/diagnostics_capture_render_stats_repair_contract.json`, `artifacts/validation/viewer_host_assert_phased_plan_sync.json`, `artifacts/validation/diagnostics_capture_render_stats_repair_hostile_audit.json`, and `artifacts/validation/diagnostics_capture_render_stats_code_quality.json`.

## Hostile Audit

- Status: complete
- Required posture: assume the stats repair is lying unless it proves the negative average cannot recur on 4k captures, the elapsed-time field is only made meaningful for capture renders, and no live FPS-sensitive path is benchmarked by default.
- Hostile review questions:
  Did I actually remove the 32-bit accumulator overflow rather than relabel the field?
  Did I keep `FractalSampleResult::iterations` and `SampleFractalPoints(...)` semantics unchanged?
  Did I benchmark finding captures without turning every live render into a benchmark render?
  Did I preserve Magnet, Explaino-all, and Color Pipeline behavior while touching shared stats/export surfaces?
  Did I close with focused native proof, receipts, push, clean tree, and no stale plan text?

## Audit Passes

- [x] Pass 1 - re-read the stats producer/exporter diff and focused tests after implementation; found a real patch-generation omission before tests and repaired the half-applied renderer/test changes.
- [x] Pass 2 - verified the benchmark routing is limited to `BuildFindingArchiveCaptureRender(...)`; normal live render defaults remain `benchmark=false` and the renderer test keeps non-benchmark timing at zero.
- [x] Pass 3 - re-read the repaired state, reran focused native/runtime proof, and no additional real defect was found after the 4k capture witness.

## Audit Findings

- [x] Real finding: the first generated product patch only carried the last replacement per file in several files, leaving a half-applied renderer/test diff; caught during hostile diff review before validation, repaired with a follow-up wrapper patch, and then reran focused proof.
- [x] Clean re-read: after the follow-up repair, focused native tests, Color Pipeline guard tests in the helper log, runtime publish, published-runtime Magnet proof, and direct 4k finding capture proof all proved cleanly.

## Action Hostile Review

- Action ID: diagnostics-capture-render-stats-overflow-repair-1
- Suspected Failure Mode: The repair could merely rename `last_iters_avg`, keep the 32-bit overflow in the renderer, or make live rendering slower by enabling benchmark timing globally.
- Correct Owner/Action: Change only the render stats accumulator/export and finding-capture render settings needed to make capture `state.json` stats truthful while preserving current fractal math and live render defaults.
- Proof Surface: `test_fractal_types`, `test_fractal_renderer`, `test_diagnostics_capture`, `test_finding_archive_actions`, contract validation, phased-plan sync, and hostile-audit validation.
- Blocked Action: Magnet redesign, Explaino-all changes, Color Pipeline changes, OS mouse tests, broad renderer refactor, or changing `SampleFractalPoints(...)` semantics.

## Notes

- Existing historical captures will remain historical; this slice repairs newly generated capture stats.
