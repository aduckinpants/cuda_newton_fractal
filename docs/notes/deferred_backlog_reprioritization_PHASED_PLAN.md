# Deferred Backlog Reprioritization

## Current Phase

Complete - backlog docs reconciled, validated, checkpointed, and receipt-backed.

## Phase Checklist

- [x] Phase 0 - create this plan/contract, lock the docs-only slice, and record starting repo state.
- [x] Phase 1 - reconcile stale deferred/current-planning docs against the current shipped parameter, pacing, capture, equation-pack, callable, and toolkit surfaces.
- [x] Phase 2 - write a difficulty-to-reward ranked backlog that separates quick wins, product polish, strategic feature productization, and major architecture work.
- [x] Phase 3 - hostile-audit the refreshed docs for stale claims, overbroad scope, and false completion language.
- [x] Phase 4 - validate contract, plan sync, hostile audit, code-quality baseline, diff hygiene, checkpoint, receipts, and clean-tree stop point.

## Explicit User Asks

- [done] Review all deferred issues and re-prioritize based on difficulty-to-reward ratio.
- [done] Compare current planning against work left on the floor.
- [done] Update the repo documentation so a future session starts from current truth rather than stale campaign text.
- [done] Keep this as a documentation/planning reconciliation slice, not implementation.

## Scope

In scope:
- `DEFERRED_THREADS.md` backlog truth and current priority order.
- `KNOWN_ISSUES.md` issue-state cleanup where current plans already supersede stale entries.
- `spec_intake/_STATUS.md` active/deferred planning index cleanup.
- This plan/contract and final handoff entry.

Out of scope:
- Product code, tests, schema, renderer, runtime harness, Color Pipeline behavior, pacing policy changes, and new fractal features.
- Claiming any open implementation item is fixed unless current checked-in docs and proof already support that claim.
- Physical mouse automation or live viewer proof changes.

## Current Repo Truth Inputs

- Bootstrap at slice start reported branch `codex/parameter-functionality-campaign`, head `55be8c8`, clean tree, active old contract `pfc_param_verification_review`.
- `HANDOFF_LOG.md` records the parameter functionality campaign closed through Step 9, slider-audit hardening, and parameter verification review.
- `spec_intake/_STATUS.md` says `CliBridgeV2_GpuSampleFn` K1-K5 and V2-A..G are done, while broader multi-client/socket transport remains deferred.
- `docs/contracts/parameter_functionality_campaign_PHASED_PLAN.md` records the campaign through Step 9 and the slider audit hardening as closed.
- `docs/contracts/fps_debounce_measurement_PHASED_PLAN.md` and `docs/notes/capture_finding_aspect_camera_PHASED_PLAN.md` record later pacing/capture repairs, so older deferred pacing text must be marked as superseded rather than treated as untouched April state.
- `docs/notes/generic_cuda_equation_pack_PAUSE_README.md` records equation-pack v1 as shipped through normal dropdown/main viewport/Color Pipeline bridge, with persistence/catalog/authoring/Salticid integration paused.
- `docs/notes/fractal_toolkit_modular_catalog_magnet_wave_PHASED_PLAN.md` records Magnet Type I as shipped and next-generation families as staged behind specific architecture contracts.
- `docs/notes/fractal_control_surface_repair_campaign_PHASED_PLAN.md` records animation applicability fixed while all-fractal descriptor proof expansion and Julia/Nova policy follow-ups remain open.

## Target Backlog Shape

The refreshed docs should leave future work in four buckets:

1. Quick trust/QoL cleanup: stale-doc cleanup, diagnostics output-path cleanup, Lens SDF control truth, known low-cost tuning notes.
2. Product polish with high user reward: categorized selector/view presets, camera/dive behavior, smooth-escape/color tuning.
3. Strategic feature productization: equation-pack persistence/catalog/editor, callable/transpiler handoff, fractal extension gallery/tooling.
4. Major architecture: perturbation expansion, remaining catalog refactor seams, Lyapunov/IFS/attractors/3D DE/raymarch, next-generation explanation-state families.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed before mutation.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported no staged, unstaged, or untracked files before mutation.
- Mandatory docs read: `AGENT_WORKING_PROTOCOL.md`, `AGENT_TERMINAL_PROTOCOL.md`, `C:\code\salticid-cuda\docs\testing_cheat_sheet.md`, `.github/copilot-instructions.md`, `docs/PHASED_PLAN_CONTINUITY_PROTOCOL.md`, `DEFERRED_THREADS.md`, `KNOWN_ISSUES.md`, `spec_intake/_STATUS.md`, and recent `HANDOFF_LOG.md` entries.
- Documentation updated: `DEFERRED_THREADS.md`, `KNOWN_ISSUES.md`, and `spec_intake/_STATUS.md` now distinguish shipped work, deferred follow-ons, and difficulty/reward priority order.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/deferred_backlog_reprioritization.contract.json --out-json artifacts/validation/deferred_backlog_reprioritization_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile-audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/deferred_backlog_reprioritization_PHASED_PLAN.md --out-json artifacts/validation/deferred_backlog_reprioritization_hostile_audit.json` passed.
- Code-quality baseline: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/deferred_backlog_reprioritization_code_quality.json` passed with score 95/100 and baseline check passed.
- Diff hygiene: `py -3.14 tools/viewer_host_run_logged_command.py --label deferred_backlog_diff_check --log artifacts/logs/deferred_backlog_diff_check.log --out-json artifacts/validation/deferred_backlog_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.

## Hostile Audit

- Status: complete
- Did I actually remove stale planning contradictions instead of adding another stale summary? Yes: `DEFERRED_THREADS.md` now marks sample_fn/K1-K5 as closed follow-ons, catalog work as partially shipped, and equation-pack as paused productization.
- Did I keep the docs refresh from becoming implementation work? Yes: no product code, tests, schema, renderer, runtime harness, Color Pipeline, capture, or pacing policy files were touched.
- Did I preserve unresolved user-visible risks instead of marking them complete from old receipts alone? Yes: viewer responsiveness is now a latency-proof follow-up, view presets/camera/dive/color tuning remain open, and perturbation/new substrates remain deferred.
- Did I distinguish shipped parameter/pacing/capture/equation-pack work from remaining productization or architecture work? Yes: shipped surfaces and resume gates are split in `DEFERRED_THREADS.md`, `KNOWN_ISSUES.md`, and `spec_intake/_STATUS.md`.
- Did I leave the backlog ordered by difficulty-to-reward rather than by historical accident? Yes: the ranked table now starts with low-risk trust/QoL work and pushes perturbation/new-substrate families behind architecture gates.

## Audit Passes

- [done] Pass 1 - stale-state audit found `DEFERRED_THREADS.md` still described K1-K5/sample_fn as active and common catalog work as queued despite later shipped work.
- [done] Pass 2 - priority-order audit moved quick docs/tooling/Lens cleanup ahead of larger view/camera/color work, and kept perturbation/new substrates behind architecture gates.
- [done] Pass 3 - stale-phrase audit found no remaining stale active-state, old queued-state, or pre-closeout phrases in the refreshed planning surfaces except expected shipped-work names.
- [done] Pass 4 - clean re-read after validation found no additional real issue in the refreshed docs boundary, and product code/test/schema/runtime files remain untouched.

## Audit Findings

- [done] Finding 1: stale deferred text claimed the CUDA sample_fn/Optimization/Reflexive initiative was active; refreshed docs now mark core work closed and keep only follow-ons.
- [done] Finding 2: stale common-fractal text still listed Spider/Celtic/Perpendicular/Magnet as future safe-wave work; refreshed docs mark those as shipped or partially shipped and prioritize selector/view presets instead.
- [done] Finding 3: pacing/capture history was easy to misread as untouched April debt; refreshed docs separate repaired pacing/capture bugs from the remaining end-to-end latency-proof gap.
- [done] Finding 4: active/follow-up spec status blurred completed FractalExtensions and CLI Bridge work; `_STATUS.md` now distinguishes completed PoC/core transport from follow-up choices.
- [done] Clean re-audit: after the repair, contract validation, phased-plan sync, code-quality baseline, and diff hygiene passed or were in-progress with no additional docs-scope defect found.

## Notes

- Exit criteria: refreshed docs explicitly say what is shipped, what remains deferred, what is superseded, and what order should be used for the next campaigns.
- Validation target: docs-only validation and code-quality baseline. No native/runtime publish is required because this slice must not touch product behavior.
