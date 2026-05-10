# Advanced Color Library Foundation Phase 6 - Grading Runtime Authority

## Current Phase

Phase 5 pending - the `phase_finish` runtime-authority slice is closed, and the next grading-authority decision is now explicitly blocked behind the deterministic scoped `salt_ndepend` coverage gate tracked in `docs/notes/ui_integration_harness_completion_PHASED_PLAN.md`.

## Phase Checklist

- [x] Phase 1 - add focused REDs that prove Grading is absent from the advanced lane catalogs/live snapshots and that bounded `contrast_lift` grading cannot yet round-trip as a first-class advanced lane
- [x] Phase 2 - promote a bounded grading lane into the reusable catalog/window live bridge while keeping legacy main-panel grading as the one authoritative mirror
- [x] Phase 3 - ship only runtime-real grading rows through that lane, starting with `contrast_lift` and leaving wider Grade inventory rows fail-closed until their runtime math exists
- [x] Phase 4 - keep reset/defaults plus diagnostics/archive persistence truthful for the bounded grading surface, then validate, hostile-audit, and checkpoint the slice cleanly
- [ ] Phase 5 - after the deterministic scoped `salt_ndepend` coverage gate closes, choose the next honest grading-authority thread: `band_finish` owner proof or the separate Balance/Void track

## Explicit User Asks

- [done] Phase 5 - keep moving on the defined advanced-color path instead of circling on already-closed work.
- [done] Phase 5-7 - keep the reusable color pipeline generic and separately owned instead of baking window-local exceptions into the product story.
- [done] Phase 6 - `contrast_lift` and `phase_finish` now land as the shipped Grading rows through the reusable core/window seams while wider Grade rows stay deferred.
- [deferred] Phase 6 - defer basin-default grading-lane retention for now; do not blur that UI cleanup into the current runtime-authority slice.
- [deferred] Phase 6 - deferred beyond the bounded grading-lane truth slice; Balance/Void stays separate from geometry work and will not widen until the simpler grading lane is proven end-to-end.
- [open] Repo gate - keep further grading expansion blocked until the deterministic scoped `salt_ndepend` coverage program is explicit enough to replace the earlier generic harness stop-line.

## Presumption Loop

The controlling blocker is no longer whether Grading exists at all or which row should widen next in isolation. The bounded `contrast_lift` lane closed at `110cd4c`, the fail-closed follow-up closed at `23b8f55`, and the `phase_finish` widening closed at `07f332a4`; however, the user has now raised a stronger cross-cutting quality gate. The next falsifiable repair for this subplan is therefore explicit pause, not immediate row selection: it must stop presenting `band_finish` or Balance/Void as executable until `docs/notes/ui_integration_harness_completion_PHASED_PLAN.md` closes the deterministic scoped `salt_ndepend` coverage gate. The cheapest disconfirming checks are the same deterministic planning rails plus the sibling stop-line plan that now owns the coverage-program pivot.

## Presumption Evidence

- `docs/notes/advanced_color_library_foundation_oracle_and_inventory.md` already locks the Grade inventory at `neutral_finish`, `contrast_lift`, `phase_finish`, `band_finish`, `tone_map_finish`, and `balance_void_grade`, so the remaining gap is runtime authority, not naming.
- `ui_app/src/color_pipeline_core.h` already defined `BuildColorPipelineGradeFunctions()` for `contrast_lift`, `phase_finish`, and `band_finish`, which made `phase_finish` the smallest truthful next widening step once the first grading lane existed.
- `ui_app/src/fractal_types.h` plus `ui_app/src/escape_time_coloring.h` already carried the live generic phase grading owners through `color_saturation` and `color_contrast`, so `phase_finish` could widen honestly without inventing a second grading authority.
- `ui_app/src/color_pipeline_core.h` now ships `phase_finish` through `GetColorPipelineLaneCatalogs()`, `ImportSupportedColorPipelineParamsFromLive(...)`, and `ApplySupportedColorPipelineRowParamsToLive(...)`, while `band_finish` still lacks an equivalent proof for its `grade.glow` surface.
- `ui_app/src/color_pipeline_core.h` still has no grading mapping for `ColorGradingPreset::basin_default`, which is why basin lane-retention remains a separate deferred cleanup instead of being smuggled into this slice.

## Proof Ledger

- Checkpointed: `07f332a4` closed the `phase_finish` runtime-authority slice; it is no longer a live working-tree-only follow-up.
- Planning gate: further grading-authority work is now explicitly blocked by `docs/notes/ui_integration_harness_completion_PHASED_PLAN.md` until the deterministic scoped `salt_ndepend` coverage program exists.
- Landed predecessor: `docs/notes/advanced_color_library_foundation_phase5_root_basin_pair_runtime_PHASED_PLAN.md` closed at `b596af6` / `ck:6460d382`, so Phase 5 no longer blocks Grading promotion.
- Checkpointed: `110cd4c` / `ck:61776381` landed the bounded `contrast_lift` grading lane through the reusable catalog, live snapshot, apply, persistence, render, and headless seams while preserving the legacy grading mirror as the only runtime authority.
- Checkpointed: `23b8f55` / `ck:10646574` kept coherent `phase_default` and `bands_default` tuples observable-but-draft-only while the matching grading rows were still unshipped.
- Landed in working tree: `ui_app/src/color_pipeline_core.h` now ships `phase_finish` as the second runtime-real Grading row, mirrors it through the existing `color_saturation` / `color_contrast` legacy owners, and aligns the row defaults with the current phase runtime defaults.
- Landed in working tree: `ui_app/src/color_pipeline_window.h` now auto-completes supported phase source/palette selections onto `phase_finish`, treats its grading params as live-backed controls, and leaves `band_finish` plus basin lane-retention deferred.
- Landed in working tree: `ui_app/tests/test_schema_binding.cpp` now locks phase catalog visibility, live snapshot import, tuple auto-complete, draft apply, render behavior, and direct-control sync in the four-lane world while preserving fail-closed coverage for `band_finish` and the deferred basin cleanup.
- Validated in working tree: `artifacts/verify_native_helper_tests_phase_finish_auditfix.log` is green for `ui_app/build_tests_vsdevcmd.cmd`, `artifacts/code_quality_report.json` stayed at the `97/100` baseline, `artifacts/phase6_phase_finish_runtime_publish_auditfix.log` republished the active runtime cleanly, and `artifacts/phase6_phase_finish_runtime_pytest_auditfix.log` reports `64 passed` against the repaired published runtime.

## Hostile Audit

- Status: complete
- Required posture: the repaired `phase_finish` widening cleared the hostile-audit bar only after finding and fixing a real grading-row reset regression, then rerunning the native and runtime/profile rails on that repaired state.

## Audit Passes

- [done] Pass 1 - added focused schema-binding REDs proving coherent phase/bands tuples still imported through the old source/palette bridge even though the grading row was not shipped, then repaired the live-snapshot and draft-selection gates.
- [done] Pass 2 - inspected the fail-closed repair for hidden second-authority behavior and found one omission: the first hostile pass had removed direct banded coverage when repointing old supported-non-basin tests, so an explicit iteration-bands fail-closed regression was added before wider validation.
- [done] Pass 3 - reran the native helper rail plus the runtime-profile validation steps, then re-read the touched seams before checkpoint closure.
- [done] Pass 4 - flipped the phase tuple schema-binding coverage from fail-closed to live-backed, widened the core/window seams for `phase_finish`, and kept iterating on adjacent stale expectations until the focused native helper rail returned green.
- [done] Pass 5 - reread the landed diff distrust-first, found the grading-row reset regression in the new auto-switch helper, added the preserving-contrast_lift regression first, repaired the helper, and reran the native plus runtime/profile rails on the repaired state.

## Audit Findings

- [done] Plan-only repair: the exploratory grading probe was restored instead of checkpointed, so this checkpoint closes only the active-plan deferral/pause repair and leaves the executable grading hostile review deferred until implementation resumes.
- [done] Current slice repair: the hostile reread caught contract-scope drift after the bounded grading slice touched `ui_app/tests/test_finding_state_actions.cpp` and `ui_app/tests/test_headless_modes.cpp`; the checked-in contract now includes those paths and has been re-locked.
- [done] Current slice repair: the hostile reread found that the first fail-closed pass had no direct iteration-bands regression after the old supported-banded coverage was repointed to smooth-escape; the missing regression was added and validated.
- [done] Current slice repair: the active contract wording was still describing the earlier lane-promotion slice, so the checked-in contract was refreshed to the current fail-closed grading work before checkpoint closure.
- [done] Current slice repair: the first `phase_finish` REDs surfaced stale schema-binding assumptions that were carrying old Shape-stack and shared live-tuple state across adjacent blocks; the harness now resets that state explicitly so the phase tuple assertions measure the real runtime-backed behavior.
- [done] Current slice repair: the first `phase_finish` auto-switch helper reset the active grading row even when the required Grading function had not changed, which would have wiped user-tuned `contrast_lift` values on ordinary escape tuple switches; the helper now preserves the row when the function id already matches, and schema-binding coverage locks that regression.

## Notes

- Expected owner files for the next bounded slice after the coverage gate closes:
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
- Non-goals for the next bounded slice:
  - do not redefine the mapped Source / Shape / Palette / Grading inventory
  - do not ship non-runtime Grade rows just because they are named in the inventory
  - do not implement `balance_void_grade` before the deterministic scoped `salt_ndepend` coverage gate closes
  - do not reopen the closed Phase 5 backend-recovery thread
  - do not create a second grading authority outside the legacy Color-panel mirror

## Resume Point

After checkpointing this `phase_finish` slice, keep basin lane-retention deferred and resume Phase 6 with the next bounded grading-authority decision: either prove `band_finish` through a real `grade.glow` owner surface or switch deliberately to the separately planned Balance/Void operator track.