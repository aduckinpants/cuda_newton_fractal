# SDF Pack Scene Follow-Up Replan

## Current Phase

Closed - post-lane replan selected the built-in SDF pack catalog seed as the next bounded implementation slice

## Phase Checklist

- [x] Phase 1 - open and lock this checked-in replan plan/contract from clean `master` at `8ea592d`.
- [x] Phase 2 - verify the closed `sdf_pack_scene` lane and deploy cleanup are merged to `master` and pushed.
- [x] Phase 3 - compare the next possible SDF pack-scene work by difficulty, reward, dependency risk, and test cost.
- [x] Phase 4 - select the next bounded implementation slice and record deferred alternatives.
- [x] Phase 5 - hostile audit, validation, checkpoint, receipts, rearward review, push, and clean tree.

## Explicit User Asks

- [done] Continue the next implementation after the disk-pressure interrupt by merging current work to `master` and selecting the next bounded slice.
- [done] Respect the `sdf_pack_scene` lane-shell mandatory replan point before adding new SDF composition families, new ops, recursive folds, apollonian fields, or broader pack-catalog UX.
- [done] Keep this modular, simple, extensible, and not a new monolith by choosing a curated built-in pack catalog seed that reuses the existing pack field producer and shipped ops.
- [done] Preserve the already-shipped `sdf_pack_scene`, Color Pipeline, SDF field, capture/replay, and deploy cleanup behavior; this slice changed planning/status surfaces only.

## Scope

In scope:

- Merge/reachability truth after `sdf_pack_scene` and deploy cleanup landed.
- Planning/status text that selects one next bounded implementation slice.
- Dependency ordering across small built-in pack catalog, new SDF ops, recursive/apollonian fields, orbit-trap modulation, and broader catalog/authoring UX.

Out of scope:

- Product code changes.
- New SDF pack ops.
- New SDF scene packs.
- New runtime UI controls.
- Renderer, Color Pipeline, SDF evaluator, schema, or test-harness mutation.


+## Proof Ledger
+
+- Merge truth: `master` fast-forwarded from `cf9754e` to `8ea592d`, which includes the closed `sdf_pack_scene` lane shell at `a57ccc6` plus the deploy disk-pressure cleanup, and was pushed to `origin/master`.
+- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `ok` for `8ea592d` before this replan branch opened.
+- Current product truth: `sdf_pack_scene` is now the first selectable SDF-native lane shell; it is not a future-only recommendation anymore.
+- Selected next implementation: `sdf_pack_builtin_catalog_seed`, a curated multi-pack catalog seed using only existing SDF pack ops and the shipped field producer.
+- Deferred boundary: new SDF ops, recursive/apollonian packs, orbit-trap modulation, and full authoring/catalog UX remain separate later slices.
## Evidence Review

Current shipped substrate on `master` after merge:

- `sdf_pack_scene` is a normal dropdown lane backed by the authored SDF pack field producer.
- `sdf_smooth_lattice_2d` is the first built-in/default pack.
- The lane exposes six normal left-panel controls and preserves no-mouse control proof.
- Existing Color Pipeline SDF rows color the lane through the shared SDF field path.
- State/capture/replay/report authority includes fractal type, pack id, controls, field dimensions/downsample, and backend used.
- The deploy cleanup now prevents diagnostics/findings bulk from being copied and pruned only generated timestamped runtime diagnostics.

Candidate next implementation slices:

1. Small built-in SDF pack catalog seed.
   - Difficulty: low/medium.
   - Reward: high immediate product value because the new dropdown lane stops feeling like a one-pack demo.
   - Dependency risk: low if it uses only shipped ops and adds a simple built-in pack selector plus curated pack JSON files.
   - Recommended first implementation target.
2. SDF pack op expansion: mirror, fold, polar repeat, inversion.
   - Difficulty: medium/high.
   - Reward: high, because it unlocks recursive/folded and apollonian-style scenes.
   - Dependency risk: requires CPU/CUDA/parser/parity work before product UX; should follow the catalog seed.
3. Recursive fold or apollonian built-in pack.
   - Difficulty: high.
   - Reward: very high visually.
   - Dependency risk: not honest until the required op expansion is green.
4. SDF-driven orbit traps or spatial parameter modulation.
   - Difficulty: high.
   - Reward: high strategic bridge between SDF and legacy fractal lanes.
   - Dependency risk: needs clearer field-to-fractal authority and should wait behind at least one more stable SDF scene catalog step.
5. Broader authored pack catalog/authoring UX.
   - Difficulty: high.
   - Reward: high strategic.
   - Dependency risk: premature until the built-in catalog path proves pack selection, presets, reports, and replay with multiple curated packs.

Selected next slice:

- Branch: `codex/sdf-pack-builtin-catalog-seed`
- Plan: `docs/notes/sdf_pack_builtin_catalog_seed_PHASED_PLAN.md`
- Contract: `docs/contracts/sdf_pack_builtin_catalog_seed.contract.json`
- Goal: add a small curated built-in pack catalog for the existing `sdf_pack_scene` lane using only currently shipped SDF pack ops.
- Product target: keep `sdf_smooth_lattice_2d` as default, add pack selection in the normal left-panel flow, and add two or three additional built-in packs that are visually distinct and control-sensitive without new parser/evaluator ops.

Proof gates for selected next slice:

- Native pack catalog validation for every built-in pack.
- Schema/binding tests proving pack selector visibility, selector identity, default pack retention, and control visibility changes when the selected pack changes.
- CPU/CUDA field producer sensitivity for each added pack.
- Diagnostics state/capture/replay round-trip for selected pack id and pack params.
- Published no-mouse runtime proof that selecting each built-in pack changes frame hashes and reports the active pack id/backend.
- Bounded performance witness for each pack at 1024 long-edge; 2048 witness for the default and one heavier pack if timing remains practical.

Deferred alternatives:

- New SDF ops: wait until the catalog seed proves multi-pack product/replay/report authority.
- Recursive/apollonian packs: wait until the required ops have CPU/CUDA parity and test coverage.
- SDF-driven orbit traps/spatial modulation: wait for a separate field-to-fractal authority plan.
- Full authored pack catalog/authoring UX: wait until curated built-ins prove the pack selector and reporting flow.

## Hostile Audit

- Status: complete

Required questions:

- Did this slice avoid product code changes?
- Did it actually honor the lane-shell mandatory replan point?
- Did it select a next slice that uses current shipped SDF pack ops instead of silently requiring unsupported math?
- Did it avoid broad authoring UX or renderer redesign?
- Did it leave enough proof gates to catch dead pack controls, missing state/replay authority, and FPS regressions?

## Audit Passes

- [done] Pass 1 - inspected current closed lane evidence and branch/merge state; `master` now contains `8ea592d` and is pushed cleanly.
- [done] Pass 2 - challenged the recommendation and selected the built-in pack catalog seed because it adds user-visible value without requiring unsupported SDF ops.
- [done] Pass 3 - clean re-read confirmed this slice changed only docs/status/plan surfaces and leaves product code untouched.

## Audit Findings

- [done] Real defect found and repaired: the initial replan bootstrap left truncated plan/contract files; iewer_host_begin_work_slice.py rejected them before the slice was locked, and the files were repaired before proceeding.
- [done] Clean re-read: roadmap/status text now treats sdf_pack_scene as shipped and points the next bounded implementation at the built-in pack catalog seed instead of reopening unsupported recursive/apollonian work.
