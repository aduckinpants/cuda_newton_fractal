# Color Pipeline Post-Merge Roadmap Truth Sync

## Current Phase

Complete - master is fast-forwarded to `08e62b6`, roadmap truth is synced, validators are green, and the repo is ready for the next SDF performance/design slice.

## Phase Checklist

- [x] Phase 0 - fast-forward the closed SDF composition runtime-authority repair branch into `master` and push it.
- [x] Phase 1 - re-read current SDF, Color Pipeline, UI-Salt, known-issues, and deferred-thread planning surfaces for stale open-regression text.
- [x] Phase 2 - update only the stale planning rows that still described the just-closed regressions as open.
- [x] Phase 3 - validate docs-only contract, plan sync, hostile audit, code-quality baseline, diff hygiene, checkpoint, receipts, rearward review, and clean-tree stop point.

## Explicit User Asks

- [done] Treat full Salticid backend/legacy-layer removal as a long-term deferred goal, not the immediate task.
- [done] Keep the immediate goal focused on a working full vertical slice foundation.
- [done] Merge the closed SDF composition repair back to `master` before starting new implementation work.
- [done] Update planning docs if the reorientation exposes stale roadmap text.
- [done] Record that the next product slice should return to the SDF performance/design choice before authored-pack UI, SDF-native lanes, or broad Color Pipeline redesign.

## Scope

In scope:
- `spec_intake/_STATUS.md` current ColorPipelineUiSaltMetadataAuthority status.
- `DEFERRED_THREADS.md` SDF active follow-up text.
- `KNOWN_ISSUES.md` Color Pipeline composition issue status.
- This checked-in docs-only plan/contract and final handoff entry.

Out of scope:
- Product code, tests, schema, renderer, runtime harness, Color Pipeline behavior, SDF postprocess behavior, or UI layout changes.
- Starting full Salticid backend cutover or removing the legacy C++ runtime/state layer.
- Per-row SDF downsample, GPU Color Pipeline postprocess, authored SDF pack UI, SDF-native lanes, or boundary-masked normal-angle implementation.
- Physical mouse automation or live viewer proof changes.

## Current Repo Truth Inputs

- Bootstrap on `codex/color-pipeline-sdf-composition-runtime-authority` reported clean head `08e62b6` with rearward review `ok`.
- `master` and `origin/master` were both at `83ae029`, and `master` was an ancestor of `08e62b6`.
- `git switch master && git merge --ff-only codex/color-pipeline-sdf-composition-runtime-authority && git push origin master` fast-forwarded and pushed `master` to `08e62b6`.
- `docs/notes/color_pipeline_sdf_composition_runtime_authority_PHASED_PLAN.md` records the disabled-row and `sdf_curvature` plus `sdf_normal_angle` regressions as repaired with native/runtime proof.
- `spec_intake/_STATUS.md`, `DEFERRED_THREADS.md`, and `KNOWN_ISSUES.md` still had open-regression wording after the merge and needed a bounded truth sync.

## Target Next Work Shape

The immediate next product slice should be selected from the SDF performance/design fork, not legacy-layer deletion:

1. Measure and reduce remaining SDF Color Pipeline cost without changing capture/replay truth.
2. Choose between per-row/multi-field SDF downsample authority and GPU Color Pipeline postprocess with measured client evidence.
3. Keep authored SDF pack UI, SDF-native lanes, boundary-masked normal-angle, and broader composition UI redesign deferred until the performance/design foundation is stable.
4. Keep full Salticid backend cutover and legacy-layer removal as a later campaign after the vertical slice is stable and covered by runtime/state/capture parity.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed before the merge and reported branch `codex/color-pipeline-sdf-composition-runtime-authority`, head `08e62b6`, clean tree.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported clean before merge and after merge.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `08e62b6`.
- Merge proof: `master` and `origin/master` were both `83ae029`; `master` was an ancestor of `08e62b6`; the branch fast-forwarded and pushed to `origin/master`.
- Documentation updated: `spec_intake/_STATUS.md`, `DEFERRED_THREADS.md`, and `KNOWN_ISSUES.md` now treat the SDF composition runtime-authority regressions as shipped and point the next product work at the SDF performance/design decision.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/color_pipeline_post_merge_roadmap_truth_sync.contract.json --out-json artifacts/validation/color_pipeline_post_merge_roadmap_truth_sync_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile-audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/color_pipeline_post_merge_roadmap_truth_sync_PHASED_PLAN.md --out-json artifacts/validation/color_pipeline_post_merge_roadmap_truth_sync_hostile_audit.json` passed.
- Code-quality baseline: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/color_pipeline_post_merge_roadmap_truth_sync_code_quality.json` passed with score 94/100 and baseline check passed.
- Diff hygiene: `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_post_merge_roadmap_truth_sync_diff_check --log artifacts/logs/color_pipeline_post_merge_roadmap_truth_sync_diff_check.log --out-json artifacts/validation/color_pipeline_post_merge_roadmap_truth_sync_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.

## Hostile Audit

- Status: complete
- Did I actually merge the closed repair to `master` before new work? Yes: `master` fast-forwarded from `83ae029` to `08e62b6` and pushed.
- Did I falsely claim full Salticid backend cutover is close? No: docs now keep full removal deferred and describe the immediate goal as vertical-slice stability.
- Did I leave repaired regressions listed as open? The stale rows in `_STATUS.md`, `DEFERRED_THREADS.md`, and `KNOWN_ISSUES.md` were updated.
- Did I start new product implementation under a docs-only reorientation? No product code/test/schema/runtime files are in scope.
- Did I preserve the next useful priority? Yes: the next immediate product decision is SDF performance/design, specifically per-row/multi-field downsample authority versus GPU Color Pipeline postprocess with measured evidence.

## Audit Passes

- [done] Pass 1 - stale-roadmap audit found `_STATUS.md` still described the disabled-row and SDF-only blend regressions as immediate open bugs.
- [done] Pass 2 - deferred-thread audit found the old active follow-up still pointed to already-closed composition repair instead of the next performance/design choice.
- [done] Pass 3 - known-issues audit found repaired regressions still listed under remaining review direction.
- [done] Pass 4 - validator-backed clean re-read after docs sync found no additional stale roadmap or scope issue.

## Audit Findings

- [done] Finding 1: `_STATUS.md` still treated the just-closed SDF composition runtime-authority bugs as open follow-ups; updated it to point at `08e62b6` and the remaining broader follow-ups.
- [done] Finding 2: `DEFERRED_THREADS.md` still said the next immediate composition repair should cover disabled rows and normal-angle plus curvature blending; updated it to mark that repair shipped and return next to SDF performance/design.
- [done] Finding 3: `KNOWN_ISSUES.md` still listed the two repaired regressions as remaining repair work; updated it to keep them as preservation expectations instead of open bugs.
- [done] Clean re-audit after validation: contract validation, plan sync, hostile-audit validation, code-quality baseline, and diff hygiene passed after the final plan update.

## Notes

- This slice is documentation and workflow truth only. No runtime publish or no-mouse viewer proof is required because no product behavior is changed here.
- The next implementation slice still needs its own branch, plan, contract, RED tests, runtime proof, hostile audit, receipts, push, and clean-tree closure.


