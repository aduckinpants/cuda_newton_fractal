# Advanced Color Library Foundation

## Current Phase

Phase 0 validated - the ExplainO oracle split, stronger color-pipeline architecture contract, and initial balanced library inventory are checked in, and the deterministic validation rails are green; checkpoint closure remains before Phase 1 extraction starts

## Phase Checklist

- [x] Phase 0 - check in the oracle/terminology note, architecture contract, and initial library inventory for the four-category foundation
- [ ] Phase 1 - extract category/function/adapter/serialization authority out of `ui_app/src/color_pipeline_window.h` so the window becomes a consumer only
- [ ] Phase 2 - land the full initial Source library through the extracted descriptor/registry core
- [ ] Phase 3 - land the full initial Shape library through the extracted descriptor/registry core
- [ ] Phase 4 - land the full initial Palette library while preserving the ExplainO CMap versus basin/root palette split
- [ ] Phase 5 - promote Grading to a first-class category and land the initial Grade library, including the bounded filmic Balance/Void-inspired grading operator
- [ ] Phase 6 - close the four-category foundation with proof matrix, D: gallery/runtime captures, and explicit extension rules
- [ ] Phase 7 - only after the foundation closes, widen into additional categories; recommended order remains Blend first, then Mask/Domain

## Explicit User Asks

- [open] Do the next step as a real planned-out full initial library of reusable functions per category, not a vague "3-ish" placeholder.
- [open] Treat this as a critical move that needs real effort, not a lazy option pass.
- [open] Use ExplainO CMap as part of the palette planning, but do not conflate it with basin/root/joy palette work.
- [open] Make the result simple to extend with nice module boundaries and clean coding.
- [open] Strengthen the architecture beyond dropdowns: make this a reusable, descriptor-driven color-pipeline core that could plausibly become its own DLL/static library later.
- [open] `color_pipeline_window.h` must stop being the authority for category/function identity, parameter meaning, runtime applicability, import/apply behavior, reset/default behavior, or serialization truth.
- [open] Add a bounded generic filmic Balance/Void grading operator, but keep that separate from fractal-family geometry work.
- [open] Plan one experimental ExplainO-BalanceVoid deformation-pack family with three neutral-default axes instead of a branch explosion of separate fractals.
- [open] Keep the reusable color pipeline generic and separately owned even if fractal families later emit fields/signals that the pipeline can consume.

## Presumption Loop

The controlling product risk is no longer just "missing functions." The current advanced-color surface still keeps too much identity and behavior authority inline in `ui_app/src/color_pipeline_window.h`, which means any widening work risks scaling the window monolith instead of proving a reusable foundation. The most falsifiable first hypothesis is that a checked-in oracle/terminology note plus a checked-in inventory/architecture contract will remove the remaining authority ambiguity: ExplainO CMap, basin/root palettes, and the generic viewport colormap will stop being conflated, and later implementation slices will have a durable contract for where descriptors, runtime adapters, and serialization truth are allowed to live. The cheapest disconfirming checks are deterministic repo rails: the contract must validate, the phased plan must stay synchronized, and the note must name a balanced initial library plus the stronger ownership rules in a way future slices can point to directly.

## Presumption Evidence

- `ui_app/src/color_pipeline_window.h` still contains the shipped lane catalogs, runtime-backed filtering, descriptor builders, and live-bridge logic inline, which proves the window is still too close to the authority surface.
- `ui_app/src/function_descriptor.h` already provides reusable descriptor types, so the repo has a natural extraction seam for a future registry/catalog core.
- The verified references split the ExplainO surfaces cleanly: the legacy LUT lineage lives in `c:\code\salticid-cuda\carl-ca-python\ca_viewer.py`, while the current basin/root palettes live in `c:\code\salticid-cuda\content\packs\explaino\cuda\explaino_cuda_helpers.inl` and `c:\code\salticid-cuda\cuda_core\src\ops_special.cu`, and the generic viewport colormap surface in `c:\code\salticid-cuda\ide_ui_dx11\ui_app\src\viewport_colormap.cpp` / `.h` does not contain an ExplainO colormap.
- `docs/EXPLAINO_EXPERIMENTAL_FAMILY_REFERENCE.md` already shows the current ExplainO-family proliferation surface, which supports the user's request to rehearse a future meta-family pattern rather than keep branching into standalone fractal types.
- The recent advanced-color roadmap in `docs/notes/advanced_color_pipeline_slice7_catalog_runtime_binding_PHASED_PLAN.md` already says widening must happen one runtime-real row family at a time, so a stronger foundation contract is the next logical prerequisite rather than a detour.

## Proof Ledger

- Landed: `docs/notes/advanced_color_library_foundation_oracle_and_inventory.md` explicitly distinguishes legacy ExplainO CMap, Root Classic Palette, Joy Root Palette, and the generic viewport colormap surface as separate authorities.
- Landed: `docs/notes/advanced_color_library_foundation_oracle_and_inventory.md` records the stronger architecture rule that `color_pipeline_window.h` must become a consumer and not remain the authority for ids, semantics, adapters, or serialization truth.
- Landed: `docs/notes/advanced_color_library_foundation_oracle_and_inventory.md` records the initial balanced library inventory for Source / Shape / Palette / Grading, including the bounded Balance/Void-inspired grading operator and a separate ExplainO-BalanceVoid family track.
- Validated: `artifacts/validation/advanced_color_library_foundation_phase0_oracle_inventory_contract.json` shows the new slice contract validates cleanly, and `artifacts/validation/viewer_host_assert_phased_plan_sync.json` shows the phased plan remains synchronized.
- Pending: the Phase 0 docs-only slice is checkpointed with receipts before Phase 1 extraction begins.

## Notes

- Expected owner files for the current slice:
  - `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_library_foundation_phase0_oracle_inventory.contract.json`
  - `docs/notes/advanced_color_library_foundation_oracle_and_inventory.md`
- Expected owner files for follow-on implementation slices:
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/function_descriptor.h`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/src/basin_coloring.h`
  - `ui_app/src/fractal_types.h`
  - `ui_app/src/fractal_family_rules.h`
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
  - `ui_app/tests/test_escape_time_coloring.cpp`
- Non-goals for this slice:
  - do not start the extraction or widen the shipped runtime catalog yet
  - do not implement the Balance/Void grading operator yet
  - do not implement ExplainO-BalanceVoid yet
  - do not widen into Blend or Mask/Domain yet
  - do not add new hard-coded window branches while planning a reusable core

## Resume Point

After the checked-in oracle/inventory note lands and validates, Phase 1 begins: extract the category/function/adapter/serialization authority out of `ui_app/src/color_pipeline_window.h` and make the window consume that core instead of owning it.
