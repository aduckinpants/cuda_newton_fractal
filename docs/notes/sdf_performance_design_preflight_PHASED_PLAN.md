# SDF Performance Design Preflight

## Current Phase

Complete - planning branch is prepared, validated, checkpointed, and ready for discussion before implementation.

## Phase Checklist

- [x] Phase 0 - start from clean pushed `master` at `66dc145` with rearward review `ok`.
- [x] Phase 1 - create fresh planning branch `codex/sdf-performance-design-prep`.
- [x] Phase 2 - add a docs-only planning contract and high-level SDF performance/design problem breakdown.
- [x] Phase 3 - validate docs-only contract, plan sync, hostile audit, code-quality baseline, diff hygiene, checkpoint, receipts, rearward review, push, and clean-tree stop point.

## Explicit User Asks

- [done] Prep the next branch for the SDF performance/design work.
- [done] Keep this careful and discussion-oriented; do not start implementation yet.
- [done] End with a first high-level breakdown of the problems this branch is setting out to address.
- [done] Keep full Salticid backend/legacy-layer removal as a long-term deferred goal.
- [done] Finish the current task foundation cleanly before product mutation.

## Scope

In scope:
- A fresh planning branch from clean `master`.
- This docs-only preflight plan and contract.
- A high-level problem breakdown for SDF performance/design refinement.
- A proposed next implementation-slice order that starts with measurement.

Out of scope:
- Product code, tests, schema, renderer, runtime harness, Color Pipeline behavior, SDF postprocess behavior, or UI layout changes.
- Selecting GPU Color Pipeline postprocess or per-row/multi-field SDF downsample before a measured witness.
- Authored SDF pack UI, SDF-native fractal lanes, boundary-masked normal-angle implementation, or broad composition UI redesign.
- Salticid runtime dependency, Salticid backend cutover, or legacy-layer removal.
- Physical mouse automation.

## Current Repo Truth Inputs

- `master` is clean and pushed at `66dc145` after the post-merge roadmap truth sync.
- Rearward review for `66dc145` is `ok`.
- `spec_intake/_STATUS.md` marks SDF field pack planning as having shipped Lens SDF control truth, scalar field authority, GPU Lens SDF backend, live SDF Source rows, capture/replay authority, phase-signal metadata, fractal-switch preservation, realtime telemetry, signal specialization, preview quality policy, and full-quality downsampled-field cell reuse.
- `DEFERRED_THREADS.md` identifies the next larger SDF performance/design choice as per-row/multi-field SDF downsample authority or GPU Color Pipeline postprocess with measured client evidence.
- `KNOWN_ISSUES.md` keeps viewer responsiveness and Color Pipeline composition UX open, but the disabled-row and normal-angle plus curvature regressions are now preservation expectations, not open repair bugs.
- `docs/notes/sdf_field_pack_near_term_TODO.md` records that current SDF Source rows share `LensSettings::downsample`; per-row or per-function SDF downsample needs an explicit authority model.

## High-Level Problem Breakdown

This branch is setting up one careful decision path, not jumping straight to a code answer.

1. Measurement authority
   - Current reports distinguish base render, SDF field generation, SDF postprocess, and total SDF timing.
   - The next implementation slice should prove which component dominates in real client use before changing policy.
   - A good first witness should include low-cost scalar SDF rows, derivative rows such as `sdf_normal_angle` and `sdf_curvature`, and multi-row blends.

2. CPU SDF postprocess cost
   - The shipped optimizations reduced unnecessary derivative sampling and reuse downsampled field-cell work.
   - Remaining cost may still be CPU-side palette/shape/grading and row-stack evaluation over many pixels or field cells.
   - If this dominates, GPU Color Pipeline postprocess may be the right larger move, but only after a bounded CPU/GPU cost witness.

3. Field generation versus field consumption
   - GPU Lens SDF field generation exists, but Color Pipeline consumption can still dominate after the field is produced.
   - The next slice must keep these separate so we do not tune Lens generation when the bottleneck is postprocess consumption.

4. Shared downsample authority limitation
   - Today all SDF Source rows share `LensSettings::downsample` / `SDF Field Downsample`.
   - That is simple and truthful, but it prevents layered compositions where coarse low-frequency rows are cheap and high-detail rows stay crisp.
   - Per-row downsample is useful, but it risks multiple hidden fields, mismatched replay state, and confusing quality semantics if added naively.

5. Per-row or multi-field design choices
   - Option A: one high-resolution field with row-local coarse sampling.
   - Option B: multiple SDF fields at different downsample levels.
   - Option C: source-stack-level quality policy rather than per-row knobs.
   - Option D: no new field authority; optimize postprocess first.
   - The correct choice depends on measured cost, replay complexity, and visual quality, not preference.

6. Preview versus capture truth
   - Interactive preview can use degraded SDF postprocess quality, but full-quality render and Capture Finding must remain authoritative.
   - Any new downsample or GPU path must preserve `state.json` replay, capture parity, and no-mouse runtime proof.

7. UX quality and predictability
   - SDF controls are now useful, but performance can make them feel unreliable.
   - The UI should not expose a quality knob that changes pixels in surprising ways without clear reporting.
   - Per-row controls should only appear after the authority model is stable enough to explain and test.

8. GPU Color Pipeline postprocess risk
   - GPU postprocess could be the biggest performance win.
   - It is also the higher-risk path because it touches palette/shape/grading parity, SDF stack semantics, capture parity, and runtime reporting.
   - A safe GPU slice should start with a narrow parity target, not the whole Color Pipeline.

9. Legacy-layer removal is not this problem
   - UI-Salt metadata has moved important lookup seams, but runtime state and execution still rely on C++ `KernelParams` and enum-backed math.
   - Full removal should wait until the vertical is stable and all runtime/state/capture paths have parity coverage.

## Proposed Next Implementation Order

1. Measurement witness slice
   - Add or extend no-mouse runtime telemetry around representative SDF Color Pipeline scenes.
   - Output a small comparison report for scalar-only rows, derivative rows, multi-row blends, preview, and full-quality/capture paths.
   - No policy changes unless the measurement itself exposes a tiny bug.

2. Decision slice
   - Use the measurement report to choose CPU specialization, GPU postprocess, or per-row/multi-field downsample.
   - Write a bounded implementation contract with explicit preservation rails.

3. First performance implementation slice
   - Implement the chosen seam with the smallest runtime-visible surface.
   - Preserve capture/replay, full-quality output, current SDF Source stack behavior, and no-mouse automation.

4. UX exposure slice only if needed
   - If the selected design requires user-visible controls, add them after the authority and proof exist.
   - Avoid exposing per-row downsample until it has clear state, replay, and report semantics.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on clean `master` at `66dc145`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported clean before branch creation.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `66dc145`.
- Branch prep: `git switch -c codex/sdf-performance-design-prep` created the planning branch from clean `master`.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_performance_design_preflight.contract.json --out-json artifacts/validation/sdf_performance_design_preflight_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile-audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/sdf_performance_design_preflight_PHASED_PLAN.md --out-json artifacts/validation/sdf_performance_design_preflight_hostile_audit.json` passed.
- Code-quality baseline: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/sdf_performance_design_preflight_code_quality.json` passed with score 94/100 and baseline check passed.
- Diff hygiene: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_performance_design_preflight_diff_check --log artifacts/logs/sdf_performance_design_preflight_diff_check.log --out-json artifacts/validation/sdf_performance_design_preflight_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.

## Hostile Audit

- Status: complete
- Did I accidentally start implementation? No product code/test/schema/runtime files are in scope.
- Did I pick GPU postprocess or per-row downsample without data? No; the proposed order starts with measurement.
- Did I ignore the current working vertical? No; the plan preserves SDF Source rows, capture/replay, preview/full-quality distinction, and UI-Salt metadata work.
- Did I conflate Salticid backend removal with this slice? No; legacy removal remains deferred.
- Did I leave the next branch with enough context for discussion? Yes; the problem breakdown names the competing bottlenecks, design choices, and preservation risks.

## Audit Passes

- [done] Pass 1 - scope audit limited this branch to planning and discussion prep.
- [done] Pass 2 - roadmap audit kept authored SDF UI, SDF-native lanes, boundary-masked normal-angle, broad composition UI, and Salticid backend removal deferred.
- [done] Pass 3 - problem-map audit found the key ambiguity: per-row downsample and GPU postprocess solve different bottlenecks, so measurement must precede implementation.
- [done] Pass 4 - validator-backed clean re-read after plan creation found no implementation drift and no premature design choice.

## Audit Findings

- [done] Finding 1: jumping directly to per-row downsample would create a second field-resolution authority without proving whether field generation or postprocess consumption is the actual bottleneck.
- [done] Finding 2: jumping directly to GPU postprocess risks a broad parity surface across shape, palette, grading, SDF stack semantics, capture replay, and reporting.
- [done] Finding 3: the safest next implementation slice is a measurement witness with representative SDF scenes before choosing a larger optimization path.
- [done] Clean re-audit after validation: contract validation, plan sync, hostile-audit validation, code-quality baseline, and diff hygiene passed after the final plan update.

## Notes

- This prep branch intentionally stops before product mutation.
- The next implementation branch should start from this discussion and use a new runtime/viewer-first contract.
