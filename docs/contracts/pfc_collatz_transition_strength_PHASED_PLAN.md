# PFC Collatz Transition Strength

## Explicit User Asks

- Start the first campaign implementation slice from `codex/parameter-functionality-campaign`.
- Add `collatz_transition_strength` as a standalone `collatz` lane control.
- Preserve canonical Collatz behavior at default `1.0`.
- Use owner-lane visibility only; do not add this owner control to `explaino_all`.
- Use no-mouse persistent runtime proof and avoid physical cursor automation.
- Do not touch Explaino Collatz, Color Pipeline, capture finding, FPS pacing, equation-pack viewport integration, perturbation zoom, generated editors, or broader engine redesign in this slice.

## Current Phase

Phase 1 is closed: the standalone Collatz transition-strength control is implemented, proven on native and published-runtime paths, hostile-audited, and prepared for repository wrapper closure.

## Phase Checklist

- [x] Bootstrap and confirm campaign branch was clean before this feature branch.
- [x] Create feature branch `codex/pfc-collatz-transition-strength` from `codex/parameter-functionality-campaign`.
- [x] Inspect the standalone Collatz formula seam and Batch 1 control pattern.
- [x] Create this checked-in plan and contract.
- [x] Lock the active slice to this Collatz contract.
- [x] RED: schema/binding tests fail because `collatz_transition_strength` is absent from the `collatz` lane.
- [x] RED: validation/state/sample/runtime tests fail because the value is not bound, persisted, or consumed.
- [x] Implement `collatz_transition_strength` with default `1.0`, hard range `[0,4]`, UI range `[0,2]`, owner-lane visibility, binding, validation, state IO, diagnostics export, animation option, sample/runtime math, and default parity.
- [x] Validate focused schema/native/sample rails.
- [x] Publish runtime and run one persistent no-mouse viewer proof for Collatz selector identity plus frame-hash sensitivity.
- [x] Run full native helper suite.
- [x] Hostile audit the diff and repair any real findings.
- [x] Prepare checkpoint, validation receipts, contract-proof receipt, push, and clean-tree closure.

## Owner Seams

- UI schema authority: `ui/fractal_binding_surface_v1.ui_schema.json`.
- Binding authority: `ui_app/src/schema_binding.cpp`.
- Runtime params/defaults: `ui_app/src/fractal_types.h` and `ui_app/src/fractal_derived_fields.cpp`.
- State IO: `ui_app/src/diagnostics_state_io.cpp`.
- Diagnostics capture export: `ui_app/src/diagnostics_capture.cpp`.
- Runtime validation: `ui_app/src/fractal_runtime_validation.h`.
- Formula authority: `ui_app/src/escape_time_specialized_formulas.h`, called through `ui_app/src/fractal_sample_device.inl`.
- Native proof: `ui_app/tests/test_ui_schema.cpp`, `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_safe_mode_schema.cpp`, `ui_app/tests/test_fractal_runtime_validation.cpp`, and `ui_app/tests/test_fractal_sample_kernel.cu`.
- Runtime proof: `tests/test_fractal_runtime_parameter_functionality.py`.

## Proof Ledger

- Slice source branch: `codex/parameter-functionality-campaign`.
- Slice branch: `codex/pfc-collatz-transition-strength`.
- Starting head: `a745eca`.
- RED schema proof: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_collatz_red_schema --log artifacts/logs/pfc_collatz_red_schema.log --out-json artifacts/validation/pfc_collatz_red_schema.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd advanced_color_grading_owner` failed before implementation with `Visible-control matrix missing control collatz_transition_strength`.
- RED sample proof: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_collatz_red_sample --log artifacts/logs/pfc_collatz_red_sample.log --out-json artifacts/validation/pfc_collatz_red_sample.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_sample_kernel` failed before implementation because `KernelParams` had no `collatz_transition_strength`.
- Implementation fact: `KernelParams::collatz_transition_strength` now defaults to `1.0`, binds through `fractal.params.collatz_transition_strength`, is visible only on `collatz`, is validated as finite in `[0,4]`, persists through diagnostics state IO, exports in capture diagnostics, and feeds `StepCollatzEscapeState(...)`.
- Schema rail: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_collatz_schema --log artifacts/logs/pfc_collatz_schema.log --out-json artifacts/validation/pfc_collatz_schema.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd advanced_color_grading_owner` passed.
- Sample rail: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_collatz_sample --log artifacts/logs/pfc_collatz_sample.log --out-json artifacts/validation/pfc_collatz_sample.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_sample_kernel` passed with `test_fractal_sample_kernel: passed=984 failed=0`.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_collatz_runtime_publish --log artifacts/logs/pfc_collatz_runtime_publish.log --out-json artifacts/validation/pfc_collatz_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 1200 -- ui_app/build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- No-mouse runtime proof: `py -3.14 -m pytest tests/test_fractal_runtime_parameter_functionality.py -q --junitxml artifacts/pytest/pfc_collatz_runtime.junit.xml` passed; one persistent viewer process selected `collatz`, set `fractal_control.collatz_transition_strength.primary`, consumed the set-value command, preserved selector identity, and changed the frame hash.
- Full native rail: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_collatz_full_native --log artifacts/logs/pfc_collatz_full_native.log --out-json artifacts/validation/pfc_collatz_full_native.json --heartbeat-seconds 30 --timeout-seconds 2400 -- ui_app/build_tests_vsdevcmd.cmd` passed with `All helper tests passed`.
- Code-quality rail: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/pfc_collatz_transition_strength_code_quality.json` passed after repairing the line-count regression.
- Diff-check rail: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_collatz_diff_check --log artifacts/logs/pfc_collatz_diff_check.log --out-json artifacts/validation/pfc_collatz_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.

## Hostile Audit

- Status: complete
- Did I actually expose `collatz_transition_strength` only on standalone `collatz`?
- Did I preserve default Collatz behavior at `1.0`?
- Did I prove non-default values change the public sample/runtime path?
- Did I avoid Explaino Collatz, `explaino_all`, Color Pipeline, capture, FPS pacing, equation-pack, perturbation, generated-editor, or broad engine drift?
- Did I use one persistent no-mouse runtime proof instead of physical cursor or repeated relaunch automation?
- Did I close with receipts, push, clean tree, and no stale plan text?

## Audit Passes

- [x] Pass 1: reviewed schema/binding/state/runtime math as if the control was visible but dead; found the sample witness was too broad and could compare nonfinite outputs.
- [x] Pass 2: reviewed runtime proof and selector identity as if the harness consumed the command without changing the rendered path; verified the persistent no-mouse pytest checks selector identity plus frame-hash change.
- [x] Pass 3: clean re-read confirmed no additional real issue found in scope or validation receipts; the slice touched standalone Collatz surfaces and did not change Explaino Collatz, `explaino_all`, Color Pipeline, capture finding, FPS pacing, equation-pack, perturbation, or generated editors.

## Audit Findings

- [x] Finding 1: the initial sample witness used a wide grid that could include nonfinite/NaN behavior and made default parity brittle. Repaired by narrowing the Collatz sample witness to finite real-axis points and rerunning the sample rail.
- [x] Finding 2: adding a safe-mode control changed the expected fractal panel shape. Repaired stale panel-count expectations in `test_ui_schema` and `test_safe_mode_schema`, then reran full native.
- [x] Finding 3: adding the parameter regressed the code-quality max-function-lines baseline in `StepCollatzEscapeState(...)`. Repaired the helper body and reran code-quality, sample, runtime publish, no-mouse runtime pytest, and full native.
- [x] Clean third pass: no additional real issue found after the repaired validation set.

## Out Of Scope

- `explaino_collatz` direct variant work.
- Explaino Julia direct constants.
- Fixed-family fold/mix controls.
- Parameter descriptor/export tooling.
- Generated/internal editors.
- Equation-pack viewport integration.
- Perturbation zoom.
- Color Pipeline, capture finding, and FPS pacing changes.
