# Advanced Color Library Foundation Phase 6 - Grading Runtime Authority

## Current Phase

Phase 1 paused - the focused grading RED is preserved, but the runtime-backed grading promotion is temporarily deferred behind the workflow-hardening stop-line required to make failures report truthfully

## Phase Checklist

- [ ] Phase 1 - add focused REDs that prove Grading is absent from the advanced lane catalogs/live snapshots and that bounded `contrast_lift` grading cannot yet round-trip as a first-class advanced lane
- [ ] Phase 2 - promote a bounded grading lane into the reusable catalog/window live bridge while keeping legacy main-panel grading as the one authoritative mirror
- [ ] Phase 3 - ship only runtime-real grading rows through that lane, starting with `contrast_lift` and leaving wider Grade inventory rows fail-closed until their runtime math exists
- [ ] Phase 4 - keep reset/defaults plus diagnostics/archive persistence truthful for the bounded grading surface, then validate, hostile-audit, and checkpoint the slice cleanly

## Explicit User Asks

- [done] Phase 5 - keep moving on the defined advanced-color path instead of circling on already-closed work.
- [done] Phase 5-7 - keep the reusable color pipeline generic and separately owned instead of baking window-local exceptions into the product story.
- [deferred] Phase 6 - deferred for the workflow-hardening stop-line pivot; the real runtime-backed grading-category promotion remains the required next product slice after terminal/tooling hardening closes.
- [deferred] Phase 6 - deferred beyond the bounded grading-lane truth slice; Balance/Void stays separate from geometry work and will not widen until the simpler grading lane is proven end-to-end.

## Presumption Loop

The controlling blocker is no longer the Phase 5 backend-recovery seam; `docs/notes/advanced_color_library_foundation_phase5_root_basin_pair_runtime_PHASED_PLAN.md` closed that path at `b596af6`. The next missing authority is Grading: `ui_app/src/color_pipeline_core.h` already carries descriptor-level grade identities for `contrast_lift`, `phase_finish`, and `band_finish`, `KernelParams` plus `ui_app/src/escape_time_coloring.h` already carry the legacy grading owner surface, and the main Color panel still exposes one live grading authority. But the advanced editor still has no `grading` lane, and only `contrast_lift` currently round-trips through the reusable live import/apply seam. The most falsifiable local hypothesis is that the first truthful Phase 6 slice must promote Grading to a first-class lane while shipping only the first runtime-real grading row, `contrast_lift`, before widening `phase_finish`, `band_finish`, `neutral_finish`, `tone_map_finish`, or `balance_void_grade`. The cheapest disconfirming checks are focused REDs in `ui_app/tests/test_schema_binding.cpp` plus the nearest persistence/reset tests that currently prove the advanced editor still cannot import, apply, or persist a bounded grading lane.

## Presumption Evidence

- `docs/notes/advanced_color_library_foundation_oracle_and_inventory.md` already locks the Grade inventory at `neutral_finish`, `contrast_lift`, `phase_finish`, `band_finish`, `tone_map_finish`, and `balance_void_grade`, so the remaining gap is runtime authority, not naming.
- `ui_app/src/color_pipeline_core.h` already defines `BuildColorPipelineGradeFunctions()` for `contrast_lift`, `phase_finish`, and `band_finish`, which proves Grade identity has started moving into the reusable core.
- `ui_app/src/color_pipeline_core.h` still returns only `source`, `shape`, and `palette` from `GetColorPipelineLaneCatalogs()`, which hard-proves the advanced editor still cannot surface Grading as a first-class lane.
- `ui_app/src/color_pipeline_core.h` only wires `contrast_lift` through `ImportSupportedColorPipelineParamsFromLive(...)` and `ApplySupportedColorPipelineRowParamsToLive(...)`, while `phase_finish` and `band_finish` descriptors exist without equivalent live owner/apply support.
- `ui_app/src/escape_time_coloring.h` only applies extra preset-specific grading behavior for `ColorGradingPreset::escape_default`, which makes `contrast_lift` the first truthful shipped grading row and keeps the wider Grade library explicitly deferred.

## Proof Ledger

- Landed predecessor: `docs/notes/advanced_color_library_foundation_phase5_root_basin_pair_runtime_PHASED_PLAN.md` closed at `b596af6` / `ck:6460d382`, so Phase 5 no longer blocks Grading promotion.
- Planned: `ui_app/src/color_pipeline_core.h` and `ui_app/src/color_pipeline_window.h` will promote a bounded grading lane into the reusable catalog and live bridge while preserving the legacy grading mirror as the only runtime authority.
- Planned: the shipped grading catalog will start with `contrast_lift` only; `phase_finish`, `band_finish`, `neutral_finish`, `tone_map_finish`, and `balance_void_grade` stay deferred until each row has real live import/apply/runtime proof.
- Planned: `ui_app/src/diagnostics_state_io.cpp`, `ui_app/src/diagnostics_capture.cpp`, `ui_app/src/runtime_reset.cpp`, and the nearest tests will keep the grading lane truthful instead of leaving it as a window-only presentation layer.

## Hostile Audit

- Status: complete
- Required posture: assume the eventual grading-lane implementation will either expose draft-only Grade rows or create a second grading authority until the resumed runtime-backed slice proves otherwise.

## Audit Passes

- [deferred] Pass 1 - resume this when the focused grading REDs are rerun after the workflow-hardening stop-line closes.
- [deferred] Pass 2 - resume this when a grading-lane implementation diff exists to inspect for hidden second-authority behavior between the advanced lane and the legacy Color panel.
- [deferred] Pass 3 - resume this when reset/default and diagnostics/archive persistence work lands for the bounded grading lane.

## Audit Findings

- [done] Plan-only repair: the exploratory grading probe was restored instead of checkpointed, so this checkpoint closes only the active-plan deferral/pause repair and leaves the executable grading hostile review deferred until implementation resumes.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`
  - `docs/notes/advanced_color_library_foundation_phase6_grading_runtime_authority_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_library_foundation_phase6_grading_runtime_authority.contract.json`
  - `ui_app/src/color_pipeline_core.h`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/diagnostics_capture.cpp`
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/src/fractal_derived_fields.cpp`
  - `ui_app/src/fractal_types.h`
  - `ui_app/src/runtime_reset.cpp`
  - `ui_app/tests/test_diagnostics_state_io.cpp`
  - `ui_app/tests/test_escape_time_coloring.cpp`
  - `ui_app/tests/test_finding_archive_actions.cpp`
  - `ui_app/tests/test_runtime_reset.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals for this slice:
  - do not redefine the mapped Source / Shape / Palette / Grading inventory
  - do not ship non-runtime Grade rows just because they are named in the inventory
  - do not implement `balance_void_grade` in the first grading-authority slice
  - do not reopen the closed Phase 5 backend-recovery thread
  - do not create a second grading authority outside the legacy Color-panel mirror

## Resume Point

After the workflow-hardening stop-line closes, resume by rerunning the preserved focused REDs in `ui_app/tests/test_schema_binding.cpp` and the nearest grading owner tests that prove the advanced editor still lacks a grading lane and cannot yet round-trip the bounded `contrast_lift` grading surface through live import, apply, reset, and diagnostics/archive persistence.