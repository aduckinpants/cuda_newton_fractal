# Advanced Color Library Foundation Phase 5 - Root-Basin Pair Runtime

## Current Phase

Complete - the bounded root-basin pair runtime slice closed at `b596af6` under `ck:6460d382`; this subplan is now a completed predecessor for Phase 6 Grading promotion

## Phase Checklist

- [x] Phase 1 - add focused REDs that prove two row-indexed root-basin pairs must apply to live state, mirror the last enabled valid pair into the legacy flat tuple, and resync from live state without collapsing back to one row
- [x] Phase 2 - add a bounded root-basin pair runtime owner while preserving backward-compatible single-pair imports and the existing root-basin renderer path
- [x] Phase 3 - wire the advanced window live bridge to pair Source and Palette rows by shared row index, fail closed on unmatched or unsupported enabled rows, and mirror the final valid pair into the legacy flat fields
- [x] Phase 4 - keep reset/defaults plus diagnostics/archive persistence truthful for the bounded pair schedule, then validate, hostile-audit, and checkpoint the slice cleanly

## Explicit User Asks

- [done] Phase 5 - keep moving on the defined advanced-color path after the family-definition checkpoint instead of drifting away.
- [done] Phase 5 - keep the already-mapped simple Factorio train-scheduler UX direction visible in planning even while generic Source/Palette runtime semantics stay deferred.
- [done] Phase 5 - make the next step a real runtime-backed slice instead of another vague planning placeholder.
- [done] Phase 5 - keep the reusable color pipeline generic and fail-closed rather than hiding unsupported mixed-row behavior behind silent fallback.

## Presumption Loop

The nearest controlling seam is the Source / Palette live bridge in `ui_app/src/color_pipeline_window.h`, plus the flat Source / Palette runtime authority in `ui_app/src/fractal_types.h`. The current advanced editor already models the conceptual lane-stack schedule with the discussed simple Factorio train-scheduler UX, but the live bridge still resolves exactly one enabled Source row and one enabled Palette row through `FindSingleEnabledColorPipelineRow(...)`. The first truthful executable family is narrower than generic composition: the row-indexed root-basin pair family defined in the predecessor plan. The falsifiable hypothesis for this slice is that a bounded runtime owner for row-indexed root-basin pairs can make a two-row root-basin schedule truthful without inventing generic Source generator composition or Palette chaining. The live bridge should zip Source and Palette rows by shared row index, require each enabled pair to be one of the supported `root_index + root_classic_palette` or `root_index + joy_root_palette` recipes, and mirror only the last enabled valid pair back into the legacy flat runtime tuple for current renderer compatibility. The cheapest disconfirming checks are focused REDs in `ui_app/tests/test_schema_binding.cpp` that currently fail because the Source / Palette bridge still rejects more than one enabled row, plus the nearest reset/persistence proofs once the owner fields exist.

## Presumption Evidence

- `docs/notes/advanced_color_pipeline_slice7_catalog_runtime_binding_PHASED_PLAN.md` already records the conceptual Source / Shape / Palette schedule as the simple Factorio train-scheduler-style UX direction.
- `ui_app/src/color_pipeline_window.h` still builds the active runtime tuple from `FindSingleEnabledColorPipelineRow(...)`, which hard-proves the current Source / Palette live bridge is still single-row only.
- `ui_app/src/color_pipeline_core.h` already knows the supported root-basin pairings for `root_index + root_classic_palette` and `root_index + joy_root_palette`, so the remaining gap is bounded runtime/live-bridge truth instead of catalog identity.
- `docs/notes/advanced_color_root_palette_tuple_switch_followup_PHASED_PLAN.md` proves the editor already co-switches those root-basin companion pairs, which means the schedule concept exists in the UI even though the runtime owner is still flat.
- `docs/notes/advanced_color_root_palette_shape_interactivity_PHASED_PLAN.md` proves the renderer already honors Shape for those root-basin tuples through the shared programmable basin path, so mirroring the last valid pair into the flat tuple preserves current runtime math instead of demanding a new palette evaluator in this slice.

## Proof Ledger

- Landed: `ui_app/tests/test_schema_binding.cpp` now proves a two-row root-basin schedule can apply to live state, mirror the final valid pair into the legacy flat runtime tuple, resync as a supported two-row Source / Palette live snapshot, and then restore the draft editor to one-row state for later single-row tuple coverage.
- Landed: `ui_app/src/fractal_types.h` plus `ui_app/src/color_pipeline_window.h` now add a bounded root-basin pair owner, route multi-row root-basin draft selection through row-indexed pair recognition, and rebuild live snapshots from the pair owner only when it coherently mirrors the flat runtime tuple.
- Landed: `ui_app/src/runtime_reset.cpp`, `ui_app/src/diagnostics_state_io.cpp`, and `ui_app/src/diagnostics_capture.cpp` now clear, load, and capture the bounded root-basin pair owner so the schedule is not a live-editor-only illusion and stale pair-owner state is not re-persisted when it no longer mirrors the flat runtime tuple.
- Landed: `ui_app/tests/test_runtime_reset.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp`, and `ui_app/tests/test_finding_archive_actions.cpp` now pin reset/default clearing, diagnostics-state round-trip, and capture/archive persistence for bounded root-basin pair schedules.
- Validated: `artifacts/root_basin_pair_runtime_green_check_5.log` is green for `ui_app/build_tests_vsdevcmd.cmd` after the bounded root-basin pair owner/live-bridge changes.
- Validated: `artifacts/root_basin_pair_persistence_native.log` and `artifacts/root_basin_pair_persistence_native_rerun.log` are green for `ui_app/build_tests_vsdevcmd.cmd` after the reset/default and diagnostics/archive persistence follow-up plus the stale-owner capture audit repair.
- Validated: `artifacts/code_quality_report.json` stayed at the repo baseline, `artifacts/verify_runtime_publish.log` republished the runtime cleanly, and `artifacts/verify_runtime_probe_session_pytest.log` reports `64 passed` against the published runtime.
- Checkpointed: the bounded root-basin pair runtime slice closed at commit `b596af6` with the linked `ck:6460d382` handoff entry plus machine-written validation and contract proof receipts.

## Hostile Audit

- Status: complete
- Required posture: assume the first implementation will preserve a hidden one-row fallback or an unmatched-row silent fallback until the repaired state proves otherwise.

## Audit Passes

- [done] Pass 1 - inspected the focused schema-binding RED/GREEN path and confirmed the bounded two-row schedule is exercised directly instead of being silently collapsed back to one row.
- [done] Pass 2 - inspected the landed runtime-owner/live-bridge diff and found one real defect: diagnostics capture was willing to persist stale root-basin pair-owner state even after the flat live tuple changed elsewhere; repaired capture to emit pair schedules only when they coherently mirror the live tuple, then reran the focused native helper rail.
- [done] Pass 3 - re-read reset/default and diagnostics/archive persistence, confirmed the new pair owner is cleared on reset, parsed from saved state, captured only when coherent, and validated that state through both native helper and published-runtime proof lanes.

## Audit Findings

- [closed] Repaired a stale-owner persistence defect in `ui_app/src/diagnostics_capture.cpp`: bounded root-basin pair schedules are now serialized only when every stored pair is runtime-backed and the final pair still mirrors the flat live tuple.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`
  - `docs/notes/advanced_color_library_foundation_phase5_source_palette_family_definition_PHASED_PLAN.md`
  - `docs/notes/advanced_color_library_foundation_phase5_root_basin_pair_runtime_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_library_foundation_phase5_root_basin_pair_runtime.contract.json`
  - `ui_app/src/color_pipeline_core.h`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/diagnostics_capture.cpp`
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/src/fractal_derived_fields.cpp`
  - `ui_app/src/fractal_types.h`
  - `ui_app/src/runtime_reset.cpp`
  - `ui_app/tests/test_diagnostics_state_io.cpp`
  - `ui_app/tests/test_finding_archive_actions.cpp`
  - `ui_app/tests/test_runtime_reset.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals for this slice:
  - do not invent generic Source generator composition yet
  - do not invent generic Palette chaining yet
  - do not widen `root_proximity` into the first family yet
  - do not resume Grading before the bounded pair-runtime slice closes honestly
  - do not replace the already-mapped simple Factorio schedule UX with a new composition model

## Resume Point

Do not reopen checkpoint closure from this subplan. Return to `docs/notes/advanced_color_library_foundation_phase6_grading_runtime_authority_PHASED_PLAN.md` under `docs/contracts/advanced_color_library_foundation_phase6_grading_runtime_authority.contract.json` and start with focused REDs around the missing grading lane plus bounded `contrast_lift` promotion.