# Explaino Reflexive Sidecar

## Current Phase

Phase 3 - R2 CUDA demonstration engine

## Phase Checklist

- [x] Phase 1 - R1 headless model scaffold
- [x] Phase 2 - R1 viewer window shell
- [ ] Phase 3 - R2 CUDA demonstration engine
- [ ] Phase 4 - R3 lens and action selection
- [ ] Phase 5 - R4 divergence, energy, and persistence

## Notes

- Spec source: `spec_intake/ExplainoAll_SmartSidecar_SpecIntake.md`
- Current bounded slice:
  - add a headless sidecar model seam outside `ui_app/src/main.cpp`
  - define the R1 orientation-vector data shape
  - derive the sidecar hypothesis space from the existing describe surface
  - add focused native tests that lock discovery and ordering behavior
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
  - `ui_app/tests/test_explaino_sidecar_measurement.cpp`
  - sidecar window measurement state and initial information-budget rendering in `ui_app/src/explaino_sidecar_window.h/.cpp`
  - runtime wiring in `ui_app/src/main.cpp` that reuses the in-process CUDA sample host and avoids recomputing the micro-sweep on idle frames
  - hostile-audit repair for stale derived Explaino polynomial state during measurement variants
- Validation achieved for the current Phase 3 slice:
  - `ui_app/build_tests_vsdevcmd.cmd`
  - `ui_app/build_vsdevcmd.cmd`
  - `py -3.14 tools/code_quality_audit.py --out artifacts/code_quality_report.json`
- Next bounded slice for Phase 3:
  - convert the current output-delta score into an explicit per-param EIG estimate surface with named budget semantics
  - persist cumulative information-gain and uncertainty state across interactions instead of recomputing only the current batch
  - keep action selection and lens projection deferred until the R2 budget state is durable
- Deferred to later phases:
  - direct CUDA micro-sweep calls
  - EIG, lens projection, and autonomous action selection