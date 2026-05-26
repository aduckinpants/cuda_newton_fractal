# Color Pipeline Composition UI Cleanup

## Current Phase

Complete

## Phase Checklist

- [x] Phase 1 - open the checked-in plan/contract and lock the active slice
- [x] Phase 2 - add RED coverage for user-facing Color Pipeline composition wording leakage
- [x] Phase 3 - replace visible draft/live-bridge wording with authored composition workflow language
- [x] Phase 4 - run focused native/copy/runtime harness proof and preserve preset/SDF behavior
- [x] Phase 5 - hostile audit, plan sync, receipts, rearward review, commit, push, and clean tree

## Explicit User Asks

- [done] Continue from the pushed preset workflow branch into the next forward slice from the planned Color Pipeline list.
- [done] Depend on unit and smoke tests because the operator cannot review every turn in depth.
- [done] Reduce confusing implementation-state wording in the Color Pipeline workflow before adding more composition features.
- [done] Do not change the current Color Pipeline behavior, row stack authority, SDF source rows, capture/replay, or preset workflow semantics.

## Scope

In scope:

- Product-facing copy and affordance cleanup inside the existing Color Pipeline window.
- Regression tests that catch visible "Draft" / "live bridge" implementation wording in normal workflow surfaces.
- Preservation tests for recipe presets, source stack behavior, SDF source rows, and supported runtime apply paths.
- Roadmap truth sync if the active planning docs still list this cleanup as unstarted after it lands.

Out of scope:

- A new Factorio-style workflow layout.
- New Color Pipeline functions, rows, masks, gates, palettes, or SDF behavior.
- Boundary-masked SDF normal-angle.
- Generic Equation Pack viewport integration.
- Salticid runtime integration.
- Physical mouse automation.

## Proof Ledger

- Start authority: `master` was fast-forwarded to `96246b6`, pushed, and rearward review returned `ok`.
- Slice lock: `ck:335b53bc` opened on branch `codex/color-pipeline-composition-ui-cleanup`.
- RED coverage: `color_pipeline_composition_copy_red` failed on visible `Reset Draft From Live` wording before implementation.
- First GREEN: `color_pipeline_composition_copy_pytest` and `color_pipeline_composition_window` passed after replacing visible draft/live-bridge wording.
- Runtime harness proof: runtime published and `color_pipeline_composition_runtime` passed `tests/test_fractal_runtime_color_pipeline_presets.py` and `tests/test_fractal_runtime_color_pipeline_sdf_rows.py` with `5 passed, 0 skipped`.
- Hostile audit: found one real second-pass gap, visible `Live selection` / `reset-from-live` summary wording, then added it to the copy test and repaired it.
- Closure validators: contract validation, phased-plan sync, hostile-audit validation, code-quality baseline, and diff check passed.

## Hostile Audit

- Status: complete
- Required posture: assume the cleanup either hides an unsupported state, changes Color Pipeline runtime behavior, weakens fail-closed diagnostics, or leaves visible implementation wording uncovered until tests prove otherwise.

## Audit Passes

- [done] Pass 1 - reviewed changed user-facing strings and preserved internal type names/serialized state.
- [done] Pass 2 - reran copy/native tests and found the second visible wording gap before closeout.
- [done] Pass 3 - runtime harness proof shows preset and SDF row controls remain accessible without physical mouse automation.
- [done] Pass 4 - re-read the repaired state after the second copy fix and no additional real issue found in the changed UI copy, internal state names, or runtime proof surface.

## Audit Findings

- [done] Initial finding - existing copy test only blocked a few exact strings and did not catch visible "Reset Draft From Live", "(draft only)", or "live bridge tuple" wording.
- [done] Second-pass finding - visible summary copy still said "Live selection" and "reset-from-live"; the copy rail now blocks those fragments and the UI text says "Current color selection" / "reset from current color".
- [done] Clean preservation finding - internal `ColorPipelineDraft*`, `draft_import_supported`, and persisted `color_pipeline_draft` names remain unchanged for implementation and state compatibility.

## Notes

- Keep internal C++ type names such as `ColorPipelineDraft*` and persisted `color_pipeline_draft` state untouched in this slice.
- User-facing replacement model: "authored composition", "row editor", "current selection", "supported runtime selection", and explicit unsupported-combination explanations.
