# SDF Scalar Preview Quality Hardening

## Current Phase

Complete - scalar-only SDF Source stacks now stay postprocess step 1 during preview, neighborhood SDF stacks still step during preview, and the runtime settle proof waits for full-size frames.

## Phase Checklist

- [x] Phase 1 - open this checked-in hardening plan and contract from the pause-point review
- [x] Phase 2 - add RED/native proof that scalar-only SDF preview policy remains pixel step 1 while neighborhood SDF stacks may step
- [x] Phase 3 - make the SDF postprocess pixel-step policy source-aware without changing capture/full-quality step 1 behavior
- [x] Phase 4 - prove runtime no-mouse behavior: scalar-only SDF preview reports step 1, heavy SDF preview still reports stepped postprocess, settle returns to step 1
- [x] Phase 5 - hostile review, validation, and closure preparation

## Explicit User Asks

- [done] Review recent work for lowest-hanging cleanup/hardening at this pause point.
- [done] Fix and harden first before a deeper review or new SDF feature work.
- [done] Avoid broad feature work: this is cleanup around the current SDF postprocess behavior.
- [done] Preserve the no-physical-mouse runtime proof pattern.

## Scope

In scope:

- Keep full-quality render, settle, capture, and headless capture at SDF postprocess pixel step 1.
- Keep preview work reduction for SDF source stacks that need neighborhood sampling, such as `sdf_normal_angle` and `sdf_curvature`.
- Preserve scalar-only SDF Source stacks at postprocess pixel step 1 during preview because they already use direct center samples.
- Add focused native and runtime proof for that policy split.

Out of scope:

- Per-row or per-function SDF downsample as persisted authored state.
- Multiple SDF fields per Color Pipeline source stack.
- GPU Color Pipeline postprocess.
- Boundary-masked normal-angle.
- Color Pipeline preset/composition redesign.
- Authored SDF pack live viewport integration.
- SDF-native selectable fractal lanes.
- Physical mouse automation.

## Review Finding

The previous slice added scalar-only SDF direct sampling, then added a preview pixel-step policy that only checks render-pacing state. That means `sdf_signed_distance`, `sdf_inside_outside`, and `sdf_boundary_band` stacks can become block-stepped during interaction even though they do not need neighborhood sampling. This is low-risk to repair because the postprocess layer already exposes `ColorPipelineSdfPostprocessCanUseDirectSamples(...)`.

## Proof Ledger

- Start authority: branch `codex/color-pipeline-sdf-source-rows`, clean head `8613983`, rearward review `status=ok`.
- Read-only review confirmed `RenderHeadlessFractalFrame` calls SDF postprocess without preview options, so headless capture defaults to step 1.
- Read-only review confirmed live diagnostic capture sets `forceFullQualityRender` before dispatch, so the diagnostic capture path also uses step 1.
- RED: `sdf_scalar_preview_quality_red_native` failed because `ResolveSdfColorPipelinePostprocessOutputPixelStep(...)` did not exist.
- GREEN: `sdf_scalar_preview_quality_native` passed with `test_color_pipeline_sdf_postprocess: passed=50 failed=0`.
- Runtime GREEN: `sdf_scalar_preview_quality_runtime_publish` published `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe` and `sdf_scalar_preview_quality_runtime_pytest` passed `tests/test_fractal_runtime_sdf_realtime_pacing.py` with one persistent no-mouse viewer lane.
- Runtime hostile finding: the first runtime edit left the viewer in preview state before loading the heavy SDF state; the test now waits for scalar settle before switching states.
- Runtime hostile finding: the settle predicate could accept `preview_active == false` while still seeing a preview-sized frame; both scalar and heavy settle predicates now require full target dimensions.
- Closure validation GREEN: contract validation, phased-plan sync, code-quality baseline, diff hygiene, and the focused native/runtime rails passed on the repaired state.

## Hostile Audit

- Status: complete
- Required posture: assume the repair accidentally disables heavy SDF preview relief, changes capture quality, weakens runtime reporting, or silently widens into the deferred per-row downsample model.

## Audit Passes

- [done] Pass 1 - inspected source-aware policy implementation for direct-vs-neighborhood correctness.
- [done] Pass 2 - inspected full-quality/capture/headless paths for accidental stepped postprocess.
- [done] Pass 3 - inspected runtime proof for one persistent viewer lane and no physical mouse automation.
- [done] Pass 4 - inspected docs/plan wording for false claims about per-row downsample, GPU postprocess, or SDF-native lanes.
- [done] Pass 5 - clean re-read of the repaired state found no additional real defect after the scalar-policy and settle-predicate repairs.

## Audit Findings

- [done] Review finding repaired: scalar-only SDF stacks were eligible for preview block stepping despite being direct-sample capable.
- [done] Runtime proof gap repaired: the test now proves scalar-only preview reports step 1 and direct samples every preview pixel.
- [done] Harness weakness repaired: settle waits now require full target dimensions so a stale preview-sized frame cannot satisfy full-quality proof.
- [done] Clean re-read: confirmed the repaired state stays limited to SDF postprocess output stepping, full-quality/headless capture remains optionless step 1, heavy SDF normal/curvature preview relief remains intact, and no deferred per-row downsample/GPU/SDF-native work was added.

## Validation Targets

- `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_scalar_preview_quality_native --log artifacts/logs/sdf_scalar_preview_quality_native.log --out-json artifacts/validation/sdf_scalar_preview_quality_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_sdf_postprocess`
- `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_scalar_preview_quality_runtime_publish --log artifacts/logs/sdf_scalar_preview_quality_runtime_publish.log --out-json artifacts/validation/sdf_scalar_preview_quality_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_vsdevcmd.cmd`
- `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_scalar_preview_quality_runtime_pytest --log artifacts/logs/sdf_scalar_preview_quality_runtime_pytest.log --out-json artifacts/validation/sdf_scalar_preview_quality_runtime_pytest.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_sdf_realtime_pacing.py`
- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_scalar_preview_quality_hardening.contract.json --out-json artifacts/validation/sdf_scalar_preview_quality_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/sdf_scalar_preview_quality_hardening_PHASED_PLAN.md --out-json artifacts/validation/sdf_scalar_preview_quality_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/sdf_scalar_preview_quality_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_scalar_preview_quality_diff_check --log artifacts/logs/sdf_scalar_preview_quality_diff_check.log --out-json artifacts/validation/sdf_scalar_preview_quality_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`
