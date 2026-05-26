# UI-Salt Metadata Catalog Lookup

## Current Phase

Phase 6 - hostile audit, validation, checkpoint, receipts, rearward review, push, and clean-tree closure.

## Phase Checklist

- [x] Phase 0 - merge closed UI-Salt runtime packaging to `master` and create a fresh implementation branch
- [x] Phase 1 - create this checked-in phased plan and contract, then lock the active slice
- [x] Phase 2 - add RED tests for metadata-backed active catalog initialization and hardcoded fallback
- [x] Phase 3 - add the metadata-to-`ColorPipelineLaneCatalog` adapter and active catalog provider
- [x] Phase 4 - initialize the published viewer catalog from the staged materialized JSON before normal UI startup
- [x] Phase 5 - prove visible Color Pipeline behavior remains frozen and compatibility/recipe routing stays hardcoded
- [ ] Phase 6 - hostile review, validation, checkpoint, receipts, rearward review, push, and clean-tree closure

## Explicit User Asks

- [closed] Merge the closed UI-Salt branch up to `master`.
- [closed] Continue serious implementation on a fresh branch.
- [closed] Move the next UI-Salt backend seam forward without half-implementing or widening the feature.
- [closed] Keep the UI behavior frozen while switching one descriptor lookup seam.
- [closed] Do not add Salticid as a viewer runtime dependency.
- [closed] Do not use physical mouse automation.

## Scope

In scope:

- Add a metadata-backed active Color Pipeline catalog provider that converts the loaded materialized contract to `ColorPipelineLaneCatalog` / `FunctionDescriptor` values.
- Keep a hardcoded fallback catalog when no materialized contract has been loaded.
- Switch `GetColorPipelineLaneCatalogs()`, `FindColorPipelineLaneCatalog()`, and `FindColorPipelineFunctionDescriptor()` to read through that active catalog provider.
- Initialize the active catalog from the staged runtime contract during normal viewer startup, after the exe directory is known and before the Color Pipeline window uses catalog descriptors.
- Add native and runtime proof that metadata-backed catalogs match the previous hardcoded catalog exactly.

Out of scope:

- Compatibility/fail-closed routing switch.
- Companion suggestions switch.
- Recipe preset expansion switch.
- New Color Pipeline composition UI.
- Factorio-style schedule/workflow layout.
- Boundary-masked SDF normal-angle beauty mode.
- SDF operands/gates as first-class composition nodes.
- Generic Equation Pack viewport integration.
- Salticid `sample_fn` adapter.
- SDF-native fractal lanes.
- Runtime Salticid dependency inside this viewer.

## Authority Decision

The first live backend seam is descriptor catalog lookup only. The materialized JSON may own lane/function/parameter descriptor metadata after parity is proven, but compatibility routing and runtime row semantics remain hardcoded in this slice. If metadata loading fails in normal UI startup, the viewer must fall back to the current hardcoded catalog and report the failure through diagnostics/log output; it must not silently accept partial metadata or change visible behavior.

## Proof Ledger

- Start authority: `master` fast-forwarded to `2106fe8`, branch `codex/ui-salt-function-descriptor-lookup`, active contract `ui_salt_metadata_catalog_lookup`, checkpoint token `ck:1cdaf4c2`.
- RED: `artifacts/validation/ui_salt_metadata_catalog_native_red.json` failed because `ui_app/src/color_pipeline_metadata_catalog.h` did not exist, proving the public catalog could not yet be installed from materialized metadata.
- GREEN native:
  - `artifacts/validation/ui_salt_metadata_catalog_native.json` - `test_color_pipeline_core: passed=1838 failed=0`.
  - `artifacts/validation/ui_salt_metadata_catalog_window.json` - `test_color_pipeline_window: passed=182 failed=0`.
  - `artifacts/validation/ui_salt_metadata_catalog_materializer_pytest.json` - `7 passed`.
- Runtime proof:
  - `artifacts/validation/ui_salt_metadata_catalog_publish.json` - published `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
  - `artifacts/validation/ui_salt_metadata_catalog_pytest.json` - `viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_ui_salt_contract.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py`, `5 passed`, reports `catalog_authority == materialized_json`.

## Hostile Audit

- Status: complete
- Required posture: assume metadata-backed descriptors omit parameters, reorder functions, change labels/tooltips/defaults, accidentally switch compatibility routing, break SDF source rows, add a runtime Salticid dependency, or make the viewer unusable when staged JSON is missing.

## Audit Passes

- [done] Pass 1 - inspected metadata-to-catalog conversion for parameter/default/range/option loss.
- [done] Pass 2 - inspected active catalog initialization and fallback for partial-load or repo-relative path mistakes.
- [done] Pass 3 - inspected normal UI/runtime surfaces for accidental compatibility, recipe, or visible behavior changes.

## Audit Findings

- [done] The materialized contract does not carry parameter help text. A direct descriptor switch would have removed visible per-param UI help text. The adapter now requires materialized descriptor parity but preserves the hardcoded help text and enum option labels, and `TestMaterializedUiSaltMetadataCanOwnPublicCatalog` locks that behavior.
- [done] Active catalog lane fields are `const char*` backed by string storage. The first implementation relied on vector move behavior for pointer lifetime. The install path now refreshes the published lane pointers after storage publication.
- [done] Early proof used equivalent-but-differently-named artifacts for some rails. The exact contract-labeled native/materializer/publish/runtime artifacts were regenerated before closure.
- [done] Clean re-read the repaired state after the help-text, pointer-refresh, and evidence-label fixes; no additional real defect found in compatibility routing, recipe expansion, UI layout, SDF rows, or Salticid runtime dependency boundaries.

## Notes

Validation targets:

- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_metadata_catalog_native --log artifacts/logs/ui_salt_metadata_catalog_native.log --out-json artifacts/validation/ui_salt_metadata_catalog_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_metadata_catalog_window --log artifacts/logs/ui_salt_metadata_catalog_window.log --out-json artifacts/validation/ui_salt_metadata_catalog_window.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_window`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_metadata_catalog_materializer_pytest --log artifacts/logs/ui_salt_metadata_catalog_materializer_pytest.log --out-json artifacts/validation/ui_salt_metadata_catalog_materializer_pytest.json --heartbeat-seconds 30 --timeout-seconds 600 -- py -3.14 -m pytest tests/test_ui_salt_materializer.py -q`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_metadata_catalog_publish --log artifacts/logs/ui_salt_metadata_catalog_publish.log --out-json artifacts/validation/ui_salt_metadata_catalog_publish.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_vsdevcmd.cmd`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_metadata_catalog_pytest --log artifacts/logs/ui_salt_metadata_catalog_pytest.log --out-json artifacts/validation/ui_salt_metadata_catalog_pytest.json --heartbeat-seconds 30 --timeout-seconds 900 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_ui_salt_contract.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py`
- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_salt_metadata_catalog_lookup.contract.json --out-json artifacts/validation/ui_salt_metadata_catalog_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/ui_salt_metadata_catalog_lookup_PHASED_PLAN.md --out-json artifacts/validation/ui_salt_metadata_catalog_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/ui_salt_metadata_catalog_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_metadata_catalog_diff_check --log artifacts/logs/ui_salt_metadata_catalog_diff_check.log --out-json artifacts/validation/ui_salt_metadata_catalog_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`
