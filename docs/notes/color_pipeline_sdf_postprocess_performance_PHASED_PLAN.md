# Color Pipeline SDF Postprocess Performance

## Current Phase

Phase 5 - hostile audit, plan sync, receipts, rearward review, push, and clean-tree closeout

## Phase Checklist

- [x] Phase 1 - open the checked-in plan/contract and lock the active slice
- [x] Phase 2 - add RED measurement/proof coverage for SDF postprocess cost and current optimization gap
- [x] Phase 3 - implement the narrowest measured SDF postprocess optimization that preserves full-quality capture/render output
- [x] Phase 4 - prove SDF source rows, capture/replay, viewport overlay, pacing telemetry, and Color Pipeline behavior stay green
- [ ] Phase 5 - hostile audit, plan sync, receipts, rearward review, push, and clean-tree closeout

## Explicit User Asks

- [done] Merge the completed Color Pipeline function-library taxonomy branch to `master`.
- [done] Start the next SDF optimization slice from the merged head.
- [done] Measure before changing SDF performance behavior.
- [done] Keep realtime exploration responsive without degrading full-quality capture/render output.
- [done] Do not reopen authored SDF pack UI, SDF-native fractal lanes, per-source downsample authority, or broader Color Pipeline redesign in this slice unless measurement proves one tiny bounded seam is strictly required.

## Scope

In scope:

- Add or extend focused no-mouse runtime measurement for SDF Color Pipeline source-stack performance in one persistent viewer process.
- Record base render timing, SDF field timing, SDF postprocess timing, total visible-frame timing, field dimensions, shared field downsample, preview postprocess pixel step, frame hash, and active source-stack metadata.
- Cover representative rows: non-SDF baseline, `sdf_signed_distance`, `sdf_boundary_band`, `sdf_normal_angle`, `sdf_curvature`, and one mixed SDF stack if cheap enough for the focused lane.
- Use the measurement to select and implement only the narrowest SDF postprocess optimization that preserves full-quality output.
- Preserve existing Color Pipeline source rows, SDF source customization, phase-signal metadata, capture/replay authority, viewport overlay, pacing telemetry, and UI-Salt catalog behavior.

Out of scope:

- Per-source or multi-field SDF downsample authority as a product feature.
- GPU Color Pipeline postprocess implementation unless current measurement proves it is the only bounded safe next step, in which case this plan must be revised before mutation.
- Authored SDF pack UI or live viewport integration.
- SDF-native selectable fractal lanes.
- Factorio-style Color Pipeline composition UI redesign.
- New Color Pipeline function entries.
- Physical mouse automation.
- Capture-quality or full-quality render degradation.

## Proof Ledger

- Start authority: `codex/color-pipeline-sdf-postprocess-performance` branched from merged `master` at `bcf5e1f`.
- Prior rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `bcf5e1f`.
- Known carried limitation: `SDF Field Downsample` is still one shared `LensSettings::downsample`, not per SDF Source row/layer.
- Known shipped baseline: SDF realtime pacing telemetry, scalar postprocess specialization, preview postprocess quality policy, and scalar preview hardening are already closed; this slice must not rebreak them.
- RED: `color_pipeline_sdf_postprocess_performance_red_native` failed because a full-quality downsampled SDF field still reported one neighborhood sample per render pixel instead of one sample per field cell.
- GREEN native: `color_pipeline_sdf_postprocess_performance_native` passed `test_color_pipeline_sdf_postprocess` with 65 passed, proving the downsampled-field path fills every render pixel and matches field-resolution expansion references for even and uneven render sizes.
- Runtime publish GREEN: `color_pipeline_sdf_postprocess_performance_publish` staged the published runtime.
- Runtime SDF rows GREEN: `color_pipeline_sdf_postprocess_performance_runtime_sdf_rows` passed 5 no-mouse tests, including shared downsample sample-count proof for scalar and normal-angle SDF source rows.
- Runtime SDF pacing GREEN: `color_pipeline_sdf_postprocess_performance_runtime_sdf_pacing` passed the persistent no-mouse realtime pacing proof.
- Native pacing GREEN: `color_pipeline_sdf_postprocess_performance_pacing_native` passed `test_viewer_render_pacing`.
- Combined runtime preservation GREEN: `color_pipeline_sdf_postprocess_performance_runtime` passed 11 no-mouse tests across SDF rows, SDF realtime pacing, resolution pacing, and capture/replay authority.

## Hostile Audit

- Status: complete
- Required posture: assume the first optimization changes pixels, hides SDF cost instead of reducing it, regresses capture/replay, breaks phase source behavior, weakens pacing telemetry, or claims responsiveness from helper-only proof until focused native/runtime evidence disproves each risk.

## Audit Passes

- [done] Pass 1 - found and repaired runtime test mis-targeting: the first heavy-source edit accidentally changed the Capture Finding parity seed instead of the Lens Downsample heavy-source proof, then asserted neighborhood samples for a signed-distance row.
- [done] Pass 2 - found and repaired native coverage weakness: the first pixel-equivalence proof only covered an even 2x mapping, so an uneven render-size expansion case was added and rerun.
- [done] Pass 3 - clean re-read of the repaired state found no additional real defect after the runtime-targeting fix, stale-doc repair, uneven-size native proof hardening, and focused native/runtime validations.

## Audit Findings

- [done] Real runtime-test targeting defect repaired: `test_lens_downsample_visible_and_authoritative_for_sdf_source_without_lens_visualization_no_mouse` now seeds `sdf_normal_angle` before asserting neighborhood sample reuse, while Capture Finding parity stays on the cheaper signed-distance row.
- [done] Real status-doc stale authority repaired: touched status docs no longer claim their last reconciliation was the older preset-workflow branch/head.
- [done] Real native proof gap repaired: the downsampled-field pixel-equivalence test now covers uneven render dimensions in addition to the even 2x case.
- [done] Clean re-read confirmed the repaired state preserves full-quality pixel expansion, Capture Finding parity, shared downsample authority, preview pixel-step policy, and the deferred status of per-source downsample/GPU postprocess.

## Notes

- Default decision path is measurement first, then a bounded CPU-side optimization if the measured seam is obvious.
- Per-row/multi-field downsample and GPU Color Pipeline postprocess remain larger follow-up designs unless the evidence forces a contract revision.
