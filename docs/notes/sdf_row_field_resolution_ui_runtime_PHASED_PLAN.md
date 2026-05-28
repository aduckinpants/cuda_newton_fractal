# SDF Row Field Resolution UI Runtime

## Current Phase

Closed - Step 3C row-local SDF field downsample UI/runtime productization has completed its validation and hostile-review record; repo checkpoint surfaces carry the post-commit receipt, rearward-review, push, and clean-tree proof.

## Phase Checklist

- [x] Phase 1 - create and lock the checked-in Step 3C plan and contract.
- [x] Phase 2 - add focused RED coverage for visible row-local SDF field downsample controls and no-mouse enum automation.
- [x] Phase 3 - expose row-local field downsample in UI-Salt/native descriptors for SDF Source rows only.
- [x] Phase 4 - bind draft/live import, apply, and automation so visible row-local edits become runtime authority.
- [x] Phase 5 - publish runtime and prove row-local controls, frame/hash effect, capture/replay, and preservation rails.
- [x] Phase 6 - hostile audit, validation, checkpoint, receipts, rearward review, push, and clean tree.

## Explicit User Asks

- [done] Continue the next todo work item after confirming the interrupted D: / Salticid build did not leave this viewer repo damaged.
- [done] Productize per-row SDF field downsample controls so SDF Source layers can choose field resolution instead of relying only on the shared SDF Field Downsample.
- [done] Keep the current good SDF performance foundation intact and provide numbers or explicit non-claims for hot-path changes.

## Scope

In scope:

- Step 3C only: visible UI/runtime productization for the row-local SDF field-resolution authority added in Step 3B.
- Add a row-local SDF Source param with the default policy `Inherit` and explicit values `1x`, `2x`, `4x`, `8x`, and `16x`.
- Show that row-local control only for SDF-backed Source rows.
- Keep the shared `SDF Field Downsample` as the default authority when the row-local value is `Inherit`.
- Make no-mouse automation able to set the row-local enum control.
- Preserve field grouping, cache/reporting, capture/replay, Lens Field v2, SDF rows, SDF pacing, and UI-Salt metadata parity.

Out of scope:

- Step 4 phase-safe normal-angle beauty UX.
- Broader Color Pipeline layout redesign.
- GPU multi-field postprocess.
- Authored SDF pack viewport integration.
- SDF-native fractal lanes.
- Global FPS improvement claims.

## Proof Ledger

- RED: `artifacts/validation/sdf_row_field_resolution_ui_runtime_red_core.json` and `artifacts/validation/sdf_row_field_resolution_ui_runtime_red_window.json` failed before implementation because SDF row descriptors and UI/runtime import did not expose visible row-local field downsample.
- Green: `artifacts/validation/sdf_row_field_resolution_ui_runtime_native_core.json` proves SDF descriptors expose `signal.sdf_field_downsample` with `Inherit`, `1x`, `2x`, `4x`, `8x`, and `16x`.
- Green: `artifacts/validation/sdf_row_field_resolution_ui_runtime_native_window.json` proves visible draft edits set `ColorPipelineSourceRuntimeParams::sdf_field_downsample`, live resync imports it, and the old hidden-preservation glue no longer overwrites visible edits.
- Green: `artifacts/validation/sdf_row_field_resolution_ui_runtime_native_report.json` proves the automation/report seam remains coherent.
- Green: `artifacts/validation/sdf_row_field_resolution_ui_runtime_ui_salt_tests.json` and `artifacts/validation/sdf_row_field_resolution_ui_runtime_materialize.json` prove UI-Salt metadata materializes with the new field.
- Green: `artifacts/validation/sdf_row_field_resolution_ui_runtime_contract.json` proves the active contract is valid.
- Green: `artifacts/validation/sdf_row_field_resolution_ui_runtime_publish.json` published the viewer runtime to `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Green: `artifacts/validation/sdf_row_field_resolution_ui_runtime_row_field.json` proves no-mouse visible row-local controls can drive distinct SDF field groups.
- Green: `artifacts/validation/sdf_row_field_resolution_ui_runtime_sdf_rows.json`, `artifacts/validation/sdf_row_field_resolution_ui_runtime_capture_replay.json`, `artifacts/validation/sdf_row_field_resolution_ui_runtime_pacing.json`, and `artifacts/validation/sdf_row_field_resolution_ui_runtime_performance_witness.json` prove SDF rows, capture/replay, pacing, and the existing performance witness stayed green.
- Green: `artifacts/validation/sdf_row_field_resolution_ui_runtime_plan_sync.json`, `artifacts/validation/sdf_row_field_resolution_ui_runtime_hostile_audit.json`, `artifacts/validation/sdf_row_field_resolution_ui_runtime_code_quality.json`, and `artifacts/validation/sdf_row_field_resolution_ui_runtime_diff_check.json` prove final plan sync, hostile audit, code-quality baseline, and diff hygiene.
- Closure machine receipts, rearward review, push, and clean-tree proof are handled by the repo checkpoint surfaces after the final commit.

## Hostile Audit

- Status: complete

Required questions:

- Did the slice actually expose row-local field downsample controls for every SDF Source row and only SDF Source rows?
- Did visible controls update `ColorPipelineSourceRuntimeParams::sdf_field_downsample` instead of being overwritten by hidden-preservation glue from Step 3B?
- Did `Inherit` preserve the shared `LensSettings::downsample` authority and old-state behavior?
- Did no-mouse automation exercise the same visible control path that the user sees?
- Did UI-Salt generated metadata stay in parity with native descriptors?
- Did SDF rows, Lens Field v2, capture/replay, and pacing remain green?
- Did this slice avoid Step 4 phase UX, authored SDF UI, SDF-native lanes, and broader Color Pipeline redesign?

## Audit Passes

- [done] Pass 1 - review descriptor/UI-Salt parity, renderability, and no-mouse automation control IDs.
- [done] Pass 2 - review draft/live apply, fractal-switch preservation, and hidden Step 3B policy preservation for conflicts with visible UI edits.
- [done] Pass 3 - clean re-read of the repaired state found no additional real defect in Step 3C scope.

## Audit Findings

- [done] Pass 1 found that the live import path built the new enum id with a temporary `std::to_string(...).c_str()` expression. It was functionally likely to copy safely, but the bridge now stores the id in a local `std::string` before passing it to `SetColorPipelineParamEnum`.
- [done] Pass 2 confirmed the old Step 3B hidden-preservation override was incompatible with a now-visible row-local field control; the override is removed, and `VisibleRowFieldDownsampleWinsOnEdit` locks that visible edits win.
- [done] Clean re-read confirmed the repaired state and no additional workflow mistake found: SDF-only descriptors and UI-Salt metadata expose the same row-local enum, visible no-mouse automation consumes the same control ids, inherited rows still preserve the shared Lens authority, and the runtime proof exercises distinct field groups without physical mouse input.
