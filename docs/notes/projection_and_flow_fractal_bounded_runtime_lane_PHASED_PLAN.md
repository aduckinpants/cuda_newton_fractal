# Projection-and-Flow Fractal - Bounded Runtime Lane

## Current Phase

Complete - the bounded Projection-and-Flow runtime lane is landed, hostile-audited, and proved through focused native, callable-surface, runtime-publish, and published-runtime witnesses.

## Phase Checklist

- [x] Phase 1 - open this checked-in plan and contract, replace the closed Explaino Counterfactual Pair lock, and prove the RED head still has no shipped Projection-and-Flow runtime lane beyond the existing sidecar projection-flow witnesses.
- [x] Phase 2 - land one bounded runtime-visible Projection-and-Flow fractal with an explicit cubic free-evolution plus unit-circle projection model, explicit class rendering, and preserved `SampleFractalPoints(...)` / `FractalSampleResult` legacy surfaces.
- [x] Phase 3 - prove the lane through the narrow native/unit/kernel/schema rails plus focused runtime publish and published-runtime proof for the new visible fractal mode.
- [x] Phase 4 - run hostile audit, checkpoint, write receipts, clear stale closeout text, and leave the repo clean on the committed head.

## Explicit User Asks

- [closed] Open and execute only one bounded Projection-and-Flow Fractal implementation slice.
- [closed] Keep the sample-evidence widening lane, Counterfactual Pair lanes, ExplainO-BalanceVoid, meta-basin, Operator-Itinerary, DSL/program-space, renderer replacement, and broad engine refactor work out of scope unless a tiny blocker seam proves otherwise.
- [closed] Preserve `FractalSampleResult` and `SampleFractalPoints(...)` as the shipped legacy projection surfaces.
- [closed] Make the free-evolution plus projection model and the classification/render output explicit enough that the result is a real fractal lane, not a sidecar relabel or generic drift coloring.
- [closed] Close only with focused native proof, runtime publish, published-runtime proof, hostile audit, checkpoint commit, machine receipts, and the committed-head stale-plan gate.

## Proof Ledger

- Bootstrap proved branch=`feature/fractal-sample-evidence-widening`, `HEAD=0ec2b55`, clean tree, fresh session, and an active locked contract still pointing at the already closed Explaino Counterfactual Pair contract `counterfactual_pair_explaino_pattern_variant`.
- Current single-orbit authority is still `FractalSampleResult` plus `SampleFractalPoints(...)` in `ui_app/src/fractal_sample_result.h` and `ui_app/src/fractal_sample_core.cu`; this slice must preserve those shipped legacy surfaces.
- Current widened evidence authority remains bounded to `sample_coord + legacy_result`, and the closed sidecar projection-flow witnesses live only in `ui_app/src/explaino_sidecar_measurement.cpp`, `ui_app/src/explaino_sidecar_lens.cpp`, and `ui_app/src/explaino_sidecar_window.cpp`; there is still no shipped `FractalType`, enum id, schema option, or runtime proof for a Projection-and-Flow fractal lane.
- Current widened payload is sufficient without more engine widening for the chosen first runtime lane because the bounded implementation stays truthfully single-orbit: it emits no richer evidence than the existing legacy sample result and therefore must not reopen `SampleFractalPoints(...)`, `FractalSampleResult`, or `FractalSampleEvidence` as the main work.
- Chosen first state model: fixed cubic `z^3 - 1` Newton free evolution followed by explicit radial projection back onto the unit circle after every free step, i.e. `z_free = z - P(z) / P'(z)` and `z_next = z_free / |z_free|` for finite nonzero `z_free`.
- Chosen first classification/render output: terminal cubic root sector crossed with a bounded projection-pressure bucket (`low` versus `high` cumulative projection distance), plus one explicit unstable class when the projected orbit does not converge.
- `SampleFractalPoints(...)` can remain untouched in signature and behavior on this lane because the runtime math change stays inside `ui_app/src/fractal_sample_device.inl` and the renderer still consumes the legacy sample result.
- The RED head proved the lane was missing: `ui_app/build_tests_vsdevcmd.cmd` failed immediately on `FractalType::projection_and_flow` being undefined, which is the expected no-runtime-lane witness after bootstrap.
- The landed runtime authority is now explicit and bounded:
  - `ui_app/src/fractal_sample_device.inl` owns the cubic `z^3 - 1` free-evolution plus unit-circle projection recurrence and encodes the seven synthetic runtime classes.
  - `ui_app/src/fractal_renderer.cu` owns the seven-class root-basin rendering surface for the new lane without replacing the renderer contract.
  - `ui_app/src/fractal_runtime_validation.h` fences the lane to `PolyKind::z3_minus_1`, so the first slice does not silently widen into a generic projection framework.
  - `ui/fractal_binding_surface_v1.ui_schema.json`, `ui_app/src/safe_mode_schema.cpp`, and `ui_app/src/enum_id_utils.h` expose the runtime-visible selector surface.
- `SampleFractalPoints(...)`, `FractalSampleResult`, `ui_app/src/fractal_sample_core.cu`, and `ui_app/src/fractal_sample_result.h` remained untouched; legacy projection authority stayed on the shipped adapter while the new math lived entirely behind it.
- Focused native proof is green on the bounded lane:
  - targeted contract/unit/schema commands passed for `test_fractal_runtime_validation`, `test_fractal_family_rules`, `test_enum_id_utils`, `test_fractal_types`, `test_safe_mode_schema`, `test_schema_binding`, and `test_ui_schema`
  - targeted CUDA commands passed for `test_fractal_renderer` and `test_fractal_sample_kernel`
  - targeted callable-surface proof passed for `test_callable_engine_adversarial`
- Published-runtime proof is green: `ui_app/build_vsdevcmd.cmd` staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`, and `py -3.14 -m pytest -q tests\test_fractal_runtime_projection_and_flow.py` passed against that published runtime.
- Minimal blocker-clearing preflight is required before RED mutations because the new plan/contract pair did not exist on the bootstrap head and the active locked scope still belongs to the closed Explaino Counterfactual Pair slice.

## Hostile Audit

- Status: complete
- Required posture: assume the first implementation will only rename sidecar witnesses, hide the projection rule inside opaque sampler code, collapse into generic drift coloring, silently replace the legacy sample surface, or stop with stale closeout text unless focused proof disproves each failure mode.

## Audit Passes

- [x] Pass 1 - proved the RED head still had no shipped Projection-and-Flow runtime lane and that the sidecar projection-flow witnesses did not already satisfy the task.
- [x] Pass 2 - proved the landed lane exposes an explicit projection rule and explicit class output instead of generic drift coloring through targeted runtime-validation, kernel, renderer, schema, and runtime-publish witnesses.
- [x] Pass 3 - real issue found and repaired: `test_safe_mode_schema.cpp` still asserted Counterfactual-Pair-only visibility on controls the checked-in safe-mode schema already shares with `explaino_counterfactual_pair`.
- [x] Pass 4 - real issue found and repaired: `test_fractal_sample_kernel.cu` depended on `ApplyFractalPresetDefaults` / `UpdateExplainoPolynomial` across a link seam the CUDA test target does not link, so the bounded defaults were made local and explicit.
- [x] Pass 5 - clean re-read of the repaired state found no additional real defect in fractal identity, explicit pressure-bucket semantics, legacy sample-surface preservation, scope fence, or published-runtime proof.

## Audit Findings

- [x] Real issue found and repaired: the safe-mode schema test had drifted behind the checked-in `counterfactual_pair,explaino_counterfactual_pair` carrier fence and would have reported a false red unrelated to the new lane’s truth.
- [x] Real issue found and repaired: the CUDA kernel test was not self-contained for the Explaino Counterfactual Pair setup it exercised, so it was repaired to carry the bounded defaults locally instead of depending on an unlinked preset helper seam.
- [x] Clean re-read of the repaired state: no additional real defect found in the Projection-and-Flow runtime lane, the explicit pressure-threshold split, the preserved `SampleFractalPoints(...)` / `FractalSampleResult` legacy surface, or the narrowed scope fence.

## Notes

- Exit criteria:
  - one bounded runtime-visible `projection_and_flow` lane exists
  - the cubic free-evolution plus unit-circle projection rule is explicit and proved
  - the class output is explicit and deterministic
  - `SampleFractalPoints(...)` and `FractalSampleResult` remain the shipped legacy projection surfaces
  - focused native rails plus runtime publish plus published-runtime proof are green
  - checkpoint commit, machine receipts, hostile-audit proof, and the committed-head stale-plan gate are complete
- Expected first REDs:
  - no `projection_and_flow` enum id or selector/runtime lane exists on the current head
  - sidecar `projection_flow_bias` is still diagnostic only, not a runtime-visible fractal lane
  - current widened payload does not force additional engine widening for this bounded lane
  - `SampleFractalPoints(...)` remains the shipped legacy projection path
  - Counterfactual Pair, ExplainO-BalanceVoid, meta-basin, Operator-Itinerary, and DSL/program-space remain out of scope
