# SDF Field Cache Color-Only Reuse

## Current Phase

Phase 5 - checkpoint-ready closure validation.

## Phase Checklist

- [x] Phase 0 - start from clean `a5c9de0` with rearward review `ok` and branch `codex/sdf-field-cache-color-only-reuse`.
- [x] Phase 1 - open this checked-in plan/contract and lock the active slice.
- [x] Phase 2 - add RED native/runtime/report proof for SDF field cache status and color-only cache reuse.
- [x] Phase 3 - implement safe Lens SDF field reuse when the rendered mask, render size, and effective downsample are unchanged.
- [x] Phase 4 - publish runtime and prove no-mouse color-only edits hit the field cache while camera/fractal-quality changes miss.
- [x] Phase 5 - update FPS measurement/witness surfaces, hostile audit, plan sync, and checkpoint-ready validation.

## Explicit User Asks

- [active] Work the next SDF slice while the repo is in a good state.
- [active] Harden the feature, testing, and FPS measurement surfaces.
- [active] Continue from the recent Lens Field v2 / SDF realtime performance work without reopening broad Color Pipeline redesign.

## Scope

In scope:

- Add a small, reportable Lens SDF field cache for live viewer frames.
- Reuse the field only when the current rendered mask bytes, render size, and effective field downsample match the cached authority.
- Preserve full-quality capture/replay semantics and existing adaptive requested/effective downsample behavior.
- Add automation report fields for cache status/hit/miss so runtime tests and performance witnesses can see what happened.
- Prove no-mouse Color Pipeline source presentation edits can change pixels while reusing the same SDF field.

Out of scope:

- Per-row/per-function SDF downsample authority.
- Multi-field composition, authored SDF pack UI, SDF-native lanes, or broad Color Pipeline UI redesign.
- Replacing CUDA JFA or changing the field-generation algorithm.
- Reusing a field across camera/fractal/render-size/mask changes.
- Physical mouse automation.

## Implementation Direction

The safe cache authority is the rendered mask itself. The first implementation should not try to hand-maintain a brittle list of every mask-affecting fractal parameter. Instead, after the normal render produces the current mask, compare it against the cached mask for the same render dimensions and effective SDF downsample. If it matches exactly, reuse the cached `SdfFieldResult`; otherwise compute the field with the existing backend seam and replace the cache.

This keeps the first win narrow:

- Color-only Color Pipeline source/shape/palette/grading edits can avoid field generation.
- Camera moves, fractal selector changes, iteration/formula parameter changes, render-size changes, and effective downsample changes miss naturally because the mask or key changes.
- Capture and replay still render full-quality requested output; the cache may skip redundant field work only when the new full-quality mask is identical.

Reports should distinguish actual cache behavior from backend behavior:

- `lens_sdf_field_cache_status`: `disabled`, `miss`, or `hit`
- `lens_sdf_field_cache_hit`: boolean
- `lens_sdf_field_cache_mask_bytes`: current mask byte count used for exact identity comparison

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on clean `codex/lens-field-v2-source-authority` at `a5c9de0`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported a clean tree.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `a5c9de0`.
- Branch: `codex/sdf-field-cache-color-only-reuse` was created from `a5c9de0`.
- Contract bootstrap: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_field_cache_color_only_reuse.contract.json --out-json artifacts/validation/sdf_field_cache_contract_bootstrap.json` passed.
- Plan sync bootstrap: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "SDF field cache color-only reuse" --profile runtime --plan docs/notes/sdf_field_cache_color_only_reuse_PHASED_PLAN.md --contract docs/contracts/sdf_field_cache_color_only_reuse.contract.json` appended `ck:07df35c8` and locked the active contract.
- RED native: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_cache_red_native --log artifacts/logs/sdf_field_cache_red_native.log --out-json artifacts/validation/sdf_field_cache_red_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_lens_sdf test_viewer_ui_automation_report` failed as expected before cache/report implementation.
- Native cache/report rail: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_cache_native --log artifacts/logs/sdf_field_cache_native.log --out-json artifacts/validation/sdf_field_cache_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_lens_sdf test_viewer_ui_automation_report` passed.
- Witness unit rail: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_cache_witness_unit --log artifacts/logs/sdf_field_cache_witness_unit.log --out-json artifacts/validation/sdf_field_cache_witness_unit.json --heartbeat-seconds 30 --timeout-seconds 300 -- py -3.14 -m pytest tests/test_sdf_performance_witness_tool.py` passed.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_cache_runtime_publish --log artifacts/logs/sdf_field_cache_runtime_publish.log --out-json artifacts/validation/sdf_field_cache_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Runtime SDF rows: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_cache_runtime_rows --log artifacts/logs/sdf_field_cache_runtime_rows.log --out-json artifacts/validation/sdf_field_cache_runtime_rows.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py` passed.
- Runtime pacing: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_cache_runtime_pacing --log artifacts/logs/sdf_field_cache_runtime_pacing.log --out-json artifacts/validation/sdf_field_cache_runtime_pacing.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_sdf_realtime_pacing.py` passed.
- Runtime capture/replay: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_cache_runtime_capture_replay --log artifacts/logs/sdf_field_cache_runtime_capture_replay.log --out-json artifacts/validation/sdf_field_cache_runtime_capture_replay.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_capture_replay_authority.py` passed.
- Performance witness: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_cache_performance_witness --log artifacts/logs/sdf_field_cache_performance_witness.log --out-json artifacts/validation/sdf_field_cache_performance_witness.json --heartbeat-seconds 30 --timeout-seconds 900 -- py -3.14 tools/viewer_host_sdf_performance_witness.py --runtime-exe D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe --out-json artifacts/sdf_field_cache_color_only_reuse/sdf_performance_witness.json --out-md artifacts/sdf_field_cache_color_only_reuse/sdf_performance_witness.md --work-dir artifacts/sdf_field_cache_color_only_reuse/work --width 640 --height 480 --include-preview-sample` passed.
- Measured same-run cache evidence: full-resolution `sdf_signed_distance` first miss used `field_ms=2.182`; subsequent color-source-only full-resolution cache hits used `field_ms=0.051`, `0.031`, and `0.033` with `field_cache_hit_count=3`.
- Hardening rerun: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_cache_native_rerun --log artifacts/logs/sdf_field_cache_native_rerun.log --out-json artifacts/validation/sdf_field_cache_native_rerun.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_lens_sdf test_viewer_ui_automation_report` passed after adding render-dimension miss proof.
- Hardening rerun: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_cache_runtime_rows_rerun --log artifacts/logs/sdf_field_cache_runtime_rows_rerun.log --out-json artifacts/validation/sdf_field_cache_runtime_rows_rerun.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py` passed after adding camera-change miss proof.
- Post-hardening contract-named native rail: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_cache_native --log artifacts/logs/sdf_field_cache_native.log --out-json artifacts/validation/sdf_field_cache_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_lens_sdf test_viewer_ui_automation_report` passed.
- Post-hardening contract-named runtime rows rail: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_cache_runtime_rows --log artifacts/logs/sdf_field_cache_runtime_rows.log --out-json artifacts/validation/sdf_field_cache_runtime_rows.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py` passed.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_field_cache_color_only_reuse.contract.json --out-json artifacts/validation/sdf_field_cache_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/sdf_field_cache_color_only_reuse_PHASED_PLAN.md --out-json artifacts/validation/sdf_field_cache_hostile_audit.json` passed.
- Code-quality baseline: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/sdf_field_cache_code_quality.json` passed with score `93/100` and no new baseline failure.
- Diff check: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_cache_diff_check --log artifacts/logs/sdf_field_cache_diff_check.log --out-json artifacts/validation/sdf_field_cache_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.

## Hostile Audit

- Status: complete
- Did I actually avoid recomputing the SDF field on a color-only edit, or did I only add report fields?
- Did I prove cache misses for mask/render-size/effective-downsample changes instead of reusing stale fields?
- Did I preserve Lens Field v2, existing SDF source rows, capture/replay, and adaptive SDF field resolution?
- Did I keep cache authority exact and simple rather than a brittle hand-maintained parameter list?
- Did I avoid per-row downsample, multi-field composition, authored SDF UI, SDF-native lanes, and broad Color Pipeline redesign?
- Did I publish the runtime and prove the real no-mouse viewer path?

## Audit Passes

- [x] Pass 1 - RED cache/report proof exposed missing cache/report types and missing automation report fields; native RED failed before implementation.
- [x] Pass 2 - implementation diff audit found a real proof gap: cache miss was proven for mask/downsample changes, but not render-dimension or real runtime camera/mask changes.
- [x] Pass 3 - re-read the repaired state after adding render-dimension and runtime camera-change miss coverage; no additional real defect found in cache authority, capture/replay, or adaptive downsample behavior.

## Audit Findings

- [x] Real finding: the first implementation could have closed with only color-only cache-hit proof plus lower-level mask/downsample miss proof; it did not directly prove render-dimension misses or real viewer camera-change misses.
- [x] Repair: added native render-dimension miss coverage in `test_lens_sdf` and no-mouse runtime camera-change miss coverage in `test_fractal_runtime_color_pipeline_sdf_rows`.
- [x] Clean re-read the repaired state: exact cache authority is still rendered mask bytes plus source dimensions plus effective downsample; capture/replay and pacing rails stayed green.
