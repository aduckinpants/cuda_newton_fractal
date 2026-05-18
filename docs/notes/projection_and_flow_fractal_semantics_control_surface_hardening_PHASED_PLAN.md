# Projection-and-Flow Fractal - Semantics And Control-Surface Hardening

## Current Phase

Phase 4 complete - the bounded Projection-and-Flow public control surface is landed, the targeted native seam pack plus focused runtime publish/pytest are green, hostile audit found and repaired real defects, and the tracked closure surfaces are synchronized for checkpoint/receipt closeout on this committed head.

## Phase Checklist

- [x] Phase 1 - open this checked-in hardening plan and contract, replace the closed Projection-and-Flow runtime-lane lock, and prove the RED head still exposes no dedicated Projection-and-Flow controls beyond the selector plus shared generic controls.
- [x] Phase 2 - land one bounded Projection-and-Flow public control surface with an explicit projection-target answer and an explicit public class-model answer while preserving `SampleFractalPoints(...)` and `FractalSampleResult`.
- [x] Phase 3 - prove the hardened state through focused schema/unit/kernel/native rails plus focused runtime publish and published-runtime Projection-and-Flow proof.
- [x] Phase 4 - run hostile audit, checkpoint, write receipts, clear stale closeout text, and leave the repo clean on the committed head.

## Explicit User Asks

- [done] Open and execute only one bounded Projection-and-Flow Fractal hardening slice.
- [done] Expose as many truthful Projection-and-Flow knobs as the current bounded runtime can support without widening into Explaino or color-pipeline work.
- [done] Settle the projection target semantics explicitly and make the class model public, deliberate, and testable.
- [done] Preserve `FractalSampleResult` and `SampleFractalPoints(...)` as the shipped legacy projection surfaces.
- [done] Close only with focused native proof, runtime publish, published-runtime proof, hostile audit, checkpoint commit, machine receipts, clean tree, and the committed-head stale-plan gate.

## Proof Ledger

- Bootstrap proved branch=`feature/fractal-sample-evidence-widening`, `HEAD=2f10f3e`, clean tree, fresh session, and an active locked contract still pointing at the already closed Projection-and-Flow runtime-lane contract `projection_and_flow_fractal_bounded_runtime_lane`.
- Current single-orbit authority is still `FractalSampleResult` plus `SampleFractalPoints(...)` in `ui_app/src/fractal_sample_result.h` and `ui_app/src/fractal_sample_core.cu`; this hardening slice must preserve those shipped surfaces.
- Current public Projection-and-Flow surfacing is still minimal: `ui/fractal_binding_surface_v1.ui_schema.json`, `ui_app/src/enum_id_utils.h`, `ui_app/src/safe_mode_schema.cpp`, and the current runtime test only prove the fractal selector plus shared generic controls such as `epsilon` and `max_iter`, not a dedicated projection-flow owner seam.
- Current projection semantics remain hardcoded in `ui_app/src/fractal_sample_device.inl`: the runtime always projects the free Newton step back onto the unit circle, always uses full-strength radial projection, and always classifies pressure against the hidden threshold `1.0`.
- Current class semantics also remain mostly sampler-internal in `ui_app/src/fractal_sample_device.inl` plus `ui_app/src/fractal_renderer.cu`: the shipped lane fixes `3` root sectors, `7` synthetic render classes, and the root-sector x pressure-bucket plus unstable model without a public owner seam.
- The current runtime can support a bounded public hardening without replacing `SampleFractalPoints(...)`: the public owner seams are `ui/fractal_binding_surface_v1.ui_schema.json`, `ui_app/src/schema_binding.cpp`, `ui_app/src/diagnostics_capture.cpp`, `ui_app/src/diagnostics_state_io.cpp`, `ui_app/src/fractal_types.h`, `ui_app/src/enum_id_utils.h`, `ui_app/src/fractal_derived_fields.cpp`, `ui_app/src/fractal_sample_device.inl`, and `ui_app/src/fractal_renderer.cu`.
- The bounded hardening target is to expose Projection-and-Flow-owned controls for root family, projection target radius, and the public pressure threshold while making the projection semantics explicit as full-strength radial projection onto the selected circle; Explaino layering, color-pipeline work, meta-basin, Operator-Itinerary, DSL/program-space, renderer replacement, and generic engine refactor work remain out of scope.
- Minimal blocker-clearing preflight was required before RED work because the closed runtime-lane contract had to be replaced, and the wrapper flow validates future scope paths before it can revise the old lock.
- RED locked truthfully through `artifacts/projection_flow_hardening_red_build.log`: the focused build now fails on missing `ProjectionAndFlowRootFamily` plus missing `KernelParams::projection_and_flow_root_family`, `projection_and_flow_target_radius`, and `projection_and_flow_pressure_threshold`, which proves the current repo still lacks a Projection-and-Flow-owned public semantics/control surface.
- GREEN owner seams now exist in `ui/fractal_binding_surface_v1.ui_schema.json`, `ui_app/src/schema_binding.cpp`, `ui_app/src/safe_mode_schema.cpp`, `ui_app/src/diagnostics_capture.cpp`, `ui_app/src/diagnostics_state_io.cpp`, `ui_app/src/fractal_runtime_validation.h`, `ui_app/src/fractal_sample_device.inl`, and `ui_app/src/fractal_renderer.cu`: Projection-and-Flow publicly owns root family, projection radius, and pressure threshold while `SampleFractalPoints(...)` and `FractalSampleResult` remain unchanged.
- The explicit runtime semantics answer is now: free Newton evolution for the selected cubic/quartic unit-root family, followed by full radial projection onto the public circle `|z| = radius`; the public class model is `2N + 1` classes where `0..2N-1` encode root sector x low/high pressure bucket and class `2N` is unstable.
- Focused native proof is green through the rebuilt seam pack (`test_enum_id_utils`, `test_fractal_types`, `test_fractal_derived_fields`, `test_fractal_runtime_validation`, `test_schema_binding`, `test_ui_schema`, `test_safe_mode_schema`, `test_diagnostics_capture`, `test_diagnostics_state_io`, `test_fractal_sample_core`, `test_fractal_sample_kernel`, `test_fractal_renderer`).
- Focused runtime-visible proof is green through `artifacts/projection_flow_hardening_runtime_publish_2.log` and `artifacts/projection_flow_hardening_runtime_pytest_2.log`.
- The full `ui_app/build_tests_vsdevcmd.cmd` helper still finishes red outside this slice on `test_fractal_probe_coverage` runtime fractal-type mismatches for `explaino_ripple`, `explaino_splice`, `explaino_vortex`, `explaino_tension`, and `explaino_balance_void`; the Projection-and-Flow seam pack above remains green.
- Contract, phased-plan-sync, and hostile-audit validators are green through `artifacts/validation/projection_and_flow_fractal_semantics_control_surface_hardening_contract.json`, `artifacts/validation/viewer_host_assert_phased_plan_sync.json`, and `artifacts/validation/projection_and_flow_fractal_semantics_control_surface_hardening_hostile_audit.json`.

## Hostile Audit

- Status: complete
- Required posture: assume the hardening attempt will only move constants around, leave the projection identity ambiguous, keep the class model hidden in sampler code, or widen into Explaino/color-pipeline work unless focused proof disproves each failure mode.
- Hostile review questions:
  Did I actually make Projection-and-Flow steerable, or only move constants around?
  Did I settle the projection semantics explicitly, or leave the key identity ambiguous?
  Did I make the class model public and testable, or keep it hidden in sampler code?
  Did I preserve `SampleFractalPoints(...)` as the shipped legacy projection path?
  Did I silently widen into Explaino layering, color-pipeline work, or a generic engine rewrite?
  Did I stop with stale closeout text again?

## Audit Passes

- [x] Pass 1 - prove the RED head still has no dedicated Projection-and-Flow controls beyond the selector plus shared generic controls, and that the projection/class semantics are still sampler-owned.
- [x] Pass 2 - hostile reread the landed public control surface for explicit projection semantics, explicit class semantics, legacy-surface preservation, and out-of-scope drift.
- [x] Pass 3 - rerun the repaired native and published-runtime proofs, then reread the repaired state and confirm a real hardening defect was found and fixed before closure or that no further slice-owned defect remains.

## Audit Findings

- [x] Real defect found and repaired: `ApplyProjectionAndFlowPresetDefaults(...)` did not restore the shipped cubic/radius/pressure baseline, so selector changes could leak stale quartic or threshold state back into Projection-and-Flow; `test_fractal_derived_fields.cpp` now regresses that leak and the preset seam now reasserts the owned defaults.
- [x] Real defect found and repaired: Projection-and-Flow synthetic class roots did not round-trip through the repo's canonical `NearestRootIndexUnitRoots(...)` decoder, so the sampler now writes the inverse-shifted class root and the CUDA kernel witness checks the decode-stable synthetic root explicitly.
- [x] Clean re-read found no additional real defect after the repaired native seam pack and the republished runtime pytest lane both went green; Explaino/color-pipeline/generic-engine scope drift was not reintroduced.

## Action Hostile Review

- Action ID: projection-flow-hardening-green-1
- Suspected Failure Mode: I could wire new Projection-and-Flow controls without syncing polynomial ownership, keep the class model hidden behind cubic-only sampler constants, or silently widen into Explaino/color/engine work while claiming hardening.
- Correct Owner/Action: Add only the bounded Projection-and-Flow schema, safe-mode, enum-id, binding, persistence, runtime-validation, sampler, renderer, and focused test seams needed for root family, target radius, and public pressure-threshold ownership while keeping `SampleFractalPoints(...)` untouched.
- Proof Surface: GREEN evidence must come from `test_fractal_types`, `test_enum_id_utils`, `test_fractal_derived_fields`, `test_schema_binding`, `test_ui_schema`, `test_safe_mode_schema`, `test_diagnostics_capture`, `test_diagnostics_state_io`, `test_fractal_runtime_validation`, `test_fractal_sample_core`, `test_fractal_sample_kernel`, `test_fractal_renderer`, and `tests/test_fractal_runtime_projection_and_flow.py`, plus the final focused runtime publish and published-runtime witness.
- Blocked Action: Mutate the Projection-and-Flow public-control, persistence, and runtime owner seams in `ui_app/src/`, `ui_app/tests/`, and `tests/test_fractal_runtime_projection_and_flow.py`.

## Notes

- Expected REDs to lock before implementation:
  1. no dedicated Projection-and-Flow controls beyond the selector plus shared generic controls
  2. projection-target and pressure-threshold semantics are still hardcoded rather than publicly owned
  3. class semantics are still mostly sampler-internal
  4. `SampleFractalPoints(...)` and legacy callers still stay on legacy semantics
  5. Explaino layering, color-pipeline integration, meta-basin, Operator-Itinerary, and DSL/program-space remain out of scope
- Landed bounded public semantics after GREEN:
  - root family: cubic or quartic unit-root Newton family
  - projection target: full radial projection onto a centered circle with public radius ownership
  - pressure split: public cumulative projection-pressure threshold
  - class model: root sector x low/high pressure bucket plus one unstable class, with decode-stable synthetic class roots for the shipped basin decoder
