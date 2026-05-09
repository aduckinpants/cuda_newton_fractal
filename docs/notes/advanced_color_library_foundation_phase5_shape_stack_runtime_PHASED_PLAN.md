# Advanced Color Library Foundation Phase 5 - Shape Stack Runtime Recovery

## Current Phase

Complete - the bounded Shape-stack runtime slice closed at `e6b55b9` under `ck:1fbf6d4d`; this subplan is now a completed predecessor for the remaining Phase 5 backend-recovery thread

## Phase Checklist

- [x] Phase 1 - add focused REDs that prove a two-row Shape lane must apply to live runtime state, sync back from live state, and affect escape-time math in order
- [x] Phase 2 - replace the single-shape runtime owner with a bounded Shape-stack runtime owner while preserving backward-compatible defaults and legacy single-row imports
- [x] Phase 3 - wire the advanced window live bridge to import/apply supported Shape stacks instead of rejecting more than one enabled Shape row
- [x] Phase 4 - validate, hostile-audit, and checkpoint the bounded Shape-stack runtime slice cleanly

## Explicit User Asks

- [done] Stop forcing the advanced color system to behave like only one function per area is real.
- [done] Do the work correctly instead of asking whether to proceed.
- [done] Keep the mapped advanced-color inventory as binding authority rather than reopening MVP definition.
- [done] Follow the mainline phased-plan and contract workflow while landing the first real backend-recovery slice.

## Presumption Loop

The nearest controlling seam is the Shape lane live bridge in `ui_app/src/color_pipeline_window.h`, plus the single-shape runtime authority in `ui_app/src/fractal_types.h` and the single-shape execution seam in `ui_app/src/escape_time_coloring.h`. The controlling defect is now concrete: the schedule editor already supports multiple Shape rows, but the live runtime still only owns one Shape enum plus one flat parameter pack, and the live bridge rejects more than one enabled Shape row outright. The falsifiable hypothesis for this bounded slice is that a fixed-size Shape-stack runtime owner can make a two-row shipped Shape lane truthful without widening Source or Palette beyond their current single-row runtime tuples. The cheapest disconfirming checks are focused REDs in `ui_app/tests/test_schema_binding.cpp` and `ui_app/tests/test_escape_time_coloring.cpp` that currently fail because the live bridge rejects two enabled Shape rows and the runtime shape math only evaluates one row.

## Presumption Evidence

- `ui_app/tests/test_schema_binding.cpp` already proves the schedule editor can append, reorder, and remove runtime-backed Shape rows, so the remaining gap is live/runtime truth rather than editor chrome.
- `ui_app/src/color_pipeline_window.h` still routes live import/apply through `FindSingleEnabledColorPipelineRow(...)`, `BuildColorPipelineLaneWithSingleRow(...)`, and `rows.front()`, which hard-proves the current live bridge is single-row only.
- `ui_app/src/fractal_types.h` still stores Shape state as one `ColorPipelineShape` plus one flat owner pack, which blocks independent per-row parameter truth for a stacked lane.
- `ui_app/src/escape_time_coloring.h` still applies Shape through `ApplyColorPipelineShapeValue(...)`, which only evaluates one active Shape row.
- `ui_app/src/diagnostics_state_io.cpp`, `ui_app/src/diagnostics_capture.cpp`, and `ui_app/src/fractal_derived_fields.cpp` prove the first honest runtime-owner change must also cover defaults and persistence instead of leaving the new Shape stack as an in-memory-only side path.

## Proof Ledger

- Landed: `ui_app/tests/test_schema_binding.cpp` now forces a supported two-row Shape draft through live apply plus live-snapshot resync instead of allowing the old single-row bridge to classify the lane as unsupported.
- Landed: `ui_app/tests/test_escape_time_coloring.cpp` now proves ordered two-row Shape composition instead of allowing runtime math to collapse to only the final Shape row.
- Landed: `ui_app/src/fractal_types.h`, `ui_app/src/escape_time_coloring.h`, `ui_app/src/color_pipeline_window.h`, and `ui_app/src/fractal_derived_fields.cpp` now own a bounded Shape-stack runtime authority with legacy single-row fallback and reset/default coverage.
- Landed: `ui_app/src/diagnostics_capture.cpp`, `ui_app/src/diagnostics_state_io.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp`, `ui_app/tests/test_finding_archive_actions.cpp`, and `ui_app/tests/test_runtime_reset.cpp` now keep `color_shape_stack` truthful through diagnostics save/load, capture/archive output, and reset behavior while mirroring the last stack row back into the legacy flat Shape fields.
- Validated: `artifacts/code_quality_report.json` is back on the repository baseline after the loader refactor, `artifacts/verify_native_helper_tests_shape_stack.log` is green, `artifacts/verify_runtime_publish.log` republished the runtime cleanly, and `artifacts/verify_runtime_probe_session_pytest.log` reports `68 passed` against the published runtime.
- Checkpointed: the slice closed at commit `e6b55b9` with the linked `ck:1fbf6d4d` handoff entry plus machine-written validation and contract proof receipts.

## Hostile Audit

- Status: complete
- Required posture: assume the first backend-recovery attempt will accidentally preserve a hidden single-row authority somewhere in apply, sync, reset, or persistence until the repaired state is re-read and disproven.

## Audit Passes

- [done] Pass 1 - inspect the REDs and landed runtime-owner diff for hidden single-row fallbacks in `color_pipeline_window.h` and `escape_time_coloring.h`.
- [done] Pass 2 - inspect defaults, reset, and diagnostics persistence so the new Shape stack does not exist only in the live editor.
- [done] Pass 3 - re-read the repaired state after validation and confirm the slice actually supports a truthful two-row Shape lane end to end.

## Audit Findings

- [done] Real defect found: the first backend-recovery implementation would still have lost supported Shape stacks on diagnostics save/load and archive capture because only the legacy flat Shape fields were being serialized; the slice now persists `color_shape_stack` explicitly and mirrors the last stack row for backward compatibility.
- [done] Real defect found: the first diagnostics loader implementation for `color_shape_stack` regressed the repo code-quality baseline by pushing a helper past the max-function limit; the loader is now split into smaller helpers and the baseline audit is green again.
- [done] Re-read result: after the persistence repair and helper split, the bounded Shape-stack slice has no remaining hidden single-row fallback in apply, runtime math, reset/defaults, or diagnostics persistence within the files owned by this sub-slice.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`
  - `docs/notes/advanced_color_library_foundation_phase5_shape_stack_runtime_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_library_foundation_phase5_shape_stack_runtime.contract.json`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/diagnostics_capture.cpp`
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/src/fractal_derived_fields.cpp`
  - `ui_app/src/fractal_types.h`
  - `ui_app/tests/test_diagnostics_state_io.cpp`
  - `ui_app/tests/test_escape_time_coloring.cpp`
  - `ui_app/tests/test_finding_archive_actions.cpp`
  - `ui_app/tests/test_runtime_reset.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals for this slice:
  - do not widen Grading yet
  - do not widen Source or Palette beyond the current single-row runtime tuples
  - do not redefine the mapped advanced-color inventory
  - do not land editor-only stacked rows without matching live/runtime truth

## Resume Point

Do not reopen checkpoint closure from this subplan. Return to the main Phase 5 foundation plan and continue backend recovery from the truthful Shape-stack base, with Source/Palette live-bridge and runtime-state authority as the next remaining seam.