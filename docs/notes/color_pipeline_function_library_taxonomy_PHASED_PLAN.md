# Color Pipeline Function Library Taxonomy

## Current Phase

Closed - taxonomy metadata/reporting implementation, hostile audit, and validation are complete

## Phase Checklist

- [x] Phase 1 - open the checked-in plan/contract and lock the active slice
- [x] Phase 2 - add RED coverage proving current function metadata lacks a first-class taxonomy surface
- [x] Phase 3 - materialize stable function taxonomy from UI-Salt into the active C++ catalog without changing current functions or controls
- [x] Phase 4 - expose taxonomy in no-mouse/report surfaces so future picker/layout work can be tested without physical mouse automation
- [x] Phase 5 - prove current Color Pipeline recipes, SDF source rows, capture/replay authority, and UI-Salt parity stay unchanged
- [x] Phase 6 - hostile audit, plan sync, and closure validation

## Explicit User Asks

- [done] Return to the queued composition-glue work after recording that per-source SDF downsample is deferred.
- [done] Start with the glue needed for a more complete compositional stance, not a broad dump of new function entries.
- [done] Keep the current Color Pipeline UI behavior frozen while using metadata/test proof to make future layout/picker work safer.
- [done] Avoid the old pattern where UI workflow changes are claimed with only narrow helper proof.

## Scope

In scope:

- Add a stable taxonomy/group field to the viewer-local UI-Salt function library contract.
- Classify the existing Color Pipeline functions into useful groups for picker/layout/reporting.
- Load the taxonomy into the materialized C++ catalog and preserve it through active metadata lookup.
- Add tests that prove taxonomy is present, generated, parsed, installed, and reported without changing function ids, params, control ids, compatibility, recipes, or source-stack behavior.
- Add no-mouse runtime proof through the published UI-Salt contract report.

Out of scope:

- New Color Pipeline functions.
- Factorio-style schedule/workflow UI.
- Full preset manager redesign.
- Per-source SDF downsample.
- GPU Color Pipeline postprocess.
- Authored SDF pack UI/live viewport integration.
- SDF-native fractal lanes.
- Salticid runtime dependency.
- Physical mouse automation.

## Proof Ledger

- Start authority: `codex/color-pipeline-function-library-taxonomy` branched from `fb0bdf5`, clean, previous rearward review `ok`.
- Known low-priority limitation carried forward: current `SDF Field Downsample` is one shared `LensSettings::downsample` authority, not per SDF Source row/layer.
- RED: `py -3.14 -m pytest tests/test_ui_salt_materializer.py` failed on missing `taxonomy_group` support and absent generated taxonomy fields.
- GREEN: `color_pipeline_function_library_taxonomy_ui_salt_materializer` passed `tests/test_ui_salt_materializer.py` with 8 tests.
- Native GREEN: `color_pipeline_function_library_taxonomy_core` passed `test_color_pipeline_core` with 2276 passed, 0 failed.
- Native GREEN: `color_pipeline_function_library_taxonomy_window` passed `test_color_pipeline_window` with 200 passed, 0 failed.
- Runtime publish GREEN: `color_pipeline_function_library_taxonomy_publish` staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Runtime no-mouse GREEN: `color_pipeline_function_library_taxonomy_runtime_ui_salt` passed `tests/test_fractal_runtime_ui_salt_contract.py`, including exact taxonomy group count and lane group report assertions.
- SDF preservation GREEN: `color_pipeline_function_library_taxonomy_runtime_sdf_rows` passed `tests/test_fractal_runtime_color_pipeline_sdf_rows.py` and `tests/test_fractal_runtime_capture_replay_authority.py` with 6 passed.
- Contract validation GREEN: `artifacts/validation/color_pipeline_function_library_taxonomy_contract.json` has `checks.contract_schema_valid=true`.
- Plan sync GREEN: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed for this plan.
- Hostile-audit validation GREEN: `artifacts/validation/color_pipeline_function_library_taxonomy_hostile_audit.json` has `ok=true`.
- Code-quality baseline GREEN: `artifacts/validation/color_pipeline_function_library_taxonomy_code_quality.json` passed baseline check.
- Diff check GREEN: `color_pipeline_function_library_taxonomy_diff_check` passed.

## Hostile Audit

- Status: complete
- Required posture: assume this accidentally changes visible function lists, breaks UI-Salt parity, silently permits unsupported combinations, hides an SDF source row, or claims future UI readiness without a no-mouse report surface until focused proof disproves each risk.

## Audit Passes

- [done] Pass 1 - found and repaired stale adapter fallout: `color_pipeline_window.h` still called the old four-argument `MakeColorPipelineFunction` helper, so the window rail caught a real compile break before closeout.
- [done] Pass 2 - found and repaired proof weakness: the runtime UI-Salt report initially accepted `taxonomy_group_count >= 12`; tightened it to the exact current count of 23 and reran the native/runtime taxonomy rails.
- [done] Pass 3 - clean re-read confirmed the repaired state preserves function ids, params, compatibility rows, companion suggestions, recipes, SDF source rows, no physical mouse automation, no new functions, no UI redesign, and no Salticid runtime dependency.

## Audit Findings

- [done] Stale adapter finding: `color_pipeline_window.h` failed to compile after the helper signature changed; fixed by threading taxonomy through the adapter and rerunning `test_color_pipeline_window`.
- [done] Test-fixture finding: tampered JSON negative fixtures were missing the new required `taxonomy_group` field and therefore stopped testing duplicate/dangling-contract errors; fixed the fixtures and reran `test_color_pipeline_core`.
- [done] Proof-strength finding: runtime taxonomy proof allowed a partial taxonomy with an `>= 12` count; tightened it to exact count `23` and reran `test_color_pipeline_core` plus published runtime UI-Salt proof.
- [done] Clean re-read the repaired state: focused native, runtime UI-Salt, and SDF preservation rails did not expose another real issue.

## Notes

- Expected first taxonomy surface: function-level `taxonomy_group` materialized from `docs/ui_salt/color_pipeline_function_library.ui.salt`.
- Expected group examples: `escape`, `phase`, `bands`, `basin`, `sdf`, `identity`, `remap`, `repeat`, `posterize`, `palette_escape`, `palette_phase`, `palette_basin`, `grade_escape`, `grade_phase`, `grade_basin`, `grade_neutral`, and `grade_manifold`.
- This slice is infrastructure for later library layout/picker work; it is not the visible library redesign itself.
