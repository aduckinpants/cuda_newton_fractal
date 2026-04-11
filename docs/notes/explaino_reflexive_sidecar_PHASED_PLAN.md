# Explaino Reflexive Sidecar

## Current Phase

Phase 2 - R1 viewer window shell

## Phase Checklist

- [x] Phase 1 - R1 headless model scaffold
- [ ] Phase 2 - R1 viewer window shell
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
- Next bounded slice for Phase 2:
  - add a secondary ImGui sidecar window fed by the headless model
  - render the discovered applicable params in deterministic order
  - keep the window logic extracted from `ui_app/src/main.cpp` as much as possible
- Deferred to later phases:
  - direct CUDA micro-sweep calls
  - EIG, lens projection, and autonomous action selection