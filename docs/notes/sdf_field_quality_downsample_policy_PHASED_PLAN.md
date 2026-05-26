# SDF Field Quality And Downsample Policy

## Current Phase

Complete - SDF preview postprocess quality policy is implemented, reported, validated, audited, and prepared for wrapper-managed closure.

## Phase Checklist

- [x] Phase 1 - create this checked-in phased plan and contract, then lock the active slice
- [x] Phase 2 - add RED/native proof that field downsample does not reduce CPU postprocess samples and that preview postprocess quality needs explicit reporting
- [x] Phase 3 - implement a bounded preview-only SDF postprocess pixel-step policy with reportable sample counts
- [x] Phase 4 - prove full-quality capture/render defaults remain step 1 and that runtime preview reduces SDF postprocess work without physical mouse automation
- [x] Phase 5 - hostile review, validation, and wrapper-managed closure preparation

## Explicit User Asks

- [done] Tackle SDF FPS/responsiveness as foundational before returning to broader SDF feature work.
- [done] Consider whether field downsample can become per-function/per-row, but do not hide a guessed authority model inside the implementation.
- [done] Preserve capture quality and existing SDF Source row behavior while improving interactive exploration.
- [done] Keep the work measurable: report what the SDF field and postprocess are actually doing.

## Scope

In scope:

- Keep `LensSettings::downsample` as the single shared SDF field-resolution authority for this slice.
- Add a preview-only Color Pipeline SDF postprocess pixel-step policy so interactive preview can reduce CPU postprocess work in addition to lowering render resolution.
- Add native stats/proof for step 1 versus stepped postprocess sampling.
- Add runtime no-mouse proof that preview reports a stepped SDF postprocess path and full-quality settle returns to step 1.
- Add report fields that make SDF postprocess quality/sample count visible to tests and future tuning.

Out of scope:

- Per-row/per-function SDF downsample as persisted authored state.
- Generating multiple SDF fields per Color Pipeline stack.
- GPU Color Pipeline postprocess rewrite.
- Boundary-masked normal-angle.
- Color Pipeline preset manager or composition UI redesign.
- Authored SDF pack viewport integration.
- SDF-native selectable fractal lanes.
- Physical mouse automation.

## Authority Decision

Current code proves a shared-field model: `LensSettings::downsample` controls the single Lens/SDF field, and all SDF Source rows sample that field. Per-row downsample is plausible later, but it needs a deliberate model: either multiple fields, a high-resolution field with row-local coarse sampling, or a source-stack-level quality policy. This slice does not choose that durable authored model. It adds the narrower missing runtime policy: when the viewer is already in interactive preview, the CPU SDF postprocess may sample and fill blocks at a reportable pixel step; full-quality render/capture remains step 1.

## Proof Ledger

- Start authority: branch `codex/color-pipeline-sdf-source-rows`, clean head `310c52a`, rearward review `status=ok`.
- RED: `sdf_field_quality_policy_red_native` failed because `SdfColorPipelinePostprocessOptions`, `output_pixel_step`, and `filled_pixel_count` did not exist.
- First GREEN: `sdf_field_quality_policy_native` passed with `test_color_pipeline_sdf_postprocess: passed=44 failed=0`.
- Runtime GREEN: `sdf_field_quality_policy_runtime_publish` published the runtime and `sdf_field_quality_policy_runtime_pytest` passed `tests/test_fractal_runtime_sdf_realtime_pacing.py` with one persistent no-mouse viewer lane.
- Post-green hostile finding: the first block-fill test only checked for a nonzero hash; it now compares against the sentinel pre-hash.

## Hostile Audit

- Status: complete
- Required posture: assume this accidentally degrades capture quality, hides per-row downsample as a fake implementation, makes SDF output blocky at full quality, or claims FPS wins without measurable sample-count/report evidence.

## Audit Passes

- [done] Pass 1 - inspect the landed diff for capture/full-quality paths accidentally using stepped postprocess.
- [done] Pass 2 - inspect runtime reporting and tests for proof that sample counts actually drop during preview.
- [done] Pass 3 - inspect docs/plan wording for any false claim that per-row or GPU postprocess work shipped.
- [done] Pass 4 - clean re-read the repaired state after hardening the native block-fill hash assertion.
- [done] Pass 5 - confirmed the repaired state after roadmap/status sync: per-row/multi-field downsample and GPU postprocess remain deferred.

## Audit Findings

- [done] Real test weakness repaired: `TestPreviewPixelStepReducesSamplesAndFillsFrame` originally checked only that the postprocess hash was nonzero; it now proves the frame changed from the sentinel hash.
- [done] Real contract scope gap repaired: the initial contract did not allow status-doc truth sync after shipping the policy; the contract was revised before editing `spec_intake/_STATUS.md`, `DEFERRED_THREADS.md`, and `KNOWN_ISSUES.md`.
- [done] Clean re-read: `RenderHeadlessFractalFrame` still calls SDF postprocess without options, so headless capture/finding defaults to step 1.
- [done] Clean re-read: live stepped postprocess is gated by `renderPacing.preview_active` and `forceFullQuality == false`; settle/full-quality renders report step 1.
- [done] Clean re-read: this slice does not persist per-row/per-function SDF downsample, generate multiple SDF fields, or rewrite postprocess on GPU.

## Notes

Validation targets:

- `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_quality_policy_native --log artifacts/logs/sdf_field_quality_policy_native.log --out-json artifacts/validation/sdf_field_quality_policy_native.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_sdf_postprocess`
- `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_quality_policy_runtime_publish --log artifacts/logs/sdf_field_quality_policy_runtime_publish.log --out-json artifacts/validation/sdf_field_quality_policy_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_vsdevcmd.cmd`
- `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_quality_policy_runtime_pytest --log artifacts/logs/sdf_field_quality_policy_runtime_pytest.log --out-json artifacts/validation/sdf_field_quality_policy_runtime_pytest.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_sdf_realtime_pacing.py`
- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_field_quality_downsample_policy.contract.json --out-json artifacts/validation/sdf_field_quality_policy_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/sdf_field_quality_downsample_policy_PHASED_PLAN.md --out-json artifacts/validation/sdf_field_quality_policy_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/sdf_field_quality_policy_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_quality_policy_diff_check --log artifacts/logs/sdf_field_quality_policy_diff_check.log --out-json artifacts/validation/sdf_field_quality_policy_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`
