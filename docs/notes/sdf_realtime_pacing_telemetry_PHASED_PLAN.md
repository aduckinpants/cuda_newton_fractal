# SDF Realtime Pacing Telemetry

## Current Phase

Closed - SDF-aware timing/reporting, pacing, and local postprocess optimizations are implemented, validated, hostile-audited, and represented in the checkpoint workflow for this slice.

## Phase Checklist

- [x] Phase 1 - create this checked-in plan/contract and open the slice
- [x] Phase 2 - add RED native/runtime proof that SDF field/postprocess cost is missing from realtime pacing telemetry
- [x] Phase 3 - measure SDF field generation and SDF postprocess cost in the live viewer report
- [x] Phase 4 - make pacing use visible-frame cost so SDF-heavy interaction can enter preview instead of bypassing recovery
- [x] Phase 5 - publish runtime and prove no-mouse SDF interaction enters preview, reduces rendered dimensions, reports SDF cost, and still settles full quality
- [x] Phase 6 - hostile audit, validation, checkpoint workflow, receipts, rearward review, push, and clean tree

## Explicit User Asks

- [done] Restore realtime exploration priority: SDF Color Pipeline options must not make camera drag and slider edits ignore the lower-resolution recovery system.
- [done] Measure before guessing; expose enough telemetry to explain whether the base render, SDF field generation, or SDF postprocess is the bottleneck.
- [done] Keep high-quality/full-quality rendering as the settle path; this slice is about interaction responsiveness, not capture-quality downgrades.
- [done] Keep future per-function/per-row SDF downsample as a follow-up unless this slice proves it is strictly required.
- [done] Do not use physical mouse automation or repeated viewer open/close loops.

## Scope

In scope:

- Live viewer timing/report fields for SDF field generation and SDF Color Pipeline postprocess.
- Pacing authority that uses the visible live-frame cost, including SDF overhead, for subsequent interaction preview decisions.
- Focused no-mouse persistent runtime proof using SDF Color Pipeline rows.
- Native report/pacing rails that keep the telemetry contract parseable and stable.

Out of scope:

- Per-function/per-row SDF field downsample implementation.
- GPU Color Pipeline postprocess rewrite.
- Boundary-masked normal-angle source implementation.
- Full preset manager or Factorio-style composition UI.
- SDF-native fractal lanes or authored SDF pack viewport integration.
- Capture-quality downscaling or physical mouse automation.

## Proof Ledger

- Start authority: branch `codex/color-pipeline-sdf-source-rows` at `269cf4b` is clean and rearward review returned `status=ok`.
- Observed code seam: `DispatchRenderFrame` computes Lens SDF and applies SDF Color Pipeline postprocess after `RenderFractalCUDA` reports `RenderStats`.
- Suspected failure: pacing uses `stats.last_render_ms`, so SDF field/postprocess cost can be invisible to `AdvanceViewerRenderPacing`.
- RED target: no-mouse runtime SDF interaction should expose nonzero SDF timing fields and enter preview when the visible frame is slow.
- Preservation target: non-SDF f32 interaction remains full resolution when measured frames are near budget; SDF capture/replay and SDF rows remain green.
- RED proof: `sdf_pacing_red_report_native` failed because `ViewerUiAutomationLensSdfProbe` had no SDF timing fields.
- RED proof: `sdf_pacing_red_runtime_pytest` failed because published runtime reports had `lens_sdf_valid` but no `lens_sdf_color_pipeline_active` / SDF timing payload.
- Native proof: `sdf_pacing_report_native`, `sdf_pacing_native`, and `sdf_pacing_postprocess_native` passed on the repaired head.
- Runtime publish proof: `sdf_pacing_runtime_publish` passed and staged the published viewer.
- Runtime proof: `sdf_pacing_runtime_pytest` passed; one persistent no-mouse viewer proves SDF timings are reported, interaction enters preview, rendered dimensions drop, Lens SDF dimensions follow the preview frame, and the settle frame returns to full quality.
- Preservation proof: `sdf_pacing_existing_resolution_runtime` passed after the final publish, preserving the existing pacing rail.
- Preservation proof: `sdf_pacing_existing_sdf_rows_runtime`, `sdf_pacing_lens_backend_runtime` with `VIEWER_HOST_ENABLE_RUNTIME_VIEWER_E2E=1`, and `sdf_pacing_overlay_runtime` passed after the final publish.
- Optimization proof: the hostile audit found repeated SDF signal sampling per Source row and unnecessary Lens SDF debug texture build/upload for Color-Pipeline-only SDF rows; both were repaired with focused native/runtime preservation.
- Measurement note: the SDF pacing witness reports base render as a small fraction of frame time while `lens_sdf_postprocess_ms` dominates, confirming that further gains should target SDF postprocess or row/field quality policy rather than the fractal kernel.

## Hostile Audit

- Status: complete
- Required posture: assume the first repair only changes report text, lies about timing, over-triggers preview for fast f32 paths, or hides full-quality settle regressions until no-mouse runtime proof disproves it.

## Audit Passes

- [done] Pass 1 - re-read timing ownership after implementation and confirm `last_render_ms` used by pacing includes SDF field/postprocess overhead only for live frames that paid that cost.
- [done] Pass 2 - re-read SDF postprocess and Lens SDF paths for capture-quality or replay regressions.
- [done] Pass 3 - re-read runtime tests to ensure one persistent viewer process proves interaction responsiveness without OS mouse automation.
- [done] Pass 4 - check that per-row downsample, preset UI, SDF-native lanes, and broader Color Pipeline redesign stayed out of this slice.
- [done] Pass 5 - re-run final published-runtime preservation rails sequentially after the texture-skip optimization.

## Audit Findings

- [done] Real pacing bug repaired: SDF field/postprocess cost happened after base render stats, so pacing could ignore slow SDF frames.
- [done] Real telemetry gap repaired: no-mouse runtime reports now expose base render time, SDF field time, SDF postprocess time, SDF total time, and SDF Color Pipeline active state.
- [done] Real hot-path issue repaired: SDF Source stacks no longer recompute the same SDF neighborhood sample once per row unless a boundary-band row uses a distinct boundary width.
- [done] Real unnecessary work repaired: Color-Pipeline-only SDF rows no longer build/upload the Lens SDF debug RGBA texture when Lens visualization and viewport overlay are both off.
- [done] Real workflow mistake found: running two runtime pytest lanes in parallel raced on the shared diagnostics bundle; final runtime preservation rails were rerun sequentially.
- [done] Clean re-read confirmed per-function/per-row SDF downsample, GPU postprocess rewrite, preset manager UI, boundary-masked normal-angle, SDF-native lanes, and authored SDF live viewport integration stayed deferred.

## Action Hostile Review

- Action ID: sdf-realtime-pacing-telemetry-open
- Suspected failure mode: SDF work happens after the base render timing and therefore bypasses the realtime pacing decision.
- Correct owner/action: live viewer dispatch and automation report timing, with focused pacing/runtime proof.
- Proof surface: `test_viewer_ui_automation_report`, `test_viewer_render_pacing`, focused SDF Color Pipeline/postprocess rails, runtime publish, no-mouse SDF pacing pytest, contract validation, phased-plan sync, hostile-audit validation, code-quality baseline, diff hygiene, rearward review, and clean tree.
- Blocked action: per-function SDF downsample, GPU postprocess rewrite, preset manager UI, SDF-native lanes, authored SDF live viewport integration, physical mouse automation, or capture-quality downgrade.
