# SDF Field-Primary Direct-Grid Optimization

## Explicit User Asks

- Continue from the clean pushed SDF field-resolution optimization matrix work.
- Merge the measurement branch into `master` when appropriate.
- Open the next bounded SDF implementation slice targeting the measured first seam.
- Optimize field-primary direct-grid field generation/downsample for `sdf_pack_scene` and `explaino_root_sdf`.
- Start with a baseline repeated-sample performance witness.
- Preserve existing visual behavior, capture/replay, and Color Pipeline semantics.
- Record before/after numbers.
- Stop for replan before new SDF ops, new visual features, per-source downsample UX, broad Color Pipeline UI work, or additional SDF-native lanes.

## Current Phase

Phase 6 - hostile audit, receipts, rearward review, push, and clean-tree close.

## Phase Checklist

- [x] Phase 0: merge `codex/sdf-field-resolution-optimization-matrix` into `master`, push, and rearward-review merged `master`.
- [x] Phase 0: create this checked-in implementation plan and contract from clean `master` at `c6e20bf`.
- [x] Phase 1: publish runtime and capture baseline repeated-sample witness for field-primary direct-grid scenarios.
- [x] Phase 2: inspect direct-grid field producer code and select one minimal behavior-preserving optimization.
- [x] Phase 3: implement the selected optimization without changing public visuals or state/capture authority.
- [x] Phase 4: run focused native rails and published no-mouse runtime proof.
- [x] Phase 5: capture after repeated-sample witness and compare before/after numbers.
- [ ] Phase 6: hostile audit, repair findings, receipts, rearward review, push, and clean-tree close.

## Scope

In scope:

- `sdf_pack_scene` field-primary direct-grid generation cost.
- `explaino_root_sdf` field-primary direct-grid generation cost.
- Baseline and after repeated-sample witnesses at the same dimensions/repeat count.
- Native parity/sensitivity tests for the touched field producer path.
- Published no-mouse runtime proof that frame hashes remain deterministic and visual authority is preserved.

Out of scope:

- New SDF ops.
- New visual features.
- New SDF-native lanes.
- Per-source downsample UX.
- Broad Color Pipeline UI or graph/editor work.
- Mask-derived Lens SDF cache/cold-miss optimization unless the direct-grid implementation strictly requires a shared helper refactor.
- Physical mouse automation.

## Phase 0 - Merge And Slice Open

### Evidence

- `codex/sdf-field-resolution-optimization-matrix` was rearward-reviewed `ok`.
- `master` fast-forwarded from `b4dd99e` to `c6e20bf`.
- `origin/master` was pushed at `c6e20bf`.
- Rearward review for merged `master` at `c6e20bf` returned `ok`.
- Branch `codex/sdf-field-primary-grid-optimization` was created from `c6e20bf`.

## Phase 1 - Baseline Witness

### Intent

Capture before numbers on the exact published runtime before changing field producer code.

### Required Evidence

- Runtime publish command.
- Repeated-sample witness with `--repeat-count 5`, width `640`, height `480`, and preview sample enabled.
- Baseline JSON and Markdown under `artifacts/sdf_field_primary_grid_optimization/`.

### Baseline Rows To Inspect

- `sdf_pack_scene_signed_distance`
- `explaino_root_sdf_static`
- `explaino_root_sdf_phase_sine`
- `explaino_root_sdf_regular_n16_static`
- `explaino_root_sdf_regular_n16_phase_sine`

### Evidence

- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_primary_grid_baseline_runtime_publish ... -- ui_app/build_vsdevcmd.cmd`, success.
- Baseline witness: `py -3.14 tools/viewer_host_sdf_performance_witness.py --runtime-exe D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe --out-json artifacts/sdf_field_primary_grid_optimization/baseline_sdf_performance_witness.json --out-md artifacts/sdf_field_primary_grid_optimization/baseline_sdf_performance_witness.md --work-dir artifacts/sdf_field_primary_grid_optimization/baseline_work --width 640 --height 480 --repeat-count 5 --include-preview-sample --timeout-seconds 120`, success.
- Baseline recommendation: `field_generation_or_downsample_candidate`.

## Phase 2 - Direct-Grid Code Inspection

### Intent

Identify one minimal optimization seam from code and baseline evidence.

Candidate seams:

- Avoid unnecessary per-sample descriptor or pack rebuild work.
- Reduce repeated AST/scene traversal overhead in direct grid producers.
- Add a safe cache only for field-primary producers when the field-authority key is unchanged.
- Improve downsample/effective resolution handling only if it preserves full-quality capture/replay semantics.

Rejected until later:

- Approximate pixels, LUT quantization, or visual simplification.
- Per-source downsample UI.
- New pack operators.
- Mixing mask-derived Lens SDF cache policy into this direct-grid slice.

### Selected Seam

- Add a descriptor-level field producer that accepts an already-lowered `SdfPackRuntimeDesc`.
- Route `explaino_root_sdf` through direct descriptor construction instead of serializing a dynamic JSON pack, parsing it, and lowering it every field compute.
- Add a transient descriptor cache for `sdf_pack_scene` / authored SDF pack viewer state keyed by exact pack JSON and parameter values, then route viewport and preview field generation through the descriptor-level producer.

## Phase 3 - Implementation

### Intent

Land only the first selected behavior-preserving optimization.

### Required Preservation

- Same selected fractal type and controls.
- Same capture/replay authority.
- Same Color Pipeline source-stack semantics.
- Full-quality frame hashes must either remain unchanged or any changed hash must be explained as intentional and covered by stronger semantic/visual proof. Default target is unchanged pixels.

### Implementation Evidence

- Added descriptor field request/CPU path/CUDA backend registration in `ui_app/src/sdf_pack_field_producer.h`, `ui_app/src/sdf_pack_field_producer.cpp`, and `ui_app/src/sdf_pack_field_producer_cuda.cu`.
- Replaced Root SDF dynamic JSON parse/lower path with direct runtime descriptor construction in `ui_app/src/explaino_root_sdf_field.cpp`.
- Added non-persisted SDF pack viewer descriptor cache and routed preview/viewport field generation through `ComputeSdfPackRuntimeFieldWithBackend(...)`.

## Phase 4 - Validation

### Native Rails

- Field producer tests for any touched producer.
- Root SDF field tests if `explaino_root_sdf` code changes.
- SDF pack field producer tests if `sdf_pack_scene` code changes.
- Viewer automation report tests if report fields change.

### Runtime Rails

- Publish runtime.
- No-mouse runtime proof for `sdf_pack_scene` and `explaino_root_sdf`.
- Capture/replay proof remains green for field-primary SDF lanes.

### Evidence

- Native helper build after descriptor producer patch: `artifacts/validation/sdf_field_primary_grid_native_build_after_descriptor_patch.json`, success.
- Native helper build after pack viewer cache patch: `artifacts/validation/sdf_field_primary_grid_native_build_after_pack_cache.json`, success.
- Runtime publish after implementation: `artifacts/validation/sdf_field_primary_grid_after_runtime_publish.json`, success.
- Focused published no-mouse runtime proof: `py -3.14 -m pytest tests/test_fractal_runtime_sdf_pack_scene_lane.py tests/test_fractal_runtime_explaino_root_sdf.py tests/test_fractal_runtime_sdf_performance_witness.py`, `9 passed`.
- Hostile review finding during runtime proof: `test_sdf_pack_scene_lane_selects_and_edits_built_in_pack_no_mouse` expected `lens_field_v2_distance` for `sdf_pack_scene`, but capability code intentionally reports Lens Field v2 only for producer kind `lens_field_v2`. The stale test was repaired to assert honest supported signals instead of false capability advertising.

## Phase 5 - After Witness And Decision

### Intent

Compare the same repeated-sample witness before and after implementation.

Required output:

- Before/after table for median field time, postprocess time, total SDF time, frame hash stability, and recommendation.
- Explicit performance claim if and only if the numbers support it.
- Explicit `optimization_unproven` wording if timing is noisy.

### Evidence

- After witness: `artifacts/sdf_field_primary_grid_optimization/after_sdf_performance_witness.json` and `.md`, success.
- Before/after target-row comparison: `artifacts/sdf_field_primary_grid_optimization/before_after_target_rows.json`.
- Same witness configuration: width `640`, height `480`, repeat count `5`, preview sample enabled.
- Recommendation remained `field_generation_or_downsample_candidate`; the first seam improved direct-grid field generation but did not fully remove field/downsample pressure.

| Scenario | Field ms before | Field ms after | Field delta | SDF total delta | Hash |
| --- | ---: | ---: | ---: | ---: | --- |
| `sdf_pack_scene_signed_distance` | 1.6456 | 1.1706 | -28.9% | -24.6% | preserved |
| `explaino_root_sdf_static` | 1.8840 | 1.2159 | -35.5% | -28.0% | preserved |
| `explaino_root_sdf_phase_sine` | 1.6654 | 1.2253 | -26.4% | -24.0% | preserved |
| `explaino_root_sdf_regular_n16_static` | 4.0467 | 3.1330 | -22.6% | -17.9% | preserved |
| `explaino_root_sdf_regular_n16_phase_sine` | 3.5884 | 3.0636 | -14.6% | -11.3% | preserved |

## Phase 6 - Close

### Exit Criteria

- Hostile audit records a real finding or enough clean re-read passes.
- Contract validation, plan sync, code-quality baseline, hostile-audit validation, and diff check pass.
- Validation and contract proof receipts are written.
- Rearward review is `ok`.
- Branch is pushed with a clean tree.

## Hostile Audit

- Status: complete

Audit questions:

- Did I optimize only the measured field-primary direct-grid seam?
- Did I preserve pixels/capture/replay/state authority?
- Did I avoid new SDF ops, new visual features, per-source downsample UX, and broad Color Pipeline UI work?
- Did I record before/after numbers from the same witness configuration?
- Did I avoid claiming performance improvement without evidence?

## Audit Passes

- [x] Pass 1 - Runtime proof found one real issue: a stale `sdf_pack_scene` supported-signal expectation claimed Lens Field v2-only support. Repaired the test to require honest capability reporting.
- [x] Pass 2 - Clean re-read of touched producer seams after repair: descriptor field request preserves geometry, backend selection, fallback behavior, source kind assignment, and pack id reporting; no additional real issue found.
- [x] Pass 3 - Clean re-read of proof artifacts after repair: focused native helper build, runtime publish, no-mouse runtime tests, after witness, code-quality audit, and diff check are green; no additional workflow mistake found.

## Audit Findings

- [x] Runtime proof found a stale capability expectation in `tests/test_fractal_runtime_sdf_pack_scene_lane.py`: `sdf_pack_scene` was expected to advertise `lens_field_v2_distance`, contradicting the current producer-kind capability gate. Repaired the test to require the five supported SDF field signals and assert that Lens Field v2-only signal is not advertised.

## Proof Ledger

| Item | Evidence |
| --- | --- |
| Measurement branch merge | `git merge --ff-only codex/sdf-field-resolution-optimization-matrix` fast-forwarded `master` to `c6e20bf`; `git push origin master` pushed it. |
| Merged-head rearward review | `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `c6e20bf`. |
| Active contract | `py -3.14 tools/viewer_host_begin_work_slice.py --intent "SDF field-primary direct-grid optimization" --profile runtime --plan docs/notes/sdf_field_primary_grid_optimization_PHASED_PLAN.md --contract docs/contracts/sdf_field_primary_grid_optimization.contract.json`; token `ck:880bba43`. |
| Baseline witness | `artifacts/sdf_field_primary_grid_optimization/baseline_sdf_performance_witness.json`; recommendation `field_generation_or_downsample_candidate`. |
| Native proof | `artifacts/validation/sdf_field_primary_grid_native_build_after_pack_cache.json`; helper tests passed. |
| Runtime publish | `artifacts/validation/sdf_field_primary_grid_after_runtime_publish.json`; active runtime `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`. |
| Published runtime proof | `py -3.14 -m pytest tests/test_fractal_runtime_sdf_pack_scene_lane.py tests/test_fractal_runtime_explaino_root_sdf.py tests/test_fractal_runtime_sdf_performance_witness.py`; `9 passed`. |
| After witness | `artifacts/sdf_field_primary_grid_optimization/after_sdf_performance_witness.json`; before/after target rows in `artifacts/sdf_field_primary_grid_optimization/before_after_target_rows.json`. |
| Plan sync | `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`; OK. |
| Contract validation | `artifacts/validation/sdf_field_primary_grid_optimization_contract_after.json`; OK. |
| Code quality | `artifacts/validation/sdf_field_primary_grid_optimization_code_quality.json`; baseline check passed, CRITICAL=0, ERROR=0. |
| Diff check | `artifacts/sdf_field_primary_grid_optimization/diff_check.log`; clean. |
