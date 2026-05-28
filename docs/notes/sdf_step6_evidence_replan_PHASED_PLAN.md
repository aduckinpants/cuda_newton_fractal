# SDF Step 6 Evidence Replan

## Current Phase

Closed - Step 5C is merged to `master`, stale roadmap truth is repaired, and the next SDF-native implementation slice is selected.

## Phase Checklist

- [done] Phase 1 - opened the docs-only Step 6 evidence/replan contract with `ck:e9d98681`.
- [done] Phase 2 - verified Step 5C is merged to `master` at `a528521` and pushed cleanly.
- [done] Phase 3 - inspected current SDF field, authored-pack, Color Pipeline, overlay, and dropdown seams.
- [done] Phase 4 - repaired stale roadmap/status text after authored-pack live consumption shipped.
- [done] Phase 5 - chose the next bounded implementation slice and recorded alternatives/dependencies.
- [done] Phase 6 - hostile audit, validation, checkpoint, receipts, rearward review, push, and clean tree.

## Explicit User Asks

- [done] Go ahead after user testing found Step 5C looking good.
- [done] Merge the Step 5C authored SDF pack Color Pipeline/overlay work to `master`.
- [done] Respect the Step 6 boundary instead of starting an SDF-native lane without a fresh evidence review.
- [done] Keep the next move modular, simple, extensible, and not a renderer monolith.

## Scope

In scope:

- Merge/push truth recording for Step 5C.
- Roadmap/status truth sync after authored SDF pack live field consumption shipped.
- Evidence review of what is now feasible for the first SDF-native product slice.
- A recommended next implementation branch with proof gates.

Out of scope:

- Product code changes.
- New `FractalType`.
- Renderer, Color Pipeline, runtime harness, schema, or test mutation.
- SDF-native lane implementation.
- Broad function-library or Salticid adapter work.

## Evidence Review

Current shipped substrate:

- Mask-derived Lens SDF can produce shared SDF fields.
- Authored SDF packs have parser, CPU reference, CUDA evaluator, hardening, field producer bridge, viewer UI controls, state persistence, and live Color Pipeline/overlay consumption.
- Color Pipeline SDF Source rows can consume SDF fields, including signed distance, inside/outside, boundary band, normal angle, curvature, and Lens Field v2-style response paths.
- Row-local SDF field downsample and multi-field grouping are shipped.
- Capture/replay authority now preserves authored SDF pack field-source state.
- `FractalType` still has no SDF-native lane; current SDF ids in `fractal_types.h` are Color Pipeline signal ids, not selectable fractal families.
- Current authored SDF pack ops include circle, box, capsule, boolean ops, smooth union, translate, rotate, scale, and repeat. Recursive folds, apollonian inversion, and richer domain operators are not yet shipped.

Candidate next implementation slices:

1. `sdf_smooth_lattice_2d` built-in SDF pack lane.
   - Best difficulty-to-reward ratio.
   - Uses already-shipped `repeat`, circle/capsule/box, transform, and smooth-union support.
   - Can be productized as a built-in authored pack surfaced through the normal dropdown/left panel without adding a bespoke evaluator.
   - Good controls: period, radius, blend, rotation, offset, cell skew or anisotropy where supported, field downsample, palette/source presets.
2. Generic `sdf_pack_scene` dropdown lane.
   - Most extensible.
   - Risk: UX may feel abstract unless it ships with a curated built-in pack library and a clear selected-pack authority.
   - Better as the lane shell that can host `sdf_smooth_lattice_2d` first, not as an empty generic lane.
3. `sdf_recursive_fold_2d`.
   - Visually promising, but likely needs new fold/domain ops and extra CUDA parity work before product UI.
   - Good follow-up after the lane shell proves field-primary rendering.
4. `sdf_apollonian_field_2d`.
   - High visual payoff, but needs iterative inversion/domain logic not present in the current pack op set.
   - Defer until the lane shell and at least one simpler built-in pack lane are green.

Recommended next slice:

- `codex/sdf-pack-scene-lane-shell`
- Goal: add one normal dropdown lane shell backed by the authored SDF pack field producer, with `sdf_smooth_lattice_2d` as the first curated built-in/default pack.
- Keep future packs data-driven through pack metadata; do not add one enum/kernel per SDF idea.
- Prove field-primary rendering, normal Color Pipeline path, state/capture/replay, no-mouse control edits, and measured FPS for the built-in pack.

## Proof Gates For Recommended Next Slice

- Native:
  - built-in pack catalog validation;
  - field-primary render planner tests;
  - state/default/load tests for selected built-in pack and pack params;
  - CPU/CUDA field parity for the selected built-in pack.
- Runtime/no-mouse:
  - dropdown selection keeps the SDF pack scene lane selected;
  - built-in pack controls are visible and change frame hashes;
  - Color Pipeline source/palette/grading still drives the lane;
  - capture/finding/replay reproduces the same pixels;
  - reports identify `sdf_pack_scene`/built-in pack id and backend used.
- Performance:
  - publish a bounded before/after or baseline witness for full-quality and interactive preview at 1024 and 2048 long-edge equivalents;
  - do not claim all authored packs are realtime.

## Proof Ledger

- Step 5C merge: `master` fast-forwarded from `dcd0ac6` to `a528521` and pushed to `origin/master`.
- Current branch for this docs-only replan: `codex/sdf-step6-evidence-replan`.
- Current SDF field evidence: authored packs now have parser, CPU reference, CUDA evaluator, field producer, viewer UI controls, persistence, Color Pipeline source-row consumption, overlay reporting, and capture/replay authority.
- Current missing product seam: no normal dropdown SDF-native lane exists yet; SDF ids in `fractal_types.h` are Color Pipeline source ids rather than fractal family ids.
- Pack op evidence: current authored SDF pack ops cover circle, box, capsule, boolean ops, smooth union, translate, rotate, scale, and repeat; recursive folds and apollonian inversion are not yet shipped.
- Recommendation: implement `sdf_pack_scene` lane shell with `sdf_smooth_lattice_2d` built-in pack first; defer recursive fold and apollonian variants until the lane shell proves field-primary rendering and the op set is expanded deliberately.

## Hostile Audit

- Status: done

Required questions:

- Did this slice only plan/reconcile, or did it accidentally mutate product code?
- Did Step 5C actually land on `master` and push cleanly before Step 6 planning?
- Did roadmap text stop claiming authored SDF pack live integration is still deferred?
- Did the recommendation avoid a monolith and reuse the existing pack field producer?
- Did the recommendation avoid unsupported recursive/apollonian math until the current pack op set can support it?
- Did the next implementation proof gates cover visible dropdown behavior, controls, Color Pipeline, capture/replay, and FPS?

## Audit Passes

- [done] Pass 1 - merge audit verified Step 5C is on `master`, pushed to origin, and rearward review is `ok`.
- [done] Pass 2 - stale-roadmap audit found current docs still said authored SDF pack UI/live viewport integration was deferred after it shipped.
- [done] Pass 3 - clean re-read after repairs confirmed this remains docs-only and the next recommendation reuses the authored-pack field producer instead of creating a renderer monolith.

## Audit Findings

- [done] Finding 1: `DEFERRED_THREADS.md`, `spec_intake/_STATUS.md`, and `docs/notes/sdf_field_pack_near_term_TODO.md` still described authored SDF pack UI/live integration as deferred even though Step 5B/5C shipped viewer controls plus Color Pipeline/overlay consumption. Repaired those roadmap/status surfaces to distinguish shipped authored-pack field-source integration from still-deferred full pack catalog/authoring UX and SDF-native lanes.
- [done] Finding 2: an initial contract used unsupported `workflow_type: docs_only`; `viewer_host_begin_work_slice.py` rejected it before slice lock. Repaired the contract to use the repo-supported `workflow_only` type, then reopened the slice with `ck:e9d98681`.
- [done] Clean re-read confirmed the repaired state: no product code changed, Step 5C is merged/pushed, authored-pack live integration is no longer described as deferred, and the next slice is bounded to a pack-backed SDF lane shell with `sdf_smooth_lattice_2d` as the first built-in pack.
