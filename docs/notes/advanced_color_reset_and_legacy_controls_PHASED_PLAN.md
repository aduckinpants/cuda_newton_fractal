# Advanced Color Reset And Legacy Controls Issues Pass

## Current Phase

Phase 3 in progress - the reset/default and legacy-control ownership repairs are landed and validated; update continuity surfaces and checkpoint this detour before returning to the planned palette work

## Phase Checklist

- [x] Phase 1 - add focused regressions for Reset All base-color defaults and legacy control ownership while the advanced window is open
- [x] Phase 2 - land the minimal runtime/UI repair without widening the planned palette/grading work
- [ ] Phase 3 - run the bounded validation rails, checkpoint the bug-fix detour, and hand back to the planned Phase 4 palette work

## Explicit User Asks

- [open] Checkpoint the current explaino slice first instead of blending more bug fixes into an uncheckpointed worktree.
- [open] Run the step-2 issues pass before resuming the planned palette slices.
- [open] `Reset All` must neutralize the Color-panel tint plus saturation/contrast controls again.
- [open] The two simple Color-panel dropdowns must stop fighting the advanced color window.
- [open] Add tests strong enough that the agent can verify the fix instead of trusting manual spot checks.

## Presumption Loop

The bounded defect is twofold: the reset authority does not currently restore the base Color-panel owner fields, and the simple Color-panel dropdowns remain live while the advanced color window is open, so the legacy path can still stomp the runtime tuple the advanced editor is trying to own. The cheapest disconfirming checks are local and deterministic: `ApplyCommonPresetDefaults(...)` should either already restore `color_saturation`, `color_contrast`, and the tint triplet or it does not, and the Color-panel render path should either disable those legacy combo controls while the advanced window is open or it does not.

## Presumption Evidence

- `ui_app/src/fractal_derived_fields.cpp` currently resets many programmable color owners in `ApplyCommonPresetDefaults(...)` but does not touch `color_saturation`, `color_contrast`, `color_tint_r`, `color_tint_g`, or `color_tint_b`.
- `ui_app/tests/test_runtime_reset.cpp` currently proves the widened programmable owner defaults but does not assert the base Color-panel saturation/contrast/tint reset values.
- `ui/fractal_binding_surface_v1.ui_schema.json` exposes two simple Color-panel combos: `fractal.params.coloring_mode` and `fractal.params.color_grading`.
- `ui_app/src/schema_binding.cpp` still routes those simple combo edits directly through the legacy mirrored pipeline path, so they remain authoritative when rendered.
- `ui_app/src/main.cpp` currently renders those simple controls unconditionally and only adds the `Color Pipeline...` button beside the `coloring_mode` combo, which leaves both surfaces live at once.

## Proof Ledger

- Landed: `ui_app/tests/test_runtime_reset.cpp` now proves that `Reset All` restores `color_saturation`, `color_contrast`, and the base tint triplet alongside the programmable color owner defaults.
- Landed: `ui_app/tests/test_schema_binding.cpp` now proves that the advanced color window owns only the simple `Coloring Mode` and `Grading` controls while open, leaving the non-conflicting Color-panel sliders available.
- Landed: `ui_app/src/fractal_derived_fields.cpp`, `ui_app/src/color_pipeline_window.h`, and `ui_app/src/main.cpp` now restore the base Color-panel defaults through the shared preset authority and disable the conflicting simple combos while the advanced color window is the active editing surface.
- Validated: `artifacts/code_quality_report.json` stayed on the `97/100` baseline, `artifacts/verify_native_helper_tests.log` is green, `artifacts/verify_runtime_publish.log` republished the runtime cleanly, `artifacts/verify_runtime_probe_session_pytest.log` reports `68 passed`, `artifacts/validation/advanced_color_reset_and_legacy_controls_contract.json` validates the detour contract, and `artifacts/validation/viewer_host_assert_phased_plan_sync.json` stayed green.
- Audit: hostile review did not find a second defect after checking whether the reset fix belonged in the shared preset authority and whether the legacy-control disablement was scoped narrowly enough to leave the non-conflicting Color-panel sliders live.

## Notes

- Expected owner files for this pass:
  - `KNOWN_ISSUES.md`
  - `docs/contracts/advanced_color_reset_and_legacy_controls.contract.json`
  - `docs/notes/advanced_color_reset_and_legacy_controls_PHASED_PLAN.md`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/fractal_derived_fields.cpp`
  - `ui_app/src/main.cpp`
  - `ui_app/src/runtime_reset.cpp`
  - `ui_app/src/schema_binding.cpp`
  - `ui_app/tests/test_runtime_reset.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals for this pass:
  - do not resume the family-gated root palette work yet
  - do not redesign the whole Color panel or remove the simple controls entirely
  - do not widen Grading or introduce new advanced-color functions
  - do not treat unrelated viewer/runtime bugs as part of this detour unless the new regressions prove they share the same control seam

## Resume Point

Close this bug-fix detour first. After the reset/default seam and the simple-control ownership seam are both proven and checkpointed, resume the planned Phase 4 root-palette widening in `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`.