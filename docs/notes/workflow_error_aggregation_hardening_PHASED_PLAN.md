# Workflow Error Aggregation Hardening

## Current Phase

Complete - workflow receipt/proof error aggregation is implemented, validated, hostile-audited, and closed on this branch.

## Phase Checklist

- [x] Phase 1 - create and lock this workflow-hardening plan/contract
- [x] Phase 2 - add RED coverage that `write-receipts` reports all predictable proof/receipt errors before writing partial receipts
- [x] Phase 3 - implement aggregated preflight diagnostics for receipt/proof closure
- [x] Phase 4 - validate focused workflow rails and workflow contract/plan surfaces
- [x] Phase 5 - hostile audit, checkpoint, receipts, rearward review, push, and merge-back if green

## Explicit User Asks

- [x] Harden the agent protocol workflow before moving on.
- [x] Stop forcing multi-trip error discovery when a workflow tool can report the full set of closure errors in one run.
- [x] Do not waste time with partial receipt/proof writes that are known to fail later in the same wrapper.
- [x] Keep this as tooling/protocol hardening, not product feature work.

## Scope

In scope:

- `viewer_host_checkpoint_slice.py write-receipts` preflight behavior.
- `viewer_host_write_contract_proof_receipt.py` direct proof-error aggregation.
- Focused tests proving missing required validation commands and missing/unknown evidence are reported together before partial receipt writes.
- Workflow documentation/plan truth for this bounded hardening.

Out of scope:

- Product viewer, renderer, Color Pipeline, SDF, fractal catalog, or UI feature work.
- Replacing the overall checkpoint guard architecture.
- Relaxing receipt, contract-proof, hostile-audit, rearward-review, or viewer-first proof requirements.
- Physical mouse automation.

## Authority Decision

The controlling defect is workflow friction from staged failure discovery. The wrapper should not write a validation receipt and only then discover that contract-proof closure was impossible. It should preflight the required command/evidence set, report all predictable receipt/proof blockers in one stderr block, and leave no partial receipt side effect when it already knows closure cannot succeed.

## Proof Ledger

- Bootstrap authority: `master` at `3bda37d`, clean, pushed, rearward review `ok`.
- Branch authority: `codex/workflow-error-aggregation-hardening`.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "workflow error aggregation hardening" --profile checkpoint --plan docs/notes/workflow_error_aggregation_hardening_PHASED_PLAN.md --contract docs/contracts/workflow_error_aggregation_hardening.contract.json` succeeded with checkpoint token `ck:ec78e2c1`.
- Current observed failure class: `viewer_host_checkpoint_slice.py write-receipts` could write a validation receipt, then fail in `viewer_host_write_contract_proof_receipt.py` for missing required commands, forcing another trip.
- RED proof: `py -3.14 -m pytest tests/test_agent_workflow_tools.py::test_checkpoint_slice_write_receipts_preflights_contract_proof_errors_before_partial_receipt -q` failed because `write-receipts` proceeded into subprocess receipt writing instead of preflighting all known proof blockers.
- RED proof: `py -3.14 -m pytest tests/test_viewer_host_contract_proof.py::test_contract_proof_receipt_reports_missing_commands_and_evidence_together -q` failed because the direct contract-proof writer stopped at missing commands and did not also report missing parseable evidence.
- Implementation: `write-receipts` now preflights contract validation commands, parseable evidence, and expected artifacts before writing a validation receipt; direct contract-proof writing now aggregates missing commands, missing evidence, and assertion failures in one report.
- GREEN proof: `py -3.14 -m pytest tests/test_agent_workflow_tools.py -q --junitxml artifacts/pytest/test_agent_workflow_tools.junit.xml` passed (`43 passed`).
- GREEN proof: `py -3.14 -m pytest tests/test_viewer_host_contract_proof.py::test_contract_proof_receipt_reports_missing_commands_and_evidence_together -q --junitxml artifacts/pytest/test_viewer_host_contract_proof.junit.xml` passed (`1 passed`).
- Contract proof: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/workflow_error_aggregation_hardening.contract.json --out-json artifacts/validation/workflow_error_aggregation_hardening_contract.json` passed.
- Residual unrelated finding: full `tests/test_viewer_host_contract_proof.py` currently has a pre-existing stale fractal-count assertion (`44` expected, `46` current); this slice records it as out of scope and uses focused workflow node proof for the new aggregation behavior.

## Hostile Audit

- Status: complete
- Required posture: assume the first fix only patches one missing-command case, still writes partial receipts on another predictable failure, or hides a required contract-proof error unless focused tests prove otherwise.

## Audit Passes

- [x] Pass 1 - inspect RED/implementation for aggregated missing required command and evidence diagnostics.
- [x] Pass 2 - inspect side-effect behavior so known-failing receipt closure does not write a partial receipt first.
- [x] Pass 3 - clean re-read workflow docs, contract, tests, and tool output for no relaxation of proof requirements.

## Audit Findings

- [x] Finding - `write-receipts` could write a validation receipt before discovering contract-proof was impossible, forcing a multi-trip repair path.
- [x] Finding - the direct contract-proof writer stopped at the first proof-error category instead of returning missing command and missing evidence categories together.
- [x] Repair - both workflow paths now aggregate predictable closure blockers before writing proof artifacts.
- [x] Clean re-read - focused workflow tests, contract validation, plan sync, code-quality baseline, hostile-audit validation, and diff check confirmed the repaired state without relaxing proof requirements.

## Validation Targets

- `py -3.14 -m pytest tests/test_agent_workflow_tools.py -q --junitxml artifacts/pytest/test_agent_workflow_tools.junit.xml`
- `py -3.14 -m pytest tests/test_viewer_host_contract_proof.py::test_contract_proof_receipt_reports_missing_commands_and_evidence_together -q --junitxml artifacts/pytest/test_viewer_host_contract_proof.junit.xml`
- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/workflow_error_aggregation_hardening.contract.json --out-json artifacts/validation/workflow_error_aggregation_hardening_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/workflow_error_aggregation_hardening_PHASED_PLAN.md --out-json artifacts/validation/workflow_error_aggregation_hardening_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/workflow_error_aggregation_hardening_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label workflow_error_aggregation_hardening_diff_check --log artifacts/logs/workflow_error_aggregation_hardening_diff_check.log --out-json artifacts/validation/workflow_error_aggregation_hardening_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`
