# Anti-Lie Claim Enforcement Replacement

## Current Phase

Phase 4 complete: the anti-lie slice was rebuilt around machine-state receipt proof plus the repo-wide closure gates that already remain authoritative elsewhere. The repaired hook path now blocks on dirty-state drift, missing validation receipts for inherited clean or session-advanced `HEAD`s, stale or missing contract proof for inherited clean or session-advanced `HEAD`s, locked-contract mismatches, open explicit user asks, pending hostile audit, and required `salt_ndepend` packet gates. Bootstrap audit output no longer rewrites the authoritative code-quality artifact path. This slice remains workflow-only and does not resume feature work.

## Phase Checklist

- [x] Phase 1 - inventory every anti-lie-owned or anti-lie-modified surface and classify the real failure each one claims to prevent
- [x] Phase 2 - remove rhetoric-only blockers from `task_complete` and `Stop` while preserving the repo-wide closure gates that remain authoritative outside this slice
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
| `evaluate_contract_proof_receipt_guard(...)` | deny false "receipted / closed" claims after session-local commits | real and still needed | previous implementation both false-blocked some inherited clean states and fail-opened inherited clean states with no proof after a fresh session | replace with a split rule: inherited clean heads still require receipts, session-advanced heads keep the session-local enforcement path |
| `validate_validation_receipt_evidence_freshness(...)` | catch stale authoritative artifacts after receipt writing | real and still needed | correct, but only when checking authoritative receipt evidence | keep |
| bootstrap audit write to `artifacts/code_quality_report.json` | quick orientation audit | real bootstrap need, wrong artifact path | rewrote authoritative closure evidence and triggered false Stop-hook stale-proof failures | replace with non-authoritative bootstrap artifact path |
| restricted status-vocabulary blocker | police words like `done` and `green` | partially real, but not a closure-state guard | high false-block risk, prose policing, duplicates actual receipt/dirty-state guards poorly | remove from active blockers |
| required completion capstone text | force a final corrective-churn slogan | no machine-state failure prevented | pure rhetoric gate; false-blocks honest completion text | remove |
| hostile-audit validator | prove the repaired state was re-audited before closing this workflow slice | real and still needed | broad anti-lie prose policing was not justified, but the repo already treats hostile review as an authoritative closure gate | keep as validation surface and keep the existing repo-wide closure gate |
| explicit-user-asks blocker inside `task_complete` / `Stop` | stop closure while plan asks remain open | real repo-wide workflow authority | removing it from the shared closure path silently fail-opened other slices that still depend on that gate | keep |
| `salt_ndepend` packet gate inside `task_complete` / `Stop` | enforce a separate packet-readiness policy | real in its own domain | removing it from the shared closure path silently fail-opened slices that still require the packet gate | keep |
| truth report / forensic timeline / claim ledger tools | offline truth and forensic reporting | can still be useful for diagnostics | not needed as closure authority for this slice | demote to optional diagnostics |

## Replacement Design

### Active hook blockers kept

- Dirty repo state that differs from the session baseline.
- Missing validation receipt when the session advanced `HEAD`.
- Missing validation receipt when a fresh session inherits a clean committed `HEAD`.
- Missing, stale, or contract-mismatched contract proof when the session advanced `HEAD`.
- Missing, stale, or contract-mismatched contract proof when a fresh session inherits a clean committed `HEAD`.
- Locked-contract drift or missing required assertion results for the current session-advanced `HEAD`.
- Open explicit user asks in the active phased plan.
- Pending hostile audit in the active phased plan.
- Required `salt_ndepend` packet gate failures for slices that declare that dependency.

### Active hook blockers removed

- Restricted status-vocabulary policing.
- Required anti-lie capstone text.

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

- [done] Pass 1 - re-read the active anti-lie hook path and found real defects: fresh clean sessions could inherit unreceipted closure debt, and session bootstrap rewrote the authoritative code-quality artifact used later for closure freshness.
- [done] Pass 2 - removed the rhetoric-only blockers, split bootstrap audit output onto a non-authoritative path, and reintroduced inherited-clean receipt checks plus the shared repo-wide closure gates that other slices still depend on.
- [done] Pass 3 - re-read the repaired state with focused regressions and confirmed the repaired slice blocks authoritative-artifact drift, inherited-clean receipt bypass, and repo-wide closure-gate bypass.

## Audit Findings

- [done] Real defect found and repaired: fresh clean sessions could inherit a committed `HEAD` with no validation receipt or contract-proof receipt. The repaired baseline/bootstrap path now refuses to materialize a fresh clean baseline until the inherited committed state has the required receipts, and the prompt / `task_complete` / `Stop` hooks now block inherited clean debt directly.
- [done] Real defect found and repaired: `tools/viewer_host_session_bootstrap.py --audit` wrote to `artifacts/code_quality_report.json`, which is also the authoritative artifact used for closure freshness checks. The repaired bootstrap audit now writes to `artifacts/bootstrap/code_quality_report.json`.
- [done] Real defect found and repaired: the anti-lie rebuild removed shared `task_complete` / `Stop` gates for open explicit asks, pending hostile audit, and required `salt_ndepend` packet state across the whole repo. Those repo-wide closure gates were restored, while rhetoric-only blockers such as restricted status vocabulary and mandatory capstone text remain removed.
- [done] No additional real defect found in the repaired state after the focused re-audit.

## Proof Ledger

- The repaired hook path lives in `tools/viewer_host_checkpoint_guard.py`.
- Bootstrap audit path separation lives in `tools/viewer_host_session_bootstrap.py`.
- Focused regressions live in `tests/test_viewer_host_checkpoint_guard.py` and `tests/test_agent_workflow_tools.py`.
- The replacement contract lives in `docs/contracts/anti_lie_claim_enforcement.contract.json`.
- Workflow guidance was trimmed in `AGENTS.md`, `AGENT_WORKING_PROTOCOL.md`, and `.github/copilot-instructions.md` so they no longer prescribe anti-lie-only rhetoric as proof, while still preserving the existing repo-wide closure gates.
- Offline diagnostics survive, but they are no longer closure authority for this slice.

## Stop Point

- The anti-lie replacement slice ends only after the repo is clean, the focused workflow validations are green, the replacement contract is re-locked and receipted, and direct hook probes confirm that clean inherited heads cannot bypass receipts and that shared closure gates still block when required.
- Do not resume feature work from this plan. Pick the next feature slice explicitly in a fresh prompt.
