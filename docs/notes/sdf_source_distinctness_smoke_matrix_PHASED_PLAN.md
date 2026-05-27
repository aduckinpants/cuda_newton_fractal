# SDF Source Distinctness Smoke Matrix

## Current Phase

Phase 5 - complete for this checkpoint; closure artifacts are tracked by validation, receipt, and rearward-review outputs.

## Phase Checklist

- [x] Phase 0 - bootstrap from clean `7daa171` with rearward review `ok`.
- [x] Phase 1 - open this checked-in plan/contract and lock the active slice.
- [x] Phase 2 - add native matrix proof that distinct SDF source rows do not alias raw signed distance under identical source scale/bias.
- [x] Phase 3 - add published-runtime no-mouse matrix proof for the same class of visible SDF source alias regressions.
- [x] Phase 4 - hostile review the test authority, prove no product semantics changed, and rerun focused rails.
- [x] Phase 5 - checkpoint, receipts, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [closed] Close the unit/smoke test gap that let Lens Field v2 visually alias normal SDF distance.
- [closed] Use the away time on real hardening instead of unrelated feature churn.
- [closed] Preserve no-mouse runtime proof and avoid physical cursor automation.

## Scope

In scope:

- Test-only coverage for SDF source rows that have distinct catalog IDs but could accidentally share runtime semantics.
- Native postprocess matrix proof for source output distinctness.
- Published-runtime no-mouse matrix proof for visible SDF source rows.
- Hostile review of whether the matrix would have caught the Lens Field v2 alias bug.

Out of scope:

- Changing Color Pipeline source semantics.
- Adding new SDF sources, palettes, or UI features.
- Per-source downsample, multi-field composition, authored SDF UI, SDF-native lanes, or Color Pipeline redesign.
- Physical mouse automation.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on clean `codex/lens-field-v2-response-repair` at `7daa171`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported a clean tree.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `7daa171`.
- Branch: `codex/sdf-source-distinctness-smoke-matrix` created from `7daa171`.
- Slice open: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "SDF source distinctness smoke matrix" --profile runtime --plan docs/notes/sdf_source_distinctness_smoke_matrix_PHASED_PLAN.md --contract docs/contracts/sdf_source_distinctness_smoke_matrix.contract.json` locked checkpoint token `ck:eab3ef21`.
- Contract bootstrap: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_source_distinctness_smoke_matrix.contract.json --out-json artifacts/validation/sdf_source_distinctness_contract_bootstrap.json` passed.
- Plan sync bootstrap: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed after opening the slice.
- Native proof: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_source_distinctness_native --log artifacts/logs/sdf_source_distinctness_native.log --out-json artifacts/validation/sdf_source_distinctness_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core test_color_pipeline_sdf_postprocess test_color_pipeline_sdf_postprocess_cuda` passed. This includes the native common-palette matrix for `sdf_inside_outside`, `sdf_boundary_band`, `sdf_normal_angle`, `sdf_curvature`, and `lens_field_v2_distance` against raw `sdf_signed_distance`.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_source_distinctness_runtime_publish --log artifacts/logs/sdf_source_distinctness_runtime_publish.log --out-json artifacts/validation/sdf_source_distinctness_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_vsdevcmd.cmd` passed and published `runtime/fractal_ui.exe`.
- Published runtime proof: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_source_distinctness_runtime_rows --log artifacts/logs/sdf_source_distinctness_runtime_rows.log --out-json artifacts/validation/sdf_source_distinctness_runtime_rows.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py` passed with 9 tests. This includes the no-mouse scalar SDF row matrix for `sdf_inside_outside`, `sdf_boundary_band`, `sdf_curvature`, and `lens_field_v2_distance` against raw `sdf_signed_distance` using the visible shipped source-row path.

## Hostile Audit

- Status: complete
- Would this matrix have failed on the pre-repair Lens Field v2 alias? Yes. Both the native common-palette matrix and the published runtime scalar matrix compare `lens_field_v2_distance` to raw `sdf_signed_distance` at scale `1.0` and bias `0.0`.
- Does it prove visible runtime behavior rather than helper-only behavior? Yes. The runtime rail uses `viewer_host_runtime_pytest_lane.py` against the published `fractal_ui.exe` and `--color-pipeline-action` no-mouse source-row edits.
- Does it avoid requiring unrelated sources to differ when their semantics are intentionally equivalent? Yes. The native matrix names only SDF sources that should be distinct from raw distance. The runtime matrix is narrower and skips phase `sdf_normal_angle` because the shipped visible path uses a different companion palette; normal angle distinctness is covered by the native common-palette matrix instead.
- Did I avoid changing product behavior while hardening tests? Yes. Only test files, plan/contract, and handoff text changed.
- Did I avoid physical mouse automation? Yes. The runtime proof uses headless/no-mouse CLI actions.

## Audit Passes

- [x] Pass 1 - native matrix review found the original coverage gap: tests proved individual SDF rows changed frames, but did not systematically reject raw signed-distance aliasing for each distinct SDF source.
- [x] Pass 2 - runtime no-mouse matrix review found a scope nuance: phase `sdf_normal_angle` should not be forced into the scalar shipped-palette runtime matrix; native common-palette proof covers it while runtime covers scalar shipped rows.
- [x] Pass 3 - clean re-read found no product behavior changes, no physical mouse usage, and no broad SDF/Color Pipeline feature work bundled into this hardening slice.

## Audit Findings

- [x] Real finding: the prior tests had a class-level gap. A source row could be catalog-visible, live-backed, and slider-sensitive while still aliasing raw `sdf_signed_distance`; the old rails only caught Lens v2 after a bespoke test was added.
- [x] Real finding: a single runtime matrix over every SDF source would overstate coverage for phase `sdf_normal_angle` because the shipped UI uses `phase_wheel`; the slice splits authority correctly into native common-palette coverage plus runtime scalar-row coverage.
- [x] Clean re-read: this slice is test-only, keeps no-mouse automation, and would have failed the Lens Field v2 raw-alias regression before the previous repair.
