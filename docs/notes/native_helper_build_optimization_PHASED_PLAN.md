# Native Helper Build Optimization

## Current Phase

Phase 4 - Hostile audit and checkpoint validation

## Phase Checklist

- [x] Phase 1 - RED harness topology proof
- [x] Phase 2 - Shared CUDA build topology implementation
- [x] Phase 3 - Focused and grouped validation
- [x] Phase 4 - Hostile audit and checkpoint validation

## Explicit User Asks

- [closed] Reduce native helper timeout/wall-time waste as a full investment slice without reducing safety or dropping coverage.
- [closed] Preserve deterministic native/CUDA proof while avoiding repeated cold rebuilds of the same heavy CUDA sources.
- [closed] Use measured before/after evidence and keep the result durable for future slices.

## Proof Ledger

- Existing baseline: prior measured full helper success was `1625.240s`, focused `test_fractal_renderer` was `136.608s`, and `tools/viewer_host_validate_native_helper_build.py` reports 25 NVCC invocations with repeated heavy CUDA source compilation.
- Manual RED: identified focused multi-target commands silently run only `%1`, and full helper repeats heavy CUDA source/arch flag lists across many compile commands.
- Checked-in regression RED: `tests/test_native_helper_build_optimization.py` added and expected to fail on the current helper before implementation.
- First GREEN: `py -3.14 -m pytest tests/test_native_helper_build_optimization.py tests/test_agent_workflow_tools.py -q --junitxml artifacts/pytest/native_helper_build_optimization.junit.xml` passed with `48 passed in 1.49s`.
- Grouped focused proof: `ui_app/build_tests_vsdevcmd.cmd test_viewer_render_pacing test_sample_tier_resolver` passed in one helper invocation (`artifacts/validation/native_helper_multi_target_non_cuda_after_audit.json`, 3.359s).
- CUDA shared-object proof: `ui_app/build_tests_vsdevcmd.cmd test_fractal_renderer test_fractal_sample_kernel` passed in one helper invocation (`artifacts/validation/native_helper_fractal_cuda_shared_objects.json`, 195.982s).
- Generic shared-object proof: `ui_app/build_tests_vsdevcmd.cmd test_generic_equation_pack_live test_generic_equation_pack` passed in one helper invocation (`artifacts/validation/native_helper_generic_cuda_shared_object.json`, 26.324s).
- Negative focused-target proof: `ui_app/build_tests_vsdevcmd.cmd definitely_not_a_target` rejected the unknown target with exit 1 (`artifacts/validation/native_helper_unknown_target_negative.json`).
- Performance witness: full helper completed in `1021.847s` after the build topology change versus the existing `1625.240s` baseline, a `37.13%` reduction (`artifacts/validation/native_helper_full_optimized_measured.json` and `artifacts/validation/native_helper_build_optimization_validator.json`).
- Validator proof: `py -3.14 tools/viewer_host_validate_native_helper_build.py --out-json artifacts/validation/native_helper_build_optimization_validator.json` passed and now requires optimized full-helper measurement, grouped one-shell proofs, negative unknown-target proof, shared CUDA topology, and zero stale `%ERRORLEVEL%` dispatch patterns.

## Hostile Audit

- Status: complete
- Questions:
  - Did the slice preserve test coverage instead of hiding slow tests? Yes: the full helper still ran and passed; focused helper batching adds a faster path without removing executables.
  - Did the helper still build the CUDA paths with the intended architecture coverage? Yes: the centralized CUDA flags still cover `sm_86`, `sm_120`, and `sm_121`; validator enforces one shared definition.
  - Did focused native targets remain available and deterministic? Yes: grouped focused runs passed, and unknown focused target names now fail explicitly.
  - Did the public profile/task surface avoid encouraging another monolithic timeout path for ordinary slices? Partially: the helper now supports grouped targets in one setup and removes repeated heavy CUDA compiles, but the full helper is still about 17 minutes and should remain a checkpoint/full native rail rather than an every-edit loop.
  - Did the final summary honestly state any remaining full-suite cost? Required: report the `1021.847s` measured optimized full helper, not just the percent improvement.

## Audit Passes

- [done] Pass 1 - hostile review found a real defect: same-line `call ... & exit /b %ERRORLEVEL%` could report stale error state for failing focused targets.
- [done] Pass 2 - re-read the repaired state: no stale `exit /b %ERRORLEVEL%` dispatch remains, grouped focused proof passes, and unknown focused target proof rejects bad input.
- [done] Pass 3 - clean re-read of proof surfaces: validator, pytest rail, grouped proof JSONs, negative proof JSON, and full-helper measurement all agree on the repaired helper topology.

## Audit Findings

- [done] Real finding: dispatch changed from `goto` to `call`, but the first implementation used same-line `%ERRORLEVEL%` expansion that could mask a failing focused target. Fixed by using `exit /b` without stale expansion, added regression coverage, added validator check `stale_errorlevel_dispatch_count == 0`, and added an unknown-target negative proof.
- [done] Clean re-read: no additional real defect found in the repaired dispatch/topology surface after grouped compiler proof and validator rerun.

## Notes

Scope:
- Optimize `ui_app/build_tests_vsdevcmd.cmd` and directly related workflow validation surfaces.
- Add/extend tests that inspect native-helper topology and prove the helper no longer recompiles the same heavy CUDA units for every related executable.
- Preserve all existing native test executables unless an explicit follow-up test proves an equivalent grouped binary has identical coverage.

Out of scope:
- Product/runtime fractal behavior changes.
- Removing CUDA tests.
- Changing SDF, Color Pipeline, Explaino, or renderer semantics.
- Replacing the whole build system with CMake or Visual Studio solution work.

Expected implementation shape:
- Start with a checked-in RED that fails on repeated CUDA-heavy source compilation.
- Add shared object build helpers for repeated CUDA sources where safe.
- Keep full multi-architecture compile flags unless a later measured slice explicitly changes architecture policy.
- Add measurement/reporting so future agents can distinguish harness slowness from product regressions.
