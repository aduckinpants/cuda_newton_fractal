# Spec Intake Packet

**Spec Title:** Nova Fractal (V1) — Escape-Time / Newton-Hybrid Addition

**Date:** 2026-01-15

## 0) Intake Summary
Add a new fractal mode **Nova** as a first “catalog expansion” unit that stays within the current rendering substrate:
- single-pass CUDA kernel producing RGBA
- schema-driven UI surface
- fail-fast parameter validation (no silent clamping/fallback)

Nova is selected via the existing `fractal.view.fractal_type` enum.

## 1) Definition (V1)
Nova is a Newton-like iteration with a constant term that produces escape-time-like structure.

V1 uses the common form:
$$
z_{n+1} = z_n - \alpha \frac{f(z_n)}{f'(z_n)} + c
$$

Where:
- $f(z)$ is a polynomial function. In V1 we **reuse the existing Newton polynomial selection** (`poly_kind` + coeffs).
- $\alpha$ is a real relaxation factor (Nova alpha).
- $c$ is a complex constant. In V1, $c$ is derived from the pixel coordinate (same as Mandelbrot-style parameterization):
  - interpret screen coordinate as complex plane coordinate `coord` (already computed)
  - set $c = coord$

**Escape condition** (V1): treat as escape-time family with radius 2:
- if $|z|^2 > 4$ => escaped

**Iteration start** (V1):
- $z_0 = 0$ (simple and stable; consistent with escape-time family)

## 2) Parameter Surface (Bindings)
### 2.1 Existing bindings reused
- `fractal.params.max_iter` (int)
- `fractal.params.epsilon` (float) — reused as Newton convergence epsilon in the Newton-step term
- `fractal.params.poly_kind` (enum)
- `fractal.params.poly_coeffs` (float[5])
- `fractal.params.coloring_mode` (enum)
- `fractal.params.exposure` (float)

### 2.2 New bindings required (V1)
- `fractal.params.nova_alpha` (float)
  - meaning: relaxation factor $\alpha$
  - default: `0.50`
  - domain: `0 < nova_alpha <= 2.0`

Optional (not required in V1):
- a separate `nova_z0_mode` or `nova_seed` is deferred.

## 3) Schema Requirements
### 3.1 Enum extension
Add enum member to `fractal.view.fractal_type`:
- `nova`

### 3.2 Controls
Add a control for `fractal.params.nova_alpha`:
- visible only when `fractal.view.fractal_type == nova`
- slider float (or supported equivalent control type)
- suggested UI range: `[0.0, 2.0]` (domain enforcement is still fail-fast in code)

### 3.3 Coloring constraints
Nova is **not Newton basin coloring**. Treat as escape-time coloring family.
- When `fractal_type == nova`, schema must prevent `coloring_mode == root_basin`.
- Allowed: `iteration_count`, `smooth_escape`.

## 4) Fail-Fast Rules (No Implicit Fallback)
### 4.1 Parameter validation (host-side)
Before dispatch:
- if `nova_alpha <= 0` or `nova_alpha > 2.0` => return error string.
- if `poly_kind == custom` and the polynomial/derivative eval produces non-finite values at runtime, we do **not** clamp; we surface error coloring.

### 4.2 Runtime numeric rules (device-side)
Inside the pixel loop:
- if any intermediate becomes NaN/Inf => mark as escaped and render explicit error color (magenta), OR (if we already have a kernel-wide error reporting mechanism later) adopt it.

No silent clamps of:
- alpha
- epsilon
- polynomial coefficients

## 5) Presets and Reset Semantics
When switching `fractal_type` to Nova or on Reset All:
- set:
  - `max_iter` (recommended initial): `300` (tuneable)
  - `epsilon`: `1e-6`
  - `nova_alpha`: `0.50`
  - `coloring_mode`: `smooth_escape`
  - `exposure`: `1.0`

View controls (center/zoom/rotation) remain unchanged.

## 6) Renderer / Kernel Integration Notes
### 6.1 Classification
Nova integrates as a **third family** in the kernel:
- Newton-like step term uses `poly_eval_real_coeffs_deg4()` and its derivative
- escape-time termination uses radius check

### 6.2 Required new code paths (planned)
- Add `FractalType::nova`
- Add `KernelParams::nova_alpha`
- Add a `case` in the kernel loop implementing the Nova update

## 7) Acceptance Criteria
- Selecting `nova` renders a non-trivial image (not all black).
- `root_basin` cannot be selected in schema when `nova` is active.
- Invalid `nova_alpha` causes a **render error** (host-side) and does not clamp.
- Numeric instability yields explicit error coloring (not undefined memory / silent corruption).
- Reset All and fractal-type switching apply explicit presets (no “black square” surprises).

## 8) Out of Scope (Explicit)
- Nova variants with separate constant $c$ controls (complex constant UI)
- Nova seeded with z0 from pixel coordinate
- Distance estimation / orbit traps
- Perturbation optimization specific to Nova
