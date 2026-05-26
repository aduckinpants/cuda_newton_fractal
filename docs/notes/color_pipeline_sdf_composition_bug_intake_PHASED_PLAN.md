# Color Pipeline SDF Composition Bug Intake

## Current Phase

Closed - user-reported SDF composition bugs are captured as the next bounded implementation slice

## Phase Checklist

- [x] Phase 1 - open a docs-only bug-intake plan/contract on a fresh branch
- [x] Phase 2 - capture the disabled-row compatibility bug and the SDF curvature/normal-angle blend regression as explicit next-slice REDs
- [x] Phase 3 - sync roadmap/known-issue surfaces without implementing the repair in this intake slice
- [x] Phase 4 - hostile audit, validation, checkpoint, receipts, rearward review, push, and clean-tree closeout

## Explicit User Asks

- [done] Include the disabled-row/draft-only error bug in the next slice.
- [done] Include the SDF curvature plus SDF normal-angle blend regression in the next slice.
- [done] Do not treat this as already fixed just because upcoming composition work may touch the area.
- [done] Keep this as next-slice intake, not an unplanned implementation detour.

## Bug Reports Captured For Next Slice

### Bug 1 - Disabled Row Still Participates In Draft-Only Error

Observed symptom:

- Clicking a Color Pipeline row `Enabled` checkbox off does not appear to affect the `Selected Source / Shape / Palette recipe is draft-only...` error.
- The row remains visually disabled, but compatibility/effective-selection reporting still behaves as if the disabled row is active.

Required next-slice RED:

- Build a no-mouse state with an unsupported Source / Shape / Palette combination.
- Disable the offending row through the visible `Enabled` checkbox automation.
- Prove the effective active selection, error reporting, and frame evaluation ignore disabled rows.
- Prove re-enabling the row restores the expected compatibility/error behavior.

Expected repair boundary:

- Disabled rows may remain in the authored row stack as editable dormant state.
- Disabled rows must not participate in active runtime evaluation, effective-source summary, compatibility gating, or draft-only error text.
- Preserve existing row ordering, row remove, row enable/disable automation, diagnostics state persistence, and capture/replay behavior.

### Bug 2 - SDF Curvature No Longer Blends With SDF Normal Angle

Observed symptom:

- A Source stack containing `sdf_normal_angle` and `sdf_curvature` is now blocked or reported as draft-only, even though this blend worked previously.
- The screenshot shows an SDF-only Source stack with normal-angle rows plus curvature. The runtime SDF postprocess should be able to blend SDF-only source rows; the UI compatibility/reporting layer appears to reject or misclassify the composition.

Required next-slice RED:

- Build a no-mouse state with `sdf_normal_angle` and `sdf_curvature` enabled in the same Source stack.
- Prove the composition is accepted as runtime-backed, not draft-only.
- Prove changing the curvature row blend weight changes the rendered frame.
- Prove disabling the curvature row removes its contribution without removing the row from authored state.
- Prove capture/replay preserves the mixed SDF Source stack.

Expected repair boundary:

- SDF-only Source stacks remain fail-closed against mixed SDF/non-SDF Source rows.
- `sdf_normal_angle` remains phase-like, `sdf_curvature` remains scalar, and the stack is still valid because the SDF postprocess blends source signals before the selected shape/palette/grading path.
- Do not broaden into per-source SDF downsample, GPU postprocess, authored SDF pack UI, SDF-native lanes, or full composition UI redesign.

## Scope

In scope for this intake:

- Record the two user-reported bugs as next-slice implementation requirements.
- Add/update roadmap and known-issue text so future sessions do not miss the regressions.
- Preserve the current clean `master` implementation; no product code changes in this intake slice.

Out of scope for this intake:

- Implementing the row-enable compatibility repair.
- Implementing the SDF curvature/normal-angle blend repair.
- Running runtime UI proof for the unfixed bugs.
- Changing Color Pipeline compatibility metadata.
- Changing SDF postprocess, capture, or pacing behavior.

## Proof Ledger

- Start authority: `master` at `9e379d8`, clean, rearward review `status=ok`.
- User report: disabled Color Pipeline rows still appear to affect the selected-shape draft-only error.
- User report: SDF curvature can no longer blend with SDF normal angle after recent boundary-band/gate work, despite working the prior night.
- This intake deliberately stops at documentation/planning authority; product behavior remains unproven until the next implementation slice adds RED runtime/native tests.
- Contract validation GREEN: `color_pipeline_sdf_composition_bug_intake_contract` passed.
- Plan sync GREEN: `viewer_host_assert_phased_plan_sync.py` passed.
- Hostile-audit validation GREEN: `color_pipeline_sdf_composition_bug_intake_hostile_audit` passed.
- Code-quality baseline GREEN: `color_pipeline_sdf_composition_bug_intake_code_quality` passed.
- Diff check GREEN: `color_pipeline_sdf_composition_bug_intake_diff_check` passed.

## Hostile Audit

- Status: complete
- Required posture: assume this intake is insufficient if it fails to name the user-visible symptoms, fails to define the next REDs, accidentally claims the bugs are fixed, or broadens the next implementation slice into unrelated SDF/performance/composition work.

## Audit Passes

- [done] Pass 1 - found the real continuity risk: both bugs were chat-only after the interrupted turn and would be easy for the next implementation slice to miss.
- [done] Pass 2 - clean re-read confirmed the plan names disabled-row active-state authority separately from the SDF-only source-stack blend regression.
- [done] Pass 3 - clean re-read confirmed the intake does not claim implementation, runtime proof, per-source downsample, GPU postprocess, authored SDF UI, or SDF-native lanes.

## Audit Findings

- [done] Real intake gap repaired: the disabled-row/draft-only error and SDF curvature/normal-angle blend regression are now explicit next-slice RED requirements.
- [done] Clean re-read confirmed no product fix is claimed in this docs-only intake.
- [done] Clean re-read confirmed the next implementation boundary stays narrowly on Color Pipeline row enable/effective compatibility and SDF-only source-stack acceptance.
