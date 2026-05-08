# Workflow Guard Hostile Review Enforcement

## Current Phase

Complete - hostile-review enforcement is validated, documented, and ready to checkpoint

## Phase Checklist

- [x] Phase 1 - add focused red tests for hostile-review debt across checkpoint commit, proof receipts, carryover prompts, and final closure
- [x] Phase 2 - add a machine-readable hostile-audit validator derived from the checked-in phased plan and register it as contract-proof evidence
- [x] Phase 3 - hard-block checkpoint commit, task_complete, Stop, and carryover prompts when hostile-review proof is missing
- [x] Phase 4 - rerun targeted workflow validations, update repo workflow docs to match the enforced behavior, and checkpoint the slice cleanly

## Explicit User Asks

- [done] Make it impossible to ignore the mandatory multi-layer hostile review.
- [done] Reuse useful workflow patterns from the mainline `salticid-cuda` repo instead of guessing.
- [done] Make it impossible to continue the same false-closure behavior through commit, prompt carryover, receipts, and final closure.
- [done] Start implementation now instead of talking about it.

## Presumption Loop

The local hypothesis is that the current repo already mirrors the mainline carryover, receipt, and phased-plan rails closely enough that the true missing owner seam is machine-readable hostile-review proof. Right now `viewer_host_checkpoint_slice.py` can still checkpoint a slice without any hostile-audit validation, `viewer_host_write_contract_proof_receipt.py` has no hostile-audit evidence requirement, and the guards only block dirty state, missing receipts, and open explicit asks. The cheapest disconfirming checks are focused workflow tests in the existing checkpoint-guard, contract-proof, and workflow-tool suites.

## Presumption Evidence

- Mainline `salticid-cuda` already provides the carryover prompt, receipt, phased-plan sync, and proof-evidence patterns the current repo should reuse.
- `tools/viewer_host_checkpoint_slice.py` currently validates only the locked contract before commit; it does not run any hostile-audit validator.
- `tools/viewer_host_contract_proof.py` already has a parseable validator/JUnit evidence registration surface that can absorb a hostile-audit validator artifact.
- `tools/viewer_host_assert_phased_plan_sync.py` already delegates to the mainline phased-plan hook, so the hostile-review continuity should stay plan-derived instead of moving into chat-only state.

## Proof Ledger

- Green: `py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py tests/test_viewer_host_contract_proof.py tests/test_agent_workflow_tools.py -q`
- Green: phased-plan sync now rejects meaningful plans that carry `## Explicit User Asks` without `## Hostile Audit`, `## Audit Passes`, and `## Audit Findings`.
- Green: checkpoint wrapper now refuses both `commit` and `write-receipts` when the active plan's hostile audit is still pending.

## Hostile Audit

- Status: done
- Required posture: reuse mainline carryover/receipt/proof-ledger rails first, then add only the missing hostile-audit proof layer.

## Audit Passes

- [done] Pass 1 - inspected the current workflow seams, added focused RED tests for task_complete, Stop, hostile-audit validator evidence recognition, and checkpoint commit denial, then drove them green.
- [done] Pass 2 - hostile review of the first implementation found a real proof-chain gap: `viewer_host_checkpoint_slice.py write-receipts` still bypassed pending hostile audit. Added the regression first, blocked receipt writing on the same validator, and reran the full workflow suites.
- [done] Pass 3 - audited the phase-start and closure docs, then aligned `viewer_host_assert_phased_plan_sync.py`, `AGENTS.md`, and `AGENT_WORKING_PROTOCOL.md` so meaningful plans now carry explicit hostile-audit sections and the written protocol matches the enforced behavior.

## Audit Findings

- [done] Real defect found and repaired: `viewer_host_checkpoint_slice.py` initially blocked `commit` but still allowed `write-receipts` while hostile audit was pending, which weakened the machine-proof layer. Added a focused regression and blocked both wrapper modes on the hostile-audit validator.
- [done] No additional real defect found in the second repaired audit pass across the touched workflow suites after rerunning the full targeted validations.

## Notes

- Expected owner files for this slice:
  - `docs/contracts/workflow_guard_hostile_review_enforcement.contract.json`
  - `docs/notes/workflow_guard_hostile_review_enforcement_PHASED_PLAN.md`
  - `tools/viewer_host_validate_hostile_audit.py`
  - `tools/viewer_host_checkpoint_guard.py`
  - `tools/viewer_host_checkpoint_dirty_prompt_guard.py`
  - `tools/viewer_host_checkpoint_slice.py`
  - `tools/viewer_host_contract_proof.py`
  - `tools/viewer_host_write_contract_proof_receipt.py`
  - `tests/test_viewer_host_checkpoint_guard.py`
  - `tests/test_viewer_host_contract_proof.py`
  - `tests/test_agent_workflow_tools.py`
- Non-goals for this slice:
  - do not fix the separate paced-loop runtime bug here
  - do not replace the existing validation-receipt chain
  - do not invent a second continuity source outside the checked-in phased plan

## Resume Point

Run the contract-required validators, append the closing `ck:11b625a3` handoff entry, checkpoint the slice, write receipts for the committed clean HEAD, and finish on a zero-diff repo.