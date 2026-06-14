# Scoped Color Root Control Authority Repair Phased Plan

## Explicit User Asks

- Repair the Color Root Field controls that still feel inert after the scoped Dynamics control repair.
- Treat this as the same visible-control authority class of bug, not as a visual preference.
- Do not remove backend/state/report compatibility for `color_root_field`.
- Prove Color Root controls are shown only when they are consumed by active root-aware Color Pipeline rows.

## Current Phase

Closed - implementation, proof, checkpoint, receipts, rearward review, push, and clean-tree closeout are complete for this repair.

## Phase Checklist

- [x] Phase 0: Create plan/contract on `codex/scoped-color-root-control-authority-repair`.
- [x] Phase 1: Add RED/native/runtime proof that Color Root controls are visible without an active Color Root consumer.
- [x] Phase 2: Add one shared active Color Root consumer predicate for schema/binding/report tests.
- [x] Phase 3: Repair schema visibility and no-mouse proof for inactive-vs-active Color Root controls.
- [x] Phase 4: Hostile audit, validation, checkpoint, receipts, rearward review, push.

## Scope Lock

This repair may change the UI schema, schema binding predicates, focused tests, this plan/contract, and handoff log. It must not add new fractal types, start new SDF work, remove `color_root_field` state compatibility, add graph UI, or use physical mouse automation.

## Concrete Problem

Manual testing shows the scoped `Color Root Field` controls can still appear inert. Inspection confirms the schema exposes `Color Prev Seed`, `Color Next Seed`, `Color Seed`, `Color Root Spread`, `Color Phase`, `Color Phase Strength`, `Color Generated Layout`, and `Color Root Count` on all composite root-field lanes. Those controls only affect pixels when an active `root_proximity` or `root_phase` Color Pipeline source row consumes `color_root_field`. When the active row stack uses `dynamics_root_field` or no root-aware row, the controls write storage but do not own the visible render.

## Required Behavior

- `Color Root Field` controls are visible when an enabled root-aware Color Pipeline source row consumes `color_root_field`.
- The same controls are hidden when the active row stack has no `color_root_field` consumer.
- `Dynamics Root Field` controls remain visible and consumed for root-field consumer lanes.
- Existing states with `color_root_field` rows still load and expose Color Root controls.
- Color Root actions mutate only Color Root authority and do not mutate Dynamics Root Field.
- No physical mouse automation is used.

## Proof Ledger

| Item | Status | Evidence |
| --- | --- | --- |
| Rearward review | done | `py -3.14 tools/viewer_host_rearward_review.py` returned `ok` for `3c99aa8`. |
| Branch | done | `codex/scoped-color-root-control-authority-repair`. |
| Plan/contract | done | This plan and `docs/contracts/scoped_color_root_control_authority_repair.contract.json`, locked with checkpoint token `ck:eec1a0a1`. |
| Native validation | done | `test_schema_binding` passed to `artifacts/scoped_color_root_control_authority_repair/native_schema_binding.log`; `test_ui_schema` passed to `artifacts/scoped_color_root_control_authority_repair/native_ui_schema.log`. |
| Runtime publish | done | `ui_app/build_vsdevcmd.cmd > artifacts/scoped_color_root_control_authority_repair/runtime_publish.log 2>&1` passed. |
| Published runtime proof | done | Two-test no-mouse proof passed to `artifacts/pytest/scoped_color_root_control_authority_repair_runtime.junit.xml`; it covers hidden inactive Color Root controls and active Color Root sensitivity. |
| Hostile audit | done | Findings below were repaired and hostile-audit validation passed for the closed repair. |
| Receipts/rearward/push | done | `b88ed71` has an `ok` rearward-review artifact and is pushed to `origin/codex/scoped-color-root-control-authority-repair`. |

## Hostile Audit

- Status: complete

Required questions:

- Do Color Root Field controls still appear without an active `color_root_field` source-row consumer?
- Does an active `color_root_field` row still expose and consume those controls?
- Do Dynamics Root Field controls remain visible and active on root-field consumer lanes?
- Did the repair avoid deleting backend/state/report compatibility for old Color Root states?
- Did runtime proof check independent visible-control behavior rather than one grouped hash?

## Audit Passes

- [x] Pass 1: Found Color Root Field controls were globally visible on composite root-field lanes even when no active source row consumed `color_root_field`.
- [x] Pass 2: Found Color Root Count visibility also needed to include the active Color Root consumer predicate, not just regular N-gon layout.
- [x] Pass 3: Clean re-read confirmed Dynamics Root controls remain visible, active Color Root rows expose Color controls, inactive Color Root controls are hidden, and focused native/runtime proofs are green.

## Audit Findings

- [x] Finding 1: Color Root Field controls were visible without an active `color_root_field` Color Pipeline consumer, creating storage-only controls that looked inert.
- [x] Finding 2: Color Root Count used only layout state for visibility and could become visible even when the Color Root Field section itself had no active consumer.

## Stop Point

Stop after Color Root control visibility matches active consumption, the repair is validated, checkpointed, receipted, rearward-reviewed, and pushed. Preplanned sliced work for this repair is exhausted at that point; stop for replan before new product work.
