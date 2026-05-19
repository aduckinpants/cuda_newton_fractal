# Fractal Parameter Surface Matrix Repair

## Current Phase

Phase 1 - standalone Julia constant-control repair is implemented and validated; Phase 2 Nova `poly_c4` is the next unresolved slice.

## Explicit User Asks

- [x] Start the parameter-surface repair campaign without claiming the whole campaign is done.
- [x] Repair the first high-confidence missing-parameter item: standalone Julia `c` real/imag controls.
- [x] Add RED native/runtime coverage before the Julia repair and prove the no-mouse visible-control path after the repair.
- [x] Keep this as cleanup and polish, not new feature expansion or a renderer rewrite.
- [x] State unfinished work explicitly at the stop point.

## Phase Checklist

- [x] Phase 0 - bootstrap, branch from the current repair baseline, and open this plan/contract.
- [x] Phase 1 - standalone Julia constant controls: add RED native/runtime coverage, implement `julia_c_real` and `julia_c_imag`, prove no-mouse rendered-frame sensitivity, and preserve existing lanes.
- [ ] Phase 2 - standalone Nova `poly_c4` policy repair: add RED coverage for quartic/custom authority, expose or explicitly block `poly_c4`, and prove runtime sensitivity.
- [ ] Phase 3 - generated all-fractal visible-control matrix: enumerate schema-visible numeric controls per fractal, classify fixed-formula lanes, and produce no-mouse set-value REDs for missing proof rows.
- [ ] Phase 4 - dead-slider repairs from the matrix, split by bounded family groups with one-at-a-time proof.
- [ ] Phase 5 - enum/combo harness support for preset/root-family/function controls, then repair or classify combo rows.
- [ ] Phase 6 - Explaino common-control matrix expansion across explicit lanes while preserving registry-axis and Color Pipeline guards.
- [x] Phase 7 - Phase 1 hostile audit, full validation, checkpoint-ready plan text, receipts-ready proof set, clean-tree target, and stale-plan guard text.

## Proof Ledger

- Starting branch: `codex/fractal-parameter-surface-matrix`.
- Starting head: `f05de97`.
- Prior closed baseline: animation target applicability cleanup is already pushed on `codex/fractal-control-surface-repair`.
- Inventory authority: `docs/notes/fractal_control_surface_audit_inventory.md`.
- Known high-confidence static suspects from the inventory:
  - `julia` hard-codes its constant and exposes no `julia_c_real` / `julia_c_imag`.
  - `nova` offers quartic/custom polynomial paths while standalone Nova hides `poly_c4`.
  - Most visible non-Magnet, non-Explaino-registry controls lack direct no-mouse rendered-frame proof.
- Required RED for Phase 1: standalone `julia` must fail because the visible-control surface lacks `julia_c_real` and `julia_c_imag`.
- Required green for Phase 1: standalone `julia` shows both controls, no-mouse set-value changes live rendered output, and fixed-formula lanes are not silently reclassified.
- Required guardrails: no physical mouse automation, no Color Pipeline regression, no Explaino-all registry regression, no broad renderer rewrite.
- RED command failed as expected: `py -3.14 tools/viewer_host_run_logged_command.py --label fractal_parameter_surface_julia_red_renderer --log artifacts/logs/fractal_parameter_surface_julia_red_renderer.log -- ui_app\build_tests_vsdevcmd.cmd test_fractal_renderer`.
- RED failure: `KernelParams` had no `julia_c_real` / `julia_c_imag`.
- Focused renderer green: `artifacts/logs/fractal_parameter_surface_julia_renderer_green.log`.
- Full native green: `artifacts/logs/fractal_parameter_surface_julia_full_native_post_audit.log`.
- Runtime publish green: `artifacts/logs/fractal_parameter_surface_julia_runtime_publish_final.log`.
- Published-runtime Julia no-mouse proof green: `artifacts/pytest/fractal_parameter_surface_matrix_julia_controls.junit.xml`.
- Published-runtime guardrails green: `artifacts/pytest/fractal_parameter_surface_matrix_runtime_guardrails.junit.xml`.
- `git diff --check` passed with line-ending warnings only.

## Hostile Audit

- Status: complete
- Required posture: assume each new slider is decorative, hidden, stale, routed to the wrong fractal, or expensive until native and published-runtime proof disprove it.

## Audit Passes

- [x] Pass 1 - inspected implementation diff and confirmed Julia controls are schema-visible only on the standalone `julia` lane.
- [x] Pass 2 - inspected runtime math, probe, perturbation, diagnostics/state, and safe-mode seams for stale standalone Julia constant ownership.
- [x] Pass 3 - inspected no-mouse harness proof and confirmed it compares renderer-owned frame hashes, not just chrome/state changes.
- [x] Pass 4 - confirmed unfinished phases remain explicit and are not implied closed by the Julia repair.
- [x] Clean re-read of the repaired state found no additional real defect in the Julia control path.

## Audit Findings

- [x] Real Phase 1 finding repaired: standalone Julia constant controls were missing from the current user surface.
- [x] Audit repair: perturbation cache key now uses the user Julia constant only when the active fractal is standalone `julia`, avoiding unrelated Mandelbrot cache churn.
- [x] Clean pass: no additional real issue found in Color Pipeline, Explaino-all registry, or animation-applicability guardrails after the repair.

## Boundaries

In scope:
- one bounded first repair for standalone Julia constant controls
- test harness additions needed to prove that first repair
- plan entries for the later matrix/Nova/dead-slider phases

Out of scope for Phase 1:
- implementing Nova `poly_c4`
- full all-fractal dead-slider repair
- enum/combo proof expansion
- Explaino common-control matrix expansion
- new fractal families or new animation features
- broad renderer or Color Pipeline redesign

## Remaining Work After Phase 1

- Phase 2: Nova `poly_c4` policy repair.
- Phase 3: generated all-fractal visible-control matrix.
- Phase 4: dead-slider repairs from that matrix.
- Phase 5: enum/combo harness support and combo rows.
- Phase 6: Explaino common-control matrix expansion.
