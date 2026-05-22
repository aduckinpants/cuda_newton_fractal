# Explaino Counterfactual Regression Repair

## Current Phase

Closed - focused native/runtime rails, hostile-audit, checkpoint, receipts, push, and clean-tree proof completed for the Explaino Counterfactual regression repair.

## Phase Checklist

- [x] Phase 0 - open a dedicated checked-in plan/contract and lock the active slice.
- [x] Phase 1 - inspect counterfactual seams and reproduce the suspected regression with current code/runtime.
- [x] Phase 2 - add the narrowest RED coverage that fails on the real counterfactual behavior, not a proxy.
- [x] Phase 3 - repair only the measured counterfactual regression while preserving Explaino selector/control, color pipeline, capture, and FPS pacing behavior.
- [x] Phase 4 - validate focused native/runtime rails, hostile-audit, checkpoint, receipts, push, and clean tree.

## Explicit User Asks

- [done] Merge the completed changes upstream before new work.
- [done] Investigate the suspected Explaino counterfactual regression; current belief is that counterfactual behavior broke at some point and was not noticed.
- [done] Do not guess or claim a fix without a reproduction and proof.
- [done] Keep this separate from FPS pacing, capture finding, color pipeline, and broad engine work unless code proof shows a strict dependency.

## Proof Ledger

- Starting point: `master` was fast-forwarded and pushed to `f5e845b` before this slice.
- Reproduction: `artifacts/runtime_measure/explaino_counterfactual_probe/measurement.json` shows `explaino_counterfactual_pair` cubic and quartic root-family captures produce the same frame hash (`037273e3...`) while standalone `counterfactual_pair` cubic/quartic captures differ.
- Reproduction: the same probe shows Explaino Counterfactual persists `counterfactual_pair_root_family=quartic_unit_roots` while staying on stale `poly_kind=custom`, so the state can claim quartic without changing the rendered formula.
- Schema finding: `counterfactual_pair_root_family` is currently visible only for `counterfactual_pair`; the other Counterfactual Pair controls are visible for both `counterfactual_pair` and `explaino_counterfactual_pair`.
- Runtime finding: `explaino_counterfactual_pair` reads `poly_coeffs`, `poly_kind`, and `explaino_root_count`; root-family changes are inert unless the Explaino carrier polynomial/root-count derivation honors that family.
- Required RED: the test must exercise the user-facing/public path for the broken counterfactual behavior.
- RED result: `artifacts/pytest/test_explaino_counterfactual_root_family_red.junit.xml` failed against the pre-rebuild published runtime because cubic Explaino Counterfactual still exported `explaino_root_count == 4`.
- Native GREEN: `artifacts/validation/explaino_counterfactual_native.json` passed the focused schema/binding/derived-field/runtime-validation/safe-mode rails.
- Hostile audit finding: stricter runtime validation initially risked breaking the native `SampleFractalPoints(...)` Explaino Counterfactual fixture; the fixture now uses the custom cubic carrier shape and `artifacts/validation/explaino_counterfactual_sample_kernel.json` passed.
- Runtime publish GREEN: `artifacts/validation/explaino_counterfactual_publish_runtime.json` rebuilt and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Runtime GREEN: `artifacts/pytest/test_explaino_counterfactual_runtime.junit.xml` passed the no-mouse published-runtime proof after rebuild.
- Contract GREEN: `artifacts/validation/explaino_counterfactual_regression_contract.json`, `artifacts/validation/explaino_counterfactual_regression_hostile_audit.json`, `artifacts/validation/explaino_counterfactual_code_quality.json`, and phased-plan sync passed.
- Non-goals: FPS debounce rework, capture finding changes, color pipeline changes, generic renderer rewrite, or new fractal feature work.

## Hostile Audit

- Status: done
- Did I prove the current counterfactual behavior before changing it? Yes: the runtime probe and RED pytest prove stale quartic root-family state was inert on the Explaino carrier.
- Did I add a test that would have caught the regression? Yes: schema/binding/derived-field/runtime-validation rails plus a no-mouse published-runtime root-family output proof.
- Did I avoid accidentally changing unrelated Explaino/color/capture/FPS behavior? Yes: diff is counterfactual/schema/test-harness focused; no color/capture/FPS source files are touched.
- Did I validate the repaired public path, not just helper math? Yes: the published-runtime pytest passed after rebuild.
- Did I close with checkpoint, receipts, push, and clean tree? Yes; the final repo proof is recorded by the checkpoint/receipt/push/clean-tree commands for this slice.

## Audit Passes

- [done] Pass 1: reviewed the reproduction as if the first symptom was misleading; confirmed schema visibility and derived polynomial/root-count authority were both wrong.
- [done] Pass 2: reviewed the repair as if it only changed a helper; added and passed published-runtime no-mouse proof plus native schema/binding rails.
- [done] Pass 3: reviewed validation blast radius; found and fixed the native sample-kernel fixture that would have failed under the stricter Explaino Counterfactual validation.
- [done] Pass 4: clean re-read of the repaired state after the sample-kernel fixture fix found no additional real defect in the touched Counterfactual, schema, validation, or runtime-proof seams.

## Audit Findings

- [done] Finding 1: Explaino Counterfactual root-family authority is incomplete: the control is hidden on the Explaino lane, and saved/edited quartic state does not change the Explaino Counterfactual runtime output.
- [done] Finding 2: runtime validation needed to reject stale Explaino Counterfactual root-family/root-count drift; the repair initially exposed a stale test fixture, now fixed and covered by `test_fractal_sample_kernel`.

## Initial Seams To Inspect

- `ui/fractal_binding_surface_v1.ui_schema.json`
- `ui_app/src/fractal_types.h`
- `ui_app/src/fractal_family_rules.h`
- `ui_app/src/fractal_sample_device.inl`
- `ui_app/src/fractal_renderer.cu`
- `ui_app/src/fractal_runtime_validation.h`
- `ui_app/src/schema_binding.cpp`
- `ui_app/src/diagnostics_state_io.cpp`
- `ui_app/tests/test_fractal_renderer.cu`
- `ui_app/tests/test_fractal_runtime_validation.cpp`
- `tests/test_fractal_runtime_explaino_escape_variants.py`
