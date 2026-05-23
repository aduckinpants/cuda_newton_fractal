# Fractal View Defaults Catalog Migration

## Current Phase

Phase 0 - ready to start after Slice A closes on `master`; this implementation slice is planned but blocked on Slice A completion.

## Phase Checklist

- [ ] Phase 0 - start from `master` after Slice A is merged, run bootstrap, lock this plan/contract, and verify Slice A catalog authority is present.
- [ ] Phase 1 - add RED coverage proving current `ApplyFractalViewPresetDefaults(...)` outputs are preserved for every current fractal type.
- [ ] Phase 2 - move current single-view defaults into the Slice A catalog authority without changing default center, zoom, rotation, or auto-max-iter behavior.
- [ ] Phase 3 - add the minimal view-preset catalog shape needed for later UI work, while keeping only the current canonical preset active by default.
- [ ] Phase 4 - prove state load/reset/startup default behavior remains compatible and explicit selector identity does not change.
- [ ] Phase 5 - hostile-audit for duplicated default authority, stale switch fallbacks, and accidental selector/view UX expansion.
- [ ] Phase 6 - validate focused native/catalog rails, checkpoint, receipts, push, and clean-tree closeout.

## Explicit User Asks

- [open] Move selector/default authority forward only after the catalog inventory foundation is in place.
- [open] Keep the work useful for adding many more supported fractals without new scattered default wiring.
- [open] Preserve current user-facing default views while moving authority.
- [open] Do not build the categorized selector UI, perturbation zoom, new formulas, Color Pipeline changes, FPS pacing, or capture changes in this slice.

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

Planned proof only; no implementation proof exists yet.

Expected focused rails:
- `ui_app/build_tests_vsdevcmd.cmd test_fractal_derived_fields`
- `ui_app/build_tests_vsdevcmd.cmd test_diagnostics_state_io`
- `ui_app/build_tests_vsdevcmd.cmd test_schema_binding`
- contract validation, plan sync, hostile audit, code-quality baseline, and diff hygiene

## Hostile Audit

- Status: pending
- Did I actually preserve current default views, or did the migration subtly move centers/zooms/auto-max-iter values?
- Did any caller bypass the catalog and keep a stale switch path alive?
- Did I accidentally add visible view-preset UI before the internal authority was stable?
- Did selector identity and `explaino_all` behavior stay unchanged?
- Did I avoid perturbation zoom, Color Pipeline, camera/dive, FPS, capture, and new fractal implementation?

## Audit Passes

- [open] Pass 1 - audit before/after default preservation for every current fractal type.
- [open] Pass 2 - audit caller seams for stale duplicate default authority.
- [open] Pass 3 - clean re-read the repaired state after any audit finding is fixed.

## Audit Findings

- [open] No findings yet; this slice is blocked until Slice A is merged.

## Notes

Slice B is the bridge from engine simplification to later product-visible work. It should leave the code ready for categorized selector and view-preset UI, but it should not ship that UI itself.
