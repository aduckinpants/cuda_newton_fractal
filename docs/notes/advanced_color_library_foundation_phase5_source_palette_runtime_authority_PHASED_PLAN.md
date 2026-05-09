# Advanced Color Library Foundation Phase 5 - Source/Palette Runtime Authority

## Current Phase

Phase 1 in progress - lock the next backend-recovery slice around Source/Palette live-bridge and runtime-state authority so supported multi-row lanes can become runtime-real without reopening Grading or the mapped inventory

## Phase Checklist

- [ ] Phase 1 - add focused REDs that prove supported multi-row Source and Palette drafts cannot currently import/apply or sync through live runtime truth
- [ ] Phase 2 - replace the single-row Source/Palette live bridge with bounded runtime-state owners while preserving shipped single-row compatibility and family-gated bridge tuples
- [ ] Phase 3 - validate, hostile-audit, and checkpoint the bounded Source/Palette runtime-authority slice cleanly

## Explicit User Asks

- [open] Phase 5-7 - Do the next step as a real planned-out full initial library of reusable functions per category, not a vague "3-ish" placeholder.
- [open] Phase 5-7 - Treat this as a critical move that needs real effort, not a lazy option pass.
- [open] Phase 5-7 - Make the result simple to extend with nice module boundaries and clean coding.
- [open] Phase 5-7 - Strengthen the architecture beyond dropdowns: make this a reusable, descriptor-driven color-pipeline core that could plausibly become its own DLL/static library later.
- [open] Phase 5-6 - `color_pipeline_window.h` must stop being the authority for category/function identity, parameter meaning, runtime applicability, import/apply behavior, reset/default behavior, or serialization truth.
- [open] Phase 5-7 - Keep the reusable color pipeline generic and separately owned even if fractal families later emit fields/signals that the pipeline can consume.

## Presumption Loop

The nearest controlling backend seam is now Source and Palette, not Shape. `ui_app/src/color_pipeline_window.h` still imports and applies those lanes through `BuildColorPipelineLaneWithSingleRow(...)`, `rows.front()`, and `FindSingleEnabledColorPipelineRow(...)`, while `ui_app/src/fractal_types.h` only owns a bounded runtime stack for Shape. The falsifiable hypothesis for this bounded slice is that adding bounded Source/Palette runtime-state authority plus matching live import/apply support can make supported multi-row Source and Palette lanes truthful without widening Grading or redefining the mapped advanced-color inventory. The cheapest disconfirming checks are focused REDs in `ui_app/tests/test_schema_binding.cpp` plus the neighboring runtime/persistence coverage that currently fail because Source and Palette still depend on single-row live-bridge helpers and lack equivalent bounded runtime owners.

## Presumption Evidence

- `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md` now records Source/Palette single-row live-bridge authority as the next remaining Phase 5 blocker after the landed Shape-stack slice.
- `ui_app/src/color_pipeline_window.h` still imports live Source and Palette through `BuildColorPipelineLaneWithSingleRow(...)`, `rows.front()`, and `FindSingleEnabledColorPipelineRow(...)`, which proves those lanes still collapse to a single enabled row at the live bridge.
- `ui_app/src/fractal_types.h` now owns `color_shape_stack`, but there is no equivalent bounded runtime-stack owner for Source or Palette yet.
- `docs/notes/advanced_color_pipeline_slice7_catalog_runtime_binding_PHASED_PLAN.md` already records Source / Shape / Palette lane stacks and ordered composition as the shipped direction, so this slice should repair runtime truth instead of reopening the product definition.

## Proof Ledger

- Landed: `docs/notes/advanced_color_library_foundation_phase5_source_palette_runtime_authority_PHASED_PLAN.md` and `docs/contracts/advanced_color_library_foundation_phase5_source_palette_runtime_authority.contract.json` now bound the next executable backend-recovery slice.
- Planned: start with focused REDs in the nearest bridge/state seams before widening live Source/Palette authority.
- Pending validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/advanced_color_library_foundation_phase5_source_palette_runtime_authority.contract.json --out-json artifacts/validation/advanced_color_library_foundation_phase5_source_palette_runtime_authority_contract.json`
- Pending validation: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`

## Hostile Audit

- Status: in progress
- Required posture: assume Shape-only recovery left a hidden single-row fallback in Source or Palette until the live bridge, runtime-state owners, and persistence paths prove otherwise.

## Audit Passes

- [open] Pass 1 - inspect the first REDs and ensure they fail because Source/Palette still route through single-row live-bridge helpers instead of bounded runtime truth.
- [open] Pass 2 - inspect the landed runtime-owner/live-bridge diff for hidden single-row fallbacks in import, apply, reset/defaults, and persistence.
- [open] Pass 3 - re-read the repaired state after validation and confirm the slice makes supported Source/Palette lanes truthful without drifting into Grading or inventory redefinition.

## Audit Findings

- [open] Pending the executable backend-recovery slice.

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
  - do not land editor-only Source/Palette stacks without matching live/runtime truth

## Resume Point

Start with focused REDs in `ui_app/tests/test_schema_binding.cpp` and the nearest runtime/persistence coverage that prove Source/Palette still collapse to single-row live authority, then land the minimal bounded Source/Palette runtime-owner and live-bridge repair.