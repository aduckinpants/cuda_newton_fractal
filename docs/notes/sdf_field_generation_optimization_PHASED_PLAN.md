# SDF Field Generation Optimization

## Current Phase

Complete - field-generation stage telemetry and safe CUDA JFA buffer reuse are implemented, validated, and checkpointed for merge.

## Phase Checklist

- [x] Phase 0 - start from pushed `master` at `a41d221` after the field-generation preflight merge.
- [x] Phase 1 - create and lock this viewer-first implementation plan/contract.
- [x] Phase 2 - add RED/native report proof for missing field-generation stage telemetry.
- [x] Phase 3 - report cache lookup, mask downsample, backend generation, and cache store timing without changing SDF pixels.
- [x] Phase 4 - publish runtime and prove no-mouse SDF reports expose the new field-stage telemetry on real viewer paths.
- [x] Phase 5 - if measurement exposes a safe tiny optimization, land it with parity; otherwise stop at measurement and do not claim performance improvement.
- [x] Phase 6 - hostile audit, plan sync, receipts, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [done] Move forward on field-generation/optimizations because that base is foundational to later SDF features.
- [done] Do not guess; measure before optimizing.
- [done] Preserve Lens Field v2, capture/replay authority, SDF postprocess CUDA paths, adaptive field resolution, and current Color Pipeline behavior.
- [done] Keep per-row/per-function downsample, authored SDF UI, SDF-native lanes, and broader composition UX out of this slice.

## Scope

In scope:

- Runtime-visible field-generation stage telemetry.
- Report fields for cache lookup, mask downsample, backend generation, cache store, requested/effective dimensions, and existing cache status.
- Native and runtime no-mouse proof that the report fields exist and are internally consistent.
- A very small safe optimization only if it falls directly out of the measurement seam and preserves exact behavior, such as avoiding duplicate mask downsample on CUDA fallback.

Out of scope:

- Per-row/per-function SDF downsample.
- New user-visible controls.
- SDF-native fractal lanes or authored SDF pack viewport UI.
- GPU Color Pipeline postprocess changes.
- Physical mouse automation.
- Claims that viewer responsiveness is solved without measured published-runtime evidence.

## Proof Ledger

- Bootstrap/rearward source: field-generation preflight merged to pushed `master` at `a41d221` with rearward review `ok`.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "SDF field generation optimization" --profile runtime --plan docs/notes/sdf_field_generation_optimization_PHASED_PLAN.md --contract docs/contracts/sdf_field_generation_optimization.contract.json` appended `ck:ff8d5958` and locked the active contract.
- RED: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_generation_red_report --log artifacts/logs/sdf_field_generation_red_report.log --out-json artifacts/validation/sdf_field_generation_red_report.json --heartbeat-seconds 30 --timeout-seconds 300 -- ui_app/build_tests_vsdevcmd.cmd test_viewer_ui_automation_report` failed because `ViewerUiAutomationLensSdfProbe` lacked field-stage report members.
- Native report rail: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_generation_native_report --log artifacts/logs/sdf_field_generation_native_report.log --out-json artifacts/validation/sdf_field_generation_native_report.json --heartbeat-seconds 30 --timeout-seconds 300 -- ui_app/build_tests_vsdevcmd.cmd test_viewer_ui_automation_report` passed.
- Native CUDA rail: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_generation_native_cuda --log artifacts/logs/sdf_field_generation_native_cuda.log --out-json artifacts/validation/sdf_field_generation_native_cuda.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_lens_sdf_cuda` passed.
- Witness unit: `py -3.14 -m pytest tests/test_sdf_performance_witness_tool.py -q` passed.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_generation_runtime_publish --log artifacts/logs/sdf_field_generation_runtime_publish.log --out-json artifacts/validation/sdf_field_generation_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published runtime pacing: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_generation_runtime_pacing --log artifacts/logs/sdf_field_generation_runtime_pacing.log --out-json artifacts/validation/sdf_field_generation_runtime_pacing.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_sdf_realtime_pacing.py` passed.
- Published performance witness: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_generation_witness --log artifacts/logs/sdf_field_generation_witness.log --out-json artifacts/validation/sdf_field_generation_witness.json --heartbeat-seconds 30 --timeout-seconds 900 -- py -3.14 tools/viewer_host_sdf_performance_witness.py --runtime-exe D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe --out-json artifacts/sdf_field_generation_optimization/sdf_performance_witness.json --out-md artifacts/sdf_field_generation_optimization/sdf_performance_witness.md --work-dir artifacts/sdf_field_generation_optimization/work --width 640 --height 480 --include-preview-sample` passed; report fields include cache lookup, mask downsample, backend generation, and cache store timing.
- Witness outcome: the current representative matrix reports `postprocess_optimization_candidate`, so this slice does not claim the FPS problem is solved by field generation alone.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_field_generation_optimization.contract.json --out-json artifacts/validation/sdf_field_generation_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile-audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/sdf_field_generation_optimization_PHASED_PLAN.md --out-json artifacts/validation/sdf_field_generation_hostile_audit.json` passed.
- Code quality: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/sdf_field_generation_code_quality.json` passed.
- Diff check: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_generation_diff_check --log artifacts/logs/sdf_field_generation_diff_check.log --out-json artifacts/validation/sdf_field_generation_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.

## Hostile Audit

- Status: complete
- Did I add real field-generation sub-stage telemetry instead of another summary-only timing?
- Did I keep current SDF pixels, capture/replay, and settled/full-quality behavior unchanged?
- Did I avoid per-row downsample and UI feature drift?
- Did I prove the published runtime report path, not just helper/unit code?
- Did I avoid claiming performance improvement unless the witness supports it?

## Audit Passes

- [done] Pass 1 - RED/report gap audit.
- [done] Pass 2 - implementation diff audit for pixel/replay/behavior changes.
- [done] Pass 3 - runtime proof audit for published report fields.
- [done] Pass 4 - clean re-read after findings were repaired.

## Audit Findings

- [done] Finding 1: the old report only exposed aggregate `lens_sdf_field_ms`, so it could not distinguish cache lookup, mask downsample, backend generation, or cache store cost. Repaired with new published report fields and witness output columns.
- [done] Finding 2: the CUDA auto-backend fallback path could recompute host mask downsample when CUDA JFA failed. Repaired by reusing the already-downsampled mask for CPU fallback while preserving CPU parity.
- [done] Finding 3: the CUDA JFA path allocated and freed all device buffers every field generation. Repaired with reusable capacity-checked JFA device buffers; native CUDA parity and published runtime rails passed.
- [done] Finding 4: the runtime pacing test had a brittle "slow enough" timing threshold that failed once field generation improved slightly. Repaired by making only the heavy-stack pressure target explicit while leaving the scalar low-cost path unchanged.

## Notes

- The first useful deliverable is trustworthy stage telemetry. If the optimization candidate is not safe inside this slice, close with measurement and route the next branch from that evidence.
