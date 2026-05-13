# Advanced Color Library Foundation Phase 8 - Source Weighted-Blend Stack

## Current Phase

Phase 4 complete. Generic Source composition for shipped non-pair Source rows now lands as a bounded `color_source_stack` with weighted blend semantics, live bridge import/apply, diagnostics/archive persistence, reset/default clearing, legacy final-row mirroring, and runtime evaluation before Shape -> Palette -> Grading. `root_index` remains pair-only on the separate bounded root-basin schedule, and this slice stayed out of Balance/Void, ExplainO-all, archive UX/recovery, hook work, and crash-recovery churn.

## Phase Checklist

- [done] Phase 1 - add focused REDs proving generic Source still collapses to one row through live bridge, runtime evaluation, diagnostics/archive persistence, and reset/default seams, while `root_index` remains pair-only
- [done] Phase 2 - add bounded `color_source_stack` runtime ownership with per-row Source id, params, and `signal.blend_weight` while mirroring the final valid generic Source row back into the legacy flat fields
- [done] Phase 3 - wire Source live snapshot import, draft apply, diagnostics save/load, capture/archive persistence, and reset/default handling to the new owner without widening Palette or Grading scope
- [done] Phase 4 - validate, hostile-audit, runtime-publish, write receipts, and checkpoint the slice cleanly

## Explicit User Asks

- [done] Ship generic Source composition via weighted blend as the highest-priority core feature lane for the active sprint.
- [done] Keep `root_index` pair-only in the bounded root-basin family; do not fold it into the generic Source stack.
- [done] Make the slice runtime-real end to end: weighted blend math, live bridge, serialization, archive persistence, reset/defaults, and proof must all exist together.
- [done] Keep Balance/Void, ExplainO-BalanceVoid, ExplainO-all, archive UX/recovery, hooks, workflow guards, and crash-recovery work out of this slice.
- [done] Preserve the stage order: Source composition first, then Shape, then Palette, then Grading.

## Presumption Loop

The controlling risk is Source-composition honesty. Shape, Palette, and Grading already own bounded stacks, and root-basin pairs already own a separate bounded Source/Palette schedule. Generic Source still routes through one flat `color_pipeline.signal` plus legacy per-signal fields. The falsifiable hypothesis for this slice is narrow: shipped non-pair Source rows can become a runtime-real bounded stack if each enabled row emits an independent scalar from the same fractal sample, row 0 seeds the composed scalar, later rows update `composed = lerp(composed, row_value, clamp(signal.blend_weight))`, the composed scalar still flows through the existing Shape -> Palette -> Grading order, and the final valid generic Source row mirrors into the flat legacy Source fields for compatibility. If a two-row generic Source draft still rejects or collapses to one row anywhere after that owner lands, the slice is still lying.

## Presumption Evidence

- `ui_app/src/fractal_types.h` owns `color_shape_stack`, `color_root_basin_pairs`, `color_palette_stack`, and `color_grading_stack`, but no equivalent `color_source_stack` runtime owner exists.
- `ui_app/src/color_pipeline_window.h` still rejects `sourceRows.size() != 1` unless every enabled Source / Palette row participates in the bounded `root_index` root-basin pair family, and `TryBuildColorPipelineLiveSnapshot(...)` still imports generic Source through `BuildColorPipelineLaneWithSingleRow(...)` plus `rows.front()`.
- `ui_app/src/color_pipeline_core.h` still imports Source params only into one row from the flat legacy fields, and `ApplySupportedColorPipelineParamsToLive(...)` currently updates Shape, Palette, and Grading stacks without any generic Source stack branch.
- `ui_app/src/escape_time_coloring.h` still resolves one programmable Source scalar from `params.color_pipeline.signal`, then applies Shape on that single scalar before Palette and Grading.
- `ui_app/src/diagnostics_state_io.cpp`, `ui_app/src/diagnostics_capture.cpp`, and `ui_app/src/runtime_reset.cpp` already persist/reset Shape, Palette, and Grading stacks plus root-basin pairs, but they still lack a generic Source stack surface.
- `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_escape_time_coloring.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp`, `ui_app/tests/test_finding_archive_actions.cpp`, and `ui_app/tests/test_runtime_reset.cpp` already prove the bounded multi-row owners for the other lanes, so this slice can follow the same test-first seams instead of inventing new authority.

## Proof Ledger

- Checkpointed predecessor: `ck:679b2bc3` proved ordered Grading stack composition with runtime ownership, live bridge import/apply, diagnostics/archive persistence, reset clearing, and runtime proof.
- Checkpointed predecessor: `ck:44faf037` proved explicit Palette RGB blend stack ownership, persistence, reset clearing, and runtime proof without overclaiming generic Source composition.
- Checkpointed predecessor: `ck:presetpreserve1` proved descriptor-driven row switches preserve same-path/type authored values and keep automatic tuple promotion honest, which is the same row-state seam this slice will reuse.
- Authority opened: `docs/notes/advanced_color_library_foundation_phase8_source_weighted_blend_stack_PHASED_PLAN.md` plus `docs/contracts/advanced_color_library_foundation_phase8_source_weighted_blend_stack.contract.json` now bind the first executable Phase 8 Source-composition slice.
- Validation evidence: `artifacts/code_quality_report.json` passed at 97/100 after the final diagnostics helper refactor.
- Validation evidence: `artifacts/verify_native_helper_tests.log` reran the full native helper sweep green after the final Phase 8 edits.
- Validation evidence: `artifacts/verify_runtime_publish.log` republished the active runtime to `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe` on the final Phase 8 state.
- Validation evidence: `artifacts/source_weighted_blend_published_runtime_proof.log` ran focused published-runtime pytest against `tests/test_fractal_runtime_explaino_escape_variants.py` and passed 2 selected runtime witnesses.

## Hostile Audit

- Status: complete
- Required posture: assume the first implementation will lie by stopping at editor-only multi-row drafts, silently collapsing runtime evaluation back to one Source row, or sneaking `root_index` into generic Source composition until repaired proof says otherwise.

## Audit Passes

- [done] Pass 1 - re-read the live Source owner seams and record the controlling defects: no `color_source_stack`, single-row generic Source import/apply, and one-signal runtime evaluation.
- [done] Pass 2 - re-read the umbrella foundation authority and repair the stale next-slice owner list so it points at Phase 8 Source weighted blend instead of the old Phase 6 Grading contract.
- [done] Pass 3 - first repaired-state audit found a live-bridge/root-basin defect: generic Source mirroring could stomp root-basin candidate comparisons and mismatched pair drafts missed the dedicated row-count validator; repaired the root-basin routing and reran the native helper sweep.
- [done] Pass 4 - second repaired-state audit found the runtime honesty gap: two-row Source stacks still collapsed to the final row and the new basin stack path needed a forward declaration for `ResolveBasinResidualMetric`; repaired sequential weighted blend evaluation and the declaration-order defect, then reran the full native helper sweep.
- [done] Pass 5 - third repaired-state audit found a code-quality regression in the new diagnostics Source-stack save/load helpers; extracted bounded helpers in diagnostics capture/load, then reran code quality plus the focused diagnostics/archive helper tests.
- [done] Pass 6 - clean re-read the repaired Source stack owner, live bridge, diagnostics persistence, reset/default handling, full native helper sweep, runtime publish, and focused published-runtime proof; no additional real defect found in the bounded Source stack seams.

## Audit Findings

- [done] Real product defect found: generic Source still has no bounded runtime owner, and the live/runtime/persistence seams still treat generic Source as one flat tuple outside the bounded root-basin pair schedule.
- [done] Real continuity defect found: the umbrella foundation plan still listed the old Phase 6 Grading plan/contract as the expected owner files for the next slice even though the next real lane is Phase 8 Source weighted blend.
- [done] Real repaired-state defect found: the first live-bridge implementation still let generic Source mirroring interfere with root-basin candidate comparisons and routed mismatched root-basin pair drafts through the wrong generic-Source rejection path; the live bridge now preserves the bounded root-basin schedule separately and fails mismatched pairs with the dedicated row-count error.
- [done] Real runtime defect found: the first Source-stack runtime path still collapsed multi-row Source composition to the final row and failed to compile cleanly for basin evaluation because `ResolveBasinResidualMetric` was referenced before declaration; the runtime now evaluates bounded Source rows sequentially with explicit weighted blend math and the basin helper declaration order is fixed.
- [done] Real code-quality defect found: the first diagnostics Source-stack persistence implementation pushed helper size past the repo baseline; bounded helper extraction restored the code-quality baseline without changing behavior.
- [done] Clean re-audit evidence: after `artifacts/code_quality_report.json`, `artifacts/verify_native_helper_tests.log`, `artifacts/verify_runtime_publish.log`, and `artifacts/source_weighted_blend_published_runtime_proof.log`, no additional real defect or workflow mistake was found in the Phase 8 Source weighted-blend seams.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`
  - `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md`
  - `docs/notes/advanced_color_library_foundation_phase8_source_weighted_blend_stack_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_library_foundation_phase8_source_weighted_blend_stack.contract.json`
  - `ui_app/src/color_pipeline_core.h`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/diagnostics_capture.cpp`
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/src/fractal_derived_fields.cpp`
  - `ui_app/src/fractal_types.h`
  - `ui_app/src/runtime_reset.cpp`
  - `ui_app/tests/test_color_pipeline_core.cpp`
  - `ui_app/tests/test_diagnostics_capture.cpp`
  - `ui_app/tests/test_diagnostics_state_io.cpp`
  - `ui_app/tests/test_escape_time_coloring.cpp`
  - `ui_app/tests/test_finding_archive_actions.cpp`
  - `ui_app/tests/test_runtime_reset.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals for this slice:
  - do not redefine the mapped Source / Shape / Palette / Grading inventory
  - do not put `root_index` into generic Source composition
  - do not widen remaining Grading owner proofs, `balance_void_grade`, ExplainO-BalanceVoid, ExplainO-all, archive UX/recovery, or workflow tooling in this slice
  - do not claim editor-only or serialization-only Source composition as shipped
  - do not create a second Source authority outside the runtime stack plus legacy compatibility mirror

## Resume Point

Checkpoint this validated Phase 8 slice through `tools/viewer_host_checkpoint_slice.py commit`, then write validation plus contract-proof receipts for the clean committed head. After closure, the next advanced-color lane should resume from the umbrella foundation authority rather than reopening this bounded Source weighted-blend slice.

## Action Hostile Review

- Action ID: action-20260513-source-weighted-blend-reds
- Suspected Failure Mode: the implementation may land only editor-side multi-row Source draft behavior, silently collapse runtime evaluation back to one Source row, or accidentally fold `root_index` into the generic Source stack.
- Correct Owner/Action: add focused REDs first, then implement bounded generic Source stack ownership, weighted blend runtime evaluation, live bridge import/apply, diagnostics/archive persistence, reset/default handling, and explicit `root_index` exclusion while preserving the bounded root-basin pair schedule.
- Proof Surface: native helper tests, code-quality audit, runtime publish, published-runtime proof, contract validation, phased-plan sync, and hostile-audit validation.
- Outcome: complete - the bounded generic Source stack, weighted blend runtime path, persistence/reset surfaces, repaired hostile-audit findings, and viewer-first proof are now all landed.
- Blocked Action: none.