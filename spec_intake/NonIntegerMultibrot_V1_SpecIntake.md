# Spec Intake Packet

**Spec Title:** Non-Integer Multibrot (V1) — Complex Power via Polar Form (Domain-Gated)

**Date:** 2026-01-15

## 0) Intake Summary
Add non-integer multibrot power $p$ (float) as a gated Phase-1 item.

This is only implemented if we can define explicit, fail-fast rules for branch cuts and undefined cases.

## 1) Open Decisions / Gate
- V1 power domain proposal: `p in [2,12]` (float).
- Negative p is **out of scope** unless explicitly defined.
- Define what happens at/near `z=0` for fractional powers.

## 2) Planned Binding Surface
- `fractal.view.fractal_type == multibrot` continues to select the family.
- Add `fractal.params.multibrot_power_float` (float) OR replace int power with float power (breaking change — avoid unless explicitly approved).

## 3) Implementation Sketch (informal)
- Use polar form: $z^p = \exp(p \log z)$ with explicit branch selection.
- Fail-fast on undefined `log(0)`.

## 4) Acceptance Criteria
- Domain violations return explicit render error.
- No silent clamps.
