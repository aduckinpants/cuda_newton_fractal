# ExplainO Root SDF Seed Dynamics Repair

## Current Phase

Phase 6 - focused validation, receipts, rearward review, push, and clean-tree closeout.

## Phase Checklist

- [x] Phase 1 - create this checked-in phased plan and contract, then lock the repair slice.
- [x] Phase 2 - add RED/native tests proving root-SDF auto-increment seed and generic seed animation are currently rejected by stale `IsExplainoFamily(...)` gates.
- [x] Phase 3 - replace seed-dynamics gates with root-layout seed authority without adding `explaino_root_sdf` to `IsExplainoFamily`.
- [x] Phase 4 - add runtime no-mouse proof that root-SDF auto-increment changes root/frame hashes and selector identity remains stable.
- [x] Phase 5 - hostile review the repeated UI authority anti-pattern and record the bounded follow-up.
- [ ] Phase 6 - focused validation, receipts, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [active] Auto-Increment Seed must work for `explaino_root_sdf`.
- [active] The repair must treat this as the same seed authority system as Prev/Next Seed, not as a separate hand-coded exception.
- [active] Other fractals where auto-increment still works must not regress.
- [active] Review the repeated UI issues and identify the anti-pattern the scanners missed.

## Contract Lock

This is a narrow seed-dynamics repair on `codex/explaino-root-sdf-field-lane`.

Allowed:

- Replace stale seed-dynamics predicates with a shared root-layout seed-control predicate such as `SupportsExplainoSeedControls(...)`.
- Cover both dedicated `auto_increment_seed` and generic `param_anim_target == "seed"` paths.
- Add focused native and no-mouse runtime proof.
- Document the broader anti-pattern for later refactor.

Forbidden:

- Add `explaino_root_sdf` to `IsExplainoFamily`.
- Route `explaino_root_sdf` through escape-time iteration or `RenderFractalCUDA`.
- Redesign animation UI, ExplainO Warp, SDF downsample UX, or the broader parameter metadata system.
- Use physical mouse automation.

## Proof Ledger

- Starting branch expected: `codex/explaino-root-sdf-field-lane`.
- Starting head expected: `82c8d4c811dadd0097ce3c3bcee6deb0c6669fc5`.
- Rearward review before repair: `ok`.
- Confirmed stale gates found by read-only review:
  - `ui_app/src/explaino_seed_dynamics.cpp` returns false unless `IsExplainoFamily(view.fractal_type)`.
  - `ui_app/src/param_anim_dynamics.cpp` returns false for the `seed` target unless `IsExplainoFamily(view.fractal_type)`.
- Anti-pattern under review: multiple UI/action/dynamics paths duplicate owner predicates instead of consulting a single capability predicate for "this lane consumes ExplainO root/seed authority."
- RED proof:
  - `artifacts/explaino_root_sdf_seed_dynamics_repair/red_focused_native.log` fails on root-SDF auto-increment before the production fix.
  - Focused helper friction was real: `test_explaino_seed_dynamics`, `test_param_anim_dynamics`, and `test_param_anim_generic` were present in the full sweep but were not all directly callable through `ui_app/build_tests_vsdevcmd.cmd` before this slice.
- Native proof after repair:
  - `artifacts/explaino_root_sdf_seed_dynamics_repair/focused_native.log` passes the dedicated seed dynamics, generic seed animation, and animation-target visibility rails.
- Published runtime proof after repair:
  - `artifacts/validation/explaino_root_sdf_seed_dynamics_runtime.json` reports success for `tests/test_fractal_runtime_explaino_root_sdf.py`, including the new no-mouse auto-increment seed test.
- Validation proof:
  - `artifacts/validation/explaino_root_sdf_seed_dynamics_contract.json` passes contract validation.
  - `artifacts/validation/explaino_root_sdf_seed_dynamics_code_quality.json` passes the code-quality baseline.
  - `artifacts/explaino_root_sdf_seed_dynamics_repair/diff_check.log` passes `git diff --check`.
- Implemented repair:
  - `ui_app/src/explaino_seed_dynamics.cpp` now gates auto-increment by `SupportsExplainoSeedControls(...)`.
  - `ui_app/src/param_anim_dynamics.cpp` now gates the generic `seed` animation target by `SupportsExplainoSeedControls(...)`.
  - `explaino_root_sdf` remains outside `IsExplainoFamily(...)`.

## Hostile Audit

- Status: complete

Required questions:

- Did both auto-increment seed and generic seed animation move to the same root-layout seed authority predicate? Yes; both use `SupportsExplainoSeedControls(...)`.
- Did root-SDF remain outside `IsExplainoFamily`? Yes; no family membership change was made.
- Did the runtime proof exercise auto-increment behavior, not only set-value or button actions? Yes; `test_explaino_root_sdf_auto_increment_seed_advances_no_mouse` enables auto-increment and waits for root/frame hash changes.
- Did existing ExplainO family seed animation still work? Yes; existing `test_explaino_seed_dynamics` and `test_param_anim_dynamics` ExplainO seed cases still pass.
- Did non-root/non-ExplainO lanes still reject ExplainO seed dynamics? Yes; existing Mandelbrot/no-op cases still pass.
- Did this slice avoid broad animation/UI redesign? Yes; changes are limited to capability predicates, focused helper targets, tests, and docs.

## Audit Passes

- [x] Pass 1 - hostile diff review after first implementation. Finding: focused helper target placement was broken for the newly added param-animation rails; fixed by moving those focused labels into the early dispatch region.
- [x] Pass 2 - clean re-read of repaired state. Confirmed the repaired state has the only auto-increment executor capability-gated; suspicious `main.cpp` seed action paths already used `SupportsExplainoSeedControls(...)`.
- [x] Pass 3 - clean re-read before closure. No additional real defect found in root-SDF seed-dynamics predicate coverage; broader scanner-backed cleanup remains deferred.

## Audit Findings

- [x] Real bug: root-SDF seed dynamics were split from the already-repaired Prev/Next/arrow seed controls because periodic dynamics still used `IsExplainoFamily(...)` instead of the shared seed-control capability predicate. Regression coverage was added in `test_explaino_seed_dynamics`, `test_param_anim_dynamics`, and runtime no-mouse auto-increment proof.
- [x] Test harness gap: the focused native helper could not directly run this exact seed-dynamics cluster, forcing a slow full sweep for the first RED. The slice adds focused helper targets for the three relevant native rails.

## Deferred Notes

- Broader UI authority refactor remains deferred: schema visibility, action buttons, generic animation targets, auto-dynamics, runtime reports, and capture summaries need a shared capability/consumption model instead of repeated string/predicate lists.
- Add a lightweight scanner/report that flags new `IsExplainoFamily(...)` gates inside UI/action/dynamics paths when a narrower capability predicate exists. Renderer, escape-time math, and ExplainO sidecar gates may still intentionally use family membership.
- A no-mouse keyboard-event harness remains deferred; existing runtime proof can test command/button/set-value automation but not synthetic keypresses.
