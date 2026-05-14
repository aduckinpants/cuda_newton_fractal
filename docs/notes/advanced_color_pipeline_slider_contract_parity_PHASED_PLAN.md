# Advanced Color Pipeline Slider Contract Parity

## Current Phase

Complete - checkpoint commit `8f2b39e` and machine proof receipts closed the slider-contract parity slice. This plan remains historical closure evidence only and must not be read as live pre-closeout restart authority.

## Phase Checklist

- [x] Phase 0 - prove the current color-pipeline control path still diverges from the working slider contract and lock the required parity behavior in focused coverage
- [x] Phase 1 - replace the custom color-pipeline numeric control contract with the working slider-plus-input, clamp-on-edit, post-lane live-apply pattern
- [x] Phase 2 - validate the parity fix on the focused seam, refresh published runtime proof, and close the slice cleanly

## Explicit User Asks

- [done] Stop reinventing slider behavior.
- [done] Make the shipped color-pipeline sliders behave like the other sliders in the application.
- [done] Restore live runtime feedback during drag instead of the current no-feedback behavior.
- [done] Fix it TDD-first with regression coverage before resuming broader color-pipeline productization.

## Presumption Loop

The controlling seam is `RenderColorPipelineParamControl(...)` plus the end-of-frame apply path in `RenderColorPipelineWindow(...)` inside `ui_app/src/color_pipeline_window.h`. The falsifiable local hypothesis is that the current parity failure is not another debounce constant; it is that the color-pipeline window still uses a separate draft-widget contract instead of the working control contract already implemented in `ui_app/src/schema_binding.cpp`. The cheapest discriminating check is a focused regression that proves auto-apply eligibility no longer depends on active-item throttling and that the current supported live-backed controls use the same clamp-on-edit plus per-frame live-apply behavior as the working slider surface.

## Presumption Evidence

- Operator proof: the working main-panel sliders behave correctly, while the color-pipeline sliders still do not.
- Code-path proof: `ui_app/src/schema_binding.cpp` uses a consistent slider-or-drag plus same-line numeric input, clamp-on-edit, and direct dirty/interacted propagation. `ui_app/src/color_pipeline_window.h` still uses a separate draft-only numeric widget path with a custom interaction gate.
- Runtime proof: the current color-pipeline auto-apply seam still depends on custom per-window timing/interaction logic instead of the same control contract as the rest of the application.

## Proof Ledger

- GREEN: `ui_app/tests/test_schema_binding.cpp` now proves supported live-backed color-pipeline drafts remain auto-applicable without the old active-item timing gate, and it adds focused clamp/range coverage for the new numeric-control helper path.
- GREEN: `ui_app/src/color_pipeline_window.h` now uses the same slider-plus-input and clamp-on-edit pattern as the working schema-binding control surface for float, double, and int parameters, while keeping the post-lane live apply ordering that avoids the old pre-pass focus drop.
- GREEN: `ui_app/src/main.cpp` now receives interaction-change signals from the color-pipeline window so viewer pacing sees those edits the same way it sees the main panel sliders.
- Validation: the focused native helper rail passed after the red regression went green, `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json` passed with `97/100`, `cmd /c ui_app\build_vsdevcmd.cmd` republished the runtime, staged `fractal_ui.exe --validate-ui` passed, and `py -3.14 tools/viewer_host_runtime_pytest_lane.py` passed with `68 passed`.
- Audit: distrust-first review did not find another local behavioral defect after removing the active-item timing gate and moving the color-pipeline controls onto the same numeric-control contract as the working panel. The dead debounce timing state was removed in the final cleanup pass so the landed code matches the new model.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_pipeline_slider_contract_parity_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_pipeline_slider_contract_parity.contract.json`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/schema_binding.h`
  - `ui_app/src/main.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
- Optional owner files if needed for shared metadata:
  - `ui_app/src/function_descriptor.h`
- Non-goals:
  - do not revert the auto-apply checkbox back to a button
  - do not keep iterating on custom debounce timing as the primary fix
  - do not widen additional Source / Shape / Palette catalog rows in this slice

## Resume Point

Closed. Do not resume from this slice's old checkpoint chores. Re-enter later advanced-color work from `docs/notes/advanced_color_feature_restart_inventory.md`, `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`, and `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md` instead.
