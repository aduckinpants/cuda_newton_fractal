# Advanced Color Library Foundation Phase 6 - Grading Runtime Authority

## Current Phase

Phase 5 complete - the deterministic scoped `salt_ndepend` coverage gate closed at `ck:29fa6c71`, the bounded `band_finish` owner proof is implemented, and repaired-state validation proves the published runtime plus physical slider path after the rebuild. `band_finish` now ships only through real legacy grading owner fields; `grade.glow`, Balance/Void, and basin lane-retention remain deferred.

## Phase Checklist

- [x] Phase 1 - add focused REDs that prove Grading is absent from the advanced lane catalogs/live snapshots and that bounded `contrast_lift` grading cannot yet round-trip as a first-class advanced lane
- [x] Phase 2 - promote a bounded grading lane into the reusable catalog/window live bridge while keeping legacy main-panel grading as the one authoritative mirror
- [x] Phase 3 - ship only runtime-real grading rows through that lane, starting with `contrast_lift` and leaving wider Grade inventory rows fail-closed until their runtime math exists
- [x] Phase 4 - keep reset/defaults plus diagnostics/archive persistence truthful for the bounded grading surface, then validate, hostile-audit, and checkpoint the slice cleanly
- [x] Phase 5 - ship the next honest grading-authority row: `band_finish` owner proof using only real runtime-backed controls while Balance/Void, `grade.glow`, and basin lane-retention remain deferred

## Explicit User Asks

- [done] Phase 5 - keep moving on the defined advanced-color path instead of circling on already-closed work.
- [done] Phase 5-7 - keep the reusable color pipeline generic and separately owned instead of baking window-local exceptions into the product story.
- [done] Phase 6 - `contrast_lift` and `phase_finish` landed as shipped Grading rows through the reusable core/window seams while wider Grade rows stayed deferred.
- [deferred] Phase 6 - defer basin-default grading-lane retention for now; do not blur that UI cleanup into the current runtime-authority slice.
- [deferred] Phase 6 - defer Balance/Void beyond this bounded `band_finish` row; it stays separate from geometry work and will not widen until the simpler grading lane is proven end-to-end.
- [done] Repo gate - the deterministic scoped `salt_ndepend` coverage program closed at `ck:29fa6c71` with `78 covered / 0 uncovered` and `freeze_ready=true`, so grading expansion may resume only under hostile-reviewed bounded slices.
- [done] Phase 6 - finish the next visible advanced-color feature gap by making the iteration-bands tuple live-backed in the advanced editor with `band_finish`, without exposing `grade.glow` or any other fake control.

## Presumption Loop

The controlling blocker is now the first remaining visible-but-unshipped grading row, not the coverage gate. The bounded `contrast_lift` lane closed at `110cd4c`, the fail-closed follow-up closed at `23b8f55`, the `phase_finish` widening closed at `07f332a4`, the Phase 5 root-basin pair runtime slice closed at `b596af6`, and the deterministic coverage gate closed at `ck:29fa6c71`. The remaining falsifiable risk is that `band_finish` could be shipped by simply allowing its old raw descriptor through the catalog even though `grade.glow` has no runtime owner. The bounded hypothesis for this slice is narrower: if `band_finish` uses the same legacy grading mirror fields that already drive the shipped phase/default grading path, the iteration-bands tuple can become live-backed without inventing `glow`, without creating a second grading authority, and without reopening Balance/Void. The cheapest disconfirming checks are focused REDs in `ui_app/tests/test_color_pipeline_core.cpp`, `ui_app/tests/test_color_pipeline_window.cpp`, and `ui_app/tests/test_schema_binding.cpp`, followed by runtime publish and physical Color Pipeline slider proof against the active `D:` executable.

## Presumption Evidence

- `docs/notes/advanced_color_library_foundation_oracle_and_inventory.md` already locks the Grade inventory at `neutral_finish`, `contrast_lift`, `phase_finish`, `band_finish`, `tone_map_finish`, and `balance_void_grade`, so the remaining gap is runtime authority, not naming.
- `ui_app/src/color_pipeline_core.h` already defined `BuildColorPipelineGradeFunctions()` for `contrast_lift`, `phase_finish`, and `band_finish`; the earlier raw `band_finish` descriptor exposed unowned `grade.glow`, which is exactly why this slice must use a narrower owner-backed descriptor.
- `ui_app/src/fractal_types.h` plus `ui_app/src/escape_time_coloring.h` already carry the live generic grading owners through `color_saturation` and `color_contrast`, so `band_finish` can widen honestly only if it mirrors those same owners instead of inventing new state.
- `ui_app/src/color_pipeline_core.h` maps `ColorGradingPreset::bands_default` to `band_finish`; before this slice it still filtered that row out of `GetColorPipelineLaneCatalogs()` because `IsColorPipelineFunctionRuntimeBacked("grading", "band_finish")` returned false.
- `ui_app/tests/test_schema_binding.cpp` locked coherent iteration-bands tuples as observable-but-draft-only until the matching grading row shipped, so that test surface is the correct RED/GREEN owner for this slice.
- `ui_app/src/color_pipeline_core.h` still has no grading mapping for `ColorGradingPreset::basin_default`, which is why basin lane-retention remains a separate deferred cleanup instead of being smuggled into this slice.

## Proof Ledger

- Checkpointed: `110cd4c` / `ck:61776381` landed the bounded `contrast_lift` grading lane through the reusable catalog, live snapshot, apply, persistence, render, and headless seams while preserving the legacy grading mirror as the only runtime authority.
- Checkpointed: `23b8f55` / `ck:10646574` kept coherent `phase_default` and `bands_default` tuples observable-but-draft-only while the matching grading rows were still unshipped.
- Checkpointed: `07f332a4` closed the `phase_finish` runtime-authority slice; it is no longer a live working-tree-only follow-up.
- Checkpointed: `b596af6` / `ck:6460d382` closed the Phase 5 root-basin pair runtime slice, so the backend honesty blocker no longer blocks Grading promotion.
- Checkpointed: `ck:29fa6c71` closed the deterministic scoped `salt_ndepend` coverage gate with `78 covered / 0 uncovered` and `freeze_ready=true`, so the old coverage stop-line no longer blocks bounded Phase 6 work.
- Runtime witness before implementation: `artifacts/verify_color_pipeline_physical_drag.log` proved the freshly published physical Color Pipeline slider drag path against the active `D:` executable before widening `band_finish`.
- RED/GREEN in working tree: `ui_app/tests/test_color_pipeline_core.cpp`, `ui_app/tests/test_color_pipeline_window.cpp`, and `ui_app/tests/test_schema_binding.cpp` now require `band_finish` to be runtime-backed only through `grade.saturation` and `grade.contrast`.
- Landed in working tree: `ui_app/src/color_pipeline_core.h` now ships `band_finish` as the third runtime-real Grading row, imports/applies it through `color_saturation` and `color_contrast`, and keeps `grade.glow` absent.
- Validated so far: `artifacts/band_finish_native_green.log` captured `ui_app/build_tests_vsdevcmd.cmd` exiting 0 with all helper tests passing after the `band_finish` repair.
- Repaired-state validation: `artifacts/code_quality_report.json` stayed on the `97/100` baseline; `artifacts/verify_runtime_publish.log` exited 0 and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`; `artifacts/verify_color_pipeline_physical_drag.log` reports `1 passed, 3 deselected`; `artifacts/validation/advanced_color_library_foundation_phase6_grading_runtime_authority_contract.json` has `checks.contract_schema_valid=true`; `artifacts/validation/viewer_host_assert_phased_plan_sync.json` was regenerated after plan repair; and `artifacts/band_finish_native_green.log` ends with `All helper tests passed`.

## Hostile Audit

- Status: complete
- Required posture: the repaired `band_finish` implementation cleared hostile review only after finding the fake `grade.glow` risk, repairing the stale dedicated-window expectation, repairing the plan/hook process defect, then rerunning the native, code-quality, runtime publish, physical slider, contract, and plan-sync proof surfaces on the repaired state.

## Audit Passes

- [done] Pass 1 - found the real product defect: the raw `band_finish` catalog row still carried unowned `grade.glow`, so simply unfiltering it would have shipped a fake control. The repair narrows `band_finish` to the real `color_saturation` / `color_contrast` mirror.
- [done] Pass 2 - found the stale dedicated-window expectation: `test_color_pipeline_window` still expected coherent iteration-bands tuples to fail closed after `band_finish` shipped. The repaired test now proves live import, reset-from-live, and out-of-sync failure behavior.
- [done] Pass 3 - found the workflow/process defect after the hook trip: the plan had stale coverage-gate language, literal `` `n`` text, and no current Action Hostile Review. This patch repairs the plan before more validation or closure.
- [done] Pass 4 - after plan repair, reran code quality, runtime publish, physical slider proof, contract validation, and phased-plan sync on the repaired state; re-read the repaired state and found no additional real defect, no additional real issue, and no additional workflow mistake beyond the recorded repairs.

## Audit Findings

- [done] Real defect found: shipping the old raw `band_finish` descriptor would expose `grade.glow` without runtime ownership. The working tree removes that fake control from the shipped row and maps `band_finish` to existing owner-backed saturation/contrast controls.
- [done] Real defect found: the dedicated `color_pipeline_window` test still encoded the previous fail-closed behavior for coherent iteration-bands tuples. The working tree now locks the new live-backed behavior through the module-named window helper.
- [done] Real defect found: the slice plan was stale/corrupted after the hook recovery, including literal `` `n`` artifacts and a coverage-gate stop line that was already closed at `ck:29fa6c71`. The active plan is being repaired before validation receipts and checkpoint closure.
- [done] Clean re-audit evidence: repaired-state proof is recorded in `artifacts/code_quality_report.json`, `artifacts/verify_runtime_publish.log`, `artifacts/verify_color_pipeline_physical_drag.log`, `artifacts/validation/advanced_color_library_foundation_phase6_grading_runtime_authority_contract.json`, `artifacts/validation/viewer_host_assert_phased_plan_sync.json`, and `artifacts/band_finish_native_green.log`; the re-read the repaired state pass did not expose another defect.

## Notes

- Expected owner files for this bounded slice:
  - `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`
  - `docs/notes/advanced_color_library_foundation_phase6_grading_runtime_authority_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_library_foundation_phase6_grading_runtime_authority.contract.json`
  - `ui_app/src/color_pipeline_core.h`
  - `ui_app/tests/test_color_pipeline_core.cpp`
  - `ui_app/tests/test_color_pipeline_window.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals for this bounded slice:
  - do not redefine the mapped Source / Shape / Palette / Grading inventory
  - do not ship non-runtime Grade rows just because they are named in the inventory
  - do not expose `grade.glow` until a runtime owner exists
  - do not implement `balance_void_grade` in the `band_finish` owner-proof slice
  - do not reopen the closed Phase 5 backend-recovery thread
  - do not create a second grading authority outside the legacy Color-panel mirror

## Resume Point

Resume after the `band_finish` checkpoint by keeping basin lane-retention, `grade.glow`, `neutral_finish`, `tone_map_finish`, and Balance/Void deferred until one of those surfaces gets a separate owner-proof contract.

## Action Hostile Review

- Action ID: action-20260511-foundation-final-proof-ledger-sync
- Suspected Failure Mode: the main foundation proof ledger can still say final closure is pending after the repaired-state validation rails have passed, leaving stale proof language in the checked-in plan.
- Correct Owner/Action: update the umbrella foundation proof ledger to record the repaired-state `band_finish` validation artifacts while preserving fake-row deferrals and the active Phase 6 ownership boundary.
- Proof Surface: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` plus the active Phase 6 contract validator artifact.
- Blocked Action: any checkpoint, receipt, or final claim that the advanced-color feature is repaired while the foundation proof ledger still says final validation is pending.

