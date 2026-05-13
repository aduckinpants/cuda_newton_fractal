# Anti-Lie Claim Enforcement Replacement

## Current Phase

Phase 4 complete: the anti-lie slice was rebuilt around machine-state closure rules only. The repaired hook path now blocks on dirty-state drift, missing validation receipts for session-advanced `HEAD`, stale or missing contract proof for session-advanced `HEAD`, and locked-contract mismatches. Bootstrap audit output no longer rewrites the authoritative code-quality artifact path. This slice remains workflow-only and does not resume feature work.

## Phase Checklist

- [x] Phase 1 - inventory every anti-lie-owned or anti-lie-modified surface and classify the real failure each one claims to prevent
- [x] Phase 2 - narrow the active hook path to machine-state blockers only and remove rhetoric-only blockers from `task_complete` and `Stop`
- [x] Phase 3 - split bootstrap audit output from authoritative receipt-evidence artifacts and add regressions for both the stale-authoritative-artifact case and the inherited-clean-head false-block case
- [x] Phase 4 - rewrite the active anti-lie contract, phased plan, and workflow guidance to describe the reduced truthful design; revalidate, refresh receipts, and leave the slice clean

## Explicit User Asks

- [done] Treat the entire previous anti-lie slice as untrusted and rebuild it from zero trust.
- [done] Remove blocker behavior that does not justify its operational cost.
- [done] Keep the system able to deny false clean, validated, receipted, or closed claims with current machine evidence.
- [done] Split bootstrap or orientation outputs away from authoritative receipt-evidence artifact paths.
- [done] Re-audit the anti-lie docs and newer workflow docs so they describe the repaired behavior instead of the previous doctrine.
- [done] Leave the replacement slice clean, receipted, and non-blocking without switching back to feature work.

## Zero-Trust Inventory

| Surface | Claimed purpose | Real failure covered | Failure mode / cost | Decision |
| --- | --- | --- | --- | --- |
| `evaluate_validation_receipt_guard(...)` | deny false "validated" closure after session-local commits | real and still needed | low false-block risk when tied to session-advanced `HEAD` only | keep |
| `evaluate_contract_proof_receipt_guard(...)` | deny false "receipted / closed" claims after session-local commits | real and still needed | previous implementation false-blocked fresh clean sessions when the active contract changed on an inherited `HEAD` | replace with narrower session-advanced-only rule |
| `validate_validation_receipt_evidence_freshness(...)` | catch stale authoritative artifacts after receipt writing | real and still needed | correct, but only when checking authoritative receipt evidence | keep |
| bootstrap audit write to `artifacts/code_quality_report.json` | quick orientation audit | real bootstrap need, wrong artifact path | rewrote authoritative closure evidence and triggered false Stop-hook stale-proof failures | replace with non-authoritative bootstrap artifact path |
| restricted status-vocabulary blocker | police words like `done` and `green` | partially real, but not a closure-state guard | high false-block risk, prose policing, duplicates actual receipt/dirty-state guards poorly | remove from active blockers |
| required completion capstone text | force a final corrective-churn slogan | no machine-state failure prevented | pure rhetoric gate; false-blocks honest completion text | remove |
| hostile-audit validator | prove the repaired state was re-audited before closing this workflow slice | useful as slice-local evidence | valuable for this repair slice, but not a general Stop/task_complete blocker | keep as validation surface, remove from active blockers |
| explicit-user-asks blocker inside `task_complete` / `Stop` | stop closure while plan asks remain open | partially useful plan hygiene | false-blocked clean inherited states and mixed plan discipline with closure proof | remove from active blockers |
| `salt_ndepend` packet gate inside `task_complete` / `Stop` | enforce a separate packet-readiness policy | real in its own domain | unrelated to this repo-wide anti-lie closure repair; over-broad in generic closure hooks | remove from active blockers |
| truth report / forensic timeline / claim ledger tools | offline truth and forensic reporting | can still be useful for diagnostics | not needed as closure authority for this slice | demote to optional diagnostics |

## Replacement Design

### Active hook blockers kept

- Dirty repo state that differs from the session baseline.
- Missing validation receipt when the session advanced `HEAD`.
- Missing, stale, or contract-mismatched contract proof when the session advanced `HEAD`.
- Locked-contract drift or missing required assertion results for the current session-advanced `HEAD`.

### Active hook blockers removed

- Restricted status-vocabulary policing.
- Required anti-lie capstone text.
- `task_complete` and `Stop` blocks driven by open explicit asks.
- `task_complete` and `Stop` blocks driven by hostile-audit state.
- `task_complete` and `Stop` blocks driven by `salt_ndepend` packet-readiness state.

### Receipt / artifact policy

- `artifacts/code_quality_report.json` remains the authoritative closure-evidence artifact for the real code-quality validation command.
- Session bootstrap audit now writes to `artifacts/bootstrap/code_quality_report.json`.
- Authoritative receipt-evidence artifacts are expected to stay stable between receipt write and later closure recheck unless explicit revalidation runs.

## Optional Diagnostics

- `tools/viewer_host_truth_report.py` survives as an optional offline truth surface.
- `tools/viewer_host_forensic_timeline.py` survives as an optional offline forensic surface.
- `tools/viewer_host_claim_ledger.py` survives as an optional offline claim-record surface.
- None of those tools are required for `task_complete` or `Stop` in the repaired anti-lie design.

## Hostile Audit

- Status: done
- Required posture: assume the previous anti-lie implementation was overfit to anger-era workflow failures and likely mixed useful closure proof with rhetoric, duplicate gates, and shared-path artifact hazards.

## Audit Passes

- [done] Pass 1 - re-read the active anti-lie hook path and found real defects: fresh clean sessions on inherited receipted `HEAD`s could still false-block on contract-proof checks, and session bootstrap rewrote the authoritative code-quality artifact used later for closure freshness.
- [done] Pass 2 - removed the rhetoric-only blockers from `task_complete` and `Stop`, narrowed contract-proof enforcement to session-advanced `HEAD`s only, and moved bootstrap audit output onto a non-authoritative path.
- [done] Pass 3 - re-read the repaired state with focused regressions and confirmed the repaired slice still blocks authoritative-artifact drift while no longer false-blocking inherited clean heads.

## Audit Findings

- [done] Real defect found and repaired: `evaluate_contract_proof_receipt_guard(...)` treated a clean inherited validation-receipted `HEAD` as closure debt for the current session. The repaired guard now returns early unless the session actually advanced `HEAD`.
- [done] Real defect found and repaired: `tools/viewer_host_session_bootstrap.py --audit` wrote to `artifacts/code_quality_report.json`, which is also the authoritative artifact used for closure freshness checks. The repaired bootstrap audit now writes to `artifacts/bootstrap/code_quality_report.json`.
- [done] Real defect found and repaired: the active `task_complete` / `Stop` path blocked on restricted status vocabulary, mandatory capstone text, hostile-audit state, open explicit asks, and `salt_ndepend` gate state. Those blockers were removed from the active closure path because they were not machine-state closure proof.
- [done] No additional real defect found in the repaired state after the focused re-audit.

## Proof Ledger

- The repaired hook path lives in `tools/viewer_host_checkpoint_guard.py`.
- Bootstrap audit path separation lives in `tools/viewer_host_session_bootstrap.py`.
- Focused regressions live in `tests/test_viewer_host_checkpoint_guard.py` and `tests/test_agent_workflow_tools.py`.
- The replacement contract lives in `docs/contracts/anti_lie_claim_enforcement.contract.json`.
- Workflow guidance was trimmed in `AGENTS.md`, `AGENT_WORKING_PROTOCOL.md`, and `.github/copilot-instructions.md` so they no longer prescribe anti-lie-only plan theater as if it were general proof.
- Offline diagnostics survive, but they are no longer closure authority for this slice.

## Stop Point

- The anti-lie replacement slice ends only after the repo is clean, the focused workflow validations are green, the replacement contract is re-locked and receipted, and direct hook probes confirm that clean inherited heads are no longer false-blocked.
- Do not resume feature work from this plan. Pick the next feature slice explicitly in a fresh prompt.
