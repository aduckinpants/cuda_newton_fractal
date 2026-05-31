# Color Pipeline Post-Mixed Source Replan

## Current Phase

Closed - roadmap truth sync, next-slice selection, and hostile audit are complete; product code changes were forbidden in this slice.

## Phase Checklist

- [x] Phase 1 - open this checked-in phased plan and contract on a stacked branch from the clean mixed-source head `3465487`.
- [x] Phase 2 - mark typed Source-row applicators and mixed SDF/non-SDF Source rows as shipped in roadmap/status surfaces.
- [x] Phase 3 - re-rank the remaining Color Pipeline composition follow-ups by dependency, difficulty, and reward.
- [x] Phase 4 - select the next bounded implementation slice without adding product code here.
- [x] Phase 5 - hostile audit, plan sync, contract validation, code-quality baseline, diff hygiene, checkpoint, receipts, rearward review, push, and clean tree.

## Explicit User Asks

- [x] Continue after the mixed Source-row slice looked good in testing.
- [x] Do not reopen the just-closed mixed Source-row implementation under stale roadmap text.
- [x] Keep docs truthful about shipped applicative glue, mixed Source-row authority, SDF pack scene/catalog seed, and remaining deferred work.
- [x] Choose the next practical Color Pipeline composition step from the current repo state.

## Scope

In scope:

- Documentation truth sync for current Color Pipeline and SDF composition status.
- Re-ranking remaining composition work after mixed Source rows shipped.
- Selecting the next bounded implementation slice.

Out of scope:

- Product code, tests, runtime behavior, schema, renderer, or UI layout mutation.
- New Color Pipeline functions, SDF ops, SDF-native lanes, authored pack authoring UX, or Salticid adapter work.
- Merging this stacked branch to `master` as part of this docs-only slice unless explicitly requested later.
- Physical mouse automation.

## Current Repo Truth

- `codex/color-pipeline-mixed-source-rows` closed at `3465487` and is pushed.
- Source-row applicator metadata/runtime modes `none`, `sdf_boundary_band`, `sdf_inside`, and `sdf_outside` are shipped.
- Mixed SDF plus non-SDF Source rows are shipped for supported renderer-backed lanes using scalar, row-ordered Source signal composition before Shape, Palette, and Grading.
- Pure SDF CUDA direct/field postprocess paths remain preserved.
- `generic_equation_pack` and `sdf_pack_scene` mixed Source rows intentionally fail closed until those lanes have renderer-backed non-SDF source-signal producers.
- The flat function library and row editor are now the limiting UX surface; adding more functions before improving picker/layout risks making the workflow harder to use.

## Remaining Work Ranking

1. Function picker/layout refinement - recommended next implementation.
   - Difficulty: medium.
   - Reward: high.
   - Why now: taxonomy metadata, compatibility metadata, companion suggestions, preset workflow truth, applicators, and mixed-source semantics are already in place. A grouped/searchable picker can improve the current growing library without changing source-stack math or adding new functions.
   - Boundary: preserve current row-stack semantics, function ids, control ids, recipes, state, and runtime pixels unless a test proves current UI behavior is already inconsistent.

2. More applicators/masks.
   - Difficulty: medium.
   - Reward: high, but depends on clearer picker/layout if new modes would otherwise disappear into the same flat controls.
   - Candidate examples: inverse boundary, signed field ramp masks, phase-aware masks, or row-local rule labels. These need a separate semantic contract.

3. Authored SDF pack catalog/authoring UX.
   - Difficulty: medium/high.
   - Reward: high.
   - Dependency: function picker/layout and existing pack catalog seed should remain stable first.

4. New SDF ops and built-in pack expansion.
   - Difficulty: medium/high.
   - Reward: high.
   - Dependency: pack catalog UX and performance proof; keep op additions curated and tested.

5. Additional SDF-native lanes.
   - Difficulty: high.
   - Reward: high.
   - Dependency: stable field producer/catalog UX, pack ops, and rendering/state contracts.

6. Salticid adapter/removal campaign.
   - Difficulty: high.
   - Reward: strategic.
   - Dependency: the viewer-local vertical must stay stable first; do not introduce a runtime Salticid dependency.

## Selected Next Slice

Next implementation should be a bounded `color_pipeline_function_picker_layout` slice:

- Use existing UI-Salt taxonomy and compatibility metadata.
- Keep the existing row editor and visible Color Pipeline structure.
- Add a grouped function picker surface for Source/Shape/Palette/Grading rows without changing row semantics.
- Preserve the current combo path or provide an equivalent no-mouse automation path so existing tests and workflows remain stable.
- Add native/window tests that prove the picker lists the same functions, grouped by taxonomy, and hides/labels unsupported choices fail-closed.
- Add published no-mouse proof that selecting representative Source, Shape, Palette, and Grading functions still changes the expected row function without OS mouse use.

## Proof Ledger

- Start authority: `codex/color-pipeline-post-mixed-source-replan` branched from clean pushed mixed-source head `3465487`; rearward review for `3465487` was `ok`.
- Bootstrap proof: `viewer_host_session_bootstrap.py --audit --tail-handoff 8` reported clean head and active prior contract `color_pipeline_mixed_source_rows`.
- Repo status proof: `viewer_host_repo_status.py` reported no staged, unstaged, or untracked files before this slice.
- Truth-sync proof: `_STATUS.md`, `DEFERRED_THREADS.md`, `KNOWN_ISSUES.md`, and `sdf_field_pack_near_term_TODO.md` now mark typed Source-row applicators and mixed Source rows as shipped.
- Replan proof: the selected next implementation slice is `color_pipeline_function_picker_layout`, bounded to picker/layout refinement with existing metadata and without new function/math semantics.

## Action Hostile Review

- Action ID: post_mixed_source_replan_truth_sync
- Suspected Failure Mode: stale roadmap text could cause the next session to reopen applicative glue or mixed Source-row support as if they were still missing.
- Correct Owner/Action: update roadmap/status surfaces to mark both shipped, then choose the next implementation based on current code and proof.
- Proof Surface: this checked-in plan/contract, roadmap diffs, phased-plan sync, hostile-audit validator, contract validator, code-quality baseline, and diff check.
- Blocked Action: product code mutation, new SDF ops, SDF-native lanes, broad UI redesign, Salticid adapter work, or physical mouse automation under this docs-only slice.

## Hostile Audit

- Status: complete
- Required posture: assume the replan hides stale text, overstates mixed-source support for unsupported lanes, chooses a next slice that is too broad, or accidentally permits product mutation. The audit found stale roadmap text and overclaim risk, both recorded below.

Required questions:

- Did the docs actually mark mixed Source rows as shipped?
- Did the docs avoid claiming `generic_equation_pack` or `sdf_pack_scene` mixed Source rows are supported?
- Did the replan choose a bounded next implementation slice instead of a broad Color Pipeline redesign?
- Did the slice avoid product code and test changes?
- Did the plan/contract validation and diff hygiene pass?

## Audit Passes

- [x] Pass 1 - roadmap/status stale-text audit found applicative glue still described as the next SDF/Color Pipeline composition seam after it had shipped.
- [x] Pass 2 - next-slice scope audit found the replan needed to state that `generic_equation_pack` and `sdf_pack_scene` mixed Source rows remain fail-closed until renderer-backed non-SDF signals exist.
- [x] Pass 3 - clean re-read confirmed the selected next slice is function picker/layout refinement, not a broad Color Pipeline redesign or new SDF/math feature.

## Audit Findings

- [x] Finding: stale roadmap/status text still named Color Pipeline applicative glue as the next SDF composition step even though applicators and mixed Source rows are now closed. Updated the status, deferred, known-issues, and SDF TODO surfaces.
- [x] Finding: the first replan text could be misread as mixed Source-row support for every lane. Tightened docs to keep `generic_equation_pack` and `sdf_pack_scene` fail-closed until renderer-backed non-SDF source signals exist.
- [x] Clean re-audit: no product code, tests, runtime behavior, schema, renderer, or UI layout files are in scope for this docs-only slice.

## Planned Validation Targets

- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/color_pipeline_post_mixed_source_replan.contract.json --out-json artifacts/validation/color_pipeline_post_mixed_source_replan_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py docs/notes/color_pipeline_post_mixed_source_replan_PHASED_PLAN.md`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/color_pipeline_post_mixed_source_replan_PHASED_PLAN.md --out-json artifacts/validation/color_pipeline_post_mixed_source_replan_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/color_pipeline_post_mixed_source_replan_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_post_mixed_source_replan_diff_check --log artifacts/logs/color_pipeline_post_mixed_source_replan_diff_check.log --out-json artifacts/validation/color_pipeline_post_mixed_source_replan_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`
