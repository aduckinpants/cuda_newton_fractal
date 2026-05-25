# CUDA Lens SDF Backend

## Current Phase

Closed - CUDA Lens SDF backend is implemented, audited, and validated.

## Phase Checklist

- [x] Phase 1 - create and lock this checked-in plan/contract
- [x] Phase 2 - add RED native CUDA backend tests and build target
- [x] Phase 3 - add explicit Lens SDF backend API with CPU default/reference preserved
- [x] Phase 4 - implement CUDA Jump Flooding backend and semantic parity checks
- [x] Phase 5 - run focused native rails, preservation rails, contract/plan/audit/code-quality/diff validation, rearward review, checkpoint, receipts, push

## Explicit User Asks

- [done] Modernize Lens SDF generation with a CUDA-backed field producer beside the current CPU chamfer path.
- [done] Keep `ComputeLensSdfFieldForMask(...)` CPU-default for this slice.
- [done] Add an explicit backend API with `cpu_chamfer`, `cuda_jfa`, and `auto`.
- [done] Keep CPU chamfer as correctness/fallback authority and prove semantic parity, not bit-exact equality.
- [done] Do not start live Color Pipeline SDF rows, viewport overlay, SDF-native lanes, or live client routing in this headless-first slice.

## Scope

In scope:

- `ui_app/src/lens_sdf.h`
- `ui_app/src/lens_sdf.cpp`
- new CUDA Lens SDF backend source/header as needed
- `ui_app/tests/test_lens_sdf.cpp`
- new `ui_app/tests/test_lens_sdf_cuda.cu`
- `ui_app/build_tests_vsdevcmd.cmd`
- this phased plan and its contract
- `HANDOFF_LOG.md`

Out of scope:

- Live client routing to `auto`.
- Runtime/headless report backend reporting.
- Lens SDF elapsed-time reporting.
- Color Pipeline SDF rows.
- Viewport overlay.
- SDF-native fractal lanes.
- Renderer replacement or passing SDF textures into fractal kernels.

## Proof Ledger

- Start authority: branch `codex/gpu-lens-sdf-backend` was created from clean pushed `master` at `dada3b8f846c5fa2f755c2e7b9677e76be974def`.
- Current seam authority: `ui_app/src/lens_sdf.cpp` owns CPU chamfer field generation and `ComputeLensSdfFieldForMask(...)`; `ui_app/build_tests_vsdevcmd.cmd` owns focused native test targets.
- RED authority: `py -3.14 tools/viewer_host_run_logged_command.py --label cuda_lens_sdf_backend_test_lens_sdf_cuda_red --log artifacts/logs/cuda_lens_sdf_backend_test_lens_sdf_cuda_red.log --out-json artifacts/validation/cuda_lens_sdf_backend_test_lens_sdf_cuda_red.json --heartbeat-seconds 30 --timeout-seconds 120 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_lens_sdf_cuda` failed because `test_lens_sdf_cuda` was not a known focused target.
- CUDA GREEN authority: `py -3.14 tools/viewer_host_run_logged_command.py --label cuda_lens_sdf_backend_test_lens_sdf_cuda --log artifacts/logs/cuda_lens_sdf_backend_test_lens_sdf_cuda.log --out-json artifacts/validation/cuda_lens_sdf_backend_test_lens_sdf_cuda.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_lens_sdf_cuda` passed after adding the CUDA JFA backend and semantic parity tests.
- CPU preservation authority: `py -3.14 tools/viewer_host_run_logged_command.py --label cuda_lens_sdf_backend_test_lens_sdf --log artifacts/logs/cuda_lens_sdf_backend_test_lens_sdf.log --out-json artifacts/validation/cuda_lens_sdf_backend_test_lens_sdf.json --heartbeat-seconds 30 --timeout-seconds 180 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_lens_sdf` passed, preserving the existing CPU Lens SDF rail.
- Consumer preservation authority: focused `test_flashlight_probe` and `test_runtime_walk_headless` logged rails passed after the Lens SDF core change.
- Implementation boundary: `ComputeLensSdfFieldForMask(...)` remains implemented in `lens_sdf.cpp` and remains CPU chamfer/default; the explicit backend API is CUDA-linked for this headless-first slice.
- Code-quality repair authority: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/cuda_lens_sdf_backend_code_quality.json` initially exposed a new `RunCudaJfa()` length warning; after refactoring CUDA buffer ownership, the same command passed at baseline score `96/100` with no new warning.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/cuda_lens_sdf_backend.contract.json --out-json artifacts/validation/cuda_lens_sdf_backend_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile-audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/cuda_lens_sdf_backend_PHASED_PLAN.md --out-json artifacts/validation/cuda_lens_sdf_backend_hostile_audit.json` passed.
- Diff hygiene: `py -3.14 tools/viewer_host_run_logged_command.py --label cuda_lens_sdf_backend_diff_check --log artifacts/logs/cuda_lens_sdf_backend_diff_check.log --out-json artifacts/validation/cuda_lens_sdf_backend_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.

## Hostile Audit

- Status: done
- Required posture: assume CUDA output has wrong sign convention, boundary shape, dimensions, invalid-input behavior, fallback behavior, or silently changes the CPU-default public API until tests prove otherwise.

## Audit Passes

- [done] Pass 1 - verified CPU-default `ComputeLensSdfFieldForMask(...)` remains behavior-compatible through the existing `test_lens_sdf` rail and explicit CPU backend parity in `test_lens_sdf_cuda`.
- [done] Pass 2 - verified CUDA parity is semantic and bounded: sign convention, dimensions, finite distances, uniform masks, odd dimensions, downsample, and bounded distance error are tested without claiming bit-exact chamfer equality.
- [done] Pass 3 - verified this slice does not route live client behavior, Color Pipeline rows, viewport overlay, SDF-native lanes, or renderer changes.

## Audit Findings

- [done] Real defect found and repaired: the first CUDA Jump Flooding loop could consume a kernel launch error inside the loop and then continue as if the loop had succeeded; the loop now carries an explicit `kernelsOk` failure flag.
- [done] Real code-quality defect found and repaired: the first `RunCudaJfa()` implementation introduced a new code-quality warning; CUDA buffer ownership is now factored into `CudaJfaBuffers`.
- [done] Clean re-read: the repaired state leaves live Lens SDF routing, runtime backend reporting, Color Pipeline SDF rows, viewport overlay, SDF-native lanes, and renderer integration out of scope.

## Action Hostile Review

- Action ID: cuda-lens-sdf-backend-start
- Suspected failure mode: adding a GPU path could silently replace the trusted CPU chamfer Lens SDF path or claim performance/product improvement without backend parity and focused tests.
- Correct owner/action: add an explicit backend API and CUDA JFA producer beside CPU chamfer, leaving the shipped CPU default untouched in this slice.
- Proof surface: focused RED/GREEN `test_lens_sdf_cuda`, existing `test_lens_sdf`, flashlight/runtime-walk preservation rails if touched, contract validation, phased-plan sync, hostile-audit validation, code-quality baseline, diff hygiene, and rearward review.
- Blocked action: live client auto routing, runtime report backend fields, Color Pipeline SDF rows, viewport overlay, SDF-native lanes, renderer replacement, or physical mouse automation.
