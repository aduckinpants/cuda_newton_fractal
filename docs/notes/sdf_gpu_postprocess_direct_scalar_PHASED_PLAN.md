# SDF GPU Postprocess Direct Scalar

## Current Phase

Phase 5 - validation and closure for the CUDA direct-scalar SDF postprocess backend.

## Phase Checklist

- [x] Phase 0 - bootstrap clean `codex/sdf-adaptive-preview-pacing` head, confirm rearward review is ok, and close stale prior plan truth.
- [x] Phase 1 - create this checked-in plan/contract and lock the active slice.
- [x] Phase 2 - add focused RED native CUDA proof for a missing direct-scalar GPU SDF postprocess backend and reporting surface.
- [x] Phase 3 - implement a CUDA direct-scalar backend for `sdf_signed_distance`, `sdf_inside_outside`, and `sdf_boundary_band`, with CPU fallback for unsupported rows.
- [x] Phase 4 - wire build/report/runtime surfaces without visible UI behavior changes.
- [x] Phase 5 - publish runtime, run focused native/runtime/performance rails, hostile audit, receipts, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [active] Continue the next SDF performance slice after the user-tested per-row quality work looked good.
- [active] Improve SDF responsiveness using measured seams rather than another debounce guess.
- [active] Keep UI behavior stable while optimizing the backend path.
- [active] Preserve Capture Finding, SDF rows, phase behavior, per-row quality, and current Color Pipeline semantics.
- [active] Do not use physical mouse automation.

## Scope

In scope:

- A new optional CUDA SDF Color Pipeline postprocess backend for direct scalar SDF Source stacks.
- Direct scalar SDF signals only: `sdf_signed_distance`, `sdf_inside_outside`, and `sdf_boundary_band`.
- Exact CPU/GPU pixel parity for supported direct scalar stacks at default/full-quality semantics.
- Runtime/reporting fields that state which SDF postprocess backend actually ran.
- CPU fallback for normal-angle, curvature, row sample-step greater than `1`, mixed non-SDF stacks, CUDA failure, and unsupported options.

Out of scope:

- GPU postprocess for `sdf_normal_angle` or `sdf_curvature`.
- GPU-resident renderer/color-pipeline rewrite.
- Per-source or multi-field SDF downsample authority.
- Visible Color Pipeline UI redesign, authored SDF pack UI, SDF-native fractal lanes, or Salticid runtime dependency.
- Capture-quality changes, preview-policy changes, or physical mouse automation.

## Implementation Direction

The first GPU postprocess vertical should be a safe backend seam, not a renderer rewrite. The CPU implementation remains the reference. The CUDA path may run only when the source stack is made entirely of supported direct scalar SDF rows with row sample step `1`, the existing shape/palette/grading functions are device-callable, and the output mapping can match CPU pixels exactly.

The GPU helper should:

- preflight the source stack before launching kernels;
- copy the existing host SDF field to device for this slice;
- render directly into a device RGBA buffer and copy back to the existing host frame buffer;
- report `cuda_direct_scalar` only when the CUDA path actually completed;
- fall back to `cpu` without changing output for unsupported or failed paths;
- leave the existing CPU serial/parallel postprocess path intact.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on clean `codex/sdf-adaptive-preview-pacing` at `00d4620`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported a clean tree.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `00d4620`.
- Prior stale-plan repair: `00d4620` closed the previous per-row SDF plan truth before this product slice opened.
- Contract bootstrap: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_gpu_postprocess_direct_scalar.contract.json --out-json artifacts/validation/sdf_gpu_postprocess_contract_bootstrap.json` passed before product mutation.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "SDF GPU postprocess direct scalar" --profile runtime --plan docs/notes/sdf_gpu_postprocess_direct_scalar_PHASED_PLAN.md --contract docs/contracts/sdf_gpu_postprocess_direct_scalar.contract.json` opened token `ck:9720efb3`.
- RED: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_gpu_postprocess_red_cuda --log artifacts/logs/sdf_gpu_postprocess_red_cuda.log --out-json artifacts/validation/sdf_gpu_postprocess_red_cuda.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_sdf_postprocess_cuda` failed on the missing backend enum/options/stats/preflight surface.
- Native CUDA: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_gpu_postprocess_native_cuda --log artifacts/logs/sdf_gpu_postprocess_native_cuda.log --out-json artifacts/validation/sdf_gpu_postprocess_native_cuda.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_sdf_postprocess_cuda` passed, `test_color_pipeline_sdf_postprocess_cuda: passed=46 failed=0`.
- Native CPU: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_gpu_postprocess_native_cpu --log artifacts/logs/sdf_gpu_postprocess_native_cpu.log --out-json artifacts/validation/sdf_gpu_postprocess_native_cpu.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_sdf_postprocess` passed, `test_color_pipeline_sdf_postprocess: passed=78 failed=0`.
- Report native: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_gpu_postprocess_report_native --log artifacts/logs/sdf_gpu_postprocess_report_native.log --out-json artifacts/validation/sdf_gpu_postprocess_report_native.json --heartbeat-seconds 30 --timeout-seconds 300 -- ui_app/build_tests_vsdevcmd.cmd test_viewer_ui_automation_report` passed.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_gpu_postprocess_publish --log artifacts/logs/sdf_gpu_postprocess_publish.log --out-json artifacts/validation/sdf_gpu_postprocess_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_vsdevcmd.cmd` passed and produced `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Runtime SDF rows: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_gpu_postprocess_runtime_sdf_rows --log artifacts/logs/sdf_gpu_postprocess_runtime_sdf_rows.log --out-json artifacts/validation/sdf_gpu_postprocess_runtime_sdf_rows.json --heartbeat-seconds 30 --timeout-seconds 900 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py` passed, including direct-scalar CUDA reporting and unsupported CPU fallback reporting.
- Runtime pacing: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_gpu_postprocess_runtime_pacing --log artifacts/logs/sdf_gpu_postprocess_runtime_pacing.log --out-json artifacts/validation/sdf_gpu_postprocess_runtime_pacing.json --heartbeat-seconds 30 --timeout-seconds 900 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_sdf_realtime_pacing.py` passed.
- Runtime capture/replay: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_gpu_postprocess_runtime_capture_replay --log artifacts/logs/sdf_gpu_postprocess_runtime_capture_replay.log --out-json artifacts/validation/sdf_gpu_postprocess_runtime_capture_replay.json --heartbeat-seconds 30 --timeout-seconds 900 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_capture_replay_authority.py` passed.
- Performance witness: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_gpu_postprocess_witness --log artifacts/logs/sdf_gpu_postprocess_witness.log --out-json artifacts/validation/sdf_gpu_postprocess_witness.json --heartbeat-seconds 30 --timeout-seconds 900 -- py -3.14 tools/viewer_host_sdf_performance_witness.py --runtime-exe D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe --out-json artifacts/sdf_gpu_postprocess_direct_scalar/sdf_performance_witness.json --out-md artifacts/sdf_gpu_postprocess_direct_scalar/sdf_performance_witness.md --work-dir artifacts/sdf_gpu_postprocess_direct_scalar/work --width 640 --height 480 --include-preview-sample` passed. Direct scalar rows reported `cuda_direct_scalar`; normal-angle/curvature scenarios reported CPU fallback.
- Measured direct-scalar witness: `sdf_signed_distance_fullres` reported `cuda_direct_scalar`, fallback `no`, postprocess `1.172 ms`, total SDF `3.288 ms`; `sdf_signed_distance_downsample4` reported `cuda_direct_scalar`, fallback `no`, postprocess `0.781 ms`, total SDF `1.473 ms`.
- Measured remaining bottleneck witness: `sdf_normal_angle_fullres` stayed CPU fallback at `11.514 ms`, `sdf_curvature_fullres` stayed CPU fallback at `10.378 ms`, and `sdf_normal_angle_curvature_stack_settled_full_quality` stayed CPU fallback at `90.550 ms` postprocess / `99.481 ms` SDF total. This slice does not close the broad phase/curvature FPS problem.

## Hostile Audit

- Status: complete
- Did I actually accelerate only the supported direct scalar SDF postprocess path, not silently change phase/curvature behavior?
- Did CPU/GPU parity prove exact pixels for supported stacks?
- Did unsupported stacks reliably fall back to CPU instead of producing partial/wrong output?
- Did runtime/reporting state the backend that actually ran?
- Did Capture Finding, SDF row controls, per-row sample step, phase signal behavior, and current UI remain unchanged?
- Did I avoid physical mouse automation, renderer rewrites, authored SDF UI, SDF-native lanes, and per-source field downsample?

## Audit Passes

- [x] Pass 1 - RED/native CUDA proof that the backend/reporting seam is missing.
- [x] Pass 2 - implementation diff audit for exact-pixel parity, fallback, and unsupported-stack handling.
- [x] Pass 3 - runtime proof audit for published viewer backend reporting and preserved SDF rows/capture behavior.
- [x] Pass 4 - clean re-read after hostile findings and repairs.

## Audit Findings

- [x] Contract bootstrap rejected the initial allowed-scope shape until the new CUDA header/source/test files existed. Added the checked-in files before locking product mutation so the active contract could validate against real mutation scope.
- [x] First CUDA native pass produced an unused helper warning in the new CUDA translation unit. Removed the unused helper and reran the focused CUDA native rail.
- [x] The performance witness proves the direct scalar backend is active and fast, but it also proves the broad SDF FPS problem is not closed: phase, curvature, and normal-angle-plus-curvature stacks still fall back to CPU and remain the dominant postprocess cost.

## Acceptance

- Native CUDA proof covers CPU/GPU exact pixel parity for signed distance, inside/outside, boundary band, direct scalar multi-row stack, downsampled-field mapping, and output pixel step behavior.
- Native proof covers CPU fallback for normal-angle, curvature, mixed stacks, and row sample step greater than `1`.
- Viewer automation report exposes the actual SDF postprocess backend.
- Published runtime proof shows a supported direct scalar stack uses CUDA when available and an unsupported phase/curvature stack falls back to CPU.
- SDF rows, SDF realtime pacing, and capture/replay focused runtime rails remain green.
- Performance witness is rerun and any improvement claim is limited to the measured supported direct scalar path.
