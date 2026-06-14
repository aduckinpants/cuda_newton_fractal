# Scoped Root-Pattern Control Authority Repair Phased Plan

## Explicit User Asks

- Repair the visible-control authority problem where nearly every control on `explaino_magnet_root_well` feels inert or dead.
- Treat this as a general control-authority problem, not another one-off Magnet patch.
- Prove the UI no longer exposes old unscoped root/animation targets for composite root-field lanes.
- Prove each visible Base Magnet and scoped Root Field control independently affects the runtime or is intentionally hidden.

## Current Phase

Phase 4 - Implementation and proof are green; checkpoint, receipts, rearward review, push, and clean-tree closeout remain.

## Phase Checklist

- [x] Phase 0: Create plan/contract on `codex/scoped-root-pattern-control-authority-repair`.
- [x] Phase 1: Add RED tests for stale unscoped animation targets and weak grouped-control runtime proof.
- [x] Phase 2: Repair schema/binding/animation target authority for composite root-field lanes.
- [x] Phase 3: Add independent no-mouse sensitivity proof for visible Magnet and scoped Root controls.
- [ ] Phase 4: Hostile audit, validation, checkpoint, receipts, rearward review, push.

## Scope Lock

This repair may change the UI schema, binding/animation dispatch, focused tests, this plan/contract, and handoff log. It must not add new fractal types, restart SDF feature work, restore Pattern B as normal UI, add graph UI, or use physical mouse automation.

## Concrete Problem

Manual testing shows `explaino_magnet_root_well` still presents controls that look authoritative but do not move the active visual path. Initial inspection found stale animation dropdown entries for old global `Seed`, `Root Spread`, and `Explaino Phase` on composite root-field lanes. Those mutate unscoped legacy fields while the user expects the scoped `Dynamics Root Field` controls to own movement. The runtime test also batches Base Magnet edits, so one live control can mask dead siblings.

## Required Behavior

- Composite root-field lanes expose scoped root controls/actions, not ambiguous unscoped root animation targets.
- `explaino_magnet_root_well` visible controls are consumed independently:
  - Magnet Seed Real
  - Magnet Seed Imag
  - Magnet Relaxation
  - Magnet Bailout
  - Dynamics Seed
  - Dynamics Root Spread
  - Dynamics Phase
  - Dynamics Phase Strength
  - Dynamics Generated Layout
  - Dynamics Root Count when regular N-gon is active
  - Root Trap Strength
  - Root Trap Scale
- Color Root controls remain Color Pipeline/root-aware source controls; they must not be confused with dynamics movement.

## Proof Ledger

| Item | Status | Evidence |
| --- | --- | --- |
| Rearward review | done | `py -3.14 tools/viewer_host_rearward_review.py` returned `ok` for `c433026`. |
| Branch | done | `codex/scoped-root-pattern-control-authority-repair`. |
| Plan/contract | done | This plan and `docs/contracts/scoped_root_pattern_control_authority_repair.contract.json`, locked with checkpoint token `ck:9c1b1ddd`. |
| Native validation | done | `test_schema_binding` passed to `artifacts/scoped_root_pattern_control_authority_repair/native_schema_binding.log`; extra `test_ui_schema` passed to `artifacts/scoped_root_pattern_control_authority_repair/native_ui_schema.log`. |
| Runtime publish | done | `ui_app/build_vsdevcmd.cmd > artifacts/scoped_root_pattern_control_authority_repair/runtime_publish.log 2>&1` passed. |
| Published runtime proof | done | `tests/test_fractal_runtime_root_field_consumers.py::test_explaino_magnet_root_well_base_magnet_controls_are_visible_and_active` passed and now checks independent per-control sensitivity. |
| Hostile audit | done | Findings below were repaired; final audit validation passed. |
| Receipts/rearward/push | pending | Required before closeout. |

## Hostile Audit

- Status: complete

Required questions:

- Did the UI still expose any old unscoped root animation target on composite lanes?
- Did each visible Magnet and Dynamics control independently change a frame/root/report value?
- Did scoped actions mutate only their owner scope?
- Did the repair avoid hiding real useful controls merely to satisfy tests?
- Did existing root-field consumer runtime proof remain green?

## Audit Passes

- [x] Pass 1: Found stale legacy animation targets for `seed`, `root_spread`, and `explaino_phase` on composite root-field lanes; removed those lanes from the legacy dropdown options.
- [x] Pass 2: Found runtime proof only batch-edited Base Magnet controls; rewrote it to reset state and prove each Base Magnet, Root Well, and Dynamics Root Field control independently changes the frame or root hash.
- [x] Pass 3: Clean re-read confirmed scoped controls remain visible, global legacy root controls stay absent, schema/native/runtime proofs are green, and no Pattern B/SDF/new-family scope was added.

## Audit Findings

- [x] Finding 1: Animation dropdown still exposed unscoped root targets on composite root-field lanes, creating dead/inert target choices.
- [x] Finding 2: Runtime control proof was too weak because a group edit could mask dead individual controls.

## Stop Point

Stop after the visible-control authority repair is validated, checkpointed, receipted, rearward-reviewed, and pushed. Preplanned sliced work for this repair is exhausted at that point; stop for replan before new product work.
