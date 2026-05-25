# SDF Roadmap Truth Sync After Phase Signal

## Current Phase

Phase 4 complete - top-level SDF roadmap/status docs are synced after capture replay authority and phase-signal metadata shipped.

## Phase Checklist

- [x] Phase 1 - create this checked-in plan/contract and open the docs truth slice
- [x] Phase 2 - update SDF status docs so capture/replay and phase-signal metadata are no longer listed as active/deferred work
- [x] Phase 3 - record that forward SDF feature work is paused behind the Color Pipeline fractal-switch preservation regression
- [x] Phase 4 - validate docs sync, hostile audit, diff hygiene, checkpoint, receipts, rearward review, push, and clean tree

## Explicit User Asks

- [done] Fix stale docs first before the Color Pipeline regression repair.
- [done] Do not return to forward SDF work until the Color Pipeline fractal-switch reset bug is addressed.
- [done] Preserve the distinction between shipped SDF substrate work and still-deferred authored SDF UI/SDF-native lanes.

## Scope

In scope:

- `spec_intake/_STATUS.md`
- `DEFERRED_THREADS.md`
- `docs/notes/sdf_field_pack_near_term_TODO.md`
- This plan and contract
- Handoff/receipt artifacts

Out of scope:

- Product code.
- Color Pipeline regression repair.
- Preset/composition UI design implementation.
- Authored SDF UI or SDF-native fractal lanes.

## Proof Ledger

- Start authority: branch `codex/color-pipeline-sdf-source-rows` at `66a631b` is clean and rearward review returned `status=ok`.
- Stale finding repaired: top-level SDF roadmap/status docs listed capture/replay authority as active and phase-signal metadata as deferred even though both shipped in `16e1df5` and `66a631b`.
- `spec_intake/_STATUS.md`, `DEFERRED_THREADS.md`, and `docs/notes/sdf_field_pack_near_term_TODO.md` now list capture/replay authority and `sdf_normal_angle` phase-signal metadata as shipped.
- Forward SDF feature work is now explicitly paused behind the Color Pipeline fractal-switch preservation regression and later Color Pipeline composition/preset UX review.
- Targeted stale grep passed: `artifacts/validation/sdf_roadmap_truth_sync_after_phase_signal_stale_grep.json`.
- Contract validation, phased-plan sync, hostile-audit validation, stale grep, and diff hygiene passed before checkpoint; receipt, rearward-review, push, and clean-tree proof are managed by the repo closure workflow after the checkpoint commit.

## Hostile Audit

- Status: complete
- Required posture: assume at least one top-level status surface still claims capture/replay or phase-signal metadata is future work until a targeted grep proves otherwise.

## Audit Passes

- [done] Pass 1 - grep SDF roadmap/status docs for stale capture/replay active wording.
- [done] Pass 2 - grep SDF roadmap/status docs for stale phase-signal deferred wording.
- [done] Pass 3 - confirm authored SDF UI and SDF-native lanes remain deferred.
- [done] Pass 4 - clean re-read the repaired state with the targeted stale grep and confirm no additional real issue found in the SDF roadmap/status surfaces.

## Audit Findings

- [done] Real stale-doc finding repaired: `spec_intake/_STATUS.md`, `DEFERRED_THREADS.md`, and `docs/notes/sdf_field_pack_near_term_TODO.md` still described capture/replay authority as active future work after the replay authority slice had shipped.
- [done] Real stale-doc finding repaired: the same docs still described `sdf_normal_angle` phase-signal metadata as deferred after the phase semantics slice had shipped.
- [done] Real continuity risk repaired: the current next SDF roadmap step now points to the Color Pipeline fractal-switch preservation regression before forward SDF feature work resumes.
- [done] Clean re-read confirmed the repaired state: authored SDF UI/live viewport integration, SDF-native selectable lanes, and broader Color Pipeline composition redesign remain deferred and were not implemented in this docs-only slice.

## Action Hostile Review

- Action ID: sdf-roadmap-truth-sync-after-phase-signal-closeout
- Suspected failure mode: a future session reads stale top-level SDF docs and reopens already-shipped capture/replay or phase metadata work instead of the real Color Pipeline regression.
- Correct owner/action: update only the roadmap/status surfaces that summarize current work state.
- Proof surface: targeted grep, contract validation, phased-plan sync, hostile-audit validation, diff hygiene, rearward review, and clean tree.
- Blocked action: product code, authored SDF UI/live viewport work, SDF-native lanes, or Color Pipeline composition redesign.
