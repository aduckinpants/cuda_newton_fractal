# SDF Postprocess Roadmap Truth Sync

## Current Phase

Closed - roadmap truth sync is ready for Step 3A per-row/multi-field SDF downsample authority planning.

## Phase Checklist

- [x] Phase 1 - open and lock the docs-only roadmap truth-sync plan/contract.
- [x] Phase 2 - audit stale SDF roadmap/status wording after the postprocess merge.
- [x] Phase 3 - update only roadmap/status truth surfaces.
- [x] Phase 4 - hostile-review the repaired docs and validate contract, plan sync, stale grep, code quality, and diff hygiene.
- [x] Phase 5 - checkpoint receipts, rearward review, push, and clean tree.

## Explicit User Asks

- [done] Continue the SDF next-five campaign implementation after Step 1.
- [done] Execute Step 2: post-merge SDF roadmap truth sync.
- [done] Do not implement per-row downsample, phase UX, authored SDF pack integration, or SDF-native lanes in this slice.

## Scope

In scope:

- Update stale roadmap/status wording caused by the measured SDF postprocess optimization now being merged to `master`.
- Mark repeated median SDF witness reporting as shipped.
- Mark CUDA SDF postprocess scratch-buffer reuse as shipped.
- Record the measured postprocess median improvement range from the closed proof: `27%` to `62%`.
- Set per-row/multi-field SDF downsample authority as the next unresolved design/product step.

Out of scope:

- Product code changes.
- Runtime publish or no-mouse runtime proof.
- Per-row/multi-field SDF downsample implementation.
- Boundary-masked/phase-safe normal-angle UX.
- Authored SDF pack viewport integration.
- SDF-native fractal lanes.

## Target Files

- `spec_intake/_STATUS.md`
- `DEFERRED_THREADS.md`
- `KNOWN_ISSUES.md`
- `docs/notes/sdf_field_pack_near_term_TODO.md`
- active SDF performance roadmap plans under `docs/notes/` only where stale postprocess-review wording remains.

## Truth-Sync Rules

- Do not claim per-row or multi-field SDF downsample is shipped.
- Do not claim SDF-native lanes are shipped.
- Do not claim authored SDF pack viewport integration is shipped.
- Do not turn the conservative performance guidance into a broad "FPS solved" claim.
- Preserve the measured fact: postprocess medians improved by `27%` to `62%` across the closed identical SDF witness rows.
- Keep the next action explicit: per-row/multi-field SDF downsample authority is next because postprocess and field-generation hot paths were already measured and optimized enough to expose field-resolution/composition authority as the next design seam.

## Stale Phrase Audit

Audit for:

- `points back to postprocess`
- `postprocess review`
- `current witness`
- `current stage-split witness`
- `current representative witness`
- `deferred until the next measured design choice`

Expected repair:

- Phrases may remain only if they are historical descriptions of already-closed work or if the plan explicitly labels them as previous-state evidence.
- No active roadmap/status surface should tell a fresh session that postprocess review is still the next work item.

Post-repair expected remaining grep contexts:

- This plan's own Stale Phrase Audit list.
- `docs/notes/sdf_next_five_work_campaign_PHASED_PLAN.md` describing why Step 2 exists.
- Historical closed plan evidence may describe old witnesses as old witnesses, but the active status files must not point current work back to postprocess.

## Required Proof

- Contract validation passes.
- Plan sync passes using receipt-compatible logged evidence.
- Hostile-audit validation passes.
- Code-quality baseline passes.
- Diff hygiene passes.
- Stale-phrase grep log is produced and reviewed.
- Final rearward review reports `ok`.
- Final `git status --short --branch` shows a clean pushed branch.

## Stop Conditions

- Any docs imply Step 3 per-row/multi-field downsample is already shipped.
- Any docs imply SDF-native lanes or authored SDF viewport integration are already shipped.
- Any required stale text cannot be corrected without product-code changes.
- The slice drifts into runtime or UI behavior.

## Proof Ledger

- Start branch: `codex/sdf-postprocess-roadmap-truth-sync`.
- Start head: `a73dd6d`.
- Step 1 merge proof: `master` contains `7ed70d6` and is pushed/clean at `a73dd6d`.
- Bootstrap: passed before opening this slice.
- Repo status: clean before opening this slice.
- Rearward review: `ok` for `a73dd6d` before opening this slice.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "SDF postprocess roadmap truth sync" --profile native --plan docs/notes/sdf_postprocess_roadmap_truth_sync_PHASED_PLAN.md --contract docs/contracts/sdf_postprocess_roadmap_truth_sync.contract.json` appended `ck:bf0bdff7`.
- Stale audit found active postprocess-review wording in `spec_intake/_STATUS.md`, `KNOWN_ISSUES.md`, `DEFERRED_THREADS.md`, and `docs/notes/sdf_field_pack_near_term_TODO.md`.
- Roadmap repair: those active surfaces now mark repeated median SDF witness reporting and CUDA SDF postprocess scratch-buffer reuse as shipped, record the `27%` to `62%` measured postprocess median improvement range, and name per-row/multi-field SDF downsample authority as the next unresolved seam.
- Remaining stale-phrase grep contexts after repair are self-referential planning instructions or historical closed-plan evidence, not active roadmap status.
- Contract revision: stale-grep receipt command was repaired from a `cmd /c` quoting form that treated regex pipes as shell commands to a PowerShell single-quoted regex form, then re-locked with `viewer_host_revise_contract.py`.
- Contract validation: passed with `artifacts/validation/sdf_postprocess_roadmap_truth_sync_contract.json`.
- Plan sync: passed with receipt-compatible logged evidence at `artifacts/validation/sdf_postprocess_roadmap_truth_sync_plan_sync.json`.
- Stale-phrase audit: passed with receipt-compatible logged evidence at `artifacts/validation/sdf_postprocess_roadmap_truth_sync_stale_grep.json`; remaining hits are the closed next-five campaign's instructions for this Step 2, not active stale roadmap status.
- Hostile-audit validation: passed with `artifacts/validation/sdf_postprocess_roadmap_truth_sync_hostile_audit.json`.
- Code-quality baseline: passed with score `93/100` and no critical/error findings at `artifacts/validation/sdf_postprocess_roadmap_truth_sync_code_quality.json`.
- Diff hygiene: passed with `artifacts/validation/sdf_postprocess_roadmap_truth_sync_diff_check.json`.
- Checkpoint closeout: this plan is the final closeout surface for the docs-only truth-sync commit; machine receipts, rearward review, push, and clean-tree proof are required immediately after the checkpoint commit.
- Final status proof: required logged `git status --short --branch` rail after push.

## Hostile Audit

- Status: done
- Did this slice repair active stale roadmap text instead of editing random historical references? Yes. The active status files and near-term TODO were repaired; older closed-plan witness text was left historical unless it would mislead active status.
- Did this slice avoid claiming per-row/multi-field SDF downsample is already shipped? Yes. It is explicitly named as the next unresolved design/product seam.
- Did this slice avoid claiming the SDF FPS problem is globally solved? Yes. The docs state only the measured `27%` to `62%` postprocess median improvement from the closed witness and keep field-resolution/composition authority open.
- Did this slice avoid product-code changes? Yes. Changes are limited to docs, the slice contract, and handoff.
- Did this slice keep Step 3 as the next implementation step? Yes. Step 3 remains per-row/multi-field SDF downsample authority.

## Audit Passes

- [x] Pass 1 - stale-roadmap audit found active postprocess-review wording in `_STATUS.md`, `KNOWN_ISSUES.md`, `DEFERRED_THREADS.md`, and `sdf_field_pack_near_term_TODO.md`.
- [x] Pass 2 - overclaim audit repaired the active wording without claiming per-row/multi-field downsample, authored-pack UI, SDF-native lanes, or global FPS resolution are shipped.
- [x] Pass 3 - clean re-read found no additional real defect in active roadmap/status wording; remaining stale-phrase hits are this plan's audit list or the already-closed next-five campaign's Step 2 instructions.

## Audit Findings

- [done] Finding 1 - real stale-roadmap defect: active SDF status surfaces still said the current witness pointed back to postprocess review after the measured postprocess optimization had merged. Repaired by marking repeated median reporting and CUDA scratch-buffer reuse shipped and pointing the next seam to per-row/multi-field downsample authority.
- [done] Finding 2 - real freshness defect: `KNOWN_ISSUES.md` and `DEFERRED_THREADS.md` still had 2026-05-26 reconciliation text from an older branch. Repaired with 2026-05-28 truth-sync wording tied to this slice.
- [done] Finding 3 - real validation-command defect: the first stale-grep receipt command used `cmd /c` quoting that let regex pipe characters split into shell commands. Repaired the contract to run the grep through PowerShell with the regex in single quotes and re-locked the active contract.
- [done] Finding 4 - clean re-read confirmed no product-code files changed and no Step 3+ implementation was started.
