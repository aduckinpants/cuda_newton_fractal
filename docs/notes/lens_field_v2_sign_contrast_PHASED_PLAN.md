# Lens Field v2 Sign Contrast

## Current Phase

Complete - Lens Field v2 sign contrast is implemented, audited, checkpointed, and closed.

## Phase Checklist

- [x] Phase 0 - bootstrap from clean pushed `636f450` with rearward review `ok`.
- [x] Phase 1 - open this checked-in plan/contract and lock the active slice.
- [x] Phase 2 - add RED/native proof that Lens Field v2 can regain inside/outside contrast without splitting into a second visible source function.
- [x] Phase 3 - implement a source-local Lens Field v2 sign-contrast control through CPU/CUDA postprocess, state IO, metadata, and no-mouse automation.
- [x] Phase 4 - prove runtime behavior: blue/yellow distance split remains, sign contrast changes visible output, no physical mouse automation.
- [x] Phase 5 - hostile review, checkpoint, receipts, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [done] Keep Lens Field v2 as one visible Color Pipeline source function unless there is a strong technical reason to split it.
- [done] Restore the missing original Lens SDF inside/outside contrast feel without making the output simply black and white.
- [done] Document the SDF Normal Angle branch-cut observation as deferred phase-field work, not part of this slice.

## Scope

In scope:

- Add one source-local Lens Field v2 control for sign/inside-outside contrast.
- Keep `lens_field_v2_distance` as the single visible source identity for this v2 behavior.
- Preserve existing scale, bias, gate, sample-step, and blend controls.
- Update hardcoded metadata, UI-Salt materialized contract, import/export/state, CPU/CUDA SDF postprocess, and focused tests.
- Runtime proof through published viewer no-mouse controls.

Out of scope:

- New visible `lens_field_v2_inside_outside` source function.
- Normal-angle seam removal, phase-safe shape lanes, vector normal sources, or Color Pipeline redesign.
- Legacy Lens panel replacement.
- Per-source downsample, authored SDF UI, SDF-native lanes, or GPU field-generation changes.
- Physical mouse automation.

## Deferred Observation

SDF Normal Angle full-field branch cuts are currently treated as an expected phase/nearest-feature visualization artifact. The intended future seam is phase-aware/vector-aware source handling or a boundary-masked normal-angle mode. This slice does not change `sdf_normal_angle`.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on clean `codex/color-source-distinctness-smoke-matrix` at `636f450`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported a clean tree.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `636f450`.
- Branch: `codex/lens-field-v2-sign-contrast` created from `636f450`.
- Contract lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "Lens Field v2 sign contrast" --profile runtime --plan docs/notes/lens_field_v2_sign_contrast_PHASED_PLAN.md --contract docs/contracts/lens_field_v2_sign_contrast.contract.json` locked `global_active_contract`.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/lens_field_v2_sign_contrast.contract.json --out-json artifacts/validation/lens_field_v2_sign_contrast_contract_after_impl.json` passed.
- UI-Salt materializer: `py -3.14 -m pytest tests/test_ui_salt_materializer.py -q` passed.
- Focused native: `test_lens_sdf`, `test_color_pipeline_core`, `test_color_pipeline_window`, `test_color_pipeline_sdf_postprocess`, `test_color_pipeline_sdf_postprocess_cuda`, `test_diagnostics_state_io`, and `test_diagnostics_capture` passed via `ui_app/build_tests_vsdevcmd.cmd`; log: `artifacts/validation/lens_field_v2_sign_contrast_focused_native.log`.
- Runtime publish: `.\ui_app\build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Runtime no-mouse proof: `py -3.14 -m pytest tests/test_fractal_runtime_color_pipeline_sdf_rows.py::test_color_pipeline_sdf_source_rows_are_live_backed_no_mouse tests/test_fractal_runtime_color_pipeline_sdf_rows.py::test_lens_field_v2_distance_source_reports_gpu_backed_no_mouse -q` passed.
- Runtime UI-Salt contract: `py -3.14 -m pytest tests/test_fractal_runtime_ui_salt_contract.py -q` passed.
- Code quality: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/lens_field_v2_sign_contrast_code_quality.json` passed at 93/100.

## Hostile Audit

- Status: complete
- Did I keep one visible Lens Field v2 source function? Yes: `lens_field_v2_distance` remains the only visible v2 source id; no `lens_field_v2_inside_outside` was added.
- Did I restore measurable inside/outside contrast without losing the v2 signed-distance split? Yes: `signal.sign_contrast=0` preserves the old response, new rows default to `0.35`, and tests prove changing it alters the frame.
- Did I update both CPU and CUDA postprocess paths? Yes: CPU direct/planned paths and CUDA direct/field paths consume the same source param, with CPU/CUDA pixel parity.
- Did state save/load and UI automation preserve the new source-local control? Yes: diagnostics state/capture tests and the no-mouse runtime control id proof cover it.
- Did I leave `sdf_normal_angle` deferred instead of hiding the phase-field issue? Yes: no `sdf_normal_angle` semantics changed.
- Did I avoid physical mouse automation and unrelated Color Pipeline redesign? Yes: runtime proof uses in-process set-control automation and UI shape is otherwise unchanged.

## Audit Passes

- [x] Pass 1 - review source-local control metadata/import/export and CPU/CUDA parity.
- [x] Pass 2 - review runtime proof for sign contrast and distance split preservation.
- [x] Pass 3 - clean re-read of the repaired state for visible-source split, normal-angle drift, physical mouse usage, and stale plan text.

## Audit Findings

- [x] User-facing finding: Lens Field v2 preserved the blue/yellow signed-distance split but lost the stronger original Lens inside/outside contrast feel. Repaired with `signal.sign_contrast` on the existing `lens_field_v2_distance` row.
- [x] Hostile review finding: the new parameter was initially present in metadata but hidden by `IsLiveColorPipelineParamPath`; fixed and locked by `test_color_pipeline_window`.
- [x] Hostile review finding: `tests/test_fractal_runtime_ui_salt_contract.py` carried stale materialized-contract counts from before the Lens Field v2 source row existed; updated to current runtime metadata counts and rerun green.
