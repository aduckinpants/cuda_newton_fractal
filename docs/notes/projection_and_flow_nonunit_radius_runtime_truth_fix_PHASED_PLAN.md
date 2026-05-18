# Projection-and-Flow Fractal - Non-Unit Radius Runtime Truth Fix

## Current Phase

Closed - the focused kernel seam, published-runtime pytest proof, contract validation, hostile-audit validation, machine receipts, and stale-plan gate are all complete for the bounded Projection-and-Flow non-unit radius truth fix.

## Phase Checklist

- [x] Phase 1 - open this checked-in follow-up plan and contract, replace the closed hardening lock, and prove the current runtime collapses non-unit Projection-and-Flow radii into a useless single-class render.
- [x] Phase 2 - repair the bounded Projection-and-Flow state/class semantics so non-unit radii still produce truthful explicit classes without replacing `SampleFractalPoints(...)` or widening into generic engine work.
- [x] Phase 3 - prove the repaired state through focused native seam tests plus focused runtime publish and published-runtime Projection-and-Flow proof.
- [x] Phase 4 - run hostile audit, checkpoint, write receipts, clear stale closeout text, and leave the repo clean on the committed head.

## Explicit User Asks

- [resolved] Fix the review-found defect where the Projection-and-Flow radius control only gives a useful rendered result at exactly `1.0`.
- [resolved] Keep this bounded to Projection-and-Flow runtime truth rather than widening into Explaino, color-pipeline work, or a broad engine refactor.
- [resolved] Preserve `FractalSampleResult` and `SampleFractalPoints(...)` as the shipped legacy projection surfaces.
- [resolved] Close only with focused native proof, runtime publish, published-runtime proof, hostile audit, checkpoint commit, machine receipts, clean tree, and the committed-head stale-plan gate.

## Proof Ledger

- Repo status at follow-up open is clean on branch `feature/fractal-sample-evidence-widening`; the active locked contract still points at the already closed `projection_and_flow_fractal_semantics_control_surface_hardening` slice and must be replaced before mutation.
- Current published runtime proof reproduces the user-reported defect: loading `projection_and_flow_target_radius` values `0.25`, `0.5`, `0.75`, `1.25`, `1.75`, `2.5`, and `4.0` yields the same single-color frame hash `31ecd83bd569aec2e9593e30ab5af5efd8b318533b59493d92753a605e75c5e5`, while the default radius `1.0` yields a six-color frame.
- The collapse is not a renderer gate: `ui_app/src/fractal_renderer.cu` treats `projection_and_flow` as an explicit synthetic-class lane regardless of `converged`.
- The collapse is currently sampler-owned in `ui_app/src/fractal_sample_device.inl`: Projection-and-Flow only assigns a root-sector class when the orbit actually converges to a unit-root solution, which makes non-unit radii effectively self-defeating because each projection step moves the state back off the root manifold and leaves most samples in the synthetic unstable class.
- The bounded owner seams for this fix are `ui_app/src/fractal_sample_device.inl`, `ui_app/tests/test_fractal_sample_kernel.cu`, and `tests/test_fractal_runtime_projection_and_flow.py`; `SampleFractalPoints(...)` and `FractalSampleResult` must remain unchanged.
- GREEN sampler fix: Projection-and-Flow now assigns its public root-sector class from the last valid projected orbit state, not only from literal root convergence, so non-unit radii still produce explicit `root sector x pressure bucket` classes while `converged` remains reserved for actual polynomial-root convergence.
- Focused native proof is green on the rebuilt `D:\salt-fractal\cuda_newton_fractal_clone\build_tests\test_fractal_sample_kernel.exe`: `passed=711 failed=0`, including the new non-unit-radius witness that requires stable explicit classes, multiple root sectors, and at least one stable sample with `converged == false`.
- Focused runtime publish plus published-runtime proof are green: `ui_app/build_vsdevcmd.cmd` rebuilt the runtime, `pytest tests/test_fractal_runtime_projection_and_flow.py -q` passed `5/5`, and direct capture receipts now show `projection_and_flow_target_radius=0.75` and `1.75` both render three unique colors instead of collapsing to one.
- Closure validation is complete on the committed head: `artifacts/validation/projection_and_flow_nonunit_radius_runtime_truth_fix_contract.json`, `artifacts/validation/viewer_host_assert_phased_plan_sync.json`, and `artifacts/validation/projection_and_flow_nonunit_radius_runtime_truth_fix_hostile_audit.json` are green, and machine validation plus contract-proof receipts have been written under `artifacts/hooks/viewer_host_validation_receipts/` and `artifacts/hooks/viewer_host_contract_proof_receipts/`.

## Hostile Audit

- Status: complete
- Required posture: assume the fix will only hide the defect behind a hash change, lie about convergence, or widen the Projection-and-Flow state model beyond the bounded lane unless focused proof disproves each failure mode.
- Hostile review questions:
  Did I actually make non-unit radii useful, or only swap one trivial frame hash for another?
  Did I keep the class semantics explicit, or reintroduce hidden sampler-only behavior?
  Did I preserve `SampleFractalPoints(...)` as the shipped legacy projection path?
  Did I silently widen into Explaino, color-pipeline work, or a generic engine rewrite?
  Did I stop with stale closeout text again?

## Audit Passes

- [x] Pass 1 - prove the current runtime and kernel witnesses collapse non-unit radii into a useless single-class result.
- [x] Pass 2 - clean re-read the repaired Projection-and-Flow state/class semantics for truthfulness, bounded scope, and legacy-surface preservation.
- [x] Pass 3 - rerun the repaired native and published-runtime proofs, then re-read the repaired state and confirm a real defect was fixed before closure or that no further slice-owned defect remains.

## Audit Findings

- [x] Projection-and-Flow non-unit radii were genuinely broken: focused RED witnesses proved the published runtime and kernel both collapsed non-unit target radii into the synthetic unstable bucket, making the public radius control effectively useless outside `1.0`.
- [x] The repaired sampler is still bounded and truthful: only `ui_app/src/fractal_sample_device.inl` changed runtime semantics, `SampleFractalPoints(...)` and `FractalSampleResult` stayed untouched, and the kernel witness now proves stable public classes can exist for non-unit radii without falsely claiming actual root convergence.
- [x] No further slice-owned defect was found after rereading the bounded diff plus rerunning the focused kernel/runtime proofs, and the final closeout mechanics were completed on the committed head.

## Action Hostile Review

- Action ID: projection-flow-nonunit-radius-green-1
- Suspected Failure Mode: I could make the frame hash differ for non-unit radii while leaving the lane effectively trivial, conflate synthetic class stability with actual root convergence, or widen into unrelated renderer/color/Explaino work.
- Correct Owner/Action: Change only the bounded Projection-and-Flow sampler semantics and the focused kernel/runtime witnesses needed to prove non-unit radii still yield explicit public classes while keeping the shipped sample/result seam intact.
- Proof Surface: GREEN evidence must come from `ui_app/tests/test_fractal_sample_kernel.cu` and `tests/test_fractal_runtime_projection_and_flow.py`, plus focused runtime publish and the published-runtime Projection-and-Flow witness.
- Blocked Action: Mutate Projection-and-Flow runtime semantics or focused witnesses outside the bounded owner seams above.

## Notes

- Expected REDs to lock before implementation:
  1. non-unit radius runtime captures collapse to a single rendered class
  2. kernel witnesses expose only the synthetic unstable class for non-unit radii
  3. `SampleFractalPoints(...)` and legacy callers remain on their shipped semantics
  4. Explaino, color-pipeline work, and generic engine widening remain out of scope
