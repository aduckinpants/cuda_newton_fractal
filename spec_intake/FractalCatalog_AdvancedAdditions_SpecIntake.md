# Spec Intake Packet

**Spec Title:** Advanced Fractal Catalog Additions (Nova / Phoenix / Lambda / Transcendental Newton / Non-Integer Multibrot / Lyapunov / Attractors / Rational Maps / Orbit Traps / Distance Estimator / Perturbation Deep Zoom)

**Date:** 2026-01-15

## 0) Intake Summary
This spec is a good “catalog expansion” draft but it mixes multiple tiers of complexity and (importantly) mixes **math changes** with **state-surface changes**.

Under the system-wide directive “no implicit fallback,” this needs to be staged so each addition has:
- explicit parameter surface (even if some are hard-coded for v1)
- explicit validity checks (fail fast)
- explicit coloring contract

The best approach is to treat this as a **roadmap** with a tight v1/v2 partition.

## 1) Current Architecture Constraints (as-is)
- One kernel dispatch producing RGBA.
- View mapping applies uniformly to all fractals.
- `fractal.view.fractal_type` selects fractal logic.
- `KernelParams` holds iteration and coloring params.
- No implicit fallback: invalid parameter combos must error or be made unselectable in schema.

## 2) Classification (How much change each item really needs)
### 2.1 Low-friction (fits current loop shape)
These can be implemented as additional `fractal_type` modes with small changes:

- **Nova fractals**
  - Newton-like update with explicit relaxation parameter $\alpha$.
  - **State impact:** needs a new parameter `nova_alpha` (float). Hard-coding is acceptable for v1 if documented.
  - **Risk:** numeric stability; must fail-fast on NaNs.

- **Transcendental Newton (sin/cosh/exp-1)**
  - Same Newton loop, different $f(z), f'(z)$.
  - **State impact:** needs function selection enum; can reuse existing Newton coloring.
  - **Risk:** overflow for `exp`, `cosh`; must fail-fast / cap domain explicitly (no silent clamp).

- **Rational maps (escape-time)** $z_{n+1} = P(z)/Q(z)$
  - Fits the escape-time pattern but requires explicit polynomial/rational parameterization.
  - **State impact:** coefficients for P and Q (or presets).
  - **Risk:** division by zero; must define explicit behavior (error color vs early escape).

### 2.2 Medium complexity (small extra state or different iteration shape)
- **Phoenix fractals**
  - Needs memory term $z_{n-1}$.
  - **State impact:** no new buffers, but per-pixel loop must keep `z_prev` register.
  - **Parameters:** phoenix `p` constant (complex or scalar) + maybe fixed `c`/coord; must be explicit.

- **Lambda fractals**
  - Typically logistic-map-like dynamics; depends on complex vs real interpretation.
  - **Clarification needed:** Is $z$ complex and $\lambda$ complex? Or real logistic map sampled over a plane?
  - **State impact:** parameter `lambda`.

- **Non-integer multibrot** ($z^p + c$ for non-integer $p$)
  - Requires a complex power implementation via polar form.
  - **State impact:** float `p` (not int).
  - **Risk:** branch cuts / undefined at $z=0$ for negative p; must define fail-fast rules.

### 2.3 High complexity / new ontology (not “just a formula swap”)
These change the rendering contract enough that they should be staged deliberately:

- **Lyapunov fractals**
  - Computes a Lyapunov exponent from a symbol sequence and logistic map.
  - **State impact:** sequence string (e.g., "AB"), parameters A/B ranges.
  - **Not the same substrate** as complex-plane iterations; still doable but contract differs.

- **Strange attractor fields (Clifford / De Jong)**
  - Iterated function systems / attractors; typically accumulate density over many steps.
  - **State impact:** either many iterations per pixel (no buffer) or an accumulation buffer (new buffer).
  - Must explicitly choose approach.

- **Orbit traps**
  - Needs per-iteration min distance tracking; still local, but adds a new coloring contract.
  - **State impact:** trap shape parameters.

- **Distance estimator shading**
  - Requires derivative tracking and a shading pipeline.
  - **State impact:** new shading params; new UI controls; likely new invariants.

- **Perturbation deep zoom**
  - Algorithmic shift; requires reference orbit and perturbations.
  - Often implies extra buffers / multi-pass / CPU-GPU coordination.

## 3) Required Clarifications (before implementation)
1) For each new fractal family, define:
   - escape-time vs Newton-like vs other
   - coloring mode(s) that are valid
   - default params and Reset All semantics
2) For Phoenix/Lambda/non-integer multibrot:
   - are parameters real or complex?
   - hard-coded defaults ok for v1?
3) For “sock-obliterating tier” items:
   - are we allowed to introduce new buffers/state surfaces? The earlier constraint says “no new buffers,” but some of these are best with them.

## 4) Recommended Staging Plan (Riker packet)
### Phase 1 (fast wins; no new buffers)
- Nova (with fixed alpha first, then expose alpha)
- Transcendental Newton (sin, exp-1) as presets
- Phoenix (with fixed p first)
- Non-integer multibrot only if we define explicit domain rules (avoid negative p initially)

## 4.1 Phase 1 Worklist (Implementation-Ready Units)
This converts “Phase 1” into a concrete work queue with explicit deliverables and binding surfaces.

### P1-A) Nova (first)
- **Spec intake packet:** `NovaFractal_V1_SpecIntake.md`
- **Code touchpoints (planned):** add `FractalType::nova`, add `KernelParams` fields for Nova (alpha/relaxation + optional seed), add kernel path.
- **Schema touchpoints (planned):** `fractal.view.fractal_type` add `nova`; add controls for nova params.
- **Fail-fast rule emphasis:** NaN/Inf detection must error/explicit-color; no silent clamp.

### P1-B) Transcendental Newton presets (sin / exp-1 / cosh)
- **Spec intake packet:** `TranscendentalNewtonPresets_V1_SpecIntake.md`
- **Code touchpoints (planned):** introduce Newton-function preset enum; implement `f(z), f'(z)` variants; define explicit overflow policy.
- **Schema touchpoints (planned):** Newton function selector shown only when `fractal_type=newton`.
- **Fail-fast rule emphasis:** overflow/invalid domain is explicit (error), not clamped.

### P1-C) Phoenix (escape-time with memory term)
- **Spec intake packet:** `PhoenixFractal_V1_SpecIntake.md`
- **Code touchpoints (planned):** add `FractalType::phoenix`; per-pixel loop tracks `z_prev`.
- **Schema touchpoints (planned):** expose Phoenix parameters (at minimum a preset selector; optional complex constant fields).
- **Fail-fast rule emphasis:** define explicit parameter domain for “memory term”; error if out of range.

### P1-D) Non-integer Multibrot (only if domain rules are explicit)
- **Spec intake packet:** `NonIntegerMultibrot_V1_SpecIntake.md`
- **Decision gate:** v1 starts with **p in [2,12]** (float), and bans negative/non-real behaviors; if we can’t define branch-cut semantics cleanly, we postpone.
- **Fail-fast rule emphasis:** `z=0` with unsupported p => explicit error behavior.

### Phase 1 exit criteria
- Each new fractal type has explicit presets applied on type change + Reset All.
- Schema prevents invalid coloring modes for each type (unselectable invalid states).
- Renderer returns explicit error string for invalid param domains.
- No new persistent GPU buffers beyond what’s already introduced for perturbation.

### Phase 2 (new coloring contracts, still no new buffers)
- Orbit trap coloring (trap shape presets)
- Distance estimator *lite* (if feasible without huge refactor)

### Phase 3 (algorithmic shift)
- Perturbation deep zoom
- Lyapunov
- Attractor density fields

## 5) Fail-Fast Rules (apply system-wide directive)
- Unknown enum ids => schema error.
- Invalid parameter domains (e.g., multibrot p < 2, or negative p without explicit handling) => render error (not clamped).
- If a fractal type requires parameters not present, do not infer; error or make the control unavailable.

## 6) Acceptance Criteria (for each added fractal)
- Switching type applies explicit presets for non-view params (no black-screen surprises).
- Valid coloring options are enforced by schema (unselectable invalid states).
- Kernel errors are surfaced explicitly (no silent clamp/fallback).
- Camera behavior loop and view mapping remain unchanged.
