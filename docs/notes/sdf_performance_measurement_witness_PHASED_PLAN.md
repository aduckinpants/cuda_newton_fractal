# SDF Performance Measurement Witness

## Current Phase

Complete - runtime measurement witness is implemented, validated, checkpoint-ready, and prepared for the next decision slice.

## Phase Checklist

- [x] Phase 0 - start from clean pushed preflight head `625eb51` with rearward review `ok`.
- [x] Phase 1 - create implementation branch `codex/sdf-performance-measurement-witness` and open this runtime/viewer-first plan and contract.
- [x] Phase 2 - add RED tests for the SDF performance witness report shape, classification, and no-mouse runtime entrypoint.
- [x] Phase 3 - implement the measurement witness tool without changing SDF rendering policy.
- [x] Phase 4 - publish runtime and run the witness through the published no-mouse viewer path.
- [x] Phase 5 - hostile audit, validation receipts, rearward review, push, and clean-tree stop point.

## Explicit User Asks

- [done] Begin implementation after the SDF performance/design preflight.
- [done] Start with the measurement witness before choosing per-row downsample or GPU Color Pipeline postprocess.
- [done] Preserve current SDF Source rows, capture/replay authority, preview/full-quality distinction, and Color Pipeline behavior.
- [done] Do not make the UI worse or add physical mouse automation.
- [done] Keep full Salticid backend/legacy-layer removal deferred.

## Scope

In scope:
- A runtime/no-mouse witness that runs representative SDF Color Pipeline scenes against the published viewer.
- A structured JSON report that records per-scenario base render, SDF field, SDF postprocess, SDF total, final render time, sample counts, preview/full-quality state, and rendered frame hash.
- Report classification that separates likely field-generation pressure, CPU postprocess pressure, preview-quality effects, and inconclusive/low-signal rows.
- Focused unit tests for report aggregation plus one published-runtime pytest that writes the witness report.

Out of scope:
- Per-row or per-function SDF downsample authority.
- GPU Color Pipeline postprocess.
- Visible UI changes.
- Color Pipeline semantics changes.
- Authored SDF pack UI, SDF-native lanes, or broader composition redesign.
- Salticid runtime dependency or legacy-layer removal.
- Physical mouse automation.

## Implementation Direction

The first product-value artifact is a small report, not a speed claim.

The witness should include:
- scalar SDF row: `sdf_signed_distance`;
- derivative/phase row: `sdf_normal_angle`;
- derivative scalar row: `sdf_curvature`;
- mixed SDF-only stack: `sdf_normal_angle` plus `sdf_curvature`;
- shared downsample variation: at least one row at `SDF Field Downsample = 4x`;
- preview interaction sample: one no-mouse control mutation that records preview timing and then full-quality settle.

The report should not use wall-clock process time as the primary measurement. It should consume the viewer's published automation report fields:
- `base_render_ms`;
- `lens_sdf_field_ms`;
- `lens_sdf_postprocess_ms`;
- `lens_sdf_total_ms`;
- `last_render_ms`;
- `lens_sdf_postprocess_direct_sample_count`;
- `lens_sdf_postprocess_neighborhood_sample_count`;
- `lens_sdf_postprocess_filled_pixel_count`;
- `lens_sdf_postprocess_pixel_step`;
- `render_pacing_preview_active`;
- rendered dimensions and frame hash.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on `codex/sdf-performance-design-prep` at `625eb51`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported clean before implementation branch creation.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `625eb51`.
- Branch prep: `git switch -c codex/sdf-performance-measurement-witness` created the implementation branch from the preflight head.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_performance_measurement_witness.contract.json --out-json artifacts/validation/sdf_performance_measurement_witness_contract.json` passed before implementation and after the final plan update.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed before implementation and after the final plan update.
- RED tests: `py -3.14 -m pytest tests/test_sdf_performance_witness_tool.py -q` failed before `tools/viewer_host_sdf_performance_witness.py` existed, proving the missing witness-report contract.
- Unit proof: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_performance_measurement_witness_unit --log artifacts/logs/sdf_performance_measurement_witness_unit.log --out-json artifacts/validation/sdf_performance_measurement_witness_unit.json --heartbeat-seconds 30 --timeout-seconds 300 -- py -3.14 -m pytest tests/test_sdf_performance_witness_tool.py -q` passed with 3 tests.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_performance_measurement_witness_publish --log artifacts/logs/sdf_performance_measurement_witness_publish.log --out-json artifacts/validation/sdf_performance_measurement_witness_publish.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_vsdevcmd.cmd` passed.
- Published runtime witness: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_performance_measurement_witness_runtime --log artifacts/logs/sdf_performance_measurement_witness_runtime.log --out-json artifacts/validation/sdf_performance_measurement_witness_runtime.json --heartbeat-seconds 30 --timeout-seconds 900 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_sdf_performance_witness.py` passed with 1 runtime test.
- Durable report: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_performance_measurement_witness_report --log artifacts/logs/sdf_performance_measurement_witness_report.log --out-json artifacts/validation/sdf_performance_measurement_witness_report.json --heartbeat-seconds 30 --timeout-seconds 300 -- py -3.14 tools/viewer_host_sdf_performance_witness.py --runtime-exe D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe --out-json artifacts/sdf_performance_measurement_witness/sdf_performance_witness.json --out-md artifacts/sdf_performance_measurement_witness/sdf_performance_witness.md --work-dir artifacts/sdf_performance_measurement_witness/work --width 640 --height 480 --include-preview-sample` passed and recommended `postprocess_optimization_candidate`.
- Code-quality baseline: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/sdf_performance_measurement_witness_code_quality.json` passed with score 94/100 and baseline check passed.
- Diff hygiene: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_performance_measurement_witness_diff_check --log artifacts/logs/sdf_performance_measurement_witness_diff_check.log --out-json artifacts/validation/sdf_performance_measurement_witness_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.
- Hostile-audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/sdf_performance_measurement_witness_PHASED_PLAN.md --out-json artifacts/validation/sdf_performance_measurement_witness_hostile_audit.json` passed.

## Hostile Audit

- Status: complete
- Did I accidentally choose an optimization before the measurement report exists? No; this slice adds witness/report only.
- Did I add a helper-only witness without published viewer proof? No; runtime proof runs through `viewer_host_runtime_pytest_lane.py` against the published executable.
- Did I use OS mouse or repeated open/close loops? No OS mouse; the witness owns the runtime automation lock and uses one persistent viewer launch.
- Did I conflate field generation with Color Pipeline postprocess? No; the report stores `lens_sdf_field_ms`, `lens_sdf_postprocess_ms`, `lens_sdf_total_ms`, and classification separately.
- Did I preserve capture/replay and full-quality output authority? Yes for this slice; no renderer or capture code changed, and the settled witness now requires target dimensions plus `lens_sdf_postprocess_pixel_step == 1`.
- Did I avoid UI and SDF semantic changes in this slice? Yes; only a tool, tests, plan, contract, and handoff are touched.
- Did the report expose enough evidence to choose the next implementation seam? Yes; the durable witness points to `postprocess_optimization_candidate`.

## Audit Passes

- [done] Pass 1 - scope audit after plan/contract creation kept optimization policy changes out of scope.
- [done] Pass 2 - report contract audit after RED tests found the missing tool/report contract and then covered classification/recommendation behavior.
- [done] Pass 3 - runtime proof audit after implementation found standalone runtime-lock ownership was missing.
- [done] Pass 4 - artifact report audit found relative work-dir paths broke direct tool runs from the viewer runtime cwd.
- [done] Pass 5 - settled-frame audit found the witness was accepting `preview_active=false` before full target dimensions and SDF postprocess step 1 returned.
- [done] Pass 6 - clean re-read after final validation confirmed the repaired state and did not expose another workflow mistake.

## Audit Findings

- [done] Finding 1: the witness tool initially depended on the caller to own the runtime automation lock; fixed by making the standalone tool acquire the lock itself and removing the redundant external lock from the runtime test.
- [done] Finding 2: direct report generation with a relative `--work-dir` launched the viewer from the runtime directory and failed to load generated state; fixed by normalizing CLI paths relative to repo root and adding a unit guard.
- [done] Finding 3: the settled sample originally accepted `render_pacing_preview_active=false` while still rendering a low-resolution preview frame with postprocess step 4; fixed by requiring target dimensions and `lens_sdf_postprocess_pixel_step == 1`.
- [done] Clean re-audit after validation: contract validation, plan sync, code-quality baseline, diff hygiene, unit proof, published runtime proof, and durable witness report all passed after the repaired state.

## Notes

- The likely next branch after this one is a decision/implementation slice based on the witness output.
- This branch must stop with a report and recommendation, not a hidden performance policy change.
