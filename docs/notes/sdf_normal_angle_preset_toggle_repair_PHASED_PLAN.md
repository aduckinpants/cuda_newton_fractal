# SDF Normal Angle Preset Toggle Repair

## Current Phase

Phase 5 - complete; checkpoint, receipts, rearward review, push, and clean-tree proof are handled by the repo workflow after this plan update.

## Phase Checklist

- [x] Phase 1 - open a bounded repair slice for Beauty -> Diagnostic preset toggling.
- [x] Phase 2 - add RED coverage proving Diagnostic clears Beauty's boundary gate.
- [x] Phase 3 - repair recipe application without changing the full-field Diagnostic path.
- [x] Phase 4 - publish runtime and prove no-mouse preset toggling changes the frame.
- [x] Phase 5 - hostile audit, validation, checkpoint, receipts, rearward review, push, and clean tree.

## Explicit User Asks

- [done] Fix the reported bug where clicking SDF Normal Angle Beauty and then SDF Normal Angle Diagnostic appears to do nothing.
- [done] Correct the shallow Beauty preset so it is a product-facing composite, not just a shortcut to an already-existing one-row boundary gate.

## Scope

In scope:

- `SDF Normal Angle Diagnostic` must clear source-local Beauty gate params back to full-field `sdf_gate=none`.
- `SDF Normal Angle Beauty` must apply a boundary-gated normal-angle accent plus a Lens Field v2 contrast row.
- No-mouse runtime proof must cover Beauty -> Diagnostic toggling.

Out of scope:

- New SDF sources.
- Broader Color Pipeline layout or preset manager redesign.
- Authored SDF pack UI and SDF-native lanes.

## Proof Ledger

- Native window rail: `sdf_normal_angle_preset_toggle_repair_native_window` passed (`test_color_pipeline_window: passed=231 failed=0`). It proves Beauty builds a two-row `sdf_normal_angle` + `lens_field_v2_distance` stack, and Diagnostic after Beauty clears the gate back to `none`.
- Runtime publish: `sdf_normal_angle_preset_toggle_repair_publish` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published no-mouse runtime proof: `sdf_normal_angle_preset_toggle_repair_runtime_presets` passed (`2 passed`). It proves Diagnostic -> Beauty -> Gate Width edit -> Diagnostic returns the frame hash to the original Diagnostic hash and removes the Lens Field v2 row.
- Contract validation: `sdf_normal_angle_preset_toggle_repair_contract` passed.
- Plan sync: `sdf_normal_angle_preset_toggle_repair_plan_sync` passed.
- Hostile audit: `sdf_normal_angle_preset_toggle_repair_hostile_audit` passed with two real findings and a clean re-read.
- Code-quality baseline: `sdf_normal_angle_preset_toggle_repair_code_quality` passed baseline (`Score: 93/100`, `CRITICAL: 0`, `ERROR: 0`).
- Diff check: `sdf_normal_angle_preset_toggle_repair_diff_check` passed.
- Closure workflow: checkpoint commit, receipts, rearward review, push, and clean-tree proof are the remaining repo-command epilogue after this committed plan text.

## Hostile Audit

- Status: complete

Required questions:

- Did Diagnostic actually restore full-field normal-angle behavior after Beauty?
- Did Beauty still apply the boundary-band gate by default while adding a meaningful Lens Field v2 contrast base?
- Did the fix route through the same recipe/draft/live path instead of a UI-only label?
- Did any SDF row, capture/replay, or row-local field-downsample behavior get widened or changed?

## Audit Passes

- [x] Pass 1 - Recipe application review found the real state-retention bug: recipe row selection preserved same-function params, so Diagnostic could inherit Beauty's `signal.sdf_gate=boundary_band`.
- [x] Pass 2 - Beauty review found the real product gap: a one-row boundary-gated normal-angle preset was only a shortcut to existing controls, not a meaningful product-facing alternative to full-field Diagnostic.
- [x] Pass 3 - Clean re-read found no additional real defect in this repair scope: Diagnostic is still full-field, Beauty now adds Lens Field v2 contrast plus boundary-gated normal-angle, and no authored-pack/SDF-native work was added.

## Audit Findings

- [x] Real finding: recipe application preserved stale same-function parameter values. Fixed by making recipe lane application reset row defaults instead of preserving existing values.
- [x] Real finding: SDF Normal Angle Beauty was too shallow to justify itself as a product preset. Fixed by making Beauty a two-row composite: boundary-gated normal-angle accent plus Lens Field v2 contrast.
- [x] Clean re-read: no additional real defect found after the native and runtime preset rails passed.
