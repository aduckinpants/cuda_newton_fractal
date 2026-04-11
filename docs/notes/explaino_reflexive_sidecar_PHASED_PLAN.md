# Explaino Reflexive Sidecar

## Current Phase

Phase 4 - R3 lens and action selection

## Phase Checklist

- [x] Phase 1 - R1 headless model scaffold
- [x] Phase 2 - R1 viewer window shell
- [x] Phase 3 - R2 CUDA demonstration engine
- [ ] Phase 4 - R3 lens and action selection
- [ ] Phase 5 - R4 divergence, energy, and persistence

## Notes

- Spec source: `spec_intake/ExplainoAll_SmartSidecar_SpecIntake.md`
- Current bounded slice:
  - derive an exploration-completeness summary from the current posterior-uncertainty / observation-count surface
  - expose a demonstrated-vs-uncertain view in the sidecar without mutating runtime state yet
  - keep the auto-demonstration loop deferred until the passive recommendation and completeness surfaces are both durable
  - add focused native tests that lock completeness ordering, coverage buckets, and fail-fast behavior
- Exit criteria for Phase 1:
  - `SidecarOrientationVector` exists as a testable type
  - sidecar model code can derive applicable parameters from `FunctionDescriptor`
  - parameters are sorted deterministically for an initial viewer presentation
  - focused native tests cover discovery, applicability, ordering, and orientation basics
- Delivered in Phase 1:
  - `ui_app/src/explaino_sidecar_model.h/.cpp`
  - `ui_app/tests/test_explaino_sidecar_model.cpp`
  - runtime/native build wiring for the new sidecar model module
- Validation achieved:
  - `ui_app/build_tests_vsdevcmd.cmd`
  - `ui_app/build_vsdevcmd.cmd`
- Delivered in Phase 2:
  - `ui_app/src/explaino_sidecar_window.h/.cpp`
  - `ui_app/tests/test_explaino_sidecar_window.cpp`
  - runtime/native build wiring for the new sidecar window module
  - hostile-audit repairs for unsupported or empty required enum selections in the filtered describe surface
- Delivered so far in Phase 3:
  - `ui_app/src/explaino_sidecar_measurement.h/.cpp`
  - `ui_app/src/explaino_sidecar_cuda_sample_host.h/.cpp`
  - `ui_app/src/explaino_sidecar_budget.h/.cpp`
  - `ui_app/tests/test_explaino_sidecar_measurement.cpp`
  - `ui_app/tests/test_explaino_sidecar_budget.cpp`
  - sidecar window measurement state and initial information-budget rendering in `ui_app/src/explaino_sidecar_window.h/.cpp`
  - persistent budget state and EIG semantics in `ui_app/src/explaino_sidecar_window.h/.cpp`
  - runtime wiring in `ui_app/src/main.cpp` that reuses the in-process CUDA sample host and avoids recomputing the micro-sweep on idle frames
  - hostile-audit repair for stale derived Explaino polynomial state during measurement variants
  - hostile-audit repairs for mean-budget math, zero-information observation counting, empty-surface batch accounting, and budget-state preservation on update failure
- Validation achieved for the current Phase 3 slice:
  - `ui_app/build_tests_vsdevcmd.cmd`
  - `ui_app/build_vsdevcmd.cmd`
  - `py -3.14 tools/code_quality_audit.py --out artifacts/code_quality_report.json`
- Delivered so far in Phase 4:
  - `ui_app/src/explaino_sidecar_lens.h/.cpp`
  - `ui_app/src/explaino_sidecar_action.h/.cpp`
  - `ui_app/tests/test_explaino_sidecar_lens.cpp`
  - `ui_app/tests/test_explaino_sidecar_action.cpp`
  - sidecar window state now derives per-param active-zone projections from the measured EIG/budget surface
  - sidecar hypothesis-space entries now preserve describe-surface `cost_hint` metadata for passive action selection
  - sidecar budget rendering now shows lens zone and guidance columns alongside the ranked rows
  - sidecar window now exposes one passive `EIG - gamma*Cost` action recommendation over the current budget/lens surface without mutating parameters
  - hostile-audit repairs for missing budget coverage, duplicate hypothesis-surface paths, dead recommendation wiring in the sidecar window, duplicate budget-row validation in the action seam, invalid `cost_hint` metadata entering the sidecar model, and cross-surface type drift in the action seam
- Validation achieved for the current Phase 4 slice:
  - `ui_app/build_tests_vsdevcmd.cmd`
  - `ui_app/build_vsdevcmd.cmd`
  - `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
  - `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/sidecar_action_code_quality_report.json`
- Next bounded slice for Phase 4:
  - classify demonstrated versus uncertain params from posterior uncertainty and observation counts on the persistent budget surface
  - render an exploration-completeness summary or map alongside the passive recommendation without enabling autonomous parameter mutation yet
  - keep auto-demonstration deferred until the completeness surface proves durable under hostile audit
- Deferred to later phases:
  - direct CUDA micro-sweep calls
  - EIG, lens projection, and autonomous action selection