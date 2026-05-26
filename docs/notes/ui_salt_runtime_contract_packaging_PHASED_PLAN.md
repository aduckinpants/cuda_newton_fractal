# UI-Salt Runtime Contract Packaging

## Current Phase

Phase 6 - closed after runtime proof, hostile-review repair, validation, checkpoint, receipts, rearward review, push, and clean-tree closure.

## Phase Checklist

- [x] Phase 1 - create this checked-in phased plan and contract, then lock the active slice
- [x] Phase 2 - add RED tests for CLI parsing, runtime staging, and published-exe materialized-contract validation
- [x] Phase 3 - stage the materialized Color Pipeline contract beside the published runtime
- [x] Phase 4 - add a published-runtime validation verb that loads the staged JSON and proves parity against current hardcoded Color Pipeline behavior
- [x] Phase 5 - keep normal Color Pipeline UI behavior frozen; do not switch the visible UI catalog in this slice
- [x] Phase 6 - hostile review, validation, checkpoint, receipts, rearward review, push, and clean-tree closure

## Explicit User Asks

- [done] Continue the UI-Salt Explaino contract work with the next bounded slice.
- [done] Remove the previous blocker by giving the published viewer a safe generated-contract path.
- [done] Move toward one backend seam at a time without changing visible Color Pipeline behavior prematurely.
- [done] Do not add Salticid as a viewer runtime dependency.
- [done] Do not use physical mouse automation.

## Scope

In scope:

- Stage `docs/ui_salt/generated/color_pipeline_function_library.contract.v1.json` into the published runtime directory.
- Add a hidden/headless validation verb that loads the staged materialized contract from the published runtime and checks it against current C++ Color Pipeline catalog behavior.
- Add a JSON report for the validation verb so runtime pytest can prove the published executable consumed the materialized contract.
- Add focused native and published-runtime tests for parsing, staging, parity, and fail-closed behavior.

Out of scope:

- Switching the normal Color Pipeline UI catalog to metadata-backed descriptors.
- New Color Pipeline rows, recipes, or visible UI layout.
- Factorio-style schedule/workflow UI.
- Boundary-masked SDF normal-angle beauty mode.
- SDF operands/gates as first-class composition nodes.
- Generic Equation Pack viewport integration.
- Salticid `sample_fn` adapter.
- SDF-native fractal lanes.
- Runtime Salticid dependency inside this viewer.

## Authority Decision

The materialized JSON remains the viewer contract authority. This slice proves the published runtime can find and consume that JSON safely. The hardcoded Color Pipeline catalog remains the shipped UI authority until a later slice switches exactly one visible lookup seam behind the now-packaged metadata path.

## Proof Ledger

- Start authority: bootstrap and rearward review were clean on `codex/ui-salt-explaino-contract` before this slice opened; active contract is `ui_salt_runtime_contract_packaging`.
- RED: `ui_salt_runtime_packaging_viewer_cli_red` failed because the CLI had no UI-Salt contract validation fields or flags.
- GREEN native: `ui_salt_runtime_packaging_viewer_cli` passed with `213 passed, 0 failed`; `ui_salt_runtime_packaging_native` passed with `1579 passed, 0 failed`; `ui_salt_runtime_packaging_materializer_pytest` passed with `7 passed`.
- Runtime proof: `ui_salt_runtime_packaging_publish` republished `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe` after the final CLI refactor; `ui_salt_runtime_packaging_pytest` passed `1 passed`; `ui_salt_runtime_packaging_runtime_report.json` shows the published exe loaded `runtime\ui_salt\generated\color_pipeline_function_library.contract.v1.json` with `schema_version=1`, `lane_count=4`, `function_count=33`, `compatibility_count=20`, `unsupported_pair_count=52`, and `errors=[]`.
- Closure validators: contract validation, phased-plan sync, hostile-audit validation, code-quality baseline, and `git diff --check` all passed.

## Hostile Audit

- Status: done
- Required posture: assume the staged JSON is missing from the published runtime, the CLI verb only checks repo files, the parity proof silently falls back to hardcoded behavior, normal UI behavior changes accidentally, unsupported pairs become accepted, or Salticid leaks into the runtime.

## Audit Passes

- [done] Pass 1 - inspected runtime staging and path resolution; direct runtime report proves the exe consumed `D:\salt-fractal\cuda_newton_fractal_clone\runtime\ui_salt\generated\color_pipeline_function_library.contract.v1.json`, not a repo-relative fallback.
- [done] Pass 2 - inspected parity validation; reusable parity walks all materialized lanes/functions/params/source signal kinds and every Source x Palette pair, with 20 supported pairs and 52 unsupported pairs in the runtime report.
- [done] Pass 3 - inspected normal UI/runtime surfaces; metadata is only used by the hidden validation verb and native parity tests, with no visible Color Pipeline catalog switch in this slice.
- [done] Clean re-read the repaired state after the helper conflict fix; `test_viewer_cli` passed again with `213 passed, 0 failed` and no additional real defect found.
- [done] Clean re-read after the code-quality refactor; republish and runtime pytest passed again, and no additional real issue found.

## Audit Findings

- [done] Real finding: the first implementation let `ValidateViewerCliModeConflicts` omit the `--sample-session` conflict for `--validate-ui-salt-contract`; the dispatcher still rejected it, but the helper contract was weaker than the tested surface. Added `TestValidateUiSaltContractConflictsWithSampleSession` and fixed the helper.
- [done] Real finding: code-quality baseline caught `viewer_cli.cpp` max function length regression after adding UI-Salt CLI parsing. Fixed by extracting metadata/headless parsing into `TryParseMetadataHeadlessArgs` instead of raising the baseline.
- [done] Clean re-read evidence: after repair, the focused CLI rail passed, code-quality baseline passed, republish/runtime proof passed again, and `rg` found no live Color Pipeline UI lookup switched to metadata and no Salticid runtime dependency in `ui_app/src`.

## Notes

Validation targets:

- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_runtime_packaging_native --log artifacts/logs/ui_salt_runtime_packaging_native.log --out-json artifacts/validation/ui_salt_runtime_packaging_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_runtime_packaging_viewer_cli --log artifacts/logs/ui_salt_runtime_packaging_viewer_cli.log --out-json artifacts/validation/ui_salt_runtime_packaging_viewer_cli.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_viewer_cli`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_runtime_packaging_materializer_pytest --log artifacts/logs/ui_salt_runtime_packaging_materializer_pytest.log --out-json artifacts/validation/ui_salt_runtime_packaging_materializer_pytest.json --heartbeat-seconds 30 --timeout-seconds 600 -- py -3.14 -m pytest tests/test_ui_salt_materializer.py -q`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_runtime_packaging_publish --log artifacts/logs/ui_salt_runtime_packaging_publish.log --out-json artifacts/validation/ui_salt_runtime_packaging_publish.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_vsdevcmd.cmd`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_runtime_packaging_pytest --log artifacts/logs/ui_salt_runtime_packaging_pytest.log --out-json artifacts/validation/ui_salt_runtime_packaging_pytest.json --heartbeat-seconds 30 --timeout-seconds 600 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_ui_salt_contract.py`
- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_salt_runtime_contract_packaging.contract.json --out-json artifacts/validation/ui_salt_runtime_packaging_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/ui_salt_runtime_contract_packaging_PHASED_PLAN.md --out-json artifacts/validation/ui_salt_runtime_packaging_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/ui_salt_runtime_packaging_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_runtime_packaging_diff_check --log artifacts/logs/ui_salt_runtime_packaging_diff_check.log --out-json artifacts/validation/ui_salt_runtime_packaging_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`
