# Advanced Color Root Palette Tuple Switch Follow-Up

## Current Phase

Complete - tuple-aware Source/Palette co-switching and fixed-row messaging are landed, the checkpoint/profile validation rail is green, and this follow-up is ready to checkpoint before Phase 4 resumes at `joy_root_palette`

## Phase Checklist

- [x] Phase 1 - add focused regressions that prove selecting `root_index` or `root_classic_palette` in the advanced editor co-switches the matching companion lane instead of leaving the user in an apparent dead-end draft-only tuple
- [x] Phase 2 - implement the smallest tuple-aware source/palette switching seam and explanatory fixed-row text needed to make the landed basin bridge usable
- [x] Phase 3 - rerun the bounded native/runtime/workflow rails, audit the repaired UI behavior, checkpoint the follow-up, and refresh the main foundation resume point

## Explicit User Asks

- [open] Existing items merely appearing in the list is not enough if they all present as non-working `(draft)` options.
- [open] Root Classic Palette exposing no parameters without clear guidance is not a usable operator experience.
- [open] If the last slice only removed prerequisites, say so clearly; otherwise treat the current selector behavior as a bug.
- [open] Resume the planned advanced-color work, but not by papering over a friction point that leaves the newly landed row effectively unusable.

## Presumption Loop

The controlling bug is local UI behavior, not missing runtime support. The published runtime already contains `root_index` and `root_classic_palette`, and the last slice proved the default basin tuple imports cleanly as a supported live snapshot. The falsifiable local hypothesis is that the combo-box gating still evaluates one-lane mutations against the whole tuple without helping the matching companion lane co-switch, so family-gated pairs read as broken even though the bridge exists. The cheapest disconfirming check is the single helper path that powers lane selection and combo candidate status in `ui_app/src/color_pipeline_window.h`; if that seam mutates only one lane and immediately asks `TryBuildColorPipelineSelectionFromDraft(...)`, the observed behavior is explained.

## Presumption Evidence

- `ui_app/src/color_pipeline_window.h` currently labels candidates as `(draft only)` when `DescribeColorPipelineCandidateApplyState(...)` cannot build an exact supported tuple after mutating only the selected lane.
- The published runtime binary already contains `Root Index` and `Root Classic Palette`, so the issue is not an unpublished build or a missing catalog string.
- `ui_app/tests/test_schema_binding.cpp` already proves the live root-basin tuple imports as `root_index + root_classic_palette`, which means the bridge exists and the remaining defect is the interactive lane-switch path.
- `root_classic_palette` is intentionally parameterless, so the UI must explain that selection changes the fixed palette lineage directly instead of pretending there should be tunable controls.

## Proof Ledger

- Read-only finding: the last `root_classic` slice landed real advanced rows, but the one-lane combo-box apply heuristic leaves those rows looking broken in normal use.
- Read-only finding: the shortest honest fix is tuple-aware companion switching for Source/Palette plus explicit fixed-row messaging, not another runtime-palette expansion.
- Landed: `ui_app/src/color_pipeline_window.h` now shares Source/Palette tuple recognition through a single lane-id helper, auto-completes unique shipped Source/Palette pairs during single-row selection, and tells the operator when a parameterless fixed row changes the live bridge tuple directly.
- Landed: `ui_app/tests/test_schema_binding.cpp` now proves the real operator path: sync from a non-basin live tuple, select `root_index` or `root_classic_palette`, auto-complete the companion lane, apply into the basin tuple, and keep shipped Source/Palette choices out of fake preview-only dead ends.
- Validated so far: `artifacts/root_tuple_switch_native_v5.log` is green for `ui_app/build_tests_vsdevcmd.cmd` after the tuple-switch repair and the hostile neighboring regression updates.
- Validated: `artifacts/code_quality_report.json` stayed on the `97/100` baseline, `artifacts/verify_native_helper_tests.log` is green, `artifacts/verify_runtime_publish.log` republished the active runtime cleanly, `artifacts/verify_runtime_probe_session_pytest.log` reports `68 passed`, the follow-up contract validated cleanly, and phased-plan sync passed after the follow-up/main-plan refresh.

## Notes

- Expected owner files for this follow-up:
  - `docs/contracts/advanced_color_root_palette_tuple_switch_followup.contract.json`
  - `docs/notes/advanced_color_root_palette_tuple_switch_followup_PHASED_PLAN.md`
  - `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals for this follow-up:
  - do not widen `joy_root_palette` yet
  - do not add new runtime parameters to `root_classic_palette`
  - do not redesign the entire advanced editor around tuple presets in one pass

## Resume Point

Checkpoint this tuple-switch follow-up, then resume the main foundation plan at `joy_root_palette` with the editor now treating shipped Source/Palette rows as auto-completed supported pairs instead of draft-only dead ends.