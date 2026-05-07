# Advanced Color Pipeline Auto-Apply Drag Stability

## Current Phase

Phase 2 - auto-apply drag stability is implemented and validated; checkpoint, receipts, and clean-worktree closure remain

## Phase Checklist

- [x] Phase 0 - confirm the active-drag focus loss is caused by the auto-apply loop rather than the slider widgets
- [x] Phase 1 - defer auto-apply while any programmable control is actively being manipulated, then apply after edit completion
- [ ] Phase 2 - validate focused behavior, refresh published runtime proof, and close the slice cleanly

## Explicit User Asks

- [open] Keep the new checkbox behavior.
- [open] Restore normal slider dragging while auto-apply stays enabled.
- [open] Stop guessing and land the next move against the confirmed culprit.

## Presumption Loop

The controlling seam is still `RenderColorPipelineWindow(...)` and `RenderColorPipelineWindowSummary(...)` in `ui_app/src/color_pipeline_window.h`. The falsifiable local hypothesis is that applying the draft inside the summary pre-pass mutates window/runtime state before the lane controls finish their active drag interaction, which drops the ImGui active item and makes sliders only clickable instead of draggable. The cheapest discriminating check is already confirmed by operator behavior: with auto-apply disabled, the same sliders drag normally.

## Presumption Evidence

- Confirmation proof: the operator confirmed that unchecking auto-apply restores normal slider dragging immediately.
- Code-path proof: the current summary path auto-applies supported drafts before lane controls render, while the slider widgets themselves are plain `SliderFloat` / `SliderInt` controls.
- Reuse proof: `ui_app/src/schema_binding.cpp` already tracks `IsItemActivated`, `IsItemActive`, and `IsItemDeactivatedAfterEdit()` to avoid mutating state while a widget is still being manipulated.

## Proof Ledger

- GREEN: `ui_app/src/color_pipeline_window.h` now tracks active programmable-control interaction and defers auto-apply until no control is still active, instead of mutating live state in the summary pre-pass while a slider drag is underway.
- GREEN: the checkbox remains persistent and default-on, and the deferred post-lane auto-apply still commits supported drafts once interaction completes.
- GREEN: `ui_app/tests/test_schema_binding.cpp` now proves armed auto-apply remains eligible when no control is active, defers while a control is active, and stays disarmed when the checkbox is off.
- Validation: focused schema-binding logged check passed, `cmd /c ui_app\build_tests_vsdevcmd.cmd` passed, `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json` passed, `cmd /c ui_app\build_vsdevcmd.cmd` published the staged runtime, staged `fractal_ui.exe --validate-ui` passed, and `py -3.14 tools/viewer_host_runtime_pytest_lane.py` passed with `68 passed`.
- Audit: distrust-first review did not find a new defect after the defer-until-not-active change. The same pass refreshed stale Shape bridge copy in the touched UI seam so user-facing help now matches the currently live-backed Shape rows.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_pipeline_auto_apply_drag_stability_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_pipeline_auto_apply_drag_stability.contract.json`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals:
  - do not revert the checkbox back to a button
  - do not disable auto-apply by default
  - do not widen runtime row support in this slice

## Resume Point

Write the handoff entry with `ck:0a88a3aa`, validate plan sync and the slice contract, checkpoint the slice, write receipts, and finish on a zero-output repo-status check.