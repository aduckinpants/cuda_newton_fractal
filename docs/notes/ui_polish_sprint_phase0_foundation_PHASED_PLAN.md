# UI Polish Sprint Phase 0 Foundation

## Current Phase

Complete

## Phase Checklist

- [x] Phase 1 - audit the current branch state and lock the Phase 0 foundation scope
- [x] Phase 2 - uplift the local workflow docs and phased-plan protocol to the selected mainline-style sections
- [x] Phase 3 - create the durable UI sprint planning surfaces for the later feature slices
- [x] Phase 4 - validate the Phase 0 foundation and record the merge/branch ready stop point

## Objective

Complete the non-feature foundation work for the UI polish sprint before any UI implementation branch starts.

Success means:

- the repo has a dedicated Phase 0 plan and contract for this sprint-foundation work
- the local workflow docs and phased-plan protocol carry the selected mainline-style planning sections
- the UI polish sprint has durable planning surfaces in the repo instead of chat-only intent
- the stop point for the later merge-to-master and sprint-branch workflow is explicit and resumable

## Non-goals

- do not implement the actual UI polish feature slices in this Phase 0 thread
- do not decide the post-Phase-0 programmable color direction in this plan
- do not blend later off-topic architecture details into the branching/doc foundation work
- do not relax the repo's existing checkpoint, contract, or receipt rules

## Explicit User Asks

- [open] Start implementation on the branching work and documentation initial steps.
- [open] Document the UI sprint work structure using the newer phased style adapted from mainline.
- [open] Prepare the repo for a later clean merge to master followed by a fresh sprint integration branch and feature branches per slice.

## Presumption Loop

This Phase 0 slice starts guilty until proven innocent. The likely failure mode is not missing prose; it is that the repo stays bound to the old workflow contract and the new sprint starts from chat memory instead of durable repo surfaces.

Hostile review before implementation assumes the correct owner is the local workflow/protocol/doc surface, not the UI renderer or the older FITS/runtime-walk contracts. Hostile review of the plan itself must happen before broad edits so the Phase 0 scope stays on branching and documentation initialization only.

Forward TDD for this slice means doc/protocol regressions where practical: if a workflow surface should advertise a required section or ritual, lock it with a focused workflow-doc test before trusting the prose change. After the first green, hostile-review the repaired documentation stack and the resulting sprint stop point again before treating Phase 0 as ready.

Any change in owner, topic, or proof surface resets this loop.

## Presumption Evidence

- Owner Proof: bootstrap showed the repo was clean on `feature/explaino-joy`, still bound to `docs/contracts/runtime_walk_fits.contract.json`, and had no dedicated Phase 0 sprint-foundation plan or contract.
- RED Witness: the new workflow-doc regression failed because the repo docs did not yet advertise `Explicit User Asks`, `Presumption Loop`, `Presumption Evidence`, and `Proof Ledger`, and the first plan-sync run exposed a real `viewer_host_assert_phased_plan_sync.py` import-path defect.
- Fix Proof: the repo now carries a dedicated Phase 0 plan/contract pair, the slice is locked to `ui_polish_sprint_phase0_foundation`, the workflow docs advertise the new section set, and the UI sprint overview plus three slice plans are checked in.
- Hostile Review Pass 1: the plan-sync wrapper needed a loader repair so the local adapter could import the mainline phased-plan hook and its sibling modules reliably.
- Hostile Review Pass 2: the Phase 0 resume point overstated the current repo state as already clean; a focused regression now locks the requirement that the handoff waits until after checkpointing.

## Proof Ledger

- Manual RED: the first Phase 0 audit found no valid sprint-foundation plan/contract surface and a stale active contract for this work thread.
- Checked-in regression RED: `py -3.14 -m pytest tests/test_agent_workflow_tools.py -q -k phase0_workflow_docs_require_mainline_style_plan_sections` failed before the workflow-doc update.
- Build/Rebuild Proof: not applicable for the plan/doc foundation slice.
- First GREEN: the focused workflow-doc regression now passes and the local plan-sync rail validates the new plan set.
- Post-green hostile finding: `viewer_host_assert_phased_plan_sync.py` needed a loader-path repair before the dirty-plan validation rail could run successfully in this repo.
- Residual friction: the git choreography remains a separate post-checkpoint step even though the branch topology is now fully documented.

## Notes

- Initial audit snapshot (before repair):
  - bootstrap reports `branch=feature/explaino-joy`, `head=38bf945`, and `status=clean`
  - the active locked contract is still `runtime_walk_fits`, which is the wrong owner for the new sprint-foundation work
  - there is no checked-in Phase 0 sprint-foundation plan or contract yet
- Phase 1 exit criteria:
  - a dedicated Phase 0 plan and contract exist and validate cleanly
  - the allowed mutation scope is bounded to workflow docs, planning docs, contracts, and focused workflow tests
  - the next action can switch from repo audit to the actual workflow/doc edits under the correct slice surface
- Phase 1 completion snapshot:
  - `docs/notes/ui_polish_sprint_phase0_foundation_PHASED_PLAN.md` and `docs/contracts/ui_polish_sprint_phase0_foundation.contract.json` now exist and validate cleanly
  - `py -3.14 tools/viewer_host_begin_work_slice.py --intent "ui polish sprint phase0 foundation" ...` locked the new contract and printed checkpoint token `ck:31092af8`
- Phase 2 exit criteria:
  - `AGENTS.md`, `AGENT_WORKING_PROTOCOL.md`, `.github/copilot-instructions.md`, and `docs/PHASED_PLAN_CONTINUITY_PROTOCOL.md` advertise the selected mainline-style phased-plan sections in repo-local language
  - the repo still avoids copying mainline-only workflow weight that does not fit the viewer-host scale
- Phase 2 completion snapshot:
  - the workflow docs now advertise `Explicit User Asks`, `Presumption Loop`, `Presumption Evidence`, and `Proof Ledger` as the preferred meaningful-plan section set
  - `tests/test_agent_workflow_tools.py::test_phase0_workflow_docs_require_mainline_style_plan_sections` now locks that guidance
- Phase 3 exit criteria:
  - the UI sprint has a durable overview plan plus the initial per-slice planning stubs needed for later feature execution
  - the plan set keeps the current focus on UI metadata/layout, render resolution/pacing, and color-authority groundwork without reopening the deferred programmable-color discussion
- Phase 3 completion snapshot:
  - the repo now carries `ui_polish_sprint_overview` plus three per-slice phased plans for schema domains, color authority, and render resolution/pacing
  - the overview plan records the intended merge-to-master then sprint-integration-branch topology without starting feature work early
- Phase 4 exit criteria:
  - focused workflow-doc validation passes
  - phased-plan sync passes
  - the plan records the exact merge/branch ready stop point for the later Phase 0 follow-on discussion
- Phase 4 completion snapshot:
  - hostile audit found and repaired one stale-stop-point defect in the Phase 0 resume text, then added `tests/test_agent_workflow_tools.py::test_ui_polish_phase0_plan_resume_point_waits_for_checkpoint`
  - `py -3.14 -m pytest tests/test_agent_workflow_tools.py -q --junitxml artifacts/pytest/test_agent_workflow_tools.junit.xml` passes with 21 green tests, covering the workflow-doc regressions, the phased-plan sync wrapper behavior, and the resume-point guard
  - `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passes across the Phase 0 plan and the four new UI sprint plans
  - the confirmed merge/branch-ready stop point is: merge `feature/explaino-joy` into `master`, create `feature/ui-polish-integration`, then peel off `feature/ui-polish-schema-domains`, `feature/ui-polish-color-authority`, and `feature/ui-polish-resolution-pacing` one at a time
  - post-checkpoint receipt proof exposed one more workflow defect: the proof layer could not derive evidence for `viewer_host_assert_phased_plan_sync.py` or generic `viewer_host_validate_slice_contract.py --out-json ...` commands, so a follow-up repair now writes a plan-sync validation artifact, teaches the proof layer both commands, and revalidates with 23 green workflow tests before the merge/branch choreography continues

## Validation

- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_polish_sprint_phase0_foundation.contract.json --out-json artifacts/validation/ui_polish_sprint_phase0_foundation_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 -m pytest tests/test_agent_workflow_tools.py -q --junitxml artifacts/pytest/test_agent_workflow_tools.junit.xml`

## Resume Point

After this slice is checkpointed from `feature/explaino-joy`, the next Phase 0 follow-on step is the git choreography: merge into `master`, create `feature/ui-polish-integration`, then start slice 1 from `feature/ui-polish-schema-domains`.