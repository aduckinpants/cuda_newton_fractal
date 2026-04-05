# Spec Intake Packet

**Spec Title:** Fractal Catalog Wave Two (Tricorn / Halley / McMullen Rational / Lambda)

**Date:** 2026-04-05

## 0) Intake Summary
The current viewer already spans Newton, Nova, three Explaino variants, and five escape-time families, but the catalog is still narrow for a fractal engine. This packet adds the next unspecced families that are visually distinct and still plausible within the current single-pass CUDA + schema-driven substrate.

This packet complements, and does not replace, the existing packets for Nova, Phoenix, Transcendental Newton, and Non-Integer Multibrot.

## 1) Selection Rules
Wave-two candidates were chosen using four filters:
- clear visual differentiation from the shipped catalog
- compatibility with the current view mapping and single-pass render loop
- explicit parameter surface with fail-fast validation
- no requirement for persistent accumulation buffers in V1

## 2) Proposed Additions

### 2.1 Tricorn / Multicorn
**Priority:** high
**Category:** escape-time

Definition:
$$
z_{n+1} = \overline{z_n}^p + c
$$

V1 recommendation:
- add `FractalType::multicorn`
- reuse `multibrot_power` as the integer power control
- default `multibrot_power = 2` so the first shipped preset is the classic Tricorn

Allowed coloring:
- `iteration_count`
- `smooth_escape`

Disallowed coloring:
- `root_basin`
- `joy_basins`

Why it belongs:
- It is a low-friction extension of the existing escape-time substrate.
- It adds a clearly different symmetry class from Mandelbrot/Multibrot.
- It keeps the catalog broad without needing new buffers or a new UI ontology.

Fail-fast rules:
- `multibrot_power < 2` is invalid.
- unknown power or invalid schema enum ids must fail validation, not fall back.

### 2.2 Halley
**Priority:** high
**Category:** root-finding

Definition:
$$
z_{n+1} = z_n - \frac{2 f(z_n) f'(z_n)}{2 (f'(z_n))^2 - f(z_n) f''(z_n)}
$$

V1 recommendation:
- add `FractalType::halley`
- reuse the existing polynomial surface: `poly_kind`, `poly_coeffs`, `epsilon`, `max_iter`, `exposure`
- add a focused `poly_eval_real_coeffs_deg4_second_derivative()` helper rather than duplicating derivative math in the kernel

Allowed coloring:
- `root_basin`
- `joy_basins`
- `iteration_count`

Optional later:
- `smooth_escape` is deferred unless a meaningful convergence-distance definition is specified

Why it belongs:
- It widens the root-finding side of the catalog, not just the escape-time side.
- It gives a direct solver-order comparison against Newton and the Explaino family on the same polynomials.
- It creates a reusable seam for future Householder-style methods.

Fail-fast rules:
- if the Halley denominator becomes non-finite or too small, render explicit error color or stop the pixel with a defined non-converged result
- do not silently clamp the denominator or the step size

### 2.3 McMullen Rational Presets
**Priority:** medium
**Category:** escape-time

V1 should avoid a fully generic `P(z)/Q(z)` surface. Instead, ship a preset-driven rational family using the McMullen-style form:
$$
z_{n+1} = z^m + \frac{\lambda}{z^n}
$$

V1 recommendation:
- add `FractalType::mcmullen`
- add a preset enum controlling `(m, n, lambda)` tuples
- keep raw coefficient editing out of V1

Allowed coloring:
- `iteration_count`
- `smooth_escape`

Why it belongs:
- It adds disconnected dust, webs, and ring topologies not represented by the current catalog.
- A preset enum keeps the surface explicit and controllable.

Fail-fast rules:
- division-by-zero neighborhoods must have explicit behavior
- invalid preset ids must fail validation
- no hidden substitution to a default preset when data is invalid

### 2.4 Lambda
**Priority:** research-medium
**Category:** complex logistic / boundedness family

Definition target:
$$
z_{n+1} = \lambda z_n (1 - z_n)
$$

Open question:
- whether V1 should treat this as a complex-plane escape-time family, a boundedness family, or a parameter-plane visualization

V1 recommendation:
- keep Lambda as a spec-first research item until the parameterization is pinned down
- if implemented later, add `FractalType::lambda` with explicit `lambda_real` and `lambda_imag` fields

Why it belongs:
- It adds a genuinely different dynamic system and broadens the engine beyond polynomials.
- It is mathematically adjacent to Lyapunov work without requiring the full Lyapunov substrate first.

Risks:
- poor default parameterization can produce uninteresting black or saturated output
- the boundedness contract must be explicit before implementation

## 3) Recommended Order
1. Tricorn / Multicorn
2. Halley
3. McMullen rational presets
4. Lambda research spike

This order is intentionally biased toward additions that fit the current engine without introducing new long-lived state.

## 4) Shared Integration Requirements
Every new fractal type in this packet must do all of the following:
- add explicit enum ids in C++ and schema
- apply deterministic defaults on type switch and Reset All
- declare allowed coloring modes in schema and in runtime validation
- expose invalid parameter domains as explicit errors, not implicit fallback
- support CLI selection and diagnostics capture naming
- add at least one focused headless test for defaults or rule classification before kernel work

## 5) Explicit Non-Goals
This packet does not authorize:
- Lyapunov fractals
- attractor density fields
- orbit-trap shading as a standalone deliverable
- distance-estimator lighting
- any new persistent accumulation buffer

Those remain future roadmap items after the current catalog and solver surfaces are more stable.