# SDF Field Generation Optimization Preflight

## Current Phase

Complete - field-generation algorithm/caching measurement is selected as the next SDF performance path, with validation, hostile audit, receipts, rearward review, push, and clean-tree proof remaining as closeout rails.

## Phase Checklist

- [x] Phase 0 - verify clean `codex/sdf-field-generation-optimization-preflight` from pushed `master` merge head `55d36a4`.
- [x] Phase 1 - create this checked-in plan/contract and lock the active slice.
- [x] Phase 2 - record the current SDF field-generation call-stack seams and existing telemetry gaps.
- [x] Phase 3 - update roadmap/status docs so field-generation optimization is the selected next SDF performance path.
- [x] Phase 4 - hostile audit, plan sync, contract validation, code-quality baseline, diff hygiene, checkpoint, receipts, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [done] Follow the previously outlined sequence: merge the Lens Field v2 sign-contrast work, perform a short planning/check slice, then move to the next SDF performance implementation.
- [done] Select field-generation/optimization as the next path instead of per-row downsample or higher-level SDF composition because it is foundational.
- [done] Avoid building more features on a weak SDF performance base.
- [done] Keep the current SDF source behavior, capture/replay authority, and Lens Field v2 work intact.

## Scope

In scope:

- A docs-only preflight branch that selects field-generation algorithm/caching measurement as the next implementation target.
- A call-stack map covering mask render/copy, mask downsample, field generation backend dispatch, field cache reuse, field transfer/reporting, and downstream postprocess handoff.
- Roadmap/status truth updates that move "field-generation optimization" from an open choice to the selected next campaign.
- A concrete next implementation contract outline that starts with measurement guardrails before algorithm or cache changes.

Out of scope:

- Product code, tests, schema, renderer, runtime harness, or Color Pipeline behavior changes in this preflight slice.
- Per-row/per-function SDF downsample authority.
- Multi-field SDF composition, authored SDF pack UI, SDF-native fractal lanes, or boundary-masked normal-angle UX.
- GPU postprocess changes; direct-scalar and field-signal CUDA postprocess paths already shipped and are not the current target.
- Physical mouse automation.

## Current Repo Truth Inputs

- `master` is pushed at merge commit `55d36a4` after merging `codex/lens-field-v2-sign-contrast`.
- Rearward review for `55d36a4` is `ok`.
- The current SDF roadmap says CUDA SDF postprocess is shipped and field generation remains the next measured performance seam.
- `ui_app/src/lens_sdf.cpp` already contains reusable field result/view authority, effective downsample policy, and a same-frame field cache keyed by mask bytes and effective downsample.
- `ui_app/src/lens_sdf_cuda.cu` currently downsampled the mask on the host, then dispatches CUDA JFA or CPU chamfer fallback.
- `ui_app/src/main.cpp` calls `ResolveEffectiveLensSdfDownsample(...)`, tries same-frame field cache reuse, computes field generation, then applies SDF overlay and Color Pipeline postprocess.
- The current runtime witness reports `lens_sdf_field_ms`, requested/effective downsample, cache status, postprocess timing, postprocess backend, and total SDF cost, but it does not yet split field-generation cost into mask downsample, backend generation, transfer/fallback, and cache-key decision components.

## Selected Next Implementation Direction

The next product slice should not guess at another debounce or UI policy. It should make field-generation cost observable enough to choose and prove the first real optimization.

Implementation order for the next branch:

1. Add measurement-only telemetry for field-generation stages.
   - Report mask downsample time.
   - Report backend generation time.
   - Report cache lookup/store status and whether a same-frame or cross-frame reuse path was available.
   - Report requested/effective field dimensions and mask byte count.
   - Preserve existing capture/replay and full-quality semantics.

2. Add a small no-mouse SDF field-generation witness matrix.
   - Cover downsample `1x`, `2x`, `4x`, `8x`, and `16x`.
   - Cover scalar signed distance, normal angle, curvature, normal-angle plus curvature, and Lens Field v2 distance.
   - Include preview and settled modes so adaptive quality and capture-quality paths remain separate.

3. Choose the first optimization only after the measurement rail is green.
   - Candidate A: avoid host-side mask downsample overhead by moving/keeping downsample on GPU.
   - Candidate B: improve field cache reuse when only Color Pipeline source/shape/palette/grading changes and the mask authority key is unchanged.
   - Candidate C: tune CUDA JFA allocation/reuse or pass scheduling if backend generation dominates.
   - Candidate D: leave field generation alone if measurement proves another seam is still dominant.

4. Defer visible controls until the authority model is proven.
   - Per-row/per-function downsample remains useful, but it would introduce new field-resolution semantics before the base field producer is measured and hardened.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on clean `codex/sdf-field-generation-optimization-preflight` at `55d36a4`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported a clean tree.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `55d36a4`.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "SDF field generation optimization preflight" --profile runtime --plan docs/notes/sdf_field_generation_optimization_preflight_PHASED_PLAN.md --contract docs/contracts/sdf_field_generation_optimization_preflight.contract.json` appended `ck:ffee4b30` and locked the active contract.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/sdf_field_generation_optimization_preflight.contract.json --out-json artifacts/validation/sdf_field_generation_optimization_preflight_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Roadmap updates: `spec_intake/_STATUS.md`, `DEFERRED_THREADS.md`, `KNOWN_ISSUES.md`, and `docs/notes/sdf_field_pack_near_term_TODO.md` now name field-generation algorithm/caching measurement and optimization as the selected next SDF performance path, with per-row/multi-field downsample deferred behind it.
- Stale-route grep: `rg -n "Next choose between per-row|field-generation algorithm/caching work or per-row|GPU Color Pipeline postprocess with measured client evidence|remain deferred$" DEFERRED_THREADS.md KNOWN_ISSUES.md spec_intake\_STATUS.md docs\notes\sdf_field_pack_near_term_TODO.md docs\notes\sdf_field_generation_optimization_preflight_PHASED_PLAN.md` returned no matches.
- Hostile-audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/sdf_field_generation_optimization_preflight_PHASED_PLAN.md --out-json artifacts/validation/sdf_field_generation_optimization_preflight_hostile_audit.json` passed.
- Code quality: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/sdf_field_generation_optimization_preflight_code_quality.json` passed.
- Diff check: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_generation_optimization_preflight_diff_check --log artifacts/logs/sdf_field_generation_optimization_preflight_diff_check.log --out-json artifacts/validation/sdf_field_generation_optimization_preflight_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.

## Hostile Audit

- Status: complete
- Did I actually select field-generation optimization as the next SDF performance path instead of leaving the roadmap ambiguous?
- Did I keep this slice docs-only and avoid mutating product code under a preflight contract?
- Did I preserve the already-shipped Lens Field v2 sign-contrast, SDF postprocess CUDA, capture/replay, and adaptive field-resolution assumptions?
- Did I identify the measurement gaps before proposing algorithm/cache changes?
- Did I avoid per-row downsample, authored-pack UI, SDF-native lanes, and Color Pipeline redesign?

## Audit Passes

- [done] Pass 1 - scope audit for docs-only preflight boundaries.
- [done] Pass 2 - call-stack audit for field-generation measurement gaps.
- [done] Pass 3 - roadmap audit for selected next path and deferred work.
- [done] Pass 4 - clean re-read after validation.

## Audit Findings

- [done] Finding 1: the roadmap/status docs still described field generation as one option among several after the user selected it as the next foundational path. Repaired by updating `spec_intake/_STATUS.md`, `DEFERRED_THREADS.md`, `KNOWN_ISSUES.md`, and `docs/notes/sdf_field_pack_near_term_TODO.md` to name field-generation measurement/optimization as the selected next path and defer per-row/multi-field downsample.
- [done] Finding 2: the current report surfaces split base render, field, postprocess, and total SDF cost, but not the field-generation sub-stages needed to choose algorithm/cache work safely. Repaired in this preflight by making the next implementation branch start with measurement-only stage telemetry before any optimization.

## Notes

- This preflight intentionally does not claim the FPS problem is solved.
- The next implementation branch should be viewer-first/runtime-visible because it will add report fields and no-mouse published-runtime proof.
