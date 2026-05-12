# Advanced Color Library Foundation Phase 6 - Grading Runtime Authority

## Current Phase

Phase 7 - capture-backed diagnostics save-path repair. Grading stack composition is checkpointed; this slice uses a low-resolution manual `explaino_joy` capture state regression plus high-precision save sentinels to prove diagnostics state load, capture serialization, and reload preserve the real advanced-color Source/Shape/Palette/Grading draft, shape stack, grading stack, sidecar orientation, controller policy, and numeric view/parameter fidelity without committing the 4096x4096 frame. Palette blend math remains the next feature slice after this serialization repair.

## Phase Checklist

- [x] Phase 1 - add focused REDs that prove Grading is absent from the advanced lane catalogs/live snapshots and that bounded `contrast_lift` grading cannot yet round-trip as a first-class advanced lane
- [x] Phase 2 - promote a bounded grading lane into the reusable catalog/window live bridge while keeping legacy main-panel grading as the one authoritative mirror
- [x] Phase 3 - ship only runtime-real grading rows through that lane, starting with `contrast_lift` and leaving wider Grade inventory rows fail-closed until their runtime math exists
- [x] Phase 4 - keep reset/defaults plus diagnostics/archive persistence truthful for the bounded grading surface, then validate, hostile-audit, and checkpoint the slice cleanly
- [x] Phase 5 - ship the next honest grading-authority row: `band_finish` owner proof using only real runtime-backed controls while Balance/Void, `grade.glow`, and basin lane-retention remain deferred
- [x] Phase 6 - add REDs and implementation for ordered Grading stack composition using only shipped rows, with stack ownership, live import/apply, persistence, reset, capture/archive, and published-runtime proof
- [x] Phase 7 - add the manual `183932_396__explaino_joy` capture-backed diagnostics serialization round-trip regression using low-resolution test data and repair save-path numeric precision

## Explicit User Asks

- [done] Phase 5 - keep moving on the defined advanced-color path instead of circling on already-closed work.
- [done] Phase 5-7 - keep the reusable color pipeline generic and separately owned instead of baking window-local exceptions into the product story.
- [done] Phase 6 - `contrast_lift` and `phase_finish` landed as shipped Grading rows through the reusable core/window seams while wider Grade rows stayed deferred.
- [deferred] Phase 6 - defer basin-default grading-lane retention for now; do not blur that UI cleanup into the current runtime-authority slice.
- [deferred] Phase 6 - defer Balance/Void beyond this bounded `band_finish` row; it stays separate from geometry work and will not widen until the simpler grading lane is proven end-to-end.
- [done] Repo gate - the deterministic scoped `salt_ndepend` coverage program closed at `ck:29fa6c71` with `78 covered / 0 uncovered` and `freeze_ready=true`, so grading expansion may resume only under hostile-reviewed bounded slices.
- [done] Phase 6 - finish the next visible advanced-color feature gap by making the iteration-bands tuple live-backed in the advanced editor with `band_finish`, without exposing `grade.glow` or any other fake control.
- [done] Composition - start the critical multi-function composition spine before continuing with single-row grading polish.
- [done] Composition - keep Grading stack composition runtime-backed through existing shipped rows only, with no new Grade inventory rows or fake controls.
- [done] Capture-backed serialization - copy the manual `D:\salt-fractal\cuda_newton_fractal_clone\findings\manual_capture\2026-05-11\183932_396__explaino_joy` state into a lower-resolution dedicated unit regression so future state additions have an end-to-end serialization tripwire.
- [done] Save-path bug report - repair diagnostics save serialization so high-precision view/parameter state is not truncated before load has a chance to reproduce the image.

## Presumption Loop

The controlling blocker is now composition authority, not the next unshipped grading row. The bounded `contrast_lift` lane closed at `110cd4c`, `phase_finish` closed at `07f332a4`, `band_finish` closed at `80cc000`, Shape stack composition closed at `e6b55b9`, the Phase 5 root-basin pair runtime slice closed at `b596af6`, and the deterministic coverage gate closed at `ck:29fa6c71`. The remaining falsifiable risk is that the editor can still present multi-row lanes while Grading has no stack owner, so multiple enabled Grading rows may reject, collapse to one legacy field pack, or appear runtime-backed without ordered evaluation. The bounded hypothesis for this slice is narrower: if Grading stack composition uses only existing shipped rows, it can prove the composition spine without exposing `grade.glow`, `neutral_finish`, `tone_map_finish`, Balance/Void, or generic Source/Palette composition.

The new follow-up risk is not missing runtime math; it is that synthetic serialization assertions can miss real capture payload drift as the state surface grows, and that the save path can flatten numeric fidelity before load has a chance to reproduce the image. The bounded hypothesis is that a real manual `explaino_joy` state copied into a low-resolution unit fixture, then saved with high-precision sentinels, can prove load -> capture serialization -> load for advanced-color draft rows, Shape/Grading stacks, sidecar orientation, sidecar policy, and numeric precision without claiming UI/UX proof or adding a high-resolution frame fixture.

## Presumption Evidence

- `docs/notes/advanced_color_library_foundation_oracle_and_inventory.md` already locks the Grade inventory at `neutral_finish`, `contrast_lift`, `phase_finish`, `band_finish`, `tone_map_finish`, and `balance_void_grade`, so the remaining gap is runtime authority, not naming.
- `ui_app/src/color_pipeline_core.h` already defined `BuildColorPipelineGradeFunctions()` for `contrast_lift`, `phase_finish`, and `band_finish`; the earlier raw `band_finish` descriptor exposed unowned `grade.glow`, which is exactly why this slice must use a narrower owner-backed descriptor.
- `ui_app/src/fractal_types.h` plus `ui_app/src/escape_time_coloring.h` already carry the live generic grading owners through `color_saturation` and `color_contrast`, so `band_finish` can widen honestly only if it mirrors those same owners instead of inventing new state.
- `ui_app/src/color_pipeline_core.h` maps `ColorGradingPreset::bands_default` to `band_finish`; before this slice it still filtered that row out of `GetColorPipelineLaneCatalogs()` because `IsColorPipelineFunctionRuntimeBacked("grading", "band_finish")` returned false.
- `ui_app/tests/test_schema_binding.cpp` locked coherent iteration-bands tuples as observable-but-draft-only until the matching grading row shipped, so that test surface is the correct RED/GREEN owner for this slice.
- `ui_app/src/color_pipeline_core.h` still has no grading mapping for `ColorGradingPreset::basin_default`, which is why basin lane-retention remains a separate deferred cleanup instead of being smuggled into this slice.
- `ui_app/src/fractal_types.h` has stack owners for Shape and bounded root-basin pairs, but no `color_grading_stack`, so the first REDs must prove Grading currently lacks truthful multi-row ownership.
- The manual capture `183932_396__explaino_joy/state.json` carries a real four-lane advanced-color draft (`smooth_escape_ramp`, `mirror_repeat`, `explaino_cmap`, `contrast_lift`), `color_shape_stack`, `color_grading_stack`, sidecar orientation hashes, and sidecar controller policy.
- The current tests included synthetic diagnostics/capture assertions, but no fixture-driven round trip from a real manual capture state before this slice.
- The failing repaired test proved `BuildStateJson()` was still using default stream precision, truncating high-precision view and advanced-color numeric state on save.

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


- RED proved: `artifacts/grading_stack_red_native.log` failed before implementation because `KernelParams` had no `color_grading_stack` owner for a two-row Grading draft.
- Landed: `ui_app/src/fractal_types.h` now owns `ColorPipelineGradingRuntimeParams`, `ColorPipelineGradingStackEntry`, `kColorPipelineMaxGradingStackCount`, and `KernelParams::color_grading_stack` / `color_grading_stack_count`.
- Landed: `ui_app/src/color_pipeline_window.h` now applies and imports ordered Grading stacks for the shipped rows `contrast_lift`, `phase_finish`, and `band_finish`, while three-lane basin snapshots can still re-enter default Grading tuples without reading as draft-only.
- Landed: `ui_app/src/escape_time_coloring.h` evaluates the bounded Grading stack in order and falls back to the legacy single-row behavior when no stack exists.
- Landed: `ui_app/src/diagnostics_state_io.cpp`, `ui_app/src/diagnostics_capture.cpp`, `ui_app/src/runtime_reset.cpp`, and `ui_app/src/fractal_derived_fields.cpp` now load/save/capture/default/clear the Grading stack while mirroring the final row back into the legacy numeric grading fields.
- Landed: `ui_app/tests/test_color_pipeline_window.cpp`, `ui_app/tests/test_escape_time_coloring.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp`, `ui_app/tests/test_finding_archive_actions.cpp`, and `ui_app/tests/test_runtime_reset.cpp` lock the live bridge, ordered evaluation, persistence, capture/archive, and reset behavior.
- Repaired-state validation: `artifacts/code_quality_report.json` is back on the 97/100 baseline; `artifacts/grading_stack_final_native_archive_assert.log` ends with `All helper tests passed`; `artifacts/grading_stack_runtime_publish.log` published `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`; and `artifacts/grading_stack_runtime_probe_session_pytest.log` reports `64 passed` against the published runtime.
- RED proved: `artifacts/capture_backed_serialization_precision_red.log` reached the real save path and failed on `manual fixture capture reload: expected manual explaino_joy view fields to survive diagnostics state load`, proving save-path numeric truncation rather than a load-only defect.
- Landed: `ui_app/tests/test_diagnostics_state_io.cpp` now carries a low-resolution manual `183932_396__explaino_joy` state fixture, reloads it, saves it through `CaptureDiagnosticsLastBundle`, reloads the emitted `state.json`, and checks advanced-color draft rows, Shape/Grading stacks, sidecar orientation, sidecar policy, and high-precision view/parameter sentinels.
- Landed: `ui_app/src/diagnostics_capture.cpp` now sets the diagnostics state JSON stream to `std::numeric_limits<double>::max_digits10`, so double and float-backed runtime state survive the save step with round-trip precision.
- Landed: `ui_app/build_tests_vsdevcmd.cmd` links `diagnostics_capture.cpp` and `render_capture_guard.cpp` into the diagnostics state IO test target so that the regression exercises the real save serializer.
- Landed: `ui_app/tests/test_finding_archive_actions.cpp` now expects the widened precise float spellings that the diagnostics serializer intentionally emits after the precision repair.
- Repaired-state validation: `artifacts/capture_backed_serialization_native.log` ends with `All helper tests passed`; `artifacts/code_quality_report.json` stays on the `97/100` baseline; `artifacts/validation/advanced_color_library_foundation_phase6_grading_runtime_authority_contract.json` has `checks.contract_schema_valid=true`; `artifacts/validation/viewer_host_assert_phased_plan_sync.json` has `ok=true`; and `artifacts/validation/advanced_color_library_foundation_phase6_grading_runtime_authority_hostile_audit.json` has `ok=true`.

## Hostile Audit

- Status: complete
- Required posture: treat the capture-backed fixture as a regression harness, not a screenshot trophy; look for false confidence from synthetic-only assertions, high-resolution fixture bloat, missing draft/policy fields, numeric truncation on save, or a round trip that only checks JSON substrings instead of reloading the emitted state.

## Audit Passes

- [done] Pass 1 - RED proved the real missing owner: `ui_app/tests/test_color_pipeline_window.cpp` failed before implementation because `KernelParams` had no `color_grading_stack` for a two-row shipped Grading draft.
- [done] Pass 2 - found the first repaired-state regression: collecting Grading rows before the root-basin early return made three-lane root-basin snapshots read as draft-only. The repaired bridge only validates Grading stacks for tuples that have a Grading lane.
- [done] Pass 3 - found the second repaired-state regression: applying a programmable tuple from a three-lane basin snapshot required a visible Grading lane instead of synthesizing the default shipped Grading row. The repaired apply path tolerates missing Grading lanes for default tuple re-entry.
- [done] Pass 4 - found the code-quality regression: adding Grading stack capture serialization pushed `WriteColorParamsJson()` over the baseline line-count gate. The repaired state extracts `WriteColorPipelineStacksJson()` and code quality returns to 97/100.
- [done] Pass 5 - reran the repaired state through code quality, full native helper tests including the tightened archive assertion, runtime publish, and published-runtime probe/session pytest; no additional real defect found in the touched seams.
- [done] Pass 6 - added the capture-backed diagnostics serialization regression and found the real save-path defect: default `std::ostringstream` precision truncated high-precision view and advanced-color numeric state before reload.
- [done] Pass 7 - repaired the serializer precision, reran the focused native helper path, and found one collateral test assumption that still expected short decimal strings; updated that archive assertion to match the intentionally widened precise float output.
- [done] Pass 8 - reran code quality and contract validation on the repaired state; no additional real defect found in the touched serializer, fixture, archive assertion, or harness-link seams.

## Audit Findings

- [done] Real defect found: Grading composition had no bounded runtime owner, so multi-row Grading either rejected or collapsed through the legacy field pack. The runtime now owns and evaluates a bounded Grading stack.
- [done] Real defect found: the first implementation made root-basin snapshots with no Grading lane read as draft-only. The row selection path now returns root-basin pairs before requiring Grading stack rows.
- [done] Real defect found: the first implementation blocked basin-to-programmable tuple re-entry because apply required a visible Grading lane. The apply path now synthesizes default Grading when the target tuple supports it and the draft has no Grading lane.
- [done] Real defect found: capture serialization grew `WriteColorParamsJson()` past the quality baseline. The stack serialization moved into `WriteColorPipelineStacksJson()` and the audit baseline is green again.
- [done] Clean re-audit evidence: repaired-state proof is recorded in `artifacts/code_quality_report.json`, `artifacts/grading_stack_final_native_archive_assert.log`, `artifacts/grading_stack_runtime_publish.log`, `artifacts/grading_stack_runtime_probe_session_pytest.log`, `artifacts/validation/advanced_color_library_foundation_phase6_grading_runtime_authority_contract.json`, and `artifacts/validation/viewer_host_assert_phased_plan_sync.json`.
- [done] Real defect found: diagnostics capture serialization used default stream precision, so save rounded high-precision view and advanced-color numeric state before load could reproduce the capture. `BuildStateJson()` now emits numeric state with `std::numeric_limits<double>::max_digits10`.
- [done] Real defect found: the diagnostics state IO regression initially called the real save path without linking the save module into the test binary. The native harness now links `diagnostics_capture.cpp` and `render_capture_guard.cpp` into that focused target.
- [done] Real defect found: an existing archive capture assertion encoded the old short-decimal formatting. The assertion now expects the high-precision float representations required by the repaired save path.
- [done] Clean re-audit evidence: repaired-state proof is recorded in `artifacts/capture_backed_serialization_native.log`, `artifacts/code_quality_report.json`, `artifacts/validation/advanced_color_library_foundation_phase6_grading_runtime_authority_contract.json`, `artifacts/validation/viewer_host_assert_phased_plan_sync.json`, and `artifacts/validation/advanced_color_library_foundation_phase6_grading_runtime_authority_hostile_audit.json`.

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

Resume from the completed capture-backed diagnostics save-path repair by running any requested closure validators/receipts, then returning the plan to Palette blend-stack as the next feature slice. Generic Source signal composition, basin lane-retention, `grade.glow`, `neutral_finish`, `tone_map_finish`, and Balance/Void remain deferred.

## Next Known Sliced Work

1. Palette blend stack with explicit RGB blend math, weights/modes, persistence, and viewer proof.
2. Source signal composition with explicit scalar mixer semantics and descriptor-owned backend precision metadata.
3. Basin-default grading lane-retention cleanup, `neutral_finish` / `tone_map_finish`, `grade.glow`, and Balance/Void owner proofs after the composition spine is truthful.
4. Foundation closure: once composition and remaining owner-proof decisions are shipped or explicitly deferred, build the proof matrix, D: gallery/runtime captures, and extension rules.

## Action Hostile Review

- Action ID: action-20260511-capture-backed-state-roundtrip
- Suspected Failure Mode: the test may copy a real capture but only assert superficial JSON substrings, miss numeric truncation on save, miss the actual load -> capture serialize -> reload path, or bloat the repo with the original high-resolution frame.
- Correct Owner/Action: add a low-resolution diagnostics-state regression in `ui_app/tests/test_diagnostics_state_io.cpp` that reloads the emitted capture state and checks the real advanced-color draft, Shape stack, Grading stack, sidecar orientation, controller policy fields, and high-precision save sentinels.
- Proof Surface: focused native helper proof through `ui_app/build_tests_vsdevcmd.cmd`, plus code-quality, contract, plan-sync, and hostile-audit validators.
- Outcome: completed: the regression failed before the serializer repair, the save path now emits round-trip precision, and native/code-quality/contract validation are green.
- Blocked Action: editing the diagnostics serialization regression before this Action ID is recorded and consumed.

