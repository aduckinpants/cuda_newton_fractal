# Root Field Preset Dropdown Repair Phased Plan

## Explicit User Asks

- Replace the massive linear Color Pipeline preset button row with a compact dropdown or equivalent selector.
- Keep the newly shipped root-field preset recipes public and usable.
- Do not remove the recipe functionality or broaden into a graph UI redesign.
- Prove no-mouse preset automation still works after the UI change.

## Current Phase

Closed - dropdown repair implemented, validated, hostile-audited, and entering checkpoint closeout.

## Phase Checklist

- [x] Phase 0: Start from clean pushed `codex/root-field-experiment-preset-pack-v1` head and rearward-review it.
- [x] Phase 0: Create `codex/root-field-preset-dropdown-repair`.
- [x] Phase 0: Create and lock this repair plan/contract.
- [x] Phase 1: Inspect current Color Pipeline preset rendering and add/update RED coverage for bounded preset UI.
- [x] Phase 2: Replace the linear preset button row with a compact selector/apply control while preserving stable no-mouse IDs for preset application.
- [x] Phase 3: Run focused native/window and no-mouse runtime proof.
- [x] Phase 4: Hostile audit, checkpoint, receipts, rearward review, push, and clean-tree closeout.

## Scope Lock

In scope:

- Color Pipeline preset selector UI around existing preset recipes.
- Focused Color Pipeline window/runtime tests that prove the UI no longer grows as a horizontal button strip.
- Existing root-field preset recipe buttons may become selector entries, but recipe IDs and automation application semantics should remain stable where possible.

Out of scope:

- New Color Pipeline graph UI.
- New preset content, new fractal lanes, new SDF ops, or backend substrate work.
- Reworking recipe materialization beyond what is needed for selector presentation.
- Physical mouse automation.

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - Found the user-visible defect: `RenderColorPipelineRecipePresetControls` rendered every recipe as a same-line button, so the preset pack scaled into a long horizontal strip.
- [x] Pass 2 - Found stale runtime proof after the UI repair: `test_root_field_experiment_preset_pack_v1_no_mouse` still waited for old visible per-recipe buttons. Repaired it to use the compact selector/apply flow.
- [x] Pass 3 - Clean re-read found no additional real defect in the repaired preset selector state; individual recipe apply IDs remain virtual automation commands, not visible button controls.

## Audit Findings

- [x] Finding 1: The Color Pipeline recipe UI exposed each preset as a separate same-line button, creating an unbounded horizontal strip as recipe count grows. Fixed by replacing the strip with one `Recipe` combo and one `Apply Recipe` button.
- [x] Finding 2: One runtime proof still encoded the old visual button model. Fixed by updating the root-field preset-pack proof to wait for `color_pipeline.recipe.selector` and `color_pipeline.recipe.apply_selected`, then select recipes through scoped no-mouse commands before applying.

## Proof Ledger

| Item | Status | Evidence |
| --- | --- | --- |
| Source head rearward review | done | `py -3.14 tools/viewer_host_rearward_review.py` returned `ok` for `3797041`. |
| Branch | done | `codex/root-field-preset-dropdown-repair`. |
| Plan/contract | done | This plan and `docs/contracts/root_field_preset_dropdown_repair.contract.json`; contract validation passed. |
| Native/window proof | done | `ui_app\build_tests_vsdevcmd.cmd test_color_pipeline_window test_color_pipeline_core > artifacts/root_field_preset_dropdown_repair/native.log 2>&1` passed. |
| Runtime publish | done | `ui_app\build_vsdevcmd.cmd > artifacts/root_field_preset_dropdown_repair/runtime_publish.log 2>&1` passed. |
| Runtime proof | done | `py -3.14 -m pytest tests/test_fractal_runtime_color_pipeline_presets.py tests/test_fractal_runtime_root_field_consumers.py::test_root_field_experiment_preset_pack_v1_no_mouse -q --junitxml artifacts/pytest/root_field_preset_dropdown_repair_runtime.junit.xml` passed with 4 tests. |
| Hostile audit | done | Real UI and stale-proof findings repaired; `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/root_field_preset_dropdown_repair_PHASED_PLAN.md --out-json artifacts/validation/root_field_preset_dropdown_repair_hostile_audit.json` passed. |
| Checkpoint/receipts/rearward/push | closeout step | Required immediately after this plan sync before final user closeout. |

## Stop Point

Stop after the preset selector UI is compact, validated, audited, checkpointed, pushed, and clean. Preplanned sliced work for this repair is exhausted at that point; stop for replan before graph UI or further Color Pipeline expansion.
