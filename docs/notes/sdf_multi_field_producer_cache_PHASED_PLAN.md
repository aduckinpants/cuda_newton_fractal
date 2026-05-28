# SDF Multi-Field Producer And Cache

## Current Phase

Closed - Step 3B multi-field producer/cache is implemented, validated, checkpointed, receipted, rearward-reviewed, and pushed.

## Phase Checklist

- [x] Phase 1 - create and lock the checked-in Step 3B plan and contract.
- [x] Phase 2 - add native REDs for row-local field policy, grouping, state IO, report fields, and one-field preservation.
- [x] Phase 3 - implement row-local SDF field policy and field-group planner.
- [x] Phase 4 - implement multi-field producer/cache wiring while preserving the inherited one-field fast path.
- [x] Phase 5 - add capture/report/runtime proof for distinct field groups.
- [x] Phase 6 - hostile audit, validation, checkpoint, receipts, rearward review, and push.

## Explicit User Asks

- [done] Keep moving forward after Step 3A.
- [done] Build on the measured SDF foundation without creating a new bad base.
- [done] Carry proof for performance/hot-path work and do not claim global FPS improvement without numbers.

## Scope

In scope:

- Step 3B only: multi-field producer/cache and report/state authority.
- Row-local field policy in saved/runtime source-row params:
  - default/inherit uses shared `LensSettings::downsample`;
  - explicit row field downsample values are `1`, `2`, `4`, `8`, or `16`.
- Field-group planner for enabled SDF-backed Source rows.
- Fast path preservation when all enabled SDF rows inherit the same shared field.
- Multi-field CPU postprocess fallback for distinct field groups, because the current CUDA postprocess backends accept one `SdfFieldView`.
- Runtime report fields for row-local field groups, field count, and per-group timing.
- Capture/state serialization of row-local field authority.
- Published no-mouse proof using state JSON/automation, not visible Step 3C UI controls.

Out of scope:

- Step 3C visible row-local controls.
- Color Pipeline layout redesign.
- Step 4 phase-safe normal-angle UX.
- Authored SDF pack viewport integration.
- SDF-native fractal lanes.
- Global FPS improvement claims.

## Proof Ledger

- `artifacts/validation/sdf_multi_field_producer_cache_contract.json` - contract schema valid after the postprocess and window preservation scope revisions.
- `artifacts/validation/sdf_multi_field_producer_cache_plan_sync.json` - phased-plan sync passed.
- `artifacts/validation/sdf_multi_field_producer_cache_native_groups.json` - `test_color_pipeline_sdf_field_groups` passed.
- `artifacts/validation/sdf_multi_field_producer_cache_native_state.json` - `test_diagnostics_state_io` passed.
- `artifacts/validation/sdf_multi_field_producer_cache_native_capture.json` - `test_diagnostics_capture` passed.
- `artifacts/validation/sdf_multi_field_producer_cache_native_report.json` - `test_viewer_ui_automation_report` passed.
- `artifacts/validation/sdf_multi_field_producer_cache_native_window.json` - `test_color_pipeline_window` passed and covers hidden row-field policy preservation through same-signal draft edits.
- `artifacts/validation/sdf_multi_field_producer_cache_native_postprocess.json` - CPU SDF postprocess rail passed, including distinct multi-field sampling.
- `artifacts/validation/sdf_multi_field_producer_cache_native_cuda_postprocess.json` - CUDA SDF postprocess preservation rail passed; unsupported multi-field stacks still fail closed.
- `artifacts/validation/sdf_multi_field_producer_cache_runtime_publish.json` - runtime published to `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- `artifacts/validation/sdf_multi_field_producer_cache_runtime_row_field.json` - published no-mouse proof for one-field inherit/same-field and two-field row-local field groups passed.
- `artifacts/validation/sdf_multi_field_producer_cache_runtime_capture_replay.json` - capture/replay authority matrix passed with distinct row-local field downsample preserved in state and effective source summary.
- `artifacts/validation/sdf_multi_field_producer_cache_runtime_sdf_rows.json` - existing Color Pipeline SDF rows proof passed.
- `artifacts/validation/sdf_multi_field_producer_cache_runtime_pacing.json` - SDF realtime pacing proof passed.
- `artifacts/validation/sdf_multi_field_producer_cache_runtime_performance_pytest.json` - published runtime performance pytest witness passed.
- `artifacts/sdf_multi_field_producer_cache/sdf_performance_witness.md` - generic SDF performance witness retained low-cost CUDA direct/field paths and reports mixed/inconclusive review, not a global FPS claim.
- `artifacts/sdf_multi_field_producer_cache/multi_field_performance_witness.md` - slice-specific witness: inherited/same-field rows remain one CUDA-fast field group; distinct field groups work but use CPU postprocess and are slower.

## Hostile Audit

- Status: complete

Required questions:

- Did this slice keep inherited/shared Lens downsample as the default and old-state behavior?
- Did it implement true field producer grouping instead of just reusing `signal.sdf_sample_step`?
- Did disabled rows and non-SDF rows avoid requesting fields?
- Did the one-field path remain the fast path for inherited/same-downsample stacks?
- Did runtime reports expose distinct field groups and timings without overclaiming performance?
- Did capture/replay preserve row-local field authority?
- Did this slice avoid Step 3C UI productization and broader SDF feature work?

## Audit Passes

- [done] Pass 1 - reviewed planner semantics for false positives and overclaim; verified runtime `KernelParams` stacks only contain enabled rows from the window bridge, and mixed SDF/non-SDF stacks fail closed.
- [done] Pass 2 - reviewed runtime/capture/report wiring for stale single-field assumptions; found and fixed hidden row-local field policy loss through same-signal draft/live edits.
- [done] Pass 3 - re-read the repaired state and found no additional real defect, with the distinct-field CPU fallback recorded as a measured limitation.

## Audit Findings

- [done] Finding: the first planner implementation tripped Windows `min`/`max` macro interference in the runtime build. Fixed by using `(std::min)` / `(std::max)` in the planner header and republishing the runtime.
- [done] Finding: capture effective-source summary was initially stale after adding row-local field downsample. Fixed diagnostics capture export and reran capture/replay against the republished runtime.
- [done] Finding: same-signal Color Pipeline draft edits could have stripped hidden `sdf_field_downsample` before Step 3C exposes row controls. Fixed by preserving the existing row-local field policy through draft/live apply and adding `test_color_pipeline_window` coverage.
- [done] Clean re-read: re-read the repaired state after the hidden-policy fix and found no additional real defect in the Step 3B scope.
- [done] Limitation: distinct row-local field groups currently use the CPU postprocess path because the CUDA postprocess backends accept one `SdfFieldView`. The measured witness shows this is correct but slower; inherited/same-field stacks keep the CUDA fast path. No global FPS improvement is claimed for distinct multi-field composition in this slice.
- [done] Deferred: visible per-row field downsample controls, GPU multi-field postprocess, per-function quality UX, phase-safe normal-angle UX, authored SDF pack integration, and SDF-native lanes remain outside Step 3B.
