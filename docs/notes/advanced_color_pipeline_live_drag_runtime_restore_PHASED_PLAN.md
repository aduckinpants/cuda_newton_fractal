# Advanced Color Pipeline Live Drag Runtime Restore

## Current Phase

Complete - live-drag runtime authority restored with seam, viewer, and palette-preserve proof; checkpoint and receipt closeout is the only remaining mechanical step in this turn.

## Phase Checklist

- [x] Phase 1 - prove on current HEAD that color-pipeline drags no longer live-update the viewport in the existing viewer/runtime harnesses, and prove the old palette-preserve witness still stays green on the same state
- [x] Phase 2 - repair the live-drag apply path so active numeric edits update the runtime continuously without resyncing or jumping sibling controls, while keeping supported palette-switch grading preservation intact
- [x] Phase 3 - rerun the narrow seams plus viewer/runtime proofs, hostile-audit the repaired state, checkpoint, and write machine proof receipts

## Explicit User Asks

- [done] Recreate the regression where color-pipeline sliders stopped updating the viewport live during drag.
- [done] Lock the live-drag regression down in the checked-in harnesses that should already have protected it.
- [done] Preserve the palette-switch grading fix while restoring live drag behavior.
- [done] Treat the prior defer-until-release behavior as rejected; the product contract is that color-pipeline sliders update live like the rest of the application.
- [done] Keep this slice bounded to live-drag/runtime-viewer repair plus the required non-regression coverage; do not reopen weighted blend, basin-default, neutral_finish, tone_map_finish, grade.glow, archive archaeology, ExplainO family work, hooks, crash recovery, anti-lie tooling, or unrelated UI polish.

## Proof Ledger

- Bootstrap on 2026-05-14 proved this repo is on `feature/advanced-color-pipeline-draft-editor-reframe` at clean `HEAD=9247b4558870cac24e0b64b76110cc3b451169a2`, and the previous active contract `advanced_color_pipeline_active_drag_and_palette_grading_preserve` is no longer valid authority because the user rejected its defer-until-release behavior.
- Existing checked-in authority already said active drags should use a direct live path: `ui_app/tests/test_schema_binding.cpp` asserts that active programmable-control drags bypass the end-of-frame helper so supported slider edits can use direct live apply instead.
- The initial machine RED on the rejected behavior came from `cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_red` after tightening the schema-binding seam: the current code failed with `Expected active advanced color numeric edits to keep the viewport live while the control remains held`.
- Hostile review also found that the old viewer witness `test_runtime_walk_viewer_physical_color_pipeline_drag_changes_frame` was too weak for this bug because whole-window pixel motion can be satisfied by UI thumb movement after or during drag without proving the fractal viewport changed. This slice adds a stronger viewer witness that samples a viewport-only region before mouse-up.
- The repair in `ui_app/src/color_pipeline_window.h` removes the active-item early-return from `TryApplySupportedColorPipelineDraftFromControl(...)` and stops `ApplyColorPipelineDraftToLiveState(...)` from immediately rebuilding the draft from the just-updated live snapshot. That keeps live runtime updates active during drag while avoiding per-tick draft reset.
- The strengthened viewer witness now lives in `tests/test_fractal_runtime_runtime_walk_viewer.py::test_runtime_walk_viewer_physical_color_pipeline_drag_updates_frame_before_release`, and the seam witness lives in `ui_app/tests/test_schema_binding.cpp`.
- Green proof on the repaired state so far:
  - `cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_red`
  - `cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner`
  - `cmd /c ui_app\build_vsdevcmd.cmd`
  - `py -3.14 -m pytest tests/test_fractal_runtime_runtime_walk_viewer.py -k physical_color_pipeline_drag -q`
  - `py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_explaino_escape_variants.py -k palette_switch_preserves_supported_balance_void_grade`
- Supported palette-switch grading preservation remains green on the repaired state, so restoring live-drag behavior did not reopen the `balance_void_grade` palette-reset regression.

## Hostile Audit

- Status: complete
- Required posture: assume the prior slice broke the product contract by confusing no-flicker with no-live-update, and assume any repair is still wrong until the seam proof, the stronger viewport-only viewer witness, and the palette-preserve witness agree on the same committed state.

## Audit Passes

- [x] Pass 1 - force the current head through the live-drag witnesses and find the real seam that fails; this exposed the false defer-until-release contract in the direct numeric apply path.
- [x] Pass 2 - clean re-read of the repaired seam and viewer proofs found no reopened palette-preserve regression while the viewport updated live during drag.
- [x] Pass 3 - clean re-read of the final diff and current proof outputs found no additional real defect or unsupported closeout claim beyond the remaining checkpoint and receipt work.

## Audit Findings

- [x] Finding 1 - the prior slice encoded the wrong product contract: it treated `has_active_item` as a reason to block direct live apply, even though the checked-in seam authority already said active drags must use a direct live path.
- [x] Finding 2 - the old physical drag viewer test was insufficient because whole-window pixel changes can come from UI motion alone. The stronger viewport-only mid-drag witness is now required for this regression family.
- [x] Clean re-read - after removing the active-item early-return and the post-apply draft reset, no additional shipped-lane regression surfaced in `advanced_color_grading_owner`, the stronger live-drag viewer subset, or the palette-preserve runtime witness.
