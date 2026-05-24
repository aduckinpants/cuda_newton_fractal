# SDF Field Pack Near-Term Planning

## Current Phase

Complete - SDF field-pack near-term TODO documented, audited, and validated.

## Phase Checklist

- [x] Phase 1 - document the near-term SDF field-pack modernization TODO and slot it into the current backlog
- [x] Phase 2 - hostile-audit the documented sequencing against current Lens SDF, Generic Equation Pack, Color Pipeline, and Salticid SDF-pack surfaces
- [x] Phase 3 - validate docs-only contract, phased-plan sync, hostile audit, code-quality baseline, and diff hygiene
- [x] Phase 4 - checkpoint the docs-only planning slice and record a clean stop point

## Explicit User Asks

- [done] Document the SDF composition / field-pack idea as near-term TODO work in enough detail that future sessions can work from the plan.
- [done] Keep this as brainstorming/planning only, not implementation.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed before mutation.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported a clean worktree before mutation.
- Documentation target: `docs/notes/sdf_field_pack_near_term_TODO.md` now records the field interface, authored pack shape, slice order, validation defaults, non-goals, and first subtype guidance.
- Backlog slot: `DEFERRED_THREADS.md` now ties Lens SDF control truth to the SDF field substrate seed.
- Planning index: `spec_intake/_STATUS.md` now lists `SdfFieldPackNearTermPlanning`.
- Prior Lens SDF Phase 3 plan: `docs/notes/lens_sdf_report_then_modernization_PHASED_PLAN.md` now points at the new TODO as its decision-complete planning deliverable.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_field_pack_near_term_planning.contract.json --out-json artifacts/validation/sdf_field_pack_near_term_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile-audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/sdf_field_pack_near_term_planning_PHASED_PLAN.md --out-json artifacts/validation/sdf_field_pack_near_term_hostile_audit.json` passed after the audit-state wording repair.
- Code-quality baseline: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/sdf_field_pack_near_term_code_quality.json` passed with baseline check passed.
- Diff hygiene: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_pack_near_term_diff_check --log artifacts/logs/sdf_field_pack_near_term_diff_check.log --out-json artifacts/validation/sdf_field_pack_near_term_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.
- Checkpoint: represented by this docs-only planning commit and the paired `HANDOFF_LOG.md` entry.

## Hostile Audit

- Status: complete

## Audit Passes

- [done] Pass 1 - verify the plan does not confuse mask-derived Lens SDF with authored analytic SDF packs.
- [done] Pass 2 - verify the plan slots into existing backlog without reopening unrelated implementation.
- [done] Pass 3 - verify the plan is detailed enough for future implementation slices but does not claim shipped behavior.
- [done] Pass 4 - clean re-read after repairing the field-view wording found no additional real issue found in the docs-only boundary.

## Audit Findings

- [done] Finding 1: the first draft risk was treating authored SDF packs as if they belonged inside the Lens SDF aux-window path; the TODO now makes Lens SDF and authored packs separate producers that meet at a field interface.
- [done] Finding 2: the backlog slot could have hidden the existing `lens.downsample` control-truth bug under a bigger SDF dream; the TODO keeps Lens SDF truth cleanup as Slice 1.
- [done] Finding 3: the initial field-view sketch could have been read as a `std::span`/C++20 mandate; the TODO now says that storage/view type is illustrative and implementation must use the repo-compatible surface.
- [done] Clean re-audit: re-read the repaired state and no additional workflow mistake found; product code, tests, schema, renderer, runtime harness, Color Pipeline, and Salticid remain untouched.

## Action Hostile Review

- Action ID: sdf-field-pack-docs-detail
- Suspected failure mode: The docs slice could silently become implementation work or blur authored SDF packs with current mask-derived Lens SDF.
- Correct owner/action: Documentation-only planning updates under the new SDF field-pack planning contract.
- Proof surface: Contract validation, phased-plan sync, hostile-audit validation, code-quality baseline, and diff hygiene.
- Blocked action: Product code, tests, schema, renderer, runtime harness, Color Pipeline, or live viewer behavior changes.

## Notes

- This is a docs-only planning slice.
- The implementation TODO lives in `docs/notes/sdf_field_pack_near_term_TODO.md`.
