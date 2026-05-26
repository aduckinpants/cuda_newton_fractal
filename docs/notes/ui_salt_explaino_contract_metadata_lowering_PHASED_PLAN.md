# UI-Salt Explaino Contract Metadata Lowering

## Current Phase

Complete - the behavior-preserving UI-Salt shadow metadata slice is implemented and validated. The first live backend lookup switch remains explicitly deferred until the generated JSON has a safe published-runtime embedding or staging path.

## Phase Checklist

- [x] Phase 0 - merge the closed SDF branch into `master` and create a fresh implementation branch
- [x] Phase 1 - create this checked-in phased plan and contract, then lock the active slice
- [x] Phase 2 - add RED tests for materialized UI-Salt contracts, strict validation, and current Color Pipeline catalog parity
- [x] Phase 3 - implement the minimal viewer-local UI-Salt materializer and generated JSON contract fixtures
- [x] Phase 4 - add a C++ metadata loader/shadow parity seam without switching shipped runtime authority
- [deferred] Phase 5 - switch the first backend lookup seam only after parity is green; this checkpoint proves parity and defers live seam switching until the generated JSON is embedded or staged beside the published runtime instead of depending on repo-relative `docs/` paths
- [x] Phase 6 - hostile review, validation, and checkpoint closeout handoff for this behavior-preserving shadow metadata slice

## Explicit User Asks

- [done] Build the first behavior-preserving compiler-style layer for the viewer function library and Color Pipeline composition system as a shadow metadata authority.
- [done] Use Salticid-inspired `ui.salt` authoring, recipe materialization, typed metadata, and an Explaino-shaped contract without running Salticid inside the viewer.
- [done] Freeze current UI behavior while generated metadata proves parity with the hardcoded implementation.
- [deferred] Move backend lookup seams one at a time only after parity is proven; first switch is blocked on a safe generated-contract packaging/embedding decision for the published viewer.
- [done] Keep feature expansion deferred until the contract layer is stable.

## Scope

In scope:

- Add viewer-local `ui.salt` authoring inputs and deterministic materialized JSON contracts.
- Add strict materializer validation for function-library, composition-recipe, and Explaino contract metadata.
- Add shadow parity tests against the existing Color Pipeline catalog, signal-kind classification, compatibility routing, and fail-closed diagnostics.
- Add a C++ metadata loader/shadow comparison seam that keeps current hardcoded behavior authoritative until parity is green.
- Optionally switch the first low-risk lookup seam after the parity matrix proves exact equivalence.

Out of scope:

- New Color Pipeline composition UI.
- Factorio-style schedule/workflow layout.
- Boundary-masked SDF normal-angle beauty mode.
- SDF operands/gates as first-class composition nodes.
- Generic Equation Pack viewport integration.
- Salticid `sample_fn` adapter.
- SDF-native fractal lanes.
- Runtime Salticid dependency inside this viewer.

## Authority Decision

`ui.salt` is an authoring surface only. The viewer authority for this slice is strict materialized JSON plus C++ shadow parity against the existing hardcoded Color Pipeline catalog. C++ must not parse raw `.ui.salt`, and the shipped UI must not change until generated metadata proves parity with the current implementation.

## Proof Ledger

- Start authority: branch `codex/ui-salt-explaino-contract`, clean head `07773fd`, rearward review `status=ok`.
- Preflight: `codex/color-pipeline-sdf-source-rows` was clean, pushed, and fast-forwarded into `master` before this branch was created, so the new slice is not hidden as another SDF-branch commit.
- RED: `tests/test_ui_salt_materializer.py` initially failed because `tools/viewer_host_materialize_ui_salt.py` did not exist; after the first generator pass, freshness proof failed because absolute input paths changed `source_path`.
- GREEN: `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_contract_materializer_pytest --log artifacts/logs/ui_salt_contract_materializer_pytest.log --out-json artifacts/validation/ui_salt_contract_materializer_pytest.json --heartbeat-seconds 30 --timeout-seconds 600 -- py -3.14 -m pytest tests/test_ui_salt_materializer.py -q` passed `7 passed`.
- GREEN: `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_contract_native --log artifacts/logs/ui_salt_contract_native.log --out-json artifacts/validation/ui_salt_contract_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core` passed with `test_color_pipeline_core: passed=1577 failed=0`.
- Runtime proof: `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_contract_runtime_publish --log artifacts/logs/ui_salt_contract_runtime_publish.log --out-json artifacts/validation/ui_salt_contract_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_vsdevcmd.cmd` published `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Runtime proof: `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_contract_runtime_pytest --log artifacts/logs/ui_salt_contract_runtime_pytest.log --out-json artifacts/validation/ui_salt_contract_runtime_pytest.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py tests/test_fractal_runtime_sdf_phase_signal_semantics.py` passed `5 passed`.
- Final runtime publish proof used the exact contracted `--timeout-seconds 600` command and republished `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Final validators: contract validation `ok=true`; phased-plan sync `OK`; hostile audit validation `ok=true`; code-quality baseline passed with score `94/100`; diff check passed.

## Hostile Audit

- Status: complete
- Required posture: assume the metadata lies, the materialized JSON drifts from current C++ behavior, the UI changes accidentally, unsupported recipes become silently accepted, Explaino claims lack proof authority, or a Salticid runtime dependency leaks into the viewer.

## Audit Passes

- [done] Pass 1 - generated metadata is compared against the current hardcoded catalog, parameter defaults/ranges/options, source signal-kind classification, recipes, Explaino proof entries, and every Source/Palette compatibility allow/deny pair.
- [done] Pass 2 - C++ loader/shadow seam was inspected for silent trust in generated JSON; it now rejects duplicate lanes/functions/params, dangling compatibility references, invalid defaults, invalid ranges, duplicate recipes, and duplicate Explaino entries.
- [done] Pass 3 - re-read the repaired state and found no additional real defect: runtime/UI behavior stayed frozen, no Color Pipeline UI path was switched to metadata in this checkpoint, runtime publish succeeded, and the focused published SDF Color Pipeline runtime proofs stayed green.

## Audit Findings

- [done] Real defect found: materializer freshness was path-sensitive because absolute `--ui-salt` inputs changed `source_path`; fixed by storing repo-relative paths when possible and adding checked-in generated JSON freshness proof.
- [done] Real defect found: the first C++ loader trusted materialized JSON more than the Python generator did; fixed by adding fail-closed validation and native tamper regressions for duplicate function ids and dangling compatibility rows.
- [deferred] First live backend lookup switch is not in this checkpoint. Runtime publish currently stages schema and launcher metadata beside the exe, not `docs/ui_salt/generated`; switching shipped UI lookup before embedding/staging the generated contract would create a file-location regression risk.

## Notes

Validation targets:

- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_contract_native --log artifacts/logs/ui_salt_contract_native.log --out-json artifacts/validation/ui_salt_contract_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_contract_materializer_pytest --log artifacts/logs/ui_salt_contract_materializer_pytest.log --out-json artifacts/validation/ui_salt_contract_materializer_pytest.json --heartbeat-seconds 30 --timeout-seconds 600 -- py -3.14 -m pytest tests/test_ui_salt_materializer.py -q`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_contract_runtime_publish --log artifacts/logs/ui_salt_contract_runtime_publish.log --out-json artifacts/validation/ui_salt_contract_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_vsdevcmd.cmd`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_contract_runtime_pytest --log artifacts/logs/ui_salt_contract_runtime_pytest.log --out-json artifacts/validation/ui_salt_contract_runtime_pytest.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py tests/test_fractal_runtime_sdf_phase_signal_semantics.py`
- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_salt_explaino_contract_metadata_lowering.contract.json --out-json artifacts/validation/ui_salt_contract_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/ui_salt_explaino_contract_metadata_lowering_PHASED_PLAN.md --out-json artifacts/validation/ui_salt_contract_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/ui_salt_contract_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_contract_diff_check --log artifacts/logs/ui_salt_contract_diff_check.log --out-json artifacts/validation/ui_salt_contract_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`
