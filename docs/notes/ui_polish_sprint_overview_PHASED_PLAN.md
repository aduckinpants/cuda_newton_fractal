# UI Polish Sprint Overview

## Current Phase

Phase 1 - complete Phase 0 foundation and branch-topology preparation

## Phase Checklist

- [ ] Phase 1 - complete Phase 0 foundation and branch-topology preparation
- [ ] Phase 2 - land slice 1 for schema domains and control polish
- [ ] Phase 3 - land slice 2 for color-mode authority and UI cleanup
- [ ] Phase 4 - land slice 3 for render-resolution defaults and pacing policy
- [ ] Phase 5 - run integration audit and close the sprint follow-up ledger

## Explicit User Asks

- [open] Do a short UI polish pass on the project.
- [open] Fix slider values that are not covering the proper domains.
- [open] Improve how the color mode is done.
- [open] Raise the starting render resolution from the current low default.
- [open] Cleanly merge to master and then start a new branch for each feature slice.
- [open] Document the work structure in the newer phased style before feature implementation.

## Presumption Loop

The likely failure mode is not missing ideas; it is starting feature work from a blended branch without durable slice boundaries. This sprint therefore starts with a Phase 0 foundation thread, then moves through three bounded feature slices with one feature branch per slice.

Hostile review assumes the right decomposition is by authority seam: schema domains first, color-mode authority second, and render-resolution/pacing policy third. If a later slice proves that ordering wrong, the overview plan must record the changed dependency before the next branch starts.

Each slice still follows local TDD and hostile audit rules. This overview plan is the ordering surface, not a replacement for slice-local regressions or contracts.

## Presumption Evidence

- Owner Proof: the current repo has one UI-schema authority surface, one color-mode/runtime surface, and one render-pacing/defaults surface, which supports a three-slice split instead of one blended branch.
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

- Planned branch topology after Phase 0:
  - merge the current foundation-ready state to `master`
  - create `feature/ui-polish-integration` for the UI polish thread
  - branch one feature slice at a time from that sprint integration point:
    - `feature/ui-polish-schema-domains`
    - `feature/ui-polish-color-authority`
    - `feature/ui-polish-resolution-pacing`
- Planned slice sequence:
  - slice 1: `docs/notes/ui_polish_slice1_schema_domains_PHASED_PLAN.md`
  - slice 2: `docs/notes/ui_polish_slice2_color_mode_authority_PHASED_PLAN.md`
  - slice 3: `docs/notes/ui_polish_slice3_render_resolution_pacing_PHASED_PLAN.md`
- Current stop point discipline:
  - do not start the feature branches until Phase 0 records the merge/branch-ready checkpoint explicitly
  - keep future programmable-color or Salticid-adjacent exploration out of this sprint overview until the user reopens that topic separately

## Resume Point

Finish the Phase 0 foundation slice, then reopen this overview plan from the fresh sprint integration branch and start slice 1.