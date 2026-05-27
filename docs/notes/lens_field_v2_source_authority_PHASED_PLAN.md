# Lens Field V2 Source Authority

## Current Phase

Closed - Lens Field v2 source authority validated for checkpoint.

## Phase Checklist

- [x] Phase 0 - start from clean `e5acaf4` with rearward review `ok` and branch `codex/lens-field-v2-source-authority`.
- [x] Phase 1 - open this checked-in plan/contract and lock the active slice.
- [x] Phase 2 - add RED native/runtime proof for Lens Field v2 source identity, GPU-backed field reporting, and legacy Lens preservation.
- [x] Phase 3 - implement the smallest Lens Field v2 Color Pipeline source authority.
- [x] Phase 4 - publish runtime and prove the no-mouse viewer path.
- [x] Phase 5 - hostile audit, plan sync, receipt-ready validation, rearward-review-ready proof, and clean checkpoint preparation.

## Explicit User Asks

- [active] Modernize the original Lens SDF idea, not just expose the old debug image as a source.
- [active] Keep the original Lens panel/debug/overlay path as-is for now.
- [active] Make the modern path GPU-backed and usable from the Color Pipeline source-function system.
- [active] Preserve the current SDF performance/field-resolution work and do not regress capture/replay quality.

## Scope

In scope:

- Add a distinct Lens Field v2 Color Pipeline source identity backed by the existing Lens SDF field substrate.
- Keep existing `sdf_signed_distance`, `sdf_inside_outside`, `sdf_boundary_band`, `sdf_normal_angle`, and `sdf_curvature` rows working.
- Prove the new source uses the GPU Lens SDF field path on the published runtime when CUDA is available.
- Preserve the legacy Lens panel, Lens SDF aux visualization, viewport overlay, capture/replay, and adaptive field-resolution reporting.

Out of scope:

- Replacing the legacy Lens panel or deleting `ComputeLensSdfRgbaForMask(...)`.
- Per-row/per-function downsample authority.
- Multi-field composition, authored SDF pack UI, SDF-native fractal lanes, or a broad Color Pipeline UI redesign.
- Replacing CUDA JFA or changing the field-generation algorithm.

## Implementation Direction

The product shape is a modernized Lens Field source, not a copy of the old grayscale debug image. The initial implementation should introduce one distinct source-function identity, tentatively `lens_field_v2_distance`, that represents the reusable Lens-derived signed-distance field as a Color Pipeline source. It may reuse the same underlying signed-distance field math, but its public source identity must not collapse to the old `sdf_signed_distance` row in descriptors, state import/export, automation, or reports.

The old Lens panel remains a legacy/debug consumer of the same field substrate. The new path should ride the existing live GPU field generation and SDF postprocess backends, and report the same requested/effective downsample, backend, and quality fields already added by the realtime slice.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on clean `codex/sdf-field-resolution-realtime` at `e5acaf4`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported a clean tree.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `e5acaf4`.
- Branch: `codex/lens-field-v2-source-authority` was created from `e5acaf4`.
- Contract bootstrap: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/lens_field_v2_source_authority.contract.json --out-json artifacts/validation/lens_field_v2_contract_bootstrap.json` passed.
- Plan sync bootstrap: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "Lens Field v2 source authority" --profile runtime --plan docs/notes/lens_field_v2_source_authority_PHASED_PLAN.md --contract docs/contracts/lens_field_v2_source_authority.contract.json` appended `ck:1c87b1e1` and locked the active contract.
- RED native proof: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_field_v2_red_native --log artifacts/logs/lens_field_v2_red_native.log --out-json artifacts/validation/lens_field_v2_red_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core test_color_pipeline_sdf_postprocess test_lens_sdf` failed because `ColorSignal::lens_field_v2_distance` was not yet implemented.
- Native core proof: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_field_v2_native_core --log artifacts/logs/lens_field_v2_native_core.log --out-json artifacts/validation/lens_field_v2_native_core.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core test_color_pipeline_sdf_postprocess test_lens_sdf` passed with `test_color_pipeline_core: passed=2484 failed=0`.
- Native window/report proof: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_field_v2_native_window --log artifacts/logs/lens_field_v2_native_window.log --out-json artifacts/validation/lens_field_v2_native_window.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_window test_viewer_ui_automation_report` passed with `test_color_pipeline_window: passed=210 failed=0`.
- Native schema-binding proof: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_field_v2_native_schema_binding --log artifacts/logs/lens_field_v2_native_schema_binding.log --out-json artifacts/validation/lens_field_v2_native_schema_binding.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_schema_binding` passed.
- UI-Salt materializer proof: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_field_v2_ui_salt_materializer --log artifacts/logs/lens_field_v2_ui_salt_materializer.log --out-json artifacts/validation/lens_field_v2_ui_salt_materializer.json --heartbeat-seconds 30 --timeout-seconds 300 -- py -3.14 -m pytest tests/test_ui_salt_materializer.py` passed with `8 passed`.
- Runtime publish proof: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_field_v2_runtime_publish --log artifacts/logs/lens_field_v2_runtime_publish.log --out-json artifacts/validation/lens_field_v2_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published runtime SDF rows proof: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_field_v2_runtime_rows --log artifacts/logs/lens_field_v2_runtime_rows.log --out-json artifacts/validation/lens_field_v2_runtime_rows.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py` passed with `8 passed`; the new Lens Field v2 test asserts `lens_sdf_enabled=false`, `lens_sdf_valid=true`, `lens_sdf_backend_used=cuda_jfa`, `lens_sdf_postprocess_backend_used=cuda_direct_scalar`, no fallback, requested quality mode, and frame-hash change from the visible source control.
- Published runtime pacing proof: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_field_v2_runtime_pacing --log artifacts/logs/lens_field_v2_runtime_pacing.log --out-json artifacts/validation/lens_field_v2_runtime_pacing.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_sdf_realtime_pacing.py` passed with `1 passed`.
- Published runtime capture/replay proof: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_field_v2_runtime_capture_replay --log artifacts/logs/lens_field_v2_runtime_capture_replay.log --out-json artifacts/validation/lens_field_v2_runtime_capture_replay.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_capture_replay_authority.py` passed with `1 passed`.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/lens_field_v2_source_authority.contract.json --out-json artifacts/validation/lens_field_v2_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed after proof-ledger and audit updates.
- Hostile audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/lens_field_v2_source_authority_PHASED_PLAN.md --out-json artifacts/validation/lens_field_v2_hostile_audit.json` passed with real findings and a clean re-read pass.
- Code quality: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/lens_field_v2_code_quality.json` passed, score `93/100`, baseline check passed.
- Diff check: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_field_v2_diff_check --log artifacts/logs/lens_field_v2_diff_check.log --out-json artifacts/validation/lens_field_v2_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.

## Hostile Audit

- Status: complete
- Did I actually create a distinct Lens Field v2 source identity instead of aliasing the old SDF signed-distance row? Current evidence: yes; native core/schema/window tests cover the `lens_field_v2_distance` enum id, catalog descriptor, source order, UI-Salt metadata, state/capture ids, and automation control id.
- Did the new source use the GPU Lens SDF field path on the published runtime when CUDA is available? Current evidence: yes; published runtime row proof asserts `lens_sdf_backend_used=cuda_jfa` and `lens_sdf_postprocess_backend_used=cuda_direct_scalar` for the new visible source row.
- Did I leave the legacy Lens panel/debug/overlay path intact? Current evidence: yes; no legacy Lens renderer or panel replacement was made, and `test_lens_sdf` plus runtime pacing/capture replay rails stayed green.
- Did I preserve capture/replay full-quality semantics and adaptive requested/effective downsample reporting? Current evidence: yes; capture/replay and SDF realtime pacing runtime rails stayed green after the source addition.
- Did I avoid per-row downsample, multi-field composition, authored SDF UI, SDF-native lanes, and broad Color Pipeline redesign? Current evidence: yes; the diff only adds one source identity and matching metadata/proof.

## Audit Passes

- [x] Pass 1 - RED/source identity proof.
- [x] Pass 2 - implementation diff audit for legacy Lens preservation and state/report identity.
- [x] Pass 3 - runtime proof audit for GPU-backed source behavior.
- [x] Pass 4 - clean re-read the repaired state after the schema-binding and UI-Salt metadata fixes; no additional real defect found.

## Audit Findings

- [done] Finding 1: the generated UI-Salt metadata for shipped SDF source rows had drifted from the hardcoded runtime descriptors by omitting `signal.sdf_sample_step`; rematerializing without fixing the authoring source exposed the mismatch. Repaired `docs/ui_salt/color_pipeline_function_library.ui.salt`, regenerated `docs/ui_salt/generated/color_pipeline_function_library.contract.v1.json`, and added the materializer rail to this contract.
- [done] Finding 2: the source catalog schema-binding and materializer tests still hardcoded the old 12-source count, which would let the new source miss one of the public catalog proof rails. Repaired those expectations and added the schema-binding and materializer rails to the contract.
