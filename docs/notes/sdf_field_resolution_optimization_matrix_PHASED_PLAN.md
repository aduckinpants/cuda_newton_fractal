# SDF Field Resolution Optimization Matrix

## Explicit User Asks

- Merge the completed ExplainO Root SDF generated N-root branch into `master`, push, and rearward-review the merged head.
- Open the next bounded SDF field-resolution optimization slice.
- Start with a repeated-sample performance matrix for Lens SDF, Lens Field v2, `sdf_pack_scene`, and `explaino_root_sdf`.
- Choose the first optimization seam from measured evidence, not from feel.
- Do not add new visual features until the measured seam is selected.

## Current Phase

Phase 4 - hostile review and closeout.

## Phase Checklist

- [x] Phase 0: merge generated N-root Root SDF branch into `master`, push, and rearward-review `master`.
- [x] Phase 0: create this checked-in plan and contract for measurement-first SDF field-resolution optimization.
- [x] Phase 1: publish the current runtime and run a repeated-sample SDF performance matrix.
- [x] Phase 2: classify the measured seam for Lens SDF, Lens Field v2, `sdf_pack_scene`, and `explaino_root_sdf`.
- [x] Phase 3: record the first bounded optimization target or explicitly stop as inconclusive.
- [ ] Phase 4: hostile audit, validation receipts, rearward review, push, and clean-tree close.

## Scope

In scope:

- Measurement-only SDF field-resolution optimization prework.
- Repeated-sample no-mouse runtime witness using the published viewer.
- Existing scenario coverage for mask-derived Lens SDF, Lens Field v2, authored-pack `sdf_pack_scene`, and `explaino_root_sdf`.
- Decision record for the first optimization seam.

Out of scope:

- New SDF visual features.
- New SDF ops, recursive/apollonian packs, or new SDF-native lanes.
- Per-source downsample UI changes unless selected after the measurement matrix.
- Color Pipeline graph UI or broad function-library redesign.
- Physical mouse automation.
- Claims of FPS improvement without before/after numbers.

## Phase 0 - Merge And Slice Open

### Intent

Move the completed N-root work into `master` and start this SDF optimization branch from a clean, rearward-reviewed head.

### Evidence

- `master` fast-forwarded from `78dbf8d` to `b4dd99e`.
- `origin/master` pushed at `b4dd99e`.
- Rearward review for `b4dd99e` returned `ok`.
- Branch `codex/sdf-field-resolution-optimization-matrix` was created from `b4dd99e`.

## Phase 1 - Repeated-Sample Matrix

### Intent

Use repeated samples in one persistent no-mouse viewer session to reduce timing noise before selecting an optimization seam.

### Required Matrix

- Lens SDF / mask-derived SDF rows.
- Lens Field v2 distance.
- `sdf_pack_scene`.
- `explaino_root_sdf`, including generated N-root rows now that they are merged.
- Include preview/settled rows where the existing witness supports them.

### Exit Criteria

- Runtime publish is current.
- Repeated-sample witness JSON and Markdown artifacts exist under `artifacts/sdf_field_resolution_optimization_matrix/`.
- The report includes enough timing split to classify field generation, mask downsample, backend generation, postprocess, cache behavior, and total SDF cost.

## Phase 2 - Seam Classification

### Intent

Decide from the matrix whether the first optimization seam should be field generation/downsample, cache reuse, CUDA JFA/backend, postprocess fallback, or an inconclusive/no-change stop.

### Classification Questions

- Does field generation dominate Lens SDF or field-primary lanes?
- Is mask downsample measurable enough to target?
- Is backend generation still material after CUDA JFA buffer reuse?
- Are `sdf_pack_scene` and `explaino_root_sdf` field-primary costs different enough to require separate seams?
- Are preview/settled paths preserving the intended quality split?

## Phase 3 - Decision Record

### Intent

Record one bounded next optimization target and the proof it needs, or stop without implementation if evidence is noisy/inconclusive.

Allowed decisions:

- `FIELD_DOWNSAMPLE_OR_GENERATION`
- `FIELD_CACHE_REUSE`
- `CUDA_BACKEND_OR_JFA`
- `POSTPROCESS_OR_FALLBACK`
- `INCONCLUSIVE_REMEASURE`

### Decision

Decision: `FIELD_DOWNSAMPLE_OR_GENERATION`, scoped first to field-primary direct-grid producers.

Evidence:

- The repeated witness recommendation is `field_generation_or_downsample_candidate`.
- `sdf_pack_scene_signed_distance` median field time is about `1.718 ms`, postprocess about `0.549 ms`, field fraction about `0.741`.
- `explaino_root_sdf_static` median field time is about `1.923 ms`, postprocess about `0.586 ms`, field fraction about `0.772`.
- `explaino_root_sdf_regular_n16_static` median field time is about `4.055 ms`, postprocess about `0.515 ms`, field fraction about `0.885`.
- `explaino_root_sdf_regular_n16_phase_sine` median field time is about `4.236 ms`, postprocess about `0.572 ms`, field fraction about `0.873`.

Important limitation:

- Mask-derived Lens SDF and Lens Field v2 full-quality rows mostly measured cache-hit behavior after the first cold sample. The raw samples show cold misses around `1.940 ms` for `sdf_signed_distance_fullres`, `0.866 ms` for `lens_field_v2_fullres`, and `1.294 ms` for the preview/settled SDF stack. Those should not be collapsed into the field-primary direct-grid decision.

Next bounded implementation target:

- Optimize or reduce field-primary direct-grid field generation/downsample cost for `sdf_pack_scene` and `explaino_root_sdf` first.
- Keep a separate later cache/cold-miss matrix for mask-derived Lens SDF if the next slice touches cache reuse or mask downsample.

## Hostile Audit

- Status: complete

Audit questions:

- Did I actually measure the current merged runtime rather than stale branch output?
- Did I cover Lens SDF, Lens Field v2, `sdf_pack_scene`, and `explaino_root_sdf`?
- Did I keep this slice measurement-first and avoid adding visual features?
- Did I avoid selecting a seam from one noisy sample?
- Did I record uncertainty if the matrix is inconclusive?

## Audit Passes

- [x] Pass 1 - reviewed the repeated matrix summary and found it was insufficient by itself because median cache-hit rows hid cold Lens SDF field-generation misses.
- [x] Pass 2 - re-read raw per-sample cache statuses and timing samples; confirmed field-primary direct-grid producers have persistent field-generation pressure while mask-derived Lens rows mostly become cache hits.
- [x] Pass 3 - clean re-read of the decision scope confirmed no visual feature, SDF op, or UI change was added under this measurement slice.

## Audit Findings

- [x] Finding 1: the one-line witness recommendation could be misread as a generic field-generation/downsample target across every SDF producer. Raw samples show mask-derived Lens SDF is mostly cache-hit after the first cold sample, while field-primary `sdf_pack_scene` and `explaino_root_sdf` pay direct grid field generation every sample. Repaired by scoping the decision to field-primary direct-grid producers and documenting mask-derived cold-cache work as a separate seam.

## Proof Ledger

| Item | Evidence |
| --- | --- |
| N-root branch merge | `git merge --ff-only codex/explaino-root-sdf-generated-n-root` fast-forwarded `master` to `b4dd99e`; `git push origin master` pushed it. |
| Merged-head rearward review | `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `b4dd99e`. |
| Active contract | `py -3.14 tools/viewer_host_begin_work_slice.py --intent "SDF field-resolution optimization repeated-sample matrix" --profile runtime --plan docs/notes/sdf_field_resolution_optimization_matrix_PHASED_PLAN.md --contract docs/contracts/sdf_field_resolution_optimization_matrix.contract.json` locked token `ck:2bd2aff9`. |
| Runtime publish | `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_resolution_matrix_runtime_publish --log artifacts/logs/sdf_field_resolution_matrix_runtime_publish.log --out-json artifacts/validation/sdf_field_resolution_matrix_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`. |
| Repeated matrix | `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_resolution_matrix_repeated_witness --log artifacts/logs/sdf_field_resolution_matrix_repeated_witness.log --out-json artifacts/validation/sdf_field_resolution_matrix_repeated_witness.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_sdf_performance_witness.py --runtime-exe D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe --out-json artifacts/sdf_field_resolution_optimization_matrix/repeated_sdf_performance_witness.json --out-md artifacts/sdf_field_resolution_optimization_matrix/repeated_sdf_performance_witness.md --work-dir artifacts/sdf_field_resolution_optimization_matrix/work --width 640 --height 480 --repeat-count 5 --include-preview-sample --timeout-seconds 120` passed. |
| Matrix coverage | The repeated matrix wrote `62` raw samples across `14` scenarios, including Lens SDF, Lens Field v2, `sdf_pack_scene`, `explaino_root_sdf`, generated `regular_ngon_v1` N=16 Root SDF, and preview/settled rows. |
| Seam decision | `FIELD_DOWNSAMPLE_OR_GENERATION` for field-primary direct-grid producers first; mask-derived Lens SDF cold-cache behavior is explicitly split out for a later seam. |
