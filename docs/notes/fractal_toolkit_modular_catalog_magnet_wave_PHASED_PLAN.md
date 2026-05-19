# Fractal Toolkit Modular Catalog - Magnet Wave

## Current Phase

Phase 4 complete - Magnet Type I is implemented and validated as the first modular fractal-toolkit slice; later next-generation fractal families remain deferred to their own contracts.

## Explicit User Asks

- [x] Plan the long-term path to support the deferred next-generation fractal families, including perturbation-based deep zoom, without creating another monolith.
- [x] Start implementation with a new fractal type at each major unblocking point; first target is a bounded Magnet Type I 2D rational escape-time lane.
- [x] Keep FPS protected by choosing low-cost per-iteration math for the first wave, preserving smooth-escape fast paths, and adding focused performance/usability witnesses.
- [x] Provide meaningful sliders and examples for new fractal families rather than shipping hidden or no-op controls.
- [x] Integrate deterministic code scanning/tooling into the validation surface so architecture drift is caught before closeout.
- [x] Close only with focused native/runtime proof, hostile audit, checkpoint, receipts, push, clean tree, and stale-plan cleanup.

## Phase Checklist

- [x] Phase 0 - bridge from the closed top-down contract only far enough to create this plan/contract, then replace the active lock before product edits.
- [x] Phase 1 - add deterministic REDs and scan-tool expectations for a modular Magnet Type I fractal lane, including enum/schema/runtime/control visibility and no monolith growth.
- [x] Phase 2 - implement Magnet Type I through direct escape-time formula seams, bounded runtime validation, schema bindings, safe-mode/CLI/catalog visibility, presets, and meaningful controls.
- [x] Phase 3 - add focused native and runtime proofs that Magnet sliders affect rendered output while FPS-sensitive smooth-escape routing remains bounded.
- [x] Phase 4 - run code-quality and architecture/fractal-toolkit scans, focused native rails, runtime publish, published-runtime proof, hostile audit, checkpoint, receipts, push, and stale-plan gate.
- [deferred] Phase 5 - Lyapunov sequence-contract slice: implement a small AB/ABC logistic-sequence family after a sequence descriptor seam exists.
- [deferred] Phase 6 - perturbation deep-zoom expansion slice: extend perturbation/reference-orbit support beyond Mandelbrot/Julia only where mathematical recurrence and precision proof are explicit.
- [deferred] Phase 7 - IFS/gasket slice: add a separate IFS/subdivision or chaos-game contract, then implement Sierpinski gasket/carpet examples.
- [deferred] Phase 8 - attractor-density slice: add accumulation/histogram render infrastructure, then implement a small Clifford/De Jong style density field.
- [deferred] Phase 9 - 3D DE/raymarch slice: add camera, raymarch, normal, lighting, and perf gates, then implement Mandelbulb.
- [deferred] Phase 10 - explanation-state research slices: prototype Root-Adjacency, Reversible Observer, Operator-Itinerary, Program-Space/DSL, and Meta-Basin only after their state/output contracts exist.

## Long-Term Prioritization

1. **Magnet Type I now** - safe 2D rational escape-time family. It exercises the modular catalog path without needing new renderer infrastructure. Expected controls: initial seed real/imag, relaxation, bailout/attractor tolerance if validation proves they are meaningful and cheap.
2. **Lyapunov next** - still 2D and viewer-friendly, but requires a parameter-sequence contract. This is the next smallest new substrate after direct orbit maps.
3. **Perturbation deep zoom** - expand precision for existing escape-time families where recurrence-specific perturbation math can be proven. This protects user experience and FPS before heavier families.
4. **IFS/gasket** - not a switch-case fractal. Needs its own map-set/subdivision or chaos-game sample contract.
5. **Attractor density fields** - needs accumulation buffers and histogram/normalization rules. Do not fake this through single-pixel escape output.
6. **Mandelbulb / 3D DE** - needs a 3D raymarch contract, camera controls, normals, lighting, and strict perf gates.
7. **Explanation-state families** - Root-Adjacency may come before full Meta-Basin, but all of these need state/classification contracts. Do not relabel existing sidecar or projection lanes.

## Architecture Rules For Every Fractal Slice

- New math lives in a family-specific formula/module seam when possible, not directly as a sprawling branch in `fractal_sample_device.inl`.
- `fractal_sample_device.inl` may dispatch, but must not become the design document or own every helper.
- Schema JSON remains the UI layout authority; C++ structs remain runtime authority; binding tests must prove both.
- Every public slider must be one of: authoritative now, intentionally hidden now, or explicitly blocked with proof.
- New fractal examples require presets/defaults and at least one runtime output-hash or sample-grid proof that the controls matter.
- FPS-sensitive paths must keep per-iteration math bounded and must not promote smooth-escape auto-tier into slow standard mode without an explicit reason and performance witness.
- Physical mouse tests are forbidden; viewer proof must use in-process automation and rendered-frame hashes.
- Deterministic scan tooling must run before closeout: code quality, architecture risk surfaces, and the new fractal-toolkit scan.

## Proof Ledger

- Bootstrap on 2026-05-19 reported branch `master`, `HEAD=f2074dc`, clean tree, and the closed `top_down_architecture_repair` contract.
- Current engine supports 2D direct/root/escape-style per-pixel sampling through `fractal_sample_device`, `SampleFractalEvidencePoints`, and legacy-projected `SampleFractalPoints`.
- Current widened evidence is still only `sample_coord + legacy_result`, so it is enough for bounded 2D direct families but not enough for full explanation-state, program-space, IFS accumulation, or 3D raymarch families.
- Current architecture plan closed only Phase 1; Color Pipeline extraction and descriptor-driven fractal family ownership remain deferred and must not be pretended complete.
- First implementation choice: Magnet Type I because it is the least risky unimplemented family that still proves the modular catalog path and gives meaningful controls without a renderer replacement.
- Bridge proof: the closed top-down contract was widened only to add this new plan/contract pair; runtime/schema/test mutation was blocked until this contract became active.
- Implemented Magnet Type I as `FractalType::magnet` with direct escape-time formula helpers, explicit enum/schema/safe-mode/binding/catalog/diagnostics/probe/runtime support, and four numeric controls: `magnet_seed_real`, `magnet_seed_imag`, `magnet_relaxation`, `magnet_bailout`.
- Meaningful-control proof: `test_fractal_renderer` verifies Magnet renders a non-flat smooth-escape field and that `magnet_relaxation` changes rendered pixels on the same lane; `tests/test_fractal_runtime_magnet.py` verifies no-mouse set-value automation consumes `fractal_control.magnet_relaxation.primary` and changes the published frame hash.
- FPS/perf guard proof: Magnet stays on the direct escape-time seam, keeps default auto-max-iter off, uses the shared escape-time auto curve only when explicitly enabled, and renderer stats stay within `max_iter` in the native Magnet sensitivity witness.
- Color Pipeline guard proof: `test_color_pipeline_core`, `test_color_pipeline_window`, and `test_escape_time_coloring` stayed green after the Magnet/no-mouse automation changes.
- Legacy projection proof: `test_fractal_sample_result` stayed green and continues to prove `SampleFractalPoints(...)` is the shipped legacy projection surface.
- Deterministic scan proof: `tools/viewer_host_validate_fractal_toolkit_scan.py` passes and checks Magnet seams plus the no-physical-mouse runtime-test ban.
- Validation green before checkpoint: `ui_app\build_tests_vsdevcmd.cmd`, focused native executable list, `ui_app\build_vsdevcmd.cmd`, and `py -3.14 -m pytest tests/test_fractal_runtime_magnet.py tests/test_function_descriptor_cli.py tests/test_callable_engine_adversarial_cli.py tests/test_fractal_runtime_explaino_escape_variants.py -q` all passed on the repaired Magnet slice.

## Hostile Audit

- Status: complete
- Required posture: assume the first implementation will become another switch-case monolith, expose sliders that do nothing, harm FPS, skip runtime proof, or accidentally reopen Color Pipeline/Explaino/Meta-Basin/DSL work unless focused proof disproves each failure mode.
- Result: real defects were found and repaired before closure; repaired state was re-read and validated cleanly.

## Audit Passes

- [x] Pass 1 - reviewed the initial REDs and implementation plan for monolith growth, fake controls, missing scan gates, and overbroad next-gen scope; found contract/test-scope risk and expanded the active contract before product mutation.
- [x] Pass 2 - reviewed the landed Magnet diff against schema/runtime/binding/renderer/test seams; found missing explicit native guard coverage and added enum/type/family/validation/preset/binding/direct-formula/probe/device/renderer/diagnostics assertions.
- [x] Pass 3 - re-read the repaired state after validation; code-quality initially caught `ResolveFractalViewPresetDefaults()` growing past baseline, then the escape-time preset helper split repaired the monolith-growth regression and no additional real issue was found.
- [x] Pass 4 - confirmed the repaired state with fresh native build, focused native executions, runtime publish, published no-mouse runtime pytest, deterministic scan, code-quality baseline, contract validation, and plan-sync validation.

## Audit Findings

- [x] Real finding: the first scanner implementation had a broken JSON newline literal; repaired by fixing `tools/viewer_host_validate_fractal_toolkit_scan.py` and rerunning `tests/test_viewer_host_fractal_toolkit_scan.py`.
- [x] Real finding: the initial active contract omitted `ui_app/tests/test_fractal_sample_device.cu`, which would have let Magnet skip the device all-fractal smoke list; repaired by contract-scope update and relock before applying the guard patch.
- [x] Real finding: Magnet initially raised `fractal_derived_fields.cpp` max-function size from 87 to 91 lines; repaired by splitting escape-time view preset defaults into `TryResolveEscapeTimeViewPresetDefaults()` and rerunning code-quality baseline successfully.
- [x] Real finding: `test_diagnostics_capture.cpp` added a Magnet persistence check without including the family-rule helper that supplies `ColorPipelineForLegacyMode`; repaired with a focused include patch and full native rebuild.
- [x] Clean re-read: after the repairs, focused native rails, Color Pipeline rails, legacy `SampleFractalPoints(...)` proof, runtime publish, no-mouse runtime pytest, scan, code quality, contract validation, and plan sync all proved cleanly.
- [x] Clean re-audit: no additional real defect found in the repaired Magnet surface after reviewing selector/catalog/probe/schema/runtime/FPS boundaries and the deferred next-gen scope.

## Action Hostile Review

- Action ID: magnet-red-scan-implementation-1
- Suspected Failure Mode: adding Magnet as another hardcoded branch in the existing hot files while leaving schema controls, scan gates, presets, runtime proof, or FPS behavior unproven.
- Correct Owner/Action: first add REDs and deterministic scan expectations around enum/schema/runtime/control visibility, then implement Magnet through the narrow direct escape-time formula seams and only widen files named by this contract.
- Proof Surface: `test_enum_id_utils`, `test_ui_schema`, `test_schema_binding`, `test_fractal_runtime_validation`, `test_fractal_sample_kernel`, `test_fractal_renderer`, the new fractal-toolkit scan, runtime publish, and a no-mouse published-runtime Magnet proof.
- Blocked Action: 3D raymarching, IFS/attractor accumulation, DSL/program-space, Meta-Basin, Operator-Itinerary, broad renderer replacement, broad Color Pipeline changes, physical mouse tests, or performance-regressing smooth-escape promotion.

- Action ID: magnet-implementation-expanded-scope-1
- Suspected Failure Mode: Magnet lands only in the renderer/sample path while diagnostics save/load and probe tooling silently drop or mis-run its slider parameters.
- Correct Owner/Action: expand the active contract to include diagnostics capture/load and probe-runner truth before the product mutation, then keep all edits bounded to the Magnet lane.
- Proof Surface: diagnostics capture/state IO tests, probe-runner tests, the deterministic fractal-toolkit scan, native schema/binding rails, and the no-mouse published-runtime Magnet proof.
- Blocked Action: shipping renderer-only Magnet controls, unsaved slider state, probe-only fake formulas, or widening into unrelated deferred fractal families.

- Action ID: magnet-catalog-probe-scope-1
- Suspected Failure Mode: Magnet is implemented in code but omitted from `fractal.sample`, `--describe-functions`, probe coverage, or runtime catalog tests.
- Correct Owner/Action: include function descriptor and probe coverage seams in the contract before implementation so public callable surfaces remain truthful.
- Proof Surface: `test_function_descriptor`, `test_fractal_probe_coverage`, `test_function_descriptor_cli.py`, and the deterministic fractal-toolkit scan.
- Blocked Action: hidden Magnet ids, callable/catalog drift, or adding catalog-only entries that do not execute.

## Notes

- This is both the long-term roadmap anchor and the first implementation slice. Later phases should split into their own plan/contract before mutation.
- The first slice must stay small enough to validate fully. If Magnet Type I needs unexpected broad renderer or sample-result widening, stop and record the exact blocker instead of forcing it through.
