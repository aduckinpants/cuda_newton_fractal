# Advanced Color Library Foundation Phase 5 Recovery

## Current Phase

Complete - the planning-repair slice closed at `433829c` under `ck:e33c073e`, and this follow-up sync repair keeps the main foundation plan plus the Shape-stack subplan aligned with that landed state

## Phase Checklist

- [x] Phase 1 - add the recovery contract/plan pair and repair the main foundation plan so it stops claiming Grading is the next executable step while the stack/backend requirement is still open
- [x] Phase 2 - validate phased-plan sync and contract shape, then checkpoint the planning-repair slice cleanly

## Explicit User Asks

- [done] Treat the already-mapped advanced-color MVP as binding authority instead of making me restate it.
- [done] Follow the mainline CUDA phased-plan/contract workflow instead of chat-only sequencing.
- [done] Correct the obvious advanced-color priority drift automatically.
- [done] Start implementation now.

## Presumption Loop

The controlling defect is no longer uncertainty about the advanced-color function set. The mapped inventory already exists in `docs/notes/advanced_color_library_foundation_oracle_and_inventory.md`, and the lane-stack requirement already exists in `docs/notes/advanced_color_pipeline_slice7_catalog_runtime_binding_PHASED_PLAN.md`. The current blocker is that `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md` is not executable under the repo workflow: it says Phase 5 is in progress while the checklist leaves Phase 5 unchecked, it still has open Explicit User Asks without hostile-audit sections, and it hand-waves the next step as "continue at Phase 5" instead of a bounded next slice. The local hypothesis is that a dedicated recovery plan/contract plus a repaired main foundation plan will restore one truthful planning authority for the next backend-recovery slice without redefining the mapped inventory. The cheapest disconfirming checks are the contract validator and phased-plan sync adapter.

## Presumption Evidence

- `docs/notes/advanced_color_library_foundation_oracle_and_inventory.md` already locks the four-category foundation and the initial function inventory for Source, Shape, Palette, and Grading.
- `docs/notes/advanced_color_pipeline_slice7_catalog_runtime_binding_PHASED_PLAN.md` already maps the lane-stack requirement: Source / Shape / Palette lane stacks, plus-button row insertion, ordered composition, and runtime-real-only shipping.
- Hostile review confirmed that `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md` had drifted into a workflow-invalid state: it said "Phase 5 in progress" while Phase 5 was unchecked, carried open Explicit User Asks without hostile-audit sections, and pointed straight to Grading instead of the backend-recovery blocker.
- `ui_app/src/color_pipeline_window.h` still initializes, imports, snapshots, and applies the live path through single-row helpers and one-enabled-row live-bridge checks, so backend recovery must come before any honest Grading follow-on.

## Proof Ledger

- Landed: `docs/notes/advanced_color_library_foundation_phase5_recovery_PHASED_PLAN.md` and `docs/contracts/advanced_color_library_foundation_phase5_recovery.contract.json` now bound the planning-repair slice so the correction happens under the repo's phased-plan and contract workflow.
- Landed: `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md` now treats the mapped inventory as settled, sequences backend recovery ahead of Grading, adds hostile-audit sections, maps explicit asks to remaining phases, and replaces the vague direct-to-Grading resume point.
- Landed: `docs/notes/advanced_color_library_foundation_phase5_shape_stack_runtime_PHASED_PLAN.md` is now reconciled with the landed `e6b55b9` / `ck:1fbf6d4d` checkpoint instead of still pointing at already-closed closure work.
- Validated: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` now passes for both the repaired foundation plan and this recovery plan, and `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/advanced_color_library_foundation_phase5_recovery.contract.json --out-json artifacts/validation/advanced_color_library_foundation_phase5_recovery_contract.json` validated the new recovery contract cleanly.
- Checkpointed: the original planning-repair slice closed at commit `433829c` with the linked `ck:e33c073e` handoff entry plus machine-written validation and contract proof receipts.

## Hostile Audit

- Status: done
- Required posture: assume the current foundation plan is wrong enough to mis-sequence the next slice until the checked-in plan, contract, and validation rails prove otherwise.

## Audit Passes

- [done] Pass 1 - inspected the main foundation plan, repaired the phase drift, added hostile-audit state, mapped the explicit asks to the remaining phases, and replaced the vague direct-to-Grading resume point.
- [done] Pass 2 - inspected the repaired planning surfaces against the real single-row backend seams and kept the next slice anchored to the one-enabled-row live bridge plus flat-field parameter blockers instead of vague stack language.
- [done] Pass 3 - re-read the repaired state after validation and confirmed the next executable implementation slice is backend recovery rather than a vague Grading continuation.

## Audit Findings

- [done] Real defect found and repaired: the main foundation plan previously said Phase 5 was in progress and pointed straight to Grading even though the mapped lane-stack requirement remained open and the plan lacked the hostile-audit sections required by the current repo workflow.
- [done] Real defect found and retained as the next technical blocker: the live advanced-color backend still hard-rejects more than one enabled row per lane and still depends on single-row import/snapshot helpers, so the next truthful implementation slice must repair backend/state authority before any honest Grading continuation.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_library_foundation_phase5_recovery_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_library_foundation_phase5_recovery.contract.json`
  - `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`
- Non-goals for this slice:
  - do not redefine the mapped advanced-color function inventory
  - do not start Grading implementation yet
  - do not start the common-fractal wave yet
  - do not promote the CUDA catalog refactor ahead of advanced-color recovery
  - do not touch runtime/editor code in this planning-repair slice

## Resume Point

Treat this as a completed predecessor. Resume from `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md` and start the next bounded backend-recovery slice on Source/Palette live-bridge and runtime-state authority instead of reopening already-closed checkpoint work.
