# Spec Intake Packet

**Spec Title:** Explaino Family Expansion (Nova / Halley / Dual-Seed / Multiplicity)

**Date:** 2026-04-05

## 0) Intake Summary
The shipped Explaino set already has three distinct variants:
- `explaino` — baseline warped-seed Newton
- `explaino_y` — feedback-heavy variant with re-warp and momentum-like behavior
- `explaino_fp` — damped/fixed-point biased variant that snaps non-converged results toward a root

That is a strong start, but it still leaves several high-value axes unexplored:
- escape-time hybrids on seeded procedural polynomials
- higher-order root-finding on the same seeded polynomial surface
- explicit topology morphing between seed families
- deliberate clustered or repeated roots to expose failure geometry

This packet defines the next Explaino family roadmap without implementing it yet.

## 1) Working Principles
- Keep the current Explaino joy palette frozen for now.
- Keep warp disabled by default for all new Explaino variants unless a spec explicitly overrides that later.
- Prefer explicit enum entries over hidden mode flags in the first expansion wave, because the shipped Explaino variants are already modeled as top-level `FractalType` values.
- Reuse the seeded polynomial generator when possible instead of inventing a second procedural root system.

## 2) Proposed Additions

### 2.1 Explaino-Nova
**Priority:** very high
**Category:** escape-time hybrid

Definition:
$$
z_{n+1} = z_n - \alpha \frac{f(z_n)}{f'(z_n)} + c
$$

Where:
- `f(z)` comes from the existing Explaino seeded polynomial generator
- `c` is derived from the pixel coordinate
- `z0 = 0`

Recommended surface:
- add `FractalType::explaino_nova`
- reuse `explaino_seed`, `explaino_seed_drift`, `explaino_phase`, `explaino_warp_strength`
- add or reuse `nova_alpha`

Allowed coloring:
- `iteration_count`
- `smooth_escape`

Why it matters:
- It directly bridges the engine's two most interesting directions: Nova and Explaino.
- It exposes whether the seeded procedural polynomial surface remains interesting when moved into an escape-time contract.
- It gives the Nova repair work an immediately valuable follow-on target.

### 2.2 Explaino-Halley
**Priority:** very high
**Category:** higher-order root-finding

Definition:
$$
z_{n+1} = z_n - \frac{2 f(z_n) f'(z_n)}{2 (f'(z_n))^2 - f(z_n) f''(z_n)}
$$

Recommended surface:
- add `FractalType::explaino_halley`
- reuse the current Explaino seeded polynomial generator and root list output
- do not add a new warp model in V1

Allowed coloring:
- `root_basin`
- `joy_basins`
- `iteration_count`

Why it matters:
- It provides an apples-to-apples solver comparison on the same seeded polynomial families.
- It is likely to reveal new basin boundaries and convergence islands without changing the seed ontology.
- It creates shared derivative infrastructure that also benefits a general Halley fractal.

### 2.3 Explaino-DualSeed
**Priority:** high
**Category:** topology morph / procedural interpolation

Concept:
- derive two seeded root sets from `seed_a` and `seed_b`
- blend those root sets with an explicit `mix` parameter before polynomial construction

Recommended surface:
- add `FractalType::explaino_dual`
- add `explaino_seed_b`
- add `explaino_mix` in `[0, 1]`

Why it matters:
- The current fractional drift gives temporal tweening between adjacent seeds, but not a stable two-seed design surface.
- A dual-seed family would make cross-seed topology changes deliberate and inspectable.
- It should be useful for sweep tooling, reproducible demos, and future preset catalogs.

### 2.4 Explaino-Multiplicity
**Priority:** high
**Category:** degenerate / clustered-root family

Concept:
- procedurally place two or more roots close together, or intentionally repeat roots, before polynomial expansion

Recommended surface:
- add `FractalType::explaino_mult`
- add a small clustered-root control such as `explaino_cluster_radius`
- optionally add a small enum for multiplicity mode (`pair`, `triple`, `near_collision`)

Why it matters:
- It turns Newton failure modes and solver sensitivity into a first-class exploratory surface.
- It should be especially informative for comparing Newton, Explaino-FP, and future Explaino-Halley behavior.
- It is more insightful than another purely cosmetic warp variant.

## 3) Recommended Order
1. Explaino-Nova
2. Explaino-Halley
3. Explaino-DualSeed
4. Explaino-Multiplicity

## 4) Shared Guardrails
Every new Explaino family in this packet must obey the same rules:
- no automatic warp-on startup behavior
- deterministic seed semantics and diagnostics capture output
- explicit defaults on fractal-type switch and Reset All
- no silent fallback to baseline Explaino when a family-specific parameter is invalid
- joy palette remains unchanged unless the user explicitly opens a palette v2 effort

## 5) Architecture Recommendation
For the next wave, keep each new Explaino family as its own `FractalType` enum value.

Reasoning:
- that matches the existing repo model
- it keeps schema visibility simple
- it avoids adding a half-designed nested `explaino_mode` system before the family lineup stabilizes

If the Explaino roster grows beyond 6-8 stable variants, a later consolidation into `explaino_mode` can be considered.

## 6) Non-Goals
This packet does not include:
- a joy palette redesign
- orbit-trap coloring as an Explaino-specific feature
- a second procedural seed ontology unrelated to the current polynomial generator
- unrestricted freeform root editing in the viewer