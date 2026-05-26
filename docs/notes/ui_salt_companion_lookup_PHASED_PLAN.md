# UI-Salt Companion Lookup

## Current Phase

Complete - hostile review, validation, checkpoint, receipts, rearward review, push, and merge-back landed.

## Phase Checklist

- [x] Phase 0 - branch from clean pushed `master` after compatibility lookup closed
- [x] Phase 1 - create this checked-in phased plan and contract, then lock the active slice
- [x] Phase 2 - add RED tests proving Source/Palette companion auto-switch is still hardcoded-only
- [x] Phase 3 - add metadata-derived companion suggestion authority with hardcoded fallback
- [x] Phase 4 - initialize the published viewer companion suggestions from the staged materialized JSON
- [x] Phase 5 - prove UI behavior stays frozen, unsupported pairs stay denied, and SDF rows still fail closed correctly
- [x] Phase 6 - hostile review, validation, checkpoint, receipts, rearward review, push, and merge-back

## Explicit User Asks

- [x] Continue the UI-Salt implementation sequence after compatibility lookup.
- [x] Take the work as far forward as possible without half-implementing.
- [x] Treat Color Pipeline annoyances as likely in this work area, but do not split into unrelated side repairs.
- [x] Keep the UI frozen while moving backend authority one seam at a time.
- [x] Do not use physical mouse automation.
- [x] Do not add a Salticid runtime dependency to the viewer.

## Scope

In scope:

- Add a metadata-derived companion suggestion provider using the already staged `viewer.composition_recipe_contract.v1` compatibility row order.
- Keep hardcoded companion suggestions as fallback/reference when no materialized contract has been installed.
- Switch the existing single-row Source/Palette companion auto-switch path behind the active companion provider.
- Preserve the current companion choices exactly: scalar/SDF scalar sources choose `heatmap`, phase sources choose `phase_wheel_palette`, banded chooses `banded_heatmap`, root basin chooses `root_classic_palette`, and palette-side selections choose their current source companions.
- Add native and runtime proof that companion auto-switch behavior stays unchanged and remains fail-closed.

Out of scope:

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

The third live backend seam is companion suggestion lookup only. The materialized compatibility table may own the Source/Palette companion choice after parity is proven. Compatibility allow/deny already routes through metadata from the previous slice. Recipe expansion and runtime enum execution remain unchanged in this slice. If metadata loading fails, the viewer retains the same hardcoded companion auto-switch behavior and reports the fallback.

## Proof Ledger

- Start authority: `master` at `f068fc0`, clean, pushed, rearward review `ok`; branch `codex/ui-salt-companion-lookup`; active contract `ui_salt_companion_lookup`; checkpoint token `ck:a2ac0de7`.
- RED: `artifacts/validation/ui_salt_companion_native_red.json` failed before implementation because the companion suggestion authority API and hardcoded-reference seam did not exist yet.
- GREEN native: `artifacts/validation/ui_salt_companion_native.json` passed with `test_color_pipeline_core: passed=2051 failed=0` after the ordering repair.
- Window preservation: `artifacts/validation/ui_salt_companion_window.json` passed with `test_color_pipeline_window: passed=182 failed=0`.
- Materializer preservation: `artifacts/validation/ui_salt_companion_materializer_pytest.json` passed with `7 passed`.
- Runtime publish: `artifacts/validation/ui_salt_companion_publish.json` passed and staged `D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe`.
- Runtime proof: `artifacts/validation/ui_salt_companion_pytest.json` passed with `5 passed`, covering UI-Salt contract reporting and Color Pipeline SDF row preservation.
- Hostile-review defect repaired: active companion count originally depended on a later-declared catalog helper; fixed to read installed metadata storage directly, and the native rail locks the count and fallback behavior.

## Hostile Audit

- Status: complete
- Required posture: assume metadata-derived suggestions change current companion choices, permit unsupported pairs, stop auto-repairing root/SDF tuples, accidentally switch recipe expansion, or regress descriptor/compatibility metadata authority from earlier slices.

## Audit Passes

- [x] Pass 1 - inspected companion provider conversion for source-side and palette-side drift; native parity now compares every Source and Palette function against the hardcoded suggestion reference.
- [x] Pass 2 - inspected startup initialization and hardcoded fallback; found the first implementation counted active metadata through a helper declared later in the header, which failed compile before green proof.
- [x] Pass 3 - clean re-read found no additional real defect: the window path now calls only the companion provider, recipe expansion remains untouched, compatibility allow/deny remains metadata-backed, and SDF row behavior stayed covered by runtime proof.

## Audit Findings

- [x] Real finding: `CountActiveColorPipelineCompanionSuggestions()` initially called `GetColorPipelineLaneCatalogs()` before that helper was declared in the header; fixed to count directly over installed metadata storage.
- [x] Real finding: the pre-slice RED proved the companion authority seam did not exist and the window still owned the hardcoded Source/Palette map; fixed by adding metadata-derived companion lookup with hardcoded fallback and native parity coverage.
- [x] Clean re-read after repair confirmed root, phase, banded, scalar SDF, and normal-angle companion choices preserve the previous UI behavior, with no recipe/layout/Salticid runtime dependency widening.

## Notes

Validation targets:

- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_companion_native --log artifacts/logs/ui_salt_companion_native.log --out-json artifacts/validation/ui_salt_companion_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_companion_window --log artifacts/logs/ui_salt_companion_window.log --out-json artifacts/validation/ui_salt_companion_window.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_window`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_companion_materializer_pytest --log artifacts/logs/ui_salt_companion_materializer_pytest.log --out-json artifacts/validation/ui_salt_companion_materializer_pytest.json --heartbeat-seconds 30 --timeout-seconds 600 -- py -3.14 -m pytest tests/test_ui_salt_materializer.py -q`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_companion_publish --log artifacts/logs/ui_salt_companion_publish.log --out-json artifacts/validation/ui_salt_companion_publish.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_vsdevcmd.cmd`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_companion_pytest --log artifacts/logs/ui_salt_companion_pytest.log --out-json artifacts/validation/ui_salt_companion_pytest.json --heartbeat-seconds 30 --timeout-seconds 900 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_ui_salt_contract.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py`
- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_salt_companion_lookup.contract.json --out-json artifacts/validation/ui_salt_companion_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/ui_salt_companion_lookup_PHASED_PLAN.md --out-json artifacts/validation/ui_salt_companion_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/ui_salt_companion_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label ui_salt_companion_diff_check --log artifacts/logs/ui_salt_companion_diff_check.log --out-json artifacts/validation/ui_salt_companion_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`
