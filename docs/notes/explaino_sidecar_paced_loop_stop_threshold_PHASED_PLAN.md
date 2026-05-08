# Explaino Sidecar Paced-Loop Stop Threshold

## Current Phase

Phase 1 queued - isolate why a zero-coverage stop threshold still allows at least one paced-loop mutation before the sidecar stops

## Phase Checklist

- [ ] Phase 1 - prove the zero-threshold paced-loop bug on the focused headless and live runtime seams
- [ ] Phase 2 - repair the sidecar controller so a zero-threshold stop point halts before any runtime mutation is applied
- [ ] Phase 3 - rerun the bounded runtime proofs and checkpoint the slice cleanly

## Explicit User Asks

- [open] Open a separate bounded slice for the unrelated paced-loop stop-threshold failure.
- [open] Keep this sidecar bug separate from the already-fixed Explaino `escape_magnitude` semantics work.
- [open] Prove the real runtime behavior instead of treating the failing audit as a vague follow-up note.

## Presumption Loop

The local hypothesis is that the sidecar auto-demo controller still arms or applies one paced-loop mutation before the zero-threshold completeness stop point is honored. The cheapest disconfirming checks are the existing focused published-runtime tests in `tests/test_fractal_runtime_explaino_escape_variants.py` and `tests/test_fractal_runtime_explaino_sidecar_live.py`, which already show `params.multibrot_power` and the live frame mutating when `stop_demonstrated_fraction=0.0` and `stop_uncertain_count=0` should stop the loop immediately.

## Presumption Evidence

- `ui_app/src/explaino_sidecar_controller.cpp` owns `HasCompletenessStopPoint(...)` and `AdvanceSidecarAutoDemoLoop(...)`, the narrow control path that decides whether the paced loop should still apply runtime mutations.
- The hostile audit on the Explaino `escape_magnitude` slice exposed a headless published-runtime failure where the zero-threshold stop policy still mutates `params.multibrot_power`.
- `tests/test_fractal_runtime_explaino_sidecar_live.py` already carries the matching live-runtime visual stability witness for the same policy.

## Proof Ledger

- Audit finding carried forward: `tests/test_fractal_runtime_explaino_escape_variants.py::test_explaino_sidecar_headless_paced_loop_respects_stop_threshold` fails because zero-threshold stop policy still mutates at least one numeric state field.
- The likely owner seam is the sidecar controller, not the Explaino color pipeline or diagnostics capture harness.

## Notes

- Expected owner files for this follow-up:
  - `docs/contracts/explaino_sidecar_paced_loop_stop_threshold.contract.json`
  - `docs/notes/explaino_sidecar_paced_loop_stop_threshold_PHASED_PLAN.md`
  - `ui_app/src/explaino_sidecar_controller.cpp`
  - `ui_app/src/explaino_sidecar_controller.h`
  - `tests/test_fractal_runtime_explaino_escape_variants.py`
  - `tests/test_fractal_runtime_explaino_sidecar_live.py`
- Non-goals for this follow-up:
  - do not reopen the Explaino `escape_magnitude` owner seam
  - do not broaden into unrelated sidecar UX or palette work
  - do not silently waive the live-runtime proof requirement

## Resume Point

Start from the published-runtime headless failure at `tests/test_fractal_runtime_explaino_escape_variants.py::test_explaino_sidecar_headless_paced_loop_respects_stop_threshold`, confirm the first mutation slips past the zero-threshold stop gate, then repair the controller before rerunning both headless and live runtime proofs.