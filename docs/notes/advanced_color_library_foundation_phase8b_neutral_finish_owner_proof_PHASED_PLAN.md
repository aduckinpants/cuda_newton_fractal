# Advanced Color Library Foundation Phase 8B - neutral_finish Owner Proof

## Current Phase

Phase 3 - the bounded neutral_finish owner path is implemented and revalidated. Native helper tests, runtime publish, and the focused published-runtime neutral_finish witness are green; checkpoint commit, receipts, and handoff remain pending because this turn stops before repo closure.

## Phase Checklist

- [x] Phase 1 - add focused REDs proving `neutral_finish` is still inventory-only across the runtime-backed Grading catalog, live import/apply bridge, runtime grading stack math, diagnostics/archive persistence, and reset/default seams while existing shipped Grading rows stay intact
- [x] Phase 2 - land the smallest truthful `neutral_finish` owner path using the existing grading owner trio (`grade.exposure`, `grade.saturation`, `grade.contrast`) plus a dedicated runtime stack-math branch, without widening into `tone_map_finish`, `grade.glow`, or `balance_void_grade`
- [ ] Phase 3 - validate, hostile-audit, runtime-publish, published-runtime proof, write receipts, and checkpoint the `neutral_finish` slice cleanly

## Explicit User Asks

- [done] Ship `neutral_finish` as a real runtime-backed Grading row end to end.
- [done] Treat basin-default grading lane retention as already closed at `e2e14df`.
- [open] Keep `tone_map_finish` deferred for this session.
- [open] Keep `grade.glow` and `balance_void_grade` out of scope.
- [open] Do not reopen Source weighted-blend work, manual archive work, hooks/workflow tooling, crash recovery, or unrelated UI polish.
- [done] Prove descriptor/catalog truth, live bridge import/apply, runtime math, diagnostics persistence, archive persistence, reset/default behavior, and runtime proof for `neutral_finish`.

## Presumption Loop

The controlling risk is owner fraud: `neutral_finish` currently exists only as inventory text, so a naive slice could expose it in the editor or persistence paths without giving it real grading math. The falsifiable local hypothesis for this slice is narrow: `neutral_finish` can ship without new grading float owner fields by reusing `ColorPipelineGradingRuntimeParams::{exposure,saturation,contrast}` together with the existing legacy mirror owners `KernelParams::{exposure,color_saturation,color_contrast}`, while adding only a new grading preset mapping and a dedicated `neutral_finish` branch in `ui_app/src/escape_time_coloring.h` so stack rows use their own parameters instead of the last-row legacy mirror.

## Presumption Evidence

- `docs/notes/advanced_color_library_foundation_oracle_and_inventory.md` and `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md` still list `neutral_finish` only as deferred inventory, and a repo search finds no `neutral_finish` code or tests in `ui_app/`.
- `ui_app/src/fractal_types.h` already persists Grading stack entries through `ColorPipelineGradingRuntimeParams::{exposure,saturation,contrast}` and `ColorPipelineGradingStackEntry`.
- `ui_app/src/color_pipeline_core.h` and `ui_app/src/color_pipeline_window.h` currently recognize only `contrast_lift`, `phase_finish`, `band_finish`, and `basin_default` for runtime-backed Grading rows and live bridge import/apply.
- `ui_app/src/escape_time_coloring.h` currently applies explicit per-row stack math only for `escape_default` and `phase_default` / `bands_default`; any new grading preset would otherwise fall through and ignore stack-entry parameter values.
- `ui_app/src/diagnostics_state_io.cpp`, `ui_app/src/diagnostics_capture.cpp`, and `ui_app/src/runtime_reset.cpp` already carry generic Grading stack persistence and reset surfaces, so adding one more shipped grading mapping should be owner reuse rather than a new serialization system.
- `tone_map_finish` remains adjacent but deferred because it would require separate grading semantics beyond this neutral owner trio, while `grade.glow` remains excluded because the checked-in foundation authority already records that it has no reusable runtime owner yet.

## Proof Ledger

- Predecessor checkpoint: `ck:5972173a` closed the bounded generic Source weighted-blend slice.
- Predecessor checkpoint: `ck:47bd4450` closed basin-default grading lane retention; the current repo bootstrap reports `branch=feature/advanced-color-pipeline-draft-editor-reframe`, `HEAD=e2e14df`, and a clean worktree.
- Session diagnosis: the active locked contract is still the closed basin-default slice (`docs/contracts/advanced_color_library_foundation_phase8a_basin_default_lane_retention.contract.json`), so a new checked-in neutral_finish plan/contract must be opened and locked before mutation.
- Owner diagnosis: `neutral_finish` still exists only in the checked-in inventory/closure docs and not in the runtime-backed Grading core/window/runtime/test seams.
- Owner diagnosis: the live grading owner trio already exists as `KernelParams::{exposure,color_saturation,color_contrast}` and `ColorPipelineGradingRuntimeParams::{exposure,saturation,contrast}`, so this slice starts by proving whether those owners are sufficient before any wider grading rewrite is considered.
- RED progression: focused native-helper reruns moved the failure one owner seam at a time through core mapping, live bridge, diagnostics classification, runtime grading math, reset/default proof, and archive persistence proof before the repaired state went green again.
- Repaired-state validation: `artifacts/neutral_finish_archive_proof_rerun.log` ends with `All helper tests passed`; `artifacts/neutral_finish_runtime_publish.log` published `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`; and `artifacts/neutral_finish_runtime_proof.log` reports `1 passed, 23 deselected` for the focused published-runtime neutral_finish witness.

## Hostile Audit

- Status: complete
- Required posture: assume the first implementation will either expose `neutral_finish` only in the editor, reuse the legacy mirror instead of real stack-entry math, regress `contrast_lift` / `phase_finish` / `band_finish` / `basin_default`, or silently widen into `tone_map_finish` / `grade.glow` until repaired proof says otherwise.

## Audit Passes

- [done] Pass 1 - the diagnostics round-trip RED exposed the first real owner defect: `TryMirroredColoringModeForPipeline()` rejected `smooth_escape` / `cyclic_escape` / `neutral_default` as unsupported. `ui_app/src/fractal_family_rules.h` now accepts `neutral_default` across the shipped non-basin mirrored tuples.
- [done] Pass 2 - the repaired-state native rerun exposed the next real owner defect: `TryBuildColorPipelineSelectionFromDraft()` still treated the source/palette bridge grading id as authoritative and rejected `neutral_finish` as a mismatched first row. `ui_app/src/color_pipeline_window.h` now promotes the first enabled Grading row to the runtime grading id for supported tuples.
- [done] Pass 3 - reran the repaired state through full native helper tests after the archive proof landed, runtime publish, and the focused published-runtime neutral_finish pytest witness; no additional real defect found in the touched seams, and deferred rows remained untouched.

## Audit Findings

- [done] Real defect found: diagnostics load failed because mirrored-mode classification excluded `neutral_default` for the shipped escape-time tuples, so saved neutral_finish states were rejected as unsupported. `ui_app/src/fractal_family_rules.h` now maps those tuples back onto the existing mirrored coloring modes.
- [done] Real defect found: live draft apply/resync failed because the draft builder still forced the source/palette bridge grading and treated `neutral_finish` as a mismatch, so the first Grading row could not become runtime-authoritative. `ui_app/src/color_pipeline_window.h` now derives the pipeline grading from the first enabled Grading row for supported tuples.
- [done] Clean re-audit evidence: repaired-state proof is recorded in `artifacts/neutral_finish_archive_proof_rerun.log`, `artifacts/neutral_finish_runtime_publish.log`, and `artifacts/neutral_finish_runtime_proof.log`.

## Notes

- Expected owner files for this bounded slice:
  - `docs/contracts/advanced_color_library_foundation_phase8b_neutral_finish_owner_proof.contract.json`
  - `docs/notes/advanced_color_feature_restart_inventory.md`
  - `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md`
  - `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`
  - `docs/notes/advanced_color_library_foundation_phase8b_neutral_finish_owner_proof_PHASED_PLAN.md`
  - `ui_app/src/color_pipeline_core.h`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/fractal_types.h`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/src/diagnostics_capture.cpp`
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/src/runtime_reset.cpp`
  - `ui_app/tests/test_color_pipeline_core.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
  - `ui_app/tests/test_escape_time_coloring.cpp`
  - `ui_app/tests/test_diagnostics_state_io.cpp`
  - `ui_app/tests/test_finding_archive_actions.cpp`
  - `ui_app/tests/test_runtime_reset.cpp`
  - `ui_app/tests/test_color_pipeline_window.cpp`
  - `tests/test_fractal_runtime_explaino_escape_variants.py`
- Non-goals for this bounded slice:
  - do not ship `tone_map_finish`
  - do not widen into `grade.glow` or `balance_void_grade`
  - do not reopen Source weighted-blend, manual archive, or workflow-tooling work
  - do not claim editor-only, persistence-only, or bridge-only `neutral_finish` as shipped
  - do not create a second grading-owner authority outside the existing grading stack params plus legacy compatibility mirror

## Resume Point

Resume from formal closure only: run the contract/plan/hostile-audit validators, write receipts, append `HANDOFF_LOG.md`, and checkpoint the neutral_finish slice once a commit is authorized.

## Action Hostile Review

- Action ID: action-20260513-neutral-finish-reds-29
- Suspected Failure Mode: the slice may expose `neutral_finish` in the catalog or bridge without real stack-entry grading math, or it may piggyback on deferred `tone_map_finish` / `grade.glow` semantics.
- Correct Owner/Action: add REDs first across core/window/runtime/persistence/reset seams, then land only the bounded `neutral_finish` owner path through the existing exposure/saturation/contrast grading owners plus a dedicated stack-math branch.
- Proof Surface: focused native helper tests for the touched owner seams, runtime publish, published-runtime proof or the closest existing truthful runtime rail, contract validation, phased-plan sync, and hostile-audit validation.
- Outcome: done for the current implementation action: REDs, owner-path repairs, archive proof, full native helper proof, runtime publish, and the focused published-runtime neutral_finish witness are green; checkpoint and receipts remain pending.
- Blocked Action: none.