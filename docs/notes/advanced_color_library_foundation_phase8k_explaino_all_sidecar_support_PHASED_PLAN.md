# Advanced Color Library Foundation Phase 8K - Explaino-all Sidecar Support

## Current Phase

Closed - `explaino_all` sidecar support is landed and proven across the filtered descriptor, real schema-derived sidecar window, published runtime sidecar loop, runtime `--describe-functions` catalog, and generic probe coverage surfaces without widening into alias migration or generic sidecar redesign.

## Phase Checklist

- [x] Phase 1 - replace the closed phase8j lock with a new bounded sidecar-support slice, inspect the sidecar owner seams, and add REDs that prove the shipped sidecar path still rejects `explaino_all`
- [x] Phase 2 - land the smallest truthful repair so the filtered probe/sidecar catalog supports `explaino_all` without reopening alias migration, generic sidecar redesign, or unrelated Explaino-family work
- [x] Phase 3 - rerun narrow native/real-schema/runtime proofs, hostile-audit the repaired state, checkpoint the slice, write machine receipts, clear stale closeout text, and stop on a clean committed head

## Explicit User Asks

- [closed] Support `explaino_all` in the Explaino sidecar so the viewer no longer shows `Unsupported required enum selection for sidecar param: fractal.view.fractal_type=explaino_all`.
- [closed] Keep this as a bounded Explaino-all follow-up rather than reopening alias migration, generic sidecar redesign, enforcement work, or unrelated advanced-color core work.
- [closed] Prove the real shipped sidecar path, not just a synthetic helper fixture, accepts the canonical Explaino identity after the repair.
- [closed] Close only with hostile review, checkpoint commit, validation receipt, contract proof receipt, clean tree, and committed-head stale-plan reread.

## Proof Ledger

- Bootstrap on 2026-05-15 re-proved branch=`feature/advanced-color-pipeline-draft-editor-reframe`, `HEAD=1574d5c`, clean tree, and an active lock on the already closed `advanced_color_library_foundation_phase8j_explaino_all_canonical_identity_axis_registry` contract.
- `py -3.14 tools/viewer_host_repo_status.py` re-proved `staged=none | unstaged=none | untracked=none`, so this follow-up starts fresh rather than as carryover.
- Minimal continuity preflight re-locked the broader phase8d continuity contract only long enough to create and lock this successor sidecar-support slice truthfully.
- Live viewer evidence now shows the Explaino Sidecar pane failing on the canonical identity with `Unsupported required enum selection for sidecar param: fractal.view.fractal_type=explaino_all`.
- Current owner seam points at the filtered probe catalog, not a separate sidecar enum table:
  - `ui_app/src/function_descriptor.cpp` owns the probe-supported fractal-type filter through `kSupportedProbeFractalTypes`, `IsProbeSamplingImplementedForFractalTypeId`, and `FilterProbeSupportedFractalTypeOptions`
  - `ui_app/src/explaino_sidecar_model.cpp` rejects required enum selections when the current bound enum id is absent from the descriptor options
  - `ui_app/src/explaino_sidecar_window.cpp` and `ui_app/tests/test_explaino_sidecar_schema_contract.cpp` consume the real schema-derived `BuildEngineCatalog(...)` path used by the shipped viewer sidecar
- Bounded-slice judgment before RED: this follow-up can stay inside the catalog/sidecar support seam if `explaino_all` only needs to survive the probe-supported fractal-type filtering plus focused witness refresh; no legacy alias migration or generic sidecar redesign is required.
- RED proof on the first modified head:
  - `artifacts/phase8k_sidecar_red/red_native.log` failed with `Expected unsupported fractal types to be filtered from the enum options, got: 7`, proving the filtered descriptor still dropped `explaino_all`
  - `artifacts/phase8k_sidecar_red/red_schema_contract.log` failed with `Expected the shipped schema-derived sidecar catalog to advertise explaino_all as a supported fractal.sample enum option`, proving the real viewer-side sidecar catalog still rejected the canonical identity
- First GREEN proof on the repaired head:
  - `artifacts/phase8k_sidecar_red/green_native.log` passed `test_function_descriptor.exe` and `test_explaino_sidecar_schema_contract.exe`
  - `artifacts/phase8k_sidecar_red/build_runtime.log` rebuilt the published runtime
  - `artifacts/phase8k_sidecar_red/runtime_sidecar_live.log` passed the focused live sidecar paced-loop witness for both `explaino` and `explaino_all`
- Hostile-audit follow-up found one truthful collateral gap after GREEN: the runtime `--describe-functions` witness and the generic probe coverage table still encoded the pre-`explaino_all` supported-type surface. Those witnesses were refreshed in-slice rather than waved away.
- Final proof sweep on the current closure head:
  - `artifacts/phase8k_sidecar_red/final_build_runtime.log` rebuilt the published runtime at `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`
  - `artifacts/phase8k_sidecar_red/final_native.log` passed `test_function_descriptor.exe` and `test_explaino_sidecar_schema_contract.exe`
  - `artifacts/phase8k_sidecar_red/final_runtime_sidecar_live.log` passed the focused live sidecar paced-loop witness for both `explaino` and `explaino_all`
  - `artifacts/phase8k_sidecar_red/final_function_descriptor_cli.log` passed the runtime catalog witness after refreshing the stale advertised-type set
  - `artifacts/phase8k_sidecar_red/final_probe_coverage.log` passed generic probe coverage with both `explaino_all` and `explaino_balance_void` explicitly exercised

## Hostile Audit

- Status: closed
- Required posture: assume the first fix only patches a unit fixture, only fixes a helper catalog, or only rebrands `explaino` while the real shipped sidecar path still rejects `explaino_all` until hostile proof disproves each failure mode.

## Audit Passes

- [x] Pass 1 - add REDs that fail because the filtered descriptor and real schema sidecar path still exclude `explaino_all`
- [x] Pass 2 - clean re-read the touched seams after the first GREEN and confirm no additional real defect found beyond the stale runtime-catalog and probe-coverage witnesses repaired in this slice
- [x] Pass 3 - rerun the narrow native, real-schema, focused runtime sidecar, runtime catalog, and probe-coverage witnesses on the repaired head and prove the repaired state cleanly before the stale-plan reread

## Audit Findings

- [closed] RED proved the real failure mode: `explaino_all` existed in enum/schema/runtime identity surfaces, but the filtered `fractal.sample` descriptor still dropped it before the sidecar model validated the required enum selection. Adding `FractalType::explaino_all` to `kSupportedProbeFractalTypes` resolved the live sidecar rejection.
- [closed] Hostile audit found and resolved a post-GREEN omission: `tests/test_function_descriptor_cli.py` and `ui_app/tests/test_fractal_probe_coverage.cpp` were still proving the old advertised probe surface and now explicitly cover the canonical support set, including the pre-existing `explaino_balance_void` descriptor entry.

## Notes

- Expected owner files for this bounded slice:
  - `docs/contracts/advanced_color_library_foundation_phase8k_explaino_all_sidecar_support.contract.json`
  - `docs/notes/advanced_color_library_foundation_phase8k_explaino_all_sidecar_support_PHASED_PLAN.md`
  - `docs/notes/advanced_color_library_foundation_explaino_all_launch_anchor.md`
  - `docs/notes/advanced_color_library_foundation_phase8j_explaino_all_canonical_identity_axis_registry_PHASED_PLAN.md`
  - `ui_app/src/function_descriptor.cpp`
  - `ui_app/tests/test_function_descriptor.cpp`
  - `ui_app/tests/test_explaino_sidecar_model.cpp`
  - `ui_app/tests/test_explaino_sidecar_window.cpp`
  - `ui_app/tests/test_explaino_sidecar_schema_contract.cpp`
  - `ui_app/tests/test_fractal_probe_coverage.cpp`
  - `tests/test_fractal_runtime_explaino_sidecar_live.py`
  - `tests/test_function_descriptor_cli.py`
  - `HANDOFF_LOG.md`
- Non-goals for this slice:
  - no legacy Explaino alias migration
  - no Explaino-all preset projection or enforcement work
  - no generic sidecar UX redesign
  - no unrelated advanced-color core reopen
  - no historical archive compatibility work

## Re-entry Note

This slice is closed. Use this plan, the linked proof logs, and `HANDOFF_LOG.md` only as historical evidence if a new Explaino sidecar regression reopens the seam.

## Action Hostile Review

- Action ID: action-20260515-explaino-all-sidecar-support
- Suspected Failure Mode: the repair only updates a local test fixture or helper path while the real schema-derived `fractal.sample` descriptor still filters out `explaino_all`, so the shipped Explaino Sidecar pane keeps failing on the canonical identity.
- Correct Owner/Action: keep the fix on the probe-catalog filter seam, prove the real schema sidecar window path accepts `explaino_all`, and add the narrowest runtime witness needed to show the shipped viewer-side sidecar path is no longer blocked on the canonical identity.
- Proof Surface: focused descriptor/sidecar native tests, real-schema sidecar contract proof, runtime publish, focused published-runtime sidecar witness, contract validation, phased-plan sync, hostile-audit validation, checkpoint commit, validation receipt, contract proof receipt, and committed-head stale-plan reread.
- Blocked Action: no legacy alias migration, no generic sidecar redesign, no enforcement follow-up, and no unrelated advanced-color or archive work in this slice.
