# SDF Per-Row Postprocess Quality

## Current Phase

Phase 5 - implementation, hostile audit, focused native rails, runtime publish, no-mouse proof, and performance witness are complete; closure validators, receipts, rearward review, push, and clean-tree proof remain.

## Phase Checklist

- [x] Phase 0 - bootstrap clean `codex/sdf-adaptive-preview-pacing` head, confirm rearward review is ok, repair stale recent SDF plan truth, and run the current SDF performance witness.
- [x] Phase 1 - create this checked-in plan/contract and lock the active slice.
- [x] Phase 2 - add RED/native and runtime-facing proof for missing per-row SDF quality/sample-step authority.
- [x] Phase 3 - add row-local SDF postprocess quality authority while preserving shared Lens SDF field downsample and default pixels.
- [x] Phase 4 - add state/window/runtime proof that the control is visible, saved, loaded, and no-mouse editable on SDF Source rows.
- [ ] Phase 5 - publish runtime, run focused native/runtime/performance rails, hostile audit, receipts, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [done] Continue the next slice after the capture/pacing repair.
- [done] Use measured evidence instead of guessing at FPS fixes.
- [done] Improve the current SDF downsample limitation where `SDF Field Downsample` is shared and not per Source function/layer.
- [done] Preserve current SDF/Color Pipeline behavior by default.
- [done] Do not implement GPU postprocess, authored SDF UI, or SDF-native lanes in this slice.
- [done] Do not use physical mouse automation.

## Scope

In scope:

- A per-SDF-Source-row postprocess quality/sample-step parameter, default `1`.
- Runtime state, Color Pipeline row descriptors, window import/apply, diagnostics save/load, and SDF postprocess execution support for that parameter.
- Focused native tests and no-mouse runtime proof through the existing SDF Source row path.
- Performance witness after implementation to show the remaining bottleneck truthfully.

Out of scope:

- Multiple SDF fields per stack.
- GPU Color Pipeline postprocess.
- Capture quality changes, sample-tier changes, or high-resolution finding changes.
- Broader Color Pipeline UI redesign, preset manager work, authored SDF pack UI, SDF-native lanes, Salticid runtime dependency, or physical mouse automation.

## Measurement Inputs

- Current-head witness `sdf_next_decision_current_witness` recommends `postprocess_optimization_candidate`.
- Large settled scenario: base render `4.366 ms`, SDF field `8.153 ms`, SDF postprocess `98.680 ms`, total render `111.198 ms`.
- Shared downsample `4x` scenario: SDF field `1.017 ms`, SDF postprocess `1.300 ms`, total render `3.206 ms` at the smaller witness size.
- The evidence says postprocess consumption remains the pressure point, while shared field downsample is powerful but too coarse as one global Source-stack authority.

## Implementation Direction

Use a row-local sample-step model rather than multiple hidden SDF fields. `LensSettings::downsample` remains the single field-resolution authority. The new row parameter controls how coarsely a specific SDF Source row is sampled/reused during CPU postprocess. Default `1` must preserve exact current full-quality pixels. Non-default values are an explicit quality/performance tradeoff for that row.

The first implementation target is conservative:

- expose `signal.sdf_sample_step` on SDF Source rows only;
- default and old saved states load as `1`;
- validate and persist bounded integer values;
- make SDF postprocess stats/reporting prove sample reduction for a non-default row;
- keep Capture Finding and full-quality defaults at step `1`;
- do not add GPU or multi-field behavior.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on clean `codex/sdf-adaptive-preview-pacing` at `ad52f81`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported clean before the prior truth-sync repair and after its closeout.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `ad52f81` and for the truth-sync closeout head.
- Truth-sync prerequisite: `ck:b9921bd5` repaired stale recent SDF closeout plan wording, wrote receipts, rearward-reviewed ok, pushed, and left a clean tree.
- Current witness: `sdf_next_decision_current_witness` passed and wrote `artifacts/sdf_next_decision/current_sdf_performance_witness.json`; recommendation is `postprocess_optimization_candidate`.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "SDF per-row postprocess quality" --profile runtime --plan docs/notes/sdf_per_row_postprocess_quality_PHASED_PLAN.md --contract docs/contracts/sdf_per_row_postprocess_quality.contract.json` locked checkpoint token `ck:f509f380`.
- Contract revision: `py -3.14 tools/viewer_host_revise_contract.py --session-id global_active_contract --contract docs/contracts/sdf_per_row_postprocess_quality.contract.json` accepted the added diagnostics capture and runtime automation report scope.
- RED native postprocess: `sdf_per_row_quality_red_native` failed before implementation because `sdf_sample_step` and source-sample postprocess stats did not exist.
- RED window: `sdf_per_row_quality_red_window` failed before implementation because SDF row descriptors/import/apply did not expose `signal.sdf_sample_step`.
- RED state IO: `sdf_per_row_quality_red_state_io` failed before implementation because SDF row sample step was not saved/loaded.
- RED automation report: `sdf_per_row_quality_red_report` failed before implementation because the viewer automation probe did not expose source sample counts.
- Implementation: added `signal.sdf_sample_step` to every SDF Source row, with default `1`, range `1..8`, save/load/capture/report support, and cached CPU SDF postprocess sampling that only engages for rows with step greater than `1`.
- Preservation: `LensSettings::downsample` remains the shared SDF field-resolution authority; the new value only controls row-local postprocess source sampling. Default `1` preserves existing full-quality pixels.
- Native postprocess proof: `sdf_per_row_quality_native_postprocess` passed after the audit fix; it covers source sample reduction, changed pixels for non-default row step, output-fill preservation, and serial/parallel-safe stats.
- Native window proof: `sdf_per_row_quality_native_window` passed; it covers SDF row rendering/import/apply for `signal.sdf_sample_step`.
- Native state proof: `sdf_per_row_quality_native_state_io` passed; it covers round-trip and invalid `sdf_sample_step` rejection.
- Native automation report proof: `sdf_per_row_quality_native_report` passed; it covers the new source sample count report fields.
- Runtime publish: `sdf_per_row_quality_runtime_publish` passed and refreshed `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Runtime no-mouse proof: `sdf_per_row_quality_runtime_sdf_rows` passed with 7 tests; it proved the visible SDF Source row control ids, no-mouse edit of `color_pipeline.source.sdf_boundary_band.signal.sdf_sample_step.primary`, source sample count reduction, changed frame hash, and unchanged shared field downsample authority.
- Performance witness: `sdf_per_row_quality_witness` passed after the final publish; `artifacts/sdf_per_row_quality/sdf_performance_witness.md` still recommends `postprocess_optimization_candidate`, with the large settled full-quality SDF stack at `101.153 ms` postprocess and `109.772 ms` SDF total.

## Hostile Audit

- Status: complete
- Required posture: assume row-quality controls change default pixels, silently create a second hidden SDF field authority, fail to reduce samples, break capture/replay, break disabled-row/source-stack compatibility, or only pass helper tests without published viewer proof.

## Audit Passes

- [done] Pass 1 - verified RED tests failed for missing row quality authority before implementation.
- [done] Pass 2 - inspected the implementation for default-pixel preservation and no hidden multi-field authority; found and repaired a cached-row invalid-index fail-closed defect.
- [done] Pass 3 - verified state/window/runtime proof exercises the visible row control, state round-trip, diagnostics capture, automation report, and no-mouse edit path.
- [done] Pass 4 - reran focused native/runtime rails after hostile findings; clean re-read found no additional defect in the active slice.

## Audit Findings

- [done] Real finding: the first cached source-row resolver could index `plan.rows[rowIndex]` after an invalid row index instead of failing closed. Repaired by returning the fail-closed value before indexing.
- [done] Real finding: state IO initially lacked an explicit invalid-value test for `sdf_sample_step`. Added rejection coverage for `sdf_sample_step: 0` and verified the error is surfaced.
- [done] Real workflow finding: an early active-runtime witness used `fractal_ui_dev.exe` while the published runtime was locked. The final publish restored `fractal_ui.exe`, and the exact witness was rerun against the published runtime.
- [done] Clean third-pass review: default row step remains `1`, shared `LensSettings::downsample` remains the only SDF field-resolution authority, disabled rows continue to be filtered by the existing source-stack logic, and no physical mouse automation was added.
