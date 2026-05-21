# Generic CUDA Equation Pack Interactive UI

## Current Phase

Closed: the bounded Generic CUDA equation-pack interactive UI slice is implemented, validated, and hostile-audited.

## Explicit User Asks

- [x] Take the next big step toward a real feature with an interactable UI.
- [x] Keep this as a solid extensible vertical on top of the existing Generic CUDA equation-pack backend.
- [x] Preserve the live enum renderer, Color Pipeline behavior, existing workbench runner, and no-mouse automation discipline.
- [x] Do not add broad engine rewrites, dynamic CUDA kernel registration, or Salticid `sample_fn` syntax in this slice.

## Phase Checklist

- [x] Phase 0 - open this phased plan, create the checked-in contract, and lock the active slice.
- [x] Phase 1 - inspect current viewer, automation-report, CLI, and generic sampler seams; add RED tests for missing interactive UI behavior.
- [x] Phase 2 - implement the native workbench state/control model and preview execution against AST-backed packs.
- [x] Phase 3 - render the viewer workbench window and expose in-process no-mouse set-value/report automation.
- [x] Phase 4 - add focused native/runtime tests proving visible controls edit preview results without OS mouse or repeated viewer launch loops.
- [x] Phase 5 - run validation, hostile audit, and prepare checkpoint/receipt closure.

## Proof Ledger

- Starting branch: `codex/engine-architecture-review`.
- Starting head: `003e866`.
- Bootstrap status: clean tree; previous active contract was `generic_cuda_equation_pack_workbench` and must be replaced before product mutation.
- Active contract lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "generic CUDA equation-pack interactive UI" --profile runtime --plan docs/notes/generic_cuda_equation_pack_interactive_ui_PHASED_PLAN.md --contract docs/contracts/generic_cuda_equation_pack_interactive_ui.contract.json` appended `ck:205ee136` and locked `generic_cuda_equation_pack_interactive_ui`.
- Existing authority: the closed backend slice added AST JSON pack parsing/lowering, `function.ast` support in `generic.sample`, CUDA proof through `SampleGenericFunction`, and a no-mouse CLI workbench runner.
- New authority target: an in-viewer Equation Pack Workbench window that loads a pack, exposes its declared controls, applies control edits through in-process automation, and runs a deterministic preview through the existing generic execution path.
- Out-of-scope seams: live enum renderer integration, new fractal enum values, dynamic CUDA kernel registration, Salticid parser/lowering integration, broad Color Pipeline changes, and physical mouse automation.
- Owner seams inspected: `main.cpp` routes schema controls and Color Pipeline controls into one automation report, `viewer_cli.cpp` owns the existing no-mouse flags, `viewer_ui_automation_report.cpp` already fails closed for visible unsupported controls, and `generic_equation_pack.cpp` plus `generic_sample_core.cu` provide the AST-to-`SampleGenericFunction` execution surface.
- RED proof: `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_interactive_ui_red_native --log artifacts/logs/generic_cuda_equation_pack_interactive_ui_red_native.log -- ui_app/build_tests_vsdevcmd.cmd test_generic_equation_pack_workbench_ui` failed because `generic_equation_pack_workbench.cpp` and `generic_equation_pack_workbench.h` did not exist.
- Focused native workbench green: `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_interactive_ui_focused_native_1 --log artifacts/logs/generic_cuda_equation_pack_interactive_ui_focused_native_1.log -- ui_app/build_tests_vsdevcmd.cmd test_generic_equation_pack_workbench_ui` passed.
- Shared automation-report native green: `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_interactive_ui_report_native_2 --log artifacts/logs/generic_cuda_equation_pack_interactive_ui_report_native_2.log -- ui_app/build_tests_vsdevcmd.cmd test_viewer_ui_automation_report` passed.
- Prior backend preservation green: `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_interactive_ui_backend_native --log artifacts/logs/generic_cuda_equation_pack_interactive_ui_backend_native.log -- ui_app/build_tests_vsdevcmd.cmd test_generic_equation_pack` passed, including CUDA parity.
- Code-quality green: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/logs/generic_cuda_equation_pack_interactive_ui_code_quality.json` passed with score 97/100.
- Final runtime publish green: `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_interactive_ui_runtime_publish_final --log artifacts/logs/generic_cuda_equation_pack_interactive_ui_runtime_publish_final.log -- ui_app/build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published-runtime pytest green: `py -3.14 -m pytest tests/test_generic_probe_cli.py tests/test_generic_equation_pack_workbench.py tests/test_generic_equation_pack_interactive_ui.py -q --junitxml artifacts/pytest/generic_cuda_equation_pack_interactive_ui_runtime.junit.xml` passed 24 tests after the final runtime publish.
- Viewer-first receipt gate repair: the new runtime proof was moved to `tests/test_fractal_runtime_generic_equation_pack_interactive_ui.py` so the checked-in contract and validation receipt use the repo's `test_fractal_runtime*.py` runtime-proof convention.
- Exact viewer-first runtime proof green: `py -3.14 -m pytest tests/test_fractal_runtime_generic_equation_pack_interactive_ui.py -q --junitxml artifacts/pytest/generic_cuda_equation_pack_interactive_ui_runtime.junit.xml` passed.
- Full native final green: `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_interactive_ui_native_final_2 --log artifacts/logs/generic_cuda_equation_pack_interactive_ui_native_final_2.log -- ui_app/build_tests_vsdevcmd.cmd` passed.
- Hostile grep: `rg "SetCursorPos|GetCursorPos|mouse_event|MOUSEEVENTF|drag_screen_rect|SendInput" tests/test_generic_equation_pack_interactive_ui.py tests/runtime_harness.py ui_app/src/generic_equation_pack_workbench.cpp -n` found no first-party physical mouse automation in the touched workbench proof path.
- Scope proof: `git diff -- ui_app/src/fractal_renderer.cu ui_app/src/fractal_types.h ui_app/src/color_pipeline_window.h ui_app/src/color_pipeline_core.h` was empty, so the slice did not mutate the live enum renderer, fractal enum, or Color Pipeline product implementation.

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - challenged whether the UI actually loads pack controls from pack schema instead of hardcoding a demo-only panel; native tests assert control IDs derive from `GenericEquationPackControl` and runtime waits for `equation_pack.c_real.primary` from the loaded pack.
- [x] Pass 2 - challenged whether edits actually affect preview execution and report deterministic sample/result changes; native and runtime tests compare the preview result hash before and after an in-process control edit.
- [x] Pass 3 - challenged whether runtime proof stayed in-process/no-mouse and avoided repeated viewer open/close loops; the runtime test uses one `PersistentRuntimeViewerAutomation` process and command JSON set-value automation.
- [x] Pass 4 - challenged whether the feature is reachable as product UI instead of hidden automation; added the Controls-window `Equation Pack...` entry point that opens the workbench and loads the default example pack.
- [x] Pass 5 - challenged whether the slice touched renderer, fractal enum, dynamic kernel, or Color Pipeline product behavior; scoped diff checks were empty for those product seams.

## Audit Findings

- [x] Real finding repaired: the first implementation only exposed the workbench through CLI automation; added a normal Controls-window `Equation Pack...` button so the feature is interactable from the viewer UI.
- [x] Clean re-read: focused native, runtime publish, published-runtime pytest, code-quality audit, and full native validation passed after the UI entry-point repair.
