# Spec Intake Packet

**Spec Title:** Color Pipeline Tune‑Pass Directive

**Date:** 2026-01-15

## 0) Spec Text (as provided)
- Escape-time smoothing parity
- Exposure curve refinement (non-linear)
- Color mapping consistency
- No hidden state (all color params explicit in schema)
- No fallback behavior (invalid params => errors)
- Non-goals: no view mapping / camera / iteration changes; no new buffers
- Rosa provides aesthetic pass

## 1) Intake Summary
Approved as a clean “quality parity” directive.

Key point: under the "no hidden state" + "no fallback" constraints, we should treat the color pipeline as an explicit, schema-driven contract. That means:
- all knobs are surfaced in schema
- each fractal family has an explicit palette selection and smoothing policy
- invalid ranges are rejected (error surfaced), not clamped

## 2) Current State (baseline)
- Escape-time smooth coloring exists but is not yet standardized across all fractal types.
- Exposure is currently a simple tone-mapped curve (not perceptually tuned per your spec).
- Newton root-basin coloring exists (separate aesthetic domain from escape-time).

## 3) Clarifications Needed (to avoid implicit magic)
1) Do we allow per-fractal **defaults** (via preset application) as long as they are defined in schema defaults/presets and documented? (This matches your earlier preset work.)
2) Do we want a single global palette for all escape-time fractals, or per-type palette selection?
3) What is the desired “error surface” when color params are invalid?
   - return a render error (UI error window)
   - or render an explicit error color (magenta) with message

## 4) Recommended Parameter Surface (schema-first)
Minimal explicit set for escape-time fractals:
- `fractal.color.palette_id` (enum)
- `fractal.color.gamma` (float)
- `fractal.color.exposure` (float) OR `fractal.color.exposure_curve` (enum + params)
- `fractal.color.smoothing` (enum: none|classic|normalized)
- `fractal.color.band_limit` / `dither_strength` (optional, explicit)

Newton-specific:
- keep root-basin palette as its own selection to preserve identity

## 5) Fail-Fast Policy (matches directive)
- Invalid palette id => schema error (unloadable) or runtime error (explicit), but never “nearest palette.”
- Invalid numeric ranges => render error (not clamped).
- Disallowed combinations (e.g., Newton-only root-basin mode in escape-time) => make unselectable in schema.

## 6) Execution Plan (next work item; not implementing yet)
1) Introduce explicit color parameter bindings + schema controls.
2) Implement a single, consistent smoothing function for all escape-time fractals.
3) Replace exposure with a perceptual curve (gamma/soft-log), controlled by schema.
4) Add palette mapping with stable zoom behavior (avoid banding).
5) Rosa pass: choose defaults, palettes, curve parameters.

## 7) Acceptance Criteria
- Each fractal type renders with intentional defaults immediately after selection.
- No banding/posterization at typical zoom levels.
- Invalid params fail fast and are visible.
- No new buffers; no view/camera/iteration changes.
