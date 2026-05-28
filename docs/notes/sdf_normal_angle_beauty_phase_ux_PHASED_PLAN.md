# SDF Normal Angle Beauty Phase UX

## Current Phase

Phase 6 - complete; checkpoint, receipts, rearward review, push, and clean-tree proof are handled by the repo workflow after this plan update.

## Phase Checklist

- [x] Phase 1 - create and lock the checked-in Step 4 plan and contract.
- [x] Phase 2 - add focused RED coverage for diagnostic versus boundary-masked normal-angle UX.
- [x] Phase 3 - expose a narrow boundary-masked normal-angle beauty source/preset without changing the existing diagnostic path.
- [x] Phase 4 - bind UI-Salt/native compatibility, companion suggestions, recipe application, and runtime authority.
- [x] Phase 5 - publish runtime and prove diagnostic/beauty modes, capture/replay preservation, SDF row preservation, and row-local downsample preservation.
- [x] Phase 6 - hostile audit, validation, checkpoint, receipts, rearward review, push, and clean tree.

## Explicit User Asks

- [done] Start the next SDF work item after merging the closed row-local field downsample branch.
- [done] Address the harsh branch-cut planes from `sdf_normal_angle` without removing the visually useful full-field diagnostic behavior.
- [done] Keep the current SDF performance foundation, row-local downsample work, capture/replay, and no-mouse proof rails intact.

## Scope

In scope:

- Step 4 only: phase-safe normal-angle product UX.
- Preserve the current full-field `SDF Normal Angle Diagnostic` preset and source behavior.
- Add a narrow boundary-masked normal-angle beauty path using existing SDF boundary-gate semantics.
- Keep source params visible and authoritative: scale, bias, SDF Gate, Gate Width, SDF Row Step, Field Downsample, and Blend Weight.
- Prove the diagnostic and beauty paths are distinct, runtime-backed, no-mouse controllable, and capture/replay safe.
- Record in docs that full-field normal-angle branch/medial-axis planes are expected diagnostic phase-field behavior, not a fractal kernel bug.

Out of scope:

- Broader Color Pipeline function-library redesign.
- SDF masks/gates as first-class composition operands beyond the existing source-local gate.
- Authored SDF pack viewport integration.
- SDF-native fractal lanes.
- Generic phase-safe shape/palette redesign for all phase sources.
- Global FPS improvement claims.

## Proof Ledger

- RED coverage:
  - `sdf_normal_angle_beauty_phase_ux_red_core` failed as expected until the fourth UI-Salt/materialized recipe existed.
  - `sdf_normal_angle_beauty_phase_ux_red_window` failed as expected until the beauty recipe applied `signal.sdf_gate=boundary_band` and `signal.sdf_gate_width_px=6.0`.
- Contract validation: `sdf_normal_angle_beauty_phase_ux_contract` passed.
- UI-Salt materialization and metadata parity: `sdf_normal_angle_beauty_phase_ux_materialize` and `sdf_normal_angle_beauty_phase_ux_ui_salt_tests` passed.
- Focused native rails:
  - `sdf_normal_angle_beauty_phase_ux_native_core` passed (`test_color_pipeline_core: passed=2656 failed=0`).
  - `sdf_normal_angle_beauty_phase_ux_native_window` passed (`test_color_pipeline_window: passed=226 failed=0`).
  - `sdf_normal_angle_beauty_phase_ux_native_postprocess` passed (`test_color_pipeline_sdf_postprocess: passed=111 failed=0`).
  - `sdf_normal_angle_beauty_phase_ux_native_capture` passed (`test_diagnostics_capture: all passed`).
- Runtime publish: `sdf_normal_angle_beauty_phase_ux_publish` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published no-mouse runtime proof:
  - `sdf_normal_angle_beauty_phase_ux_runtime_presets` passed (`2 passed`) and proves Diagnostic and Beauty recipes both apply through controls, render distinct hashes, and Beauty exposes/uses Gate Width.
  - `sdf_normal_angle_beauty_phase_ux_runtime_sdf_rows` passed (`9 passed`) preserving SDF row behavior.
  - `sdf_normal_angle_beauty_phase_ux_runtime_capture_replay` passed (`1 passed`) preserving the beauty gate in capture/replay authority.
  - `sdf_normal_angle_beauty_phase_ux_runtime_row_field` passed (`2 passed`) preserving row-local field downsample authority.
- Phased-plan sync: `sdf_normal_angle_beauty_phase_ux_plan_sync` passed.
- Hostile-audit validation: `sdf_normal_angle_beauty_phase_ux_hostile_audit` passed with two real findings and a clean re-read.
- Code-quality baseline: `sdf_normal_angle_beauty_phase_ux_code_quality` passed baseline (`Score: 93/100`, `CRITICAL: 0`, `ERROR: 0`).
- Diff check: `sdf_normal_angle_beauty_phase_ux_diff_check` passed.
- Closure workflow: checkpoint commit, receipts, rearward review, push, and clean-tree proof are the remaining repo-command epilogue after this committed plan text.

## Hostile Audit

- Status: complete

Required questions:

- Did the slice keep full-field `sdf_normal_angle` diagnostic behavior available and intentionally diagnostic?
- Did the new beauty path actually boundary-mask normal-angle instead of hiding or deleting phase behavior?
- Did the new source/preset route through the same runtime-backed SDF postprocess path and not a fake UI-only label?
- Did `sdf_normal_angle` remain classified as phase-like while signed distance, boundary band, and curvature stayed non-phase?
- Did SDF row-local field downsample authority from Step 3C remain intact?
- Did capture/replay preserve the selected normal-angle path and source params?
- Did this slice avoid authored SDF UI, SDF-native lanes, and broad Color Pipeline redesign?

## Audit Passes

- [x] Pass 1 - Descriptor/UI-Salt parity found real drift: runtime UI-Salt contract tests still expected three recipes after adding SDF Normal Angle Beauty. Repaired both recipe counters and reran `sdf_normal_angle_beauty_phase_ux_ui_salt_tests` green.
- [x] Pass 2 - Runtime capture/replay review found real authority drift: `color_effective_source.source_stack` omitted `sdf_gate` and `sdf_gate_width_px`, so the beauty gate could render and replay while the summary stayed incomplete. Repaired `diagnostics_capture`, added native capture coverage, rebuilt, and reran capture/replay green.
- [x] Pass 3 - Clean re-read of the repaired state found no additional real defect in Step 4 scope: full-field Diagnostic remains available, Beauty uses the same runtime-backed `sdf_normal_angle` source with boundary-band gating, SDF rows and row-local downsample runtime rails stayed green, and no authored-pack/SDF-native work was added.

## Audit Findings

- [x] Real finding: UI-Salt runtime metadata proof was under-specified for recipe count and would have let the new recipe drift from published metadata parity. Fixed in `tests/test_fractal_runtime_ui_salt_contract.py`.
- [x] Real finding: capture effective-source summary did not include source-local SDF gate authority. Fixed in `ui_app/src/diagnostics_capture.cpp`, `ui_app/tests/test_diagnostics_capture.cpp`, and `tests/test_fractal_runtime_capture_replay_authority.py`.
- [x] Clean re-read: no additional real defect found after the repaired metadata and capture-summary rails passed.
