# Fractal Catalog Authority Inventory

## Current Phase

Phase 0 - ready to start from `master`; this implementation slice is planned but not yet opened.

## Phase Checklist

- [ ] Phase 0 - create a fresh branch from `master`, run bootstrap, lock this plan/contract, and record the starting repo state.
- [ ] Phase 1 - inspect current enum, selector, schema, default, descriptor, family-rule, animation-applicability, and validation seams.
- [ ] Phase 2 - add RED coverage proving every current `FractalType` must have one catalog metadata row with required fields.
- [ ] Phase 3 - implement the typed catalog authority without changing current default behavior.
- [ ] Phase 4 - prove current defaults, selector ids, schema visibility, descriptor export, and animation applicability remain behavior-compatible.
- [ ] Phase 5 - hostile-audit the new catalog for future-fractal extensibility, missing-row failure behavior, and accidental renderer/schema drift.
- [ ] Phase 6 - validate focused native/catalog rails, checkpoint, receipts, push, and clean-tree closeout.

## Explicit User Asks

- [open] Make engine simplification and selector-default authority the next major foundation before perturbation expansion.
- [open] Ensure this work makes future supported fractal additions easier, not harder.
- [open] Avoid creating another scattered hand-edited path that future fractals must step through.
- [open] Do not start perturbation zoom, new fractal formulas, Color Pipeline changes, FPS pacing, or capture work in this slice.

## Scope

In scope:
- A typed fractal catalog authority surface for current `FractalType` entries.
- Required metadata rows for category, display name, family, default view owner, default parameter owner, sample/probe capability, schema/control-surface capability, animation applicability, smooth-escape/coloring applicability, and perturbation eligibility flag.
- Fail-closed tests when a future `FractalType` lacks required catalog/default/schema coverage.
- Behavior-preservation tests around current defaults and selector identity.

Out of scope:
- Moving view default behavior. That is Slice B.
- Categorized selector UX, view-preset dropdown, startup-default policy change, camera/dive implementation, color tuning, perturbation zoom, new fractal types, or generic renderer redesign.
- Replacing `ViewState` or `KernelParams` as runtime authority.

## Required Owner Seams To Inspect

- `ui_app/src/fractal_types.h`
- `ui_app/src/enum_id_utils.h`
- `ui_app/src/fractal_family_rules.h`
- `ui_app/src/fractal_derived_fields.cpp`
- `ui_app/src/fractal_parameter_surface_descriptor.*`
- `ui_app/src/schema_binding.cpp`
- `ui/fractal_binding_surface_v1.ui_schema.json`
- `ui_app/src/fractal_runtime_validation.h`
- `ui_app/src/sample_tier_resolver.cpp`
- `ui_app/src/perturbation_reference_orbit.h`
- `ui_app/tests/test_fractal_types.cpp`
- `ui_app/tests/test_fractal_derived_fields.cpp`
- `ui_app/tests/test_fractal_family_rules.cpp`
- `ui_app/tests/test_fractal_parameter_surface_descriptor.cpp`
- `ui_app/tests/test_schema_binding.cpp`
- `tests/test_fractal_parameter_surface_descriptor_cli.py`

## Catalog Row V1 Fields

Minimum row shape for this slice:
- `FractalType type`
- stable selector id
- display label
- category/group
- family/subfamily
- default view policy owner
- default parameter policy owner
- runtime class flags: escape-time, basin/root-coloring, generic-equation-pack, generated/internal-root authority, perturbation-eligible
- UI/capability flags: has visible controls, supports no-mouse set-value proof, supports animation applicability, supports smooth escape, supports Color Pipeline frame coloring
- future-growth flag: whether a new supported 2D formula can be added by formula implementation plus catalog row, or requires a separate substrate contract

## RED Targets

- Adding a `FractalType` enum value without a catalog row fails a native test.
- Adding a catalog row without selector/string/schema/descriptor coverage fails a native or CLI test.
- Current default presets remain field-compatible for representative families.
- Existing explicit selector identity remains unchanged.
- Current `explaino_all` registry/common-axis behavior is not changed.
- Perturbation eligibility remains truthful: current support is Mandelbrot/Julia only unless a later perturbation slice proves more.

## Proof Ledger

Planned proof only; no implementation proof exists yet.

Expected focused rails:
- `ui_app/build_tests_vsdevcmd.cmd test_fractal_types`
- `ui_app/build_tests_vsdevcmd.cmd test_fractal_derived_fields`
- `ui_app/build_tests_vsdevcmd.cmd test_fractal_family_rules`
- `ui_app/build_tests_vsdevcmd.cmd test_fractal_parameter_surface_descriptor`
- `ui_app/build_tests_vsdevcmd.cmd test_schema_binding`
- `py -3.14 -m pytest tests/test_fractal_parameter_surface_descriptor_cli.py -q`
- contract validation, plan sync, hostile audit, code-quality baseline, and diff hygiene

## Hostile Audit

- Status: pending
- Did I actually make new fractal additions simpler, or just create another table that must be manually duplicated elsewhere?
- Did every current `FractalType` get exactly one truthful catalog row?
- Did selector identity, schema visibility, descriptor export, and defaults stay behavior-compatible?
- Did the catalog fail closed when a row is missing or incomplete?
- Did I accidentally start Slice B, perturbation zoom, color tuning, camera/dive, or new fractal implementation?

## Audit Passes

- [open] Pass 1 - audit the initial catalog model for duplicated authority and missing fields.
- [open] Pass 2 - audit behavior-preservation proof against current default and selector surfaces.
- [open] Pass 3 - clean re-read the repaired state after any audit finding is fixed.

## Audit Findings

- [open] No findings yet; this slice is not started.

## Notes

The target outcome is not abstraction for its own sake. The target is a fail-closed, low-friction path where a supported 2D fractal addition starts with formula implementation plus one catalog row, and tests identify the remaining required seams.
