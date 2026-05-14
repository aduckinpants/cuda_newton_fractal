# Advanced Color Library Foundation Phase 8E - tone_map_finish Owner Proof

## Current Phase

Complete - checkpoint commit `a59e657` and machine proof receipts closed the bounded `tone_map_finish` owner-proof slice. This plan remains historical closure evidence only and must not be read as live pre-closeout restart authority.

## Phase Checklist

- [x] Phase 1 - open and lock the bounded `tone_map_finish` slice, inspect the owner seams, and add focused REDs proving `tone_map_finish` is absent from the runtime-backed Grading catalog, live bridge, runtime math, diagnostics/archive persistence, and reset/default truth while existing shipped Grading rows stay intact
- [x] Phase 2 - land the smallest truthful `tone_map_finish` owner path using the existing grading owner trio (`grade.exposure`, `grade.saturation`, `grade.contrast`) plus a distinct tone-map-last runtime branch, without widening into `grade.glow`, `balance_void_grade`, archive recovery, or weighted blend
- [x] Phase 3 - validate through the phase8c command ladder, hostile-audit the repaired state, update the authority docs, and prepare the slice for checkpoint closeout and receipts

## Explicit User Asks

- [done] Ship `tone_map_finish` as a real runtime-backed Grading row end to end.
- [done] Treat weighted blend as already closed at `ck:5972173a`.
- [done] Treat basin-default as already closed at `ck:47bd4450`.
- [done] Treat `neutral_finish` as already closed at `ck:a0ce2d03`.
- [deferred] Keep `grade.glow` and `balance_void_grade` out of scope.
- [done] Do not reopen Source weighted-blend, manual archive, hooks/workflow tooling, crash recovery, anti-lie tooling, or unrelated UI polish.
- [done] Prove descriptor/catalog truth, live bridge import/apply, runtime math, diagnostics/archive persistence, reset/default behavior, and published-runtime truth for `tone_map_finish`.

## Presumption Loop

The controlling risk was owner fraud: `tone_map_finish` could have been exposed only in the editor or persistence paths while collapsing back onto `neutral_finish` semantics. The repaired state disproves that narrower lie: `tone_map_finish` reuses the existing grading owner trio (`ColorPipelineGradingRuntimeParams::{exposure,saturation,contrast}` plus `KernelParams::{exposure,color_saturation,color_contrast}`) but executes a distinct tone-map-last runtime grading branch instead of the shared neutral ordering.

## Presumption Evidence

- `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md` and `docs/notes/advanced_color_feature_restart_inventory.md` started this slice with `tone_map_finish` recorded as deferred inventory only.
- `ui_app/src/color_pipeline_core.h`, `ui_app/src/color_pipeline_window.h`, and `ui_app/src/enum_id_utils.h` now map `tone_map_finish` / `tone_map_default` into the runtime-backed Grading catalog, live bridge, diagnostics/archive ids, and reset/import surfaces.
- `ui_app/src/escape_time_coloring.h` now gives `tone_map_finish` a dedicated tone-map-last runtime grading branch while preserving the existing `contrast_lift`, `phase_finish`, `band_finish`, `basin_default`, and `neutral_finish` behavior.
- `ui_app/src/fractal_family_rules.h` now treats `tone_map_default` as escape-like for the shipped smooth-escape mirror tuples without widening it into phase/band or basin-only semantics.
- `grade.glow` remains excluded because no reusable runtime owner was introduced for it, and `balance_void_grade` remains a separate later grading/operator slice.

## Proof Ledger

- Bootstrap on 2026-05-13 reported `branch=feature/advanced-color-pipeline-draft-editor-reframe`, `HEAD=560cbd1`, a clean worktree, and active locked contract `advanced_color_library_foundation_phase8d_restart_authority_repair`.
- Minimal continuity preflight widened the closed phase8d contract just enough to create and lock this successor plan/contract, then immediately replaced it with the phase8e `tone_map_finish` slice lock.
- Focused RED progression moved the failure one seam at a time from the missing internal grading enum/id surface through live bridge/core owner paths and into the dedicated runtime grading-math witness before the repaired state went green.
- Native focused RED rail: `cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_red` is green on the repaired state.
- Native owner rail: `cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner` is green on the repaired state, including diagnostics, archive, and reset helpers.
- Viewer-first publish: `cmd /c ui_app\build_vsdevcmd.cmd` published `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`; the log tail is captured in `artifacts/logs/tone_map_finish_build_vsdevcmd.log`.
- Focused published-runtime proof: `py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_explaino_escape_variants.py -k tone_map_finish` passed with `1 passed, 24 deselected` against the published runtime.
- Broader native rerun note: the full `cmd /c ui_app\build_tests_vsdevcmd.cmd` sweep still ends at the pre-existing batch-script label miss `full_test_run` after the touched-owner rails compile and the targeted advanced-color tests pass. That harness issue is outside the bounded `tone_map_finish` slice and was not repaired here.

## Hostile Audit

- Status: complete
- Required posture: assume the first implementation would either expose `tone_map_finish` only in the editor, duplicate `neutral_finish` ordering instead of shipping real tone-map-last math, regress the already-shipped grading rows, or silently widen into `grade.glow` / `balance_void_grade` until repaired proof disproved it.

## Audit Passes

- [done] Pass 1 - the first RED proved the internal grading authority was still missing: `tone_map_finish` had no shipped enum/id/catalog surface, so the row could not truthfully exist beyond inventory.
- [done] Pass 2 - the first green attempt still left an owner gap in the extracted core: `ApplySupportedColorPipelineRowParamsToLive()` did not mirror `tone_map_finish` through the legacy runtime owners, which would have left editor/persistence truth ahead of the shared owner core.
- [done] Pass 3 - re-read the repaired state through focused native RED/owner rails, runtime publish, and the focused published-runtime `tone_map_finish` witness; no additional real defect found in the touched seams, and `grade.glow` / `balance_void_grade` remained untouched.

## Audit Findings

- [done] Real defect found: `tone_map_finish` initially had no shipped internal grading authority (`ColorGradingPreset`, enum ids, runtime-backed catalog membership, or live-bridge support), so the row was still inventory-only. The repaired state adds `tone_map_default` / `tone_map_finish` across the runtime-backed catalog, bridge, diagnostics/archive ids, and reset/import surfaces.
- [done] Real defect found: the first repaired state still skipped `tone_map_finish` in the extracted core owner apply path, which meant the live owner trio was not actually reusable outside the window bridge. `ui_app/src/color_pipeline_core.h` now imports and applies the shared exposure/saturation/contrast owner trio for `tone_map_finish` directly.
- [done] Clean re-audit evidence: `advanced_color_grading_red`, `advanced_color_grading_owner`, runtime publish, and the focused published-runtime pytest witness are green; out-of-scope rows remain deferred and no `grade.glow` / `balance_void_grade` code paths were widened.
- [done] Non-blocking residual note: the unrelated full-suite helper harness still ends at the pre-existing batch label miss `full_test_run`; this slice did not touch that workflow surface.

## Notes

- Expected owner files for this bounded slice:
  - `docs/contracts/advanced_color_library_foundation_phase8e_tone_map_finish_owner_proof.contract.json`
  - `docs/notes/advanced_color_feature_restart_inventory.md`
  - `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md`
  - `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`
  - `docs/notes/advanced_color_library_foundation_phase8e_tone_map_finish_owner_proof_PHASED_PLAN.md`
  - `ui_app/src/color_pipeline_core.h`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/fractal_types.h`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/src/diagnostics_capture.cpp`
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/src/runtime_reset.cpp`
  - `ui_app/src/fractal_family_rules.h`
  - `ui_app/src/enum_id_utils.h`
  - `ui_app/tests/test_color_pipeline_core.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
  - `ui_app/tests/test_escape_time_coloring.cpp`
  - `ui_app/tests/test_diagnostics_state_io.cpp`
  - `ui_app/tests/test_finding_archive_actions.cpp`
  - `ui_app/tests/test_runtime_reset.cpp`
  - `ui_app/tests/test_color_pipeline_window.cpp`
  - `tests/test_fractal_runtime_explaino_escape_variants.py`
- Non-goals for this bounded slice:
  - do not ship `grade.glow`
  - do not ship `balance_void_grade`
  - do not reopen Source weighted-blend or manual archive work
  - do not broaden into hooks, crash recovery, anti-lie tooling, or unrelated UI polish
  - do not claim editor-only, bridge-only, or persistence-only `tone_map_finish` as shipped

## Resume Point

Closed. Do not resume from this slice's old checkpoint chores. Re-enter later advanced-color work from `docs/notes/advanced_color_feature_restart_inventory.md`, `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`, and `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md` instead.

## Action Hostile Review

- Action ID: action-20260513-tone-map-finish-reds-1
- Suspected Failure Mode: the slice could expose `tone_map_finish` in the catalog or bridge without distinct runtime grading math, or it could widen into deferred `grade.glow` / `balance_void_grade` semantics under adjacent grading pressure.
- Correct Owner/Action: add REDs first across core/window/runtime/persistence/reset seams, then land only the bounded `tone_map_finish` owner path through the existing grading owner trio plus a dedicated tone-map-last runtime branch.
- Proof Surface: focused native helper tests for the touched owner seams, runtime publish, focused published-runtime proof, contract validation, phased-plan sync, and hostile-audit validation.
- Outcome: closed at checkpoint commit `a59e657` with machine proof receipts. REDs, owner-path repairs, diagnostics/archive/reset proof, runtime publish, and the focused published-runtime `tone_map_finish` witness are all green without widening into deferred grading rows.
- Blocked Action: the unrelated full-suite `full_test_run` batch-label miss remains outside this slice.
