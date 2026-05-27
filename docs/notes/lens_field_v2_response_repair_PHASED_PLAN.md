# Lens Field V2 Response Repair

## Current Phase

Phase 5 - complete for this checkpoint; closure artifacts are tracked by validation, receipt, and rearward-review outputs.

## Phase Checklist

- [x] Phase 0 - bootstrap from clean `dabff6c` with rearward review `ok`.
- [x] Phase 1 - open this checked-in plan/contract and lock the active slice.
- [x] Phase 2 - add RED/native proof that `lens_field_v2_distance` is not a raw alias of `sdf_signed_distance`.
- [x] Phase 3 - implement the legacy Lens response scalar for Lens Field v2 while preserving normal SDF signed-distance behavior.
- [x] Phase 4 - publish runtime and prove no-mouse Lens Field v2 differs from normal SDF signed distance while staying GPU-backed.
- [x] Phase 5 - hostile audit, plan sync, receipts, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [closed] Lens Field v2 should not look exactly like normal SDF signed distance.
- [closed] Modernize the original Lens SDF idea rather than only exposing the existing raw signed-distance field.
- [closed] Preserve the legacy Lens panel/debug path and the recent SDF performance work.

## Scope

In scope:

- Keep the `lens_field_v2_distance` source identity for saved-state and automation compatibility.
- Change that source's scalar semantics from raw signed-distance pixels to the legacy Lens response shape: clamp signed distance into a normalized `0..1` response using the existing `48px` legacy field radius convention.
- Keep `sdf_signed_distance` as raw signed-distance pixels with its existing default scale/bias.
- Keep GPU direct-scalar and field-signal paths consistent with CPU.
- Update UI-Salt and hardcoded descriptors so Lens Field v2 defaults match normalized response semantics.
- Add pixel-scale proof so downsampled SDF fields keep the same full-resolution legacy response convention.

Out of scope:

- Replacing the legacy Lens panel/debug/overlay path.
- Adding a grayscale palette or a broad Color Pipeline UI redesign.
- Per-source downsample, multi-field composition, authored SDF UI, or SDF-native lanes.
- Changing CUDA JFA field generation.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on clean `codex/sdf-field-cache-color-only-reuse` at `dabff6c`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported a clean tree.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `dabff6c`.
- Branch: `codex/lens-field-v2-response-repair` created from `dabff6c`.
- Slice open: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "Lens Field v2 response repair" --profile runtime --plan docs/notes/lens_field_v2_response_repair_PHASED_PLAN.md --contract docs/contracts/lens_field_v2_response_repair.contract.json` locked checkpoint token `ck:9a1a68c0`.
- Contract bootstrap: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/lens_field_v2_response_repair.contract.json --out-json artifacts/validation/lens_field_v2_response_contract_bootstrap.json` passed.
- RED proof: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_field_v2_response_red_native --log artifacts/logs/lens_field_v2_response_red_native.log --out-json artifacts/validation/lens_field_v2_response_red_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_sdf_postprocess` failed before implementation because Lens Field v2 and raw SDF signed distance produced identical frame hashes.
- Native proof: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_field_v2_response_native --log artifacts/logs/lens_field_v2_response_native.log --out-json artifacts/validation/lens_field_v2_response_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core test_color_pipeline_sdf_postprocess test_color_pipeline_sdf_postprocess_cuda test_lens_sdf` passed after implementation and after the pixel-scale hardening tests.
- UI-Salt proof: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_field_v2_response_ui_salt --log artifacts/logs/lens_field_v2_response_ui_salt.log --out-json artifacts/validation/lens_field_v2_response_ui_salt.json --heartbeat-seconds 30 --timeout-seconds 300 -- py -3.14 -m pytest tests/test_ui_salt_materializer.py` passed.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_field_v2_response_runtime_publish --log artifacts/logs/lens_field_v2_response_runtime_publish.log --out-json artifacts/validation/lens_field_v2_response_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_vsdevcmd.cmd` passed and published `runtime/fractal_ui.exe`.
- Published runtime proof: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_field_v2_response_runtime_rows --log artifacts/logs/lens_field_v2_response_runtime_rows.log --out-json artifacts/validation/lens_field_v2_response_runtime_rows.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py` passed; Lens Field v2 differs from raw `sdf_signed_distance` at the same scale/bias and remains GPU-backed.
- SDF pacing preservation: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_field_v2_response_runtime_pacing --log artifacts/logs/lens_field_v2_response_runtime_pacing.log --out-json artifacts/validation/lens_field_v2_response_runtime_pacing.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_sdf_realtime_pacing.py` passed.
- Capture/replay preservation: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_field_v2_response_runtime_capture_replay --log artifacts/logs/lens_field_v2_response_runtime_capture_replay.log --out-json artifacts/validation/lens_field_v2_response_runtime_capture_replay.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_capture_replay_authority.py` passed.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/lens_field_v2_response_repair.contract.json --out-json artifacts/validation/lens_field_v2_response_contract.json` passed.
- Code-quality baseline: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/lens_field_v2_response_code_quality.json` passed.
- Diff check: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_field_v2_response_diff_check --log artifacts/logs/lens_field_v2_response_diff_check.log --out-json artifacts/validation/lens_field_v2_response_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.

## Hostile Audit

- Status: complete
- Did I actually make Lens Field v2 semantically distinct from normal SDF signed distance? Yes: RED/native and published runtime proofs compare identical scale/bias settings and require different frame hashes.
- Did I keep normal `sdf_signed_distance` raw-distance behavior unchanged? Yes: raw SDF still maps to `SdfFieldSignalKind::signed_distance_px`; added pixel-scale proof confirms raw signed distance ignores `field.pixel_scale`.
- Did CPU and CUDA postprocess paths agree? Yes: `test_color_pipeline_sdf_postprocess_cuda` covers Lens Field v2 exact CPU/GPU pixels, including a scaled field.
- Did the published runtime prove the real no-mouse viewer path and not only helper functions? Yes: `tests/test_fractal_runtime_color_pipeline_sdf_rows.py` ran through the published executable via the no-mouse runtime lane.
- Did I preserve legacy Lens panel/debug behavior and recent SDF cache/adaptive performance behavior? Yes: the legacy Lens RGBA builder was not replaced, SDF pacing proof passed, and capture/replay proof passed.
- Did I avoid broad Color Pipeline redesign and deferred SDF composition work? Yes: the change is limited to Lens Field v2 source semantics, descriptors, and tests.

## Audit Passes

- [x] Pass 1 - RED/native alias proof found the real defect: Lens Field v2 had distinct catalog identity but runtime resolved to the same raw signed-distance signal as normal SDF signed distance.
- [x] Pass 2 - implementation diff audit repaired a proof gap by adding pixel-scale coverage; Lens Field v2 now proves the legacy response convention at non-1x field scale while raw SDF remains unchanged.
- [x] Pass 3 - clean re-read after repair found no additional real defect in CPU/CUDA source resolution, UI-Salt descriptor defaults, published runtime proof, SDF pacing, or capture/replay preservation.

## Audit Findings

- [x] Real finding: `lens_field_v2_distance` was only a distinct function ID/catalog row; CPU/CUDA postprocess still treated it as raw `sdf_signed_distance`, so it could visually collapse to normal SDF distance.
- [x] Real finding: the first implementation proof did not cover non-1x `field.pixel_scale`; this mattered because the legacy Lens response uses `48px / downsample` in the Lens field. Added CPU and CUDA pixel-scale tests.
- [x] Clean re-read: normal SDF signed distance stayed raw, Lens Field v2 uses the normalized legacy response scalar, CPU and CUDA agree, and the published no-mouse runtime path proves the user-visible behavior.
