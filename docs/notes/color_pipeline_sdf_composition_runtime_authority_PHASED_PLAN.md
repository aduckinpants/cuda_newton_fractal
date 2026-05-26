# Color Pipeline SDF Composition Runtime Authority

## Current Phase

Closed - Color Pipeline SDF composition runtime authority repaired and validated

## Phase Checklist

- [x] Phase 1 - open the checked-in implementation plan/contract and lock the active slice
- [x] Phase 2 - add RED native/runtime coverage for disabled rows still affecting active compatibility and SDF curvature/normal-angle blends being rejected
- [x] Phase 3 - route active row-stack compatibility/effective selection through runtime-authoritative enabled-row filtering and metadata-backed SDF-only Source-stack acceptance
- [x] Phase 4 - prove row enable/disable, SDF-only source blending, capture/replay, preset workflow, and current SDF postprocess behavior stay green
- [x] Phase 5 - hostile audit, plan sync, receipts, rearward review, push, and clean-tree closeout

## Explicit User Asks

- [done] Move forward toward the new Color Pipeline backend/runtime authority path.
- [done] Repair the reported bug where disabling a row does not affect draft-only compatibility/error state.
- [done] Repair or prove the reported regression where `sdf_curvature` no longer blends with `sdf_normal_angle`.
- [done] Keep the work bounded; do not start the full composition UI redesign, per-source downsample, GPU postprocess, authored SDF UI, or SDF-native lanes.
- [done] Use no-mouse runtime proof for UI-workflow regressions.

## Scope

In scope:

- Make disabled Color Pipeline rows inactive for effective runtime selection, compatibility gating, selected-recipe/draft-only error text, and frame evaluation while preserving them as authored dormant rows.
- Ensure SDF-only Source stacks containing `sdf_normal_angle` and `sdf_curvature` are accepted as runtime-backed SDF postprocess compositions.
- Add focused native/window tests and no-mouse runtime tests for the two reported regressions.
- Preserve existing row enable/disable, remove, reorder, diagnostics state persistence, capture/replay, SDF Source controls, and preset workflow behavior.

Out of scope:

- Full Factorio-style composition UI redesign.
- Per-row/per-function SDF downsample authority.
- GPU Color Pipeline postprocess.
- Boundary-masked normal-angle beauty mode beyond existing source-local gate behavior.
- Authored SDF pack UI/live viewport integration.
- SDF-native selectable fractal lanes.
- New Color Pipeline functions.
- Physical mouse automation.

## Proof Ledger

- Start authority: `codex/color-pipeline-sdf-composition-runtime-authority` branched from clean `master` at `83ae029`.
- Prior rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `83ae029`.
- RED evidence: `artifacts/logs/color_pipeline_window_red.log` failed on normal-angle plus curvature SDF Source-stack apply before the compatibility authority fix.
- Runtime finding: `artifacts/logs/runtime_color_pipeline_sdf_composition.log` exposed that base-owned SDF Source-stack captures could not be loaded by the viewer because diagnostics state IO still required final-row flat-signal mirroring.
- Native validation: `artifacts/logs/color_pipeline_sdf_composition_native_final.log` passed `test_color_pipeline_core`, `test_color_pipeline_window`, `test_color_pipeline_sdf_postprocess`, and `test_diagnostics_state_io`.
- Runtime validation: `artifacts/logs/runtime_color_pipeline_sdf_composition_final.log` passed the published no-mouse SDF rows and capture/replay lane, 7 passed.
- Runtime publish: `artifacts/logs/publish_runtime_after_sdf_state_load.log` built and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Hygiene validation: `artifacts/validation/color_pipeline_sdf_composition_code_quality_final.json`, `artifacts/validation/color_pipeline_sdf_composition_diff_check_final.json`, `artifacts/validation/color_pipeline_sdf_composition_contract.json`, `artifacts/validation/color_pipeline_sdf_composition_hostile_audit.json`, and plan sync are green.
- User bug report: disabling a row does not clear or alter the selected-shape draft-only error.
- User bug report: `sdf_curvature` can no longer blend with `sdf_normal_angle` despite being an SDF-only Source-stack composition.

## Hostile Audit

- Status: complete
- Required posture: assume the first implementation only hides the error text, ignores disabled rows in one UI path but not runtime, accidentally drops dormant authored rows, allows mixed SDF/non-SDF stacks, changes SDF postprocess pixels, or relies on helper-only proof until native and published-runtime evidence disprove each risk.

## Audit Passes

- [done] Pass 1 - re-read active row filtering and compatibility seams; found the compatibility owner mismatch where all-SDF Source stacks were judged by the last enabled row even though SDF runtime evaluates from the first row.
- [done] Pass 2 - reran runtime proof after the first fix; found diagnostics state IO still rejected base-owned SDF Source-stack captures because it required final-row flat-signal mirroring.
- [done] Pass 3 - challenged row-disable proof; added an in-process no-mouse Source-row enabled-checkbox assertion and reran the focused runtime lane clean.

## Audit Findings

- [done] Finding 1: `sdf_normal_angle` plus `sdf_curvature` was rejected because draft compatibility used the last enabled SDF Source row as palette authority. Repaired `TryBuildColorPipelineSelectionFromDraft()` to use the first enabled SDF Source row for all-SDF stacks and added native/runtime regressions.
- [done] Finding 2: base-owned SDF Source-stack captures could not be reloaded because diagnostics state IO required the final source row to mirror `color_signal`. Repaired loader authority to accept first-row or final-row mirrors for all-SDF stacks while preserving final-row legacy semantics for non-SDF stacks.
- [done] Finding 3: Source-row disable was initially only native-covered. Added a published-runtime in-process click proof that disables the `sdf_curvature` row and leaves validation messages empty.
- [clean] Clean re-read found no mutation outside Color Pipeline SDF composition/runtime authority, diagnostics state load, focused tests, plan/contract, and handoff surfaces.
