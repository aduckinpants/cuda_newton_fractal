# Fractal Catalog Authority Inventory

## Current Phase

Closed - Slice A catalog authority inventory validated; Slice B default-routing consumption remains deferred.

## Phase Checklist

- [x] Phase 0 - create a fresh branch from `master`, run bootstrap, lock this plan/contract, and record the starting repo state.
- [x] Phase 1 - inspect current enum, selector, schema, default, descriptor, family-rule, animation-applicability, and validation seams.
- [x] Phase 2 - add RED coverage proving every current `FractalType` must have one catalog metadata row with required fields.
- [x] Phase 3 - implement the typed catalog authority without changing current default behavior.
- [x] Phase 4 - prove current defaults, selector ids, schema visibility, descriptor export, and animation applicability remain behavior-compatible.
- [x] Phase 5 - hostile-audit the new catalog for future-fractal extensibility, missing-row failure behavior, and accidental renderer/schema drift.
- [x] Phase 6 - validate focused native/catalog rails, checkpoint, receipts, push, and clean-tree closeout.

## Explicit User Asks

- [done] Make engine simplification and selector-default authority the next major foundation before perturbation expansion.
- [done] Ensure this work makes future supported fractal additions easier, not harder.
- [done] Avoid creating another scattered hand-edited path that future fractals must step through.
- [done] Do not start perturbation zoom, new fractal formulas, Color Pipeline changes, FPS pacing, or capture work in this slice.

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
- runtime class flags: escape-time, basin/root-coloring, Explaino family, perturbation-eligible
- UI/capability flags: sample/probe, schema/control-surface, animation applicability, smooth-escape coloring, Color Pipeline frame coloring, generic equation pack, root/basin coloring
- future-growth surface: native 2D formula, native composite formula, or generic equation pack

## RED Targets

- Adding a `FractalType` enum value without a catalog row fails `test_fractal_catalog_authority` through catalog count and enum-id coverage.
- Adding a catalog row without selector/id coverage fails `test_fractal_catalog_authority` through enum-id reverse lookup and duplicate checks.
- Current default presets remain field-compatible through `test_fractal_types` and `test_fractal_derived_fields`.
- Existing explicit selector identity remains unchanged through the existing family-rule/schema rails.
- Current `explaino_all` registry/common-axis behavior is not changed through `test_fractal_family_rules`.
- Perturbation eligibility remains truthful: current support is Mandelbrot/Julia only, cross-checked against `SupportsPerturbationReferenceOrbit(...)`.

## Proof Ledger

RED and implementation proof:
- RED: `py -3.14 tools/viewer_host_run_logged_command.py --label fractal_catalog_red --log artifacts/logs/fractal_catalog_red.log --out-json artifacts/validation/fractal_catalog_red.json --heartbeat-seconds 30 --timeout-seconds 180 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_catalog_authority` failed on missing `../src/fractal_catalog.h`.
- GREEN: `py -3.14 tools/viewer_host_run_logged_command.py --label fractal_catalog_green --log artifacts/logs/fractal_catalog_green.log --out-json artifacts/validation/fractal_catalog_green.json --heartbeat-seconds 30 --timeout-seconds 180 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_catalog_authority` passed after adding the typed catalog.
- GREEN: `py -3.14 tools/viewer_host_run_logged_command.py --label fractal_catalog_capability_green --log artifacts/logs/fractal_catalog_capability_green.log --out-json artifacts/validation/fractal_catalog_capability_green.json --heartbeat-seconds 30 --timeout-seconds 180 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_catalog_authority` passed after adding explicit UI/capability flags.

Final focused rails run green in this slice:
- `py -3.14 tools/viewer_host_run_logged_command.py --label fractal_catalog_final_catalog_authority --log artifacts/logs/fractal_catalog_final_catalog_authority.log --out-json artifacts/validation/fractal_catalog_final_catalog_authority.json --heartbeat-seconds 30 --timeout-seconds 180 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_catalog_authority`
- `py -3.14 tools/viewer_host_run_logged_command.py --label fractal_catalog_final_types --log artifacts/logs/fractal_catalog_final_types.log --out-json artifacts/validation/fractal_catalog_final_types.json --heartbeat-seconds 30 --timeout-seconds 180 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_types`
- `py -3.14 tools/viewer_host_run_logged_command.py --label fractal_catalog_final_derived_fields --log artifacts/logs/fractal_catalog_final_derived_fields.log --out-json artifacts/validation/fractal_catalog_final_derived_fields.json --heartbeat-seconds 30 --timeout-seconds 180 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_derived_fields`
- `py -3.14 tools/viewer_host_run_logged_command.py --label fractal_catalog_final_family_rules --log artifacts/logs/fractal_catalog_final_family_rules.log --out-json artifacts/validation/fractal_catalog_final_family_rules.json --heartbeat-seconds 30 --timeout-seconds 180 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_family_rules`
- `py -3.14 tools/viewer_host_run_logged_command.py --label fractal_catalog_final_parameter_surface_descriptor --log artifacts/logs/fractal_catalog_final_parameter_surface_descriptor.log --out-json artifacts/validation/fractal_catalog_final_parameter_surface_descriptor.json --heartbeat-seconds 30 --timeout-seconds 240 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_parameter_surface_descriptor`
- `py -3.14 tools/viewer_host_run_logged_command.py --label fractal_catalog_final_schema_binding --log artifacts/logs/fractal_catalog_final_schema_binding.log --out-json artifacts/validation/fractal_catalog_final_schema_binding.json --heartbeat-seconds 30 --timeout-seconds 300 -- ui_app/build_tests_vsdevcmd.cmd test_schema_binding`
- `py -3.14 tools/viewer_host_run_logged_command.py --label fractal_catalog_final_descriptor_cli_pytest --log artifacts/logs/fractal_catalog_final_descriptor_cli_pytest.log --out-json artifacts/validation/fractal_catalog_final_descriptor_cli_pytest.json --heartbeat-seconds 30 --timeout-seconds 180 -- py -3.14 -m pytest tests/test_fractal_parameter_surface_descriptor_cli.py -q`
- `py -3.14 tools/viewer_host_run_logged_command.py --label fractal_catalog_full_native --log artifacts/logs/fractal_catalog_full_native.log --out-json artifacts/validation/fractal_catalog_full_native.json --heartbeat-seconds 30 --timeout-seconds 1800 -- ui_app/build_tests_vsdevcmd.cmd`

Validation and hygiene rails run green:
- `py -3.14 tools/viewer_host_run_logged_command.py --label fractal_catalog_contract_validation --log artifacts/logs/fractal_catalog_contract_validation.log --out-json artifacts/validation/fractal_catalog_contract_validation_command.json --heartbeat-seconds 30 --timeout-seconds 120 -- py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/fractal_catalog_authority_inventory.contract.json --out-json artifacts/validation/fractal_catalog_authority_inventory_contract.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label fractal_catalog_plan_sync_check --log artifacts/logs/fractal_catalog_plan_sync_check.log --out-json artifacts/validation/fractal_catalog_plan_sync_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_run_logged_command.py --label fractal_catalog_code_quality --log artifacts/logs/fractal_catalog_code_quality.log --out-json artifacts/validation/fractal_catalog_code_quality_command.json --heartbeat-seconds 30 --timeout-seconds 180 -- py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/fractal_catalog_authority_inventory_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label fractal_catalog_authority_inventory_diff_check --log artifacts/logs/fractal_catalog_authority_inventory_diff_check.log --out-json artifacts/validation/fractal_catalog_authority_inventory_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`

## Hostile Audit

- Status: complete
- Did I actually make new fractal additions simpler, or just create another table that must be manually duplicated elsewhere? Current answer: Slice A creates the fail-closed row authority; Slice B must still consume it from default-routing surfaces before the larger simplification campaign is complete.
- Did every current `FractalType` get exactly one truthful catalog row? Current answer: yes by focused native proof.
- Did selector identity, schema visibility, descriptor export, and defaults stay behavior-compatible? Current answer: final focused rails are green on the repaired state.
- Did the catalog fail closed when a row is missing or incomplete? Current answer: yes for enum/catalog/id/capability coverage in `test_fractal_catalog_authority`.
- Did I accidentally start Slice B, perturbation zoom, color tuning, camera/dive, or new fractal implementation? Current answer: no; this diff adds catalog/test/harness only.

## Audit Passes

- [done] Pass 1 - audit the initial catalog model for duplicated authority and missing fields.
- [done] Pass 2 - audit behavior-preservation proof against current default and selector surfaces.
- [done] Pass 3 - clean re-read the repaired state after audit findings were fixed.

## Audit Findings

- [fixed] Finding 1: The original contract named a multi-target focused native command, but `ui_app/build_tests_vsdevcmd.cmd` only consumed `%~1` and lacked focused dispatch entries for most named rails. Fixed by adding focused targets and revising the contract to list runnable rails explicitly.
- [fixed] Finding 2: The first catalog implementation had runtime flags but did not explicitly carry UI/capability metadata required by this plan. Fixed with `FractalCatalogCapabilityFlag`, `capability_flags`, and focused tests for sample/probe, schema/control-surface, animation applicability, smooth-escape coloring, Color Pipeline frame coloring, and generic-pack ownership.
- [fixed] Finding 3: The first hardened catalog duplicated the perturbation eligibility predicate locally. Fixed by consuming `SupportsPerturbationReferenceOrbit(...)` from the existing perturbation seam directly.
- [clean] Pass 3: Diff review and forbidden-scope grep found no renderer, FPS pacing, capture, Color Pipeline, new-fractal, or perturbation implementation drift in this slice.

## Notes

The target outcome is not abstraction for its own sake. The target is a fail-closed, low-friction path where a supported 2D fractal addition starts with formula implementation plus one catalog row, and tests identify the remaining required seams. Slice B still needs to consume this catalog from default-routing surfaces before the selector/default authority simplification is complete.
