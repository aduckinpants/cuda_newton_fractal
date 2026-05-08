# Viewer-First Runtime Proof Guardrail Follow-Up

## Current Phase

Complete - runtime-proof guardrail follow-up validated and ready to hand off before the paused feature roadmap resumes

## Phase Checklist

- [x] Phase 1 - add a focused published-runtime Explaino basin smoke that exercises the new shared programmable basin path through `root_proximity` or `phase`
- [x] Phase 2 - harden the viewer-first closure workflow so receipts and final closure fail when a runtime-visible slice lacks both runtime publish and published-runtime proof
- [x] Phase 3 - run the bounded validation rails, audit the guardrail behavior, checkpoint the follow-up, and leave the repo ready for the paused real work

## Explicit User Asks

- [open] Add one more published-runtime smoke for basin `phase` or `root_proximity` now that the shared programmable basin path is real.
- [open] Keep on this longer before resuming feature work because repeated TDD/runtime-proof failures are not acceptable.
- [open] Update the repo rules/workflow so it becomes impossible to claim a runtime-visible fix is done without actual runtime proof.
- [open] Stop closing features or bug fixes on control-layer confidence when the user is being asked to test the real runtime behavior.

## Presumption Loop

The repeated failure mode is procedural, not just local implementation error: the repo rules say viewer-first work needs runtime proof, but the enforced closure path only checks for the presence of receipts, not whether a viewer-first receipt actually records runtime publish plus published-runtime proof. The cheapest disconfirming checks are local and deterministic: inspect `viewer_host_checkpoint_guard.py` and `write_validation_receipt(...)` to see whether they classify `viewer_first` commands semantically, and inspect the existing Explaino runtime smoke file to see whether basin `root_proximity` or `phase` is already covered. Those checks show the guard gap and the missing smoke.

## Presumption Evidence

- `tools/viewer_host_checkpoint_guard.py` currently blocks missing receipts and failed contract assertions, but it does not semantically reject a `viewer_first` validation receipt that records no runtime publish command or no published-runtime proof command.
- `tools/viewer_host_write_validation_receipt.py` currently accepts any command list once the repo is clean, which means a viewer-first slice can still record a formally valid but semantically weak receipt.
- `AGENTS.md`, `AGENT_WORKING_PROTOCOL.md`, and `.github/copilot-instructions.md` already state the viewer-first proof expectation, which means the missing piece is deterministic enforcement and sharper workflow language, not a brand-new policy concept.
- `tests/test_fractal_runtime_explaino_escape_variants.py` now covers Explaino programmable `smooth_escape` runtime proof, but it does not yet add a second published-runtime smoke for the basin-only `root_proximity` path.

## Proof Ledger

- Read-only finding: the previous slice closed correctly on repo hygiene, but the guardrails still allowed a viewer-first validation receipt to be structurally valid without proving runtime behavior semantically.
- Read-only finding: the highest-value follow-up is a deterministic viewer-first receipt guard plus one more published-runtime Explaino basin smoke that proves the new shared programmable basin path on `root_proximity`.
- Implementation: `tools/viewer_host_checkpoint_guard.py` now rejects `viewer_first` validation receipts that do not record both runtime publish and published-runtime proof, both when writing the receipt and again during closure evaluation.
- Implementation: `tests/test_viewer_host_checkpoint_guard.py` now covers the new viewer-first receipt rejection/acceptance paths plus closure denial for publish-only receipts.
- Implementation: `tests/test_fractal_runtime_explaino_escape_variants.py` now adds a published-runtime Explaino `root_proximity` smoke that proves `color_root_proximity_scale` changes the active runtime frame hash.
- Workflow update: `AGENTS.md`, `AGENT_WORKING_PROTOCOL.md`, and `.github/copilot-instructions.md` now state the same explicit viewer-first receipt requirement that the guard enforces.
- Validation: `py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q -k "viewer_first or write_validation_receipt"` passed after fixing the local test fixture setup.
- Validation: `py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q` passed after the hostile-audit classifier widening.
- Validation: `py -3.14 -m pytest tests/test_fractal_runtime_explaino_escape_variants.py -q -k root_proximity_programmable_color_pipeline` passed against the active published runtime.
- Validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/viewer_first_runtime_proof_guardrail_followup.contract.json --out-json artifacts/validation/viewer_first_runtime_proof_guardrail_followup_contract.json` passed.
- Validation: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py docs/notes/viewer_first_runtime_proof_guardrail_followup_PHASED_PLAN.md` passed after the completion-state update.
- Validation: `verify: runtime publish` passed and refreshed `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe` before the final `root_proximity` runtime smoke rerun.
- Validation: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json` passed at `97/100`.

## Notes

- Expected owner files for this pass:
  - `.github/copilot-instructions.md`
  - `AGENTS.md`
  - `AGENT_WORKING_PROTOCOL.md`
  - `docs/contracts/viewer_first_runtime_proof_guardrail_followup.contract.json`
  - `docs/notes/viewer_first_runtime_proof_guardrail_followup_PHASED_PLAN.md`
  - `tests/test_fractal_runtime_explaino_escape_variants.py`
  - `tests/test_viewer_host_checkpoint_guard.py`
  - `tools/viewer_host_checkpoint_guard.py`
- Non-goals for this pass:
  - do not resume unrelated advanced-color library work yet
  - do not redesign the entire validation system beyond the viewer-first receipt/closure seam needed to stop this specific failure pattern
  - do not widen runtime smoke coverage across unrelated fractal families in the same slice

## Resume Point

This follow-up is ready to checkpoint and hand off. After closure completes, return to the paused advanced-color roadmap instead of reopening this workflow hardening thread.