# SDF Postprocess Signal Specialization

## Current Phase

Closed - roadmap sync, RED proof, scalar direct-sample implementation, shared boundary-band helper, runtime publish, no-mouse runtime proof, hostile audit, and validator proof are complete and represented in the checkpoint workflow for this slice.

## Phase Checklist

- [x] Phase 1 - create this checked-in plan/contract and open the slice
- [x] Phase 2 - sync roadmap priority so measured SDF postprocess performance comes before broader composition UX polish
- [x] Phase 3 - add RED native proof that scalar-only SDF Source rows still use the full neighborhood/normal/curvature sampling path
- [x] Phase 4 - implement direct-center sampling for scalar-only SDF rows while preserving derivative sampling for `sdf_normal_angle` and `sdf_curvature`
- [x] Phase 5 - publish runtime and prove scalar-only SDF postprocess remains live, measured, and no-mouse through the normal viewer path
- [x] Phase 6 - hostile audit, validation, checkpoint workflow, receipts, rearward review, push, and clean tree

## Explicit User Asks

- [done] Slot the newly measured SDF FPS bottleneck into the existing planning instead of leaving it as chat-only follow-up.
- [done] Prioritize realtime exploration and responsiveness over broader SDF feature growth for this step.
- [done] Improve the SDF Color Pipeline path with measured, bounded changes rather than guessing at debounce policy.
- [done] Keep per-function/per-row SDF downsample and GPU postprocess as planned follow-ups unless this slice proves they are strictly required.
- [done] Avoid physical mouse automation and repeated viewer open/close loops.

## Scope

In scope:

- Roadmap grooming that places measured SDF postprocess optimization ahead of broader composition/preset UX polish.
- Native postprocess classification/stats that prove scalar-only SDF rows can use direct center samples.
- CPU postprocess fast path for `sdf_signed_distance`, `sdf_inside_outside`, and `sdf_boundary_band` stacks that do not require normal/curvature neighborhoods.
- Preservation of full neighborhood sampling for `sdf_normal_angle` and `sdf_curvature`.
- Focused no-mouse runtime proof against the published viewer.

Out of scope:

- Per-function or per-row SDF downsample UI/authority.
- CUDA/GPU Color Pipeline postprocess rewrite.
- Boundary-masked normal-angle beauty mode.
- Preset manager or Factorio-style composition UI.
- SDF-native fractal lanes or authored SDF pack viewport integration.
- Capture-quality downgrade or physical mouse automation.

## Groomed Priority

Current ordering after the SDF realtime pacing telemetry slice:

1. Scalar-only SDF postprocess specialization - small/high reward. Common SDF Source rows such as signed distance and boundary band should not pay for normal angle or curvature derivatives.
2. Measured end-to-end responsiveness telemetry - medium/high reward if the client still feels stuck after this specialization.
3. Per-function/per-row SDF field quality policy - medium/high reward, but it needs a clear authority model so row-local downsample does not become a second hidden resolution system.
4. GPU Color Pipeline SDF postprocess - high reward/high risk, after CPU authority and tests prove exactly what must be ported.
5. Composition UX polish, boundary-masked normal-angle, masks/gates, authored SDF UI, and SDF-native lanes - still valuable, but behind the current measured FPS bottleneck.

## Proof Ledger

- Start authority: branch `codex/color-pipeline-sdf-source-rows` at `3c36a3f` is clean and rearward review returned `status=ok`.
- Prior measurement: the SDF realtime pacing witness reports base render as a small fraction of frame time while `lens_sdf_postprocess_ms` dominates.
- Code seam: `ApplyLensSdfColorPipelinePostprocess` currently resolves SDF rows through `SampleSdfFieldSignals`, whose implementation always samples neighbors and computes normal angle plus curvature.
- Product risk: common scalar SDF rows can remain too slow even after pacing reacts correctly, because they still pay derivative cost they do not use.
- RED target: scalar-only SDF stacks should report direct center samples and zero neighborhood samples; normal-angle/curvature stacks should still report neighborhood samples.
- RED proof: `sdf_postprocess_specialization_red_native` failed because `SdfColorPipelinePostprocessStats` did not exist.
- Native proof: `sdf_postprocess_specialization_native` passed; scalar-only SDF stacks report 16 direct samples and zero neighborhood samples on a 4x4 frame, while `sdf_normal_angle` reports 16 neighborhood samples.
- Runtime target: published viewer reports SDF postprocess timing for scalar SDF rows through one persistent no-mouse runtime lane.
- Runtime publish proof: `sdf_postprocess_specialization_runtime_publish` passed and staged the published viewer.
- Runtime proof: `sdf_postprocess_specialization_runtime_pytest` passed; one persistent viewer starts from a scalar signed-distance SDF stack, then loads the heavier derivative stack and proves interaction preview/settle behavior.
- Hostile-review repair: the boundary-band calculation is now shared through `ResolveSdfBoundaryBandFromSignedDistancePx(...)` so direct and neighborhood paths cannot drift.

## Hostile Audit

- Status: complete
- Required posture: assume the first optimization lies by changing test-only counters, breaks boundary-band width behavior, changes phase/curvature output, or hides a real need for per-row downsample/GPU postprocess.

## Audit Passes

- [done] Pass 1 - re-read scalar/direct sampling to prove it uses the same center signed-distance values as the old full sample path.
- [done] Pass 2 - re-read boundary-band width behavior and mixed scalar stack blending for parity.
- [done] Pass 3 - re-read normal-angle/curvature rows to prove they still use neighborhood sampling.
- [done] Pass 4 - re-read runtime proof to ensure one persistent viewer path is used and no physical mouse automation is introduced.
- [done] Pass 5 - verify per-row downsample, GPU postprocess, preset UI, boundary-masked normal angle, SDF-native lanes, and authored SDF live integration stayed deferred.

## Audit Findings

- [done] Real measured-priority gap repaired: SDF postprocess specialization is now slotted ahead of broader composition UX polish in the active SDF roadmaps.
- [done] Real native proof gap repaired: postprocess tests now prove scalar-only SDF stacks use direct center samples and normal-angle rows still use neighborhood samples.
- [done] Real drift risk repaired: direct and full SDF sampling now share the same boundary-band helper instead of duplicating the formula.
- [done] Real runtime-test setup bug repaired: the scalar SDF runtime proof now keeps the legacy flat `color_signal` consistent with the one-row source stack before loading the derivative-heavy stack in the same viewer process.
- [done] Clean re-read confirmed per-row downsample, GPU postprocess rewrite, preset manager UI, boundary-masked normal-angle, SDF-native lanes, and authored SDF live integration stayed deferred.

## Action Hostile Review

- Action ID: sdf-postprocess-signal-specialization-open
- Suspected failure mode: scalar SDF sources still pay for normal/curvature sampling, keeping SDF Color Pipeline frames slow even after pacing measures them correctly.
- Correct owner/action: specialize `ApplyLensSdfColorPipelinePostprocess` and focused postprocess tests; do not change debounce policy by feel.
- Proof surface: `test_color_pipeline_sdf_postprocess`, runtime publish, focused no-mouse SDF runtime proof, contract validation, phased-plan sync, hostile-audit validation, code-quality baseline, diff hygiene, rearward review, and clean tree.
- Blocked action: per-row downsample, GPU postprocess rewrite, preset manager UI, SDF-native lanes, authored SDF live viewport integration, physical mouse automation, or capture-quality downgrade.
