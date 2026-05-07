# Advanced Color Library Foundation

## Current Phase

Phase 4 ready - Phase 3 Shape is now fully widened through `smooth_window`; the next bounded slice is the initial Palette library through the extracted registry/core, family-gating, live-bridge, and persistence seams

## Phase Checklist

- [x] Phase 0 - check in the oracle/terminology note, architecture contract, and initial library inventory for the four-category foundation
- [x] Phase 1 - extract category/function/catalog/live-bridge authority out of `ui_app/src/color_pipeline_window.h` so the window becomes a consumer for the shipped catalog truth before any library widening starts
- [x] Phase 2 - land the full initial Source library through the extracted descriptor/registry core and the broader runtime-source authority refactor the user explicitly chose
- [x] Phase 3 - land the full initial Shape library through the extracted descriptor/registry core
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
- Validated: `artifacts/validation/advanced_color_library_foundation_phase0_oracle_inventory_contract.json` shows the Phase 0 contract validated cleanly, and `artifacts/validation/viewer_host_assert_phased_plan_sync.json` shows the phased plan stayed synchronized.
- Checkpointed: Phase 0 closed at commit `6624781` with handoff and machine proof receipts.
- Landed: `ui_app/src/color_pipeline_core.h` now owns the shipped advanced color lane-catalog type, descriptor builders, function-id mappings, runtime-backed filtering, and schedule-bridge tuple mapping.
- Landed: `ui_app/src/color_pipeline_window.h` now delegates the shipped catalog and bridge helpers to `ui_app/src/color_pipeline_core.h` instead of defining that authority inline.
- Landed: `ui_app/tests/test_schema_binding.cpp` now exercises the extracted core directly, proving the shipped Shape catalog and the banded schedule-bridge tuple without routing through the window helper first.
- Validated: `artifacts/validation/advanced_color_library_foundation_phase1_core_extraction_contract.json` shows the Phase 1 contract validated cleanly, `artifacts/validation/viewer_host_assert_phased_plan_sync.json` shows the plan stayed synchronized, `artifacts/code_quality_report.json` stayed at baseline (`97/100`), `artifacts/verify_native_helper_tests.log` is green, `artifacts/verify_runtime_publish.log` published the runtime cleanly, and `artifacts/verify_runtime_probe_session_pytest.log` reports `68 passed`.
- Deferred explicitly: apply/reset/default/serialization ownership cleanup is still part of the broader foundation architecture, but Phase 1 did not claim to finish that seam; it only moved the shipped catalog and bridge truth into the dedicated core header.
- Resolved ambiguity: the user explicitly chose the broader long-term-correct Phase 2 path instead of narrowing to only already-runtime-real source rows. Phase 2 therefore owns the runtime-source expansion and mirror/bridge refactor needed to land `escape_magnitude`, `orbit_stripe`, and family-gated `root_proximity` honestly.
- Landed: `ui_app/src/fractal_types.h`, `ui_app/src/enum_id_utils.h`, `ui_app/src/color_pipeline_core.h`, `ui_app/src/fractal_family_rules.h`, and `ui_app/src/schema_binding.cpp` now treat `escape_magnitude`, `orbit_stripe`, and family-gated `root_proximity` as shipped Source rows with real runtime ids, selectable bridge tuples, and fail-closed family gating.
- Landed: `ui_app/src/escape_time_coloring.h`, `ui_app/src/basin_coloring.h`, `ui_app/src/color_pipeline_window.h`, `ui_app/src/diagnostics_capture.cpp`, and `ui_app/src/diagnostics_state_io.cpp` now own the widened Source runtime parameters, programmable signal sampling, draft/live bridge behavior, and persisted diagnostics state for the new rows.
- Landed: `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_escape_time_coloring.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp`, and `ui_app/tests/test_finding_archive_actions.cpp` now lock the widened Source catalog, runtime behavior, diagnostics load/save, capture-finding persistence, and the phase-wheel interior-color regression found during hostile audit.
- Validated: `artifacts/validation/advanced_color_library_foundation_phase2_source_runtime_expansion_contract.json` remains green for the Phase 2 contract, `artifacts/validation/viewer_host_assert_phased_plan_sync.json` stays green, `artifacts/code_quality_report.json` is back on the `97/100` baseline, `artifacts/verify_native_helper_tests.log` is green after the hostile-audit regression fix, `artifacts/verify_runtime_publish.log` republished the runtime cleanly, and `artifacts/verify_runtime_probe_session_pytest.log` reports `68 passed` against the republished runtime.
- Audit: hostile review found that programmable `orbit_stripe` samples mirrored through the phase wheel were forcing interior pixels to black instead of preserving the dim phase-wheel interior color; `ui_app/tests/test_escape_time_coloring.cpp` now locks that case and `ui_app/src/escape_time_coloring.h` allows programmable phase-wheel palettes to render their interior color.
- Checkpointed: Phase 2 closed at commit `f3c8bd0` with the linked `ck:021d50d4` handoff entry plus machine-written validation and contract proof receipts.
- Next bounded slice: Phase 3 starts with `posterize` as the first honest Shape widening step because the descriptor already exists while the runtime, live-bridge, and persistence seams still reject it as draft-only.
- Landed: `ui_app/src/fractal_types.h`, `ui_app/src/enum_id_utils.h`, `ui_app/src/color_pipeline_core.h`, `ui_app/src/escape_time_coloring.h`, `ui_app/src/color_pipeline_window.h`, `ui_app/src/diagnostics_capture.cpp`, `ui_app/src/diagnostics_state_io.cpp`, and `ui_app/src/fractal_derived_fields.cpp` now treat `posterize` as a real Shape row with runtime ids, defaults, live import/apply/reset support, shape math, and diagnostics persistence.
- Landed: `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_escape_time_coloring.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp`, `ui_app/tests/test_finding_archive_actions.cpp`, and `ui_app/tests/test_runtime_reset.cpp` now lock posterize catalog visibility, live bridge apply/sync behavior, runtime signal shaping, diagnostics round-trip, Capture Finding persistence, and reset defaults.
- Validated: `artifacts/validation/advanced_color_library_foundation_phase3_shape_posterize_contract.json` validates the Phase 3 posterize contract, `artifacts/validation/viewer_host_assert_phased_plan_sync.json` stays green, `artifacts/code_quality_report.json` remains on the `97/100` baseline, `artifacts/verify_native_helper_tests.log` is green, `artifacts/verify_runtime_publish.log` republished the runtime cleanly, and `artifacts/verify_runtime_probe_session_pytest.log` reports `68 passed`.
- Audit: hostile review of the posterize runtime-authority, live-bridge, and persistence diffs did not uncover a second real defect after the local draft-apply choke-point repair and the strengthened posterize runtime regression sample.
- Checkpointed: the `posterize` Phase 3 sub-slice closed at commit `080701a` with the linked `ck:76888967` handoff entry plus machine-written validation and contract proof receipts.
- Next bounded slice: `mirror_repeat` is the next honest Shape widening step because it can reuse the existing repeat frequency/phase owners while widening the runtime shape math and live bridge beyond the current sawtooth-only repeat behavior.
- Landed: `ui_app/src/fractal_types.h`, `ui_app/src/enum_id_utils.h`, `ui_app/src/color_pipeline_core.h`, `ui_app/src/escape_time_coloring.h`, `ui_app/src/color_pipeline_window.h`, and `ui_app/src/diagnostics_capture.cpp` now treat `mirror_repeat` as a real Shape row with runtime ids, descriptor authority, mirrored-wave shape math, live import/apply support, and Capture Finding serialization while reusing the existing repeat frequency/phase owner fields.
- Landed: `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_escape_time_coloring.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp`, and `ui_app/tests/test_finding_archive_actions.cpp` now lock mirror_repeat catalog visibility, live bridge apply/sync behavior, runtime signal shaping, diagnostics load, and Capture Finding persistence through the reused repeat owner fields.
- Validated: `artifacts/validation/advanced_color_library_foundation_phase3_shape_mirror_repeat_contract.json` validates the Phase 3 mirror_repeat contract, `artifacts/validation/viewer_host_assert_phased_plan_sync.json` stays green, `artifacts/code_quality_report.json` remains on the `97/100` baseline, `artifacts/verify_native_helper_tests.log` is green, `artifacts/verify_runtime_publish.log` republished the runtime cleanly, and `artifacts/verify_runtime_probe_session_pytest.log` reports `68 passed`.
- Audit: hostile review of the mirror_repeat runtime-authority, live-bridge, and persistence diffs did not uncover a second real defect after the targeted hard-coded-shape-list audit passes.
- Checkpointed: the `mirror_repeat` Phase 3 sub-slice closed at commit `6147b08` with the linked `ck:f44e55b8` handoff entry plus machine-written validation and contract proof receipts.
- Next bounded slice: `bias_gain_curve` is the next honest Shape widening step after `mirror_repeat` because it needs its own explicit shape.bias / shape.gain runtime owners rather than descriptor-only coverage or owner-field reuse, followed by `smooth_window` through the same extracted registry/core and live-bridge seams.
- Landed: `ui_app/src/fractal_types.h`, `ui_app/src/enum_id_utils.h`, `ui_app/src/color_pipeline_core.h`, `ui_app/src/color_pipeline_window.h`, `ui_app/src/escape_time_coloring.h`, `ui_app/src/diagnostics_capture.cpp`, `ui_app/src/diagnostics_state_io.cpp`, and `ui_app/src/fractal_derived_fields.cpp` now treat `bias_gain_curve` as a real Shape row with runtime ids, dedicated shape.bias / shape.gain owners, live import/apply support, runtime curve math, diagnostics load/save, and reset defaults.
- Landed: `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_escape_time_coloring.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp`, `ui_app/tests/test_finding_archive_actions.cpp`, and `ui_app/tests/test_runtime_reset.cpp` now lock bias_gain_curve catalog visibility, live bridge apply/sync behavior, runtime signal shaping, diagnostics round-trip, Capture Finding persistence, and reset defaults through the dedicated owner pair.
- Validated: `artifacts/validation/advanced_color_library_foundation_phase3_shape_bias_gain_curve_contract.json` validates the Phase 3 bias_gain_curve contract, `artifacts/validation/viewer_host_assert_phased_plan_sync.json` stays green, `artifacts/code_quality_report.json` remains on the `97/100` baseline, `artifacts/verify_native_helper_tests.log` is green, `artifacts/verify_runtime_publish.log` republished the runtime cleanly, and `artifacts/verify_runtime_probe_session_pytest.log` reports `68 passed`.
- Audit: hostile review of the bias_gain_curve runtime-authority, live-bridge, and persistence diffs did not uncover a second real defect after the targeted hard-coded-shape-list and owner-field audit passes.
- Checkpointed: the `bias_gain_curve` Phase 3 sub-slice closed at commit `75e63b3` with the linked `ck:bd23c697` handoff entry plus machine-written validation and contract proof receipts.
- Next bounded slice: `smooth_window` is the final honest Shape widening step after `bias_gain_curve` because the existing `EscapeTimeColorSmoothstep(...)` helper already supports a bounded smooth window once the row gets explicit shape.center / shape.width / shape.softness runtime owners.
- Landed: `ui_app/src/fractal_types.h`, `ui_app/src/enum_id_utils.h`, `ui_app/src/color_pipeline_core.h`, `ui_app/src/color_pipeline_window.h`, `ui_app/src/escape_time_coloring.h`, `ui_app/src/diagnostics_capture.cpp`, `ui_app/src/diagnostics_state_io.cpp`, and `ui_app/src/fractal_derived_fields.cpp` now treat `smooth_window` as a real Shape row with runtime ids, dedicated shape.center / shape.width / shape.softness owners, live import/apply support, wrapped smooth-window math, diagnostics load/save, and reset defaults.
- Landed: `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_escape_time_coloring.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp`, `ui_app/tests/test_finding_archive_actions.cpp`, and `ui_app/tests/test_runtime_reset.cpp` now lock smooth_window catalog visibility, live bridge apply/sync behavior, runtime signal shaping, seam-wrap behavior, diagnostics round-trip, Capture Finding persistence, and reset defaults through the dedicated owner triple.
- Validated: `artifacts/validation/advanced_color_library_foundation_phase3_shape_smooth_window_contract.json` validates the Phase 3 smooth_window contract, `artifacts/validation/viewer_host_assert_phased_plan_sync.json` stays green, `artifacts/code_quality_report.json` remains on the `97/100` baseline, `artifacts/verify_native_helper_tests.log` is green, `artifacts/verify_runtime_publish.log` republished the runtime cleanly, and `artifacts/verify_runtime_probe_session_pytest.log` reports `68 passed` against the published runtime.
- Audit: hostile review found that the first smooth_window implementation failed to wrap a window centered near the upper domain edge across the shape seam; `ui_app/tests/test_escape_time_coloring.cpp` now locks that case and `ui_app/src/escape_time_coloring.h` computes smooth_window via wrapped center distance instead of a non-wrapping lo/hi interval.

## Notes

- Expected owner files for the current slice:
  - `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_library_foundation_phase3_shape_smooth_window.contract.json`
  - `ui_app/src/color_pipeline_core.h`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/diagnostics_capture.cpp`
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/src/enum_id_utils.h`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/src/fractal_derived_fields.cpp`
  - `ui_app/src/fractal_types.h`
  - `ui_app/tests/test_diagnostics_state_io.cpp`
  - `ui_app/tests/test_escape_time_coloring.cpp`
  - `ui_app/tests/test_finding_archive_actions.cpp`
  - `ui_app/tests/test_runtime_reset.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals for this slice:
  - do not widen beyond `smooth_window` inside the Shape lane yet
  - do not implement the Balance/Void grading operator yet
  - do not implement ExplainO-BalanceVoid yet
  - do not widen Palette or Grading yet
  - do not widen into Blend or Mask/Domain yet
  - do not add preview-only shape rows or fake bridge support

## Resume Point

After `smooth_window` lands as a validated runtime-real Shape row, close Phase 3 and continue into the initial Palette library through the same extracted registry/core, family-gating, and live-bridge seams.







