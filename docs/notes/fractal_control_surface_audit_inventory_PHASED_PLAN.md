# Fractal Control Surface Audit Inventory

## Current Phase

Phase 2 - validation green; checkpoint, receipts, push, clean-tree check, and stale-plan proof remain as mechanical closure steps for this documentation-only slice.

## Explicit User Asks

- [x] Document a future series of work for native cross-board Explaino/fractal exploration expansion.
- [x] Go through every defined fractal and list parameters that are not exposed as user sliders/controls.
- [x] List sliders/visible controls that are not currently proven to affect output or are statically suspect.
- [x] Do not fix behavior in this slice; start with documentation so the list can be cross-checked before RED tests and repairs.

## Phase Checklist

- [x] Phase 0 - bootstrap, branch, and inspect current schema/runtime surfaces.
- [x] Phase 1 - write the all-fractal control-surface inventory and future-work note.
- [x] Phase 2 - validate plan/contract/doc-only closeout, hostile audit, checkpoint, receipts, push, clean tree, and stale-plan proof.

## Proof Ledger

- Starting branch: `codex/fractal-control-surface-audit-docs`.
- Starting head: `c774128`.
- Bootstrap reports clean tree and closed `magnet_quick_benefits` active contract before this new documentation slice.
- Static inventory inputs: `ui_app/src/fractal_types.h`, `ui/fractal_binding_surface_v1.ui_schema.json`, `ui_app/src/schema_binding.cpp`, `ui_app/src/fractal_sample_device.inl`, `ui_app/src/escape_time_direct_formulas.h`, `ui_app/src/escape_time_specialized_formulas.h`, `ui_app/src/fractal_derived_fields.cpp`, `ui_app/src/fractal_family_rules.h`, and existing runtime/native tests.
- Schema inventory pass found 44 `fractal_type` options, matching the 44 `FractalType` ids.
- Documentation artifact: `docs/notes/fractal_control_surface_audit_inventory.md`.
- Product/test mutation status: none intended in this slice.
- Validation command passed: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/fractal_control_surface_audit_inventory.contract.json --out-json artifacts/validation/fractal_control_surface_audit_inventory_contract.json`.
- Validation command passed: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`.
- Validation command passed: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/fractal_control_surface_audit_inventory_PHASED_PLAN.md --out-json artifacts/validation/fractal_control_surface_audit_inventory_hostile_audit.json`.
- Validation command passed: `git diff --check`.

## Hostile Audit

- Status: complete
- Required posture: assume the inventory misses a fractal, confuses a global control with a family control, treats "mentioned in tests" as proof of UI/UX harness behavior, or overstates a static suspicion as a proven dead slider.

## Audit Passes

- [x] Pass 1 - checked the inventory against enum ids, schema fractal options, and binding-visible controls; no missing fractal option found.
- [x] Pass 2 - checked suspected missing controls against runtime formula usage; recorded real static suspects instead of calling them fixed or broken.
- [x] Pass 3 - checked proof-gap claims against existing no-mouse UI/UX harness tests; direct rendered-frame proof is currently much narrower than the visible schema surface.
- [x] Clean re-read pass - no additional real defect found in the documentation boundary after separating static suspects from proven dead sliders.
- [x] Third clean re-read - no additional workflow mistake found; the inventory remains doc-only and does not mutate product code or tests.

## Audit Findings

- [x] Real finding: standalone `julia` currently hard-codes its Julia constant and exposes no `julia_c_real` / `julia_c_imag` controls.
- [x] Real finding: standalone `nova` exposes quartic/custom polynomial selection while hiding `poly_c4`, even though the Nova runtime reads all five polynomial coefficients.
- [x] Real finding: the current no-mouse UI/UX rendered-frame proof covers Magnet controls and Explaino registry axes, but not most visible family controls across the schema.
- [x] Clean re-read evidence: the document classifies McMullen, Collatz, fixed-formula escape families, and generated/internal Explaino fields as policy/coverage questions rather than falsely proven dead sliders.

## Boundaries

In scope:
- documentation-only inventory and future-work plan
- static control-surface analysis
- existing-test proof-gap analysis

Out of scope:
- product fixes
- new RED tests
- runtime/schema/control mutations outside documentation
- broad renderer or Color Pipeline changes
