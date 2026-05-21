# Generic CUDA Equation Pack Workbench

## Current Phase

Closed: the bounded Generic CUDA equation-pack workbench vertical is implemented, validated, and hostile-audited on this slice.

## Explicit User Asks

- [x] Implement the AST-defined equation-pack workbench vertical.
- [x] Keep v1 off the live viewport renderer path: no new `FractalType`, no `RenderFractalCUDA` custom-formula hook, and no dynamic CUDA kernel registration.
- [x] Make AST JSON the pack/source-of-truth contract, with Salticid `sample_fn` lowering deferred to a later adapter.
- [x] Lower packs deterministically to `GenericFunctionDesc` and prove execution on the existing generic CUDA sampler path.
- [x] Add a workbench runner that writes request/response JSON plus PNG gallery frames without OS mouse automation or repeated viewer launch loops.
- [x] Preserve existing `generic.sample`, function descriptor, Color Pipeline, and no-mouse harness behavior.

## Phase Checklist

- [x] Phase 0 - open this phased plan, create the checked-in contract, and lock the active slice.
- [x] Phase 1 - add RED native tests for AST pack parsing/lowering failures and known formula equivalence.
- [x] Phase 2 - implement the native AST pack parser/lowerer and wire `generic.sample` to accept AST-backed requests.
- [x] Phase 3 - add CUDA parity proof for AST-lowered direct, compose, and iterative maps.
- [x] Phase 4 - add the Python workbench runner, pack examples, docs, and focused workbench/runtime tests.
- [x] Phase 5 - run focused/full validation, hostile audit, checkpoint, receipts, and stale-plan check.

## Proof Ledger

- Starting branch: `codex/engine-architecture-review`.
- Starting head: `f4bed50`.
- Bootstrap status: clean tree; previous active contract was `fractal_parameter_surface_matrix` and must be replaced before mutation.
- Existing authority: `generic.sample` already has expression parsing, CPU/CUDA backend selection, `runtime.backend_used`, and `SampleGenericFunction` CUDA execution.
- New authority target: equation-pack AST JSON lowers to `GenericFunctionDesc`; generated expression text may exist only as transport/diagnostic scaffolding, not as the source of truth.
- Out-of-scope seams: live viewport renderer integration, new fractal enum values, dynamic CUDA kernel registration, Salticid parser/lowering integration, Color Pipeline product mutation, physical mouse automation.
- RED proof: `py -3.14 -m pytest tests/test_generic_equation_pack_workbench.py -q --junitxml artifacts/pytest/generic_equation_pack_workbench_red.junit.xml` failed because `tools.reality_toolkit.fractal_explorer.generic_equation_pack` did not exist.
- Focused native proof: `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_focused_native --log artifacts/logs/generic_cuda_equation_pack_focused_native.log -- ui_app/build_tests_vsdevcmd.cmd test_generic_equation_pack` initially failed on pack/expression equivalence and descriptive unknown-key casing, proving the RED rail caught implementation drift.
- Focused native green: `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_focused_native_2 --log artifacts/logs/generic_cuda_equation_pack_focused_native_2.log -- ui_app/build_tests_vsdevcmd.cmd test_generic_equation_pack` passed; C++ pack parser/lowerer and CUDA parity were green.
- Focused generic probe green: `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_focused_generic_probe --log artifacts/logs/generic_cuda_equation_pack_focused_generic_probe.log -- ui_app/build_tests_vsdevcmd.cmd test_generic_probe` passed.
- Runtime publish first attempt found a real link omission: `generic_equation_pack.obj` was compiled but not linked. Repaired before runtime proof.
- Runtime publish green: `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_workbench_runtime_publish_2 --log artifacts/logs/generic_cuda_equation_pack_workbench_runtime_publish_2.log -- ui_app/build_vsdevcmd.cmd` passed and activated `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published-runtime pytest green: `py -3.14 -m pytest tests/test_generic_probe_cli.py tests/test_generic_equation_pack_workbench.py -q --junitxml artifacts/pytest/generic_cuda_equation_pack_workbench_runtime.junit.xml` passed 23 tests.
- Actual no-mouse workbench smoke green: `py -3.14 tools/reality_toolkit/scripts/run_generic_equation_pack_workbench.py --pack-json docs/examples/equation_packs/newton_z3_minus_1_pack.json --out-dir artifacts/equation_pack_workbench/newton_z3_minus_1_cuda_smoke --backend cuda --grid-width 16 --grid-height 16` wrote request/response/manifest/PNG with backend `cuda`.
- Code-quality final green: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/logs/generic_cuda_equation_pack_workbench_code_quality.json` passed baseline with score 97/100 after the parser helper split.
- Published runtime final candidate green: `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_workbench_runtime_publish_final_candidate --log artifacts/logs/generic_cuda_equation_pack_workbench_runtime_publish_final_candidate.log -- ui_app/build_vsdevcmd.cmd` passed.
- Published-runtime pytest final green: `py -3.14 -m pytest tests/test_generic_probe_cli.py tests/test_generic_equation_pack_workbench.py -q --junitxml artifacts/pytest/generic_cuda_equation_pack_workbench_runtime.junit.xml` passed 23 tests.
- Actual no-mouse workbench smoke final green: `py -3.14 tools/reality_toolkit/scripts/run_generic_equation_pack_workbench.py --pack-json docs/examples/equation_packs/newton_z3_minus_1_pack.json --out-dir artifacts/equation_pack_workbench/newton_z3_minus_1_cuda_smoke --backend cuda --grid-width 16 --grid-height 16` wrote request/response/manifest/PNG with backend `cuda`.
- Full native final green: `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_workbench_native_final --log artifacts/logs/generic_cuda_equation_pack_workbench_native_final.log -- ui_app/build_tests_vsdevcmd.cmd` passed.

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - challenged AST authority; generated workbench request contains `function.ast` and no `function.expression`, runtime malformed AST returns `AST lower error`, and no fallback expression exists in the workbench path.
- [x] Pass 2 - challenged CUDA proof; `test_generic_equation_pack_cuda` runs AST-lowered descriptors through `SampleGenericFunction`, published-runtime pytest proves explicit `backend_preference: cuda`, and workbench smoke response reports `runtime.backend_used: cuda`.
- [x] Pass 3 - challenged no-mouse/workbench behavior; the runner uses `run_sample_request` stdin/stdout plus `write_generic_sample_gallery`, grep found no `SetCursorPos`, `GetCursorPos`, `SendInput`, or mouse automation in the touched workbench/tests, and full native plus runtime pytest proved cleanly.

## Audit Findings

- [x] Real finding repaired: runtime publish first failed because `generic_equation_pack.obj` was compiled but omitted from the runtime link response; fixed `ui_app/build_vsdevcmd.cmd`.
- [x] Real finding repaired: code-quality audit flagged the new pack parser as a large new function; split formula parsing into `ParseFormula`.
- [x] Clean re-read: reran published runtime build plus pytest after the link repair, and both proved cleanly.
- [x] Clean re-read: reran code-quality after the parser split, and `generic_equation_pack.cpp` no longer appeared as a top-offender warning.
- [x] Clean re-read: scoped diff check confirmed no changes to `ui_app/src/fractal_renderer.cu`, `ui_app/src/fractal_types.h`, or Color Pipeline files, so the slice did not widen into live renderer, enum, or color-pipeline work.
