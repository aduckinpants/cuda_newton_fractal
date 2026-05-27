# SDF Adaptive Preview Pacing

## Current Phase

Complete - SDF adaptive preview pacing is validated, audited, checkpointed, and closed.

## Phase Checklist

- [x] Phase 0 - merge the closed SDF postprocess hotpath branch into `master`, push it, and branch `codex/sdf-adaptive-preview-pacing` from the updated head.
- [x] Phase 1 - open this runtime/viewer-first plan and contract, lock the active slice, and add RED proof for persistent interaction telemetry plus adaptive preview floor behavior.
- [x] Phase 2 - add the smallest telemetry/reporting seam needed to measure command-to-preview and command-to-settle timing in one persistent no-mouse viewer process.
- [x] Phase 3 - make the preview floor adaptive from measured visible-frame cost while preserving the user `preview_min_scale` as the normal floor.
- [x] Phase 4 - publish runtime and prove fast/no-loss interaction stays full quality, severe SDF interaction drops low enough to help, and settle returns to full quality.
- [x] Phase 5 - hostile audit, validation receipts, contract proof receipt, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [done] Continue after the tested SDF postprocess hotpath improvement.
- [done] Treat adaptive preview minimum scale as the likely next issue, but do not tune it blind.
- [done] Measure interaction responsiveness in a persistent no-mouse viewer process before claiming the pacing problem is fixed.
- [done] Preserve full-quality settle, capture quality, SDF source semantics, Color Pipeline behavior, and the shared Lens downsample authority.
- [done] Keep GPU Color Pipeline postprocess and per-source/per-row SDF downsample deferred for separate measured slices.

## Scope

In scope:
- `ViewerRenderPacing` policy and focused native pacing tests.
- Automation/report fields or runtime harness tooling needed for persistent no-mouse interaction timing.
- Runtime proof for fast interaction, severe SDF interaction, adaptive preview scaling, and full-quality settle.
- Plan, contract, handoff, receipts, and validation artifacts.

Out of scope:
- GPU Color Pipeline postprocess.
- Per-source or per-row SDF downsample authority.
- Visible Color Pipeline UI changes.
- SDF source semantics, gates, phase behavior, and capture/replay changes.
- Authored SDF pack UI, SDF-native fractal lanes, preset manager UI, or Salticid runtime dependency changes.
- Physical mouse automation.

## Implementation Direction

The first change should be a RED-first measurement seam: prove the current reports or harness cannot tell whether a control mutation produced a timely preview and settle inside one persistent viewer process. The proof should record command issue time, first matching preview report time, first matching full-quality settle report time, preview scale, target/live dimensions, SDF timing, and report publication latency.

The pacing policy should then make the effective preview floor adaptive from measured visible-frame cost. The user-configured `preview_min_scale` remains the normal floor for ordinary slow frames. Severe measured frames may temporarily use the existing emergency floor when the budget-derived scale is below the normal floor. Recovery must stay gradual and hysteretic, and capture/full-quality settle renders must stay step 1/full target resolution.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on `codex/sdf-postprocess-hotpath-specialization` at `f9fd7e5`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported a clean tree.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `f9fd7e5`.
- Merge prep: `master` was fast-forwarded to `f9fd7e5`, pushed to `origin/master`, and this branch was created from that pushed head.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "SDF adaptive preview pacing" --profile runtime --plan docs/notes/sdf_adaptive_preview_pacing_PHASED_PLAN.md --contract docs/contracts/sdf_adaptive_preview_pacing.contract.json` appended `ck:220a04a2` and locked the active contract.
- Native pacing proof: `sdf_adaptive_pacing_native` passed `test_viewer_render_pacing`; it now proves measured over-budget frames can temporarily drop below the configured `0.5` preview floor when the budget-derived scale requires it.
- Native report proof: `sdf_adaptive_pacing_report_native` passed `test_viewer_ui_automation_report`.
- Runtime publish proof: `sdf_adaptive_pacing_runtime_publish` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published SDF runtime proof: `sdf_adaptive_pacing_runtime_sdf` passed; the persistent no-mouse viewer records command-to-preview timing, report publish/observe latency, SDF timing, preview dimensions, and full-quality settle timing.
- Published pacing preservation proof: `sdf_adaptive_pacing_runtime_resolution` passed with 4 tests when rerun sequentially, preserving fast/no-loss full-quality behavior and severe slow-frame preview behavior.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_adaptive_preview_pacing.contract.json --out-json artifacts/validation/sdf_adaptive_pacing_contract.json` returned `ok=true`.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Code-quality baseline: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/sdf_adaptive_pacing_code_quality.json` passed after shrinking the touched pacing function back within baseline.

## Hostile Audit

- Status: complete
- Required posture: assume adaptive preview floor logic over-triggers on fast f32 paths, under-triggers on severe SDF paths, hides latency behind lower-quality preview reports, breaks full-quality settle, or records helper-only timing that does not reflect the published viewer.
- Did I avoid blind tuning? Yes; the native proof uses measured frame time and budget-derived scale, and runtime proof records command/report/settle timing around a real no-mouse command.
- Did I preserve fast/no-loss full-quality interaction behavior? Yes; the existing resolution pacing runtime rail still passes.
- Did I preserve severe SDF preview and full-quality settle? Yes; the SDF runtime rail proves preview dimensions drop, SDF postprocess pixel step is used during interaction, and settle returns to target dimensions with postprocess step 1.
- Did I leave GPU postprocess and per-source downsample deferred? Yes; only pacing policy and runtime harness timing were changed.

## Audit Passes

- [done] Pass 1 - inspected the current pacing/reporting seam and identified the real policy gap: the old effective minimum stayed at the configured floor for badly over-budget frames until a fixed severe multiplier tripped.
- [done] Pass 2 - re-read the implementation diff for over-aggressive preview activation, under-aggressive severe-frame behavior, stale timing, and full-quality settle regressions.
- [done] Pass 3 - re-read the runtime proof to ensure one persistent no-mouse viewer process proves command-to-preview and command-to-settle behavior.
- [done] Pass 4 - clean re-read after repairing the overly strict runtime timing assertion found no visible UI, SDF source, capture/replay, GPU postprocess, or per-source downsample drift.
- [done] Pass 5 - revalidated runtime rails sequentially after the shared diagnostics race in the parallel attempt.

## Audit Findings

- [done] Finding 1: the current adaptive floor was too coarse. A frame could be far enough over budget to need a below-0.5 preview scale, but still be clamped at the configured floor until the fixed severe multiplier tripped. Repaired with budget-derived effective floor logic and native pacing tests.
- [done] Finding 2: the first runtime telemetry assertion demanded a 25% command-to-preview roundtrip reduction against an already-optimized SDF baseline. That was a misleading proof target after the hotpath slice. Repaired to assert measured command-to-report timing, bounded report publish/observe latency, and preview arrival without waiting for another full baseline frame.
- [done] Finding 3: running published runtime lanes in parallel raced on the shared diagnostics/runtime surfaces and produced a missing `diagnostics/last/state.json` in the resolution rail. Repaired procedurally by rerunning the runtime preservation rail sequentially; it passed with 4 tests.
- [done] Clean re-read confirmed the repaired state after Finding 1, Finding 2, and Finding 3: focused native proof, published SDF runtime proof, and published resolution pacing preservation proof all pass.

## Notes

- Acceptance should distinguish real responsiveness proof from raw render-time proof: a lower `last_render_ms` alone is not enough if command-to-preview or command-to-settle timing is still unmeasured.
- If the witness is noisy, close only with the strongest truthful status and do not claim the whole FPS issue is fixed.
