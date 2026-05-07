# Advanced Color Pipeline Finding Capture Draft State

## Current Phase

Phase 3 - validated and audited; checkpoint closure remains pending explicit commit/handoff work

## Phase Checklist

- [x] Phase 0 - add focused RED coverage proving Capture Finding and state load do not yet preserve the programmable advanced-color draft
- [x] Phase 1 - serialize and deserialize optional advanced-color draft state through diagnostics capture, finding capture, and state load
- [x] Phase 2 - validate with focused finding/state tests, native helpers, runtime publish, staged validate-ui, runtime pytest, contract validation, and phased-plan sync

## Explicit User Asks

- [open] Stay on the advanced-color productization work longer and build out the feature surface honestly.
- [open] Include advanced color state in the Capture Finding button flow.
- [open] Plan the work in detail instead of hand-waving the broader changes.

## Presumption Loop

The explicit Capture Finding gap is not missing scalar color params; those are already serialized. The missing user-visible state is the programmable advanced-color draft owned by `ColorPipelineWindowState` in `ui_app/src/color_pipeline_window.h` and `ui_app/src/main.cpp`. The falsifiable hypothesis is that the smallest honest first slice is to persist and reload that optional draft state through diagnostics capture and finding load before widening more advanced-color categories. The cheapest disconfirming checks are RED coverage in `ui_app/tests/test_diagnostics_state_io.cpp`, `ui_app/tests/test_finding_archive_actions.cpp`, and `ui_app/tests/test_finding_state_actions.cpp` showing that a non-default advanced-color draft is absent from captured `state.json` and cannot be restored on load until the persistence path lands.

## Presumption Evidence

- `BuildStateJson(...)` in `ui_app/src/diagnostics_capture.cpp` already serializes resolved color params but does not serialize `ColorPipelineWindowState` or any advanced-color draft rows.
- `RunInLoopFindingCapture(...)` in `ui_app/src/main.cpp` and `CaptureAndArchiveFindingBundle(...)` in `ui_app/src/finding_archive_actions.cpp` only pass `ViewState`, `KernelParams`, and render/capture metadata into the bundle path.
- `LoadDiagnosticsStateFile(...)` and `LoadFindingSelectionIntoRuntime(...)` restore runtime state but have no output seam for `ColorPipelineWindowState`.
- The advanced-color draft model already exists as stable structs in `ui_app/src/color_pipeline_window.h`, so the missing capability is persistence and restoration, not new UI state definitions.

## Proof Ledger

- Done: focused RED coverage now exists in `ui_app/tests/test_diagnostics_state_io.cpp`, `ui_app/tests/test_finding_archive_actions.cpp`, and `ui_app/tests/test_finding_state_actions.cpp` for programmable draft capture/load round-trip.
- Done: diagnostics capture now writes an optional `color_pipeline_draft` object and preserves backward compatibility when the field is absent.
- Done: finding/state load now restores the saved draft into `ColorPipelineWindowState`, seeds the live snapshot from loaded runtime state, and avoids first-render draft clobber.
- Done: the adjacent runtime-coloring repair in `ui_app/src/escape_time_coloring.h` now keeps repeat Shape wrapping in the iteration-band domain, so live band-count changes still affect banded coloring. The focused `ui_app/tests/test_escape_time_coloring.cpp` regression is green, the native helper rail is green, runtime publish is green, staged `--validate-ui` exits cleanly, and the runtime pytest lane passes 68/68.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_pipeline_finding_capture_draft_state_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_pipeline_finding_capture_draft_state.contract.json`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/src/diagnostics_capture.h`
  - `ui_app/src/diagnostics_capture.cpp`
  - `ui_app/src/diagnostics_state_io.h`
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/src/finding_archive_actions.h`
  - `ui_app/src/finding_archive_actions.cpp`
  - `ui_app/src/finding_state_actions.h`
  - `ui_app/src/finding_state_actions.cpp`
  - `ui_app/src/main.cpp`
  - `ui_app/tests/test_diagnostics_state_io.cpp`
  - `ui_app/tests/test_escape_time_coloring.cpp`
  - `ui_app/tests/test_finding_archive_actions.cpp`
  - `ui_app/tests/test_finding_state_actions.cpp`
- Non-goals:
  - do not open the Grade lane in this slice
  - do not implement multi-function-per-lane composition in this slice
  - do not widen posterize or other advanced-color functions in this slice

## Resume Point

Add the failing capture/load regressions first, then wire the optional `color_pipeline_draft` payload through diagnostics capture, finding capture, and load.