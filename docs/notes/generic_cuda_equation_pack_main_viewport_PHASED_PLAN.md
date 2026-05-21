# Generic CUDA Equation Pack Main Viewport Integration

## Current Phase

Phase 5 complete - normal selector, left-panel pack controls, main viewport bridge, validation, hostile audit, and closure proof are complete for this slice

## Phase Checklist

- [x] Phase 1 - Open normal-selector live-viewport slice
- [x] Phase 2 - RED: prove the workbench-only boundary is not the requested product path
- [x] Phase 3 - Green: selectable normal dropdown lane, left-panel JSON/control surface, and Generic CUDA main viewport render
- [x] Phase 4 - Validate focused native/runtime rails, code-quality, contract, plan sync, and hostile audit
- [x] Phase 5 - Checkpoint, write receipts, push, clean tree, and stale-plan grep

## Explicit User Asks

- [done] Continue moving from temporary Equation Pack Workbench scaffolding toward normal main viewport integration.
- [done] Expose the custom equation-pack path as a selectable value in the normal Fractal Type dropdown.
- [done] Put the user-facing JSON/textbox and controls in the existing left-side Controls flow.
- [done] Keep the workbench as scaffolding only; do not claim it is the final feature.
- [done] Do not rewrite the fixed enum renderer or dynamic-register CUDA kernels in this slice.
- [done] Preserve prior Generic CUDA, Color Pipeline, no-mouse automation, and shipped fractal behavior.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on branch `codex/engine-architecture-review`, head `16818cb`, clean tree; active contract was the already-closed integer-control repair and was replaced before product mutation.
- New slice contract: `docs/contracts/generic_cuda_equation_pack_main_viewport.contract.json` locked `generic_cuda_equation_pack_main_viewport` as a viewer-first slice with normal dropdown, left Controls panel, AST JSON authority, no physical mouse proof, no dynamic kernel registration, and no Color Pipeline product mutation.
- Repo-grounded seam review / RED: current pre-slice schema/dropdown had no `generic_equation_pack` option; `main.cpp` exposed only an `Equation Pack...` detached workbench button; viewport rendering still always called `RenderFractalCUDA(...)`; `GenericEquationPackWorkbenchState` and `SampleGenericFunction(...)` already existed as the pack/runtime seams.
- Selector/schema GREEN: `FractalType::generic_equation_pack = 44`, enum-id round trip, safe-mode schema, and `ui/fractal_binding_surface_v1.ui_schema.json` now expose `generic_equation_pack` as a normal dropdown option. `test_schema_binding`, `test_enum_id_utils`, `test_cli_args`, and `test_fractal_types` cover that inventory.
- Left-panel GREEN: `ui_app/src/generic_equation_pack_workbench.*` now shares the pack JSON editor and pack-owned numeric controls between the detached workbench and `RenderGenericEquationPackInlinePanel(...)`, with no-mouse automation ids for `equation_pack.json_text`, `equation_pack.apply_json`, and numeric pack controls.
- Main viewport GREEN: `ui_app/src/generic_equation_pack_live.*` renders the loaded AST pack through `SampleGenericFunction(...)` into the existing RGBA/mask buffers when the public selector is `generic_equation_pack`. Direct fixed-fractal rendering remains on `RenderFractalCUDA(...)`, and direct `RenderFractalCUDA` validation now fails closed for `generic_equation_pack` instead of silently falling back.
- Focused native live proof: `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_main_viewport_live_native_1 --log artifacts/logs/generic_cuda_equation_pack_main_viewport_live_native_1.log -- ui_app/build_tests_vsdevcmd.cmd test_generic_equation_pack_live` passed.
- Focused workbench proof: `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_workbench_ui_after_inline_1 --log artifacts/logs/generic_cuda_equation_pack_workbench_ui_after_inline_1.log -- ui_app/build_tests_vsdevcmd.cmd test_generic_equation_pack_workbench_ui` passed.
- Focused parser/CUDA proof: `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_parser_cuda_after_inline_1 --log artifacts/logs/generic_cuda_equation_pack_parser_cuda_after_inline_1.log -- ui_app/build_tests_vsdevcmd.cmd test_generic_equation_pack` passed with parser and CUDA pack rails green.
- Published no-mouse runtime proof: `py -3.14 -m pytest tests/test_fractal_runtime_generic_equation_pack_interactive_ui.py -q --junitxml artifacts/pytest/generic_cuda_equation_pack_main_viewport_runtime.junit.xml` passed; the new case selects `generic_equation_pack` in one persistent viewer process, observes left-panel pack controls, and proves `equation_pack.steps.primary` changes the rendered main viewport hash.
- Runtime publish proof: `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_main_viewport_runtime_publish_final --log artifacts/logs/generic_cuda_equation_pack_main_viewport_runtime_publish_final.log -- ui_app/build_vsdevcmd.cmd` passed and staged the active runtime at `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Full native proof: `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_main_viewport_native_final --log artifacts/logs/generic_cuda_equation_pack_main_viewport_native_final.log -- ui_app/build_tests_vsdevcmd.cmd` passed with `All helper tests passed.`
- Code-quality proof: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/logs/generic_cuda_equation_pack_main_viewport_code_quality.json` passed with score `97`, no criticals, and no errors.
- Contract proof: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/generic_cuda_equation_pack_main_viewport.contract.json --out-json artifacts/validation/generic_cuda_equation_pack_main_viewport_contract.json` passed with `checks.contract_schema_valid = true`.
- Scope proof: no Color Pipeline product files were edited; no OS mouse tests were introduced; no dynamic CUDA kernel registration, Salticid adapter, or broad `RenderFractalCUDA` rewrite landed.

## Hostile Audit

- Status: complete
- Required questions answered before closeout:
  - Did the normal `fractal_type` dropdown actually expose a `generic_equation_pack` lane? Yes; schema, enum-id, CLI, safe-mode schema, and schema-binding tests cover it.
  - Did selecting it keep public selector identity instead of secretly opening the detached workbench only? Yes; the published no-mouse runtime proof selects `generic_equation_pack` and asserts `current_fractal_type == "generic_equation_pack"` in one persistent viewer process.
  - Did the left-side Controls panel expose an editable JSON/textbox surface and pack-owned controls? Yes; `RenderGenericEquationPackInlinePanel(...)` is rendered only when the normal selector is active, and runtime proof observes `equation_pack.json_text`, `equation_pack.apply_json`, and `equation_pack.steps.primary`.
  - Did the main viewport render from the loaded AST pack through `SampleGenericFunction`? Yes; `RenderGenericEquationPackLiveFrame(...)` lowers AST JSON to `GenericFunctionDesc`, calls `SampleGenericFunction(...)`, writes the RGBA buffer, and focused/native/runtime hash proofs cover the path.
  - Did ordinary shipped fractals still route through `RenderFractalCUDA` unchanged? Yes; `DispatchRenderFrame(...)` only diverts `FractalType::generic_equation_pack`; all other selectors still call `RenderFractalCUDA(...)`.
  - Did no-mouse automation prove the published viewer path in one persistent process? Yes; the new runtime pytest uses `PersistentRuntimeViewerAutomation` and asserts `launch_count == 1`.
  - Did this slice avoid dynamic CUDA kernel registration, Salticid sample_fn syntax, and Color Pipeline product mutation? Yes; those remain explicit deferrals and no Color Pipeline product behavior was changed.

## Audit Passes

- [done] Pass 1 - reviewed selector/schema/live-render/UI seams after first green; no fixed-fractal renderer diversion was found, and the generic selector is fail-closed outside the new live bridge.
- [done] Pass 2 - challenged no-mouse runtime proof and shipped-fractal non-regression; the runtime proof uses one persistent viewer process and a pack control hash change, while the code diff confines render diversion to `FractalType::generic_equation_pack`.
- [done] Pass 3 - clean re-read of the repaired proof chain after the native wrapper timeout was rejected as evidence; the final native rerun and final runtime publish logs are the closure evidence, and no additional real issue found.

## Audit Findings

- [done] Real workflow defect found and repaired: the phased plan still described RED/GREEN/proof/audit work as pending after the implementation and rails had already run. This plan update replaces stale `[open]` asks and pending proof text with the actual proof ledger before checkpoint closure.
- [done] Real proof-chain risk found and repaired: an earlier full-native wrapper run timed out at the wrapper level even though the child process continued. That run is not used as closure evidence; the final `generic_cuda_equation_pack_main_viewport_native_final` logged run passed cleanly and is the recorded native proof.
- [done] Clean re-read result: after reviewing `ui_app/src/main.cpp`, `ui_app/src/generic_equation_pack_live.*`, `ui_app/src/generic_equation_pack_workbench.*`, schema/enum surfaces, build wiring, and runtime pytest proof, no additional real defect or workflow mistake was found in this slice.

## Deferred Work

- [deferred] Persist edited pack JSON and selected pack state into saved viewer state files.
- [deferred] Add a catalog/preset browser for multiple equation packs instead of only the default AST pack and pasted JSON.
- [deferred] Add a Salticid `sample_fn` / pack adapter that emits the v1 AST JSON schema.
- [deferred] Integrate deeper performance/color-pipeline controls for generic packs; this slice keeps the bridge bounded and does not claim final renderer parity.
- [deferred] Dynamic CUDA kernel registration and live `RenderFractalCUDA` enum-kernel generation remain future work.

## Notes

- This slice is not the full final custom-fractal feature. It is the first normal-viewer vertical: `generic_equation_pack` is selectable in the existing Fractal Type dropdown, has an inline JSON/control panel in the existing left Controls flow, and renders the current AST pack in the main viewport through the existing Generic CUDA sampler.
- The detached Equation Pack Workbench remains available as scaffolding and debug tooling, but it is no longer the only interactive product surface for packs.
- Existing fixed fractals continue through the fixed enum renderer; the generic selector is the only viewport path that uses the new bridge.
