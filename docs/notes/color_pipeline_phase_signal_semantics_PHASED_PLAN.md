# Color Pipeline Phase Signal Semantics

## Current Phase

Phase 6 complete - phase-signal metadata and the compact no-mouse diagnostic matrix are implemented and validated.

## Phase Checklist

- [x] Phase 1 - create this checked-in plan/contract and open the slice
- [x] Phase 2 - add RED native/runtime proof that SDF normal angle is currently an unclassified circular signal
- [x] Phase 3 - add Color Pipeline source signal-kind metadata and mark `sdf_normal_angle` as phase-like
- [x] Phase 4 - add a compact diagnostic matrix proving normal-angle seam behavior is phase/wrap behavior, not a fractal kernel or signed-distance bug
- [x] Phase 5 - publish runtime and prove the phase-signal metadata/diagnostic path without physical mouse automation
- [x] Phase 6 - hostile audit, preservation rails, receipts, rearward review, push, and clean-tree closeout

## Explicit User Asks

- [done] Pause broader SDF feature work until replay authority and the phase-signal follow-up are handled.
- [done] Treat `sdf_normal_angle` as circular/phase-like data instead of ordinary scalar data.
- [done] Add a diagnostic matrix proving branch-cut-like planes are expected/movable for `sdf_normal_angle`.
- [done] Prove those planes do not imply the same artifact class for signed distance, boundary band, normal smooth/root coloring, or non-SDF modes.
- [done] Do not redesign the whole Color Pipeline, add authored SDF UI, or add SDF-native fractal lanes in this slice.

## Scope

In scope:

- Add source signal-kind metadata with `scalar`, `phase`, and `categorical` classifications.
- Classify `sdf_normal_angle` as `phase`.
- Keep signed distance, boundary band, curvature, and normal smooth/root coloring as non-phase scalar signals.
- Keep inside/outside and root index as categorical signals.
- Add a compact diagnostic proof that normal-angle phase offset changes the SDF normal-angle frame while scalar SDF sources remain separate non-phase rows.
- Preserve Capture Finding replay authority, SDF Source rows, viewport overlay, and existing Color Pipeline behavior.

Out of scope:

- Phase-safe shape transforms.
- Vector normal source rows.
- SDF-native selectable fractal lanes.
- Authored SDF pack UI/live viewport integration.
- Broader Color Pipeline redesign.
- Physical mouse automation.

## Proof Ledger

- Start authority: current branch `codex/color-pipeline-sdf-source-rows` at `16e1df5` was clean and rearward review returned `status=ok`.
- Slice opened with checkpoint token `ck:74203b73` under `global_active_contract`.
- RED proof: `color_pipeline_phase_signal_red_core` failed because `ColorPipelineSourceSignalKind`, `ColorPipelineSourceSignalKindForSignal`, `ColorPipelineSourceSignalKindForFunctionId`, and `ColorPipelineSourceFunctionIsPhaseSignal` did not exist.
- Native source-catalog proof passed: `color_pipeline_phase_signal_test_core` reported `test_color_pipeline_core: passed=175 failed=0`.
- Native SDF diagnostic proof passed: `color_pipeline_phase_signal_test_sdf_postprocess` reported `test_color_pipeline_sdf_postprocess: passed=26 failed=0`, including normal-angle phase-bias frame sensitivity and scalar SDF classification checks.
- Runtime publish passed: `color_pipeline_phase_signal_runtime_publish` staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published no-mouse runtime proof passed: `color_pipeline_phase_signal_runtime_pytest` reported `7 passed`, covering the new phase matrix plus capture/replay authority, SDF Source rows, and viewport overlay preservation.
- Code-quality baseline passed with score `94/100`.
- Contract validation, phased-plan sync, hostile-audit validation, and diff hygiene are required before checkpoint.

## Hostile Audit

- Status: complete
- Required posture: assume the metadata is ornamental and unused, `sdf_normal_angle` is still treated as scalar authority, the diagnostic matrix cannot distinguish phase-wrap behavior from a capture/replay or kernel bug, or the slice quietly widens into phase-safe transform redesign until focused proof disproves each risk.

## Audit Passes

- [done] Pass 1 - re-read source catalog and ColorSignal mapping to ensure every source signal is classified deliberately.
- [done] Pass 2 - re-read SDF postprocess signal extraction to ensure normal-angle behavior remains unchanged except for metadata/reporting.
- [done] Pass 3 - re-read runtime matrix outputs to ensure the proof uses small no-mouse captures and does not rely on physical cursor automation.
- [done] Pass 4 - run preservation rails for Capture Finding replay, SDF Source rows, and viewport overlay.

## Audit Findings

- [done] Real product/proof gap found and repaired: the Source catalog could expose `sdf_normal_angle` as a row while carrying no durable metadata that it is circular phase data rather than an ordinary scalar SDF source.
- [done] Real diagnostic gap found and repaired: the existing SDF postprocess test only proved each SDF source changed pixels; it did not prove normal-angle phase offset changed the frame or that signed distance and boundary band remained scalar classifications.
- [done] Real runtime proof gap found and repaired: the existing runtime SDF tests covered row visibility, replay, and overlay preservation, but had no compact matrix that exercised normal-angle phase behavior beside scalar SDF and non-SDF captures.
- [done] Workflow mistake found and corrected: the first native validation attempt parallelized two build-script targets and hit a shared object-directory collision. Heavy native build/test rails were rerun serially and should stay serial for this repo.
- [done] Clean re-read: no phase-safe shape lanes, vector normal source rows, SDF-native lanes, authored SDF UI/live viewport integration, renderer replacement, or physical mouse automation was added.

## Action Hostile Review

- Action ID: color-pipeline-phase-signal-semantics-closeout
- Suspected failure mode: `sdf_normal_angle` can generate expected phase-wrap seams, but the source catalog has no durable metadata to distinguish that from scalar SDF sources or future phase-safe UX work.
- Correct owner/action: source catalog metadata, ColorSignal classification helpers, SDF postprocess diagnostics, and small no-mouse runtime matrix must move together.
- Proof surface: focused Color Pipeline native rails, SDF postprocess diagnostic proof, runtime publish, no-mouse published-runtime matrix, replay/SDF rows/overlay preservation, contract validation, phased-plan sync, hostile-audit validation, code-quality baseline, and diff hygiene.
- Blocked action: phase-safe shape lanes, vector normal source rows, SDF-native lanes, authored SDF UI/live viewport integration, renderer replacement, or physical mouse automation.
