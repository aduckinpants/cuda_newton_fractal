# SDF Targeted Repeated Measurement Matrix 2026-06-11

## Current Phase

Phase 6 - measurement matrix and harness repair are complete; hostile audit is recorded, and the slice is in checkpoint validation/receipt closeout.

## Phase Checklist

- [x] Phase 0 - inspect the existing SDF witness tool and confirm repeated sampling support exists.
- [x] Phase 1 - create and lock this measurement-only plan/contract.
- [x] Phase 2 - publish the current runtime from this head.
- [x] Phase 3 - run repeated no-mouse SDF witness at 1024x768.
- [x] Phase 4 - run repeated no-mouse SDF witness at 2048x1536.
- [x] Phase 5 - classify the repeated rows and choose the next implementation seam, or explicitly stop as still inconclusive.
- [ ] Phase 6 - hostile audit, validation, receipts, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [active] Continue from the prior SDF measurement replan.
- [active] Run a targeted repeated-sample matrix before implementation.
- [active] Do not guess that field generation is the next optimization seam from the compact witness alone.
- [active] Keep this as measurement and decision work unless the evidence is strong enough to justify the next bounded implementation slice.

## Scope

In scope:

- Replace the closed active callable-API documentation contract with this SDF measurement contract.
- Publish the current viewer runtime.
- Run repeated samples through the existing no-mouse persistent runtime SDF witness.
- Repair a narrow witness-tool bug if it blocks the matrix while leaving product behavior unchanged.
- Capture both 1024x768 and 2048x1536 rows.
- Preserve Lens SDF, Lens Field v2, `sdf_pack_scene`, direct signed distance, normal angle, curvature, and normal-angle-plus-curvature rows.
- Record whether field generation, postprocess, cache behavior, or mixed pressure is the next measured seam.

Out of scope:

- SDF runtime behavior changes.
- Color Pipeline behavior changes.
- New SDF ops or pack content.
- UI changes.
- Per-row downsample changes.
- GPU postprocess or field-generation implementation work.
- Physical mouse automation.

## Current Repo Truth

- Branch: `codex/sdf-measurement-replan-20260611`.
- Starting head for this slice: `4bd434a`.
- Worktree was clean before this plan was created.
- Rearward review for `4bd434a` returned `ok`.
- The active repo contract before this slice was the already-closed `callable_engine_api_compatibility_followups`; it must be replaced before this SDF measurement work mutates further.
- The prior compact SDF witness was mixed/inconclusive and explicitly recommended a targeted repeated matrix before implementation.

## Measurement Design

Use the existing `tools/viewer_host_sdf_performance_witness.py` repeated-sample path rather than adding new harness code unless a concrete tool gap is found.

Rows to measure through the tool's current scenario set:

- `plain_smooth_escape`
- `sdf_signed_distance_fullres`
- `sdf_normal_angle_fullres`
- `sdf_curvature_fullres`
- `sdf_normal_angle_curvature_stack`
- `sdf_signed_distance_downsample4`
- `lens_field_v2_fullres`
- `sdf_pack_scene_signed_distance`

Matrix:

- 1024x768, `--repeat-count 3`, include preview sample.
- 2048x1536, `--repeat-count 3`, include preview sample.

Decision rule:

- If one seam dominates repeated medians across both sizes and relevant rows, plan that seam next.
- If `sdf_pack_scene` is field-heavy but Lens/Lens Field rows are not, do not call the whole SDF engine field-generation-bound.
- If signed distance remains split, treat it as a split-seam candidate rather than a broad rewrite.
- If timing remains mixed/noisy, stop with an explicit inconclusive result and avoid implementation.

## Proof Ledger

- Bootstrap: pending.
- Repo status: pending.
- Rearward review: pending for this slice after lock.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "SDF targeted repeated measurement matrix before optimization" --profile runtime --plan docs/notes/sdf_targeted_repeated_measurement_matrix_20260611_PHASED_PLAN.md --contract docs/contracts/sdf_targeted_repeated_measurement_matrix_20260611.contract.json` appended `ck:57466de9` and locked the active contract.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_targeted_repeated_measurement_matrix_20260611.contract.json --out-json artifacts/validation/sdf_targeted_repeated_measurement_matrix_20260611_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_repeated_matrix_runtime_publish --log artifacts/logs/sdf_repeated_matrix_runtime_publish.log --out-json artifacts/validation/sdf_repeated_matrix_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 1200 -- ui_app/build_vsdevcmd.cmd` passed.
- Witness-tool regression: `py -3.14 -m pytest tests/test_sdf_performance_witness_tool.py` passed after the viewport-clamped settle predicate repair.
- 1024 repeated witness: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_repeated_matrix_1024 --log artifacts/logs/sdf_repeated_matrix_1024.log --out-json artifacts/validation/sdf_repeated_matrix_1024.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_sdf_performance_witness.py --runtime-exe D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe --out-json artifacts/sdf_targeted_repeated_measurement_matrix_20260611/sdf_performance_witness_1024.json --out-md artifacts/sdf_targeted_repeated_measurement_matrix_20260611/sdf_performance_witness_1024.md --work-dir artifacts/sdf_targeted_repeated_measurement_matrix_20260611/work_1024 --width 1024 --height 768 --repeat-count 3 --include-preview-sample` passed.
- 2048 repeated witness: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_repeated_matrix_2048 --log artifacts/logs/sdf_repeated_matrix_2048.log --out-json artifacts/validation/sdf_repeated_matrix_2048.json --heartbeat-seconds 30 --timeout-seconds 1800 -- py -3.14 tools/viewer_host_sdf_performance_witness.py --runtime-exe D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe --out-json artifacts/sdf_targeted_repeated_measurement_matrix_20260611/sdf_performance_witness_2048.json --out-md artifacts/sdf_targeted_repeated_measurement_matrix_20260611/sdf_performance_witness_2048.md --work-dir artifacts/sdf_targeted_repeated_measurement_matrix_20260611/work_2048 --width 2048 --height 1536 --repeat-count 3 --include-preview-sample` passed.
- Code-quality baseline: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/sdf_targeted_repeated_measurement_matrix_20260611_code_quality.json` passed.
- Diff check: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_targeted_repeated_measurement_matrix_20260611_diff_check --log artifacts/logs/sdf_targeted_repeated_measurement_matrix_20260611_diff_check.log --out-json artifacts/validation/sdf_targeted_repeated_measurement_matrix_20260611_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.
- Hostile audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/sdf_targeted_repeated_measurement_matrix_20260611_PHASED_PLAN.md --out-json artifacts/validation/sdf_targeted_repeated_measurement_matrix_20260611_hostile_audit.json` passed.

## Hostile Audit

- Status: complete

Questions:

- Did this slice actually replace the stale callable-API active contract before measurement mutation?
- Did it avoid product/runtime SDF behavior changes?
- Did it use repeated samples instead of a single-row witness?
- Did it measure large enough sizes to challenge the compact 640x480 conclusion?
- Did it keep Lens SDF, Lens Field v2, and `sdf_pack_scene` distinct instead of generalizing across them?
- Did it avoid claiming a performance fix when no optimization was implemented?
- Did it leave a clear next seam or an honest inconclusive stop?

## Audit Passes

- [closed] Pass 1 - pre-measurement scope audit found the 2048 preview-settle predicate incorrectly required actual rendered frame dimensions to reach requested render settings, even when the live viewer reports full-quality pacing at the available viewport size.
- [closed] Pass 2 - repeated-witness result audit rejected broad field-generation wording; cached Lens/Lens Field rows are postprocess-heavy, while `sdf_pack_scene` and cache-miss settle are still field-heavy.
- [closed] Pass 3 - closeout re-read found the plan now states this as a split-seam decision and does not claim an optimization was implemented.
- [closed] Pass 4 - clean re-read after the witness predicate repair, unit regression, 1024/2048 reruns, contract validation, plan sync, code-quality baseline, and diff check found no additional harness or measurement-scope defect.

## Audit Findings

- [closed] Finding 1 - The witness tool treated `rendered_frame_width < target_render_width` as not settled, but the viewer can be full-quality at an actual viewport smaller than the requested render setting. Repaired with a focused predicate test and reran both matrix rows with the repaired tool.

## Measurement Result

Top-level witness recommendations:

- 1024x768 repeated matrix: `postprocess_optimization_candidate`.
- 2048x1536 repeated matrix: `postprocess_optimization_candidate`.

Important row medians:

| Size | Row | Class | Field ms | Post ms | Total SDF ms | Last render ms | Notes |
|---|---|---|---:|---:|---:|---:|---|
| 1024 | `sdf_signed_distance_fullres` | postprocess | 0.125 | 1.398 | 1.523 | 2.913 | cached field, direct scalar postprocess |
| 1024 | `sdf_normal_angle_fullres` | postprocess | 0.128 | 1.392 | 1.508 | 2.892 | cached field signal |
| 1024 | `lens_field_v2_fullres` | postprocess | 0.108 | 3.157 | 3.245 | 4.445 | cached direct scalar |
| 1024 | `sdf_pack_scene_signed_distance` | field generation | 2.521 | 1.139 | 3.633 | 3.720 | pack field producer, cache disabled |
| 2048 | `sdf_signed_distance_fullres` | postprocess | 0.382 | 11.150 | 12.783 | 17.204 | cached field, direct scalar postprocess |
| 2048 | `sdf_normal_angle_fullres` | postprocess | 0.393 | 4.712 | 5.134 | 9.859 | cached field signal |
| 2048 | `lens_field_v2_fullres` | postprocess | 0.389 | 4.926 | 5.315 | 10.389 | cached direct scalar |
| 2048 | `sdf_pack_scene_signed_distance` | field generation | 8.813 | 4.114 | 13.105 | 13.471 | pack field producer, cache disabled |
| 2048 | interaction preview | preview sample | 1.735 | 1.133 | 2.867 | 5.694 | actual rendered frame 1360x1020 |
| 2048 | settled after interaction | field generation | 5.476 | 2.577 | 8.053 | 11.895 | cache miss at actual rendered frame 1779x1334 |

Decision:

- The measured next seam is not broad field-generation work for every SDF path.
- Cached Lens SDF, Lens Field v2, signed distance, normal angle, curvature, and normal-angle-plus-curvature rows are postprocess-heavy at both measured sizes.
- `sdf_pack_scene` remains field-generation-heavy and should stay a separate pack-field producer/cache optimization lane.
- Cache-miss interaction settle can still become field-heavy, so any postprocess work must not be sold as solving every interaction cost.
- The next implementation slice should be a bounded SDF postprocess/direct-scalar optimization or investigation for cached live Color Pipeline SDF rows, with an explicit guard that `sdf_pack_scene` field generation remains a separate follow-up.
