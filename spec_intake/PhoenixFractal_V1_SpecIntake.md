# Spec Intake Packet

**Spec Title:** Phoenix Fractal (V1) — Escape-Time with Memory Term

**Date:** 2026-01-15

## 0) Intake Summary
Add Phoenix as a Phase-1 fractal type using the existing per-pixel loop shape (no new persistent buffers).

## 1) Open Decisions
- Parameterization: real vs complex constants (e.g., `p` term), and whether they are fixed presets in v1.
- Mapping: does pixel coordinate supply `c`, or does it supply initial z0?

## 2) Planned Binding Surface
- `fractal.view.fractal_type` add `phoenix`
- `fractal.params.phoenix_p_real`, `fractal.params.phoenix_p_imag` (or preset enum)

## 3) Coloring Constraints
- Phoenix is escape-time family: disallow `root_basin`.

## 4) Fail-Fast Rules
- Explicit parameter domains.
- Non-finite values => explicit error.

## 5) Acceptance Criteria
- Switching to Phoenix applies presets.
- Renders non-trivial structure.
