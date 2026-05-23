# PFC Phoenix Parameterization

## Explicit User Asks

- Continue the parameter functionality campaign after the closed Collatz transition-strength slice.
- Work Step 2: `codex/pfc-phoenix-parameterization`.
- Expose/prove the existing Phoenix `p` parameterization only if the runtime already has a stable authority seam.
- Preserve existing Phoenix and Explaino-Phoenix behavior at defaults.
- Use owner-lane visibility and no-mouse persistent runtime proof.
- Do not change `explaino_all`, Color Pipeline, capture finding, FPS pacing, equation-pack viewport integration, perturbation zoom, generated editors, or broad engine architecture in this slice.

## Current Phase

Phase 1 is closed: the existing Phoenix authority seam is proven, the missing safe-mode control surface is repaired, persistent no-mouse runtime proof passed, hostile audit found and repaired real issues, and the slice is prepared for repository wrapper closure.

## Phase Checklist

- [x] Integrate the closed Collatz slice into sprint holder `codex/parameter-functionality-campaign`.
- [x] Create feature branch `codex/pfc-phoenix-parameterization` from the integrated sprint holder.
- [x] Bootstrap and confirm branch `codex/pfc-phoenix-parameterization`, head `694e4b8`, clean tree, and stale closed Collatz active contract.
- [x] Inspect current Phoenix authority seams before mutation.
- [x] Create this checked-in plan and contract.
- [x] Lock the active slice to this Phoenix contract.
- [x] RED/baseline: prove main schema/runtime already has `phoenix_p_real` and `phoenix_p_imag` authority while safe-mode schema lacks the controls.
- [x] Add focused safe-mode schema coverage for Phoenix `p` controls.
- [x] Add persistent no-mouse runtime proof for visible Phoenix `p` controls preserving selector identity and changing frame hash.
- [x] Repair only the missing Phoenix control-surface/proof gaps found by RED/baseline.
- [x] Validate focused schema/safe-mode/sample rails.
- [x] Publish runtime and run the persistent no-mouse Phoenix proof.
- [x] Run full native helper suite.
- [x] Hostile audit the diff and repair any real findings.
- [x] Prepare checkpoint, validation receipts, contract-proof receipt, push, and clean-tree closure.

## Owner Seams

- Main UI schema authority: `ui/fractal_binding_surface_v1.ui_schema.json`.
- Safe-mode schema authority: `ui_app/src/safe_mode_schema.cpp`.
- Focused native harness authority: `ui_app/build_tests_vsdevcmd.cmd`.
- Binding authority: `ui_app/src/schema_binding.cpp`.
- Runtime params/defaults: `ui_app/src/fractal_types.h`, `ui_app/src/fractal_derived_fields.cpp`, and `ui_app/src/fractal_family_rules.h`.
- State IO and capture export: `ui_app/src/diagnostics_state_io.cpp` and `ui_app/src/diagnostics_capture.cpp`.
- Runtime validation: `ui_app/src/fractal_runtime_validation.h`.
- Formula authority: `ui_app/src/escape_time_direct_formulas.h` through `ui_app/src/fractal_sample_device.inl`.
- Native proof: `ui_app/tests/test_ui_schema.cpp`, `ui_app/tests/test_safe_mode_schema.cpp`, `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_fractal_runtime_validation.cpp`, and `ui_app/tests/test_fractal_sample_kernel.cu`.
- Runtime proof: `tests/test_fractal_runtime_parameter_functionality.py`.

## Proof Ledger

- Slice source branch: `codex/parameter-functionality-campaign`.
- Slice branch: `codex/pfc-phoenix-parameterization`.
- Starting head: `694e4b8`.
- Current code fact: `phoenix_p_real` and `phoenix_p_imag` exist in `KernelParams`, bind through `schema_binding`, persist through state IO, export through diagnostics capture, validate as finite in `[-1,1]`, and feed the Phoenix direct formula as complex `phoenixP`.
- Baseline proof: main JSON schema already exposed `phoenix_p_real` and `phoenix_p_imag` on Phoenix-step lanes; safe-mode schema had no `phoenix_p_*` controls.
- RED harness proof: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_phoenix_red_safe_mode --log artifacts/logs/pfc_phoenix_red_safe_mode.log --out-json artifacts/validation/pfc_phoenix_red_safe_mode.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_safe_mode_schema` first exposed a missing focused test target, then failed correctly with `TestSafeModeSchemaExposesPhoenixControls_Real` and `_Imag`.
- Implementation fact: safe-mode schema now exposes `phoenix_p_real` and `phoenix_p_imag` as bounded `[-1,1]` controls on `phoenix` and the existing Phoenix-step carrier lanes, explicitly excluding `explaino_all`.
- Safe-mode rail: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_phoenix_safe_mode --log artifacts/logs/pfc_phoenix_safe_mode.log --out-json artifacts/validation/pfc_phoenix_safe_mode.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_safe_mode_schema` passed with `test_safe_mode_schema: 50 passed, 0 failed`.
- Schema rail: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_phoenix_schema --log artifacts/logs/pfc_phoenix_schema.log --out-json artifacts/validation/pfc_phoenix_schema.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd advanced_color_grading_owner` passed.
- Sample rail: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_phoenix_sample --log artifacts/logs/pfc_phoenix_sample.log --out-json artifacts/validation/pfc_phoenix_sample.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_sample_kernel` passed with `test_fractal_sample_kernel: passed=991 failed=0`.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_phoenix_runtime_publish --log artifacts/logs/pfc_phoenix_runtime_publish.log --out-json artifacts/validation/pfc_phoenix_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 1200 -- ui_app/build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- No-mouse runtime proof: `py -3.14 -m pytest tests/test_fractal_runtime_parameter_functionality.py -q --junitxml artifacts/pytest/pfc_phoenix_runtime.junit.xml` passed; one persistent viewer loaded Phoenix, set `fractal_control.phoenix_p_real.primary` and `fractal_control.phoenix_p_imag.primary`, consumed set-value commands, preserved selector identity, and changed frame hashes.
- Full native rail: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_phoenix_full_native --log artifacts/logs/pfc_phoenix_full_native.log --out-json artifacts/validation/pfc_phoenix_full_native.json --heartbeat-seconds 30 --timeout-seconds 2400 -- ui_app/build_tests_vsdevcmd.cmd` passed with `All helper tests passed`.
- Code-quality rail: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/pfc_phoenix_parameterization_code_quality.json` passed, score `97/100`.

## Hostile Audit

- Status: complete
- Did I actually prove and preserve the existing Phoenix runtime authority seam instead of inventing another one?
- Did I repair the safe-mode control surface without changing the main formula semantics?
- Did I keep `phoenix_p_real` and `phoenix_p_imag` off `explaino_all`?
- Did I preserve Phoenix and Explaino-Phoenix selector identity and defaults?
- Did I use one persistent no-mouse runtime proof instead of physical cursor or repeated relaunch automation?
- Did I avoid Color Pipeline, capture finding, FPS pacing, equation-pack, perturbation, generated-editor, and broad architecture drift?
- Did I close with receipts, push, clean tree, and no stale plan text?

## Audit Passes

- [x] Pass 1: reviewed schema/safe-mode/binding as if the control was visible in one surface but missing in another; found the focused safe-mode rail could not run because the build script lacked a focused target.
- [x] Pass 2: reviewed sample/runtime proof as if set-value was consumed but the formula path did not change; found and repaired the overlinked sample witness, then proved Phoenix `p` real and imaginary sample sensitivity and no-mouse frame-hash changes.
- [x] Pass 3: clean re-read confirmed no additional real issue found in scope; diff does not change main JSON schema, state IO, validation, direct formula semantics, `explaino_all`, Color Pipeline, capture finding, FPS pacing, equation-pack, perturbation, generated editors, or renderer architecture.

## Audit Findings

- [x] Finding 1: the required focused safe-mode RED initially failed on an unknown `test_safe_mode_schema` target, not on product behavior. Repaired by adding a focused safe-mode target to `ui_app/build_tests_vsdevcmd.cmd`.
- [x] Finding 2: the initial Phoenix sample witness called `ApplyFractalPresetDefaults(...)`, which would have expanded the focused CUDA sample target's link surface. Repaired the witness to use the existing inline Phoenix carrier descriptor instead.
- [x] Finding 3: full native found stale `test_ui_schema` safe-mode panel shape expectations after adding the two Phoenix controls. Repaired the expected count from 21 to 23 and reran full native.
- [x] Clean third pass: no additional real issue found after the repaired validation set.

## Out Of Scope

- New Phoenix math model or new `FractalType`.
- Explaino Collatz direct variant work.
- Explaino Julia direct constants.
- Fixed-family fold/mix controls.
- Parameter descriptor/export tooling.
- Generated/internal editors.
- Equation-pack viewport integration.
- Perturbation zoom.
- Color Pipeline, capture finding, and FPS pacing changes.
