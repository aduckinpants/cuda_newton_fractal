# Counterfactual Pair Fractal - Explaino-Pattern Variant

## Current Phase

Closed - checkpoint commit, machine receipts, and the committed-head stale-plan gate are complete for the Explaino-pattern Counterfactual Pair slice.

## Phase Checklist

- [x] Phase 1 - open this checked-in plan and contract, replace the closed hardening lock, and prove the RED head still has no shipped Explaino-pattern Counterfactual Pair selector/runtime lane.
- [x] Phase 2 - land one bounded Explaino-pattern Counterfactual Pair runtime lane with explicit identity mapping, explicit shared-vs-Explaino control ownership, and preserved legacy sample surfaces.
- [x] Phase 3 - prove the lane through focused schema/unit/kernel/native rails plus focused runtime publish and published-runtime Explaino-variant proof.
- [x] Phase 4 - run hostile audit, checkpoint, write receipts, clear stale closeout text, and leave the repo clean on the committed head.

## Explicit User Asks

- [x] Open and execute only one bounded Explaino-pattern Counterfactual Pair variant slice.
- [x] Layer the steerable standalone Counterfactual Pair object into the Explaino pattern as a real runtime-visible lane, not just a renamed selector.
- [x] Keep color-pipeline work, ExplainO-BalanceVoid, meta-basin, Operator-Itinerary, DSL/program-space, and broad engine refactor work out of scope.
- [x] Preserve `FractalSampleResult` and `SampleFractalPoints(...)` as the shipped legacy projection surfaces.
- [x] Close only with focused native proof, runtime publish, published-runtime proof, hostile audit, checkpoint commit, machine receipts, and the committed-head stale-plan gate.

## Proof Ledger

- Bootstrap proved branch=`feature/fractal-sample-evidence-widening`, `HEAD=84bdd3225792b6e0b0272a5b0ffca1f31fe4d0cd`, clean tree, fresh session, and an active locked contract still pointing at the already closed Counterfactual Pair hardening contract `counterfactual_pair_fractal_semantics_control_surface_hardening`.
- Current single-orbit authority remains `FractalSampleResult` plus `SampleFractalPoints(...)` in `ui_app/src/fractal_sample_result.h` and `ui_app/src/fractal_sample_core.cu`; this slice preserved those shipped surfaces.
- RED proved there was no shipped Explaino-pattern Counterfactual Pair runtime lane on the bootstrap head: no `explaino_counterfactual_pair` enum id, no Explaino selector mapping, no shared-vs-Explaino control ownership in schema, and no published-runtime witness in `tests/test_fractal_runtime_explaino_escape_variants.py`.
- GREEN landed one bounded Explaino-pattern Counterfactual Pair runtime lane: `FractalType::explaino_counterfactual_pair` is explicit in `ui_app/src/fractal_types.h`, enum/schema binding round-trips through `ui_app/src/enum_id_utils.h` and `ui_app/src/schema_binding.cpp`, Explaino family identity is explicit in `ui_app/src/fractal_family_rules.h`, preset/default ownership is explicit in `ui_app/src/fractal_derived_fields.cpp`, runtime pair sampling lives in `ui_app/src/fractal_sample_device.inl`, and the four-class synthetic basin render path remains explicit in `ui_app/src/fractal_renderer.cu`.
- Shared controls stayed bounded and explicit: `counterfactual_pair_frame`, `counterfactual_pair_offset_x`, `counterfactual_pair_offset_y`, and `counterfactual_pair_reconvergence_ratio` are shared between standalone and Explaino pair lanes, while standalone `counterfactual_pair_root_family` remains the standalone-only owner seam.
- Explaino-specific identity is explicit and uses existing runtime seams rather than a new engine pass: the Explaino variant reads `explaino_phase`, `explaino_seed`, `explaino_warp_strength`, `explaino_damping`, and the existing custom polynomial/root-pack seam from `UpdateExplainoPolynomial(...)`; no extra sample-evidence widening or color-pipeline work was required.
- Focused schema/native/kernel/runtime proof on the current dirty head is green: `test_ui_schema`, `test_schema_binding`, `test_diagnostics_capture`, `test_diagnostics_state_io`, `test_fractal_sample_kernel`, `test_fractal_types`, `test_enum_id_utils`, runtime publish via `ui_app/build_vsdevcmd.cmd`, `tests/test_fractal_runtime_explaino_escape_variants.py -k explaino_counterfactual_pair`, and `tests/test_fractal_runtime_counterfactual_pair.py`.
- Hostile audit found one real defect in the first landing: the published-runtime comparison witness failed because the standalone `counterfactual_pair` diagnostics load path rejected a cloned Explaino-state bundle before it honored the explicit `counterfactual_pair_root_family` owner field.
- The repaired loader state is proved cleanly: `ui_app/src/diagnostics_state_io.cpp` now parses explicit `counterfactual_pair_root_family` authority before the standalone poly-kind fallback, `ui_app/tests/test_diagnostics_state_io.cpp` locks that case directly, and the published runtime now accepts the standalone comparison witness without reopening engine scope.

## Hostile Audit

- Status: done
- Required posture: assume the first implementation renamed the standalone mode, left the Explaino identity implicit, leaked standalone root-family ownership into the Explaino lane, or broke runtime state load/replay until focused proof disproved each failure mode.
- Hostile review questions:
  Did I actually implement an Explaino-pattern Counterfactual Pair lane, or only rename the standalone mode?
  Did I make the Explaino identity explicit, or leave it implicit and hand-wavy?
  Did I preserve `SampleFractalPoints(...)` as the shipped legacy projection path?
  Did I silently widen into color-pipeline work, ExplainO-BalanceVoid, meta-basin, or a generic engine rewrite?
  Did I stop with stale closeout text again?

## Audit Passes

- [x] Pass 1 - proved the RED head still had no shipped Explaino-pattern Counterfactual Pair selector/runtime lane or published-runtime witness.
- [x] Pass 2 - hostile reread of the landed identity/control mapping found one real defect: standalone `counterfactual_pair` runtime reload rejected a cloned Explaino-state bundle because diagnostics load consulted stale custom `poly_kind` before explicit `counterfactual_pair_root_family` authority.
- [x] Pass 3 - re-read the repaired loader branch, reran the native loader proof plus published-runtime Explaino and standalone Counterfactual Pair witnesses, and did not expose another slice-owned defect.

## Audit Findings

- [x] Real defect repaired - `LoadDiagnosticsStateFile(...)` rejected standalone `counterfactual_pair` bundles cloned from Explaino captures with `counterfactual_pair requires a supported root-family polynomial preset` because the loader read stale Explaino `poly_kind=custom` before the explicit `counterfactual_pair_root_family` owner field; the fix reordered explicit root-family parsing ahead of the standalone fallback and the published runtime now accepts the comparison witness cleanly.
- [x] Clean re-audit - reread the repaired loader path and reran `test_diagnostics_state_io`, `tests/test_fractal_runtime_explaino_escape_variants.py -k explaino_counterfactual_pair`, and `tests/test_fractal_runtime_counterfactual_pair.py`; no additional real defect found.

## Notes

- Public runtime authority for this slice is `ui_app/src/fractal_types.h`, `ui_app/src/fractal_family_rules.h`, `ui_app/src/fractal_derived_fields.cpp`, `ui_app/src/fractal_sample_device.inl`, `ui_app/src/fractal_renderer.cu`, `ui/fractal_binding_surface_v1.ui_schema.json`, `ui_app/src/schema_binding.cpp`, `ui_app/src/diagnostics_capture.cpp`, and `ui_app/src/diagnostics_state_io.cpp`.
- Shared control mapping stayed bounded: partner frame, partner offsets, and reconvergence ratio are shared with the standalone Counterfactual Pair lane; Explaino-owned controls remain the existing Explaino seed/warp/damping/custom-root seams; standalone root-family ownership did not widen into the Explaino lane.
- Exit criteria:
  - one bounded Explaino-pattern Counterfactual Pair runtime lane exists
  - its identity and control surface are explicit and proved
  - focused native rails plus runtime publish plus published-runtime proof are green
  - checkpoint commit, machine receipts, hostile-audit proof, and committed-head stale-plan gate are complete
