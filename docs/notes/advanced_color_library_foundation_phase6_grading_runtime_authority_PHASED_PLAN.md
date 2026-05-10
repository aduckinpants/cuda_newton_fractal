# Advanced Color Library Foundation Phase 6 - Grading Runtime Authority

## Current Phase

Phase 4 in progress - the non-runtime fail-closed grading slice now lands in the working tree: coherent phase/bands tuples stay observable but draft-only, the shipped escape-default path still applies, and the runtime-profile rails are green; hostile-audit closure and checkpoint bookkeeping remain

## Phase Checklist

- [x] Phase 1 - add focused REDs that prove Grading is absent from the advanced lane catalogs/live snapshots and that bounded `contrast_lift` grading cannot yet round-trip as a first-class advanced lane
- [x] Phase 2 - promote a bounded grading lane into the reusable catalog/window live bridge while keeping legacy main-panel grading as the one authoritative mirror
- [x] Phase 3 - ship only runtime-real grading rows through that lane, starting with `contrast_lift` and leaving wider Grade inventory rows fail-closed until their runtime math exists
- [ ] Phase 4 - keep reset/defaults plus diagnostics/archive persistence truthful for the bounded grading surface, then validate, hostile-audit, and checkpoint the slice cleanly

## Explicit User Asks

- [done] Phase 5 - keep moving on the defined advanced-color path instead of circling on already-closed work.
- [done] Phase 5-7 - keep the reusable color pipeline generic and separately owned instead of baking window-local exceptions into the product story.
- [active] Phase 6 - resumed with the bounded grading-category promotion: `contrast_lift` now lands as the first shipped Grading row through the core/window/persistence/headless seams while wider Grade rows stay deferred.
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
- Checkpointed: `110cd4c` / `ck:61776381` landed the bounded `contrast_lift` grading lane through the reusable catalog, live snapshot, apply, persistence, render, and headless seams while preserving the legacy grading mirror as the only runtime authority.
- Checkpointed: the shipped grading catalog still starts with `contrast_lift` only; `phase_finish`, `band_finish`, `neutral_finish`, `tone_map_finish`, and `balance_void_grade` remain deferred because no wider live import/apply/runtime proof exists yet.
- Landed in working tree: `ui_app/src/color_pipeline_window.h` now fail-closes coherent phase/bands tuples at both the live-snapshot and draft-selection/apply gates when their grading row is not shipped, while still allowing escape-default tuples to use the shipped `contrast_lift` default from older three-lane draft states.
- Landed in working tree: `ui_app/src/diagnostics_state_io.cpp` now restores coherent draft-only tuples against valid-but-not-import-supported live snapshots instead of rejecting saved drafts outright.
- Validated in working tree: `py -3.14 tools/viewer_host_run_logged_command.py --label phase6-grading-failclosed-bands-proof --log artifacts/phase6_grading_failclosed_bands_proof.log -- ui_app/build_tests_vsdevcmd.cmd`, `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json`, `py -3.14 tools/viewer_host_run_logged_command.py --label phase6-grading-failclosed-runtime-publish --log artifacts/phase6_grading_failclosed_runtime_publish.log -- cmd /c ui_app\build_vsdevcmd.cmd`, and `py -3.14 tools/viewer_host_run_logged_command.py --label phase6-grading-failclosed-runtime-pytest --log artifacts/phase6_grading_failclosed_runtime_pytest.log -- py -3.14 tools/viewer_host_runtime_pytest_lane.py` are green for this slice.

## Hostile Audit

- Status: complete
- Required posture: assume the eventual grading-lane implementation will either expose draft-only Grade rows or create a second grading authority until the resumed runtime-backed slice proves otherwise.

## Audit Passes

- [done] Pass 1 - added focused schema-binding REDs proving coherent phase/bands tuples still imported through the old source/palette bridge even though the grading row was not shipped, then repaired the live-snapshot and draft-selection gates.
- [done] Pass 2 - inspected the fail-closed repair for hidden second-authority behavior and found one omission: the first hostile pass had removed direct banded coverage when repointing old supported-non-basin tests, so an explicit iteration-bands fail-closed regression was added before wider validation.
- [done] Pass 3 - reran the native helper rail plus the runtime-profile validation steps, then re-read the touched seams before checkpoint closure.

## Audit Findings

- [done] Plan-only repair: the exploratory grading probe was restored instead of checkpointed, so this checkpoint closes only the active-plan deferral/pause repair and leaves the executable grading hostile review deferred until implementation resumes.
- [done] Current slice repair: the hostile reread caught contract-scope drift after the bounded grading slice touched `ui_app/tests/test_finding_state_actions.cpp` and `ui_app/tests/test_headless_modes.cpp`; the checked-in contract now includes those paths and has been re-locked.
- [done] Current slice repair: the hostile reread found that the first fail-closed pass had no direct iteration-bands regression after the old supported-banded coverage was repointed to smooth-escape; the missing regression was added and validated.
- [done] Current slice repair: the active contract wording was still describing the earlier lane-promotion slice, so the checked-in contract was refreshed to the current fail-closed grading work before checkpoint closure.

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

After checkpointing this fail-closed slice, resume Phase 6 by deciding whether the grading lane should remain visibly first-class even across basin-default imports or whether that broader lane-retention cleanup belongs to a later bounded slice.