# Advanced Color Library Foundation Phase 6 - Palette Blend Stack

## Current Phase

Complete - checkpoint commit `f9d26f3` and machine proof receipts closed the bounded Palette blend stack slice. This plan remains historical closure evidence only and must not be read as live pre-closeout restart authority.

## Phase Checklist

- [done] Phase 1 - add focused REDs proving Palette multi-row drafts cannot yet apply/import, evaluate RGB blends, persist/reset/capture, or publish as runtime-real state
- [done] Phase 2 - add bounded Palette stack runtime owners with explicit per-row palette id, palette params, blend weight, and blend mode while mirroring the active row back into the legacy flat tuple
- [done] Phase 3 - wire the reusable core/window live bridge and runtime color evaluator so shipped Palette rows sample RGB and blend in order without silent last-row collapse
- [done] Phase 4 - validate, hostile-audit, publish the viewer/runtime proof, write receipts, and checkpoint the slice cleanly

## Explicit User Asks

- [done] Continue moving forward on the advanced-color foundation after the serialization repair instead of drifting to unrelated backlog work.
- [done] Make the next composition slice real and runtime-backed, not descriptor-only or UI-only.
- [done] Keep the reusable color pipeline generic and separately owned while deferring unsupported generic Source composition.
- [done] Preserve the pit-of-success rule: visible rows must execute in runtime, persist, reset, capture, and reload truthfully.

## Presumption Loop

The controlling risk is Palette composition honesty. Shape now owns ordered runtime stacks, root-basin pairs own a bounded Source/Palette paired schedule, and Grading now owns an ordered stack for shipped grading rows. Palette still resolves through one live palette in the main runtime path unless a bounded stack owner and explicit blend math are added. The falsifiable hypothesis for this slice is narrow: shipped Palette rows can become a runtime-real ordered RGB stack if each row samples a concrete runtime palette from the current shaped source value and blends into an accumulator with explicit per-row weight/mode math. This does not claim generic Source generator composition, generic arbitrary palette chaining beyond the defined RGB blend operator, root_proximity pairing, or new Palette inventory rows.

The first runtime semantics should be boring and inspectable: row 0 seeds the accumulator from its sampled RGB color; later enabled rows blend their sampled RGB color over the accumulator using stored `palette.blend_weight` and `palette.blend_mode`. Unsupported rows, unknown modes, or mismatched row state must fail closed instead of collapsing to the last row. The legacy flat palette tuple remains a compatibility mirror of the final enabled valid row, while the new stack is the runtime authority when present.

## Presumption Evidence

- `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md` names Palette blend composition as the next critical foundation slice after the Grading stack and serialization repair.
- `docs/notes/advanced_color_library_foundation_phase6_grading_runtime_authority_PHASED_PLAN.md` records Palette blend stack as the next known sliced work, with Source signal composition and remaining Grading owner proofs deferred.
- `ui_app/src/fractal_types.h` already owns `color_shape_stack`, `color_root_basin_pairs`, and `color_grading_stack`, but no equivalent `color_palette_stack` owner exists yet.
- `ui_app/src/escape_time_coloring.h` already centralizes runtime color evaluation, so it is the correct seam for explicit RGB palette-row sampling and blend math.
- `ui_app/src/color_pipeline_core.h` and `ui_app/src/color_pipeline_window.h` already provide descriptor-driven row state and live bridge helpers that should be reused instead of adding window-local Palette authority.
- The capture-backed serialization repair proved diagnostics state save/load can catch flattened or truncated color-pipeline state, so this slice should extend those same rails for Palette stack persistence.

## Proof Ledger

- Checkpointed predecessor: Grading stack composition closed under `ck:679b2bc3`, proving ordered stack ownership, evaluation, persistence, reset, capture/archive, native validation, runtime publish, and published-runtime pytest proof for shipped Grading rows.
- Checkpointed predecessor: capture-backed diagnostics serialization repair closed under `ck:2f9efd8f`, proving save-path numeric fidelity and the manual `explaino_joy` color-pipeline draft/state round trip.
- RED/green: `ui_app/tests/test_schema_binding.cpp` now proves a two-row Palette draft imports/applies into `KernelParams::color_palette_stack`, preserves root-basin pair fail-closed behavior, exposes blend controls, and mirrors the final valid Palette row to the legacy flat tuple.
- RED/green: `ui_app/tests/test_escape_time_coloring.cpp` now proves two Palette rows render through explicit RGB blend math and do not collapse to final-row-only output.
- RED/green: `ui_app/tests/test_diagnostics_state_io.cpp` now loads `color_palette_stack` from diagnostics JSON, writes it through `CaptureDiagnosticsLastBundle`, asserts `blend_weight` and `blend_mode` in emitted `state.json`, and reloads the emitted save.
- RED/green: `ui_app/tests/test_runtime_reset.cpp` now seeds dirty Palette stack state and proves runtime reset clears it.
- Green rail: `artifacts/palette_blend_stack_native_green_after_phase_saturation_retry.log` ends with `All helper tests passed` after the audited phase-wheel saturation repair.
- Green rail: `artifacts/code_quality_report.json` records score `97/100` and baseline check passed after the final runtime-coloring changes.
- Green rail: `artifacts/palette_blend_stack_runtime_publish_final.log` publishes the active runtime to `D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe`.
- Green rail: `artifacts/palette_blend_stack_runtime_probe_session_pytest_final.log` records `64 passed` for the published-runtime probe/session lane.
- Green rail: `artifacts/validation/advanced_color_library_foundation_phase6_palette_blend_stack_contract.json`, `artifacts/validation/viewer_host_assert_phased_plan_sync.json`, and `artifacts/validation/advanced_color_library_foundation_phase6_palette_blend_stack_hostile_audit.json` record contract, plan-sync, and hostile-audit validation.

## Hostile Audit

- Status: complete
- Required posture: assume the first implementation is a fake stack until it proves real RGB blending, persisted per-row weight/mode state, no hidden last-row fallback, and viewer/runtime proof against the published executable.
- Result: the audit found and repaired real regressions in diagnostics draft compatibility, Palette blend renderability, flat root-basin Palette sync, the focused core missing-parameter regression test, runtime sampler size, and phase-wheel saturation runtime backing. The repaired state passed native, code-quality, runtime publish, and published-runtime proof rails.

## Audit Passes

- [done] Pass 1 - REDs found the missing runtime owner: multi-row Palette drafts had no bounded `color_palette_stack`, runtime evaluation collapsed to legacy flat Palette state, and diagnostics/reset had no Palette stack persistence surface.
- [done] Pass 2 - first repaired-state audit found a diagnostics compatibility defect: older saved Palette draft rows without the newly added `palette.blend_weight` and `palette.blend_mode` failed load on descriptor count mismatch. The loader now fills omitted blend params from descriptor defaults while still rejecting unknown, duplicate, excess, or non-blend missing params.
- [done] Pass 3 - second repaired-state audit found visible-control drift: Palette blend params existed in descriptors but `CollectRenderableColorPipelineParamIndexes()` hid them from the live row controls. The renderability map now includes `palette.blend_weight` and `palette.blend_mode` for shipped non-basin Palette stack rows.
- [done] Pass 4 - third repaired-state audit found a root-basin regression: the generic Palette stack import path rejected flat `root_index` / `root_classic_palette` live tuples outside the bounded pair schedule. The live snapshot path now preserves the root-basin Palette row import path separately from generic Palette stacks.
- [done] Pass 5 - fourth repaired-state audit found a stale core regression test: after blend controls were added, the missing-parameter assertion removed `palette.blend_mode` while leaving an invalid saturation value, so it no longer proved missing real params fail closed. The test now removes `palette.saturation` explicitly.
- [done] Pass 6 - clean re-read the repaired runtime owner, evaluator, diagnostics save/load, reset clearing, and schema/core test seams after `artifacts/palette_blend_stack_native_green_after_core_fix.log`; no additional real defect found in the touched Palette stack seams.
- [done] Pass 7 - code-quality audit found a max-function-lines regression in `SampleProgrammableEscapeTimePalette`; the runtime sampler was split into stack and row helpers, then native helper tests and code quality both passed cleanly.
- [done] Pass 8 - clean re-read after runtime publish and published-runtime probe/session proof found one additional Palette-param honesty bug: `phase_wheel_palette` persisted saturation but did not render it in the Palette row sampler and hid it from live row controls. The row sampler and renderability map now treat saturation as runtime-backed.
- [done] Pass 9 - clean re-read after the phase-wheel saturation repair, final native helper rail, code-quality audit, runtime publish, and published-runtime proof; no additional real defect found in the Palette stack runtime, persistence, or validation seams.

## Audit Findings

- [done] Real defect found: diagnostics draft restore rejected older Palette rows after new blend params changed descriptor counts. Repaired by allowing omitted `palette.blend_weight` and `palette.blend_mode` only, using descriptor defaults, while preserving fail-closed behavior for other missing paths.
- [done] Real defect found: Palette blend controls were descriptor-backed but hidden from the row renderability map. Repaired by marking blend weight and blend mode as live-renderable for shipped non-basin Palette stack rows.
- [done] Real defect found: flat root-basin Palette live snapshots regressed when the generic Palette stack importer took over all Palette rows. Repaired by preserving the root-basin Palette import branch before generic stack entry import.
- [done] Real defect found: the core missing-parameter test no longer removed a required runtime parameter after blend controls were appended. Repaired by deleting `palette.saturation` explicitly so the regression still proves missing real params fail closed.
- [done] Real defect found: the first runtime evaluator implementation pushed `SampleProgrammableEscapeTimePalette` past the code-quality max-function-lines baseline. Repaired by extracting bounded stack sampling and legacy row sampling helpers while preserving explicit RGB blend math.
- [done] Real defect found: `phase_wheel_palette` carried a persisted saturation param but the Palette row sampler ignored it and the renderability map hid it. Repaired by applying saturation in `SampleColorPipelinePaletteRowRgb`, keeping legacy phase-wheel saturation at its historical default, exposing the param as live-renderable, and adding native runtime/schema assertions.
- [done] Clean re-audit evidence: repaired-state proof is recorded in `artifacts/palette_blend_stack_native_green_after_phase_saturation_retry.log`, `artifacts/code_quality_report.json`, `artifacts/palette_blend_stack_runtime_publish_final.log`, `artifacts/palette_blend_stack_runtime_probe_session_pytest_final.log`, `artifacts/validation/advanced_color_library_foundation_phase6_palette_blend_stack_contract.json`, `artifacts/validation/viewer_host_assert_phased_plan_sync.json`, and `artifacts/validation/advanced_color_library_foundation_phase6_palette_blend_stack_hostile_audit.json`.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`
  - `docs/notes/advanced_color_library_foundation_phase6_palette_blend_stack_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_library_foundation_phase6_palette_blend_stack.contract.json`
  - `ui_app/src/color_pipeline_core.h`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/diagnostics_capture.cpp`
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/src/fractal_derived_fields.cpp`
  - `ui_app/src/fractal_types.h`
  - `ui_app/src/runtime_reset.cpp`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/tests/test_diagnostics_state_io.cpp`
  - `ui_app/tests/test_escape_time_coloring.cpp`
  - `ui_app/tests/test_finding_archive_actions.cpp`
  - `ui_app/tests/test_runtime_reset.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals for this slice:
  - do not implement generic Source signal composition
  - do not add new Palette inventory rows
  - do not widen `root_proximity` into Palette pairing
  - do not resume `grade.glow`, `neutral_finish`, `tone_map_finish`, basin lane-retention, or Balance/Void
  - do not claim UI-only or descriptor-only Palette composition

## Resume Point

Closed. Do not resume from this slice's old implementation checklist. Re-enter later advanced-color work from `docs/notes/advanced_color_feature_restart_inventory.md`, `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`, and `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md` instead.

## Action Hostile Review

- Action ID: action-20260512-palette-blend-stack-reds
- Suspected Failure Mode: the implementation may accidentally ship a descriptor-only Palette stack, silently collapse to the last row, or claim generic Palette chaining without explicit RGB blend math and persisted per-row weight/mode state.
- Correct Owner/Action: add focused REDs first, then implement bounded Palette stack runtime ownership, live bridge import/apply, explicit RGB blend evaluation, diagnostics/archive persistence, reset clearing, and published-runtime proof.
- Proof Surface: native helper tests, code-quality audit, runtime publish, published-runtime pytest/UI harness proof, contract validation, phased-plan sync, and hostile-audit validation.
- Outcome: done - action consumed by REDs, implementation, repaired hostile-audit findings, native helper proof, code-quality proof, runtime publish, and published-runtime proof.
- Blocked Action: editing Palette runtime/bridge code before this Action ID is recorded and consumed.
