# SDF Postprocess Hotpath Specialization

## Current Phase

Phase 5 - validation, hostile audit, receipts, rearward review, push, and clean-tree closeout.

## Phase Checklist

- [x] Phase 0 - fast-forward `master` to measured head `f403ce8`, push it, and branch `codex/sdf-postprocess-hotpath-specialization`.
- [x] Phase 1 - open this runtime/viewer-first plan and contract, lock the active slice, and capture the before-edit no-mouse SDF performance baseline.
- [x] Phase 2 - add focused RED native/reporting tests for postprocess plan stats, worker-count reporting, and serial-vs-parallel exact-pixel parity.
- [x] Phase 3 - implement private SDF postprocess plan construction and bounded serial/parallel tile execution without visible UI changes.
- [x] Phase 4 - publish runtime and rerun focused SDF rows, pacing, capture/replay, and performance witness rails.
- [ ] Phase 5 - hostile audit, validation receipts, contract proof receipt, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [done] Optimize the measured SDF Color Pipeline CPU postprocess hot path first.
- [done] Preserve exact full-quality pixels; do not use approximation or LUT quantization.
- [done] Keep worker threading bounded and local to the SDF postprocess loop.
- [done] Do not change visible Color Pipeline UI, SDF source semantics, gates, phase behavior, capture/replay, shared Lens downsample authority, or preview policy.
- [done] Defer GPU Color Pipeline postprocess, per-row downsample, authored SDF UI, and SDF-native lanes.
- [done] Measure before and after through no-mouse published-runtime proof, and do not claim the FPS issue is solved unless the measured witness supports it.

## Scope

In scope:
- `ApplyLensSdfColorPipelinePostprocess(...)` internals and directly related options/stats/reporting.
- Focused native tests for exact-pixel preservation and worker reporting.
- Published no-mouse SDF runtime performance witness comparison.
- Plan, contract, handoff, receipts, and validation artifacts.

Out of scope:
- Renderer/mask kernel changes.
- Lens SDF field generation tuning.
- GPU Color Pipeline postprocess.
- Per-row or per-source SDF downsample authority.
- Visible UI redesign, SDF source semantics changes, authored SDF pack UI, SDF-native fractal lanes, or Salticid runtime dependency changes.

## Implementation Direction

The first implementation should build a private per-call postprocess execution plan that resolves the SDF-only Source stack, signal kinds, boundary/gate config, scale/bias, blend weight, direct-vs-neighborhood mode, output pixel step, and expected sample grid before entering the hot loop.

The executor should preserve the existing serial path and add an automatic bounded worker path for large workloads. Workers may write only disjoint output spans and must combine stats after completion. The default worker cap should be conservative, such as `min(max(1, hardware_concurrency / 2), 6)`, with serial fallback for small sample counts and a test option to force serial or bounded parallel mode.

The stats/reporting surface may add only the minimum field needed to prove behavior, currently `worker_count`, reported as `lens_sdf_postprocess_worker_count` in the viewer automation report.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on `codex/sdf-performance-measurement-witness` at `f403ce8`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported a clean tree.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `f403ce8`.
- Merge prep: `master` was fast-forwarded from `66dc145` to `f403ce8`, pushed to `origin/master`, and this branch was created from that pushed head.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "SDF postprocess hotpath specialization" --profile runtime --plan docs/notes/sdf_postprocess_hotpath_specialization_PHASED_PLAN.md --contract docs/contracts/sdf_postprocess_hotpath_specialization.contract.json` appended `ck:41621ef1` and locked the active contract.
- Baseline witness: `sdf_postprocess_hotpath_baseline` passed and wrote `artifacts/sdf_postprocess_hotpath_specialization/baseline_sdf_performance_witness.json`; baseline full-quality postprocess examples were signed distance `31.378ms`, normal angle `45.861ms`, curvature `42.533ms`, normal-angle plus curvature `50.378ms`, and settled large frame `358.482ms`.
- RED native: `sdf_postprocess_hotpath_red_native` failed because `SdfColorPipelinePostprocessOptions::max_worker_threads` and `SdfColorPipelinePostprocessStats::worker_count` did not exist.
- RED report native: `sdf_postprocess_hotpath_red_report_native` failed because `ViewerUiAutomationLensSdfProbe::postprocess_worker_count` did not exist.
- Native postprocess GREEN: `sdf_postprocess_hotpath_native` passed `test_color_pipeline_sdf_postprocess` with `passed=71 failed=0` after the final hostile-review fix.
- Native report GREEN: `sdf_postprocess_hotpath_report_native` passed `test_viewer_ui_automation_report`.
- Witness unit GREEN: `sdf_postprocess_hotpath_witness_unit` passed `tests/test_sdf_performance_witness_tool.py` with 3 tests.
- Runtime publish GREEN: `sdf_postprocess_hotpath_publish` staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published runtime SDF rows GREEN: `sdf_postprocess_hotpath_runtime_sdf_rows` passed 6 no-mouse tests.
- Published runtime SDF pacing GREEN: `sdf_postprocess_hotpath_runtime_pacing` passed 1 no-mouse test.
- Published runtime capture/replay GREEN: `sdf_postprocess_hotpath_runtime_capture_replay` passed 1 no-mouse test.
- After witness GREEN: `sdf_postprocess_hotpath_after_witness` passed and wrote `artifacts/sdf_postprocess_hotpath_specialization/after_sdf_performance_witness.json`; full-quality postprocess examples became signed distance `9.614ms`, normal angle `12.606ms`, curvature `13.283ms`, normal-angle plus curvature `12.716ms`, downsample4 `1.211ms`, and settled large frame `103.596ms`.
- Measured result: full-quality SDF postprocess improved about 68-75% for representative rows and about 71% for the settled larger frame. The preview sample is not claimed as a speed win because the unchanged pacing policy selected a higher-quality step 2 preview instead of the prior step 4 sample.
- Code-quality baseline: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/sdf_postprocess_hotpath_code_quality.json` passed with score 94/100 and baseline check passed.

## Hostile Audit

- Status: complete
- Did I actually optimize the measured CPU SDF postprocess seam instead of tuning the wrong stage? Yes; renderer/mask and Lens SDF field generation code were not changed, and the measured improvement is in `lens_sdf_postprocess_ms`.
- Did I preserve exact full-quality pixels for scalar, phase, curvature, gated, downsampled-field, and preview paths? Yes; the focused native postprocess rail includes serial-vs-parallel exact pixel parity and existing SDF postprocess coverage, and runtime SDF rows/capture replay stayed green.
- Did I keep worker threading bounded, disjoint, deterministic in output, and local to postprocess? Yes after the hostile-review fix; workers split disjoint field/render row ranges, use byte worker status storage, cap at 6, and combine stats after join.
- Did I preserve SDF rows, capture/replay, preview policy, pacing telemetry, and UI-Salt/Color Pipeline behavior? Yes; focused runtime SDF rows, SDF pacing, and capture/replay rails passed.
- Did I avoid GPU postprocess, per-row downsample, authored SDF UI, SDF-native lanes, and UI redesign? Yes; this slice added no visible controls and did not change those deferred surfaces.
- Did I prove runtime behavior through no-mouse published viewer proof instead of helper-only evidence? Yes; publish plus SDF rows, pacing, capture/replay, and performance witness all used the published runtime.
- Did I report performance truthfully, including `unproven` if the witness does not show material improvement? Yes; full-quality improvement is proven, while the preview sample is explicitly not claimed as a raw speed win because its quality step changed from 4 to 2.

## Audit Passes

- [done] Pass 1 - pre-edit call-stack audit and baseline witness identified CPU SDF postprocess as the measured hot path.
- [done] Pass 2 - RED/native proof audit added missing worker-count and serial/parallel exactness coverage before implementation.
- [done] Pass 3 - implementation diff audit found and repaired the `std::vector<bool>` worker-status race risk.
- [done] Pass 4 - runtime/performance witness audit proved full-quality improvements and preserved SDF rows, pacing, and capture/replay.
- [done] Pass 5 - clean re-read confirmed the repaired state and found no renderer, field-generation, GPU postprocess, per-row downsample, authored SDF UI, or visible UI drift.

## Audit Findings

- [done] Initial suspected failure mode: a first optimization could change pixels, hide cost through preview/downsample, or claim responsiveness from non-published proof. Repaired with serial/parallel exact-pixel tests, full-quality postprocess timing comparison, and published no-mouse runtime proof.
- [done] Finding 1: the first threaded implementation used `std::vector<bool>` for worker success flags, which is bit-packed and can race across worker writes. Replaced it with byte storage and reran the exact-pixel native rail.
- [done] Finding 2: the preview comparison is not apples-to-apples because the cheaper full-quality path made the unchanged pacing policy choose preview step 2 instead of step 4. Documented this as not a preview speed claim while preserving the existing preview policy.
- [done] Clean re-read confirmed the repaired state after Finding 1 and Finding 2: focused native exact-pixel proof, report native proof, witness unit proof, runtime publish, SDF rows runtime proof, SDF pacing proof, capture/replay proof, after-witness performance proof, contract validation, plan sync, and code-quality baseline all passed on the repaired state.

## Notes

- Performance acceptance target: derivative full-quality SDF postprocess scenarios should improve materially, with a target of at least 25% median improvement on the same machine. Scalar, downsample4, and preview scenarios must not regress beyond timing noise, target no more than 10%.
- If timing is noisy or below target, close only as pixel-safe optimization with client improvement unproven.
