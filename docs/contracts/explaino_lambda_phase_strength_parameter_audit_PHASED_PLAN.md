# Explaino Lambda Phase-Strength And Parameter Audit Plan

## Explicit User Asks

- Fix the confirmed parameter issue instead of hiding functionality.
- Prefer more functionality when possible.
- After the repair, audit for additional parameters or functionality opportunities similar to the Multibrot complex-exponent expansion.
- Preserve no-mouse persistent runtime proof and avoid unrelated renderer, pacing, capture, or Color Pipeline churn.

## Current Phase

Phase 1 is closed once this checked-in plan lands with the checkpoint commit and machine receipts: `explaino_lambda` phase-strength authority is repaired and green on focused native/runtime rails, and hostile review found and fixed one UI-help wording issue.

## Phase Checklist

- [x] Bootstrap and confirm repo status.
- [x] Open a bounded checked-in plan and contract.
- [x] Add the failing `explaino_lambda` phase-strength runtime regression.
- [x] Repair the narrow runtime authority seam.
- [x] Validate native/runtime proof rails.
- [x] Run the follow-on parameter/functionality exposure audit and write the artifact.
- [x] Complete hostile audit and plan sync.
- [x] Checkpoint the slice.

## Proof Ledger

- Prior sweep artifact: `artifacts/audit/wide_parameter_sweep/wide_parameter_sweep_summary.md`.
- Prior runtime finding: `explaino_lambda::explaino_phase_strength` was visible and set-value automation was consumed, but the rendered hash stayed unchanged with `explaino_phase=1.2` and `explaino_phase_strength=7.0`.
- RED runtime proof failed as expected: `py -3.14 -m pytest tests/test_fractal_runtime_explaino_escape_variants.py::test_explaino_lambda_phase_strength_changes_live_output_no_mouse -q --junitxml artifacts/pytest/test_explaino_lambda_phase_strength_runtime_red.junit.xml`; selector stayed `explaino_lambda`, set-value was consumed, and the rendered hash did not change.
- Runtime repair: `ui_app/src/fractal_sample_device.inl` now routes `view.explaino_phase_strength` into the `explaino_lambda` warp-start phase as `phase * phaseStrength`, preserving default behavior at `1.0`.
- Native focused rail green: `py -3.14 tools/viewer_host_run_logged_command.py --label explaino_lambda_phase_strength_native --log artifacts/logs/explaino_lambda_phase_strength_native.log --out-json artifacts/validation/explaino_lambda_phase_strength_native.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_sample_kernel`; `test_fractal_sample_kernel: passed=969 failed=0`.
- Runtime publish green: `py -3.14 tools/viewer_host_run_logged_command.py --label explaino_lambda_phase_strength_publish_runtime --log artifacts/logs/explaino_lambda_phase_strength_publish_runtime.log --out-json artifacts/validation/explaino_lambda_phase_strength_publish_runtime.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_vsdevcmd.cmd`.
- Published-runtime no-mouse proof green: `py -3.14 -m pytest tests/test_fractal_runtime_explaino_escape_variants.py::test_explaino_lambda_phase_strength_changes_live_output_no_mouse -q --junitxml artifacts/pytest/test_explaino_lambda_phase_strength_runtime.junit.xml`; `1 passed`.
- Parameter/functionality audit artifact: `artifacts/audit/explaino_lambda_phase_strength_parameter_audit.md`.
- Contract validation green: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/explaino_lambda_phase_strength_parameter_audit.contract.json --out-json artifacts/validation/explaino_lambda_phase_strength_parameter_audit_contract.json`.
- Plan sync green: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`.
- Code-quality baseline green: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/explaino_lambda_phase_strength_parameter_audit_code_quality.json`; score `97/100`, baseline passed.
- Final-tree native focused rail green after the UI/schema help edit: `py -3.14 tools/viewer_host_run_logged_command.py --label explaino_lambda_phase_strength_native_final --log artifacts/logs/explaino_lambda_phase_strength_native_final.log --out-json artifacts/validation/explaino_lambda_phase_strength_native_final.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_sample_kernel`; `test_fractal_sample_kernel: passed=969 failed=0`.
- Final-tree runtime publish green after the UI/schema help edit: `py -3.14 tools/viewer_host_run_logged_command.py --label explaino_lambda_phase_strength_publish_runtime_final --log artifacts/logs/explaino_lambda_phase_strength_publish_runtime_final.log --out-json artifacts/validation/explaino_lambda_phase_strength_publish_runtime_final.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_vsdevcmd.cmd`.
- Final-tree published-runtime no-mouse proof green: `py -3.14 -m pytest tests/test_fractal_runtime_explaino_escape_variants.py::test_explaino_lambda_phase_strength_changes_live_output_no_mouse -q --junitxml artifacts/pytest/test_explaino_lambda_phase_strength_runtime_final.junit.xml`; `1 passed`.
- Schema JSON parse green: `py -3.14 -m json.tool ui/fractal_binding_surface_v1.ui_schema.json > $null`.

## Hostile Audit

- Status: complete
- Did I actually repair the confirmed no-effect `explaino_lambda::explaino_phase_strength` control? Yes: native sample output and published-runtime no-mouse frame hash both change after the repair.
- Did I preserve explicit selector identity? Yes: the runtime proof asserts `current_fractal_type == explaino_lambda` before and after the set-value command.
- Did I avoid physical mouse automation and viewer relaunch loops? Yes: the proof uses `PersistentRuntimeViewerAutomation` under `runtime_automation_lock`.
- Did I avoid widening into Color Pipeline, capture, pacing, generic renderer, or new fractal features? Yes: the code change is limited to Lambda phase routing, one help string, and focused tests/docs.
- Did I honestly separate fixed bugs from future functionality? Yes: Spider feedback, rational-escape denominator power, Collatz controls, fixed-family variants, generated/internal editors, equation-pack viewport integration, and perturbation zoom are documented as future slices only.

## Audit Passes

- [complete] Pass 1: reviewed the repair diff. The runtime mutation is limited to `explaino_lambda` phase seeding and keeps selector identity/test harness changes narrow.
- [complete] Pass 2: reviewed the UI/schema text after the runtime repair. Found one real wording issue: `explaino_phase_strength` help described only root-angle modulation even though `explaino_lambda` now uses it for warp-start phase. Fixed the help text to say Explaino phase modulation.
- [complete] Pass 3: reviewed the current parameter exposure baseline against schema and formula seams. Multibrot complex exponent, Julia direct c, McMullen direct controls, and the prior all-44 cleanup are already harvested; the remaining useful opportunities require new runtime authority rather than binding-only surfacing.
- [complete] Pass 4: clean re-read of the repaired state reran final-tree native sample, runtime publish, published-runtime no-mouse proof, contract validation, plan sync, code-quality baseline, and schema JSON parse with no additional real defect found.

## Audit Findings

- [x] Real bug fixed: `explaino_lambda` displayed `explaino_phase_strength`, consumed the set-value command, preserved selector identity, but did not route the value into the rendered frame.
- [x] Real audit finding fixed: Phase Strength help text was too narrow for the repaired Lambda behavior.
- [x] Not claimed fixed: `nova::epsilon` and `explaino_y::epsilon` remain weak-witness rows from the wide sweep, not confirmed dead sliders; they need tuned near-threshold views before product claims.
- [x] Not implemented in this slice: Spider feedback, rational-escape denominator power, Collatz controls, fixed-family variants, generated/internal editors, equation-pack viewport integration, and perturbation zoom. These are documented as future functionality slices, not silently bundled.
