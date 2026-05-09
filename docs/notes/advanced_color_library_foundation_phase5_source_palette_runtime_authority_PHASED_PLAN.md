# Advanced Color Library Foundation Phase 5 - Source/Palette Runtime Authority

## Current Phase

Complete - the first bounded Source/Palette backend-recovery slice moved the exact shipped tuple and row-param authority into the reusable core seam, leaving the first concrete multi-row Source/Palette recipe family as the next remaining Phase 5 runtime step

## Phase Checklist

- [x] Phase 1 - add focused REDs that prove the shipped Source/Palette tuple mapping and param import/apply seams are still owned by `color_pipeline_window.h` instead of the reusable core layer
- [x] Phase 2 - move the exact shipped Source/Palette tuple and param authority into shared core/runtime helpers while preserving shipped single-row compatibility and naming multi-row execution as the next remaining seam
- [x] Phase 3 - validate, hostile-audit, and checkpoint the bounded Source/Palette authority-extraction slice cleanly

## Explicit User Asks

- [done] Phase 5-7 - This bounded executable slice landed as a checked-in planned step with real code/test/validation closure instead of a vague placeholder.
- [done] Phase 5-7 - The bounded extraction treated the Source/Palette backend seam as a critical architectural move and closed it with real implementation plus hostile audit, not a lazy option pass.
- [done] Phase 5-7 - The landed seam is simpler to extend because the shipped Source/Palette row/lane model and helper authority now live in the reusable core instead of the window header.
- [done] Phase 5-7 - The architecture is stronger after this slice because tuple rebuild, row build/defaults, and Source/Palette import/apply meaning now live in a reusable descriptor-driven core seam that can keep widening without re-expanding the window layer.
- [done] Phase 5-6 - For this bounded slice, `color_pipeline_window.h` stopped being the authority for the shipped Source/Palette tuple mapping and row-param import/apply meaning; the still-open multi-row/runtime-owner/defaults/persistence work is explicitly deferred to the next Phase 5 slice.
- [done] Phase 5-7 - The reusable color pipeline stays generic and separately owned here because the extracted helper seam remains catalog/row driven rather than baking fractal-family specifics back into the window layer.

## Presumption Loop

The nearest controlling backend seam is now Source and Palette, not Shape. `ui_app/src/color_pipeline_window.h` still owns the shipped Source/Palette tuple mapping plus the row-param import/apply helpers, while `ui_app/src/color_pipeline_core.h` already owns the reusable catalog and bridge-id descriptors. There is not yet a checked-in first concrete multi-row Source/Palette recipe family, so the falsifiable hypothesis for this bounded slice is narrower: if the exact shipped tuple mapping and Source/Palette row-param import/apply seams move into the reusable core layer first, `color_pipeline_window.h` can stop being the authority for those behaviors without widening Grading, inventing unsupported multi-row semantics, or redefining the mapped advanced-color inventory. The cheapest disconfirming checks are focused REDs in `ui_app/tests/test_schema_binding.cpp` that fail until the core layer owns those exact shipped seams.

## Presumption Evidence

- `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md` now records the bounded shipped-seam extraction as landed and points the next Phase 5 work at naming the first concrete multi-row Source/Palette runtime family instead of pretending Grading is next.
- `ui_app/src/color_pipeline_core.h` now owns `ColorPipelineParamState`, `ColorPipelineRowState`, `ColorPipelineLaneState`, `ColorPipelineLiveSnapshot`, `TryBuildColorPipelineSelectionFromLaneIds(...)`, `BuildColorPipelineRowFromFunctionId(...)`, `ImportSupportedColorPipelineParamsFromLive(...)`, and `ApplySupportedColorPipelineRowParamsToLive(...)` for the shipped Source/Palette rows.
- `ui_app/src/color_pipeline_window.h` now delegates those shipped tuple and row-param seams to the reusable core layer and limits itself to window orchestration plus the still-open multi-row/runtime-owner work.
- `docs/notes/advanced_color_pipeline_slice7_catalog_runtime_binding_PHASED_PLAN.md` still records Source / Shape / Palette lane stacks and ordered composition as the shipped direction, but it still does not name a first concrete multi-row Source/Palette recipe family, so that remains the next honest implementation seam.

## Proof Ledger

- Landed: `docs/notes/advanced_color_library_foundation_phase5_source_palette_runtime_authority_PHASED_PLAN.md` and `docs/contracts/advanced_color_library_foundation_phase5_source_palette_runtime_authority.contract.json` now bound the next executable backend-recovery slice.
- Landed: `ui_app/src/color_pipeline_core.h` now owns the shared Source/Palette row and lane model types plus the shipped tuple rebuild, row build/defaults, live import, and row apply/reset helpers.
- Landed: `ui_app/src/color_pipeline_window.h` now delegates the shipped Source/Palette tuple and row-param seams to `ui_app/src/color_pipeline_core.h` instead of defining those behaviors inline.
- Landed: `ui_app/tests/test_schema_binding.cpp` now locks the extracted core through shipped tuple rebuild coverage, `phase_orbit` row import coverage, and `explaino_cmap` row apply/reset coverage.
- Landed: `docs/notes/advanced_color_library_foundation_phase5_source_palette_family_definition_PHASED_PLAN.md` plus `docs/contracts/advanced_color_library_foundation_phase5_source_palette_family_definition.contract.json` now define the first truthful Source/Palette multi-row family as the row-indexed root-basin pair family and bind the next bounded implementation slice.
- Validated: `artifacts/validation/advanced_color_library_foundation_phase5_source_palette_runtime_authority_contract.json` validates the revised bounded extraction contract and `artifacts/validation/viewer_host_assert_phased_plan_sync.json` stays green.
- Validated: `artifacts/code_quality_report.json` stayed at the repository baseline, `artifacts/verify_native_helper_tests_red_check.log` is green for the fresh native helper rebuild, `artifacts/verify_runtime_publish.log` republished the runtime cleanly, and `artifacts/verify_runtime_probe_session_pytest.log` reports `64 passed` against the published runtime.

## Hostile Audit

- Status: complete
- Required posture: assume Shape-only recovery left Source/Palette tuple and param truth trapped in `color_pipeline_window.h` until the reusable core owns those exact shipped seams and the window only orchestrates them.

## Audit Passes

- [x] Pass 1 - inspect the first REDs and ensure they fail because the shipped Source/Palette tuple or param helpers still live in `color_pipeline_window.h` instead of the reusable core seam.
- [x] Pass 2 - inspect the landed authority-extraction diff for hidden window-local fallbacks in tuple mapping, row defaults, and Source/Palette param import/apply.
- [x] Pass 3 - re-read the repaired state after validation and confirm the slice removes those shipped Source/Palette seams from `color_pipeline_window.h` without drifting into Grading or inventing unsupported multi-row execution.

## Audit Findings

- [done] Real defect found and fixed during the extraction attempt: the first patch pass accidentally preserved a stale `joy_root_palette -> root_basin` mapping and temporarily clobbered `ApplyColorPipelineDraftToLiveState`; the repaired state restores the original `joy_basins` behavior and the apply path before the validation rails go green.
- [done] Re-read finding: after the repaired native rebuild and published-runtime rails passed, the bounded shipped tuple/param seams no longer live in `color_pipeline_window.h`; the next honest remaining seam is naming a concrete multi-row Source/Palette runtime family and its matching runtime-owner shape.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`
  - `docs/notes/advanced_color_library_foundation_phase5_source_palette_runtime_authority_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_library_foundation_phase5_source_palette_runtime_authority.contract.json`
  - `ui_app/src/color_pipeline_core.h`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/diagnostics_capture.cpp`
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/src/fractal_derived_fields.cpp`
  - `ui_app/src/fractal_types.h`
  - `ui_app/tests/test_diagnostics_state_io.cpp`
  - `ui_app/tests/test_escape_time_coloring.cpp`
  - `ui_app/tests/test_finding_archive_actions.cpp`
  - `ui_app/tests/test_runtime_reset.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals for this slice:
  - do not widen Grading yet
  - do not redefine the mapped advanced-color inventory
  - do not reopen the closed Shape-stack recovery slice as if it were still pending
  - do not invent or claim a concrete multi-row Source/Palette execution family before it is explicitly named in checked-in plan authority

## Resume Point

Treat this as a completed predecessor. Resume from `docs/notes/advanced_color_library_foundation_phase5_source_palette_family_definition_PHASED_PLAN.md` under `docs/contracts/advanced_color_library_foundation_phase5_source_palette_family_definition.contract.json` instead of reopening the shipped-seam extraction slice.