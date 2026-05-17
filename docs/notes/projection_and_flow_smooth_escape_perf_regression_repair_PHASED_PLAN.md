# Projection-and-Flow Fractal - Smooth-Escape and Performance Regression Repair

## Current Phase

Complete - the bounded Projection-and-Flow smooth-escape and hot-loop regression repair is committed, receipted, pushed on `feature/fractal-sample-evidence-widening`, and the committed-head stale-plan reread is clean.

## Phase Checklist

- [x] Phase 1 - open this checked-in plan and contract, replace the closed radius/class-model lock, and prove the current public smooth-escape path plus the current perf witness are both red on the bounded Projection-and-Flow lane.
- [x] Phase 2 - land one explicit Projection-and-Flow colorability repair and one bounded hot-loop repair without widening into Explaino work, color-pipeline expansion, or generic engine refactor.
- [x] Phase 3 - prove the repaired state through focused native seam tests, focused runtime publish, published-runtime Projection-and-Flow proof, and one concrete bounded before-after perf witness.
- [x] Phase 4 - run hostile audit, checkpoint, write receipts, push `feature/fractal-sample-evidence-widening`, clear stale closeout text, and leave the repo clean on the committed head.

## Explicit User Asks

- [closed] Repair the real Projection-and-Flow regression where the public smooth-escape / programmable basin path can collapse to black on meaningful stable classes.
- [closed] Repair the bounded FPS regression, or stop at one truthful bounded performance limit with concrete proof.
- [closed] Keep this bounded to Projection-and-Flow smooth-escape plus hot-loop repair rather than Explaino integration, color-pipeline expansion, renderer replacement, or generic engine refactor.
- [closed] Preserve `FractalSampleResult` and `SampleFractalPoints(...)` as the shipped legacy sampling surfaces.
- [closed] Close only with focused native proof, runtime publish, published-runtime Projection-and-Flow proof, hostile audit, checkpoint commit, machine receipts, push, clean tree, and the committed-head stale-plan gate.

## Proof Ledger

- Bootstrap proved branch `feature/fractal-sample-evidence-widening`, `HEAD=8eebd31`, clean tree, fresh session, active locked contract `projection_and_flow_radius_semantics_truthful_class_model_repair`, and the branch is `ahead 7` of `origin/feature/fractal-sample-evidence-widening`, so push is mandatory at closeout.
- Minimal blocker-clearing preflight was required because the active lock still pointed at the already closed radius/class-model repair contract; the temporary widening on that closed contract is restored before checkpoint so the closed slice stays closed on the final head.
- Public smooth-escape is currently allowed for Projection-and-Flow by the checked-in authority surfaces: `SupportsBasinColoring(...)` keeps the family on the basin branch, `IsColoringModeAllowedForFractal(...)` allows every coloring mode for basin families, and the live schema still exposes `smooth_escape` on the public coloring-mode control.
- Current runtime RED was reproduced on the shipped public surface: a 320x240 published-runtime Projection-and-Flow capture at `projection_and_flow_target_radius=1.75` still rendered eight non-black colors in the default root-basin view, but the explicit `smooth_escape` tuple (`coloring_mode=smooth_escape`, `color_signal=smooth_escape`, `color_palette=cyclic_escape`, `color_grading=escape_default`) collapsed to one all-black color with frame hash `0a0d445e35a19901a2e44f98525fa1cc771cbf3efd4ef0533ac6d8c29a4141c6`.
- The current proof bundle missed that public coloring path: `tests/test_fractal_runtime_projection_and_flow.py` only covered runtime visibility, hardening controls, and root-basin/radius-class witnesses; it carried no Projection-and-Flow smooth-escape or programmable-basin runtime proof.
- Current Projection-and-Flow class semantics are sampler-owned and synthetic: `ui_app/src/fractal_sample_device.inl` encodes the final class as a synthetic unit-root representative with `escaped=false`, and meaningful non-unit stable classes can still leave `converged=false`.
- Current color-path split was inconsistent: `ui_app/src/fractal_renderer.cu` already treated Projection-and-Flow as an explicit synthetic-class lane for `root_basin` and `joy_basins`, but `ui_app/src/escape_time_coloring.h` still blacked programmable basin pixels whenever `converged=false` and the signal was not `root_proximity` or `phase_wheel`.
- Current hot-loop cost was Projection-and-Flow-specific and bounded: the sampler paid an extra `poly_eval_real_coeffs_deg4*` at `freeZ` on every projection iteration and also recomputed `NearestRootIndexUnitRoots(projected, projectionRootCount)` every projected iteration even though only the final class was consumed.
- Current same-lane wall-time witness was red on the published runtime: five 1280x960 Projection-and-Flow headless captures at `projection_and_flow_target_radius=1.75` averaged `196.112 ms` per run with min `177.817 ms` and max `225.674 ms`.
- Landed colorability repair: `ui_app/src/fractal_renderer.cu` now passes Projection-and-Flow synthetic stable classes into the programmable basin path as colorable samples, while `ui_app/src/escape_time_coloring.h` keeps the actual convergence bit separate from the black-gate decision through the explicit `colorable` parameter and the restored legacy overload.
- Landed hot-loop repair: `ui_app/src/fractal_sample_device.inl` now uses a value-only free-step polynomial eval from `ui_app/src/polynomial_eval_real_coeffs.h` / `ui_app/src/fractal_device_math.cuh` and defers Projection-and-Flow root-sector decode until after the last successful projected point, removing the per-iteration derivative work and root decode that were not consumed by the shipped class output.
- Legacy sampling surface is still stable: `ui_app/tests/test_fractal_sample_kernel.cu` now proves Projection-and-Flow color controls do not alter `SampleFractalPoints(...)` results, and `artifacts/projection_flow_test_fractal_sample_kernel.log` is green on the repaired head.
- First hostile/native pass found a real regression in my own change: widening `MakeProgrammableBasinColor(...)` broke the existing 7-argument color-test seam in `test_escape_time_coloring.cpp`; I restored a forwarding overload in `ui_app/src/escape_time_coloring.h` and reran the focused color proof.
- Focused runtime publish is green at `artifacts/projection_flow_runtime_build_after_fix.log`, and published-runtime proof is green at `artifacts/projection_flow_runtime_after_rebuild.log` (`7 passed`).
- Focused native seam executables are green: `artifacts/projection_flow_test_escape_time_coloring.log`, `artifacts/projection_flow_test_fractal_family_rules.log`, `artifacts/projection_flow_test_fractal_renderer.log`, `artifacts/projection_flow_test_fractal_sample_kernel.log`, `artifacts/projection_flow_test_ui_schema.log`, and `artifacts/projection_flow_test_safe_mode_schema.log`.
- Same-lane performance witness is improved at `artifacts/projection_flow_perf_witness_after.json`: average wall time moved from `196.112 ms` to `173.880 ms` at `1280x960` / `projection_and_flow_target_radius=1.75` (`-22.232 ms`, `11.34%` faster) with stable frame hash `3df85e6242a65cd5103ba970e3f6ec22b15c6c1232e21502a8cdd8f33fe70ecf` across the after-fix runs.
- Closeout discipline is complete on the committed head: the final checkpoint carries fresh validation and contract-proof receipts, `feature/fractal-sample-evidence-widening` is pushed, and the committed-head stale-plan reread leaves none of the banned pre-closeout phrases in this active plan.
- Scope remained bounded: code changes stay inside Projection-and-Flow programmable basin semantics, the bounded sampler hot loop, and regression tests; `SampleFractalPoints(...)` and `FractalSampleResult` stay intact.

## Hostile Audit

- Status: complete
- Required posture: assume the first fix will only keep root-basin green, paint around the black smooth-escape bug, or call a tiny perf change "good enough" without proving the same-lane witness moved until focused proof disproves each failure mode.
- Hostile review questions:
  Did I actually repair smooth-escape / programmable Projection-and-Flow behavior, or only leave root-basin green?
  Did I actually reduce the hot-loop cost that caused the FPS cliff, or just rename it?
  Did I preserve `SampleFractalPoints(...)` and `FractalSampleResult` as the shipped legacy surfaces?
  Did I silently widen into Explaino layering, color-pipeline expansion, or a generic engine rewrite?
  Did I stop with push or stale-plan work still open?

## Audit Passes

- [x] Pass 1 - prove the public smooth-escape Projection-and-Flow path still collapses to black and the current proof bundle failed to cover it.
- [x] Pass 2 - prove the bounded hot-loop cost and capture the same-lane before-after perf witness.
- [x] Pass 3 - re-read the repaired state plus focused native/runtime proof, repair any real defect the audit finds, and rerun the hostile audit on the fixed state.

## Audit Findings

- [x] First post-implementation native pass caught a real compatibility regression: the new 8-argument `MakeProgrammableBasinColor(...)` broke `test_escape_time_coloring.cpp`; fixed by restoring the 7-argument forwarding overload and rerunning focused native/runtime proof.
- [x] No additional real defect found after clean re-read of the repaired Projection-and-Flow state and the focused proofs listed above.

## Action Hostile Review

- Action ID: projection-flow-smooth-escape-perf-regression-repair
- Suspected Failure Mode: I could repair only the root-basin happy path, leave the public smooth-escape tuple black, or claim perf recovery while the sampler still pays unnecessary per-iteration work on the bounded Projection-and-Flow lane.
- Correct Owner/Action: Change only the Projection-and-Flow programmable colorability seam plus the bounded sampler hot loop needed to preserve current class semantics and materially improve the same-lane witness.
- Proof Surface: GREEN evidence must come from focused Projection-and-Flow kernel, renderer/color, family-rule, schema, and published-runtime witnesses plus one explicit before-after performance witness on the current branch.
- Blocked Action: Explaino integration, color-pipeline feature expansion, renderer replacement, generic engine rewrite, `SampleFractalPoints(...)` replacement, or unrelated tooling work.

## Notes

- Acceptance for this slice is the focused Projection-and-Flow native/runtime seam set above rather than a broad unrelated test-farm sweep.
- Closeout discipline is complete on the final committed head; no further closeout chores remain in this plan.
