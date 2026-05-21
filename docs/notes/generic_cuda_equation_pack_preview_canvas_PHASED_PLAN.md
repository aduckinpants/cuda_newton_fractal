# Generic CUDA Equation Pack Preview Canvas

## Current Phase

Complete - preview-canvas slice validated on this head; checkpoint, machine receipts, push, clean tree, and stale-plan gate are represented by the final repo state for this slice.

## Explicit User Asks

- [x] Take the next forward slice toward the full Generic CUDA equation-pack feature.
- [x] Make the interactive workbench more feature-complete without drifting into a renderer rewrite.
- [x] Preserve AST JSON authority, Generic CUDA execution, no-mouse runtime proof, and existing Color Pipeline behavior.
- [x] Do not add dynamic CUDA kernels, a new `FractalType`, live viewport renderer integration, or Salticid `sample_fn` syntax in this slice.

## Phase Checklist

- [x] Phase 0 - open this phased plan, create the checked-in contract, and lock the active slice.
- [x] Phase 1 - add RED native/runtime tests proving the workbench lacks a real preview-image contract.
- [x] Phase 2 - derive preview image pixels and image hash from AST-backed Generic CUDA sample results.
- [x] Phase 3 - render the preview canvas inside the workbench and publish image metadata in the no-mouse automation report.
- [x] Phase 4 - validate focused native/runtime rails, full native, runtime publish, and hostile audit.
- [x] Phase 5 - checkpoint, receipts, push, clean tree, and stale-plan check.

## Proof Ledger

- Starting branch: `codex/engine-architecture-review`.
- Starting head: `b32df8f`.
- Bootstrap status: clean tree; previous active contract was `generic_cuda_equation_pack_interactive_ui` and must be replaced before mutation.
- Existing authority: the previous slice added the Controls-window workbench entry point, pack-schema sliders, no-mouse set-value automation, and a CUDA preview result hash.
- New authority target: a visible preview canvas in the workbench whose dimensions, pixels, and image hash are derived from the same `SampleGenericFunction` preview samples.
- Out-of-scope seams: `RenderFractalCUDA`, `FractalType`, dynamic CUDA kernel registration, Salticid adapter syntax, Color Pipeline product behavior, and physical mouse automation.
- RED proof: `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_preview_canvas_red_native --log artifacts/logs/generic_cuda_equation_pack_preview_canvas_red_native.log -- ui_app/build_tests_vsdevcmd.cmd test_generic_equation_pack_workbench_ui` failed because `preview_image_width`, `preview_image_height`, `preview_image_hash`, and `pixels_rgba` were absent.
- Implementation proof: `GenericEquationPackWorkbenchPreviewSummary` now stores deterministic RGBA pixels plus image dimensions/hash derived from `SampleGenericFunction` results; the workbench draws those pixels as an ImGui preview canvas and the automation report exports the image metadata.
- Focused native proof: `artifacts/logs/generic_cuda_equation_pack_preview_canvas_focused_native_after_audit.log` reports `test_generic_equation_pack_workbench_ui: passed=24`.
- Runtime publish proof: `artifacts/logs/generic_cuda_equation_pack_preview_canvas_runtime_publish_final.log` reports `result=success` and stages `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- No-mouse runtime proof: `py -3.14 -m pytest tests/test_fractal_runtime_generic_equation_pack_interactive_ui.py -q --junitxml artifacts/pytest/generic_cuda_equation_pack_preview_canvas_runtime.junit.xml` reports `1 passed`; the test waits for `equation_pack.preview_canvas`, uses set-value automation, asserts `viewer.launch_count == 1`, and proves both result and image hashes change.
- Full native proof: `py -3.14 tools/viewer_host_run_logged_command.py --label generic_cuda_equation_pack_preview_canvas_native_final --log artifacts/logs/generic_cuda_equation_pack_preview_canvas_native_final.log -- ui_app/build_tests_vsdevcmd.cmd` reports `result=success` and `All helper tests passed`.
- Code-quality proof: `artifacts/logs/generic_cuda_equation_pack_preview_canvas_code_quality.json` reports baseline check passed with score 97/100.
- Contract proof: `artifacts/validation/generic_cuda_equation_pack_preview_canvas_contract.json` reports `ok: true`.
- Hostile-audit proof: `artifacts/validation/generic_cuda_equation_pack_preview_canvas_hostile_audit.json` reports `ok: true`.
- Plan-sync proof: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` reports `OK` for this plan and writes `artifacts/validation/viewer_host_assert_phased_plan_sync.json`.

## Hostile Audit

- Status: done

## Audit Passes

- [x] Pass 1 - real defect found: the first ImGui runtime build broke because a local `max` canvas variable collided with the Windows `max` macro; fixed by renaming it to `rectMax`.
- [x] Pass 2 - real UI hardening issue found: the first canvas sizing pass forced a 96px minimum even if the content region was narrower; fixed so the canvas honors available width down to a bounded 32px floor.
- [x] Pass 3 - clean re-read the repaired state: preview pixels are pushed from every `GenericSampleResult`, `preview_image_hash` hashes those pixels plus dimensions, and control edits are proven to change both result and image hashes.
- [x] Pass 4 - clean re-read confirmed the repaired state did not touch `RenderFractalCUDA`, `FractalType`, dynamic CUDA kernel registration, Salticid adapter syntax, or Color Pipeline product behavior; runtime proof stays no-mouse with one persistent viewer process.

## Audit Findings

- [x] Real defect found and repaired: Windows `max` macro collision in the first canvas implementation broke runtime compilation until the rectangle local was renamed to `rectMax`.
- [x] Real UI hardening issue found and repaired: the canvas now honors the available ImGui content width instead of forcing avoidable overflow.
- [x] Clean re-read evidence: focused native, runtime publish, no-mouse runtime proof, and full native helper tests all pass on the repaired state.
