# Color Pipeline Mixed Source Rows

## Current Phase

Active - Phase 6 validation and closure. Implementation is landed in the working tree; focused native rails, runtime publish, published no-mouse runtime proof, capture/replay preservation, and bounded performance witnesses are green. Checkpoint receipts, rearward review, push, and clean-tree proof are the remaining workflow actions for this slice.

## Phase Checklist

- [x] Phase 1 - create this checked-in phased plan and contract, then lock the active slice on stacked branch `codex/color-pipeline-mixed-source-rows`.
- [x] Phase 2 - add RED/native coverage proving mixed SDF plus non-SDF Source rows currently fail closed and source-signal sidecar authority is missing.
- [x] Phase 3 - add renderer source-signal sidecar for supported non-SDF Source rows without changing existing `RenderFractalCUDA(...)` callers.
- [x] Phase 4 - extend SDF postprocess planning to consume mixed SDF-field and renderer-signal rows with signal-exact ordered blending.
- [x] Phase 5 - remove the UI bridge mixed-source fail-closed rule while preserving disabled-row, root-basin, pure-SDF CUDA, and pure non-SDF behavior.
- [x] Phase 6 - publish runtime, prove no-mouse mixed Source rows, capture/replay, reports, pure-SDF preservation, performance witness, hostile audit, receipts, push, and clean tree.

## Explicit User Asks

- [x] Implement the Color Pipeline Mixed Source Rows plan.
- [x] Start a stacked branch from clean pushed `c9403a2`.
- [x] Remove the current “all SDF rows or all non-SDF rows” limitation.
- [x] Use signal-exact Source-stack semantics, not color overlay blending.
- [x] Keep existing `RenderFractalCUDA(...)` API working unchanged.
- [x] Add a dedicated renderer helper/overload that emits one scalar Source signal plane per enabled Source row index.
- [x] Preserve row order and sequential `blend_weight` semantics before Shape, Palette, and Grading.
- [x] Keep disabled rows inactive.
- [x] Keep `root_index` on the separate root-basin pair path and out of this mixed generic Source-stack slice.
- [x] Preserve pure SDF CUDA-backed paths and pure non-SDF behavior.
- [x] Keep `sdf_pack_scene` SDF-only unless a renderer-backed non-SDF signal is available.
- [x] Report `source_stack_kind` and mixed source-signal-frame usage.
- [x] Prove native, renderer, no-mouse runtime, capture/replay, and bounded performance behavior.

## Scope

In scope:

- Mixed SDF and non-SDF Source rows for supported non-root Source signals.
- Renderer-side scalar signal planes for enabled Source row indices.
- CPU mixed SDF postprocess if needed; pure SDF CUDA direct/field paths must stay available.
- Existing Color Pipeline layout and existing Source-row storage.
- Focused native and published no-mouse runtime proof.

Out of scope:

- Color overlay semantics.
- New Color Pipeline functions, new SDF ops, new fractal types, or broad UI redesign.
- Generic root-basin pair mixing with SDF rows.
- CUDA mixed-stack postprocess unless the performance witness proves it is the next bottleneck.
- Physical mouse automation.
- `generic_equation_pack` mixed Source rows until that renderer has a real source-signal producer.
- `sdf_pack_scene` mixed Source rows until authored SDF pack scenes expose renderer-backed non-SDF source signals.

## Proof Ledger

- Start authority: stacked from clean pushed `c9403a2` on `codex/color-pipeline-applicative-glue`; rearward review for `c9403a2` was `ok`.
- Active slice lock: `ck:fc6e0b95`, plan `docs/notes/color_pipeline_mixed_source_rows_PHASED_PLAN.md`, contract `docs/contracts/color_pipeline_mixed_source_rows.contract.json`.
- RED window proof: `artifacts/logs/color_pipeline_mixed_source_rows_window_red.log` failed because mixed SDF plus non-SDF Source rows were still rejected by the UI bridge.
- RED SDF postprocess proof: `artifacts/logs/color_pipeline_mixed_source_rows_sdf_postprocess_red.log` failed because mixed postprocess lacked a renderer source-signal frame.
- RED renderer proof: `artifacts/logs/color_pipeline_mixed_source_rows_renderer_red.log` failed with unresolved `RenderFractalCUDAWithColorSourceSignals`.
- Replay RED found during hostile audit: `artifacts/logs/color_pipeline_mixed_source_rows_state_replay_red.log` failed with `color_source_stack final entry must mirror the saved flat color pipeline signal` for mixed captured state.
- State replay fix proof: `artifacts/logs/color_pipeline_mixed_source_rows_focused_native_diagnostics_state.log` passed `test_diagnostics_state_io: all passed`.
- Window proof: `artifacts/logs/color_pipeline_mixed_source_rows_focused_native_window.log` passed `test_color_pipeline_window: passed=232 failed=0`.
- SDF postprocess proof: `artifacts/logs/color_pipeline_mixed_source_rows_focused_native_sdf_postprocess.log` passed `test_color_pipeline_sdf_postprocess: passed=118 failed=0`.
- Renderer sidecar proof: `artifacts/logs/color_pipeline_mixed_source_rows_focused_native_renderer.log` passed `test_fractal_renderer: passed=80 failed=0`.
- Pure SDF CUDA preservation: `artifacts/logs/color_pipeline_mixed_source_rows_focused_native_sdf_postprocess_cuda.log` passed `test_color_pipeline_sdf_postprocess_cuda: passed=130 failed=0`.
- Runtime publish proof: `artifacts/logs/color_pipeline_mixed_source_rows_publish_fail_closed_final.log` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`; after the renderer refactor, `artifacts/logs/color_pipeline_mixed_source_rows_publish_after_refactor.log` also passed against the final source tree.
- Published mixed runtime proof: `artifacts/logs/color_pipeline_mixed_source_rows_runtime_mixed_final.log` passed the no-mouse mixed Source-row test; after the renderer refactor, `artifacts/logs/color_pipeline_mixed_source_rows_runtime_mixed_after_refactor.log` passed the same published-runtime proof.
- Published preservation lane: `artifacts/logs/color_pipeline_mixed_source_rows_runtime_preservation_lane.log` passed 14 runtime tests across SDF rows, capture/replay authority, and Color Pipeline presets; after the renderer refactor, `artifacts/logs/color_pipeline_mixed_source_rows_runtime_preservation_after_refactor.log` passed the same 14-test lane.
- Pure SDF performance witness: `artifacts/color_pipeline_mixed_source_rows/sdf_performance_witness.json` and `.md` recorded the bounded SDF witness; recommendation was `mixed_or_inconclusive_measurement_review_required`, so no broad performance-improvement claim is made.
- Mixed stack timing witness: `artifacts/color_pipeline_mixed_source_rows/mixed_stack_timing_summary.json` recorded `source_stack_kind=mixed`, `mixed_source_signal_frame_used=true`, `lens_sdf_backend_used=cuda_jfa`, and a 512x384 mixed-stack `lens_sdf_total_ms` of about 8.85 ms on this run.
- Diff hygiene: `git diff --check` passed after EOF cleanup and again after the renderer refactor in `artifacts/logs/color_pipeline_mixed_source_rows_diff_check_after_refactor.log`.
- Renderer refactor proof after code-quality finding: `artifacts/logs/color_pipeline_mixed_source_rows_renderer_after_refactor.log` passed `test_fractal_renderer: passed=80 failed=0`.
- Code-quality baseline proof after renderer refactor: `artifacts/validation/color_pipeline_mixed_source_rows_code_quality.json` passed with score 93/100 and no baseline regressions.

## Action Hostile Review

- Action ID: mixed_source_signal_exact_v1
- Suspected Failure Mode: a quick fix could delete the UI fail-closed check while leaving runtime mixed stacks unsupported, or could blend already-colored pixels instead of scalar Source signals.
- Correct Owner/Action: REDs were added at window, SDF postprocess, diagnostics state replay, and renderer sidecar seams before claiming closure.
- Proof Surface: `test_color_pipeline_window`, `test_color_pipeline_sdf_postprocess`, `test_color_pipeline_sdf_postprocess_cuda`, `test_diagnostics_state_io`, `test_fractal_renderer`, published no-mouse runtime Color Pipeline tests, capture/replay tests, and bounded performance witnesses.
- Blocked Action: claiming generic equation-pack or SDF pack scene mixed Source rows; those fail closed until a renderer-backed non-SDF signal source exists.

## Hostile Audit

- Status: complete
- Result: three real implementation/workflow defects were found and repaired during hostile review; no remaining in-scope defect was found after the final re-read.

Required questions:

- Did mixed SDF and non-SDF Source rows apply instead of fail closed? Yes; window and runtime tests prove mixed `smooth_escape + sdf_signed_distance` applies.
- Did row values blend as scalar Source signals in author order before Shape/Palette/Grading? Yes; SDF postprocess consumes renderer scalar planes and SDF scalar rows before existing Shape/Palette/Grading.
- Did changing the non-SDF row alter mixed output? Yes; no-mouse runtime hash changes after setting `color_pipeline.source.smooth_escape_ramp.signal.scale.primary`.
- Did changing the SDF row or applicator alter mixed output? Yes; no-mouse runtime hash changes after setting `color_pipeline.source.sdf_signed_distance.signal.scale.primary`, and native SDF applicator rails remain green.
- Did disabled rows stay inactive for compatibility, reports, runtime, capture, and replay? Yes; existing disabled-row window/runtime preservation rails remain in the runtime preservation lane.
- Did pure SDF CUDA direct/field paths remain green? Yes; `test_color_pipeline_sdf_postprocess_cuda` passed with 130 checks.
- Did existing `RenderFractalCUDA(...)` callers remain source-compatible? Yes; existing renderer tests pass and the old API delegates to the sidecar overload with no signal frame.
- Did no-mouse runtime proof exercise the published viewer path? Yes; runtime lane used `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Did the slice avoid new functions, new SDF ops, new fractal types, broad UI redesign, and physical mouse automation? Yes.

## Audit Passes

- [x] Pass 1 - RED/native gap audit.
- [x] Pass 2 - renderer source-signal sidecar audit.
- [x] Pass 3 - mixed SDF postprocess semantics audit.
- [x] Pass 4 - UI/report/capture/runtime audit.
- [x] Pass 5 - pure SDF CUDA and performance preservation audit.
- [x] Pass 6 - final clean re-read after validation.

## Audit Findings

- [x] Finding: the initial mixed implementation allowed headless captures but the diagnostics state loader still rejected replay/live startup unless the final Source row mirrored the flat `color_signal`. Added `test_diagnostics_state_io` coverage for mixed first-row flat authority and updated the loader to let SDF-containing stacks mirror from the base or final row.
- [x] Finding: the live path initially allowed `generic_equation_pack` mixed Source rows to allocate a source-signal frame without filling it. Added explicit fail-closed behavior for `generic_equation_pack` and `sdf_pack_scene` until those lanes have renderer-backed non-SDF source signals.
- [x] Finding: the code-quality baseline caught a `max_fn_lines` regression in `ensure_buffers` after the source-signal buffer work. Split cached buffer free/reset/allocation helpers, reran `test_fractal_renderer`, and reran the code-quality audit green.
- [x] Clean re-read: after repairing the state loader, generic-equation-pack fail-closed path, and renderer helper size regression, the final validation pass did not expose another in-scope mixed Source-row defect. Runtime publish can exceed 240 seconds on this machine; the successful publish used a 600-second wrapper timeout.

## Planned Validation Targets

- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/color_pipeline_mixed_source_rows.contract.json --out-json artifacts/validation/color_pipeline_mixed_source_rows_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/color_pipeline_mixed_source_rows_PHASED_PLAN.md --out-json artifacts/validation/color_pipeline_mixed_source_rows_hostile_audit.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_mixed_source_rows_window --log artifacts/logs/color_pipeline_mixed_source_rows_window.log --out-json artifacts/validation/color_pipeline_mixed_source_rows_window.json --heartbeat-seconds 30 --timeout-seconds 900 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_color_pipeline_window`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_mixed_source_rows_sdf_postprocess --log artifacts/logs/color_pipeline_mixed_source_rows_sdf_postprocess.log --out-json artifacts/validation/color_pipeline_mixed_source_rows_sdf_postprocess.json --heartbeat-seconds 30 --timeout-seconds 900 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_color_pipeline_sdf_postprocess`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_mixed_source_rows_sdf_postprocess_cuda --log artifacts/logs/color_pipeline_mixed_source_rows_sdf_postprocess_cuda.log --out-json artifacts/validation/color_pipeline_mixed_source_rows_sdf_postprocess_cuda.json --heartbeat-seconds 30 --timeout-seconds 900 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_color_pipeline_sdf_postprocess_cuda`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_mixed_source_rows_renderer --log artifacts/logs/color_pipeline_mixed_source_rows_renderer.log --out-json artifacts/validation/color_pipeline_mixed_source_rows_renderer.json --heartbeat-seconds 30 --timeout-seconds 900 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_fractal_renderer`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_mixed_source_rows_publish --log artifacts/logs/color_pipeline_mixed_source_rows_publish.log --out-json artifacts/validation/color_pipeline_mixed_source_rows_publish.json --heartbeat-seconds 30 --timeout-seconds 1200 -- cmd /c ui_app\build_vsdevcmd.cmd`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_mixed_source_rows_runtime --log artifacts/logs/color_pipeline_mixed_source_rows_runtime.log --out-json artifacts/validation/color_pipeline_mixed_source_rows_runtime.json --heartbeat-seconds 30 --timeout-seconds 1800 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py tests/test_fractal_runtime_capture_replay_authority.py tests/test_fractal_runtime_color_pipeline_presets.py`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/color_pipeline_mixed_source_rows_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_mixed_source_rows_diff_check --log artifacts/logs/color_pipeline_mixed_source_rows_diff_check.log --out-json artifacts/validation/color_pipeline_mixed_source_rows_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`
