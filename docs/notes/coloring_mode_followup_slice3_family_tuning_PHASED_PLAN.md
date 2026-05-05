# Coloring Mode Follow-Up Slice 3 - Family Tuning And Render Polish

## Current Phase

Phase 1 - wait for slice 2 closeout and define the render proof set

## Phase Checklist

- [ ] Phase 1 - wait for slice 2 closeout and capture the family-tuning baseline
- [ ] Phase 2 - define the concrete proof scenes and target tuning seams
- [ ] Phase 3 - implement the per-family tuning and focused render regressions
- [ ] Phase 4 - hostile audit the resulting color behavior and checkpoint the slice

## Explicit User Asks

- [open] Bring the remaining coloring-mode follow-up back to the table after the UI polish merge.
- [open] Keep this thread separate from camera, view presets, and responsiveness.
- [open] Stay on the bounded simpler architecture first.

## Presumption Loop

The likely owner is the current escape-time coloring seam plus the family rules that decide what combinations are legal. This slice should target the concrete known issues: per-family smooth-escape tuning, interior treatment, and deep-zoom normalization.

Hostile review assumes aesthetic drift is the biggest risk. The slice therefore needs a fixed proof set before any tuning lands so "looks better" has a deterministic meaning.

## Presumption Evidence

- Owner Proof: `KNOWN_ISSUES.md` already names the open escape-time tuning defects and points to the current renderer/escape-time seams.
- RED Witness: pending.
- Fix Proof: pending.
- Hostile Review Pass 1: pending.
- Hostile Review Pass 2: pending.

## Proof Ledger

- Manual RED: pending.
- Checked-in regression RED: pending.
- First GREEN: pending.
- Post-green hostile finding: pending.

## Notes

- Expected owner files:
  - `ui_app/src/fractal_renderer.cu`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/src/basin_coloring.h`
  - `ui_app/src/fractal_family_rules.h`
  - `ui_app/tests/test_escape_time_coloring.cpp`
  - `ui_app/tests/test_basin_coloring.cpp`
- Non-goals:
  - do not widen into live pacing or camera behavior
  - do not treat programmable color expressions as part of this slice

## Resume Point

Slice 3 is blocked on slice 2. Once the split UI/state surface is stable, tune the family behavior against a fixed proof set rather than ad hoc aesthetic comparison.