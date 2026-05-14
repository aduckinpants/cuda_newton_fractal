# Advanced Color Pipeline Active Drag And Palette Grading Preserve

## Current Phase

Phase 3 - validation and checkpoint closure on the repaired active-drag and supported-palette-preserve seams.

## Phase Checklist

- [x] Phase 1 - add focused REDs that prove active slider edits currently apply and resync during manipulation, and prove source or palette tuple changes reset a still-supported grading row
- [x] Phase 2 - defer live apply and resync until active slider interaction completes, and preserve the current grading row and values across supported source-palette switches unless the new tuple truly disallows that grading
- [ ] Phase 3 - rerun the narrow owner and runtime lanes, hostile-audit the repaired state, checkpoint, and write machine proof receipts

## Explicit User Asks

- [done] Recreate the regression where sliding any function parameter makes the rest of the function controls flicker or jump.
- [done] Lock the slider regression down in the existing UX or window harness instead of trusting manual behavior.
- [done] Recreate the regression where changing Palette resets a grading row even when that grading remains supported for the new Source and Palette tuple.
- [done] Lock the supported-grading-preserve regression down in a runtime-backed harness.
- [done] Keep balance_void_grade feature work itself out of scope here; the user reports the new function path appears to work.
- [done] Do not reopen weighted blend, basin-default, neutral_finish, tone_map_finish, grade.glow, manual archive, historical `234919_563__explaino_inertial`, hooks, crash recovery, anti-lie tooling, or unrelated UI polish.

## Proof Ledger

- Bootstrap already proved this repo is on `feature/advanced-color-pipeline-draft-editor-reframe` at clean `HEAD=bdef6ce2caabf1acb5f35cf9f070e3e03667d271`, and the previous active contract `advanced_color_library_foundation_phase8g_balance_void_grade_owner_proof` is closed continuity that cannot authorize new mutation.
- Minimal preflight opened and re-locked this slice under `advanced_color_pipeline_active_drag_and_palette_grading_preserve` so the regressions could be repaired without mutating under the closed balance_void_grade contract.
- Palette-preserve RED was reproduced in the published-runtime seam with `py -3.14 -m pytest tests/test_fractal_runtime_explaino_escape_variants.py -k palette_switch_preserves_supported_balance_void_grade -q`, which previously failed because the runtime grading fell back to `escape_default` instead of preserving `balance_void_default` on a supported palette switch.
- Active-drag RED was first reproduced at the direct helper seam after adding the initial regression coverage; later hostile audit found that helper-only coverage still missed the numeric input ordering path. After tightening `ui_app/tests/test_schema_binding.cpp` to recreate the ordering bug, `cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_red` failed with `Expected active advanced color numeric edits to stay draft-only until release instead of applying before the control reports its active state`.
- The repair now lives in the actual owner seams: `ui_app/src/color_pipeline_window.h` preserves a still-supported grading row across source/palette tuple switches, blocks direct apply while a control remains active, and centralizes numeric-control commit ordering so active-item state is recorded before live apply is attempted.
- Window-harness proof now lives in `ui_app/tests/test_color_pipeline_window.cpp` for supported palette-switch grading preservation and in `ui_app/tests/test_schema_binding.cpp` for numeric-control active-drag ordering. Published-runtime proof lives in `tests/test_fractal_runtime_explaino_escape_variants.py`.
- Green proof on the repaired state so far:
  - `cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_red`
  - `cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner`
  - `cmd /c ui_app\build_vsdevcmd.cmd`
  - `py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_explaino_escape_variants.py -k palette_switch_preserves_supported_balance_void_grade`
- The owner lane continues to cover the existing shipped grading rows `contrast_lift`, `phase_finish`, `band_finish`, `basin_default`, `neutral_finish`, `tone_map_finish`, and `grade_glow` while this slice stays bounded to the two UX regressions.
- Historical archive compatibility remains excluded because this slice only repairs current live-bridge draft behavior and current diagnostics/runtime persistence for a shipped grading operator; the accepted `234919_563__explaino_inertial` incompatibility remains a non-goal here, and ExplainO-BalanceVoid or ExplainO-all family work remains excluded because no ExplainO family operator expansion is needed to preserve a generic grading row across a supported palette switch.

## Hostile Audit

- Status: complete
- Required posture: assume the first repair still left at least one unproven active-control path or preserved grading only in the draft until the direct control seam, the window harness, the owner lane, and the published runtime all agree on the repaired behavior.

## Audit Passes

- [x] Pass 1 - re-read the active-drag apply path and grading co-switch path after landing REDs; this exposed a real ordering bug in the numeric input seam instead of a clean closure state.
- [x] Pass 2 - clean re-read of the repaired owner and runtime lanes found no additional real defect while `advanced_color_grading_red`, `advanced_color_grading_owner`, and the focused published-runtime palette witness stayed green.
- [x] Pass 3 - clean re-read of the final diff, plan text, validator outputs, and current proof outputs found no additional workflow mistake or unsupported closeout claim beyond the remaining checkpoint and receipt work.

## Audit Findings

- [x] Finding 1 - the first active-drag repair only proved `TryApplySupportedColorPipelineDraftFromControl(...)` when `has_active_item` was already seeded. It did not prove that the numeric input path records `has_active_item` before attempting live apply. The strengthened schema-binding regression exposed the real ordering bug, and the fix now routes float, double, and int controls through `CommitColorPipelineNumericParamEdit(...)` so active-item state is recorded before live apply is attempted.
- [x] Clean re-read - after the ordering repair, no additional real defect found in the supported palette-switch grading path; the window harness, owner lane, and published runtime witness still preserve the `balance_void_grade` row, `balance_void_default`, and the owner values.
