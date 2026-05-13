# Advanced Color Library Foundation Phase 8A - Basin-Default Grading Lane Retention

## Current Phase

Phase 3 - native helper proof, hostile audit, runtime publish, and published-runtime proof are green for basin-default grading lane retention. Checkpoint commit plus machine receipts remain before any follow-up grading row. This sub-slice still owns only the `basin_default` grading-owner truth for bounded root-basin tuples: live bridge import/apply, diagnostics/archive persistence, reset/default behavior, and runtime truth must stay aligned without widening into `neutral_finish`, `tone_map_finish`, `grade.glow`, Balance/Void, archive UX/recovery, or generic grading rewrites.

## Phase Checklist

- [x] Phase 1 - add focused REDs that prove `basin_default` is still dropped or hidden across the Grading lane, live snapshot import, draft apply, diagnostics/archive persistence, and reset/default seams while root-basin pair behavior stays intact
- [x] Phase 2 - land the smallest truthful owner path for `basin_default` lane retention across core/window/runtime/persistence seams without exposing unsupported controls or widening into other Grading rows
- [ ] Phase 3 - validate, hostile-audit, runtime-publish, write receipts, and checkpoint the basin-default slice cleanly before considering any follow-up grading row

## Explicit User Asks

- [done] Repair basin-default grading lane retention truthfully end to end for bounded root-basin tuples.
- [done] Preserve root-basin pair behavior for `root_index` + `root_classic_palette` and `root_index` + `joy_root_palette`.
- [done] Keep the slice bounded to basin-default lane retention first; do not start `neutral_finish` or `tone_map_finish` unless this slice closes cleanly with commit plus receipts.
- [done] Keep `grade.glow`, `balance_void_grade`, ExplainO-BalanceVoid, ExplainO-all, manual archive recovery, archive/viewer fallback UX, Source composition, and workflow tooling out of scope.

## Presumption Loop

The controlling risk is a split authority: bounded root-basin tuples still carry `ColorGradingPreset::basin_default`, but the shipped Grading catalog, live bridge, and Grading-stack persistence only recognize `contrast_lift`, `phase_finish`, and `band_finish`. The falsifiable hypothesis for this slice is narrow: a parameterless, runtime-real `basin_default` grading row can retain truthful Grading-lane state for bounded root-basin tuples without widening the shipped Grading controls or disturbing the separate root-basin Source/Palette schedule.

## Presumption Evidence

- `ui_app/src/color_pipeline_core.h` maps `root_index + root_classic_palette` and `root_index + joy_root_palette` to `ColorGradingPreset::basin_default`, but `AdvancedColorGradingFunctionId`, `TryParseAdvancedColorGradingFunctionId`, `BuildColorPipelineGradeFunctions()`, and `IsColorPipelineFunctionRuntimeBacked("grading", ...)` do not expose any `basin_default` grading row.
- `ui_app/src/color_pipeline_window.h` only imports/applies Grading rows through `TryBuildColorPipelineGradingStackEntryFromRow`, `ImportSupportedColorPipelineParamsFromGradingStackEntry`, and `TryBuildColorPipelineGradingLaneFromLive`, all of which currently allow only `contrast_lift`, `phase_finish`, and `band_finish`.
- `ui_app/src/diagnostics_state_io.cpp` parses `color_grading_stack` only for the three shipped Grading rows and mirrors legacy grading only from `escape_default`, `phase_default`, and `bands_default`.
- `ui_app/src/escape_time_coloring.h` still applies truthful default grading for basin tuples when no grading stack is present, so the missing truth appears to be lane ownership and persistence rather than missing runtime math for the default basin pass itself.
- `ui_app/tests/test_color_pipeline_window.cpp`, `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp`, `ui_app/tests/test_finding_archive_actions.cpp`, and `ui_app/tests/test_runtime_reset.cpp` already prove bounded root-basin pair ownership and persistence, but none yet lock a visible, truthful `basin_default` Grading-lane row.

## Proof Ledger

- Predecessor checkpoint: `ck:679b2bc3` closed ordered Grading stack composition for `contrast_lift`, `phase_finish`, and `band_finish` only.
- Predecessor checkpoint: `ck:44faf037` closed Palette blend stack without claiming basin-default Grading-lane retention.
- Predecessor checkpoint: `ck:5972173a` closed generic Source weighted blend and left remaining Grading owner proofs explicitly deferred.
- Session diagnosis: the current branch `feature/advanced-color-pipeline-draft-editor-reframe` is clean at `HEAD=5e93e48`, but the active locked contract is still the closed Source weighted-blend slice, so a new basin-default grading-owner sub-slice is required before mutation.
- Session diagnosis: `ColorGradingPreset::basin_default` still lacks an advanced Grading function mapping while bounded root-basin tuples continue to persist the enum in runtime/capture state.
- Focused proof: `artifacts/verify_native_helper_tests_basin_default_rerun3.log` is green after landing the parameterless `basin_default` grading owner path and repairing the adjacent schema-binding proofs that still modeled the old three-row grading set.
- Focused proof: `artifacts/code_quality_report_basin_default.json` passed `tools/code_quality_audit.py --check-baseline` after the same repair set.
- Focused proof: `artifacts/verify_runtime_publish_basin_default.log` published the active runtime successfully to `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe` for this basin-default closure slice.
- Focused proof: `artifacts/verify_runtime_proof_basin_default.log` is green for the narrowed `tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_explaino_escape_variants.py -k "test_explaino_root_classic_and_joy_palettes_render_distinct_published_runtime_frames or test_explaino_root_palette_shape_changes_published_runtime_frames"` witness, proving published runtime behavior for the Explaino root-basin states that carry `color_grading=basin_default`.

## Hostile Audit

- Status: complete
- Required posture: assume the first repair will either fix only the editor surface, silently regress root-basin pair behavior, or overclaim `neutral_finish` / `tone_map_finish` progress until repaired proof says otherwise.

## Audit Passes

- [done] Pass 1 - re-read the owner seams and confirm the concrete omission: bounded root-basin tuples still carry `basin_default`, but the shipped Grading catalog, live bridge, and Grading-stack persistence have no `basin_default` row mapping.
- [done] Pass 2 - after the REDs and first implementation, the native helper reruns exposed two repaired follow-up gaps: diagnostics load still rejected `basin_default` grading-stack rows, and schema-binding proofs still modeled the old three-row grading set plus three-lane basin snapshots.
- [done] Pass 3 - re-read the repaired state, reran `artifacts/verify_native_helper_tests_basin_default_rerun3.log` plus `artifacts/code_quality_report_basin_default.json`, and confirmed the repaired state cleanly with no additional real defect found in the touched owner seams.

## Audit Findings

- [done] Real defect found: `ColorGradingPreset::basin_default` still has no advanced Grading function mapping, so bounded root-basin tuples can be runtime-valid while the Grading lane drops or cannot re-import that truth.
- [done] Real follow-up issue found and repaired: `ui_app/tests/test_schema_binding.cpp` still asserted the pre-basin-default grading trio and three-lane basin snapshots, and one smooth-escape resync path changed only the legacy tuple while leaving the authoritative grading stack count non-zero. The proof now clears the grading stack before expecting legacy fallback and asserts the four-lane basin snapshot truthfully.

## Notes

- Expected owner files for this bounded slice:
  - `docs/notes/advanced_color_feature_restart_inventory.md`
  - `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`
  - `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md`
  - `docs/notes/advanced_color_library_foundation_phase8a_basin_default_lane_retention_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_library_foundation_phase8a_basin_default_lane_retention.contract.json`
  - `ui_app/src/color_pipeline_core.h`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/fractal_types.h`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/src/diagnostics_capture.cpp`
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/src/runtime_reset.cpp`
  - `ui_app/tests/test_color_pipeline_core.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
  - `ui_app/tests/test_escape_time_coloring.cpp`
  - `ui_app/tests/test_diagnostics_state_io.cpp`
  - `ui_app/tests/test_finding_archive_actions.cpp`
  - `ui_app/tests/test_runtime_reset.cpp`
  - `ui_app/tests/test_color_pipeline_window.cpp`
  - `tests/test_fractal_runtime_explaino_escape_variants.py`
- Non-goals for this bounded slice:
  - do not ship `neutral_finish` or `tone_map_finish`
  - do not widen into `grade.glow` or `balance_void_grade`
  - do not turn basin-default retention into a generic Grading rewrite
  - do not weaken root-basin pair behavior or fake unsupported controls

## Resume Point

Resume from phased-plan sync, hostile-audit validation, checkpoint commit, and receipt writing for this bounded basin-default slice. Do not reopen Source weighted-blend archaeology, manual archive recovery, or generic Grading inventory. Do not start `neutral_finish` until this slice closes cleanly with publish proof plus receipts.

## Action Hostile Review

- Action ID: action-20260513-basin-default-reds-10
- Suspected Failure Mode: the repair may preserve only tuple metadata while still dropping the Grading lane, or it may reintroduce a fake control path by treating basin-default as a generic editable Grading row.
- Correct Owner/Action: add focused REDs first across core/window/persistence/reset seams, then land the smallest parameterless `basin_default` grading-owner path that preserves bounded root-basin pair behavior without widening into other Grading rows.
- Proof Surface: focused native helper tests for the touched owner seams, runtime publish, published runtime proof for root-basin behavior, contract validation, phased-plan sync, and hostile-audit validation.
- Outcome: native helper proof, code-quality audit, runtime publish, and published-runtime proof are green for the landed basin-default owner path; checkpoint commit and machine receipts remain.
- Blocked Action: none.
