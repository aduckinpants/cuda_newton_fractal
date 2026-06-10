# Deferred Research Intake 2026-06-10

## Current Phase

Complete - deferred notes are written, indexed, validated, and checkpointed.

## Phase Checklist

- [x] Phase 1 - Capture the three separate deferred threads as distinct notes.
- [x] Phase 2 - Index the notes from `DEFERRED_THREADS.md` without promoting any thread to active work.
- [x] Phase 3 - Run docs-only validation, record proof, hostile audit, and checkpoint the documentation slice.

## Explicit User Asks

- [closed] Document the genetic-algorithm-over-slime-policy idea for much later follow-up.
- [closed] Document the slime stopping/switching-policy feedback for much later follow-up.
- [closed] Document the capture-only AA / color-tuning feasibility review for later follow-up.
- [closed] Document the SDF built-in pack packaging, UI recovery, and IFS/Flame + SDF hybrid substrate review for later follow-up.
- [closed] Keep these as separate lines of thought.
- [closed] Bring contradictions and show-stoppers to the surface.
- [closed] Do not start implementation or planning for active product work today.

## Proof Ledger

- Repo bootstrap: passed on `master` at `4dfed13`, clean tree.
- Rearward review: passed for `4dfed13`.
- Documentation RED: current repo had no dedicated deferred notes for these three lines of thought.
- Documentation GREEN: the three notes and `DEFERRED_THREADS.md` index entry are added.
- Contract validation: passed (`artifacts/validation/deferred_research_intake_20260610_contract.json`).
- Phased-plan sync: passed (`py -3.14 tools/viewer_host_assert_phased_plan_sync.py`).
- Code-quality baseline: passed (`artifacts/validation/deferred_research_intake_20260610_code_quality.json`).
- Hostile-audit validation: passed (`artifacts/validation/deferred_research_intake_20260610_hostile_audit.json`).
- Diff check: passed (`artifacts/validation/deferred_research_intake_20260610_diff_check.json`).

## Hostile Audit

- Status: complete

Questions:
- Did the documentation keep the three ideas separate instead of collapsing them into one vague future feature?
- Did the slime note distinguish sidecar policy GA from runtime-walk field slime policy?
- Did the capture AA note avoid overclaiming AA as a fix for palette/shape banding?
- Did the SDF note call out the published-runtime built-in-pack packaging risk as a short-term SDF blocker?
- Did this docs slice avoid runtime/product mutation?

## Audit Passes

- [closed] Pass 1 - reread the docs against the user asks and confirmed the three lines are separate notes, not a single broad future feature.
- [closed] Pass 2 - searched for active-work and overclaiming language; hits were deferred/not-active language or existing historical status text.
- [closed] Pass 3 - checked the highest-risk contradiction: the SDF note separates near-term packaging/recovery hardening from much-later IFS/Flame substrate work.
- [closed] Pass 4 - clean re-read of the repaired state found no additional real defect, overclaim, or workflow mistake.

## Audit Findings

- [closed] Finding 1 - The easiest future mistake would be collapsing "SDF packaging hardening" and "IFS/Flame hybrid substrate" into one SDF expansion lane. The SDF note now names the pack staging/recovery items as near-term hardening and treats IFS/Flame as separate non-SDF substrate campaigns.
- [closed] Finding 2 - The AA note could be misread as "AA fixes the capture." It now explicitly says AA will not fix Color Pipeline banding/repeat harshness and ranks per-family color tuning first for the cited image.
- [closed] Finding 3 - The slime note could blur sidecar parameter policy and runtime-walk field slime movement. It now splits those genome surfaces and recommends separate evaluators.

## Notes

This slice is documentation-only. The intended next product direction remains SDF work later, but no SDF implementation should start under this docs-intake contract.
