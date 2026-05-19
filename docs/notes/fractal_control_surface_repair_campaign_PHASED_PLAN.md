# Fractal Control Surface Repair Campaign

## Current Phase

Phase 1 is implemented and validated on `codex/fractal-control-surface-repair`. The next campaign slice is Phase 2, deterministic all-fractal control-surface descriptor/test harness work; it has not started in this checkpoint.

## Explicit User Asks

- [ ] Repair the control-surface issues documented in `docs/notes/fractal_control_surface_audit_inventory.md`.
- [ ] Repair the testing problems around missing no-mouse UI/UX proof for visible controls.
- [x] Clean up the animation system so it is driven by what is applicable to the currently loaded fractal instead of a static all-fractals mess for the existing Phase 1 animation target dropdown.
- [x] Keep the work as cleanup and polish; do not add unrelated new fractal features.

## Phase Checklist

- [x] Phase 0 - bootstrap, inspect current branch/head, and review the audit inventory plus animation seams.
- [x] Phase 1 - RED/GREEN existing animation target applicability and option filtering.
- [ ] Phase 2 - add deterministic all-fractal control-surface test descriptors for visible numeric controls and no-mouse proof planning.
- [ ] Phase 3 - repair high-confidence missing control surfaces: Julia constant controls and Nova `poly_c4` policy.
- [ ] Phase 4 - expand focused no-mouse UI/UX proof across existing visible standalone-family controls.
- [ ] Phase 5 - expand focused no-mouse UI/UX proof across existing visible Explaino common and owner-lane controls, preserving Explaino-all registry guards.
- [ ] Phase 6 - classify preset-only/fixed-formula lanes without silently turning policy decisions into feature creep.
- [x] Phase 7 - hostile audit, validation receipts, checkpoint, push, clean tree, and stale-plan proof for the Phase 1 bounded slice.

## Slice Plan

1. Animation applicability cleanup.
   - Add RED tests showing `param_anim_target` currently exposes targets such as Magnet and Explaino axes on unrelated fractal lanes.
   - Add option-level visibility support in the schema loader/binding layer.
   - Annotate the existing animation options so only currently applicable targets are visible.
   - Keep existing target ids stable; do not add new animation targets in this slice.
   - Add tests proving all visible animation options resolve and hidden options are not offered.
2. Control-surface descriptor/test harness.
   - Build a deterministic descriptor from the schema for every fractal and every visible numeric control.
   - Classify visible numeric controls as render-sensitive, non-rendering, combo-only, generated/internal, or fixed-formula lane.
   - Generate RED UI/UX harness cases from that descriptor instead of hand-picked Python subsets.
3. Missing-control repairs.
   - Julia: expose/runtime-bind real/imag constant controls if the RED confirms the fixed constant is the product gap.
   - Nova: resolve the `poly_c4` contradiction by either exposing it on Nova or narrowing the selectable polynomial policy, with tests first.
4. Existing-slider proof expansion.
   - Standalone families first: Phoenix, Newton/Halley, Projection-and-Flow, Counterfactual Pair, Multibrot, Multicorn, Lambda.
   - Explaino family lanes second: common Explaino controls, dual/mix, cluster-radius lanes, Phoenix-step/coupling lanes, and explicit owner lanes.
   - Keep Magnet and Explaino registry axes as existing green guards.

## Proof Ledger

- Starting branch for implementation campaign: `codex/fractal-control-surface-repair`.
- Starting head: `8c85393`.
- Prior documentation inventory: `docs/notes/fractal_control_surface_audit_inventory.md`.
- Bootstrap confirmed clean tree before this implementation campaign.
- RED: `ui_app/build_tests_vsdevcmd.cmd` failed at `test_param_anim_generic` before the fix because Mandelbrot, Magnet, and `explaino_ripple` all saw inapplicable animation options. Log: `artifacts/logs/fractal_control_surface_repair_phase1_red_build_tests.log`.
- GREEN focused animation rail: focused compile/run of `test_param_anim_generic` passed. Log: `artifacts/logs/fractal_control_surface_repair_param_anim_generic.log`.
- GREEN schema/binding and Color Pipeline guard rail: `ui_app/build_tests_vsdevcmd.cmd advanced_color_grading_red` passed, including `test_schema_binding`, `test_color_pipeline_core`, `test_color_pipeline_window`, `test_escape_time_coloring`, and `test_fractal_family_rules`. Log: `artifacts/logs/fractal_control_surface_repair_schema_binding_focus.log`.
- GREEN schema loader rail: focused compile/run of `test_ui_schema` passed. Log: `artifacts/logs/fractal_control_surface_repair_ui_schema_focus.log`.
- GREEN full native rail: `ui_app/build_tests_vsdevcmd.cmd` passed. Log: `artifacts/logs/fractal_control_surface_repair_build_tests_green.log`.
- GREEN runtime publish: `ui_app/build_vsdevcmd.cmd` passed and staged `D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe`. Log: `artifacts/logs/fractal_control_surface_repair_build_viewer.log`.
- GREEN no-mouse runtime harness rail: `py -3.14 -m pytest tests/test_fractal_runtime_runtime_walk_viewer.py -k "no_mouse_schema_int_set_value or unknown_set_value_fails_closed" -q --junitxml artifacts/pytest/fractal_control_surface_repair_runtime_ui_harness.junit.xml` passed. Log: `artifacts/logs/fractal_control_surface_repair_runtime_no_mouse.log`.
- GREEN no-mouse Explaino registry proof: `py -3.14 -m pytest tests/test_fractal_runtime_runtime_walk_viewer.py -k "explaino_registry_controls_no_mouse_set_value_change_live_viewport" -q --junitxml artifacts/pytest/fractal_control_surface_repair_runtime_explaino_registry_controls.junit.xml` passed. Log: `artifacts/logs/fractal_control_surface_repair_runtime_explaino_registry_controls.log`.
- Phase 1 implementation kept existing animation option ids stable and added no new user-facing animation target ids.
- `param_anim_target` now uses option-level visibility predicates; Explaino axis target options get `explaino_all` plus owner-lane visibility from `kExplainoAxisRegistry` during schema load.
- The generic animation test now reads the schema option list instead of a stale hard-coded target subset and covers the previously omitted Magnet, Vortex, and Tension targets.

## Hostile Audit

- Status: closed
- Posture used: assumed the implementation left stale targets visible, broke existing target ids, diverged schema option filtering from runtime applicability, or mutated unrelated Color Pipeline behavior until the proof ledger disproved those claims.

## Audit Passes

- [x] Pass 1 - checked all existing animation option ids remain stable and no new targets are introduced.
- [x] Pass 2 - checked option filtering against representative non-Explaino, standalone Magnet, canonical `explaino_all`, and owner-lane `explaino_ripple` cases.
- [x] Pass 3 - checked animation tests are schema-driven rather than another hard-coded subset.
- [x] Pass 4 - checked Color Pipeline focused rails still pass after shared schema/binding changes.
- [x] Pass 5 - checked no-mouse runtime harness proof still passes and the Explaino registry visible-control runtime proof remains green.
- [x] Pass 6 - clean re-read of the repaired state found no additional real defect in the animation option filtering or schema-driven test coverage.
- [x] Pass 7 - clean re-read of the repaired state found no additional workflow mistake in validation, plan sync, or runtime no-mouse proof.

## Audit Findings

- [x] Finding 1 - The original RED confirmed the static animation dropdown advertised inapplicable targets on unrelated fractals.
- [x] Finding 2 - `test_param_anim_generic.cpp` was not a trustworthy authority because it used a stale hard-coded subset and omitted shipped targets; it now loads the schema option list.
- [x] Finding 3 - The first implementation review caught an over-broad local replacement that removed `changed` from `RenderIntComboControl`; it was fixed before validation.
- [x] Finding 4 - The patch path stayed inside the locked Phase 1 contract scope; Color Pipeline code and renderer behavior were not changed.
- [x] Clean re-audit evidence - confirmed the repaired state after fixing `RenderIntComboControl`; no additional real issue found in the touched schema/binding/test seams.

## Boundaries

In scope for Phase 1:
- `param_anim_target` option applicability
- schema option visibility parsing/filtering
- focused native tests for the animation dropdown and target resolution
- existing no-mouse runtime harness proof for the touched schema/control path

Out of scope for Phase 1:
- adding new fractal types
- adding new user-facing animation targets
- repairing Julia/Nova controls before REDs are landed
- broad renderer or Color Pipeline changes
- physical mouse test paths
