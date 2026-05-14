# Advanced Color Pipeline - Enabled Toggle Preserve Disabled Rows

## Current Phase

Complete - checkpoint commit `0e35f7a` and machine proof receipts closed the preserve-disabled-rows regression slice. The published-runtime witness proves the real viewer path preserves disabled grading and unsupported palette rows in the draft, flips only their enabled state, and still routes deletion solely through the explicit Remove control.

## Phase Checklist

- [x] Phase 1 - keep this bounded regression slice locked; finish the in-app viewer automation control-activation seam, then add focused REDs in the window and published-runtime harnesses that prove or disprove delete-on-uncheck on the real viewer path, unsupported disabled-row visibility, and separate explicit remove behavior without physical mouse input
- [x] Phase 2 - preserve disabled rows across live resync on the real viewer checkbox path, keep unsupported rows visible but disabled, and keep explicit remove semantics unchanged
- [x] Phase 3 - validate with the phase8c ladder, hostile-audit the repaired state, update touched authority surfaces if required, checkpoint, and write machine proof receipts

## Explicit User Asks

- [done] Fix the color-pipeline enabled-toggle regression where unchecking a row removes it from the draft list instead of preserving the row disabled.
- [done] Keep unsupported rows visible in the draft when disabled instead of destroying them.
- [done] Keep remove as a separate explicit action instead of conflating it with disable.
- [done] Lock the regression with the existing UX/window harness.
- [done] Treat `grade.glow` as already closed at `ck:2f8a4f58`.
- [deferred] Keep `balance_void_grade` out of scope.
- [done] Do not reopen weighted blend, basin-default, `neutral_finish`, `tone_map_finish`, `grade.glow`, manual archive, hooks, crash recovery, anti-lie tooling, or unrelated UI polish.

## Proof Ledger

- Bootstrap on 2026-05-13 reported branch `feature/advanced-color-pipeline-draft-editor-reframe`, `HEAD=5273e7b`, clean tree, and active locked contract `advanced_color_library_foundation_phase8f_grade_glow_owner_proof`; the minimal continuity preflight replaced that closed contract with this bounded successor slice before mutation.
- Owner seam proof: `ui_app/src/imgui_stack_editor.h` and `RenderColorPipelineWindowLane(...)` keep `Enabled` and `Remove` as distinct controls, and row deletion still routes only through `rowResult.remove_requested`.
- First runtime RED proof: the published-runtime witness consumed the requested enabled-control activation and then deleted the grading row, proving the live viewer regression was real and tracing the fault beyond the harness seam.
- Repair proof: `SetColorPipelineRowEnabledFromUi(...)` now stages `requestedEnabled` instead of pre-mutating `row.enabled`, and `ResetColorPipelineDraftFromLiveState(...)` merges disabled rows from the pre-reset draft back into the rebuilt live lanes so row count, stable row identity, and enabled-state truth survive live resync.
- Visibility and validation proof: `CollectEnabledColorPipelineRows(...)` still validates enabled rows only, so supported and unsupported disabled rows remain visible in draft state while the remaining enabled tuple applies and validates truthfully.
- Harness proof: `ui_app/src/imgui_stack_editor.h`, `ui_app/src/viewer_cli.cpp`, `ui_app/src/viewer_cli.h`, `ui_app/src/main.cpp`, and `tests/test_fractal_runtime_runtime_walk_viewer.py` now expose deterministic `--ui-automation-click-control-id` activation plus row telemetry, so the published runtime can exercise the checkbox path without a physical mouse.
- Focused proof commands on the repaired head: `cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner`; `cmd /c ui_app\build_vsdevcmd.cmd`; `py -3.14 -m pytest tests/test_fractal_runtime_runtime_walk_viewer.py -k "grading_enabled_checkbox_preserves_disabled_row or unsupported_palette_checkbox_preserves_disabled_row or explicit_remove_button_still_removes_row" -q`; `D:\salt-fractal\cuda_newton_fractal_clone\build_tests\test_viewer_cli.exe`; `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/advanced_color_pipeline_enabled_toggle_preserve_disabled_rows.contract.json --out-json artifacts/validation/advanced_color_pipeline_enabled_toggle_preserve_disabled_rows_contract.json`; `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`; and `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/advanced_color_pipeline_enabled_toggle_preserve_disabled_rows_PHASED_PLAN.md --out-json artifacts/validation/advanced_color_pipeline_enabled_toggle_preserve_disabled_rows_hostile_audit.json` all passed.

## Hostile Audit

- Status: complete
- Required posture: assume the first fix will either only patch one narrow lane, keep ghost rows that cannot re-enable or reapply, break explicit remove semantics, or overstate unsupported-row validation until the window tests, headless action tests, and published-runtime witness all agree on row count, enabled state, validation behavior, and separate remove behavior.

## Audit Passes

- [x] Pass 1 - re-read `SetColorPipelineRowEnabledFromUi(...)`, `ApplyColorPipelineDraftToLiveState(...)`, `SyncColorPipelineWindowFromLiveState(...)`, and the viewer checkbox seam; this exposed a real defect where the draft reset rebuilt lanes from enabled live rows only and dropped disabled rows after uncheck.
- [x] Pass 2 - rerun the focused owner/runtime proofs and explicitly check row count, stable row ids, enabled flags, validation text, and explicit remove behavior on the repaired state; grading disable, unsupported palette disable, explicit remove, and the new viewer CLI flag witnesses all stayed green.
- [x] Pass 3 - re-read the repaired state and final proof surface; no additional real defect was found beyond the repaired live-resync deletion path.

## Audit Findings

- [x] Viewer checkbox deletion was real: the live viewer path pre-mutated checkbox state and then rebuilt draft lanes from the enabled live snapshot, which deleted disabled rows on uncheck. The repair now stages the requested enabled state separately and merges disabled rows back into `ResetColorPipelineDraftFromLiveState(...)`, with published-runtime proof for supported disable, unsupported disable, and explicit remove separation.

## Notes

- Expected owner files:
  - `docs/contracts/advanced_color_pipeline_enabled_toggle_preserve_disabled_rows.contract.json`
  - `docs/notes/advanced_color_pipeline_enabled_toggle_preserve_disabled_rows_PHASED_PLAN.md`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/imgui_stack_editor.h`
  - `ui_app/src/main.cpp`
  - `ui_app/src/viewer_cli.h`
  - `ui_app/src/viewer_cli.cpp`
  - `ui_app/tests/test_color_pipeline_window.cpp`
  - `ui_app/tests/test_viewer_cli.cpp`
  - `tests/test_fractal_runtime_runtime_walk_viewer.py`
  - `HANDOFF_LOG.md`
- Non-goals:
  - do not implement `balance_void_grade`
  - do not reopen weighted blend, basin-default, `neutral_finish`, `tone_map_finish`, or `grade.glow` owner-proof work
  - do not reopen manual archive work
  - do not widen into hooks, crash recovery, anti-lie tooling, or unrelated UI polish

## Resume Point

Closed. Do not resume from this slice's old checkpoint chores. Re-enter later advanced-color work from `docs/notes/advanced_color_feature_restart_inventory.md`, `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`, and `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md` instead.
