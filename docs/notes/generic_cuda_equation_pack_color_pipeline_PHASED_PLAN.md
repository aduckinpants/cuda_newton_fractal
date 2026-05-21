# Generic CUDA Equation Pack Color Pipeline Integration

## Current Phase

Phase 5 complete - generic equation-pack viewport coloring now uses existing Color Pipeline semantics; validation and audit proof are recorded for wrapper closure.

## Phase Checklist

- [x] Phase 1 - open the color-pipeline integration follow-up and prove the current private generic-pack color path is not the final architecture
- [x] Phase 2 - RED: native/runtime proofs fail unless generic equation-pack frames respond to existing Color Pipeline source/palette/grading controls
- [x] Phase 3 - Green: route generic equation-pack live coloring through the existing Color Pipeline semantics instead of a private palette
- [x] Phase 4 - Validate focused native/runtime rails, Color Pipeline guard rails, code-quality, contract, plan sync, and hostile audit
- [x] Phase 5 - record wrapper checkpoint, receipts, push, clean-tree, and stale-plan-grep proof for the closed slice

## Explicit User Asks

- [done] Generic equation-pack viewport rendering cannot live outside the Color Pipeline system.
- [done] Keep the normal dropdown and left Controls flow from the prior slice; this is a follow-up integration, not a rollback.
- [done] Do not make the detached workbench or private CPU palette the product coloring path.
- [done] Preserve existing shipped fractals, existing Color Pipeline behavior, no-mouse automation, and previous Generic CUDA pack functionality.
- [done] Keep this bounded to Color Pipeline integration; do not implement dynamic CUDA kernel registration or Salticid sample_fn lowering here.

## Proof Ledger

- Bootstrap: complete. Session opened from clean branch `codex/engine-architecture-review` at `5b8e8b5`; prior closed contract was replaced by `generic_cuda_equation_pack_color_pipeline`.
- Contract lock: complete. `py -3.14 tools/viewer_host_begin_work_slice.py --intent "generic CUDA equation-pack Color Pipeline integration" --profile runtime --plan docs/notes/generic_cuda_equation_pack_color_pipeline_PHASED_PLAN.md --contract docs/contracts/generic_cuda_equation_pack_color_pipeline.contract.json` produced checkpoint token `ck:c01a940b`.
- Contract correction: complete. Unsupported direct focused target `test_color_pipeline_core` was replaced with the exposed `advanced_color_grading_red` guard bundle, then re-locked with `py -3.14 tools/viewer_host_revise_contract.py --session-id global_active_contract --cwd . --plan docs/notes/generic_cuda_equation_pack_color_pipeline_PHASED_PLAN.md --contract docs/contracts/generic_cuda_equation_pack_color_pipeline.contract.json`.
- RED owner seam: complete. `ui_app/src/generic_equation_pack_live.cpp` had a private `LivePixelFromResult(...)` palette path and did not accept `KernelParams`, so the rendered generic pack lane could not be owned by the Color Pipeline.
- RED native proof: complete. `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_color_pipeline_red_native --log artifacts/logs/generic_cuda_equation_pack_color_pipeline_red_native.log -- ui_app/build_tests_vsdevcmd.cmd test_generic_equation_pack_live` failed on the new test signature before implementation: `RenderGenericEquationPackLiveFrame` did not take the Color Pipeline parameter surface.
- First GREEN: complete. `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_color_pipeline_native_1 --log artifacts/logs/generic_cuda_equation_pack_color_pipeline_native_1.log -- ui_app/build_tests_vsdevcmd.cmd test_generic_equation_pack_live` passed.
- Workbench guard: complete. `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_color_pipeline_workbench_guard_1 --log artifacts/logs/generic_cuda_equation_pack_color_pipeline_workbench_guard_1.log -- ui_app/build_tests_vsdevcmd.cmd test_generic_equation_pack_workbench_ui` passed with `passed=61`.
- Contract validation: complete. `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/generic_cuda_equation_pack_color_pipeline.contract.json --out-json artifacts/validation/generic_cuda_equation_pack_color_pipeline_contract.json` passed.
- Color Pipeline guard proof: complete. `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_color_pipeline_guard_2 --log artifacts/logs/generic_cuda_equation_pack_color_pipeline_guard_2.log -- ui_app/build_tests_vsdevcmd.cmd advanced_color_grading_red` passed, including `test_color_pipeline_core`, `test_color_pipeline_window`, `test_schema_binding`, and `test_escape_time_coloring`.
- Runtime publish proof: complete. `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_color_pipeline_runtime_publish_final --log artifacts/logs/generic_cuda_equation_pack_color_pipeline_runtime_publish_final.log -- ui_app/build_vsdevcmd.cmd` passed and staged the published viewer runtime.
- Published no-mouse runtime proof: complete. `py -3.14 -m pytest tests/test_fractal_runtime_generic_equation_pack_interactive_ui.py -q --junitxml artifacts/pytest/generic_cuda_equation_pack_color_pipeline_runtime.junit.xml` passed with `4 passed`, including the Color Pipeline scale-control frame-hash witness.
- Code-quality proof: complete. `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/logs/generic_cuda_equation_pack_color_pipeline_code_quality.json` passed with score `97/100` and no critical/error findings.
- Final focused native proof: complete. `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_color_pipeline_native_final --log artifacts/logs/generic_cuda_equation_pack_color_pipeline_native_final.log -- ui_app/build_tests_vsdevcmd.cmd test_generic_equation_pack_live` passed.
- Final Color Pipeline guard proof: complete. `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_color_pipeline_guard_native_final --log artifacts/logs/generic_cuda_equation_pack_color_pipeline_guard_native_final.log -- ui_app/build_tests_vsdevcmd.cmd advanced_color_grading_red` passed.
- Broad native regression proof: complete. `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_color_pipeline_full_native_final --log artifacts/logs/generic_cuda_equation_pack_color_pipeline_full_native_final.log -- ui_app/build_tests_vsdevcmd.cmd` passed with `All helper tests passed.`
- Whitespace proof: complete. `git diff --check` passed.

## Hostile Audit

- Status: complete
- Required questions before closeout:
  - Did `generic_equation_pack` still appear as a normal Fractal Type dropdown lane? Yes; the selector lane from the prior slice stayed intact and `main.cpp` still calls the generic live renderer only for `FractalType::generic_equation_pack`.
  - Did the left Controls JSON/pack controls remain available and no-mouse automatable? Yes; the runtime test still automates `equation_pack.json_text`, `equation_pack.apply_json`, and `equation_pack.steps.primary`.
  - Did generic equation-pack viewport coloring actually route through existing Color Pipeline signal, shape, palette, and grading semantics? Yes; the live renderer now maps `GenericSampleResult` through `MakeEscapeTimeBaseColor(...)` and `ApplyFractalColorGrading(...)` using live `KernelParams`.
  - Did at least one existing Color Pipeline numeric control change the generic equation-pack rendered frame on the published viewer path? Yes; the no-mouse runtime test changes `color_pipeline.source.smooth_escape_ramp.signal.scale.primary` and asserts a changed frame hash.
  - Did existing fixed fractals still route through their normal renderer and Color Pipeline behavior without product mutation? Yes; only the generic equation-pack live-render seam and its call site were touched, and the Color Pipeline guard bundle plus full native suite passed.
  - Did this slice avoid private palette fallback, dynamic kernel registration, Salticid parser work, or broad renderer rewrites? Yes; the private palette was removed from the generic live path and no dynamic kernel/Salticid/parser work was added.

## Audit Passes

- [x] Pass 1 - found and removed the remaining private color path in `generic_equation_pack_live.cpp`; the repaired path now consumes live `KernelParams` and Color Pipeline helpers.
- [x] Pass 2 - challenged runtime proof so it had to prove published viewer Color Pipeline control sensitivity; clean re-read confirmed the test mutates the Color Pipeline scale control, not only the pack parameter.
- [x] Pass 3 - re-read the repaired state, contract, Color Pipeline guards, phased plan, and stale closeout wording; no additional real defect found after the contract target correction and final validation reruns.

## Audit Findings

- [x] Real product defect: the previous main-viewport bridge used a private `LivePixelFromResult(...)` CPU palette, so generic equation-pack viewport color lived outside the Color Pipeline even though the feature sits in the normal viewport.
- [x] Real workflow defect: the first contract named `test_color_pipeline_core` as a direct focused target even though the exposed native rail is the `advanced_color_grading_red` bundle; the contract was corrected, re-locked, and validated.
- [x] Clean re-read: after the product and workflow repairs, no additional real issue found in the generic live-render seam, Color Pipeline guard rail, no-mouse runtime proof, or plan text.

## Notes

- The prior main-viewport slice remains valid as an intermediate bridge, but this slice replaces its private generic-pack color path with existing Color Pipeline semantics.
- The implementation passes live `KernelParams` into the generic pack live renderer and reuses `MakeEscapeTimeBaseColor(...)` plus `ApplyFractalColorGrading(...)` for visible generic equation-pack frames.
- Deferred unless directly blocking: saved-state persistence for edited packs, pack catalogs, Salticid adapters, dynamic kernel generation, and broader renderer replacement.
