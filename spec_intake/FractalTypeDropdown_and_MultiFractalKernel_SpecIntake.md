# Spec Intake Packet

**Spec Title:** Fractal Type Dropdown + Multi-Fractal Kernel Switch

**Date:** 2026-01-15

## 0) Intake Summary
The spec is directionally solid: a single `fractal_type` switch driving multiple formulas is a clean extension point and fits the current architecture.

However, there are two concrete mismatches with the current implementation and a few hidden-contract questions that should be resolved before adding new kernels:

- **Binding path + location mismatch:** spec requests `fractal.view.fractal_type` in the **View** panel; the current implementation uses `fractal.params.fractal_type` in the **Fractal** panel.
- **Option set mismatch:** spec’s initial set includes `burning_ship` and `multibrot (n=3)`; current code/schema supports only `newton|mandelbrot|julia`.
- **Parameter applicability:** Newton-only knobs (`epsilon`, `poly_kind`, `poly_coeffs`, `root_basin` coloring) are not meaningful for escape-time fractals unless explicitly defined.

## 1) Current Implementation Snapshot (Already Shipped)
This is the “as built” state as of this intake:

- **Enums:** `FractalType` exists with values `newton|mandelbrot|julia`.
- **State location:** `FractalType fractal_type` currently lives in `KernelParams`, not `ViewState`.
- **Schema placement:** the existing dropdown is under panel `fractal` and bound to `fractal.params.fractal_type`.
- **UI behavior:** UI warns that only Newton is actually implemented; non-Newton selection does not change rendering yet.
- **Reset actions:** `Reset View` and `Reset All` exist; `Reset All` resets `params.fractal_type` back to Newton.

## 2) Spec Clarifications Needed (Blocking Questions)
### 2.1 Binding Path: `fractal.view.fractal_type` vs `fractal.params.fractal_type`
The spec says “add enum to the view state” and binds `fractal.view.fractal_type`.

In the current architecture, `fractal_type` is conceptually a **kernel/iteration choice**, which fits naturally under `params`.

**Decision needed (pick one):**
1) **Adopt spec literally:** move/duplicate `fractal_type` into `ViewState` and plumb it into the kernel params (or into the kernel arguments).
2) **Revise spec:** keep `fractal.params.fractal_type` as canonical, but place the dropdown in the View panel for UX reasons.
3) **Alias layer (best of both):** keep `params.fractal_type` as the engine source of truth, but accept `fractal.view.fractal_type` as a UI alias that writes through to `params.fractal_type`.

Recommendation: **(3) alias** if we want to match the spec text without disturbing existing code/bindings too much.

### 2.2 Control Placement: View vs Fractal panel
Spec explicitly wants the dropdown under **View**.

This is a pure schema decision and can be done regardless of whether the binding is `view` or `params`.

### 2.3 Coloring + iteration controls across fractal types
Newton uses “basins of attraction” and polynomial controls. Mandelbrot/Julia/Burning Ship/Multibrot are typically **escape-time** fractals.

Questions:
- When `fractal_type != newton`, what does `coloring_mode = root_basin` mean? (Probably should be hidden/disabled.)
- Does `epsilon` apply only to Newton? (Likely yes.)
- Should `max_iter` be shared across all types? (Likely yes.)

Recommendation: treat Newton controls as **type-specific** and hide them when not applicable using schema predicates.

## 3) Lippert-Style Notes (Risk/Scope Hygiene)
- “No new buffers/state” is compatible with these additions. Escape-time loops use only local registers.
- The later “sock-obliterating” ideas (Lyapunov, perturbation deep zoom, distance estimators, orbit traps) range from “still no buffers” to “new math contracts.” They should be **explicitly staged** as v2/v3 features.
- Multibrot: the spec mentions both “Multibrot (n=3)” and “p in {3,4,5}”. For v1, lock it to **n=3** to avoid adding another control surface.

## 4) Recommended Revisions (Minimal, High-Leverage)
### 4.1 Canonical binding + UI contract
- Keep engine canonical as `fractal.params.fractal_type`.
- Add optional alias `fractal.view.fractal_type` (write-through) only if you want the spec to read “view” while engine stays stable.

### 4.2 Expand enum set (v1)
Target enum values:
- `newton`
- `mandelbrot`
- `julia`
- `burning_ship`
- `multibrot_3`

### 4.3 Julia constant
Hard-code `c = -0.7 + 0.27015i` for now (as spec suggests). No UI yet.

### 4.4 Parameter applicability rules (documented)
- For escape-time fractals: ignore `poly_kind`, `poly_coeffs`, `epsilon`.
- Restrict available coloring modes to those that have meaning for escape-time (e.g., `iteration_count`, `smooth_escape`).

## 5) Riker Plan Packet (Implementation Plan, Not Started Yet)
### Phase A — Spec alignment + schema cleanup (small)
1) Decide binding canonical (`params` vs `view` vs alias).
2) Move the combo control into the `view` panel (schema-only change).
3) Add type-specific visibility predicates to hide Newton-only controls when `fractal_type != newton`.

### Phase B — Add the new kernel formulas (core work)
1) Extend `FractalType` enum with `burning_ship`, `multibrot_3`.
2) In `fractal_renderer.cu`, implement a `switch(fractal_type)`:
   - **Newton:** existing logic.
   - **Mandelbrot:** escape-time loop.
   - **Julia:** escape-time loop with fixed `c`.
   - **Burning Ship:** escape-time loop with abs on real/imag before squaring.
   - **Multibrot_3:** escape-time loop using `z^3 + c`.
3) Define “escape threshold” constant (e.g., `|z|^2 > 4`) and share across escape-time types.

### Phase C — Visual correctness & performance sanity
1) Ensure `max_iter` is applied consistently.
2) Ensure the existing camera behavior (auto-dive, zoom, pan, rotation) works identically across types.
3) Ensure coloring does not fall into “all black” for non-Newton modes.

## 6) Acceptance Criteria
- Dropdown exists under **View** panel with options: Newton, Mandelbrot, Julia, Burning Ship, Multibrot (n=3).
- Switching types changes the image immediately under auto-refresh (or on render-once if auto-refresh off).
- Pan/zoom/rotation and the camera behavior loop work unchanged.
- No new buffers/state surfaces are introduced.

## 7) Deferred Ideas (Explicitly Out of Scope for v1)
- Nova / Phoenix (Phoenix adds a “previous z” register; still no buffers, but new semantics).
- Orbit traps (adds per-iteration tracking; still no buffers but new coloring contract).
- Distance estimator shading (adds derivative tracking; changes shading/UX expectations).
- Perturbation deep zoom (significant algorithmic shift; likely new numeric/stability policies).
