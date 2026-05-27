# SDF GPU Postprocess Neighborhood

## Current Phase

Phase 5 - validation and closure for the CUDA field-signal SDF postprocess backend.

## Phase Checklist

- [x] Phase 0 - bootstrap clean `codex/sdf-adaptive-preview-pacing` head and confirm rearward review is ok after the direct-scalar GPU slice.
- [x] Phase 1 - create this checked-in plan/contract and lock the active slice.
- [x] Phase 2 - add focused RED native CUDA proof for missing `sdf_normal_angle` and `sdf_curvature` GPU postprocess support.
- [x] Phase 3 - implement CUDA field-signal postprocess for SDF-only Source stacks with row sample step `1`.
- [x] Phase 4 - wire runtime/report/performance surfaces without visible UI behavior changes.
- [x] Phase 5 - publish runtime, run focused native/runtime/performance rails, hostile audit, receipts, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [active] Work the harder SDF performance seam after direct scalar GPU testing looked good.
- [active] Keep the optimization measured and bounded, not another debounce guess.
- [active] Preserve current SDF phase/curvature semantics, gates, source blending, Color Pipeline UI, capture/replay, and per-row quality behavior.
- [active] Do not use physical mouse automation.

## Scope

In scope:

- A second CUDA SDF Color Pipeline postprocess backend for SDF field-signal rows that need local neighborhoods.
- Supported signals: `sdf_normal_angle`, `sdf_curvature`, plus mixed SDF-only stacks containing the existing direct scalar SDF signals.
- Source row scale, bias, blend weight, boundary-band gate, boundary width, downsampled-field expansion, and output pixel step parity.
- Runtime/reporting fields that state when the field-signal GPU backend actually ran.
- CPU fallback for row sample step greater than `1`, mixed SDF/non-SDF stacks, CUDA failure, invalid fields, and unsupported options.

Out of scope:

- Per-source or multi-field SDF downsample authority.
- Approximate/LUT/quality-changing normal or curvature math.
- GPU-resident renderer/color-pipeline rewrite.
- Visible Color Pipeline UI redesign, authored SDF pack UI, SDF-native fractal lanes, or Salticid runtime dependency.
- Capture-quality changes, preview-policy changes, or physical mouse automation.

## Implementation Direction

The CPU implementation remains the reference. The CUDA field-signal path may run only for SDF-only source stacks whose rows use `sdf_sample_step == 1`. The device path must compute the same center, clamped left/right/up/down neighborhood, angle normalization, curvature estimate, source blending, shape, palette, and grading as the CPU path.

Auto backend order should remain conservative:

1. `cuda_direct_scalar` for direct scalar rows.
2. `cuda_field_signal` for neighborhood rows and mixed SDF-only field-signal stacks.
3. CPU fallback for unsupported cases.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on clean `codex/sdf-adaptive-preview-pacing` at `47434a7`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported a clean tree.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `47434a7`.
- Prior direct-scalar GPU slice: `47434a7` shipped `cuda_direct_scalar` for signed-distance, inside/outside, and boundary-band SDF rows; its witness still identified phase/curvature CPU fallback as the remaining bottleneck.
- Contract bootstrap: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_gpu_postprocess_neighborhood.contract.json --out-json artifacts/validation/sdf_gpu_neighborhood_contract_bootstrap.json` passed.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "SDF GPU postprocess neighborhood" --profile runtime --plan docs/notes/sdf_gpu_postprocess_neighborhood_PHASED_PLAN.md --contract docs/contracts/sdf_gpu_postprocess_neighborhood.contract.json` opened token `ck:c3d166db`.
- RED: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_gpu_neighborhood_red_cuda --log artifacts/logs/sdf_gpu_neighborhood_red_cuda.log --out-json artifacts/validation/sdf_gpu_neighborhood_red_cuda.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_sdf_postprocess_cuda` failed on the missing `cuda_field_signal` enum/backend and missing field-signal preflight.
- Native CUDA: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_gpu_neighborhood_native_cuda --log artifacts/logs/sdf_gpu_neighborhood_native_cuda.log --out-json artifacts/validation/sdf_gpu_neighborhood_native_cuda.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_sdf_postprocess_cuda` passed, `test_color_pipeline_sdf_postprocess_cuda: passed=78 failed=0`.
- Native CPU: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_gpu_neighborhood_native_cpu --log artifacts/logs/sdf_gpu_neighborhood_native_cpu.log --out-json artifacts/validation/sdf_gpu_neighborhood_native_cpu.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_sdf_postprocess` passed, `test_color_pipeline_sdf_postprocess: passed=78 failed=0`.
- Report native: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_gpu_neighborhood_report_native --log artifacts/logs/sdf_gpu_neighborhood_report_native.log --out-json artifacts/validation/sdf_gpu_neighborhood_report_native.json --heartbeat-seconds 30 --timeout-seconds 300 -- ui_app/build_tests_vsdevcmd.cmd test_viewer_ui_automation_report` passed.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_gpu_neighborhood_publish --log artifacts/logs/sdf_gpu_neighborhood_publish.log --out-json artifacts/validation/sdf_gpu_neighborhood_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_vsdevcmd.cmd` passed and produced `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Runtime SDF rows: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_gpu_neighborhood_runtime_sdf_rows --log artifacts/logs/sdf_gpu_neighborhood_runtime_sdf_rows.log --out-json artifacts/validation/sdf_gpu_neighborhood_runtime_sdf_rows.json --heartbeat-seconds 30 --timeout-seconds 900 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py` passed, including `cuda_field_signal` reporting for normal-angle SDF rows.
- Runtime pacing: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_gpu_neighborhood_runtime_pacing --log artifacts/logs/sdf_gpu_neighborhood_runtime_pacing.log --out-json artifacts/validation/sdf_gpu_neighborhood_runtime_pacing.json --heartbeat-seconds 30 --timeout-seconds 900 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_sdf_realtime_pacing.py` passed.
- Runtime capture/replay: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_gpu_neighborhood_runtime_capture_replay --log artifacts/logs/sdf_gpu_neighborhood_runtime_capture_replay.log --out-json artifacts/validation/sdf_gpu_neighborhood_runtime_capture_replay.json --heartbeat-seconds 30 --timeout-seconds 900 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_capture_replay_authority.py` passed.
- Performance witness: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_gpu_neighborhood_witness --log artifacts/logs/sdf_gpu_neighborhood_witness.log --out-json artifacts/validation/sdf_gpu_neighborhood_witness.json --heartbeat-seconds 30 --timeout-seconds 900 -- py -3.14 tools/viewer_host_sdf_performance_witness.py --runtime-exe D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe --out-json artifacts/sdf_gpu_postprocess_neighborhood/sdf_performance_witness.json --out-md artifacts/sdf_gpu_postprocess_neighborhood/sdf_performance_witness.md --work-dir artifacts/sdf_gpu_postprocess_neighborhood/work --width 640 --height 480 --include-preview-sample` passed.
- Measured field-signal witness: `sdf_normal_angle_fullres` reported `cuda_field_signal`, fallback `no`, postprocess `0.997 ms`, SDF total `3.308 ms`; `sdf_curvature_fullres` reported `cuda_field_signal`, fallback `no`, postprocess `0.967 ms`, SDF total `3.160 ms`; `sdf_normal_angle_curvature_stack` reported `cuda_field_signal`, fallback `no`, postprocess `0.936 ms`, SDF total `3.513 ms`.
- Remaining bottleneck witness: the witness recommendation moved to `field_generation_or_downsample_candidate`; postprocess pressure is no longer the dominant measured SDF cost in the focused scenarios.

## Hostile Audit

- Status: complete
- Did I actually move normal-angle/curvature SDF postprocess work to CUDA for supported stacks?
- Did CPU/GPU parity prove exact pixels for supported normal-angle, curvature, and mixed SDF-only stacks?
- Did row sample step greater than `1` still fall back instead of silently changing per-row quality semantics?
- Did direct scalar rows keep using the existing direct-scalar backend?
- Did runtime/reporting state the backend that actually ran?
- Did capture/replay, SDF rows, pacing, phase-signal behavior, per-row quality, and visible UI remain unchanged?

## Audit Passes

- [x] Pass 1 - RED/native CUDA proof that field-signal GPU backend support is missing.
- [x] Pass 2 - implementation diff audit for exact neighborhood sampling, blend/gate parity, and fallback handling.
- [x] Pass 3 - runtime proof audit for published viewer backend reporting and preserved SDF rows/capture behavior.
- [x] Pass 4 - clean re-read after hostile finding repairs.

## Audit Findings

- [x] The first implementation review found a fail-closed gap in explicit GPU backend requests: unsupported explicit `cuda_direct_scalar` or `cuda_field_signal` requests could silently fall into CPU behavior. Added native CUDA regressions and repaired explicit unsupported GPU backend requests to fail instead of hiding fallback.
- [x] The performance witness disproved the old postprocess bottleneck framing after this slice: field-signal postprocess is now under about `1.1 ms` in the focused witness, while SDF field generation dominates. Updated the roadmap truth to keep future work aimed at field generation/downsample authority rather than more CPU postprocess tuning.

## Acceptance

- Native CUDA proof covers CPU/GPU exact pixel parity for `sdf_normal_angle`, `sdf_curvature`, normal-angle-plus-curvature stacks, direct-plus-neighborhood stacks, boundary-gated rows, downsampled-field mapping, and output pixel step behavior.
- Native proof covers CPU fallback for row sample step greater than `1` and mixed SDF/non-SDF stacks.
- Viewer automation report exposes `cuda_field_signal` when the new backend actually runs.
- Published runtime proof shows SDF normal-angle/curvature stacks use CUDA when available and row-step paths fall back to CPU.
- SDF rows, SDF realtime pacing, and capture/replay focused runtime rails remain green.
- Performance witness is rerun and any improvement claim is limited to measured supported field-signal paths.
