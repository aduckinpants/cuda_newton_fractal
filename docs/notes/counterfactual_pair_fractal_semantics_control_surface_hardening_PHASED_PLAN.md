# Counterfactual Pair Fractal - Semantics And Control-Surface Hardening

## Current Phase

Complete - Counterfactual Pair now exposes an explicit bounded control surface, explicit perturbation-frame semantics, and explicit public class-model text on the current runtime, with focused native proof plus published-runtime proof recorded for checkpoint closure.

## Phase Checklist

- [x] Phase 1 - open this checked-in hardening plan and contract, replace the closed Counterfactual Pair runtime-lane lock, and prove the RED head still exposes no dedicated Counterfactual Pair controls beyond the selector plus shared generic controls.
- [x] Phase 2 - land one bounded Counterfactual Pair public control surface with an explicit perturbation-frame answer and an explicit public class-model answer while preserving `SampleFractalPoints(...)` and `FractalSampleResult`.
- [x] Phase 3 - prove the hardened state through focused schema/unit/kernel/native rails plus focused runtime publish and published-runtime Counterfactual Pair proof.
- [x] Phase 4 - run hostile audit, checkpoint, write receipts, clear stale closeout text, and leave the repo clean on the committed head.

## Explicit User Asks

- [x] Open and execute only one bounded Counterfactual Pair Fractal hardening slice.
- [x] Expose as many truthful Counterfactual Pair knobs as the current bounded runtime can support without widening into Explaino or color-pipeline work.
- [x] Settle the perturbation-frame semantics explicitly: world-absolute, view-relative, or a truthful selectable mode.
- [x] Make the pair-class semantics public, deliberate, and testable instead of leaving them mostly in hidden sampler constants.
- [x] Preserve `FractalSampleResult` and `SampleFractalPoints(...)` as the shipped legacy projection surfaces and close only with commit, machine receipts, clean tree, and the committed-head stale-plan gate.

## Proof Ledger

- Bootstrap proved branch=`feature/fractal-sample-evidence-widening`, `HEAD=17111961f5e9d9b2d1b036d9e2287f7306fd5a8f`, clean tree, fresh session, and an active locked contract still pointing at the already closed Counterfactual Pair runtime-lane contract `counterfactual_pair_fractal_bounded_runtime_lane`.
- Current single-orbit authority is still `FractalSampleResult` plus `SampleFractalPoints(...)` in `ui_app/src/fractal_sample_result.h` and `ui_app/src/fractal_sample_core.cu`; this hardening slice must preserve those shipped surfaces.
- Current public Counterfactual Pair surfacing is still minimal: `ui/fractal_binding_surface_v1.ui_schema.json`, `ui_app/src/enum_id_utils.h`, `ui_app/src/safe_mode_schema.cpp`, and the current runtime test only prove the fractal selector plus shared root-finding controls such as `epsilon`, not a dedicated pair-model control surface.
- Current perturbation-frame semantics are sampler-internal in `ui_app/src/fractal_sample_device.inl`: the partner offset still hardcodes `pairOffsetX = 0.08 * (2.0 / exp2(view.log2_zoom))` and `pairOffsetY = 0.04 * (2.0 / exp2(view.log2_zoom))`, which means the shipped head currently implements a view-relative perturbation frame without a public owner seam.
- Current class semantics are also sampler-internal in `ui_app/src/fractal_sample_device.inl`: the same-root reconvergence split still hardcodes `sameRootDriftThreshold = initialGap * 0.60`, the class ids still live as `0=reconverged`, `1=same-basin drift`, `2=basin swap`, `3=unstable`, and the renderer still receives those classes only through the synthetic root mapping `unit_root_k((pairClass + 2) % 4, 4)`.
- Current runtime proof remains existence-only: `tests/test_fractal_runtime_counterfactual_pair.py` proves that `counterfactual_pair` is a runtime-visible fractal mode with a stable frame hash and distinct output from `newton`, but it does not yet prove control-surface round-trip, frame semantics, or class-threshold ownership.
- Current test seams already prove there is no public hardening yet: `ui_app/tests/test_ui_schema.cpp`, `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_safe_mode_schema.cpp`, and `ui_app/tests/test_diagnostics_capture.cpp` only cover selector visibility, generic binding, safe-mode presence, and persisted fractal-type identity.
- Tiny adjacent seam expected before GREEN: if focused runtime proof needs round-tripping new Counterfactual Pair params through `--load-state-json`, then `ui_app/src/diagnostics_state_io.cpp` becomes a truthful hardening seam rather than scope drift, because the runtime harness already uses state-json load for published-runtime semantics proof.
- Out-of-scope proof remains explicit on the RED head: no current seam ties Counterfactual Pair to Explaino layering, color-pipeline integration, meta-basin, Operator-Itinerary, or DSL/program-space work, and this slice must keep those lanes closed.
- RED locked truthfully through `artifacts/cfpair_hardening_red_build.log`: `test_fractal_types.cpp` failed on missing `CounterfactualPairRootFamily`, missing `CounterfactualPairFrame`, and missing `KernelParams` Counterfactual Pair owner fields, which proved the RED head still lacked a public pair-model surface.
- GREEN landed in the bounded public owner seams only: `ui/fractal_binding_surface_v1.ui_schema.json`, `ui_app/src/safe_mode_schema.cpp`, `ui_app/src/schema_binding.cpp`, `ui_app/src/enum_id_utils.h`, `ui_app/src/fractal_types.h`, `ui_app/src/fractal_sample_device.inl`, `ui_app/src/diagnostics_capture.cpp`, and `ui_app/src/diagnostics_state_io.cpp` now expose/select/persist the root family, perturbation frame, partner offsets, and reconvergence ratio without touching `SampleFractalPoints(...)` or `FractalSampleResult`.
- Focused native proof is green on rebuilt binaries after the final post-audit refactor: `artifacts/cfpair_hardening_build5.log` again reaches only the unrelated `test_fractal_probe_coverage` tail, while `test_fractal_types.exe`, `test_enum_id_utils.exe`, `test_schema_binding.exe`, `test_diagnostics_capture.exe`, `test_safe_mode_schema.exe`, `test_ui_schema.exe`, `test_fractal_sample_kernel.exe`, and `test_fractal_sample_result.exe` all pass on the rebuilt `D:\salt-fractal\cuda_newton_fractal_clone\build_tests\` outputs.
- Focused published-runtime proof is green on the final runtime build: `artifacts/cfpair_hardening_runtime_build2.log` published `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`, `artifacts/cfpair_hardening_runtime_validate_ui2.log` passed `fractal_ui.exe --validate-ui`, and `artifacts/cfpair_hardening_runtime_pytest_direct2.log` passed `py -3.14 -m pytest tests/test_fractal_runtime_counterfactual_pair.py -q` with `3 passed`.
- `SampleFractalPoints(...)` and `FractalSampleResult` remain the shipped legacy projection surfaces, proven on the current build by `test_fractal_sample_result.exe` and by the fact that this slice only widened Counterfactual Pair’s public controls and bounded sampler semantics, not the legacy result contract.
- The repo’s generic `tools/code_quality_audit.py --check-baseline` rail is still branch-red on bootstrap-existing drift in untouched `ui_app/src/escape_time_coloring.h` and `ui_app/src/fractal_family_rules.h`; this hardening slice did not reopen those unrelated lanes, so closure truth stays on the contract’s focused native/runtime rails instead of falsely claiming a branch-wide audit green.

## Hostile Audit

- Status: complete
- Required posture: assume the first hardening attempt will only move constants around, leave the perturbation identity ambiguous, keep the class model hidden in sampler code, or widen into Explaino/color-pipeline work unless focused proof disproves each failure mode.
- Hostile review questions:
  Did I actually make Counterfactual Pair steerable, or only rename sampler constants?
  Did I settle the perturbation-frame semantics explicitly, or leave the key identity ambiguous?
  Did I make the class model public and testable, or keep it hidden behind synthetic-root plumbing?
  Did I preserve `SampleFractalPoints(...)` as the shipped legacy projection path?
  Did I silently widen into Explaino layering, color-pipeline work, or a generic engine rewrite?

## Audit Passes

- [x] Pass 1 - prove the RED head still has no dedicated Counterfactual Pair controls beyond the selector plus shared generic controls, and that the perturbation-frame and class semantics are still sampler-owned.
- [x] Pass 2 - hostile reread the landed public control surface for explicit frame semantics, explicit class semantics, legacy-surface preservation, and out-of-scope drift.
- [x] Pass 3 - rerun the repaired native and published-runtime proofs, then reread the repaired state and confirm a real hardening defect was found and fixed before closure or that no further slice-owned defect remains.

## Audit Findings

- [x] Workflow proof defect found and repaired: `py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_counterfactual_pair.py -q` produced an ambiguous `passed=0 failed=0` summary even though the focused pytest file executed, so it could not serve as truthful published-runtime proof for this slice. The repaired closure rail uses direct published-runtime pytest instead: `py -3.14 -m pytest tests/test_fractal_runtime_counterfactual_pair.py -q`, recorded green in `artifacts/cfpair_hardening_runtime_pytest_direct2.log`.
- [x] Control-surface audit debt found and repaired: the first implementation left an unused Counterfactual Pair derived-fields helper and grew `schema_binding.cpp`’s enum setter into a less-auditable monolith. The repaired state removes the dead helper and splits the Counterfactual Pair/color/fractal-type setter branches into dedicated local helpers before the final rebuilt native/runtime proof pass.
- [x] No additional real defect found in the repaired state after the focused re-audit of the touched seams (`schema_binding.cpp`, `fractal_sample_device.inl`, `safe_mode_schema.cpp`, `fractal_binding_surface_v1.ui_schema.json`, `diagnostics_capture.cpp`, `diagnostics_state_io.cpp`, and the focused tests) plus the final rebuilt native/runtime witness runs.

## Notes

- Expected public runtime authority after GREEN: `ui/fractal_binding_surface_v1.ui_schema.json` plus `ui_app/src/schema_binding.cpp` for the user-facing controls, `ui_app/src/fractal_types.h` / `ui_app/src/enum_id_utils.h` for any new user-facing enum ids, `ui_app/src/fractal_sample_device.inl` for the bounded Counterfactual Pair pair-model math, and `ui_app/src/diagnostics_capture.cpp` plus `ui_app/src/diagnostics_state_io.cpp` if runtime state round-trip is needed for the published-runtime proof.
- Expected REDs to lock before implementation:
  1. no dedicated Counterfactual Pair controls beyond the selector plus shared generic controls
  2. perturbation frame still hardcoded rather than publicly owned
  3. class semantics and threshold still mostly sampler-internal
  4. `SampleFractalPoints(...)` and legacy callers still bound to legacy semantics
  5. Explaino layering, color-pipeline integration, meta-basin, Operator-Itinerary, and DSL/program-space remain out of scope
- Exit criteria:
  - one explicit Counterfactual Pair public control surface exists
  - perturbation-frame semantics are explicit and proved
  - class semantics are explicit and proved
  - focused native rails plus runtime publish plus published-runtime proof are green
  - checkpoint commit, machine receipts, hostile-audit proof, and committed-head stale-plan gate are complete
