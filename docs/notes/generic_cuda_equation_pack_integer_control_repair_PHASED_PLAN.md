# Generic CUDA Equation Pack Integer Control Repair

## Current Phase

Complete - integer iteration-control repair is validated, hostile-audited, checkpointed, and represented by the final repo state.

## Explicit User Asks

- [x] Repair the visible workbench failure where dragging `Steps` creates a fractional iterate count and the preview errors with `iterate count must be an integer`.
- [x] Treat this as evidence the feature is not done and harden the actual control surface, not only the report.
- [x] Preserve no-mouse runtime proof, avoid repeated viewer open/close loops, and keep the work bounded.
- [x] Preserve AST JSON pack authority, strict lowerer validation, Generic CUDA execution, Color Pipeline behavior, and the live enum renderer boundary.

## Phase Checklist

- [x] Phase 0 - open this phased plan, create the checked-in contract, and lock the active slice.
- [x] Phase 1 - add RED native/runtime tests for fractional iteration-param controls causing preview errors.
- [x] Phase 2 - classify `formula.iteration_param` controls as integer-valued in the workbench and automation report.
- [x] Phase 3 - normalize manual-equivalent and command-json set-value edits before preview execution while preserving raw lowerer strictness.
- [x] Phase 4 - validate focused native/runtime rails, full native, runtime publish, and hostile audit.
- [x] Phase 5 - checkpoint, receipts, push, clean tree, and stale-plan check.

## Proof Ledger

- Starting branch: `codex/engine-architecture-review`.
- Starting head: `a5828c9`.
- Bootstrap status: clean tree; previous active contract was the closed `generic_cuda_equation_pack_controls_report_reset` slice and must be replaced before mutation.
- User screenshot proof: the Newton pack workbench could show `Steps` as `67.565`, then failed preview with `AST lower error: iterate count must be an integer`.
- Active contract lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "generic CUDA equation-pack integer iteration control repair" --profile runtime --plan docs/notes/generic_cuda_equation_pack_integer_control_repair_PHASED_PLAN.md --contract docs/contracts/generic_cuda_equation_pack_integer_control_repair.contract.json` appended `ck:21d17b42` and locked `generic_cuda_equation_pack_integer_control_repair`.
- Existing authority: `ValidateGenericFunctionDesc(...)` and the lowerer correctly reject non-integer iterate counts; that strict raw-pack behavior stayed intact.
- New authority: the workbench now classifies controls whose `param` equals `formula.iteration_param` as `integer_value`, renders them through integer ImGui widgets, and normalizes set-value/reset edits before preview execution.
- RED proof: `artifacts/logs/generic_cuda_equation_pack_integer_control_repair_red_workbench.log` failed because `GenericEquationPackWorkbenchControlReport` did not expose `integer_value` and no workbench integer-control contract existed.
- Focused native workbench proof: `artifacts/logs/generic_cuda_equation_pack_integer_control_repair_workbench_native_1.log` reports `test_generic_equation_pack_workbench_ui: passed=61`, including fractional `steps=67.565` rounding to `68.0` before preview.
- Raw lowerer strictness proof: `artifacts/logs/generic_cuda_equation_pack_integer_control_repair_pack_native_1.log` reports `test_generic_equation_pack: pass=23 fail=0` and CUDA pack parity `pass=54 fail=0`; raw fractional iterate packs still fail during lowering with an integer error.
- Shared automation-report proof: `artifacts/logs/generic_cuda_equation_pack_integer_control_repair_report_native_1.log` reports `test_viewer_ui_automation_report: all passed`.
- Full native proof: `artifacts/logs/generic_cuda_equation_pack_integer_control_repair_native_final.log` reports `result=success` and `All helper tests passed`.
- Runtime publish proof: `artifacts/logs/generic_cuda_equation_pack_integer_control_repair_runtime_publish_final.log` reports `result=success` and stages `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- No-mouse runtime proof: `py -3.14 -m pytest tests/test_fractal_runtime_generic_equation_pack_interactive_ui.py -q --junitxml artifacts/pytest/generic_cuda_equation_pack_integer_control_repair_runtime.junit.xml` reports `2 passed`; the module-scoped persistent viewer proof sets `equation_pack.steps.primary` to `67.565`, observes `integer_value: true`, observes reported value `68.0`, and keeps `launch_count == 1`.
- Code-quality proof: `artifacts/logs/generic_cuda_equation_pack_integer_control_repair_code_quality.json` reports baseline check passed with score 97/100.
- Contract proof: `artifacts/validation/generic_cuda_equation_pack_integer_control_repair_contract.json` reports `ok: true`.
- Scope proof: `git diff -- ui_app/src/fractal_renderer.cu ui_app/src/fractal_types.h ui_app/src/color_pipeline_window.h ui_app/src/color_pipeline_core.h docs/examples/equation_packs` is empty.
- Hostile-audit proof: `artifacts/validation/generic_cuda_equation_pack_integer_control_repair_hostile_audit.json` reports `ok: true`.
- Plan-sync proof: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` reports `OK` for this plan and writes `artifacts/validation/viewer_host_assert_phased_plan_sync.json`.
- First checkpoint commit: `c985e1c` (`Repair equation pack integer controls`).

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - real defect found: the previous workbench treated `formula.iteration_param` controls as ordinary float sliders, so manual UI could store fractional `steps` values that the lowerer correctly rejected.
- [x] Pass 2 - clean re-read confirmed the repaired workbench derives integer classification from `pack.formula_kind == "iterate_map" && control.param == pack.iteration_param`, not from a hardcoded `steps` id.
- [x] Pass 3 - clean re-read confirmed raw pack/lowerer strictness was not weakened; the new native pack test still rejects a raw `steps: 67.565` pack during lowering.
- [x] Pass 4 - clean re-read confirmed the runtime proof remains one-process/no-mouse, the report publishes `integer_value: true`, the fractional set-value command reports `68.0`, and forbidden renderer/fractal enum/Color Pipeline/example-pack diffs are empty.

## Audit Findings

- [x] Real finding repaired: integer iterate controls were missing as a first-class workbench control kind, causing a product-visible `AST lower error` after ordinary slider interaction.
- [x] Clean re-read: focused workbench native, raw lowerer strictness native/CUDA, shared automation-report native, full native, runtime publish, no-mouse runtime pytest, code quality, and contract validation all passed on the repaired state.

## Notes

- Minimal repair landed: derive integer-valued workbench controls from `pack.formula_kind == "iterate_map" && control.param == pack.iteration_param`.
- Manual UI and `--ui-automation-set-control-value` apply the same normalization through `SetGenericEquationPackWorkbenchControlValue(...)`.
- `LowerGenericEquationPackToDesc(...)` continues rejecting raw fractional `steps` values when they come from pack authority rather than workbench edits.
- Preserved boundaries:
  - no `RenderFractalCUDA` edit
  - no `FractalType` edit
  - no dynamic CUDA kernel registration
  - no Salticid adapter syntax
  - no Color Pipeline product implementation edit
  - no equation-pack example mutation
