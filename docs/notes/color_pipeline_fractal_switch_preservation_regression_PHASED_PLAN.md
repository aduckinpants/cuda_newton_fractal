# Color Pipeline Fractal Switch Preservation Regression

## Current Phase

Phase 5 complete - compatible fractal switches preserve supported Color Pipeline state, unsupported switches project to target defaults, and focused native/runtime proof is green.

## Phase Checklist

- [x] Phase 1 - create this checked-in plan/contract and open the regression slice
- [x] Phase 2 - add RED native/no-mouse proof that switching to a compatible fractal preserves Color Pipeline live state and authored draft edits
- [x] Phase 3 - repair fractal-switch derived defaults so supported Color Pipeline selections and source-stack params are not reset
- [x] Phase 4 - preserve deterministic fallback only when the selected pipeline is unsupported by the new fractal
- [x] Phase 5 - hostile audit, focused native/runtime rails, receipts, rearward review, push, and clean tree

## Explicit User Asks

- [done] Treat Color Pipeline reset on fractal switch as a regression before returning to forward SDF work.
- [done] Do not reset tailored Color Pipeline form/live state when the newly selected fractal still supports the active pipeline.
- [done] If a fractal does not support the active pipeline, project safely instead of silently destroying authored draft intent.
- [done] Keep the earlier preset/composition UI miss documented and deferred; do not widen this bug slice into a full Color Pipeline redesign.

## Scope

In scope:

- Fractal-type switch and derived-default code paths that currently reset `KernelParams::color_pipeline`, source-stack params, or Color Pipeline draft state.
- Native schema/window/state tests that prove supported Color Pipeline state survives a compatible fractal switch.
- No-mouse runtime proof that a customized Color Pipeline remains active after switching fractals through the normal selector path.
- Documentation note that the broader chain-oriented/preset workflow remains a later design slice.

Out of scope:

- SDF-native fractal lanes.
- Authored SDF pack UI/live viewport integration.
- Full Color Pipeline preset manager or Factorio-style composition UI.
- New color functions, palettes, or shape semantics.
- Physical mouse automation.

## Proof Ledger

- Start authority: branch `codex/color-pipeline-sdf-source-rows` at `608f4b3` is clean and rearward review returned `status=ok`.
- Prior design authority: `docs/notes/advanced_color_preset_pit_of_success_PHASED_PLAN.md` already records that changing supported functions must not blow away slider customizations and that authored draft intent should be preserved separately from live-supported projection.
- RED proof: `color_pipeline_fractal_switch_test_derived_fields_red` failed with `Compatible fractal switch should preserve Color Pipeline live/source-stack params`.
- Repaired seam: normal main-loop fractal selector changes now call `ApplyFractalPresetDefaultsForFractalSwitch`, which applies target fractal numeric defaults while restoring the prior Color Pipeline runtime snapshot only when the target fractal supports the top-level pipeline, source-stack signals, and root-basin pairs.
- Repaired second defect found during hostile review: `ApplyCommonPresetDefaults` now clears `color_source_stack_count` and source entries, so unsupported/default projection cannot retain stale Source rows.
- Native proof passed: `color_pipeline_fractal_switch_test_derived_fields`, `color_pipeline_fractal_switch_test_window`, and `color_pipeline_fractal_switch_test_schema_binding`.
- Runtime publish passed: `color_pipeline_fractal_switch_runtime_publish`.
- Published no-mouse runtime proof passed: `color_pipeline_fractal_switch_runtime_pytest` reported `5 passed`, including the new persistent selector-switch proof and existing SDF Source row preservation proof.
- Code-quality baseline passed with score `94/100`.
- Roadmap truth sync updated `spec_intake/_STATUS.md`, `DEFERRED_THREADS.md`, and `docs/notes/sdf_field_pack_near_term_TODO.md` so the repaired regression is no longer listed as the active blocker before forward SDF work.
- Contract validation, phased-plan sync, hostile-audit validation, diff hygiene, rearward review, receipts, push, and clean tree are required before final closeout.

## Hostile Audit

- Status: complete
- Required posture: assume the first fix only preserves top-level mode while still losing source-stack params, draft row params, unsupported-fractal fallback truth, or runtime selector behavior until focused native and no-mouse proof disproves each risk.

## Audit Passes

- [done] Pass 1 - re-read fractal-switch/default-derived field paths and identify every place Color Pipeline state can be reset.
- [done] Pass 2 - re-read Color Pipeline draft/live bridge paths to ensure authored draft params are preserved when live projection remains supported.
- [done] Pass 3 - re-read runtime no-mouse proof outputs to ensure the normal selector path is tested, not only helper/state injection.
- [done] Pass 4 - clean re-read the repaired state and no additional real issue found in the touched selector/default/window/runtime seams.

## Audit Findings

- [done] Real product regression repaired: the normal main-loop fractal selector path reapplied full fractal defaults and reset a compatible user-tailored Color Pipeline.
- [done] Real default-reset bug repaired: `ApplyCommonPresetDefaults` did not clear stale `color_source_stack` rows, so unsupported/default projection could leave a contradictory Source stack behind.
- [done] Real proof gap repaired: native tests now cover compatible preservation, unsupported fallback, and authored draft preservation; runtime proof now exercises the normal selector path through in-process no-mouse enum automation.
- [done] Clean re-read confirmed the repaired state: full preset manager, Factorio-style composition UI, new color-function library work, SDF-native lanes, authored SDF live integration, and physical mouse automation remain out of scope.

## Action Hostile Review

- Action ID: color-pipeline-fractal-switch-preservation-closeout
- Suspected failure mode: switching fractals reapplies defaults that overwrite a user-tailored Color Pipeline even when the new fractal supports the same pipeline.
- Correct owner/action: repair the derived-default and binding/window seams that control normal fractal selector transitions without changing unrelated Color Pipeline composition behavior.
- Proof surface: focused native Color Pipeline/schema/window tests, runtime publish, no-mouse published-runtime selector-switch proof, SDF/Color Pipeline preservation rails, contract validation, phased-plan sync, hostile-audit validation, code-quality baseline, diff hygiene, rearward review, and clean tree.
- Blocked action: full preset manager, Factorio-style chain UI, new color-function library expansion, SDF-native lanes, authored SDF pack live integration, or physical mouse automation.
