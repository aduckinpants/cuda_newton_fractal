# Color Source Distinctness Smoke Matrix

## Current Phase

Phase 5 - checkpoint, receipts, rearward review, push, and clean-tree closeout.

## Phase Checklist

- [x] Phase 0 - bootstrap from clean pushed `700b66e` with rearward review `ok`.
- [x] Phase 1 - open this checked-in plan/contract and lock the active slice.
- [x] Phase 2 - add native source-signal matrix proof for shipped non-SDF Source row distinctness.
- [x] Phase 3 - add published-runtime no-mouse proof for the shipped non-SDF source-row path where supported.
- [x] Phase 4 - hostile review the matrix, prove no product semantics changed, and rerun focused rails.
- [x] Phase 5 - checkpoint, receipts, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [open] Use the away time to close test gaps instead of letting blatant source-row alias regressions survive.
- [open] Keep smoke/unit coverage honest for user-facing Color Pipeline behavior.
- [open] Do not use physical mouse automation.

## Scope

In scope:

- Test-only coverage for shipped non-SDF Color Pipeline Source rows that have distinct catalog identities.
- Native signal-level proof that source rows do not silently alias `smooth_escape_ramp`.
- Published-runtime no-mouse proof for the supported visible source-row selections.
- Hostile review of whether the new matrix is discriminating rather than cosmetic.

Out of scope:

- Changing source-row product semantics.
- SDF source behavior already covered by `sdf_source_distinctness_smoke_matrix`.
- Reworking Color Pipeline composition UX.
- Adding new source functions, palettes, or fractal types.
- Physical mouse automation.

## Proof Ledger

- Bootstrap carryover: previous slice closed cleanly on `codex/sdf-source-distinctness-smoke-matrix` at pushed `700b66e`.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `700b66e`.
- Branch: `codex/color-source-distinctness-smoke-matrix` created from `700b66e`.
- Slice open: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "Color source distinctness smoke matrix" --profile runtime --plan docs/notes/color_source_distinctness_smoke_matrix_PHASED_PLAN.md --contract docs/contracts/color_source_distinctness_smoke_matrix.contract.json` locked checkpoint token `ck:c412bbdb`.
- Contract bootstrap: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/color_source_distinctness_smoke_matrix.contract.json --out-json artifacts/validation/color_source_distinctness_contract_bootstrap.json` passed.
- Plan sync bootstrap: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed after opening the slice.
- Native implementation: `ui_app/tests/test_escape_time_coloring.cpp` now compares native source-signal signatures for `smooth_escape_ramp`, `phase_orbit`, `banded_signal`, `escape_magnitude`, `orbit_stripe`, `root_proximity`, and `root_index`.
- Runtime implementation: `tests/test_fractal_runtime_color_pipeline_presets.py` now captures supported visible non-SDF source/palette selections through no-mouse headless Color Pipeline actions and rejects aliases with `smooth_escape_ramp` plus duplicate hashes across the matrix.
- Native proof: `py -3.14 tools/viewer_host_run_logged_command.py --label color_source_distinctness_native --log artifacts/logs/color_source_distinctness_native.log --out-json artifacts/validation/color_source_distinctness_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core test_escape_time_coloring` passed.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label color_source_distinctness_runtime_publish --log artifacts/logs/color_source_distinctness_runtime_publish.log --out-json artifacts/validation/color_source_distinctness_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_vsdevcmd.cmd` passed and published `runtime/fractal_ui.exe`.
- Published runtime proof: `py -3.14 tools/viewer_host_run_logged_command.py --label color_source_distinctness_runtime_presets --log artifacts/logs/color_source_distinctness_runtime_presets.log --out-json artifacts/validation/color_source_distinctness_runtime_presets.json --heartbeat-seconds 30 --timeout-seconds 900 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_color_pipeline_presets.py` passed with 2 tests.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/color_source_distinctness_smoke_matrix.contract.json --out-json artifacts/validation/color_source_distinctness_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/color_source_distinctness_smoke_matrix_PHASED_PLAN.md --out-json artifacts/validation/color_source_distinctness_hostile_audit.json` passed.
- Code quality: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/color_source_distinctness_code_quality.json` passed the existing baseline.
- Diff check: `py -3.14 tools/viewer_host_run_logged_command.py --label color_source_distinctness_diff_check --log artifacts/logs/color_source_distinctness_diff_check.log --out-json artifacts/validation/color_source_distinctness_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.

## Hostile Audit

- Status: complete
- Does the matrix actually prove source distinctness, or only parse/catalog identity? It proves runtime signal/output distinctness. The native rail computes source signatures through `ResolveProgrammableEscapeTimeSignal(...)`; the runtime rail captures published viewer output after applying visible source/palette rows.
- Does runtime proof use the published viewer path and no-mouse actions? Yes. It runs through `viewer_host_runtime_pytest_lane.py`, the active published `runtime/fractal_ui.exe`, `--color-pipeline-action`, and capture diagnostics.
- Did this slice accidentally change Color Pipeline behavior? No. Product files are untouched; only tests, plan/contract, and handoff text changed.
- Are unsupported source/palette combinations kept fail-closed instead of forced through brittle tests? Yes. The runtime matrix uses only shipped supported pairings from the existing compatibility surface.

## Audit Passes

- [x] Pass 1 - review native matrix discrimination.
- [x] Pass 2 - review published-runtime proof scope and unsupported-combination handling.
- [x] Pass 3 - clean re-read the repaired state for product-behavior changes, physical mouse usage, and stale plan text; no additional real issue found.

## Audit Findings

- [x] Read-first finding: SDF source rows now have a raw-alias matrix, but non-SDF source rows still mostly rely on catalog/roundtrip and parameter-sensitivity rails rather than a direct source-distinctness matrix.
- [x] Real hostile-review finding: the first runtime draft only rejected aliases with `smooth_escape_ramp`, while the native proof was pairwise. The runtime matrix now also rejects duplicate frame hashes across all shipped non-SDF source selections.
- [x] Clean re-read: no product runtime code changed; the new runtime proof uses no physical mouse automation and avoids unsupported source/palette combinations.
