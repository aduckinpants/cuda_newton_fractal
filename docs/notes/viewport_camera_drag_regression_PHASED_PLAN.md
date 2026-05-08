# Viewport Camera Drag Regression

## Current Phase

Phase 3 complete - the live post-schema HP resync seam is repaired and validated; checkpoint closure and receipts remain

## Phase Checklist

- [x] Phase 0 - add focused RED coverage proving viewport X/Y/Zoom drags still write the wrong authority fields or ship the wrong zoom UI metadata
- [x] Phase 1 - repair the camera control render path so X/Y/Zoom edits update the HP camera authority correctly and remove the bogus shipped zoom UI cap
- [x] Phase 2 - validate with focused schema/view tests, native helpers, runtime publish, staged validate-ui, contract validation, and phased-plan sync
- [x] Phase 3 - lock the live post-panel resync seam with a focused regression and stop schema camera edits from round-tripping HP state back through float UI mirrors

## Explicit User Asks

- [open] Fix the broken viewport X/Y/Zoom dragging as its own dedicated regression slice before anything else.
- [open] Lock the bug down with unit and regression tests.
- [open] Stop reintroducing stupid zoom limits; the camera must support deep zoom instead of capping out at a tiny UI range.
- [open] After this regression slice, realign the reported advanced-color dropdown/function completeness with what actually ships.

## Presumption Loop

The viewport regression is likely not in the pan/zoom math itself; the existing `view_hp_sync` and `viewport_interaction` seams already cover HP camera behavior. The more falsifiable hypothesis is that the schema-driven camera controls in `ui_app/src/schema_binding.cpp` are writing the UI mirror floats (`view.center.x`, `view.center.y`, `view.zoom`) directly instead of the runtime authority fields (`center_hp_x`, `center_hp_y`, `log2_zoom`), and the shipped schema still advertises an unnecessary zoom UI cap. The cheapest disconfirming checks are focused regressions in `ui_app/tests/test_schema_binding.cpp` and `ui_app/tests/test_ui_schema.cpp` proving that camera control edits fail to keep HP state synchronized or that the shipped zoom control still exposes the bad UI maximum.

The reopened local hypothesis is narrower: the binding seam is now HP-aware, but the live Controls window in `ui_app/src/main.cpp` still sees the changed camera UI mirrors after `RenderSchemaPanels(...)` and immediately calls `SyncViewHpFromUi(view)`, which re-collapses the just-edited HP camera state through float mirrors. The cheapest disconfirming check is a focused schema-binding regression that proves the live post-panel sync decision must stay off after camera edits, especially for zoom-only edits where a stale center mirror can make zooming appear to change the rendered fractal.

## Presumption Evidence

- `RenderFloatControl(...)` in `ui_app/src/schema_binding.cpp` binds `fractal.view.center.x`, `fractal.view.center.y`, and `fractal.view.zoom` as ordinary floats and mutates them directly.
- `ViewState` authority is split: UI mirrors live in `view.center` / `view.zoom`, while runtime authority lives in `center_hp_x`, `center_hp_y`, and `log2_zoom` via `ui_app/src/view_hp_sync.cpp`.
- The shipped schema in `ui/fractal_binding_surface_v1.ui_schema.json` still declares `ui_min` / `ui_max` metadata for the zoom drag control even though deep zoom is HP-owned and should not be UI-capped.
- Existing tests already cover numeric range resolution and HP view math, so the missing proof seam is the schema-driven camera control write-back path itself.
- `ui_app/src/main.cpp` still compares `view.center` / `view.zoom` before and after `RenderSchemaPanels(...)` and calls `SyncViewHpFromUi(view)` when they differ, even though camera float controls now commit through HP authority inside `ApplyFloatControlEdit(...)`.
- A zoom-only camera edit still rewrites `view.center` from HP for display, so the stale post-panel sync can clobber `center_hp_x/y` back to float precision and make zoom round-trips fail to preserve the same rendered view.

## Proof Ledger

- Landed: `ui_app/tests/test_schema_binding.cpp` now locks the camera float control seam against stale UI mirrors and verifies X/Y/Zoom edits write back through HP authority.
- Landed: `ApplySchemaDefaultForControl(...)` now routes camera defaults through the same HP-aware seam, and `ui_app/tests/test_schema_binding.cpp` covers that reset/init path too.
- Landed: `ui_app/tests/test_ui_schema.cpp`, `ui/fractal_binding_surface_v1.ui_schema.json`, and `ui_app/src/safe_mode_schema.cpp` now keep zoom on a hard minimum only, with no shipped UI max cap.
- Landed: `ui_app/src/schema_binding.cpp` now renders camera float controls from HP authority and commits edits back through HP fields instead of mutating only UI mirror floats.
- Validated: code quality audit `97/100`, native helper suite green, runtime publish green, runtime pytest lane green (`68 passed`), and published `fractal_ui.exe --validate-ui` returned cleanly.
- Reopened: user reported the live zoom control is still broken and that zooming can change the rendered fractal, so the prior slice closure missed the post-panel HP resync seam in `ui_app/src/main.cpp`.
- Landed: `ui_app/src/schema_binding.h/cpp` now mark HP-authoritative camera edits and expose `ShouldSyncViewHpFromSchemaUiMirrors(...)`, while `ui_app/src/main.cpp` stops calling `SyncViewHpFromUi(view)` after schema panels when the camera edit already committed through HP authority.
- Landed: `ui_app/tests/test_schema_binding.cpp` now proves the reopened seam directly: a zoom-only schema camera edit marks HP-authority ownership, the live post-panel sync decision stays off, and the legacy float round-trip witness would collapse the HP center.
- Revalidated: code quality audit `97/100`, native helper suite green, runtime publish green, published `fractal_ui.exe --validate-ui` clean, runtime pytest lane green (`68 passed`), contract validation green, and phased-plan sync green on the repaired state.

## Notes

- Expected owner files:
  - `docs/notes/viewport_camera_drag_regression_PHASED_PLAN.md`
  - `docs/contracts/viewport_camera_drag_regression.contract.json`
  - `ui/fractal_binding_surface_v1.ui_schema.json`
  - `ui_app/src/main.cpp`
  - `ui_app/src/schema_binding.h`
  - `ui_app/src/schema_binding.cpp`
  - `ui_app/src/safe_mode_schema.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
  - `ui_app/tests/test_ui_schema.cpp`
- Non-goals:
  - do not widen into advanced-color dropdown completeness in this slice
  - do not reopen unrelated camera math in `viewport_interaction.cpp` unless the regressions prove the bug is actually there

## Resume Point

Add the failing schema-binding regression for the live post-panel HP resync decision first, then repair the dedicated camera control render path in `main.cpp` so camera edits stop round-tripping through float mirrors.