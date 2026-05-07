# Advanced Color Pipeline Debounced Live Preview

## Current Phase

Phase 2 - debounced live preview is implemented and validated; checkpoint, receipts, and clean-worktree closure remain

## Phase Checklist

- [x] Phase 0 - prove the current gate starves live feedback during active drag and lock the desired debounced-preview behavior in focused coverage
- [x] Phase 1 - restore live preview with short debounced end-of-frame apply while keeping the checkbox persistent, default-on, and drag-safe
- [ ] Phase 2 - validate the focused seam, refresh published runtime proof, and close the slice cleanly

## Explicit User Asks

- [open] Keep the checkbox behavior instead of bringing back a separate apply button.
- [open] Restore live runtime feedback while dragging programmable color sliders.
- [open] Make these sliders behave like the rest of the application instead of waiting until mouse release.
- [open] Land the fix with TDD and regression coverage before discussing wider color-pipeline productization.

## Presumption Loop

The controlling seam is still the post-lane auto-apply path in `RenderColorPipelineWindow(...)` inside `ui_app/src/color_pipeline_window.h`. The falsifiable local hypothesis is that the current `!has_active_item` gate fixed the focus drop by overcorrecting: it suppresses all runtime updates until the control is released, which removes the live visual feedback operators expect from the rest of the slider surface. The cheapest discriminating check is a focused helper-level regression: keep auto-apply armed, mark an item active, and prove the helper allows apply only after a short debounce interval elapses while the draft still diverges from live.

## Presumption Evidence

- Operator proof: with the drag-safety fix in place, the user reports that the slider now gives zero feedback during drag and no longer behaves like the other runtime sliders.
- Code-path proof: `ShouldAutoApplySupportedColorPipelineDraft(...)` currently hard-blocks every active interaction by requiring `!interactionState.has_active_item`, so the draft cannot reach runtime until the slider is released.
- Reuse proof: the rest of the app already treats slider interaction as a live path; this window is the outlier only because it stages through a draft and needs a short apply throttle instead of a release-only gate.

## Proof Ledger

- GREEN: `ui_app/tests/test_schema_binding.cpp` now proves armed auto-apply remains immediate when no item is active, stays throttled while an item remains active inside the short debounce window, resumes live preview once that interval elapses, and still stays disarmed when the checkbox is off.
- GREEN: `ui_app/src/color_pipeline_window.h` now keeps the post-lane auto-apply ordering from the drag-safety fix but replaces the release-only block with a short debounced live-preview gate keyed off the last successful auto-apply timestamp.
- GREEN: the checkbox remains persistent and default-on, supported drafts still apply after edit completion, and active drags now receive short-interval runtime feedback instead of waiting until mouse release.
- Validation: focused schema-binding native coverage passed via `artifacts/schema_binding_debounced_preview.log`, `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json` passed with `97/100`, `cmd /c ui_app\build_tests_vsdevcmd.cmd` passed, `cmd /c ui_app\build_vsdevcmd.cmd` republished the runtime, staged `fractal_ui.exe --validate-ui` passed, and `py -3.14 tools/viewer_host_runtime_pytest_lane.py` passed with `68 passed`.
- Audit: distrust-first review did not find another local defect after the debounce change; the smallest controlling seam remains the auto-apply decision helper plus the end-of-frame apply timestamp update.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_pipeline_debounced_live_preview_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_pipeline_debounced_live_preview.contract.json`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals:
  - do not revert the checkbox back to a button
  - do not disable auto-apply by default
  - do not widen runtime row support in this slice

## Resume Point

Run phased-plan sync and contract validation for `advanced_color_pipeline_debounced_live_preview`, append the handoff entry with `ck:b3259d9b`, checkpoint the slice, write receipts, and finish on a zero-output repo-status check.