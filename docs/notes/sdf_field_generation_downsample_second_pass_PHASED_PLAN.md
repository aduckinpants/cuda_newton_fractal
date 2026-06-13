# SDF Field-Generation Downsample Second-Pass Measurement

## Explicit User Asks

- Continue from the merged SDF field-primary descriptor optimization.
- Start the next SDF slice from updated `master`.
- Measure the remaining field-generation/downsample pressure before choosing another implementation seam.
- Preserve the built-in pause boundary: do not add new SDF ops, new UI features, per-source downsample redesign, broad Color Pipeline work, or new SDF-native lanes.
- Stop for replan if the measured next seam is ambiguous.

## Current Phase

Closed - stop for replan before product optimization.

## Phase Checklist

- [x] Phase 0: merge `codex/sdf-field-primary-grid-optimization` into `master`, push, and rearward-review merged `master`.
- [x] Phase 0: create this checked-in measurement plan and contract from clean `master` at `344de1d`.
- [x] Phase 1: publish runtime and capture second-pass repeated-sample field-generation/downsample witness.
- [x] Phase 2: classify the remaining bottleneck across producer kinds and downsample settings.
- [x] Phase 3: choose exactly one next implementation seam or declare ambiguity and stop for replan.
- [x] Phase 4: hostile audit, validation, receipts, rearward review, push, and clean-tree close.

## Scope

In scope:

- Measurement only until Phase 3.
- Field-generation/downsample timing for `sdf_pack_scene`, `explaino_root_sdf`, Lens Field v2, and mask-derived Lens SDF.
- Downsample rows for requested/effective `1x`, `2x`, `4x`, `8x`, and available row-local settings where the existing runtime supports them.
- Larger representative dimensions than the prior 640x480 witness, with at least 1024-long-edge coverage and optional 2048-long-edge coverage if runtime cost is acceptable.
- Hash preservation checks for repeated rows.
- Report clarity around field cache status, requested/effective downsample, producer kind, backend kind, postprocess backend, and quality mode.

Out of scope:

- New SDF operators.
- New visual features or presets.
- New SDF-native fractal lanes.
- Per-source downsample UX redesign.
- Broad Color Pipeline graph/editor work.
- Salticid adapter/removal.
- Perturbation zoom.
- Approximate pixels, LUT quantization, or visual simplification.
- Physical mouse automation.

## Phase 0 - Merge And Slice Open

### Evidence

- `codex/sdf-field-primary-grid-optimization` was fast-forward merged to `master`.
- `origin/master` was pushed to `344de1d`.
- Rearward review for merged `master` returned `ok`.
- Branch `codex/sdf-field-generation-downsample-second-pass` was created from `344de1d`.

## Phase 1 - Second-Pass Measurement Matrix

### Intent

Measure the remaining field-generation/downsample bottleneck after descriptor generation and pack descriptor caching have landed.

### Required Evidence

- Runtime publish command.
- Repeated-sample witness JSON and Markdown under `artifacts/sdf_field_generation_downsample_second_pass/`.
- At least one compact witness for fast iteration and one larger witness for user-relevant field-primary cost.
- Rows must include `sdf_pack_scene`, `explaino_root_sdf`, Lens Field v2, and mask-derived Lens SDF where supported by the existing witness tooling.

### Result

- Initial measurement found a real harness gap: the witness reported downsample fields but did not vary field-primary producer downsample rows.
- The witness now covers field-primary downsample rows for `sdf_pack_scene`, Root SDF N=16, and Lens Field v2.
- A second harness gap was found: preview/settle proof only covered a mask-derived SDF stack, not field-primary rows. The witness now includes preview/settle rows for `sdf_pack_scene` and Root SDF N=16.
- Corrected artifacts:
  - `artifacts/sdf_field_generation_downsample_second_pass/witness_1024.json`
  - `artifacts/sdf_field_generation_downsample_second_pass/witness_1024.md`
  - `artifacts/sdf_field_generation_downsample_second_pass/witness_2048.json`
  - `artifacts/sdf_field_generation_downsample_second_pass/witness_2048.md`

## Phase 2 - Bottleneck Classification

### Intent

Decide whether the next narrow implementation seam is:

- field cache/reuse keying;
- live-only effective downsample policy;
- per-producer direct grid specialization;
- CUDA/JFA buffer or launch overhead;
- postprocess still dominating in larger dimensions;
- or ambiguity requiring replan.

### Classification Rules

- If repeated field cache misses dominate with unchanged authority keys, choose cache/reuse inspection.
- If high-cost rows improve mostly with downsample and capture/replay remains full-quality, choose adaptive live downsample policy only after proving settled/capture paths stay requested quality.
- If `explaino_root_sdf` N-root rows dominate while `sdf_pack_scene` is acceptable, choose producer-specific root descriptor/grid specialization.
- If Lens Field v2 or mask-derived Lens SDF dominate instead, do not force pack-scene/root-SDF work into that problem.
- If timings are noisy, classify as `ambiguous` and stop for replan.

### Result

Classification artifact:

- `artifacts/sdf_field_generation_downsample_second_pass/classification_summary.md`
- `artifacts/sdf_field_generation_downsample_second_pass/classification_summary.json`

The corrected matrix shows a split bottleneck:

- 1024-class rows favor field-generation/downsample work.
- 2048-class full-quality rows still include broad postprocess pressure.
- Full-quality field-primary `sdf_pack_scene` and `explaino_root_sdf` at requested 1x remain field-generation heavy.
- Increasing requested downsample materially reduces field-primary generation cost.
- Interactive preview already raises effective downsample for expensive field-primary rows, while lower-cost 1024 `sdf_pack_scene` remains at requested quality.

## Phase 3 - Decision Gate

### Allowed Outcomes

1. Select one bounded implementation seam and open a follow-up implementation slice.
2. Declare the next seam ambiguous and stop with the strongest measurement artifacts.
3. Find a correctness/reporting bug in the measurement harness, repair that bug as the next bounded slice, then rerun the witness.

No implementation is allowed in this plan unless the measured next seam is unambiguous and tiny enough to repair inside this measurement slice without violating the scope.

### Result

Decision: stop for replan before product optimization.

Candidate next slices, in priority order:

1. Review and harden live-only adaptive field-primary effective downsample thresholds/reporting, preserving settled/capture requested quality.
2. Postprocess second-pass if the target is broad 2048 full-quality SDF stacks rather than interactive field-primary lanes.
3. Producer-specific Root SDF N-root direct-grid specialization after adaptive downsample is proven insufficient.

## Phase 4 - Close

### Exit Criteria

- Measurement artifacts are checked into the proof ledger or saved under `artifacts/`.
- Hostile audit records a real finding or enough clean re-read passes.
- Contract validation, plan sync, code-quality baseline, hostile-audit validation, and diff check pass.
- Receipts are written.
- Rearward review is `ok`.
- Branch is pushed with a clean tree.

## Hostile Audit

- Status: complete

Audit questions:

- Did I keep this measurement-first rather than sneaking in product mutation?
- Did I include all relevant current producer kinds?
- Did I measure downsample behavior instead of guessing?
- Did I preserve capture/replay and settled-quality boundaries?
- Did I stop for replan if the next seam was ambiguous?

## Audit Passes

- [x] Pass 1 - found missing field-primary downsample coverage in the witness matrix.
- [x] Pass 2 - found missing field-primary preview/settle coverage in the witness matrix.
- [x] Pass 3 - corrected classification artifact after one overbroad sentence about adaptive preview behavior.
- [x] Pass 4 - clean re-read of the repaired witness scenario list, focused unit coverage, and classification summary; no additional real defect found.

## Audit Findings

- [x] Finding 1: witness rows only varied downsample for a mask-derived SDF row, not field-primary `sdf_pack_scene` or Root SDF. Repaired in `tools/viewer_host_sdf_performance_witness.py` and covered by `tests/test_sdf_performance_witness_tool.py`.
- [x] Finding 2: preview/settle proof covered only `sdf_normal_angle_curvature_stack`. Repaired by adding field-primary preview rows for `sdf_pack_scene` and Root SDF N=16, with focused test coverage.
- [x] Finding 3: first generated classification wording implied field-primary preview never adapted. Corrected to distinguish lower-cost 1024 `sdf_pack_scene` from expensive field-primary preview rows that did adapt.

## Proof Ledger

| Item | Evidence |
| --- | --- |
| Previous optimization merge | `git merge --ff-only codex/sdf-field-primary-grid-optimization` fast-forwarded `master` to `344de1d`; `git push origin master` pushed it. |
| Merged-head rearward review | `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `344de1d`. |
| Slice lock | `py -3.14 tools/viewer_host_begin_work_slice.py --intent "SDF field-generation/downsample second-pass measurement" --profile runtime --plan docs/notes/sdf_field_generation_downsample_second_pass_PHASED_PLAN.md --contract docs/contracts/sdf_field_generation_downsample_second_pass.contract.json` returned `checkpoint_id=ck:09b30046`. |
| Runtime publish | `cmd /c ui_app\build_vsdevcmd.cmd > artifacts\logs\sdf_field_generation_downsample_second_pass_runtime_publish.log 2>&1` exited 0. |
| Witness 1024 | `py -3.14 tools/viewer_host_sdf_performance_witness.py --runtime-exe D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe --out-json artifacts/sdf_field_generation_downsample_second_pass/witness_1024.json --out-md artifacts/sdf_field_generation_downsample_second_pass/witness_1024.md --work-dir artifacts/sdf_field_generation_downsample_second_pass/witness_1024_work --width 1024 --height 768 --repeat-count 5 --include-preview-sample --timeout-seconds 180` exited 0. |
| Witness 2048 | `py -3.14 tools/viewer_host_sdf_performance_witness.py --runtime-exe D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe --out-json artifacts/sdf_field_generation_downsample_second_pass/witness_2048.json --out-md artifacts/sdf_field_generation_downsample_second_pass/witness_2048.md --work-dir artifacts/sdf_field_generation_downsample_second_pass/witness_2048_work --width 2048 --height 1536 --repeat-count 3 --include-preview-sample --timeout-seconds 240` exited 0. |
| Witness tool unit | `py -3.14 -m pytest tests/test_sdf_performance_witness_tool.py` passed `8 passed`. |
| Classification | `artifacts/sdf_field_generation_downsample_second_pass/classification_summary.md` records `stop_for_replan_split_bottleneck`. |
| Published-runtime witness proof | `py -3.14 -m pytest tests/test_fractal_runtime_sdf_performance_witness.py` passed `1 passed`. |
| Contract validation | `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_field_generation_downsample_second_pass.contract.json --out-json artifacts/validation/sdf_field_generation_downsample_second_pass_contract.json` returned `ok=true`. |
| Plan sync | `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` returned OK. |
| Hostile audit validation | `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/sdf_field_generation_downsample_second_pass_PHASED_PLAN.md --out-json artifacts/validation/sdf_field_generation_downsample_second_pass_hostile_audit.json` returned `ok=true`. |
| Code quality baseline | `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/sdf_field_generation_downsample_second_pass_code_quality.json` passed with CRITICAL=0 and ERROR=0. |
| Diff check | `git diff --check > artifacts/sdf_field_generation_downsample_second_pass/diff_check.log 2>&1` exited 0. |
| Final focused proof | `py -3.14 -m pytest tests/test_sdf_performance_witness_tool.py tests/test_fractal_runtime_sdf_performance_witness.py` passed `9 passed`. |
