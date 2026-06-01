# SDF Field Generation Downsample Optimization

## Current Phase

Phase 7 - final validation, receipts, rearward review, push, and clean-tree closeout.

## Phase Checklist

- [x] Phase 0 - merge the completed SDF engine sprint into `master`, push, verify rearward review `ok`, and branch `codex/sdf-field-generation-downsample-optimization`.
- [x] Phase 1 - create and lock this plan/contract from updated `master`.
- [x] Phase 2 - record a fresh baseline witness for Lens SDF, Lens Field v2, and `sdf_pack_scene`.
- [x] Phase 3 - add RED/native proof that authored SDF pack CUDA field generation should use a direct grid field producer instead of the generic host-point sampler path.
- [x] Phase 4 - implement the narrow direct-grid CUDA field path with CPU/CUDA parity and current pixel preservation.
- [x] Phase 5 - publish runtime and run no-mouse `sdf_pack_scene`/SDF performance proof.
- [x] Phase 6 - compare before/after field-generation timings and update roadmap truth without overclaiming FPS.
- [x] Phase 7 - hostile audit, final validators, checkpoint handoff, receipts, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [closed] Continue the next SDF engine step after the SDF engine completion sprint.
- [closed] Focus on field-generation/downsample optimization before new SDF ops.
- [closed] Provide numbers for hot-path performance changes.
- [closed] Preserve capture/replay/report authority, no-mouse proof, and current SDF/color behavior.

## Scope

In scope:

- Authored SDF pack field-generation performance for `sdf_pack_scene` and authored-pack field consumers.
- Direct CUDA field-grid generation that computes field pixels from viewport geometry without first materializing a host point array.
- Native CPU/CUDA parity for existing shipped SDF pack ops.
- Published runtime performance witness before and after the change.
- Roadmap/status updates that state measured results honestly.

Out of scope:

- New SDF ops, recursive/apollonian packs, or SDF-native families.
- New visible UI controls or broad Color Pipeline redesign.
- Changing SDF source-row semantics, row-local downsample semantics, capture/replay authority, or full-quality settled output.
- Mixed non-SDF Source support for `sdf_pack_scene` or `generic_equation_pack`.
- Physical mouse automation.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` reported clean branch `codex/sdf-engine-completion-sprint`, `HEAD=dc30c12`, active prior contract `sdf_engine_completion_sprint`, and code-quality baseline OK.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported no staged, unstaged, or untracked files.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `ok` for `dc30c12`.
- Merge setup: `master` fast-forwarded to `dc30c12`, pushed to `origin/master`, rearward review remained `ok`, and `codex/sdf-field-generation-downsample-optimization` was created.
- Baseline witness: `artifacts/sdf_field_generation_downsample_optimization/baseline_sdf_performance_witness.md` measured `sdf_pack_scene_signed_distance` at field `7.823 ms`, postprocess `0.648 ms`, SDF total `8.471 ms`, and last render `8.506 ms`.
- RED: `artifacts/validation/sdf_field_downsample_direct_grid_red.json` failed because `SdfPackFieldReport` did not expose `direct_grid_evaluation`, proving the native rail did not yet enforce direct grid evaluation.
- Native focused proof: `artifacts/validation/sdf_field_downsample_native_cuda.json` passed `test_sdf_pack_field_producer_cuda` with 608 checks and `test_viewer_ui_automation_report`.
- Witness unit proof: `artifacts/validation/sdf_field_downsample_witness_unit.json` passed `tests/test_sdf_performance_witness_tool.py` and `tests/test_fractal_runtime_sdf_performance_witness.py` with 5 tests.
- Runtime publish: `artifacts/validation/sdf_field_downsample_runtime_publish.json` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Runtime proof: `artifacts/validation/sdf_field_downsample_runtime_proof.json` passed 5 no-mouse runtime tests for `sdf_pack_scene` and SDF performance witness coverage.
- After witness: `artifacts/sdf_field_generation_downsample_optimization/after_sdf_performance_witness.md` measured `sdf_pack_scene_signed_distance` at field `1.341 ms`, postprocess `0.636 ms`, SDF total `1.976 ms`, last render `2.013 ms`, with `Pack Grid=yes`.
- Contract validation: `artifacts/validation/sdf_field_downsample_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed for this plan.
- Hostile audit validation: `artifacts/validation/sdf_field_downsample_hostile_audit.json` passed with two real findings and clean re-read evidence.
- Code quality: `artifacts/validation/sdf_field_downsample_code_quality.json` passed baseline with score 93/100.
- Diff check: `artifacts/validation/sdf_field_downsample_diff_check.json` passed `git diff --check`.

## Hostile Audit

- Status: complete
- Required posture: assume the first direct-grid implementation is wrong, slower, or subtly changes field pixels until CPU/CUDA parity and published runtime proof say otherwise.

Required questions:

- Did I prove the current `sdf_pack_scene` field-generation hotspot before changing it?
- Did the CUDA direct-grid path preserve field dimensions, pixel scale, sign convention, Y orientation, and CPU parity?
- Did I avoid changing Color Pipeline source semantics, row-local downsample semantics, capture/replay, or UI controls?
- Did the published runtime still load staged built-in SDF packs without repo-root dependence?
- Did I record before/after timing numbers and avoid claiming FPS improvement if timing is noisy?
- Did I keep unsupported field-primary mixed Source rows fail-closed?

## Audit Passes

- [x] Pass 1 - baseline/RED audit found the direct-grid report/status requirement was missing before implementation.
- [x] Pass 2 - implementation diff and parity audit found the runtime automation report had a `pack_direct_grid_evaluation` field in the struct but did not serialize or populate it.
- [x] Pass 3 - published runtime and performance audit found the performance witness JSON/Markdown did not expose the direct-grid bit, making speed evidence less self-verifying.
- [x] Pass 4 - clean re-read after repairs confirmed native CUDA parity, automation-report serialization, runtime proof, and after-witness artifacts now all carry the direct-grid signal.

## Audit Findings

- [x] The first implementation exposed direct-grid status only in the low-level report struct; it was not populated in `main.cpp` or serialized in `viewer_ui_automation_report.cpp`. Fixed by wiring `pack_direct_grid_evaluation` into the runtime probe, JSON report, and `sdf_pack_scene` runtime assertion.
- [x] The performance witness originally captured timing but not whether the authored-pack field used direct-grid evaluation. Fixed by adding `lens_sdf_pack_direct_grid_evaluation` to the witness JSON aggregation and a `Pack Grid` Markdown column.
- [x] Clean re-read after repairs did not expose another real defect: focused native, witness unit, runtime publish, runtime proof, and after-witness paths are green.

## Planned Validation Targets

- `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_downsample_native_cuda --log artifacts/logs/sdf_field_downsample_native_cuda.log --out-json artifacts/validation/sdf_field_downsample_native_cuda.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_sdf_pack_field_producer_cuda`
- `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_downsample_witness_unit --log artifacts/logs/sdf_field_downsample_witness_unit.log --out-json artifacts/validation/sdf_field_downsample_witness_unit.json --heartbeat-seconds 30 --timeout-seconds 300 -- py -3.14 -m pytest tests/test_sdf_performance_witness_tool.py tests/test_fractal_runtime_sdf_performance_witness.py -q`
- `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_downsample_runtime_publish --log artifacts/logs/sdf_field_downsample_runtime_publish.log --out-json artifacts/validation/sdf_field_downsample_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 1200 -- cmd /c ui_app\build_vsdevcmd.cmd`
- `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_downsample_runtime_proof --log artifacts/logs/sdf_field_downsample_runtime_proof.log --out-json artifacts/validation/sdf_field_downsample_runtime_proof.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_sdf_pack_scene_lane.py tests/test_fractal_runtime_sdf_performance_witness.py`
- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_field_generation_downsample_optimization.contract.json --out-json artifacts/validation/sdf_field_downsample_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/sdf_field_generation_downsample_optimization_PHASED_PLAN.md --out-json artifacts/validation/sdf_field_downsample_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/sdf_field_downsample_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_downsample_diff_check --log artifacts/logs/sdf_field_downsample_diff_check.log --out-json artifacts/validation/sdf_field_downsample_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`
