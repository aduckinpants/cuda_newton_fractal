# Optimization Staging Zero-Axis

## Current Phase

Phase 3 - parameter sensitivity sweep script

## Phase Checklist

- [x] Phase 1 - zero-axis grid equivalence regression
- [x] Phase 2 - zero-axis micro-benchmark harness
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
- Phase 3 next slice:
  - add a parameter sensitivity sweep script that walks each Explaino variant from zero axis to shipped default strength and emits the morphology/cost table needed by later describe/probe work
  - keep the sweep on the extracted sample seam and reuse the benchmark case metadata/artifact layout from Phase 2 so later cost-hint work stays deterministic
- Deferred to later phases:
  - GPU timing benchmark harness
  - CSV sensitivity sweep tooling
  - cost-hint / describe-surface changes

- Phase 2 delivered:
  - added `ui_app/src/explaino_variant_benchmark.h/.cpp` as the headless case/config surface for baseline Explaino plus ripple/splice/vortex/tension zero-axis and default-strength benchmark rows
  - added `ui_app/tests/test_explaino_variant_benchmark.cpp` and wired it into `ui_app/build_tests_vsdevcmd.cmd` so the benchmark case table and runtime state builder are regression-tested in the helper matrix
  - added `ui_app/tests/bench_explaino_variants.cu` plus `ui_app/bench_explaino_variants_vsdevcmd.cmd` to compile a dedicated sample-path benchmark harness, emit `D:\salt-fractal\cuda_newton_fractal_clone\build_tests\bench_explaino_variants.csv`, and capture PTXAS register-pressure output in `D:\salt-fractal\cuda_newton_fractal_clone\build_tests\bench_explaino_variants_ptxas.log`
  - first measured table shows the shared zero-axis rows collapse near baseline Explaino cost/iteration behavior while shipped default strengths separate cleanly: ripple default ~17.6 gpu ms per 1M at ~250.5 avg iters, splice default ~13.9 gpu ms per 1M at ~8.1 avg iters, vortex default ~6.8 gpu ms per 1M at ~12.8 avg iters, and tension default ~16.7 gpu ms per 1M at ~9.5 avg iters
  - PTXAS register surface is now durable and visible from the batch runner output: `fractal_sample_kernel` used 94 registers for `sm_86` and 96 registers for `sm_120`/`sm_121` in the benchmark compile log