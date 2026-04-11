# Optimization Staging Zero-Axis

## Current Phase

Phase 2 - zero-axis micro-benchmark harness

## Phase Checklist

- [x] Phase 1 - zero-axis grid equivalence regression
- [ ] Phase 2 - zero-axis micro-benchmark harness
- [ ] Phase 3 - parameter sensitivity sweep script
- [ ] Phase 4 - describe/probe exposure
- [ ] Phase 5 - composed-kernel follow-on

## Notes

- Spec source: `spec_intake/OptimizationStaging_ExplainoZeroAxis_SpecIntake.md`
- Phase 1 delivered:
  - added `ui_app/tests/test_explaino_zero_axis_equivalence.cu` as a headless CUDA-path regression over a shared 256x256 coordinate grid for baseline `explaino` plus `explaino_ripple`, `explaino_splice`, `explaino_vortex`, and `explaino_tension`
  - proved the root surface already matched at zero axis, then exposed a real runtime defect where zero-valued variants still ran their branch-local solver substrate instead of collapsing to baseline Explaino
  - repaired the shared runtime path in `ui_app/src/fractal_sample_device.inl` by normalizing zero-valued variant dispatch to `FractalType::explaino` before iteration, which fixes both sample and renderer consumers of the shared device seam
  - wired the regression into `ui_app/build_tests_vsdevcmd.cmd` so the helper matrix executes it deterministically
- Phase 2 next slice:
  - add a zero-axis micro-benchmark harness that measures baseline and default-cost behavior for the four Explaino variants on the extracted sample seam
  - emit a durable table of `{variant, param_value, mean_iters, gpu_ms_per_1M}` suitable for later describe/probe and sidecar cost-hint work
- Deferred to later phases:
  - GPU timing benchmark harness
  - CSV sensitivity sweep tooling
  - cost-hint / describe-surface changes