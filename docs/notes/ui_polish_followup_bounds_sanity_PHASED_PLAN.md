# UI Polish Follow-Up - Bounds Sanity And Drag Clamp Repair

## Current Phase

Phase 3 complete - drag controls no longer turn UI-only ranges into hard limits, and the pre-merge bounds sanity follow-up is checkpoint-ready

## Phase Checklist

- [x] Phase 1 - capture the drag-range clamp baseline and lock the first failing regression
- [x] Phase 2 - implement the binding/schema repair and sweep nearby bound-value cases
- [x] Phase 3 - hostile audit the repaired control ranges, validate on the published runtime, and checkpoint the follow-up

## Explicit User Asks

- [done] Remove the effective hard bounds on camera zoom and center x/y.
- [done] Review the current bound values for more cases where UI-only ranges are being treated as real limits.
- [done] Finish this sanity pass before merging and running the sprint audit.

## Presumption Loop

The most likely owner is still the binding/render seam, not the raw schema metadata. The schema already distinguishes `ui_min`/`ui_max` from `min`/`max`, but the current drag-control renderer appears to pass widget ranges directly into ImGui, which turns a visual hint into a real clamp.

Hostile review assumes this is not only a zoom bug. Once the drag seam is fixed, sweep the nearby `drag_*` controls and confirm that UI-only ranges no longer clamp while true hard bounds still do.

## Presumption Evidence

- Owner Proof: `ui_app/src/schema_binding.cpp` still feeds `ResolveNumericControlRange(...)` widget bounds into `ImGui::DragFloat`, `ImGui::DragInt`, and `ImGui::DragScalar`, so `ui_min`/`ui_max` metadata on drag controls currently behaves like a hard limit during interaction.
- RED Witness: the live schema still advertises `ui_min/ui_max` for `center_x`, `center_y`, and `zoom`, and the user confirmed that zoom tops out around the old `64.0` UI cap despite the engine supporting vastly deeper zoom.
- Workflow Proof: this is a pre-merge repair on `feature/ui-polish-resolution-pacing`, so closure still requires native helper validation plus published-runtime proof on `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Fix Proof: `ResolveNumericDragWidgetBounds(...)` now strips drag-widget bounds down to real bilateral hard clamps only, so UI-only ranges like `center_x`, `center_y`, and the one-sided `zoom >= 1e-12` control remain editable beyond the old widget hints while still preserving true post-edit clamps.
- Hostile Review Pass 1: the shared drag fix was audited against three cases: pure UI-only drags (`center_x`), one-sided hard clamps (`zoom`), and true bilateral hard clamps (`lambda_real`). The new regression coverage proved that only the bilateral clamp still reaches the widget layer.
- Hostile Review Pass 2: after the seam repair, the native helper rail, code-quality audit, runtime publish, and deployed `--validate-ui` check all passed. No second clamp surface remained in the touched controls.

## Proof Ledger

- Manual RED: owner inspection showed that `drag_*` controls still received `widget_min/widget_max` from `ResolveNumericControlRange(...)`, which means the supposed UI hint ranges for camera center/zoom were acting as hard drag limits.
- Checked-in regression RED: the new schema-binding regressions now lock three concrete cases in `ui_app/tests/test_schema_binding.cpp`: UI-only drag ranges must not create widget clamp bounds, one-sided hard limits like `zoom >= 1e-12` must also avoid widget clamp bounds, and truly clamped drags like `lambda_real` must keep their bilateral hard bounds.
- First GREEN: `cmd /c ui_app\build_tests_vsdevcmd.cmd` passed after the drag render path switched to `ResolveNumericDragWidgetBounds(...)`, leaving post-edit hard clamps intact while removing in-widget bounds for UI-only and one-sided cases.
- Post-green hostile finding: the global sweep did not uncover a second owner bug. The same drag seam covered the nearby camera and polynomial-coefficient cases, while true hard-clamped drags still retain their widget bounds.

## Notes

- Branch owner:
  - active branch: `feature/ui-polish-resolution-pacing`
  - integration base: `feature/ui-polish-integration`
- Expected owner files:
  - `ui_app/src/schema_binding.h`
  - `ui_app/src/schema_binding.cpp`
  - `ui/fractal_binding_surface_v1.ui_schema.json`
  - `ui_app/src/safe_mode_schema.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
  - `ui_app/tests/test_ui_schema.cpp`
- Non-goals:
  - do not reopen color-mode authority or render-resolution defaults here
  - do not start the sprint merge/audit flow until this bounds sanity follow-up is closed
- Validation target:
  - focused schema/binding regressions for drag-bound behavior
  - relevant native helper and published-runtime validation before checkpoint

## Resume Point

The pre-merge bounds sanity follow-up is complete on `feature/ui-polish-resolution-pacing`. The next step is to merge this branch into `feature/ui-polish-integration` and run the planned sprint integration audit.