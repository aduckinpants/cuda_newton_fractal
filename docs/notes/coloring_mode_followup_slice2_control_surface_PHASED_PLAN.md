# Coloring Mode Follow-Up Slice 2 - Control Surface And State Migration

## Current Phase

Phase 1 - wait for slice 1 closeout and define the UI/state migration surface

## Phase Checklist

- [ ] Phase 1 - wait for slice 1 closeout and capture the current schema/state surface
- [ ] Phase 2 - define the split signal/palette/grading control model and legality rules
- [ ] Phase 3 - implement the schema, binding, and state migration changes
- [ ] Phase 4 - hostile audit the resulting UI/state authority and checkpoint the slice

## Explicit User Asks

- [open] Keep one obvious public coloring path rather than splintering the UI again.
- [open] Keep the thread separate from non-color follow-ups.
- [open] Stay on the simple viewer-host path rather than a DSL-first design.

## Presumption Loop

This slice should only begin after slice 1 proves the runtime can synthesize split-color state without breaking old `coloring_mode` loads. The likely owner is the schema/binding/state-I/O seam, not a new metadata registry.

Hostile review assumes the main risk is creating a second source of truth for defaults or legality. The UI filtering must still flow from runtime rules, not from standalone schema folklore.

## Presumption Evidence

- Owner Proof: slice 2 will sit on top of the already-landed single public `coloring_mode` surface from the UI polish sprint.
- RED Witness: pending.
- Fix Proof: pending.
- Hostile Review Pass 1: pending.
- Hostile Review Pass 2: pending.

## Proof Ledger

- Manual RED: pending.
- Checked-in regression RED: pending.
- First GREEN: pending.
- Post-green hostile finding: pending.

## Notes

- Expected owner files:
  - `ui/fractal_binding_surface_v1.ui_schema.json`
  - `ui_app/src/schema_binding.cpp`
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/tests/test_ui_schema.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
  - `ui_app/tests/test_diagnostics_state_io.cpp`
- Non-goals:
  - do not reopen per-family duplicated public controls
  - do not widen into Salticid, parser, or AST work

## Resume Point

Slice 2 is blocked on slice 1. Once the runtime split is stable, migrate the state/UI surface without breaking the single public control authority.