# ExplainO Pattern Bank / Root Pattern References V1 Phased Plan

## Explicit User Asks

- Merge the completed `codex/root-metric-color-source-wave3` branch into `master` before starting this slice.
- Branch `codex/explaino-pattern-bank-root-refs` from the merged and rearward-reviewed `master`.
- Add a bounded two-slot ExplainO root-pattern bank with `Pattern A` preserving current behavior and `Pattern B` as generated-only secondary authority.
- Route root-field consumers and root-aware Color Pipeline source rows through explicit `pattern_ref` values.
- Prove `explaino_magnet_root_well` can use regular N roots for dynamics while Color Pipeline root rows use legacy roots.
- Validate, checkpoint, push, and stop before broader Wave 4, graph-editor, root-index-pattern, or arbitrary pattern-algebra work.

## Current Phase

Phase 6 - hostile audit, receipts, rearward review, push, and clean-tree closeout.

## Phase Checklist

- [x] Phase 0: fast-forward `master` to closed Wave 3 root-phase head and push.
- [x] Phase 0: rearward-review merged `master`.
- [x] Phase 0: create `codex/explaino-pattern-bank-root-refs`.
- [x] Phase 0: validate this plan/contract and lock the active slice.
- [x] Phase 1: add RED/native tests for pattern descriptors, root-aware source refs, state/schema visibility, and invalid-ref fail-closed behavior.
- [x] Phase 2: implement two-slot root-pattern authority and root-field consumer `pattern_ref` routing.
- [x] Phase 3: implement root-aware Color Pipeline source-row `signal.root_pattern_ref` routing.
- [x] Phase 4: add report, Capture Finding, and `fractal-state.json` pattern/consumer truth.
- [x] Phase 5: published runtime proof with `explaino_magnet_root_well` primary dynamics plus secondary root-aware source row.
- [ ] Phase 6: hostile audit, hardening, receipts, rearward review, push, clean tree, and stop before broader pattern work.

## Scope

In scope:

- Two fixed pattern slots only: `primary` / `secondary`.
- Pattern A / `primary` is current root authority and must preserve current descriptor hashes and visuals unless explicitly selected otherwise.
- Pattern B / `secondary` is generated-only and supports `legacy_quartic_v1` and `regular_ngon_v1`.
- `explaino_root_field_pattern_ref` for root-field consumer lanes.
- `signal.root_pattern_ref` for `root_proximity` and `root_phase` Color Pipeline source rows.
- Minimal UI for Pattern B layout/count, Dynamics Root Pattern, and per-row Root Pattern on root-aware source rows.
- Runtime report and Capture Finding review truth for `root_patterns[]` and `root_pattern_consumers[]` while preserving existing flat report fields.

Out of scope:

- Unlimited pattern lists, graph editor, pattern algebra, or Factorio-style composition UI.
- Pattern B custom coordinate editing.
- `root_index` pattern refs or categorical root-index routing changes.
- Root adjacency, pair-gap metrics, additional root-metric sources, or Wave 4 idea work.
- SDF ops, SDF-native lanes, perturbation zoom, Salticid adapter/removal, or unrelated Color Pipeline redesign.
- Physical mouse automation.

## Phase 0 - Branch Hygiene And Lock

### Completed

- `master` was fast-forwarded from `3f2f149` to Wave 3 closeout `5d6a6a1`.
- `master` was pushed to `origin/master`.
- Rearward review returned `ok` for `5d6a6a1`.
- Branch `codex/explaino-pattern-bank-root-refs` was created from that merged head.

### Exit Criteria

- Contract validates.
- Phased plan sync validates.
- Active slice is locked with `viewer_host_begin_work_slice.py`.
- No product code mutation before lock.

## Phase 1 - RED Tests

Required REDs:

- Pattern A descriptor/hash parity is locked against current descriptor behavior.
- Pattern B `legacy_quartic_v1` and `regular_ngon_v1` descriptors are deterministic.
- Invalid pattern refs fail closed with a structured reason.
- `root_proximity(primary)` and `root_proximity(secondary)` can differ when patterns differ.
- `root_phase(secondary)` is deterministic and uses secondary roots.
- State/schema/binding tests prove Pattern B controls and per-row root pattern controls round-trip and visibility is scoped to root-aware lanes.

## Phase 2 - Root Pattern Authority

Implement:

- `primary` and `secondary` pattern ref parsing/formatting.
- Pattern A delegates to existing root descriptor authority with no default behavior drift.
- Pattern B is generated-only:
  - default layout `legacy_quartic_v1`, count `4`;
  - optional layout `regular_ngon_v1`, count bounded by existing descriptor capacity;
  - no secondary custom coordinate storage in v1.
- Root-field consumer lanes read `explaino_root_field_pattern_ref`.
- Derived/effective roots are never written back to custom root authority.

## Phase 3 - Root-Aware Color Pipeline Source Rows

Implement:

- `signal.root_pattern_ref` runtime param for `root_proximity` and `root_phase`.
- Default `primary` preserves current behavior.
- Per-row `secondary` samples from Pattern B.
- Non-root-aware rows do not expose the Root Pattern control.
- `root_index` remains unchanged on the existing root-basin path.

## Phase 4 - Report And Capture Truth

Implement:

- `state.json` stores authoritative controls and selected refs.
- Runtime report includes `root_patterns[]` and `root_pattern_consumers[]` with ids, layout, count, source kind, hashes, consumer refs, and fail-closed reasons where applicable.
- `fractal-state.json` includes the same review-focused pattern and consumer truth.
- Existing flat root-field/root-metric report fields remain for compatibility.

## Phase 5 - Published Runtime Proof

Target proof:

- Select `explaino_magnet_root_well`.
- Set Pattern A / primary to `regular_ngon_v1`, count `11`.
- Set Pattern B / secondary to `legacy_quartic_v1`, count `4`.
- Set dynamics/root-field consumer ref to `primary`.
- Set a root-aware Color Pipeline row (`root_proximity` or `root_phase`) to `secondary`.
- Prove selector identity, report refs, frame hash sensitivity, and Capture Finding reload/replay parity.

## Phase 6 - Hostile Audit And Close

### Exit Criteria

- Hostile audit records at least one real finding or three clean passes after repair.
- Focused native rails pass.
- Runtime publish passes.
- Published no-mouse runtime proof passes.
- Contract validation, plan sync, code quality, hostile-audit validation, and diff check pass.
- Validation and contract proof receipts are written.
- Rearward review is `ok` on final committed head.
- Branch is pushed and tree is clean.
- Preplanned work for this slice is exhausted; stop before broader Wave 4 or graph-editor work.

## Hostile Audit

- Status: complete
- Classification: real finding repaired; final pass found no additional V1 blocker.
- Replan boundary: stop after V1 pattern refs are proven; do not start unlimited pattern lists, graph editor, or additional root-metric functions.

Audit questions:

- Did Pattern A preserve current root descriptor behavior and hashes?
- Did Pattern B stay generated-only and avoid writing derived roots back as authority?
- Did root-field consumers and root-aware Color Pipeline rows route through explicit refs instead of implicit globals?
- Did `root_index` stay unchanged?
- Did unsupported or invalid refs fail closed with structured reasons?
- Did reports and Capture Finding describe both patterns and consumers truthfully?
- Did runtime proof use the published viewer path, not helper-only evidence?
- Did the slice stop before Wave 4, graph UI, pattern algebra, or extra root-metric source growth?

## Audit Passes

- [x] Pass 1 - found replay proof gap: manual runtime state used `root_phase` as a loaded top-level/source-stack replay path, but that path exits before viewer startup on current runtime.
- [x] Pass 2 - re-read the repaired state after switching runtime proof to `root_proximity`; native `root_phase` secondary-pattern sampling remains covered by `test_escape_time_coloring`, and no additional real defect found in the V1 route.
- [x] Pass 3 - clean re-read of final diff, plan, contract, native proof, and published-runtime proof found no additional V1 blocker; root-phase replay hardening remains explicitly deferred outside this slice.

## Audit Findings

- [x] Finding 1: `root_phase` is not currently safe as a replay-loaded top-level/source-stack state in the persistent viewer harness. This slice does not broaden into that separate replay bug; the V1 runtime proof now uses `root_proximity` as allowed by the plan, and the native root-phase secondary-pattern signal test remains in place. Defer root-phase loaded-state hardening to a later Color Pipeline replay slice.

## Proof Ledger

| Item | Evidence |
| --- | --- |
| Wave 3 merge | `git merge --ff-only codex/root-metric-color-source-wave3` fast-forwarded `master` to `5d6a6a1`; `git push origin master` pushed it. |
| Merged-head rearward review | `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `5d6a6a1`. |
| Branch opened | `git checkout -b codex/explaino-pattern-bank-root-refs`. |
| Active slice lock | `py -3.14 tools/viewer_host_begin_work_slice.py --intent "ExplainO Pattern Bank / Root Pattern References V1" --profile runtime --plan docs/notes/explaino_pattern_bank_root_refs_PHASED_PLAN.md --contract docs/contracts/explaino_pattern_bank_root_refs.contract.json` opened `ck:c8a13420`. |
| RED native proof | `py -3.14 tools/viewer_host_run_logged_command.py --label explaino_pattern_bank_red_native --log artifacts/explaino_pattern_bank_root_refs/red_native.log -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_color_pipeline_core test_escape_time_coloring test_schema_binding test_diagnostics_state_io` failed first on missing `signal.root_pattern_ref`, proving the root-aware source-row gap. |
| Focused native proof | `py -3.14 tools/viewer_host_run_logged_command.py --label explaino_pattern_bank_native_final_focus --log artifacts/explaino_pattern_bank_root_refs/native_final_focus.log -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_color_pipeline_core test_escape_time_coloring test_schema_binding test_diagnostics_state_io test_viewer_ui_automation_report test_diagnostics_capture` passed. |
| Runtime publish | Final clean receipt command: `py -3.14 tools/viewer_host_run_logged_command.py --label explaino_pattern_bank_runtime_publish_final --log artifacts/explaino_pattern_bank_root_refs/runtime_publish_final.log -- cmd /c ui_app\build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`. Earlier `runtime_publish.log` captured a wrapper timeout while the build continued; it is not used as closeout proof. |
| Published runtime proof | Final command: `py -3.14 tools/viewer_host_run_logged_command.py --label explaino_pattern_bank_runtime_pytest_final --log artifacts/explaino_pattern_bank_root_refs/runtime_pytest_final.log -- py -3.14 -m pytest tests/test_fractal_runtime_root_field_consumers.py -q` passed: `6 passed`. Earlier `runtime_pytest_root_field_2.log` also passed after the replay-state repair. |
| Code quality | `py -3.14 tools/code_quality_audit.py --out artifacts/explaino_pattern_bank_root_refs/code_quality_audit.json` scored `93/100`, with no critical or error findings. |
| Contract validation | `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/explaino_pattern_bank_root_refs.contract.json --out-json artifacts/validation/explaino_pattern_bank_root_refs_contract_final.json` passed. |
| Plan sync | `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed on dirty plan state before closeout. |
| Receipts | Pending. |
| Rearward review | Pending. |
| Branch push | Pending. |
| Stop boundary | Pending final closeout; preplanned V1 pattern-bank product work is exhausted after receipts/rearward review/push. |
