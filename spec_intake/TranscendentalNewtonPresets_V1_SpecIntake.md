# Spec Intake Packet

**Spec Title:** Transcendental Newton Presets (V1) — sin / exp-1 / cosh

**Date:** 2026-01-15

## 0) Intake Summary
Add Newton-function presets under the existing Newton fractal type, without changing the overall render substrate.

This is a Phase-1 unit following `NovaFractal_V1_SpecIntake.md`.

## 1) Open Decisions (must be resolved before implementation)
- Which transcendental functions are in-scope for v1: `sin`, `exp-1`, `cosh`?
- How we define fail-fast behavior for overflow (exp/cosh can overflow quickly).
- Whether these are purely presets (no user coefficients) or allow parameterized variants.

## 2) Planned Binding Surface
- Add a Newton-only enum: `fractal.params.newton_function` (e.g., poly|sin|exp_minus_1|cosh)
- Visible only when `fractal.view.fractal_type == newton`

## 3) Fail-Fast Policy
- No silent clamps.
- Non-finite intermediate values must map to explicit error coloring or explicit render error.

## 4) Acceptance Criteria
- Each preset yields a distinct image.
- Invalid domains are explicit.
