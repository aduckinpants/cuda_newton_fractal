# UI Polish Slice 1 - Schema Domains And Control Polish

## Current Phase

Phase 1 - wait for Phase 0 closeout and capture the schema-domain baseline

## Phase Checklist

- [ ] Phase 1 - wait for Phase 0 closeout and capture the schema-domain baseline
- [ ] Phase 2 - define the target domains, widget hints, and grouping fixes in the schema/binding seam
- [ ] Phase 3 - implement the schema, binding, and focused test updates
- [ ] Phase 4 - hostile audit the resulting UI-domain behavior and checkpoint the slice

## Explicit User Asks

- [open] Fix slider values that are not covering the proper domains.
- [open] Treat the UI polish as a bounded slice instead of a broad refactor.

## Presumption Loop

The most likely owner is the schema metadata and binding layer, not a pile of one-off ImGui widget overrides in `main.cpp`. The slice should start from the existing schema authority and only step into renderer-facing code if focused tests prove the metadata surface cannot express the required domains.

Hostile review assumes the current static ranges are too weak or too generic for several controls. Write the narrowest regression that proves a wrong domain or missing widget hint before broadening the schema surface.

## Presumption Evidence

- Owner Proof: current UI review showed domain/range behavior is centralized in `ui/fractal_binding_surface_v1.ui_schema.json`, `ui_app/src/ui_schema.h`, and `ui_app/src/schema_binding.cpp`.
- RED Witness: pending.
- Fix Proof: pending.
- Hostile Review Pass 1: pending.
- Hostile Review Pass 2: pending.

## Proof Ledger

- Manual RED: pending.
- Checked-in regression RED: pending.
- First GREEN: pending.
- Post-green hostile finding: pending.

## Notes

- Expected owner files:
  - `ui/fractal_binding_surface_v1.ui_schema.json`
  - `ui_app/src/ui_schema.h`
  - `ui_app/src/schema_binding.cpp`
  - `ui_app/src/function_descriptor.cpp`
- Non-goals:
  - do not redesign the color-mode authority here
  - do not change render-resolution defaults or pacing policy here
- Validation target:
  - focused workflow or headless tests for schema/binding behavior
  - relevant native helper validation before checkpoint

## Resume Point

Start by inventorying the controls whose configured domains do not match operator expectation, then turn that inventory into the smallest failing schema/binding regression.