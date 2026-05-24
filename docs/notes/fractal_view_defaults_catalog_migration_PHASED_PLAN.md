# Fractal View Defaults Catalog Migration

## Current Phase

Closed - catalog-owned view defaults are implemented and validated; `ApplyFractalViewPresetDefaults(...)` now consumes the catalog surface without changing current defaults.

## Phase Checklist

- [x] Phase 0 - start from `master` after Slice A is merged, run bootstrap, lock this plan/contract, and verify Slice A catalog authority is present.
- [x] Phase 1 - add RED coverage proving current `ApplyFractalViewPresetDefaults(...)` outputs are preserved for every current fractal type.
- [x] Phase 2 - move current single-view defaults into the Slice A catalog authority without changing default center, zoom, rotation, or auto-max-iter behavior.
- [x] Phase 3 - add the minimal view-preset catalog shape needed for later UI work, while keeping only the current canonical preset active by default.
- [x] Phase 4 - prove state load/reset/startup default behavior remains compatible and explicit selector identity does not change.
- [x] Phase 5 - hostile-audit for duplicated default authority, stale switch fallbacks, and accidental selector/view UX expansion.
- [x] Phase 6 - validate focused native/catalog rails, checkpoint, receipts, push, and clean-tree closeout.

## Explicit User Asks

- [done] Move selector/default authority forward only after the catalog inventory foundation is in place.
- [done] Keep the work useful for adding many more supported fractals without new scattered default wiring.
- [done] Preserve current user-facing default views while moving authority.
- [done] Do not build the categorized selector UI, perturbation zoom, new formulas, Color Pipeline changes, FPS pacing, or capture changes in this slice.

## Scope

In scope:
- Migrate current hardcoded single-view defaults into the catalog authority created by Slice A.
- Preserve existing `ApplyFractalViewPresetDefaults(...)` behavior as the public compatibility function.
- Add tests that enumerate all current fractal ids and compare before/after default center, zoom, rotation, and auto-max-iter behavior.
- Prepare a small internal view-preset row shape that later categorized selector/view-preset UI can consume.

Out of scope:
- Categorized dropdown UI and visible `view_preset` control. That is the following product slice.
- Changing startup default policy.
- Camera/dive behavior modes.
- Smooth-escape/color tuning.
- Perturbation deep-zoom expansion.
- New fractal formulas or substrate families.

## Required Owner Seams To Inspect

- `ui_app/src/fractal_derived_fields.cpp`
- `ui_app/src/fractal_derived_fields.h`
- Slice A catalog files created by `fractal_catalog_authority_inventory`
- `ui_app/src/viewer_state_init.cpp`
- `ui_app/src/runtime_reset.cpp`
- `ui_app/src/main.cpp`
- `ui_app/src/runtime_walk_bootstrap.cpp`
- `ui_app/src/diagnostics_state_io.cpp`
- `ui_app/tests/test_fractal_derived_fields.cpp`
- `ui_app/tests/test_diagnostics_state_io.cpp`
- `ui_app/tests/test_schema_binding.cpp`

## Migration Rules

- Preserve current public defaults exactly unless a RED proves an existing default is invalid and the user explicitly accepts a behavior change.
- Keep `ViewState` as runtime authority and `KernelParams` as parameter authority.
- Keep `ApplyFractalViewPresetDefaults(...)` as the compatibility seam for callers in this slice.
- Do not add a second persisted resolution/camera/view source of truth.
- Do not make future view-preset UI decisions beyond the minimal internal row shape.

## RED Targets

- A native all-fractal default-preservation test captures current view defaults before migration and fails when catalog output diverges.
- A test fails when a `FractalType` lacks a default-view catalog row or explicit default-view policy.
- Existing reset/startup/state-load paths still call the compatibility seam and produce current values.
- Future view-preset row shape exists but is not surfaced as a new user control in this slice.

## Proof Ledger

Proof so far:
- RED: `fractal_view_defaults_red_derived_fields` failed because `FractalCatalogViewDefaults` / `FindFractalCatalogViewDefaults(...)` did not exist.
- GREEN: `fractal_view_defaults_final_derived_fields` passed after `ApplyFractalViewPresetDefaults(...)` consumed catalog-owned defaults.
- GREEN: `fractal_view_defaults_catalog_authority` passed after catalog rows exposed `default_view` and lookup identity.
- GREEN: `fractal_view_defaults_final_diagnostics_state_io` passed, preserving state load defaults.
- GREEN: `fractal_view_defaults_final_schema_binding` passed, preserving schema/binding behavior.
- GREEN: `fractal_view_defaults_catalog_migration_code_quality_preclose` passed baseline audit.
- GREEN: `fractal_view_defaults_catalog_migration_contract` passed contract validation.
- GREEN: `viewer_host_assert_phased_plan_sync.py` passed.
- GREEN: `fractal_view_defaults_catalog_migration_hostile_audit` passed.
- GREEN: `fractal_view_defaults_catalog_migration_code_quality` passed baseline audit.
- GREEN: `fractal_view_defaults_catalog_migration_diff_check` passed.
- GREEN: `fractal_view_defaults_full_native` passed full native helper suite: all helper tests passed.

Final focused rails:
- `py -3.14 tools/viewer_host_run_logged_command.py --label fractal_view_defaults_final_derived_fields --log artifacts/logs/fractal_view_defaults_final_derived_fields.log --out-json artifacts/validation/fractal_view_defaults_final_derived_fields.json --heartbeat-seconds 30 --timeout-seconds 180 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_derived_fields`
- `py -3.14 tools/viewer_host_run_logged_command.py --label fractal_view_defaults_final_diagnostics_state_io --log artifacts/logs/fractal_view_defaults_final_diagnostics_state_io.log --out-json artifacts/validation/fractal_view_defaults_final_diagnostics_state_io.json --heartbeat-seconds 30 --timeout-seconds 180 -- ui_app/build_tests_vsdevcmd.cmd test_diagnostics_state_io`
- `py -3.14 tools/viewer_host_run_logged_command.py --label fractal_view_defaults_final_schema_binding --log artifacts/logs/fractal_view_defaults_final_schema_binding.log --out-json artifacts/validation/fractal_view_defaults_final_schema_binding.json --heartbeat-seconds 30 --timeout-seconds 240 -- ui_app/build_tests_vsdevcmd.cmd test_schema_binding`
- contract validation, plan sync, hostile audit, code-quality baseline, and diff hygiene

## Hostile Audit

- Status: complete
- Did I actually preserve current default views, or did the migration subtly move centers/zooms/auto-max-iter values?
- Did any caller bypass the catalog and keep a stale switch path alive?
- Did I accidentally add visible view-preset UI before the internal authority was stable?
- Did selector identity and `explaino_all` behavior stay unchanged?
- Did I avoid perturbation zoom, Color Pipeline, camera/dive, FPS, capture, and new fractal implementation?

## Audit Passes

- [done] Pass 1 - audit before/after default preservation for every current fractal type found and removed a literal fallback duplicate in `ApplyFractalViewPresetDefaults(...)`.
- [done] Pass 2 - audit caller seams for stale duplicate default authority found no remaining `ResolveFractalViewPresetDefaults`, `TryResolveEscapeTimeViewPresetDefaults`, `FractalViewPresetDefaults`, or `default_auto_max_iter` symbols.
- [done] Pass 3 - clean re-read after the fallback fix found no visible UI expansion, perturbation work, Color Pipeline changes, FPS changes, capture changes, or new formulas.

## Audit Findings

- [done] Preflight finding 1: the initial Slice B contract used a bare multi-target native command that would not produce separate logged proof artifacts. Replaced it with logged single-target native rails before feature edits.
- [done] Hostile audit finding 2: the first GREEN implementation left a literal fallback default in `ApplyFractalViewPresetDefaults(...)`. Replaced it with `FractalCatalogViewDefaultsFor(...)` so fallback behavior stays on the catalog surface.

## Notes

Slice B is the bridge from engine simplification to later product-visible work. It should leave the code ready for categorized selector and view-preset UI, but it should not ship that UI itself.
