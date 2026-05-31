# Color Pipeline Function Picker Layout

## Current Phase

Phase 5 - hostile audit, focused native rails, runtime publish/proof, plan sync, contract validation, receipts, rearward review, push, and clean tree.

## Phase Checklist

- [x] Phase 1 - open this checked-in phased plan and contract on `codex/color-pipeline-function-picker-layout` from clean post-mixed replan head `e868a81`.
- [x] Phase 2 - add grouped function-picker helpers and native/window regressions proving all existing functions remain selectable exactly once under taxonomy groups.
- [x] Phase 3 - wire the grouped picker into the existing Color Pipeline row editor without changing row semantics, function ids, control ids, recipes, source-stack math, or persisted state.
- [x] Phase 4 - add no-mouse runtime/report proof that representative Source, Shape, Palette, and Grading selections still apply through the published viewer.
- [ ] Phase 5 - hostile audit, focused native rails, runtime publish/proof, plan sync, contract validation, receipts, rearward review, push, and clean tree.

## Explicit User Asks

- [x] Continue from the post-mixed Source-row replan.
- [x] Treat the next step as implementation, not another broad discussion.
- [x] Keep the current Color Pipeline behavior stable while improving the function-library workflow.
- [x] Do not add new functions, SDF ops, fractal lanes, or a broad UI redesign in this slice.

## Scope

In scope:

- Function picker/layout refinement inside the existing Color Pipeline row editor.
- Grouping selectable functions by existing UI-Salt taxonomy metadata.
- Search/filter or equivalent low-risk ergonomics only if it preserves the current visible row editor and no-mouse automation path.
- Focused native/window tests and published no-mouse runtime proof for representative row selections.

Out of scope:

- New Color Pipeline functions or new math behavior.
- New SDF ops, SDF-native lanes, authored pack catalog/authoring UX, or Salticid runtime integration.
- Changing row-stack composition, mixed Source-row scalar semantics, applicator semantics, recipes, state serialization, capture/replay, or renderer behavior.
- Physical mouse automation.

## Current Repo Truth

- The mixed Source-row slice is closed and pushed at `3465487`.
- The post-mixed replan is closed and pushed at `e868a81` on `codex/color-pipeline-post-mixed-source-replan`.
- The current branch `codex/color-pipeline-function-picker-layout` starts clean from `e868a81`.
- Existing UI-Salt metadata already provides taxonomy groups and signal/function classifications; the row editor still presents a flat function combo.
- Mixed SDF/non-SDF Source rows are supported only for renderer-backed lanes; `generic_equation_pack` and `sdf_pack_scene` remain intentionally fail-closed for mixed stacks until those lanes have renderer-backed non-SDF signals.

## Implementation Plan

1. Inspect the current function descriptor catalog, taxonomy metadata, row editor combo path, and no-mouse automation/report surfaces.
2. Add a small grouping helper that consumes the existing `FunctionDescriptor` list and returns stable taxonomy groups without dropping or duplicating entries.
3. Add native/window tests for group membership, stable ordering, disabled/unsupported labeling, and no missing functions.
4. Replace the flat combo rendering with grouped sections while preserving `SelectColorPipelineRowFunction(...)` and existing control ids.
5. Add runtime proof/report coverage only as needed to prove the published viewer still selects representative Source, Shape, Palette, and Grading functions without OS mouse use.
6. Run hostile review and validation before checkpointing.

## Proof Ledger

- Start authority: clean branch `codex/color-pipeline-function-picker-layout` at `e868a81`; rearward review for `e868a81` is `ok`.
- Active preflight: the previous docs-only contract was temporarily widened only to permit creating this successor plan/contract.
- Preflight cleanup: after this contract was locked, the prior replan contract was restored to its closed docs-only mutation scope before product code changes.
- Required product proof: grouped picker lists every existing function once, row selection still calls the same edit path, and representative published no-mouse selections still work.
- Native finding: the first grouped-picker helper merged repeated taxonomy groups and reordered existing options. The repaired helper groups contiguous taxonomy runs so headers are added without moving current choices.
- Runtime harness finding: the first published no-mouse proof hardcoded row IDs `1..4`, but live-imported drafts reported `5..8`. The proof now derives function-picker control ids from the runtime `rows` report.
- Focused native proof: `test_color_pipeline_window`, `test_color_pipeline_core`, and `test_viewer_ui_automation_report` passed through the checked-in VSDevCmd helper.
- Runtime proof: `ui_app/build_vsdevcmd.cmd` published `D:\salt-fractal\cuda_newton_fractal_clone
untime
ractal_ui.exe`, and `tests/test_fractal_runtime_ui_salt_contract.py -q` passed with the visible function-picker controls covered.
- Broad native helper status: `ui_app/build_tests_vsdevcmd.cmd` was attempted twice and timed out after 900s and 1800s while compiling later broad CUDA/generic targets; that rail is unproven for this slice and is not claimed as green.

## Action Hostile Review

- Action ID: color_pipeline_function_picker_layout
- Suspected Failure Mode: grouping could hide functions, duplicate functions, break unsupported choice visibility, break no-mouse selection, or accidentally change Color Pipeline row semantics.
- Correct Owner/Action: keep grouping as a presentation layer over existing metadata and selection/edit paths.
- Proof Surface: native/window tests, runtime no-mouse selection proof, plan sync, contract validation, hostile audit, and published runtime proof.
- Blocked Action: new functions, new SDF ops, new renderer behavior, row-stack semantic changes, broad Factorio-style layout redesign, or physical mouse automation.

## Hostile Audit

- Status: complete
- Required posture: assume the grouped picker lies by omission, drops entries, changes row authority, or weakens published no-mouse proof until tests disprove it.

Required questions:

- Did every existing function remain selectable exactly once?
- Did grouping use current taxonomy metadata instead of hardcoded parallel lists?
- Did row selection still go through the existing edit/write path?
- Did unsupported choices remain fail-closed/labelled instead of silently becoming allowed?
- Did no-mouse automation/report proof cover the published viewer path?
- Did this slice avoid new functions, SDF ops, renderer changes, and broad UI redesign?

## Audit Passes

- [x] Pass 1 - implementation diff/test audit found merged taxonomy grouping would reorder existing options when a taxonomy appeared in multiple catalog runs; fixed to contiguous taxonomy groups.
- [x] Pass 2 - runtime/no-mouse proof audit found hardcoded row-id assumptions; fixed to derive function-picker control ids from the runtime report rows.
- [x] Pass 3 - final clean re-read after the ordering and row-id fixes found no additional product/code defects in the grouped picker, row edit path, control ids, or published no-mouse proof surface.

## Audit Findings

- [x] Finding: merged taxonomy groups would move existing options. Repaired by grouping contiguous taxonomy runs, preserving catalog order while adding headers.
- [x] Finding: runtime proof hardcoded row ids. Repaired by deriving function-picker control ids from the published runtime report.
- [x] Clean third pass: no additional dropped/duplicated function entries, row-selection path changes, unsupported-choice behavior changes, new function additions, SDF-op changes, renderer changes, or physical mouse automation were found.

## Planned Validation Targets

- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/color_pipeline_function_picker_layout.contract.json --out-json artifacts/validation/color_pipeline_function_picker_layout_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/color_pipeline_function_picker_layout_PHASED_PLAN.md --out-json artifacts/validation/color_pipeline_function_picker_layout_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/color_pipeline_function_picker_layout_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_function_picker_layout_window_focused_after_harness --log artifacts/logs/color_pipeline_function_picker_layout_window_focused_after_harness.log --out-json artifacts/validation/color_pipeline_function_picker_layout_window_focused_after_harness.json --heartbeat-seconds 30 --timeout-seconds 300 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_window`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_function_picker_layout_core_focused --log artifacts/logs/color_pipeline_function_picker_layout_core_focused.log --out-json artifacts/validation/color_pipeline_function_picker_layout_core_focused.json --heartbeat-seconds 30 --timeout-seconds 300 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_function_picker_layout_report_focused --log artifacts/logs/color_pipeline_function_picker_layout_report_focused.log --out-json artifacts/validation/color_pipeline_function_picker_layout_report_focused.json --heartbeat-seconds 30 --timeout-seconds 300 -- ui_app/build_tests_vsdevcmd.cmd test_viewer_ui_automation_report`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_function_picker_layout_runtime_publish --log artifacts/logs/color_pipeline_function_picker_layout_runtime_publish.log --out-json artifacts/validation/color_pipeline_function_picker_layout_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_vsdevcmd.cmd`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_function_picker_layout_runtime_ui_salt_retry --log artifacts/logs/color_pipeline_function_picker_layout_runtime_ui_salt_retry.log --out-json artifacts/validation/color_pipeline_function_picker_layout_runtime_ui_salt_retry.json --heartbeat-seconds 30 --timeout-seconds 420 -- py -3.14 -m pytest tests/test_fractal_runtime_ui_salt_contract.py -q`
