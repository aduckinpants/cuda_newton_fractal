# PFC Parameter Descriptor Export

## Explicit User Asks

- Continue the parameter functionality campaign after the closed fixed-family fold/mix slice.
- Work Step 4: `codex/pfc-parameter-descriptor-export`.
- Add a deterministic parameter surface descriptor/export guardrail so future slider audits compare schema, binding, runtime params, validation/state authority hints, and animation applicability from one machine-readable surface.
- Keep this as cleanup/guardrail work, not a new fractal feature.
- Preserve no-mouse runtime proof and avoid physical cursor automation.
- Do not change Color Pipeline behavior, capture finding, FPS pacing, equation-pack viewport integration, perturbation zoom, generated editors, or broad renderer architecture.

## Current Phase

Phase 1 is active: descriptor/export implementation is landed and under validation. Remaining work is contract/sync/hostile validators, checkpoint receipts, push, sprint-holder integration, and clean-tree proof.

## Phase Checklist

- [x] Integrate the closed fixed-family fold/mix slice into sprint holder `codex/parameter-functionality-campaign`.
- [x] Create feature branch `codex/pfc-parameter-descriptor-export` from the integrated sprint holder.
- [x] Create this checked-in plan and contract.
- [x] Lock the active slice to this descriptor-export contract.
- [x] RED: native tests prove the current all-lane parameter descriptor is only a test-local artifact, not a reusable source/runtime export.
- [x] RED: runtime tests prove no published CLI exports the all-lane parameter surface descriptor.
- [x] Implement a reusable descriptor builder derived from checked-in schema and binding authority.
- [x] Add CLI/runtime export for the descriptor without opening the viewer or using mouse automation.
- [x] Validate descriptor contents for owner-lane controls, `explaino_all` boundaries, binding resolution, state/validation authority hints, and animation applicability.
- [x] Run focused native descriptor/schema rails.
- [x] Publish runtime and run no-mouse CLI descriptor proof.
- [x] Run full native helper suite.
- [x] Hostile audit the diff and repair any real findings.
- [ ] Checkpoint, write validation and contract-proof receipts, push branch, integrate back to sprint holder, and verify clean tree.

## Owner Seams

- Schema authority: `ui/fractal_binding_surface_v1.ui_schema.json` and `ui_app/src/ui_schema.cpp`.
- Binding authority: `ui_app/src/schema_binding.cpp`.
- Fractal id authority: `ui_app/src/enum_id_utils.h`.
- Parameter animation authority: `ui_app/src/param_anim_dynamics.cpp` and the schema `param_anim_target` combo.
- Runtime export authority: `ui_app/src/headless_modes.cpp`, `ui_app/src/viewer_cli.*`, and `ui_app/src/main.cpp`.
- Existing proxy artifact: `ui_app/tests/test_schema_binding.cpp` currently writes `artifacts/analysis/phase9_10_all45_control_surface_descriptor.json` from test-only code.
- New descriptor surface: a small reusable native module under `ui_app/src`.
- Native proof: focused descriptor test plus existing schema/binding rails.
- Runtime proof: published runtime CLI export parsed by pytest.

## Proof Ledger

- Slice source branch: `codex/parameter-functionality-campaign`.
- Slice branch: `codex/pfc-parameter-descriptor-export`.
- Starting head: `2463241`.
- Current code fact: `test_schema_binding.cpp` contains a test-local descriptor writer, but product/runtime code has no reusable all-lane parameter-surface export.
- RED native witness: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_descriptor_red_native --log artifacts/logs/pfc_descriptor_red_native.log --out-json artifacts/validation/pfc_descriptor_red_native.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_parameter_surface_descriptor` failed because `fractal_parameter_surface_descriptor.cpp/.h` do not exist.
- RED runtime witness: `py -3.14 -m pytest tests/test_fractal_parameter_surface_descriptor_cli.py -q --junitxml artifacts/pytest/pfc_descriptor_red_runtime.junit.xml` failed because the published runtime has no `--describe-parameter-surface` / JSON export and falls through to viewer startup.
- Implementation witness: `ui_app/src/fractal_parameter_surface_descriptor.cpp` exports a reusable C++ descriptor from `UISchema`, `enum_id_utils::kFractalTypeIds`, `BindingContext`, and the schema `param_anim_target` visibility mirror.
- Runtime CLI witness: `--describe-parameter-surface` emits descriptor JSON to stdout and `--describe-parameter-surface-json <path>` writes the same descriptor to a file through the published runtime path.
- Guardrail witness: owner-lane controls `burning_ship_fold_mix`, `celtic_abs_mix`, `perpendicular_fold_mix`, `spider_feedback`, `collatz_transition_strength`, and `phoenix_p_real` appear on owning lanes with binding/validation/animation authority fields; the owner-specific fixed-family controls do not leak onto `explaino_all`.
- Additive Explaino-all witness: registry Explaino axes remain visible on both `explaino_all` and their explicit registry owner lanes; ad hoc smoke output reported 45 lanes and 225 visible family-control cells.
- Focused descriptor rail: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_descriptor_native --log artifacts/logs/pfc_descriptor_native.log --out-json artifacts/validation/pfc_descriptor_native.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_parameter_surface_descriptor` passed.
- Focused schema rail: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_descriptor_schema --log artifacts/logs/pfc_descriptor_schema.log --out-json artifacts/validation/pfc_descriptor_schema.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd advanced_color_grading_owner` passed.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_descriptor_runtime_publish --log artifacts/logs/pfc_descriptor_runtime_publish.log --out-json artifacts/validation/pfc_descriptor_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 1200 -- ui_app/build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- No-mouse runtime CLI proof: `py -3.14 -m pytest tests/test_fractal_parameter_surface_descriptor_cli.py -q --junitxml artifacts/pytest/pfc_descriptor_runtime.junit.xml` passed.
- Full native helper suite: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_descriptor_full_native --log artifacts/logs/pfc_descriptor_full_native.log --out-json artifacts/validation/pfc_descriptor_full_native.json --heartbeat-seconds 30 --timeout-seconds 2400 -- ui_app/build_tests_vsdevcmd.cmd` passed.
- Code quality: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/pfc_parameter_descriptor_export_code_quality.json` passed baseline with score 96/100.
- Validation pending: contract validation, plan sync, hostile audit validation, checkpoint, receipts, push, sprint-holder integration, and clean-tree proof.

## Hostile Audit

- Status: complete
- Descriptor generation moved out of the test-only helper into `ui_app/src/fractal_parameter_surface_descriptor.*`, and both native and published-runtime tests consume it.
- Descriptor rows are derived from loaded `UISchema`, `enum_id_utils::kFractalTypeIds`, and `BindingContext`; the runtime proof is not a hardcoded Python subset.
- Owner-specific fixed-family controls stay off `explaino_all`; registry Explaino axes intentionally remain additive on `explaino_all` and their explicit owner lanes.
- The descriptor is consumable through the published runtime CLI without opening the viewer and without mouse automation.
- Color Pipeline, capture finding, FPS pacing, equation-pack, perturbation, generated-editor, and renderer behavior were not changed by this slice.
- Closeout is not complete yet: receipts, push, sprint-holder integration, clean tree, and stale-plan review are still pending in the phase checklist.

## Audit Passes

- [x] Pass 1: reviewed descriptor contents as if a visible slider is missing or assigned to the wrong lane; focused native and runtime tests cover owner controls, lane count, binding paths, and fixed-family `explaino_all` non-leakage.
- [x] Pass 2: reviewed binding/animation flags as if they are schema-only and not runtime-authoritative; exporter resolves through `BindingContext` and `param_anim_target` visible options, with native tests requiring `binding_resolves` and `animatable`.
- [x] Pass 3: reviewed scope as if this accidentally widens into feature behavior or renderer changes; diff is descriptor/CLI/build/test only and the Color Pipeline focused rail passed.

## Audit Findings

- [x] Finding: the first focused descriptor target linked `schema_binding.cpp` without `explaino_seed.cpp`, producing unresolved `ExplainoSeedCombined` / `ExplainoSeedSetCombined` symbols. Fixed the descriptor test target link inputs and reran `pfc_descriptor_native` green.
- [x] Clean re-read: re-read the repaired state after the link-input fix; no additional real defect found in formula, renderer, Color Pipeline, capture finding, FPS pacing, equation-pack, perturbation, generated-editor behavior, or descriptor CLI routing.

## Out Of Scope

- Repairing branch-dead Explaino controls.
- Adding new fractal types or Explaino variants.
- Generated/internal editors.
- Equation-pack viewport integration.
- Perturbation zoom.
- Color Pipeline, capture finding, FPS pacing, and renderer behavior changes.
