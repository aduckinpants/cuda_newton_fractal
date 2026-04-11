# Optimization Staging Zero-Axis

## Current Phase

Complete - composed Explaino perturbation chain checkpointed and validation-receipted

## Phase Checklist

- [x] Phase 1 - zero-axis grid equivalence regression
- [x] Phase 2 - zero-axis micro-benchmark harness
- [x] Phase 3 - parameter sensitivity sweep script
- [x] Phase 4 - describe/probe exposure
- [x] Phase 5 - composed-kernel follow-on

## Notes

- Spec source: `spec_intake/OptimizationStaging_ExplainoZeroAxis_SpecIntake.md`
- Phase 5 delivered:
  - `ui_app/src/fractal_probe_runner.cpp` and `ui_app/src/fractal_sample_device.inl` now route `explaino_ripple`, `explaino_splice`, `explaino_vortex`, and `explaino_tension` through one composed perturbation chain whenever any of `ripple_amplitude`, `splice_offset`, `vortex_strength`, or `tension_strength` is non-zero; plain `explaino` still ignores latent variant params
  - `ui_app/src/fractal_derived_fields.cpp` now emits `poly_coeffs_b` for splice-active composed-capable Explaino states so splice alternation works behind any legacy Explaino variant label
  - `ui_app/tests/test_explaino_zero_axis_equivalence.cu` now proves secondary-only reduction, plain Explaino latent-param isolation, and multi-active label invariance; final helper validation recorded `test_explaino_zero_axis_equivalence: 110 passed, 0 failed`
  - `ui_app/tests/test_fractal_probe.cpp` adds matching host probe reductions and composed-label invariance coverage so the probe sampler stays aligned with the shared device/runtime seam
  - this slice intentionally keeps composition internal behind the existing legacy Explaino variant labels; no new schema/describe/UI surface or public `explaino_composed` fractal type was introduced yet
  - closure validation is green with `ui_app/build_tests_vsdevcmd.cmd`, `ui_app/build_vsdevcmd.cmd`, `py -3.14 -m pytest tests/test_fractal_runtime_probe_cli.py -q`, and `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/phase5_composed_code_quality_report_audit2.json`
- Phase 1 delivered:
  - added `ui_app/tests/test_explaino_zero_axis_equivalence.cu` as a headless CUDA-path regression over a shared 256x256 coordinate grid for baseline `explaino` plus `explaino_ripple`, `explaino_splice`, `explaino_vortex`, and `explaino_tension`
  - proved the root surface already matched at zero axis, then exposed a real runtime defect where zero-valued variants still ran their branch-local solver substrate instead of collapsing to baseline Explaino
  - repaired the shared runtime path in `ui_app/src/fractal_sample_device.inl` by normalizing zero-valued variant dispatch to `FractalType::explaino` before iteration, which fixes both sample and renderer consumers of the shared device seam
  - wired the regression into `ui_app/build_tests_vsdevcmd.cmd` so the helper matrix executes it deterministically
- Deferred to later phases:
  - GPU timing benchmark harness
  - CSV sensitivity sweep tooling
  - any future public `explaino_composed` schema/describe/UI surface beyond the internal composed routing landed here

- Phase 2 delivered:
  - added `ui_app/src/explaino_variant_benchmark.h/.cpp` as the headless case/config surface for baseline Explaino plus ripple/splice/vortex/tension zero-axis and default-strength benchmark rows
  - added `ui_app/tests/test_explaino_variant_benchmark.cpp` and wired it into `ui_app/build_tests_vsdevcmd.cmd` so the benchmark case table and runtime state builder are regression-tested in the helper matrix
  - added `ui_app/tests/bench_explaino_variants.cu` plus `ui_app/bench_explaino_variants_vsdevcmd.cmd` to compile a dedicated sample-path benchmark harness, emit `D:\salt-fractal\cuda_newton_fractal_clone\build_tests\bench_explaino_variants.csv`, and capture PTXAS register-pressure output in `D:\salt-fractal\cuda_newton_fractal_clone\build_tests\bench_explaino_variants_ptxas.log`
  - first measured table shows the shared zero-axis rows collapse near baseline Explaino cost/iteration behavior while shipped default strengths separate cleanly: ripple default ~17.6 gpu ms per 1M at ~250.5 avg iters, splice default ~13.9 gpu ms per 1M at ~8.1 avg iters, vortex default ~6.8 gpu ms per 1M at ~12.8 avg iters, and tension default ~16.7 gpu ms per 1M at ~9.5 avg iters
  - PTXAS register surface is now durable and visible from the batch runner output: `fractal_sample_kernel` used 94 registers for `sm_86` and 96 registers for `sm_120`/`sm_121` in the benchmark compile log

- Phase 3 delivered:
  - added `tools/reality_toolkit/fractal_explorer/explaino_param_sensitivity.py` as the headless Python sweep seam for ripple/splice/vortex/tension, including the Phase 2 benchmark case ids/default strengths so later describe work can stay aligned with the measured zero/default artifact surface
  - added `tools/reality_toolkit/scripts/run_explaino_param_sensitivity.py` as the thin CLI wrapper that runs per-variant `fractal.sample` `sequence_grid` requests and writes deterministic artifact bundles under a repo-local output directory
  - the new sweep writes per-variant `probe_request.json`, `probe_response.json`, `probe_manifest.csv`, and `probe_summary.json` files plus a combined `explaino_variant_sensitivity.csv` / `explaino_variant_sensitivity_summary.json` table that records `mean_iterations`, `escape_fraction`, `converged_fraction`, `nonfinite_fraction`, `pole_fraction`, and computed `root_entropy_bits`
  - root morphology diversity now comes from the existing probe contract's per-sample `root_index` metric, so Phase 3 did not need a new native API surface to compute Shannon entropy over converged basin membership
  - added `tests/test_explaino_param_sensitivity.py`; targeted validation is green with `py -3.14 -m pytest tests/test_fractal_param_probe_sweep.py tests/test_explaino_param_sensitivity.py -q`, and the new CLI wrapper smoke-tested successfully in dry-run mode via `py -3.14 tools/reality_toolkit/scripts/run_explaino_param_sensitivity.py --variant explaino_ripple --ticks 2 --dry-run --out-dir artifacts/explaino_param_sensitivity_smoke`

- Phase 4 delivered:
  - `ui_app/src/function_descriptor.h/.cpp` now carries measured `cost_hint` plus a structured `sensitivity` payload for `fractal.params.ripple_amplitude`, `splice_offset`, `vortex_strength`, and `tension_strength`, backed by the Phase 2 benchmark ratios and the Phase 3 live 5-tick sensitivity sweep data
  - the probe contract now supports `sequence.mode = "variant_crossfade"` for Explaino ripple/splice/vortex/tension transitions, expanding each request into an explicit odd-step path that ramps the source variant down to plain `explaino` at the midpoint and ramps the target variant back up to its shipped default strength
  - the runner keeps the old V1 guard for generic sequence axes by applying crossfade fractal-type changes through a dedicated internal path instead of permitting ordinary `vary` overrides to mutate `fractal.view.fractal_type`
  - closure validation is green: `ui_app/build_tests_vsdevcmd.cmd` (`artifacts/variant_crossfade_build_tests.log` ends with `All helper tests passed.`), `ui_app/build/test_fractal_probe.exe`, `ui_app/build_vsdevcmd.cmd`, `py -3.14 -m pytest tests/test_fractal_runtime_probe_cli.py -q`, and `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json`