# SDF Measurement Replan 2026-06-11

## Current Phase

Complete - compact SDF witness has run, the measured result is mixed/inconclusive, and no implementation option is selected under this slice.

## Phase Checklist

- [x] Phase 0 - merge deferred research notes into `master`, push, and verify rearward review `ok`.
- [x] Phase 1 - create fresh branch `codex/sdf-measurement-replan-20260611` from pushed `master`.
- [x] Phase 2 - create and lock this measurement-only plan/contract.
- [x] Phase 3 - publish the current runtime from this head.
- [x] Phase 4 - rerun compact SDF performance witness against the published runtime.
- [x] Phase 5 - stop and record measured options without changing SDF behavior.
- [x] Phase 6 - validate docs/contract, hostile audit, checkpoint, receipts, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [closed] Run through the agreed SDF restart path.
- [closed] Stop at the measured step-5 decision point and consider options.
- [closed] Do not start implementation before the measurement result is known.

## Scope

In scope:

- Merge the previously closed deferred-doc branch into `master`.
- Open a fresh SDF measurement/replan branch from `master`.
- Publish the current runtime.
- Rerun the compact SDF performance witness.
- Record the observed recommendation and next viable options.

Out of scope:

- SDF code changes.
- New SDF ops.
- New Color Pipeline functions or UI layout work.
- Per-row downsample changes.
- Authored pack catalog/authoring UX.
- Runtime behavior changes or debounce/pacing policy changes.
- Physical mouse automation.

## Current Repo Truth

- `master` has been fast-forwarded to `53c766e` with the deferred research notes.
- The last SDF engine hardening sprint is already merged into `master`.
- The last SDF field-generation optimization branch is already merged into `master`.
- The current SDF roadmap says field-generation/downsample optimization is the next recommended product step, but the latest field-generation witness noted `postprocess_optimization_candidate`.
- This slice exists to refresh that evidence before any new SDF mutation.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on clean `codex/deferred-research-intake-20260610` at `53c766e`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported clean tree.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `ok` for `53c766e`.
- Merge: `master` fast-forwarded to `53c766e` and pushed to `origin/master`.
- Branch: `codex/sdf-measurement-replan-20260611` created from pushed `master`.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "SDF measurement replan before next implementation" --profile runtime --plan docs/notes/sdf_measurement_replan_20260611_PHASED_PLAN.md --contract docs/contracts/sdf_measurement_replan_20260611.contract.json` appended `ck:c1801db3` and locked the active contract.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_measurement_replan_20260611.contract.json --out-json artifacts/validation/sdf_measurement_replan_20260611_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_measurement_replan_runtime_publish --log artifacts/logs/sdf_measurement_replan_runtime_publish.log --out-json artifacts/validation/sdf_measurement_replan_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 1200 -- ui_app/build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Compact witness: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_measurement_replan_witness --log artifacts/logs/sdf_measurement_replan_witness.log --out-json artifacts/validation/sdf_measurement_replan_witness.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_sdf_performance_witness.py --runtime-exe D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe --out-json artifacts/sdf_measurement_replan_20260611/sdf_performance_witness.json --out-md artifacts/sdf_measurement_replan_20260611/sdf_performance_witness.md --work-dir artifacts/sdf_measurement_replan_20260611/work --width 640 --height 480 --include-preview-sample` passed.
- Witness recommendation: `mixed_or_inconclusive_measurement_review_required`.
- Key witness rows:
  - `sdf_signed_distance_fullres`: mixed/inconclusive, field `2.212 ms`, postprocess `1.739 ms`, SDF total `3.951 ms`, full frame `4.686 ms`.
  - `sdf_pack_scene_signed_distance`: field-generation pressure, field `1.525 ms`, postprocess `0.728 ms`, SDF total `2.253 ms`, full frame `2.288 ms`.
  - cached `sdf_normal_angle`, `sdf_curvature`, and normal-angle-plus-curvature rows: low SDF cost, SDF totals `0.682 ms` to `0.788 ms`.
  - `lens_field_v2_fullres`: low SDF cost in this compact run, field `1.037 ms`, postprocess `0.725 ms`, SDF total `1.763 ms`.
- Hostile-audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/sdf_measurement_replan_20260611_PHASED_PLAN.md --out-json artifacts/validation/sdf_measurement_replan_20260611_hostile_audit.json` passed.
- Code-quality baseline: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/sdf_measurement_replan_20260611_code_quality.json` passed.
- Diff check: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_measurement_replan_20260611_diff_check --log artifacts/logs/sdf_measurement_replan_20260611_diff_check.log --out-json artifacts/validation/sdf_measurement_replan_20260611_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.

## Hostile Audit

- Status: complete

Questions:

- Did this slice actually stop at measurement instead of mutating SDF behavior?
- Did it publish the current runtime before running the witness?
- Did it record the witness recommendation without overstating performance or readiness?
- Did it keep known unsupported mixed Source lanes fail-closed?
- Did it avoid new SDF ops, UI changes, and physical mouse automation?
- Did closeout leave a clean branch with receipts and rearward review?

## Audit Passes

- [closed] Pass 1 - scope audit before runtime publish confirmed this branch is measurement-only and product files remain untouched.
- [closed] Pass 2 - witness result audit found the refreshed compact matrix does not support a single implementation choice; field generation dominates `sdf_pack_scene`, while full-res signed distance is split between field and postprocess.
- [closed] Pass 3 - clean re-read before checkpoint found the decision options now explicitly require another targeted matrix or a bounded choice before any SDF mutation.

## Audit Findings

- [closed] Finding 1 - The roadmap still suggested field-generation/downsample as the next likely step, but the refreshed witness is mixed/inconclusive. Repaired this plan by recording no implementation selection under this slice and requiring the next step to choose between targeted measurement, field/downsample/cache, or postprocess based on the concrete rows.

## Decision Options To Record At Step 5

The refreshed compact witness is mixed/inconclusive, not a clean field-generation or postprocess verdict.

Options to consider next:

1. Run a targeted SDF measurement matrix before implementation.
   - Add repeated samples per scenario instead of single samples.
   - Include 1024 and 2048 long-edge equivalents, not only 640x480.
   - Separate Lens SDF, Lens Field v2, and `sdf_pack_scene` rows.
   - This is the safest next move if we want no guessing.
2. Plan a bounded `sdf_pack_scene` field-generation/cache slice.
   - Justification: `sdf_pack_scene_signed_distance` is the clearest field-generation pressure row.
   - Risk: it may not improve Lens SDF/Lens Field v2 or general Color Pipeline SDF rows.
3. Plan a bounded full-res signed-distance split-seam slice.
   - Justification: `sdf_signed_distance_fullres` spends meaningful time in both field generation and postprocess.
   - Risk: this needs careful scope so it does not become a broad postprocess rewrite.
4. Defer optimization and re-rank feature work.
   - Justification: most cached normal/curvature paths are low cost at 640x480.
   - Risk: this ignores larger viewport and high-cost user scenarios.

Recommended decision from this slice: run a short targeted repeated-sample matrix next before choosing implementation. The compact witness changed the answer from "field-generation is obviously next" to "field generation matters for `sdf_pack_scene`, but full-res signed distance is split and cached phase/curvature paths are currently low."
