# Counterfactual Pair Fractal - Bounded Runtime Lane

## Current Phase

Phase 4 - hostile audit, validation receipts, checkpoint closure, and the committed-head stale-plan gate on the repaired Counterfactual Pair green head.

## Phase Checklist

- [x] Phase 1 - open this checked-in plan and contract, replace the closed sample-evidence stop-point lock, and prove the RED head still has no shipped Counterfactual Pair Fractal runtime lane beyond the existing sidecar witnesses.
- [x] Phase 2 - land one bounded runtime-visible Counterfactual Pair Fractal with an explicit paired-state model and deterministic classification/render output while preserving `SampleFractalPoints(...)` and `FractalSampleResult`.
- [x] Phase 3 - prove the repaired state through focused native contract/unit/kernel/schema rails plus focused runtime publish and published-runtime proof for the new visible fractal mode.
- [ ] Phase 4 - run hostile audit, checkpoint, write receipts, clear stale closeout text, and leave the repo clean.

## Explicit User Asks

- [x] Open and execute only one bounded Counterfactual Pair Fractal implementation slice.
- [x] Keep the sample-evidence widening lane closed unless a tiny concrete seam repair is strictly required.
- [x] Preserve `FractalSampleResult` and `SampleFractalPoints(...)` as the shipped legacy projection surfaces.
- [x] Make the first paired-state model and classification output explicit enough that the result is a real runtime fractal lane, not a sidecar overlay or generic drift coloring.
- [ ] Close only with commit, validation receipt, contract proof receipt, clean tree, and a committed-head stale-plan reread.

## Proof Ledger

- Bootstrap proved branch=`feature/fractal-sample-evidence-widening`, `HEAD=4bf6397`, clean tree, fresh session, and an active locked contract still pointing at the already closed sample-evidence stop-point contract `fractal_sample_evidence_widening_explaino_balance_void_viability_owner_seam_proof`.
- Current single-orbit authority is still `FractalSampleResult` plus `SampleFractalPoints(...)` in `ui_app/src/fractal_sample_result.h` and `ui_app/src/fractal_sample_core.cu`; the renderer remains a single-sample consumer in `ui_app/src/fractal_renderer.cu`.
- Existing widened consumers in `ui_app/src/explaino_sidecar_measurement.cpp`, `ui_app/src/explaino_sidecar_lens.cpp`, and `ui_app/src/explaino_sidecar_window.cpp` do not already count as a shipped fractal implementation because they consume sample evidence off the sidecar path rather than through a runtime-visible `FractalType`.
- RED seam scan proved the repo currently has no shipped Counterfactual Pair Fractal runtime lane: `FractalType` ends at the closed Explaino selectors, schema/fractal-type ids expose no counterfactual-pair mode, and `fractal_sample_device.inl` plus `fractal_renderer.cu` remain single-sample runtime seams.
- Contract-scope preflight exposed one false boundary in the first draft: the RED touched `ui_app/tests/test_callable_engine_adversarial.cpp`, so the contract was repaired and re-locked before feature mutations continued.
- Native RED/GREEN preflight exposed a second truthful owner seam: `BuildSafeModeSchema()` still owns a shipped `fractal_type` selector and omitted `counterfactual_pair`, so the contract had to widen by exactly `ui_app/src/safe_mode_schema.cpp` before the runtime-visible lane could be proved.
- The rerun native rail exposed one more shipped continuity seam instead of a codegen bug: `ui_app/tests/test_safe_mode_schema.cpp` still encoded the pre-Explaino-all safe-mode default and never asserted the new root-finding member, so that witness had to be brought back into sync before runtime proof.
- Published-runtime proof then exposed the first actual runtime defect: `ui_app/src/diagnostics_capture.cpp` still serialized top-level `fractal_type` through a stale local switch that fell through to `"unknown"` for `counterfactual_pair`, so the contract had to widen one final time to the diagnostics exporter and its direct native witness.
- The bounded implementation candidate is a runtime-visible fractal mode whose per-pixel sampler evolves one baseline orbit plus one perturbed partner orbit together, then maps the explicit pair outcome onto a deterministic render class.
- The first paired-state model will stay bounded: one baseline state, one perturbed partner, one fixed perturbation rule, and one small set of explicit pair classes. No universal paired-state engine or multiple new fractal ideas are in scope.
- The first classification goal is bounded pair outcome rendering, not a replay of sidecar projection-flow or BalanceVoid logic. Meta-basin, Operator-Itinerary, DSL/program-space, and generic engine work remain out of scope unless a focused RED proves a tiny blocker seam.
- Minimal blocker-clearing preflight is required before mutations because the new plan/contract pair did not exist on the bootstrap head and the active lock still points at a closed prior slice.
- The first runtime build/test sweep proved the new lane is not a sidecar relabel: `test_ui_schema`, `test_schema_binding`, `test_enum_id_utils`, `test_fractal_types`, `test_fractal_sample_result`, `test_fractal_renderer`, `test_fractal_sample_core`, and `test_fractal_sample_kernel` all went green on the touched head once the safe-mode selector continuity seams were repaired.
- The broad helper sweep still stops later at `test_fractal_probe_coverage` on pre-existing Explaino runtime-id expectations for `explaino_ripple`, `explaino_splice`, `explaino_vortex`, `explaino_tension`, and `explaino_balance_void`; that probe red is outside the Counterfactual Pair contract scope and did not touch the new fractal lane.
- Focused native reruns on the repaired head are green for `test_ui_schema.exe`, `test_safe_mode_schema.exe`, `test_schema_binding.exe`, `test_enum_id_utils.exe`, `test_fractal_types.exe`, `test_fractal_sample_result.exe`, `test_fractal_renderer.exe`, `test_fractal_sample_core.exe`, `test_fractal_sample_kernel.exe`, `test_callable_engine_adversarial.exe`, and the direct exporter witness `test_diagnostics_capture.exe`.
- Focused runtime publish is green through `ui_app/build_vsdevcmd.cmd`, and published-runtime proof is green through `py -3.14 -m pytest tests/test_fractal_runtime_counterfactual_pair.py -q` after the diagnostics exporter stopped serializing `counterfactual_pair` as `"unknown"`.

## Hostile Audit

- Status: complete
- Required posture: assume the first implementation is either just a sidecar relabel, a vague drift coloring with no explicit pair semantics, an accidental replacement of `SampleFractalPoints(...)`, or a scope-drifted engine rewrite until focused proof disproves each failure mode.
- Hostile review answers:
  Did I actually implement a Counterfactual Pair Fractal, or only repackage a sidecar witness? The shipped lane is a new `FractalType::counterfactual_pair` sampler plus renderer path, and the sidecar seams remain unchanged consumers rather than the runtime authority.
  Did I prove the paired-state semantics are explicit, or let this collapse into generic drift coloring? The sampler evolves baseline and perturbed Newton partners together and classifies them into reconverged, same-basin drift, basin swap, or unstable classes that render through explicit synthetic roots.
  Did I prove the chosen widened evidence use is necessary, or reuse the engine seam by inertia? The fractal lane itself still samples through `SampleFractalPoints(...)`; widened evidence remained closed as supporting groundwork and was not reopened as a new main seam.
  Did I preserve `SampleFractalPoints(...)` as the shipped legacy projection path? Focused tests kept `FractalSampleResult` and `SampleFractalPoints(...)` green and untouched in signature/behavior.
  Did I silently widen into meta-basin, ExplainO-BalanceVoid, or a generic engine rewrite? No touched file crossed into those lanes; the only extra seams were safe-mode selector continuity and diagnostics state export needed to make the new public mode truthful.

## Audit Passes

- [x] Pass 1 - prove the RED head has no shipped Counterfactual Pair Fractal runtime lane and that the existing sidecar witnesses do not already satisfy the task.
- [x] Pass 2 - re-read the landed runtime lane for explicit paired-state semantics, deterministic class mapping, legacy-surface preservation, and out-of-scope lane drift.
- [x] Pass 3 - rerun the repaired native and published-runtime proofs, then re-read the repaired state and confirm no additional real defect found before checkpoint closure.

## Audit Findings

- [x] Audit pass 1 found a shipped continuity defect: `ui_app/src/safe_mode_schema.cpp` omitted `counterfactual_pair` from the safe-mode root-finding selector, so the new fractal was not truthfully runtime-visible across the public selector surfaces.
- [x] Audit pass 1 found the paired native witness lying about current defaults: `ui_app/tests/test_safe_mode_schema.cpp` still expected pre-Explaino-all startup state and never asserted the new root-finding member.
- [x] Audit pass 2 found a real published-runtime defect: `ui_app/src/diagnostics_capture.cpp` exported top-level `fractal_type` through a stale local switch and serialized `counterfactual_pair` as `"unknown"`.
- [x] Audit pass 3 did not find another slice-owned defect after the focused native reruns, diagnostics-capture regression, runtime republish, and published-runtime pytest rerun.
