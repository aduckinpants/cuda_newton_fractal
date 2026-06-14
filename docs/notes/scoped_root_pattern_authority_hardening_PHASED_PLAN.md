# Scoped Root-Pattern Authority Hardening Phased Plan

## Explicit User Asks

- Prove the scoped root-pattern authority model beyond smoke-level reporting.
- Demonstrate distinct `dynamics_root_field` and `color_root_field` descriptors with distinct hashes.
- Prove scoped controls/actions mutate only their owner scope.
- Prove root-field consumers and root-aware Color Pipeline rows consume their declared scoped refs.
- Prove invalid or missing refs fail closed with reportable diagnostics instead of silently falling back.
- Prove save/load/capture/replay preserves scoped descriptors and consumer refs.
- Build at least one divergent capture/runtime matrix before moving on to new fractal families or SDF work.

## Current Phase

Phase 6 - Implementation and focused proof are green; hostile audit, plan sync, receipts, rearward review, push, and clean-tree closeout remain.

## Phase Checklist

- [x] Phase 0: Branch `codex/scoped-root-pattern-authority-hardening` from clean `d17c212` and create this plan/contract.
- [x] Phase 1: Add RED tests proving current smoke is insufficient: dynamics/color roots share backing or invalid refs silently normalize.
- [x] Phase 2: Add independent Color Root Field descriptor storage/binding/action authority.
- [x] Phase 3: Route root-field consumers and root-aware Color Pipeline rows through explicit scoped refs with fail-closed invalid-ref behavior.
- [x] Phase 4: Add divergent no-mouse runtime matrix and capture/replay proof.
- [x] Phase 5: Preserve legacy single-pattern behavior and compatibility alias loading.
- [ ] Phase 6: Hostile audit, validation, checkpoint, receipts, rearward review, push, and stop for replan.

## Scope Lock

This hardening pass may change scoped root-pattern descriptors, schema/binding/report surfaces, root-aware Color Pipeline row routing, tests, and capture/replay sidecars required to prove divergent scoped authority. It must not add new fractal families, return to SDF product work, add graph-editor UI, add arbitrary pattern banks, add custom N-root coordinate editing, or use physical mouse automation.

## Concrete Problem

The shipped scoped-authority slice reports `active_root_field`, `root_patterns[]`, and `root_pattern_consumers[]`, but the manual smoke capture used identical generated legacy quartic settings for both `dynamics_root_field` and `color_root_field`. That proves the report shape exists. It does not prove that the two scopes are independently controllable, independently consumable, or safe when a row asks for a bad ref.

## Required Divergent Demonstration

Use `explaino_magnet_root_well` as the first proof target:

- Dynamics Root Field: `regular_ngon_v1`, count `11`.
- Color Root Field: `legacy_quartic_v1`, count `4`.
- Root Well consumer: `dynamics_root_field`.
- Root-aware Color Pipeline row: `root_proximity` or `root_phase` with `color_root_field`.

Required invariant:

- Dynamics root hash and Color root hash must differ.
- Root Well consumer report must name `dynamics_root_field`.
- Color source row consumer report must name `color_root_field`.
- Frame hash must differ from the same state with the color row consuming `dynamics_root_field`.

## Authority Requirements

### Descriptor Independence

`dynamics_root_field` and `color_root_field` must be able to differ by:

- layout kind
- generated root count
- seed
- phase
- spread
- base/effective root hash

Changing one scope must not mutate the other scope.

### Scoped Actions

Actions must be scoped:

- `fractal.actions.root_pattern.dynamics.prev_seed`
- `fractal.actions.root_pattern.dynamics.next_seed`
- `fractal.actions.root_pattern.color.prev_seed`
- `fractal.actions.root_pattern.color.next_seed`

Legacy global actions may remain only for single-root ExplainO lanes where no ambiguity exists.

### Consumer References

Root-aware consumers must use exact refs:

- `explaino_magnet_root_well` may consume `dynamics_root_field` or `color_root_field` as configured.
- `root_proximity` and `root_phase` source rows may consume either scoped field.
- Invalid refs must fail closed instead of silently using dynamics roots.

### Persistence And Replay

`state.json`, runtime reports, and Capture Finding `fractal-state.json` must preserve:

- root-pattern descriptors
- scoped refs
- layout kind
- root count
- seed/phase/spread where present
- consumer refs

Reload must reproduce the same root hashes, consumer report, and deterministic frame for fixed settings.

## RED Targets

Expected first failures should show one or more of:

- Color Root Field lacks independent seed/phase/spread storage or binding.
- Root-aware row invalid refs silently fall back to `dynamics_root_field`.
- Runtime report cannot show divergent dynamics/color hashes.
- No-mouse proof can pass while both root patterns share identical hashes.
- State/capture sidecars preserve old fields but do not prove scoped divergence.

## Proof Ledger

| Item | Status | Evidence |
| --- | --- | --- |
| Bootstrap | done | `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` on clean `codex/scoped-explaino-root-pattern-authority` head `d17c212`. |
| Repo status | done | `py -3.14 tools/viewer_host_repo_status.py` reported clean. |
| Rearward review | done | `py -3.14 tools/viewer_host_rearward_review.py` returned `ok` for `d17c212`. |
| Plan/contract | done | This plan and `docs/contracts/scoped_root_pattern_authority_hardening.contract.json`, locked through `viewer_host_begin_work_slice.py` with checkpoint token `ck:df802f9f`. |
| RED tests | done | Added native/runtime assertions for divergent scoped roots, owner-only Color Root actions, invalid root-pattern refs, and state reload of scoped Color Root scalar fields. |
| Native validation | done | `cmd /c "ui_app\build_tests_vsdevcmd.cmd test_schema_binding test_color_pipeline_core test_diagnostics_state_io test_diagnostics_capture test_viewer_ui_automation_report > artifacts\scoped_root_pattern_authority_hardening\native_scoped_root_pattern_hardening.log 2>&1"` passed after hardening assertions. |
| Refactor validation | done | Code-quality found `escape_time_coloring.h` function-size regression; refactored generated-root list building, then reran native rail to `artifacts\scoped_root_pattern_authority_hardening\native_scoped_root_pattern_hardening_after_refactor.log` and code-quality JSON `artifacts\validation\scoped_root_pattern_authority_hardening_code_quality_after_refactor.json`, both green. |
| Runtime publish | done | `cmd /c "ui_app\build_vsdevcmd.cmd > artifacts\scoped_root_pattern_authority_hardening\runtime_publish_after_refactor.log 2>&1"` passed after the refactor. |
| Published runtime proof | done | `py -3.14 -m pytest tests/test_fractal_runtime_root_field_consumers.py -q --junitxml artifacts/pytest/scoped_root_pattern_authority_hardening_runtime.junit.xml` passed after repairing the stale secondary-root expectation. |
| Hostile audit | done | Hostile audit complete; final clean re-read found no additional blocking defect after repaired include, behavior-proof, invalid-ref, scoped-control, and code-quality issues. |
| Receipts/rearward/push | pending | Required before final closeout. |

## Implementation Slices

### Phase 1 - RED Divergence Tests

Add focused tests for descriptor independence, scoped action ownership, invalid-ref fail-closed behavior, and divergent runtime matrix expectations.

Candidate rails:

- `test_schema_binding`
- `test_color_pipeline_core`
- `test_diagnostics_state_io`
- `test_diagnostics_capture`
- `test_viewer_ui_automation_report`
- `tests/test_fractal_runtime_root_field_consumers.py`

### Phase 2 - Independent Color Root Field Authority

Add the minimum persistent fields and binding aliases needed for Color Root Field movement controls to differ from Dynamics Root Field. Existing `explaino_secondary_root_pattern_layout/count` can remain compatibility storage only if new scoped authority fields make ownership explicit.

### Phase 3 - Scoped Routing And Fail-Closed Refs

Centralize ref resolution so valid scoped refs resolve to descriptors and invalid refs return a structured failure. Legacy aliases `primary` and `secondary` still load, but arbitrary invalid refs must not silently normalize.

### Phase 4 - Runtime Matrix And Capture Replay

Publish runtime and prove:

- Dynamics regular-N 11 plus Color legacy quartic 4 produces distinct root hashes.
- Color row ref switch changes frame without changing dynamics root hash.
- Dynamics consumer ref switch changes root-field consumer hash/report.
- Capture Finding/replay preserves scoped descriptors and frame hashes.

### Phase 5 - Legacy Compatibility

Prove old single-pattern states and Root SDF generated legacy mode still load deterministically and do not persist generated roots as custom authority.

### Phase 6 - Hostile Audit And Closure

Audit for shared storage, invalid fallback, missing report refs, stale A/B vocabulary, and runtime tests that prove only reporting rather than behavior.

## Test Targets

- Native descriptor/binding tests prove distinct dynamics/color descriptors and owner-only mutation.
- Native Color Pipeline tests prove root-aware rows can target each scoped ref and invalid refs fail closed.
- State/capture tests prove scoped refs and hashes round-trip.
- Runtime no-mouse proof selects `explaino_magnet_root_well`, builds divergent Dynamics/Color root fields, switches the root-aware row ref, proves frame/report changes, and captures/replays deterministically.

## Hostile Audit

- Status: complete

Required questions:

- Did the proof actually create different dynamics and color root hashes?
- Did scoped controls/actions mutate only their owner scope?
- Did Color Pipeline rows consume their declared scoped pattern ref?
- Did invalid refs fail closed instead of falling back?
- Did save/load/capture/replay preserve the same refs and hashes?
- Did legacy single-root behavior remain deterministic?
- Did the slice avoid new fractal families, SDF work, graph UI, and custom N-root coordinate banks?

## Audit Passes

- [x] Pass 1: Found missing native build include for `ExplainoSeedCombined` after adding generated root descriptor reuse; fixed by including `explaino_seed.h`.
- [x] Pass 2: Found runtime proof initially checked a nonexistent click-consumed field instead of behavior; fixed by asserting scoped Color Root seed/hash/frame changes after `color_root_field_next_seed`.
- [x] Pass 3: Clean re-read checked UI labels, scoped control/action bindings, runtime behavior assertions, state reload seams, and compatibility storage; no additional blocking defect found.
- [x] Pass 4: Code-quality closure gate found a real function-size regression in `escape_time_coloring.h`; factored generated root list building into smaller config/regular-ngon/legacy-quartic helpers.
- [x] Pass 5: Reran focused native rail, runtime publish, published runtime proof, and code-quality baseline after the refactor; no additional blocking defect found.
- [x] Pass 6: Full contracted runtime file found stale secondary-root proof expecting Color Root output to change from Dynamics-only edits; fixed the test to mutate scoped Color Root fields explicitly and reran the full runtime file green.

## Audit Findings

- [x] Finding 1: Color Root Field initially had only layout/count coverage; added scoped seed/root-spread/phase/phase-strength storage, binding, save/load, reporting, and runtime proof.
- [x] Finding 2: The first implementation did not prove invalid root-pattern refs fail closed at the row-edit seam; added a focused native rejection assertion.
- [x] Finding 3: The generated-root helper used by Color Pipeline source rows currently resolves without a caller fractal type and uses the root-field consumer lane semantics. This is acceptable for current root-field consumers, but it is recorded as a future seam if cluster-specific root-source semantics are added.
- [x] Finding 4: Generated-root list reuse inflated `escape_time_coloring.h` past the code-quality baseline; refactored the helper and reran native/runtime/code-quality proof.
- [x] Finding 5: An older runtime test still encoded the pre-scope ambiguity by expecting Color Root Field frames to change when only Dynamics fields changed; updated it to mutate Color Root Field authority explicitly.

## Stop Point

Stop after the divergent scoped root-pattern authority proof is validated, checkpointed, receipted, rearward-reviewed, and pushed. Preplanned hardening work is exhausted at that point; stop for replan before new fractal families, graph UI, or SDF work.
