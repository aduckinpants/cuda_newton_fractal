# Generic Sampler Forward Path

## Current Phase

Phase 4 - add execution-level backend preference and provenance

## Phase Checklist

- [x] Phase 1 - lock contract and build CPU/CUDA parity rails
- [x] Phase 2 - extract shared execution and response marshalling seams
- [x] Phase 3 - professionalize iterate count semantics
- [ ] Phase 4 - add execution-level backend preference and provenance
- [ ] Phase 5 - document temporary gallery scaffolding and close continuity

## Notes

- Why this plan exists:
  - `generic.sample` now has broader probe coverage and a useful generic-only gallery helper, but the public execution path is still CPU-backed and the iterate-count semantics are still too permissive for the long-term scientific-tool posture
  - the next work must improve the runtime carefully: prove CPU/CUDA parity first, keep the initial backend rollout explicit and observable, and avoid quietly turning the current gallery helper into a permanent product surface
- Product boundary:
  - keep this initiative on the existing `generic.sample` callable surface, its CPU/CUDA execution seams, and the minimal documentation needed to mark the current gallery helper as temporary scaffolding
  - do not reopen dynamic kernel registration, add a new fractal family, or formalize a permanent gallery audit contract in the same slice
- Requirements locked on 2026-04-14:
  - the public backend control for `generic.sample` should stay low-prominence: request-side `execution.backend_preference` with `default | cpu | cuda`, not a permanent hard-coded CPU/CUDA function parameter
  - the runtime owns the meaning of `default`; current policy may resolve it to CPU today and later to CUDA without breaking callers that omit the field
  - successful responses must report the actual executor via `runtime.backend_used`
  - `iterate(body, count)` needs professional semantics: count should become strict and inspectable rather than silently clamped or repaired
  - the current generic gallery output path remains useful temporary scaffolding; it should be written down and used, but not enshrined as the long-term output contract
- Current stop point:
  - landed this plan plus a small user-facing note in the dynamic-function cheatsheet that the current generic gallery helper and `gallery_manifest.json` path are temporary scaffolding rather than the final output contract
  - landed `ui_app/tests/test_generic_sample_parity.cu`, which compares the current public CPU semantics against `SampleGenericFunction()` on representative direct-eval, compose, Newton-style, and quadratic-iterate expressions built from the shipped parser surface
  - the checked-in native rail now compiles and runs that parity executable via `ui_app/build_tests_vsdevcmd.cmd`
  - hostile-review finding: the first parity attempt failed only on forward-difference derivative tolerances; observed CPU-vs-CUDA deltas stayed in the low `1e-8` relative range, so the parity rail now uses a dedicated derivative tolerance of `1e-7` while keeping tighter `1e-10` parity checks on sampled values and `abs2`
  - landed Phase 2 seam extraction inside `ui_app/src/fractal_probe_runner.cpp`: generic.sample request preparation, response skeleton creation, per-sequence parameter application, root-level CPU evaluation, and GenericSampleResult-to-FractalProbeSample marshalling now live in dedicated internal helpers while the public path stays CPU-default
  - added a characterization guard in `ui_app/tests/test_generic_probe.cpp` proving that summary-only generic.sample requests suppress sample payloads while preserving summary metrics, which protects the extracted response-skeleton and metric-selection seams
  - validation confirmed no public behavior change across the native rail, runtime build, focused Python regressions, and the existing parity harness
  - landed Phase 3 strict iterate-count semantics across the parser, descriptor validation, CPU reference path, public generic.sample runner, CUDA sampler core, and the parity mirror: `iterate(body, count)` now accepts only finite integers in `[1, 10000]`, supports bare scalar-param counts, and fails fast instead of silently clamping or truncating invalid values
  - extended the parser, native probe, CLI, and CPU/CUDA parity rails to cover valid parameterized counts plus missing, zero, negative, fractional, oversized, and non-finite count failures
  - hostile-review finding: the first Phase 3 rerun exposed a bad new acceptance test rather than a runtime bug. The test incorrectly expected repeated squaring from `iterate(z, steps)`, which is the identity map; the acceptance surface was corrected to use `iterate(z * z, steps)` with a 4-step count so the assertion actually proves parameterized iteration semantics
  - validation confirmed the strict-count change across the native rail, published runtime build, and focused CLI regression file
  - next step: validate the landed execution-preference surface and close the Phase 4 continuity with the low-prominence `execution.backend_preference` request shape, explicit `runtime.backend_used` provenance, and the build wiring needed to link the standalone generic CUDA sampler into the public runner
- Phase 1 exit criteria:
  - an in-repo plan records the rollout and semantics decisions above
  - a focused native parity harness compares the current public CPU semantics against the existing CUDA sampler core for representative shipped expressions
  - the native build rail runs that parity harness automatically
- Phase 2 exit criteria:
  - generic request preparation, backend execution, and response marshalling are separated enough that CPU and CUDA paths can share one public response contract without duplicated bookkeeping
  - CPU remains the default backend during this extraction phase
- Phase 3 exit criteria:
  - iterate-count handling is strict enough for a professional scientific tool: invalid counts hard-error instead of silently clamping or truncating
  - parser, evaluator, and public request regressions cover both valid parameterized counts and invalid-count failures
- Phase 4 exit criteria:
  - the public `generic.sample` path accepts `execution.backend_preference = default | cpu | cuda`, with `default` remaining runtime-policy-owned
  - the path can be pinned explicitly to CUDA while preserving the same response semantics proven by the parity suite
  - backend selection is observable through `runtime.backend_used`, documented, and explicit backend failures are not silently hidden
- Phase 5 exit criteria:
  - the current gallery helper is documented as temporary scaffolding with its present output contract and deferred future-tool responsibilities called out explicitly
  - continuity surfaces record the validated backend/parity stop point and the output-path future TODO boundary

## Validation

- `ui_app\build_tests_vsdevcmd.cmd`
- `ui_app\build_vsdevcmd.cmd`
- `py -3.14 -m pytest tests/test_generic_probe_cli.py -q`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`