# UI-Salt Recipe Expansion Lookup

## Current Phase

Complete - hostile review, validation, checkpoint, receipts, rearward review, push, and merge-back landed.

## Phase Checklist

- [x] Phase 0 - branch from clean pushed `master` after companion lookup closed
- [x] Phase 1 - create this checked-in phased plan and contract, then lock the active slice
- [x] Phase 2 - add RED tests proving Color Pipeline recipe expansion is not metadata-owned yet
- [x] Phase 3 - add metadata-derived recipe expansion authority with hardcoded fallback/reference
- [x] Phase 4 - initialize the published viewer recipe provider from the staged materialized JSON
- [x] Phase 5 - prove UI behavior stays frozen, unsupported recipes fail closed, and SDF rows still replay correctly
- [x] Phase 6 - hostile review, validation, checkpoint, receipts, rearward review, push, and merge-back

## Explicit User Asks

- [x] Continue the UI-Salt implementation sequence after companion lookup.
- [x] Take the next seam seriously and avoid half-implementation.
- [x] Keep the UI frozen while moving backend authority one seam at a time.
- [x] Treat current Color Pipeline annoyances as likely in this area, but do not split into unrelated UI redesign.
- [x] Do not use physical mouse automation.
- [x] Do not add a Salticid runtime dependency to the viewer.

## Scope

In scope:

- Add a metadata-derived Color Pipeline recipe expansion provider using the already staged `viewer.composition_recipe_contract.v1` recipe rows.
- Keep the current three recipe presets as the hardcoded fallback/reference: `default_smooth_escape`, `phase_orbit_wheel`, and `sdf_normal_angle_diagnostic`.
- Switch the current recipe preset lookup/expansion seam behind the active provider after parity is proven.
- Preserve current visible UI layout and control ids; only bounded status/report text may change when needed to make authority truthful.
- Add native and runtime proof that recipes expand to the same Source, Shape, Palette, and Grading rows as before.

Out of scope:

- New Color Pipeline composition UI.
- New visible Color Pipeline rows or functions.
- Factorio-style schedule/workflow layout.
- Boundary-masked SDF normal-angle beauty mode.
- SDF operands or gates as first-class composition nodes.
- Generic Equation Pack viewport integration.
- Salticid `sample_fn` adapter.
- SDF-native fractal lanes.
- Runtime Salticid dependency inside this viewer.

## Authority Decision

The fourth live backend seam is recipe preset expansion only. Catalog lookup, compatibility lookup, and companion suggestions are already metadata-backed with hardcoded fallback. This slice may let the materialized recipe table own recipe lookup/expansion after parity is proven. It must not change runtime enum execution, function descriptors, row layout, or source-stack semantics. If metadata loading fails, the viewer keeps the same hardcoded recipe presets.

## Proof Ledger

- Start authority: `master` at `b853cac`, clean, pushed, rearward review `ok`; branch `codex/ui-salt-recipe-expansion`; active contract `ui_salt_recipe_expansion`; checkpoint token `ck:b54a5bb3`.
- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8`, `py -3.14 tools/viewer_host_repo_status.py`, and `py -3.14 tools/viewer_host_rearward_review.py` passed before this plan was created.
- RED: `artifacts/validation/ui_salt_recipe_native_red.json` failed before implementation because the recipe authority API, active recipe lookup, and recipe lane expansion seam did not exist yet.
- GREEN native: `artifacts/validation/ui_salt_recipe_native.json` passed with `test_color_pipeline_core: passed=2093 failed=0`.
- Window preservation: `artifacts/validation/ui_salt_recipe_window.json` passed with `test_color_pipeline_window: passed=194 failed=0`.
- Materializer preservation: `artifacts/validation/ui_salt_recipe_materializer_pytest.json` passed with `7 passed`.
- Runtime publish: `artifacts/validation/ui_salt_recipe_publish.json` passed and staged `D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe`.
- Runtime proof: `artifacts/validation/ui_salt_recipe_pytest.json` passed with `5 passed`, covering UI-Salt contract report recipe authority/count and Color Pipeline SDF row preservation.
- Contract validation: `artifacts/validation/ui_salt_recipe_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile audit validator: `artifacts/validation/ui_salt_recipe_hostile_audit.json` passed.
- Code quality baseline: `artifacts/validation/ui_salt_recipe_code_quality.json` passed.
- Diff check: `artifacts/validation/ui_salt_recipe_diff_check.json` passed.
- Hostile-review defect repaired: runtime/native proof originally asserted active recipe authority but did not assert the materialized `recipe_count`; fixed by asserting `recipe_count == 3` in native parity and the published runtime report test.

## Hostile Audit

- Status: complete
- Required posture: assume metadata-derived recipes change current recipe rows, silently permit unsupported combinations, add new visible UI, bypass existing draft/live bridge validation, regress SDF source-stack replay, or accidentally import Salticid runtime behavior.

## Audit Passes

- [x] Pass 1 - inspected recipe provider conversion for id, label, lane order, and hardcoded parity drift; native parity now checks all three recipe rows exactly.
- [x] Pass 2 - inspected startup initialization, fallback behavior, and runtime reporting; found active recipe count was reported without asserting materialized `recipe_count`.
- [x] Pass 3 - clean re-read confirmed the window recipe helper uses the existing draft row selection/apply bridge, unknown recipe ids fail closed, no new visible rows were added, and no Salticid runtime dependency was introduced.

## Audit Findings

- [x] RED finding: current recipe expansion authority was not metadata-owned and had no provider/report proof; fixed by adding active recipe lookup, hardcoded fallback/reference, lane expansion, and report fields.
- [x] Real finding: runtime/native proof initially checked active recipe count but not the materialized `recipe_count`; fixed by asserting `recipe_count == 3` in core parity and the published runtime report test.
- [x] Clean re-read confirmed recipe expansion stayed bounded to metadata lookup and draft expansion, with Color Pipeline UI layout and SDF source behavior preserved.

## Notes

Validation targets:

- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_recipe_native --log artifacts/logs/ui_salt_recipe_native.log --out-json artifacts/validation/ui_salt_recipe_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_recipe_window --log artifacts/logs/ui_salt_recipe_window.log --out-json artifacts/validation/ui_salt_recipe_window.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_window`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_recipe_materializer_pytest --log artifacts/logs/ui_salt_recipe_materializer_pytest.log --out-json artifacts/validation/ui_salt_recipe_materializer_pytest.json --heartbeat-seconds 30 --timeout-seconds 600 -- py -3.14 -m pytest tests/test_ui_salt_materializer.py -q`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_recipe_publish --log artifacts/logs/ui_salt_recipe_publish.log --out-json artifacts/validation/ui_salt_recipe_publish.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_vsdevcmd.cmd`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_recipe_pytest --log artifacts/logs/ui_salt_recipe_pytest.log --out-json artifacts/validation/ui_salt_recipe_pytest.json --heartbeat-seconds 30 --timeout-seconds 900 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_ui_salt_contract.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py`
- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_salt_recipe_expansion.contract.json --out-json artifacts/validation/ui_salt_recipe_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/ui_salt_recipe_expansion_PHASED_PLAN.md --out-json artifacts/validation/ui_salt_recipe_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/ui_salt_recipe_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_recipe_diff_check --log artifacts/logs/ui_salt_recipe_diff_check.log --out-json artifacts/validation/ui_salt_recipe_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`
