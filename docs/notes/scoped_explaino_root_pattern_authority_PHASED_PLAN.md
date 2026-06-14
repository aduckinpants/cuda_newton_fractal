# Scoped ExplainO Root-Pattern Authority Phased Plan

## Explicit User Asks

- Replace the hidden `primary` / `secondary` Pattern A/B workflow with scoped, role-labelled root-pattern controls.
- Treat this as a general authority model for every current and future root-aware preset/lane, not another one-off toggle repair.
- Every visible root-pattern control must belong to a named root-pattern scope, and actions such as Prev Seed / Next Seed must mutate only that scope.
- Root-field consumers and root-aware Color Pipeline rows must report which scoped root field they used.
- `explaino_magnet_root_well` must keep the base Magnet controls and expose the normal root-field movement controls through a scoped section.
- Existing states must remain loadable, but new normal UI/report language must use role labels instead of Pattern A/B vocabulary.
- Do not start graph UI, arbitrary root banks, Salticid/ui.salt integration, custom N-root coordinate banks, new fractal families, or SDF work under this campaign.

## Current Phase

Phase 7 - Implementation and runtime proof are green; formal validation, receipts, rearward review, push, and clean-tree closeout remain.

## Phase Checklist

- [x] Phase 0: Merge/push the completed Magnet Root Well control repair into `master`, branch `codex/scoped-explaino-root-pattern-authority`, and create this plan/contract.
- [x] Phase 1: Add RED tests proving current global/ambiguous root-pattern controls and reports.
- [x] Phase 2: Add scoped root-pattern role/ref model and compatibility aliases.
- [x] Phase 3: Add scoped action dispatch and scoped descriptor resolution.
- [x] Phase 4: Replace composite root-aware lane UI with role-labelled root-pattern sections.
- [x] Phase 5: Route root-field consumers and root-aware Color Pipeline rows through scoped refs.
- [x] Phase 6: Update state, runtime reports, Capture Finding `fractal-state.json`, and compatibility loading.
- [ ] Phase 7: Hostile audit, formal validation, checkpoint, receipts, rearward review, push, and stop for hardening/replan.

## Scope Lock

This campaign may change only root-pattern authority, scoped root-pattern UI/action/report surfaces, tests, docs, and compatibility loading required to remove the current A/B/global ambiguity. It must not add new root math, new fractal families, new SDF ops, graph-editor UI, unlimited root-pattern banks, custom N-root coordinate editing, or Salticid/ui.salt integration.

## Concrete Problem

Current root-aware lanes still reuse global ExplainO controls and hidden `primary` / `secondary` refs. That makes the UI and automation ambiguous once one lane or preset has more than one root system. For example, `explaino_magnet_root_well` now shows base Magnet controls and global ExplainO root controls, but the UI cannot tell whether a `Prev Seed` action is mutating dynamics roots, color roots, trap roots, or a later secondary root system. The previous Pattern B work had the right backend concept but exposed the wrong user-facing model.

## Desired Authority Model

Root-pattern controls are reusable components, but each instance is scoped by a role.

V1 roles:

- `dynamics_root_field` - labelled `Dynamics Root Field`; normal root-field consumer authority.
- `color_root_field` - labelled `Color Root Field`; root-aware Color Pipeline row authority when the row deliberately uses a second field.

Compatibility aliases:

- old `primary` state/report values load as `dynamics_root_field`.
- old `secondary` state/report values load as `color_root_field`.

Normal UI vocabulary must use role labels, not Pattern A/B. Pattern A/B may remain as legacy/advanced compatibility terminology only.

## Root-Pattern Section Contract

Each root-pattern scope may expose a section containing some or all of:

- scoped `Prev Seed` and `Next Seed` actions
- scoped `Seed`
- scoped `Root Spread`
- scoped `Phase`
- scoped `Phase Strength`
- scoped `Generated Layout`
- scoped `Root Count`
- scoped custom root coordinates only for scopes that explicitly support custom authority

All control ids and action paths must be scoped. Example action paths:

- `fractal.actions.root_pattern.dynamics.prev_seed`
- `fractal.actions.root_pattern.dynamics.next_seed`
- `fractal.actions.root_pattern.color.prev_seed`
- `fractal.actions.root_pattern.color.next_seed`

Old unscoped action paths remain only for old single-root ExplainO lanes where exactly one root field is possible.

## Target UI Shape

For `explaino_magnet_root_well`, the normal layout becomes:

1. `Base Magnet`
   - `magnet_seed_real`
   - `magnet_seed_imag`
   - `magnet_relaxation`
   - `magnet_bailout`
2. `Dynamics Root Field`
   - scoped seed actions and movement/root-layout controls
3. `Root Well Consumer`
   - root well strength/scale controls
   - consumes `Dynamics Root Field`

Root-aware Color Pipeline Source rows expose a role-labelled pattern selector for rows that support root fields:

- `Dynamics Root Field`
- `Color Root Field`

Non-root-aware Source rows must not show root-pattern controls.

## Compatibility Rules

- Existing states with `primary` / `secondary` refs must load.
- Existing reports may keep flat legacy fields, but new report authority is `root_patterns[]` and `root_pattern_consumers[]` with scoped role ids.
- Existing `explaino_seed`, `explaino_root_spread`, `explaino_phase`, `explaino_generated_root_layout`, and `explaino_generated_root_count` map to `dynamics_root_field` for compatibility.
- Existing secondary generated layout/count fields map to `color_root_field` for compatibility.
- Derived/effective roots are never written back as custom authority.

## RED Targets

Before implementation, add failing tests that prove the current product gap:

- `prev_seed` / `next_seed` are global action paths on root-aware composite lanes.
- Root-field consumer lanes expose one global `explaino_seed` surface instead of a scoped section.
- Root-aware Color Pipeline row choices still carry A/B-style refs rather than role-labelled scoped refs.
- Runtime report text cannot distinguish dynamics root controls from color root controls as the main review surface.
- Existing no-mouse action coverage can miss scoped action bugs because it exercises global action paths.

## Proof Ledger

| Item | Status | Evidence |
| --- | --- | --- |
| Bootstrap | done | `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` on clean `codex/explaino-magnet-root-well-control-repair` head. |
| Repo status | done | `py -3.14 tools/viewer_host_repo_status.py` reported clean before merge and branch. |
| Rearward review | done | `py -3.14 tools/viewer_host_rearward_review.py` returned `ok` for `3b7cd92`. |
| Merge/push previous repair | done | `master` fast-forwarded from `5d6a6a1` to `3b7cd92` and pushed to origin. |
| Plan/contract | done | This plan and `docs/contracts/scoped_explaino_root_pattern_authority.contract.json`; contract revised to include `docs/ui_salt`. |
| RED tests | done | Initial focused rail failed on global `explaino_seed`/unscoped root controls for composite root-aware lanes, then implementation repaired it. |
| Native validation | done | `ui_app\build_tests_vsdevcmd.cmd test_ui_schema test_safe_mode_schema test_schema_binding test_color_pipeline_core test_diagnostics_state_io test_diagnostics_capture test_viewer_ui_automation_report` passed in `artifacts/scoped_explaino_root_pattern_authority/native_scoped_root_pattern_step3.log`. |
| Runtime publish | done | `ui_app\build_vsdevcmd.cmd` passed in `artifacts/scoped_explaino_root_pattern_authority/runtime_publish_step3.log`. Earlier 5-minute wrapper timeout was retried with a longer timeout and completed cleanly. |
| Published runtime proof | done | `py -3.14 -m pytest tests\test_fractal_runtime_root_field_consumers.py -q --junitxml artifacts\pytest\scoped_explaino_root_pattern_authority_runtime.junit.xml` passed in `artifacts/scoped_explaino_root_pattern_authority/runtime_root_field_consumers_step3.log`. |
| Stale vocabulary scan | done | `rg` scans found no `Pattern A/B` normal product vocabulary and no serialized `pattern_ref`/`root_pattern_ref` use of old `primary`/`secondary` strings in scoped proof surfaces. |
| Hostile audit | in progress | Audit findings were found and repaired; formal validator still pending. |
| Receipts/rearward/push | pending | Required before final closeout. |

## Implementation Slices

### Phase 1 - RED Ambiguity Tests

Add or update focused tests proving the current ambiguity. Expected starting failures should identify unscoped controls/actions and A/B vocabulary surfaces.

Candidate rails:

- `test_ui_schema`
- `test_safe_mode_schema`
- `test_schema_binding`
- `test_viewer_ui_automation_report`
- `test_color_pipeline_core`
- `tests/test_fractal_runtime_root_field_consumers.py`

### Phase 2 - Core Scoped Authority

Add root-pattern scope/ref types and helper APIs. The helpers must centralize:

- role id normalization
- legacy alias mapping
- role labels
- descriptor construction from existing storage
- invalid/missing ref fail-closed reasons

### Phase 3 - Scoped Actions

Add scoped action dispatch for dynamics and color root fields. Preserve old global action paths only for old single-root ExplainO lanes.

### Phase 4 - Schema/UI Sections

Add reusable scoped root-pattern schema/safe-mode sections. Composite root-aware lanes must use scoped sections instead of the global ExplainO seed cluster.

### Phase 5 - Consumers And Color Pipeline

Route root-field consumers and root-aware Color Pipeline rows through scoped refs. Existing presets and current root-aware runtime tests must migrate to scoped refs.

### Phase 6 - State/Report/Capture

Update save/load, runtime reports, and Capture Finding review sidecars so scoped role ids are the main truth. Legacy flat fields may remain only for compatibility.

### Phase 7 - Hostile Audit And Closure

Run a hostile review over every root-aware lane/preset/report path touched by this campaign. Repair any one-off lists or visible inert controls found by the audit before checkpointing.

## Test Targets

- Native tests prove scoped descriptors resolve deterministic roots and hashes.
- Native tests prove scoped actions mutate only their owning root pattern.
- State tests prove old `primary` / `secondary` states load as `dynamics_root_field` / `color_root_field`.
- Schema/safe-mode/binding tests prove scoped sections appear on root-field consumer lanes and unscoped root controls do not.
- Color Pipeline tests prove `root_proximity` and `root_phase` use role-labelled scoped refs and old aliases load.
- Runtime no-mouse proof selects `explaino_magnet_root_well`, mutates scoped dynamics controls, mutates scoped color controls through a root-aware row, and proves report refs and frame hashes change as expected.
- Capture Finding/reload/replay proof preserves scoped descriptors and frame hashes.

## Hostile Audit

- Status: complete

Required questions:

- Did every visible root-pattern control belong to a named scope?
- Did scoped Prev/Next Seed mutate only the owning scope?
- Did `explaino_magnet_root_well` keep base Magnet controls and gain scoped root movement controls?
- Did root-aware Color Pipeline rows use role-labelled scoped refs instead of A/B UI vocabulary?
- Did reports and `fractal-state.json` expose scoped `root_patterns[]` and `root_pattern_consumers[]` as the main truth?
- Did old states load through aliases without keeping half legacy/half revised product surfaces?
- Did I avoid hidden one-off fractal lists that will repeat this bug for the next root-field consumer?
- Did no-mouse proof exercise action controls, not only set-value sliders?

## Audit Passes

- [x] Pass 1: Found stale Capture Finding sidecar/test fallback to `primary` after scoped reports were introduced.
- [x] Pass 2: Found runtime proof still using old role refs and missing real action-click coverage; repaired to click scoped action IDs and assert root/frame changes.
- [x] Pass 3: Found dead safe-mode Pattern B/Dynamics Root Pattern builders and stale test messages; removed/renamed them and reran native/runtime proofs.
- [x] Pass 4: Clean re-read the repaired state; no additional real defect found in stale Pattern A/B vocabulary scans, scoped runtime report refs, action-click coverage, or safe-mode source.

## Audit Findings

- [x] Capture Finding sidecar had a stale `primary` fallback and test expectation; repaired to `dynamics_root_field` plus `Dynamics Root Field` label.
- [x] Runtime tests still configured `primary`/`secondary` in new-state paths and expected the wrong action control IDs; repaired to role-labelled refs and actual click action IDs.
- [x] Safe-mode schema retained unused Pattern B/Dynamics Root Pattern builders; removed them so fallback schema source cannot reintroduce the old UX.
- [x] The first runtime publish hit a 5-minute timeout while compiling CUDA; reran the checked-in publish with a longer timeout and recorded the successful log.
- [x] Clean re-read confirmed the repaired state: no additional real issue found by stale-vocabulary scans, focused native bundle, runtime publish, or no-mouse root-field consumer proof.

## Stop Point

Stop after scoped authority is implemented for all current root-aware lanes/presets, validated, checkpointed, receipted, rearward-reviewed, pushed, and the next product direction is left for hardening/replan. Preplanned scoped-authority work is exhausted at that point; stop for replan before graph UI, arbitrary pattern lists, custom N-root coordinate banks, new fractal families, or SDF work.
