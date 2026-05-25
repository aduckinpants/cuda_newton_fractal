# Live GPU Lens SDF Integration

## Current Phase

Closed - live GPU Lens SDF integration is implemented, audited, and validated.

## Phase Checklist

- [x] Phase 1 - create and lock this checked-in plan/contract
- [x] Phase 2 - add RED no-mouse runtime proof for live Lens SDF backend reporting
- [x] Phase 3 - repair the visible checkbox no-mouse set-value seam needed by the Lens proof
- [x] Phase 4 - route live Lens SDF generation through the explicit `auto` backend with CPU fallback
- [x] Phase 5 - expose Lens SDF backend report fields in the viewer automation report
- [x] Phase 6 - publish runtime and run focused native/runtime preservation rails
- [x] Phase 7 - run hostile audit, repair findings, checkpoint, receipts, rearward review, push

## Explicit User Asks

- [done] Route live Lens SDF generation through the explicit `auto` backend.
- [done] Prefer CUDA when available and fall back to CPU chamfer on CUDA failure.
- [done] Preserve the current RGBA Lens SDF visualization and existing Lens UI controls.
- [done] Report `lens_sdf_backend_used` and related backend/dimension state in runtime automation reports.
- [done] Add a no-mouse runtime proof for Lens enabled, CUDA backend used, downsample-respecting SDF output, and unchanged base fractal frame.
- [done] Do not add live Color Pipeline SDF rows, viewport overlay productization, SDF-native fractal lanes, or mouse automation in this slice.

## Scope

In scope:

- `ui_app/src/main.cpp`
- `ui_app/src/viewer_ui_automation_report.h`
- `ui_app/src/viewer_ui_automation_report.cpp`
- `ui_app/src/schema_binding.cpp`
- `ui_app/tests/test_schema_binding.cpp`
- `ui_app/build_vsdevcmd.cmd`
- `tests/test_fractal_runtime_lens_sdf_backend.py`
- this phased plan and its contract
- `HANDOFF_LOG.md`

Out of scope:

- Color Pipeline SDF source rows.
- Viewport overlay productization.
- SDF-native fractal lanes.
- Renderer replacement or passing Lens SDF textures into fractal kernels.
- Physical mouse or OS cursor automation.
- Claiming a client performance improvement without a bounded timing witness.

## Proof Ledger

- Start authority: branch `codex/live-gpu-lens-sdf-integration` starts from clean head `a24f6a22b1bcee724e1c7ceff92238b32cf5c348`, where the explicit CUDA Lens SDF backend is implemented and CPU default is preserved.
- RED authority: `$env:VIEWER_HOST_ENABLE_RUNTIME_VIEWER_E2E='1'; py -3.14 -m pytest tests/test_fractal_runtime_lens_sdf_backend.py -q` fails because the in-process set-value harness rejects the visible checkbox `fractal_control.lens_enabled.primary`.
- Expected next RED after checkbox repair: the focused runtime no-mouse pytest must still fail before live backend integration because the published viewer automation report does not expose `lens_sdf_backend_used` and the runtime build does not link `lens_sdf_cuda.cu`.
- Schema harness repair: `ui_app/tests/test_schema_binding.cpp` now carries the native check that visible checkbox set-value automation writes `fractal.lens.enabled` through the existing schema edit/dirty/interacted path.
- Invalid proof rejected: a nested PowerShell runtime pytest command expanded `$env:VIEWER_HOST_ENABLE_RUNTIME_VIEWER_E2E` incorrectly and produced a skip-only run; this is not accepted as evidence, and the contract runtime command now uses `cmd /c set ...&& pytest`.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label live_gpu_lens_sdf_integration_runtime_publish --log artifacts/logs/live_gpu_lens_sdf_integration_runtime_publish.log --out-json artifacts/validation/live_gpu_lens_sdf_integration_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- cmd /c ui_app\build_vsdevcmd.cmd` passed, compiling and linking `lens_sdf_cuda.cu` into the published runtime.
- Runtime proof: `py -3.14 tools/viewer_host_run_logged_command.py --label live_gpu_lens_sdf_integration_runtime_pytest --log artifacts/logs/live_gpu_lens_sdf_integration_runtime_pytest.log --out-json artifacts/validation/live_gpu_lens_sdf_integration_runtime_pytest.json --heartbeat-seconds 30 --timeout-seconds 300 -- cmd /c "set VIEWER_HOST_ENABLE_RUNTIME_VIEWER_E2E=1&& py -3.14 -m pytest tests/test_fractal_runtime_lens_sdf_backend.py -q --junitxml artifacts/pytest/live_gpu_lens_sdf_integration_runtime.junit.xml"` passed with `tests=1`, `skipped=0`.
- Native schema proof: `py -3.14 tools/viewer_host_run_logged_command.py --label live_gpu_lens_sdf_integration_test_schema_binding --log artifacts/logs/live_gpu_lens_sdf_integration_test_schema_binding.log --out-json artifacts/validation/live_gpu_lens_sdf_integration_test_schema_binding.json --heartbeat-seconds 30 --timeout-seconds 180 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_schema_binding` passed.
- Native Lens SDF preservation: focused `test_lens_sdf_cuda` and `test_lens_sdf` logged rails passed.
- Code-quality status: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/live_gpu_lens_sdf_integration_code_quality_dev.json` passed at baseline score `96/100`.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/live_gpu_lens_sdf_integration.contract.json --out-json artifacts/validation/live_gpu_lens_sdf_integration_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile-audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/live_gpu_lens_sdf_integration_PHASED_PLAN.md --out-json artifacts/validation/live_gpu_lens_sdf_integration_hostile_audit.json` passed.
- Diff hygiene: `py -3.14 tools/viewer_host_run_logged_command.py --label live_gpu_lens_sdf_integration_diff_check --log artifacts/logs/live_gpu_lens_sdf_integration_diff_check.log --out-json artifacts/validation/live_gpu_lens_sdf_integration_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.
- Performance claim status: no client FPS/performance improvement is claimed by this slice; the proof is backend availability/reporting and behavior preservation only.
- Required preservation: `ComputeLensSdfFieldForMask(...)` remains CPU-default; live routing uses the explicit `ComputeLensSdfFieldForMaskWithBackend(..., LensSdfBackend::auto_backend, ...)` API.
- Runtime proof target: published runtime with Lens enabled reports CUDA backend on this machine, emits valid Lens SDF dimensions consistent with `lens.downsample`, and leaves the base fractal `rendered_frame_hash` unchanged compared with Lens disabled.

## Hostile Audit

- Status: done
- Required posture: assume live routing silently stayed on CPU, CUDA was not linked into the runtime, the report fields lie or stay stale, Lens enabled mutates the base fractal render, fallback is unreported, or this slice accidentally starts deferred Color Pipeline/overlay/SDF-native work.

## Audit Passes

- [done] Pass 1 - re-read the diff and proved visible checkbox set-value automation is scoped to `RenderCheckboxControl`/`TryApplyPrimaryUiAutomationSetValue`, uses no physical mouse path, and is covered by `test_schema_binding`.
- [done] Pass 2 - proved the runtime executable actually links the CUDA Lens SDF object through `build_vsdevcmd.cmd` compiling `lens_sdf_cuda.cu` and the published runtime proof reporting `cuda_jfa`.
- [done] Pass 3 - proved the automation report fields are updated by the current frame: the runtime JUnit asserts enabled state, `cuda_jfa`, fallback false, 2x dimensions, pixel scale, and unchanged base frame hash.
- [done] Pass 4 - clean re-read the repaired state: existing Lens UI controls and RGBA visualization are preserved, `ComputeLensSdfFieldForMask(...)` stays CPU-default, and Color Pipeline rows, viewport overlay, and SDF-native lanes remain untouched.

## Audit Findings

- [done] Real RED finding repaired: the no-mouse runtime harness could not set a visible schema checkbox, so the Lens proof could not enable Lens without a physical click or special-case command; boolean schema set-value now uses the shared schema edit path and `test_schema_binding` covers `fractal.lens.enabled`.
- [done] Real workflow finding repaired: the first JUnit runtime command was skip-only because nested PowerShell quoting stripped `$env:VIEWER_HOST_ENABLE_RUNTIME_VIEWER_E2E`; the contract command was changed to `cmd /c set ...&& pytest`, rerun, and the JUnit now records one executed test and zero skips.
- [done] Clean re-read: live routing calls the explicit `auto` backend and reports `cuda_jfa`, while CPU fallback/reporting remains available through `LensSdfBackendReport`.
- [done] Clean re-read: deferred SDF consumers remain deferred; this slice does not add Color Pipeline SDF rows, viewport overlay productization, or SDF-native fractal lanes.
