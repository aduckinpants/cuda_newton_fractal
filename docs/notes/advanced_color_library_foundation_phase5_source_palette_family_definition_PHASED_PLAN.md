# Advanced Color Library Foundation Phase 5 - Source/Palette First Family Definition

## Current Phase

Complete - the planning-authority slice defined the first truthful Source/Palette multi-row runtime family as the row-indexed root-basin pair family, and it locked the bounded runtime-owner/live-bridge rules the next executable slice must implement before Grading resumes

## Phase Checklist

- [x] Phase 1 - prove the current runtime math still only defines one Source generator and one Palette mapper, then choose the first family that can widen honestly from existing shipped tuples
- [x] Phase 2 - bind the first family to row-indexed root-basin pair recipes with explicit runtime-owner/live-bridge rules instead of inventing generic Source or Palette composition
- [x] Phase 3 - validate, hostile-audit, and checkpoint the family-definition slice cleanly

## Explicit User Asks

- [done] Phase 5 - keep moving forward on the defined advanced-color slices instead of drifting back to unrelated backlog work.
- [done] Phase 5 - call out the real workflow and product friction instead of silently smoothing it over.
- [done] Phase 5-7 - make the next step a real bounded plan with a named first family instead of a vague "multi-row later" placeholder.
- [done] Phase 5-7 - keep the reusable color pipeline generic and separately owned by deferring unsupported composition math instead of baking window-local exceptions into the product story.

## Presumption Loop

The controlling blocker is now semantic, not ownership extraction. `ui_app/src/color_pipeline_core.h` already owns the shipped Source/Palette row and lane helpers, but `ui_app/src/escape_time_coloring.h` still evaluates exactly one `params.color_pipeline.signal`, runs Shape as the only ordered scalar-transform stack, and then samples exactly one `params.color_pipeline.palette`. `ui_app/src/fractal_types.h` still mirrors that model with flat Source/Palette owner fields instead of per-row runtime stacks. Generic multi-row Source generator composition and generic Palette mapper chaining are therefore undefined in checked-in runtime math. The most falsifiable local hypothesis is that the first truthful family must reuse an already shipped coupled tuple instead of inventing new composition semantics. The nearest existing family is the root-basin tuple lineage: `root_index + root_classic_palette` and `root_index + joy_root_palette` already import/apply as supported companion selections, and the root-palette Shape repair proved those tuples now run through the shared programmable basin/Shape path. If Phase 5 narrows the first family to row-indexed root-basin pair recipes with last-enabled-valid-pair selection, the next executable slice can widen runtime owners and the live bridge honestly while explicitly deferring generic intra-lane Source/Palette composition. The cheapest current disconfirming checks are the deterministic planning rails plus the existing root-pair proofs: if this named family required new Source or Palette math beyond those shipped tuples, the slice would be overclaiming.

## Presumption Evidence

- `ui_app/src/escape_time_coloring.h` still resolves one Source signal and one Palette mapper, while Shape owns the only ordered composition seam.
- `ui_app/src/fractal_types.h` still stores flat Source/Palette owner fields, but it already carries the per-shipped owner fields needed by `root_index`, `root_classic_palette`, and `joy_root_palette`.
- `docs/notes/advanced_color_pipeline_slice7_catalog_runtime_binding_PHASED_PLAN.md` already records the simple Factorio train-scheduler-style schedule metaphor for Source / Shape / Palette lane stacks, so the current deferment is about generic runtime composition math, not about forgetting or retracting the mapped UX direction.
- `docs/notes/advanced_color_root_palette_tuple_switch_followup_PHASED_PLAN.md` proves the editor already auto-completes `root_index + root_classic_palette` as a supported companion pair instead of a fake draft-only dead end.
- `docs/notes/advanced_color_root_palette_shape_interactivity_PHASED_PLAN.md` proves `root_index + root_classic_palette` and `root_index + joy_root_palette` now render through the shared programmable basin/Shape path, so they are the nearest runtime-real coupled Source/Palette family.
- `root_proximity` shares the root-sample substrate, but the checked-in root palettes are still explicit root-index palette lineages, so `root_proximity` remains out of the first paired family until a later slice defines how non-index root signals should feed those palettes.

## Proof Ledger

- Landed: `docs/notes/advanced_color_library_foundation_phase5_source_palette_family_definition_PHASED_PLAN.md` and `docs/contracts/advanced_color_library_foundation_phase5_source_palette_family_definition.contract.json` now bind the first-family planning slice under repo workflow authority.
- Landed: the first truthful Source/Palette multi-row runtime family is now defined as the row-indexed root-basin pair family with supported first pairs `root_index + root_classic_palette` and `root_index + joy_root_palette`.
- Landed: the next implementation slice must keep separate Source / Shape / Palette editor lanes intact, pair Source and Palette rows by shared row index at the live bridge, and promote only the last enabled valid root-basin pair into the runtime.
- Landed: the next implementation slice must add bounded runtime-owner state for paired Source/Palette rows while mirroring the selected active pair back into the legacy flat fields for current renderer/runtime compatibility.
- Landed: the conceptual schedule model remains the already-mapped simple Factorio train-scheduler pattern; only the unsupported generic Source/Palette runtime semantics stay deferred.
- Landed: generic Source generator composition, generic Palette chaining, `root_proximity` pairing, and Grading are explicitly deferred until later plan authority names the needed math.

## Hostile Audit

- Status: complete
- Required posture: assume the first-family story is wrong until the named family reuses only already-proven coupled tuples and explicitly defers every unsupported composition claim.

## Audit Passes

- [x] Pass 1 - re-read the runtime math and confirm that Source still emits one scalar and Palette still maps one scalar, with Shape as the only ordered composition seam.
- [x] Pass 2 - inspect the nearest shipped paired tuples and confirm the root-basin family already has companion-pair editor behavior plus shared programmable runtime proof.
- [x] Pass 3 - re-read the named first-family rules and confirm they narrow the next implementation slice to bounded pair-state and live-bridge work instead of a fake generic composition claim.

## Audit Findings

- [done] Real defect found: the Phase 5 resume point still implied a concrete multi-row Source/Palette family could be named directly from the current runtime model even though the checked-in math only defines one Source generator and one Palette mapper.
- [done] Real defect repaired: this successor authority now narrows the first family to existing root-basin paired tuples and explicitly defers generic Source/Palette composition until a later slice defines real math and owner rules for it.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_library_foundation_phase5_source_palette_family_definition_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_library_foundation_phase5_source_palette_family_definition.contract.json`
  - `docs/notes/advanced_color_library_foundation_phase5_source_palette_runtime_authority_PHASED_PLAN.md`
  - `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`
- Non-goals for this slice:
  - do not touch runtime or editor code in this planning-only slice
  - do not claim generic multi-row Source generator composition yet
  - do not claim generic Palette chaining or new palette parameter surfaces yet
  - do not widen `root_proximity` into the first paired family yet
  - do not resume Grading before the bounded pair-state/runtime slice lands honestly

## Resume Point

Treat this as the completed predecessor for the next executable Phase 5 slice. Resume from `docs/notes/advanced_color_library_foundation_phase5_root_basin_pair_runtime_PHASED_PLAN.md` under `docs/contracts/advanced_color_library_foundation_phase5_root_basin_pair_runtime.contract.json`, starting with focused REDs in `ui_app/tests/test_schema_binding.cpp` and the nearest runtime-owner/runtime-color tests that prove row-indexed root-basin pair selection fails until bounded Source/Palette pair-stack owners and the last-enabled-valid-pair live bridge exist.