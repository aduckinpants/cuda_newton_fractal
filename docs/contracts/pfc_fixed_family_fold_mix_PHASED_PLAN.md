# PFC Fixed-Family Fold Mix

## Explicit User Asks

- Continue the parameter functionality campaign after the closed Phoenix parameterization slice.
- Work Step 3: `codex/pfc-fixed-family-fold-mix`.
- Add default-preserving fold/mix controls for fixed escape-time families:
  - `burning_ship_fold_mix`
  - `celtic_abs_mix`
  - `perpendicular_fold_mix`
- Keep controls owner-lane only; do not add them to `explaino_all`.
- Preserve current visual behavior at default `1.0`.
- Use no-mouse persistent runtime proof and avoid physical cursor automation.
- Do not change Color Pipeline, capture finding, FPS pacing, equation-pack viewport integration, perturbation zoom, generated editors, or broad engine architecture in this slice.

## Current Phase

Phase 1 is complete: the three owner-lane fold/mix controls were added with binding, validation, state IO, direct-formula consumption, native sample sensitivity, published-runtime no-mouse proof, full native validation, code-quality repair, hostile audit, checkpoint closeout, receipts, push, and clean-tree verification.

## Phase Checklist

- [x] Integrate the closed Phoenix slice into sprint holder `codex/parameter-functionality-campaign`.
- [x] Create feature branch `codex/pfc-fixed-family-fold-mix` from the integrated sprint holder.
- [x] Bootstrap and confirm branch `codex/pfc-fixed-family-fold-mix`, head `7758ec8`, clean tree, and stale closed Phoenix active contract.
- [x] Inspect fixed-family direct-formula seams before mutation.
- [x] Create this checked-in plan and contract.
- [x] Lock the active slice to this fold-mix contract.
- [x] RED: schema/safe-mode/binding tests fail because the three fold/mix controls are absent.
- [x] RED: sample/runtime tests fail because the three values are not bound, persisted, or consumed.
- [x] Implement the three owner-lane controls with default `1.0`, range `[0,1]`, binding, validation, state IO, diagnostics export, safe-mode parity, direct-formula math, and default parity.
- [x] Validate focused schema/safe-mode/sample rails and full-native direct-formula/runtime-validation rails.
- [x] Publish runtime and run one persistent no-mouse viewer proof for all three controls.
- [x] Run full native helper suite.
- [x] Hostile audit the diff and repair any real findings.
- [x] Checkpoint, write validation and contract-proof receipts, push branch, and verify clean tree.

## Owner Seams

- Main UI schema authority: `ui/fractal_binding_surface_v1.ui_schema.json`.
- Safe-mode schema authority: `ui_app/src/safe_mode_schema.cpp`.
- Binding authority: `ui_app/src/schema_binding.cpp`.
- Runtime params/defaults: `ui_app/src/fractal_types.h` and `ui_app/src/fractal_derived_fields.cpp`.
- State IO and capture export: `ui_app/src/diagnostics_state_io.cpp` and `ui_app/src/diagnostics_capture.cpp`.
- Runtime validation: `ui_app/src/fractal_runtime_validation.h`.
- Formula authority: `ui_app/src/escape_time_direct_formulas.h` through `ui_app/src/fractal_sample_device.inl` and `ui_app/src/fractal_probe_runner.cpp`.
- Native proof: `ui_app/tests/test_ui_schema.cpp`, `ui_app/tests/test_safe_mode_schema.cpp`, `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_fractal_runtime_validation.cpp`, `ui_app/tests/test_escape_time_direct_formulas.cpp`, and `ui_app/tests/test_fractal_sample_kernel.cu`.
- Runtime proof: `tests/test_fractal_runtime_parameter_functionality.py`.

## Proof Ledger

- Slice source branch: `codex/parameter-functionality-campaign`.
- Slice branch: `codex/pfc-fixed-family-fold-mix`.
- Starting head: `7758ec8`.
- Current code fact: Burning Ship, Celtic Mandelbrot, and Perpendicular Burning Ship use hard-coded fold/abs transforms in `StepEscapeTimeDirectState(...)`; no `KernelParams` fields, schema controls, binding, validation, state IO, or runtime proof exist for their mix factors.
- RED schema witness: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_fold_mix_red_schema --log artifacts/logs/pfc_fold_mix_red_schema.log --out-json artifacts/validation/pfc_fold_mix_red_schema.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd advanced_color_grading_owner` failed as expected with `Visible-control matrix missing control burning_ship_fold_mix`.
- RED sample witness: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_fold_mix_red_sample --log artifacts/logs/pfc_fold_mix_red_sample.log --out-json artifacts/validation/pfc_fold_mix_red_sample.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_sample_kernel` failed as expected because `KernelParams` had no `burning_ship_fold_mix`, `celtic_abs_mix`, or `perpendicular_fold_mix`.
- Required runtime witness passed: one persistent viewer process loads each owning fractal, sets its fold/mix control, keeps the owning selector selected, consumes set-value commands, and changes the rendered frame hash.
- Required default parity passed: default `1.0` preserves current formula behavior for all three families in `test_escape_time_direct_formulas` and `test_fractal_sample_kernel`.
- Required non-default sensitivity passed: non-default values change `SampleFractalPoints(...)` results for all three families.
- Focused schema/state/binding rail passed: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_fold_mix_schema --log artifacts/logs/pfc_fold_mix_schema.log --out-json artifacts/validation/pfc_fold_mix_schema.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd advanced_color_grading_owner`.
- Safe-mode rail passed: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_fold_mix_safe_mode --log artifacts/logs/pfc_fold_mix_safe_mode.log --out-json artifacts/validation/pfc_fold_mix_safe_mode.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_safe_mode_schema` (`54 passed, 0 failed`).
- Sample rail passed: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_fold_mix_sample --log artifacts/logs/pfc_fold_mix_sample.log --out-json artifacts/validation/pfc_fold_mix_sample.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_sample_kernel` (`passed=1006 failed=0`).
- Full native helper suite passed: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_fold_mix_full_native --log artifacts/logs/pfc_fold_mix_full_native.log --out-json artifacts/validation/pfc_fold_mix_full_native.json --heartbeat-seconds 30 --timeout-seconds 2400 -- ui_app/build_tests_vsdevcmd.cmd` (`All helper tests passed`).
- Runtime publish passed: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_fold_mix_runtime_publish --log artifacts/logs/pfc_fold_mix_runtime_publish.log --out-json artifacts/validation/pfc_fold_mix_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 1200 -- ui_app/build_vsdevcmd.cmd`.
- No-mouse runtime pytest passed: `py -3.14 -m pytest tests/test_fractal_runtime_parameter_functionality.py -q --junitxml artifacts/pytest/pfc_fold_mix_runtime.junit.xml` (`1 passed`).
- Code-quality audit passed after splitting the safe-mode fractal control list to remove the local function-length regression: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/pfc_fixed_family_fold_mix_code_quality.json` (`Baseline check passed`).
- After the safe-mode control-list refactor, the affected safe-mode rail, full native helper suite, runtime publish, and no-mouse runtime pytest were rerun and passed again against the final working tree.
- Extra attempted focused commands for `test_escape_time_direct_formulas` and `test_fractal_runtime_validation` failed before compilation because those target names are not exposed by `ui_app/build_tests_vsdevcmd.cmd`; the full native rail did compile and run both tests.

## Hostile Audit

- Status: complete
- Did I actually add the three controls only on their owning fixed-family lanes?
- Did I preserve existing Burning Ship, Celtic Mandelbrot, and Perpendicular Burning Ship behavior at default `1.0`?
- Did I prove non-default values affect the shipped direct-formula sample/runtime path?
- Did I keep these owner-specific controls off `explaino_all`?
- Did I avoid Color Pipeline, capture finding, FPS pacing, equation-pack, perturbation, generated-editor, and broad architecture drift?
- Did I use one persistent no-mouse runtime proof instead of physical cursor or repeated relaunch automation?
- Did I close with receipts, push, clean tree, and no stale plan text?

## Audit Passes

- [closed] Pass 1: reviewed schema/binding/state/runtime math as if the controls were visible but dead; sample and published-runtime no-mouse witnesses now prove non-default values change output.
- [closed] Pass 2: reviewed default parity as if the default changed the existing formula; helper and sample tests preserve default `1.0` behavior for all three families.
- [closed] Pass 3: reviewed scope as if this accidentally widened into Explaino umbrella, Color Pipeline, or renderer redesign work; controls are owner-lane only and no Color Pipeline/capture/FPS/equation-pack/perturbation/generated-editor seams were changed.

## Audit Findings

- [x] Real gap found and fixed: fixed-family folds were hard-coded formulas with no user-facing parameter authority. Regression coverage now exists in schema/binding, safe-mode schema, runtime validation, diagnostics state IO, direct formula helper, CUDA sample sensitivity, and no-mouse published runtime proof.
- [x] Code-quality issue found and fixed: adding the three safe-mode controls lengthened `BuildSafeModeFractalPanel()` past baseline; `BuildSafeModeFractalControls()` now owns the list and `code_quality_audit.py --check-baseline` passes.
- [x] Clean re-read of the repaired state confirmed the repaired state after the real gap was fixed: no additional real defect found in owner-lane visibility, binding, state IO, direct formula math, default parity, sample sensitivity, or no-mouse runtime proof.
- [x] Workflow note: `ui_app/build_tests_vsdevcmd.cmd` does not expose focused target names for `test_escape_time_direct_formulas` or `test_fractal_runtime_validation`; this slice did not widen into harness changes, and the full native helper rail covers both tests.

## Out Of Scope

- Explaino Collatz direct variant work.
- Explaino Julia direct constants.
- Parameter descriptor/export tooling.
- Generated/internal editors.
- Equation-pack viewport integration.
- Perturbation zoom.
- Color Pipeline, capture finding, and FPS pacing changes.
