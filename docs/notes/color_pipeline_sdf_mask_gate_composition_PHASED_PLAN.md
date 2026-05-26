# Color Pipeline SDF Mask/Gate Composition

## Current Phase

Closed - checkpoint, receipts, rearward review, push, and clean tree are the remaining mechanical repository steps outside the committed plan text

## Phase Checklist

- [x] Phase 1 - open the checked-in plan/contract and lock the active slice
- [x] Phase 2 - record the shared SDF downsample limitation as deferred, not active implementation scope
- [x] Phase 3 - add RED coverage for a source-local SDF boundary gate on `sdf_normal_angle`
- [x] Phase 4 - implement the narrow runtime/metadata/UI/state glue for the source-local SDF gate
- [x] Phase 5 - prove boundary-masked `sdf_normal_angle` changes pixels, preserves full-field diagnostic behavior, and keeps SDF/preset/capture rails green
- [x] Phase 6 - hostile audit, plan sync, receipts, rearward review, commit, push, and clean tree

## Explicit User Asks

- [done] Return to the planned composition-glue work after confirming the SDF normal-angle pixelation was user-selected `2x` downsample, not a regression.
- [done] Record the real low-priority limitation: `SDF Field Downsample` is currently one shared authority, not per SDF Source row/layer.
- [done] Treat the next step as compositional glue first, not a broad dump of new function entries.
- [done] Keep the future agent-skill idea deferred until one real vertical proves the repeatable pattern.

## Scope

In scope:

- Add one source-local SDF boundary gate/mask contract to the Color Pipeline Source row model.
- Make `sdf_normal_angle` the first visible proof case: full-field diagnostic remains available when the gate is off, and boundary-gated normal angle suppresses off-boundary full-field planes when the gate is on.
- Persist/load the new source-row gate params through state/capture/replay.
- Use no-mouse runtime proof that the gate control is visible and materially changes pixels through the normal viewer path.
- Keep the shared `LensSettings::downsample` authority as-is and document per-source downsample as deferred.

Out of scope:

- Per-row or per-function SDF downsample implementation.
- Multi-field SDF generation.
- GPU Color Pipeline postprocess.
- Full Factorio-style composition UI.
- New broad function-library expansion.
- Authored SDF pack UI/live viewport integration.
- SDF-native fractal lanes.
- Physical mouse automation.

## Proof Ledger

- Start authority: `master` at `846809a`, clean, rearward review `ok`.
- User-error finding: SDF Normal Angle Diagnostic pixelation was caused by `2x` SDF field downsample; `1x (Full)` restores expected quality.
- Deferred limitation to record: current SDF Source rows all share `LensSettings::downsample`; there is no per-layer field quality authority yet.
- RED coverage: `color_pipeline_sdf_mask_gate_red` failed as expected on missing `ColorPipelineSourceRuntimeParams::sdf_gate`, `ColorPipelineSdfGateMode`, and `sdf_gate_width_px`.
- First GREEN: `test_color_pipeline_sdf_postprocess` passed in `color_pipeline_sdf_mask_gate_native_1`.
- Focused native GREEN:
  - `test_color_pipeline_core`: `color_pipeline_sdf_mask_gate_core_2`
  - `test_color_pipeline_window`: `color_pipeline_sdf_mask_gate_window_2`
  - `test_color_pipeline_sdf_postprocess`: `color_pipeline_sdf_mask_gate_audit_postprocess`
  - `test_diagnostics_state_io`: `color_pipeline_sdf_mask_gate_diagnostics`
  - `test_diagnostics_capture`: `color_pipeline_sdf_mask_gate_diagnostics_capture`
  - `tests/test_ui_salt_materializer.py`: `color_pipeline_sdf_mask_gate_ui_salt_pytest`
- Runtime publish: `color_pipeline_sdf_mask_gate_runtime_publish` staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Runtime no-mouse proof: `color_pipeline_sdf_mask_gate_runtime_pytest` passed `tests/test_fractal_runtime_color_pipeline_sdf_rows.py` and `tests/test_fractal_runtime_capture_replay_authority.py` with 6 passed.
- Viewport overlay preservation: `color_pipeline_sdf_mask_gate_viewport_overlay_pytest` passed.
- Contract validation: `artifacts/validation/color_pipeline_sdf_mask_gate_contract.json` has `checks.contract_schema_valid=true`.
- Hostile-audit validation: `artifacts/validation/color_pipeline_sdf_mask_gate_hostile_audit.json` has `ok=true`.
- Plan sync: `color_pipeline_sdf_mask_gate_plan_sync_2` passed.
- Code-quality audit: `color_pipeline_sdf_mask_gate_code_quality_baseline` passed and wrote `artifacts/validation/color_pipeline_sdf_mask_gate_code_quality.json`.
- Diff check: `color_pipeline_sdf_mask_gate_diff_check` passed.
- Hostile audit: found and repaired a proof gap where gate width was not explicitly proven inert when gate mode is `none`; added `TestNormalAngleGateWidthIsInactiveWhenGateIsOff`, then re-read the repaired state through the focused postprocess rail.

## Hostile Audit

- Status: complete
- Required posture: assume this accidentally adds only a one-off function, breaks full-field diagnostic normal-angle, mixes SDF/non-SDF stacks incorrectly, silently changes capture replay, or hides the shared-downsample limitation until focused proof disproves each risk.

## Audit Passes

- [done] Pass 1 - source-local gate behavior is generic runtime glue on all SDF Source rows through `ColorPipelineSourceRuntimeParams`, descriptor metadata, source-row apply/import, and SDF postprocess, not only a preset trick.
- [done] Pass 2 - re-read the repaired state: full-field `sdf_normal_angle` remains stable when gate mode is off; `TestNormalAngleGateWidthIsInactiveWhenGateIsOff` proves gate width is inert unless the gate is enabled.
- [done] Pass 3 - persisted state/capture/replay and no-mouse controls expose the gate truthfully; diagnostics state/capture and runtime SDF row pytest passed.

## Audit Findings

- [done] Initial hostile finding: the first implementation proved gated normal-angle changes pixels but did not prove the full-field diagnostic ignores gate width while gate mode is `none`. Added the missing native regression and reran `test_color_pipeline_sdf_postprocess`.

## Notes

- Expected first gate mode: `none` and `sdf_boundary_band`.
- Expected first visible control paths: `signal.sdf_gate` and `signal.sdf_gate_width_px` or equivalent source-local names.
- Expected implementation seam: `ColorPipelineSourceRuntimeParams`, `BuildColorPipelineSourceFunctions`, Source-row apply/import/state serialization, and `ApplyLensSdfColorPipelinePostprocess`.
