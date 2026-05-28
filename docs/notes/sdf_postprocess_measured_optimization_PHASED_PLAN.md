# SDF Postprocess Measured Optimization

## Current Phase

Phase 6 - measured SDF postprocess optimization validated for checkpoint closeout.

## Phase Checklist

- [x] Phase 1 - create and lock this measured hot-path plan/contract.
- [x] Phase 2 - run a branch-local published-runtime baseline witness before code edits.
- [x] Phase 3 - inspect the SDF postprocess call stack and add focused RED proof for the selected measured seam.
- [x] Phase 4 - implement only exact-pixel postprocess optimization backed by native parity.
- [x] Phase 5 - publish runtime, rerun the same witness, and write before/after performance numbers.
- [x] Phase 6 - hostile audit, validation, and checkpoint-ready plan sync.

## Explicit User Asks

- [done] Continue the SDF performance campaign from the merged field-generation optimization head.
- [done] For hot-path work, produce measured before/after performance numbers rather than qualitative claims.
- [done] Preserve current SDF pixels, Lens Field v2 behavior, capture/replay authority, no-mouse runtime harness behavior, and Color Pipeline UI behavior.
- [done] Keep per-row/per-function downsample, authored SDF UI, SDF-native lanes, phase-signal UX redesign, and broad renderer changes out of this slice.

## Scope

In scope:

- Branch-local baseline witness before product edits.
- SDF Color Pipeline postprocess call-stack review using the field-generation telemetry already landed.
- One narrow exact-pixel optimization in the SDF postprocess path if a measured seam is found.
- Native parity tests for the selected seam and runtime no-mouse proof through the published viewer.
- Before/after witness artifacts and a short comparison table for the representative matrix.

Out of scope:

- New visible UI controls or user-facing workflow changes.
- Per-source or per-row SDF downsample authority.
- Authored SDF pack UI, SDF-native fractal lanes, and broader Color Pipeline composition redesign.
- Pacing/debounce policy changes by feel.
- Physical mouse automation.
- Claims that FPS or responsiveness is solved without measured published-runtime evidence.

## Proof Ledger

- Start authority: `master` was merged and pushed at `22859e7` after `codex/sdf-field-generation-optimization`; rearward review for the current clean head is `ok`.
- Prior witness: `artifacts/sdf_field_generation_optimization/sdf_performance_witness.md` reports `postprocess_optimization_candidate`; representative rows show postprocess around `1.0-1.4 ms` after field cache/JFA reuse.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "SDF postprocess measured optimization" --profile runtime --plan docs/notes/sdf_postprocess_measured_optimization_PHASED_PLAN.md --contract docs/contracts/sdf_postprocess_measured_optimization.contract.json` appended `ck:a1842e85` and locked the active contract.
- Baseline first sample: `sdf_postprocess_measured_baseline_witness` passed, but `sdf_signed_distance_fullres` reported noisy `Post ms = 7.743`.
- Baseline rerun sample: `sdf_postprocess_measured_baseline_witness_rerun` passed and reported the same row at `Post ms = 1.336`, proving single-sample timing is too noisy for hot-path claims.
- RED proof: `py -3.14 -m pytest tests/test_sdf_performance_witness_tool.py::test_sdf_measurement_report_aggregates_repeated_rows_by_median -q` failed because the witness had no `raw_sample_count` or median aggregation.
- GREEN proof: `py -3.14 -m pytest tests/test_sdf_performance_witness_tool.py -q` passed after adding median aggregation and raw timing samples.
- Branch-local median baseline witness: `sdf_postprocess_measured_baseline_witness` passed with `--repeat-count 3`.
  - `sdf_signed_distance_fullres`: post `1.199 ms`, SDF total `1.228 ms`, last `1.775 ms`.
  - `sdf_normal_angle_fullres`: post `1.097 ms`, SDF total `1.132 ms`, last `1.744 ms`.
  - `sdf_curvature_fullres`: post `1.158 ms`, SDF total `1.187 ms`, last `1.813 ms`.
  - `sdf_normal_angle_curvature_stack`: post `1.289 ms`, SDF total `1.322 ms`, last `1.869 ms`.
  - `sdf_signed_distance_downsample4`: post `0.876 ms`, SDF total `0.947 ms`, last `1.579 ms`.
- RED/native product proof: `sdf_postprocess_measured_reuse_red` failed because `SdfColorPipelinePostprocessStats` had no `backend_buffer_reused` / `backend_buffer_grew` fields.
- Implementation proof: CUDA SDF postprocess now reuses a bounded shared scratch buffer instead of per-call allocating/freeing device field/RGBA buffers; the shared buffer is host-mutex guarded and exposes reuse/grow stats through native stats and no-mouse runtime reports.
- Native CPU proof: `sdf_postprocess_measured_native_cpu` passed (`test_color_pipeline_sdf_postprocess`, `passed=105 failed=0`).
- Native CUDA proof: `sdf_postprocess_measured_native_cuda` passed after the mutex hardening (`test_color_pipeline_sdf_postprocess_cuda`, `passed=104 failed=0`).
- Native report proof: `sdf_postprocess_measured_native_report` passed (`test_viewer_ui_automation_report`).
- Runtime publish proof: `sdf_postprocess_measured_runtime_publish` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Runtime no-mouse proof: `sdf_postprocess_measured_runtime_pacing` passed (`1 passed`), `sdf_postprocess_measured_runtime_rows` passed (`9 passed`), and `sdf_postprocess_measured_capture_replay` passed (`1 passed`).
- Final after-witness: `sdf_postprocess_measured_after_witness` passed with `--repeat-count 3`; global recommendation remained conservative (`mixed_or_inconclusive_measurement_review_required`) because the total witness includes low-cost and one-sample preview rows, but repeated full-quality SDF postprocess medians improved materially.
- Before/after median comparison from identical witness rows:
  - `sdf_signed_distance_fullres`: post `1.199 -> 0.875 ms` (`-27.0%`), SDF total `1.228 -> 0.929 ms` (`-24.3%`), last `1.775 -> 1.683 ms` (`-5.2%`).
  - `sdf_normal_angle_fullres`: post `1.097 -> 0.578 ms` (`-47.3%`), SDF total `1.132 -> 0.633 ms` (`-44.1%`), last `1.744 -> 1.313 ms` (`-24.7%`).
  - `sdf_curvature_fullres`: post `1.158 -> 0.485 ms` (`-58.1%`), SDF total `1.187 -> 0.514 ms` (`-56.7%`), last `1.813 -> 1.051 ms` (`-42.0%`).
  - `sdf_normal_angle_curvature_stack`: post `1.289 -> 0.486 ms` (`-62.3%`), SDF total `1.322 -> 0.523 ms` (`-60.5%`), last `1.869 -> 1.076 ms` (`-42.4%`).
  - `sdf_signed_distance_downsample4`: post `0.876 -> 0.356 ms` (`-59.4%`), SDF total `0.947 -> 0.413 ms` (`-56.4%`), last `1.579 -> 1.179 ms` (`-25.4%`).
  - `sdf_normal_angle_curvature_stack_interaction_preview`: post `1.023 -> 0.461 ms` (`-55.0%`), SDF total `1.940 -> 1.273 ms` (`-34.3%`), last `2.475 -> 1.801 ms` (`-27.2%`).
- Contract validation: `sdf_postprocess_measured_contract` passed.
- Code-quality proof: `sdf_postprocess_measured_code_quality` passed baseline check, score `93/100`, no critical/error findings.
- Diff hygiene: `sdf_postprocess_measured_diff_check` passed.
- Plan sync: `viewer_host_assert_phased_plan_sync.py` passed for this plan.
- Hostile-audit validation: `sdf_postprocess_measured_hostile_audit` passed with three completed audit passes and four real findings recorded.

## Hostile Audit

- Status: done
- Did I record branch-local baseline numbers before editing hot-path code? Yes: the repeated median baseline witness was recorded before product code edits.
- Did I optimize only the measured SDF postprocess seam instead of drifting back into field generation, pacing policy, or UI redesign? Yes: the only product optimization is CUDA postprocess scratch-buffer reuse plus telemetry.
- Did I preserve exact pixels for full-quality, capture, replay, and no-mouse runtime rows? Yes: native CUDA parity, SDF rows runtime, pacing runtime, and capture/replay authority rails are green.
- Did I include before/after numbers from the same witness shape before claiming any improvement? Yes: the proof ledger lists identical-row median deltas.
- Did I keep deferred SDF work deferred? Yes: no per-row downsample, authored SDF UI, SDF-native lane, phase UX redesign, or broad renderer work landed.

## Audit Passes

- [done] Pass 1 - median baseline and call-stack review found single-sample witness noise and selected the per-call CUDA postprocess allocation seam.
- [done] Pass 2 - implementation diff audit found the shared CUDA scratch buffer needed explicit host-side serialization; repaired with a mutex and reran CUDA native proof.
- [done] Pass 3 - re-read the repaired state, runtime proof, capture/replay proof, and before/after witness; no additional real defect found.

## Audit Findings

- [done] Finding 1: single-sample witness timing was noisy enough to make hot-path conclusions unsafe (`sdf_signed_distance_fullres` postprocess `7.743 ms` vs `1.336 ms` on immediate rerun). Repaired by adding repeat aggregation with median timing and raw sample retention to the witness.
- [done] Finding 2: CUDA SDF postprocess allocated and freed device buffers on every call. Repaired by reusing a bounded shared scratch buffer, adding native reuse/grow stats, and reporting those stats through no-mouse runtime JSON.
- [done] Finding 3: the first implementation made the shared CUDA buffer implicit single-thread state. Repaired by guarding the shared buffer with a host mutex before final runtime proof.
- [done] Finding 4: the SDF realtime pacing proof used requested target dimensions and accepted a transient adaptive-quality report as settled authority. Repaired by comparing against observed full-quality frame dimensions and requiring requested-quality SDF settle reports.
- [done] Finding 5: clean re-read after repairs confirmed the slice did not alter visible UI, capture/replay semantics, per-row downsample authority, phase-signal UX, authored SDF UI, SDF-native lanes, or broad renderer behavior.

## Action Hostile Review

- Action ID: sdf-postprocess-measured-buffer-reuse-final
- Suspected failure mode: postprocess timing claims could be polluted by noisy single samples, per-call CUDA allocation overhead, or a runtime proof that accepted the wrong settled frame authority.
- Correct owner/action: use repeated-row median witnesses, reuse the CUDA postprocess scratch buffers safely, expose buffer reuse/grow telemetry, and harden no-mouse pacing proof around observed viewer frame authority and requested-quality settle reports.
- Proof surface: branch-local baseline and after witness JSON/Markdown, focused native SDF postprocess tests, runtime publish, no-mouse SDF runtime proof, capture/replay preservation if the pixel path changes, contract validation, phased-plan sync, hostile-audit validation, code-quality baseline, diff hygiene, rearward review, and clean tree.
- Blocked action: per-row downsample, authored SDF UI, SDF-native lanes, phase-signal UX redesign, debounce-policy guessing, broad renderer changes, physical mouse automation, or performance claims without before/after numbers.

## Notes

- The performance ledger for this branch should compare identical witness rows and report at least median field, postprocess, total SDF, and last frame timings.
- If the after witness is noisy or does not improve materially, close truthfully as pixel-safe work with unproven client improvement; do not relabel it as an FPS fix.
