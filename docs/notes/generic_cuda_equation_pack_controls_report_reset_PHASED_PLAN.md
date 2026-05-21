# Generic CUDA Equation Pack Controls Report And Reset

## Current Phase

Phase 5 - record final evidence, machine receipts, push, clean tree, and stale-plan check.

## Explicit User Asks

- [x] Continue fleshing out and hardening the Generic CUDA equation-pack feature.
- [x] Keep the work modular, simple, and extensible rather than turning it into renderer or engine churn.
- [x] Preserve no-mouse runtime proof and avoid repeated viewer open/close loops.
- [x] Preserve AST JSON pack authority, Generic CUDA execution, Color Pipeline behavior, and the live enum renderer boundary.

## Phase Checklist

- [x] Phase 0 - open this phased plan, create the checked-in contract, and lock the active slice.
- [x] Phase 1 - add RED native/runtime tests for missing workbench control inventory, current values, and reset-defaults automation.
- [x] Phase 2 - publish pack controls and current values in the automation report.
- [x] Phase 3 - add an in-process reset-to-defaults UI/automation path using existing workbench edit semantics.
- [x] Phase 4 - validate focused native/runtime rails, full native, runtime publish, and hostile audit.
- [ ] Phase 5 - record final evidence, machine receipts, push, clean tree, and stale-plan check.

## Proof Ledger

- Starting branch: `codex/engine-architecture-review`.
- Starting head: `cfc79fc`.
- Bootstrap status: clean tree; previous active contract was `generic_cuda_equation_pack_preview_canvas` and must be replaced before mutation.
- Existing authority: the previous slices added AST JSON pack parsing/lowering, generic.sample AST runtime requests, Controls-window workbench entry, pack-schema sliders, no-mouse set-value automation, CUDA preview hashes, and a visible preview canvas.
- Active contract lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "generic CUDA equation-pack controls report and reset defaults" --profile runtime --plan docs/notes/generic_cuda_equation_pack_controls_report_reset_PHASED_PLAN.md --contract docs/contracts/generic_cuda_equation_pack_controls_report_reset.contract.json` appended `ck:0e438456` and locked `generic_cuda_equation_pack_controls_report_reset`.
- New authority target: the in-viewer workbench now reports the loaded pack's visible controls and current values, and supports an in-process reset-to-defaults path that is testable without physical mouse movement.
- Out-of-scope seams: `RenderFractalCUDA`, `FractalType`, dynamic CUDA kernel registration, Salticid adapter syntax, broad Color Pipeline changes, and physical mouse automation.
- RED proof: `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_controls_report_reset_red_native --log artifacts/logs/generic_cuda_equation_pack_controls_report_reset_red_native.log -- ui_app/build_tests_vsdevcmd.cmd test_generic_equation_pack_workbench_ui` failed because the report lacked `pack_load_error`, `controls`, `GenericEquationPackWorkbenchControlReport`, and `ResetGenericEquationPackWorkbenchControlsToDefaults(...)`.
- Focused native workbench proof: `artifacts/logs/generic_cuda_equation_pack_controls_report_reset_focused_native_1.log` reports `test_generic_equation_pack_workbench_ui: passed=52`.
- Shared automation-report proof: `artifacts/logs/generic_cuda_equation_pack_controls_report_reset_report_native_1.log` reports `test_viewer_ui_automation_report: all passed`.
- Color Pipeline guard proof: `artifacts/logs/generic_cuda_equation_pack_controls_report_reset_color_pipeline_guard_1.log` reports `test_color_pipeline_core: passed=120 failed=0`, `test_color_pipeline_window: passed=155 failed=0`, `test_schema_binding: all passed`, and `test_escape_time_coloring: all passed`.
- Runtime publish proof: `artifacts/logs/generic_cuda_equation_pack_controls_report_reset_runtime_publish_final.log` reports `result=success` and stages `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- No-mouse runtime proof: `py -3.14 -m pytest tests/test_fractal_runtime_generic_equation_pack_interactive_ui.py -q --junitxml artifacts/pytest/generic_cuda_equation_pack_controls_report_reset_runtime.junit.xml` reports `2 passed`; both runtime test nodes consume one module-scoped persistent viewer proof and assert `launch_count == 1`.
- Full native proof: `artifacts/logs/generic_cuda_equation_pack_controls_report_reset_native_final.log` reports `result=success` and `All helper tests passed`.
- Code-quality proof: `artifacts/logs/generic_cuda_equation_pack_controls_report_reset_code_quality.json` reports baseline check passed with score 97/100.
- Contract proof: `artifacts/validation/generic_cuda_equation_pack_controls_report_reset_contract.json` reports `ok: true`.
- Scope proof: `git diff -- ui_app/src/fractal_renderer.cu ui_app/src/fractal_types.h ui_app/src/color_pipeline_window.h ui_app/src/color_pipeline_core.h docs/examples/equation_packs` is empty.
- No-physical-mouse proof: `rg "SetCursorPos|GetCursorPos|mouse_event|MOUSEEVENTF|SendInput|drag_screen_rect" tests/test_fractal_runtime_generic_equation_pack_interactive_ui.py tests/runtime_harness.py ui_app/src/generic_equation_pack_workbench.cpp ui_app/src/main.cpp -n` found no matches in the touched proof path.
- Hostile-audit proof: pending final validator run after this plan sync.
- Plan-sync proof: pending final validator run after this plan sync.

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - real defect found: the first runtime proof added a second viewer-launching pytest while the previous set-value proof still opened its own viewer, which violated the no repeated open/close loop constraint.
- [x] Pass 2 - clean re-read of the repaired runtime test: both runtime test nodes now consume one module-scoped persistent viewer proof, and both assert `launch_count == 1`.
- [x] Pass 3 - clean re-read of the product state: the report controls derive from the loaded pack controls and current params, reset uses the same workbench state/default/dirty path as slider edits, and runtime proof observes the reset value returning to default.
- [x] Pass 4 - clean re-read of scope and collateral risk: forbidden renderer/fractal enum/Color Pipeline product diffs are empty, the Color Pipeline guard rail is green, and no physical cursor APIs appear in the touched proof path.

## Audit Findings

- [x] Real finding repaired: duplicate viewer launches in the runtime pytest module were removed by replacing per-test viewer sessions with one module-scoped persistent viewer proof shared by the historical set-value node and the new controls/reset node.
- [x] Clean re-read: focused native, shared automation-report native, Color Pipeline guard, runtime publish, no-mouse runtime pytest, full native, code quality, and contract validation all passed on the repaired state.

## Notes

- Implemented workbench control report fields: `control_id`, `id`, `param`, `label`, current `value`, optional `min`, `max`, `step`, and `default`.
- Implemented visible reset automation id: `equation_pack.reset_defaults`.
- Preserved boundaries:
  - no `RenderFractalCUDA` edit
  - no `FractalType` edit
  - no dynamic CUDA kernel registration
  - no Salticid adapter syntax
  - no Color Pipeline product implementation edit
  - no physical mouse automation in the touched proof path
