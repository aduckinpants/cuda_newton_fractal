# UI-Salt Compatibility Lookup

## Current Phase

Complete - hostile review, validation, checkpoint, receipts, rearward review, push, and merge-back landed.

## Phase Checklist

- [x] Phase 0 - branch from clean pushed `master` after descriptor catalog lookup closed
- [x] Phase 1 - create this checked-in phased plan and contract, then lock the active slice
- [x] Phase 2 - add RED tests proving compatibility/fail-closed lookup is still hardcoded-only
- [x] Phase 3 - add metadata-backed compatibility authority with hardcoded fallback
- [x] Phase 4 - initialize the published viewer compatibility table from the staged materialized JSON
- [x] Phase 5 - prove UI behavior stays frozen, unsupported pairs stay denied, and SDF rows still fail closed correctly
- [x] Phase 6 - hostile review, validation, checkpoint, receipts, rearward review, push, and merge-back

## Explicit User Asks

- [x] Continue the UI-Salt implementation sequence after descriptor catalog lookup.
- [x] Take the work as far forward as possible without half-implementing.
- [x] Treat current Color Pipeline annoyances as likely in this work area, but do not split into an unrelated side repair unless the compatibility seam exposes a bounded regression.
- [x] Keep the UI frozen while moving backend authority one seam at a time.
- [x] Do not use physical mouse automation.
- [x] Do not add a Salticid runtime dependency to the viewer.

## Scope

In scope:

- Add a metadata-backed Color Pipeline compatibility lookup provider using the already staged `viewer.composition_recipe_contract.v1` compatibility rows.
- Keep hardcoded compatibility as fallback/reference when no materialized contract has been installed.
- Switch the existing Source + Palette compatibility allow/deny path behind the active compatibility provider.
- Preserve the current runtime selection tuple mapping and fail-closed behavior exactly.
- Add native and runtime proof that unsupported Source/Palette pairs stay denied and supported pairs produce the same `ColorPipelineSelection`.

Out of scope:

- Recipe preset expansion switch.
- Companion suggestion switch.
- New Color Pipeline composition UI.
- Factorio-style schedule/workflow layout.
- Boundary-masked SDF normal-angle beauty mode.
- SDF operands/gates as first-class composition nodes.
- Generic Equation Pack viewport integration.
- Salticid `sample_fn` adapter.
- SDF-native fractal lanes.
- Runtime Salticid dependency inside this viewer.

## Authority Decision

The second live backend seam is compatibility lookup only. The materialized JSON may own Source/Palette allow/deny and fail-closed reason metadata after parity is proven. Runtime enum execution and recipe expansion remain hardcoded in this slice. If metadata loading fails, the viewer retains the same hardcoded compatibility behavior and reports the fallback; it must not allow an unsupported pair or silently project to a different formula.

## Proof Ledger

- Start authority: `master` at `8dac656`, clean, pushed, rearward review `ok`; branch `codex/ui-salt-compatibility-lookup`; active contract `ui_salt_compatibility_lookup`; checkpoint token `ck:843cf33f`.
- RED: `artifacts/validation/ui_salt_compatibility_native_red.json` failed before implementation because the compatibility authority API and hardcoded-reference seam did not exist yet.
- GREEN native: `artifacts/validation/ui_salt_compatibility_native.json` passed with `test_color_pipeline_core: passed=2002 failed=0` after the hostile-review fallback-count repair.
- Window preservation: `artifacts/validation/ui_salt_compatibility_window.json` passed with `test_color_pipeline_window: passed=182 failed=0`.
- Materializer preservation: `artifacts/validation/ui_salt_compatibility_materializer_pytest.json` passed with `7 passed`.
- Runtime publish: `artifacts/validation/ui_salt_compatibility_publish.json` passed and staged `D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe`.
- Runtime proof: `artifacts/validation/ui_salt_compatibility_pytest.json` passed with `5 passed`, covering UI-Salt contract reporting and Color Pipeline SDF row preservation.
- Hostile-review defect repaired: fallback reporting now returns the hardcoded compatibility count instead of `0`, and the native test locks that truth surface.

## Hostile Audit

- Status: complete
- Required posture: assume metadata-backed compatibility permits unsupported SDF/non-SDF mixes, drops supported rows, changes default grading/palette tuples, hides fail-closed reasons, accidentally switches recipe expansion, or regresses the descriptor catalog installed by the previous slice.

## Audit Passes

- [x] Pass 1 - inspected compatibility provider conversion for allow/deny and tuple drift; found fallback reporting counted `0` active compatibility rows even though hardcoded fallback still owned a full table.
- [x] Pass 2 - clean re-read of the repaired fallback and startup initialization confirmed fallback count now equals the hardcoded table count, while metadata activation still reports the materialized count.
- [x] Pass 3 - no additional real defect found in the UI/runtime diff: recipe expansion, companion suggestions, visible layout, SDF row behavior, and descriptor catalog authority stayed outside this slice.

## Audit Findings

- [x] Real finding: `CountActiveColorPipelineCompatibilityRows()` was truthful only for materialized metadata and reported `0` for hardcoded fallback; fixed to report the hardcoded compatibility count and added a native fallback assertion.
- [x] Real finding: the first implementation compile caught a missing forward declaration for `CountHardcodedColorPipelineCompatibilityRows()`; fixed before any green claim.
- [x] Clean re-read after repair confirmed unsupported SDF pairs remain denied, supported pairs preserve the exact hardcoded tuple, and the published runtime reports `compatibility_authority = materialized_json` with `active_compatibility_count = 20`.

## Notes

Validation targets:

- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_compatibility_native --log artifacts/logs/ui_salt_compatibility_native.log --out-json artifacts/validation/ui_salt_compatibility_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_compatibility_window --log artifacts/logs/ui_salt_compatibility_window.log --out-json artifacts/validation/ui_salt_compatibility_window.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_window`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_compatibility_materializer_pytest --log artifacts/logs/ui_salt_compatibility_materializer_pytest.log --out-json artifacts/validation/ui_salt_compatibility_materializer_pytest.json --heartbeat-seconds 30 --timeout-seconds 600 -- py -3.14 -m pytest tests/test_ui_salt_materializer.py -q`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_compatibility_publish --log artifacts/logs/ui_salt_compatibility_publish.log --out-json artifacts/validation/ui_salt_compatibility_publish.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_vsdevcmd.cmd`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_compatibility_pytest --log artifacts/logs/ui_salt_compatibility_pytest.log --out-json artifacts/validation/ui_salt_compatibility_pytest.json --heartbeat-seconds 30 --timeout-seconds 900 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_ui_salt_contract.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py`
- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_salt_compatibility_lookup.contract.json --out-json artifacts/validation/ui_salt_compatibility_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/ui_salt_compatibility_lookup_PHASED_PLAN.md --out-json artifacts/validation/ui_salt_compatibility_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/ui_salt_compatibility_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_compatibility_diff_check --log artifacts/logs/ui_salt_compatibility_diff_check.log --out-json artifacts/validation/ui_salt_compatibility_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`
