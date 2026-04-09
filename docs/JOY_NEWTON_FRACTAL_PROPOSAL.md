# Joy Newton: Proposal for a Root-Critical Coupled Newton Family

## Proposal Summary

**Joy Newton** is a proposed fractal iteration that couples Newton's method on
a polynomial P with Newton's method on its derivative P', creating a
continuously parameterized family whose fixed points differ from those of
either method alone. The coupling strength gamma deforms the system between
standard Newton (gamma=0, targeting roots of P) and derivative-Newton (gamma=1,
targeting critical points of P). Intermediate gamma values produce a combined
fixed-point equation with up to 6 solutions for degree-4 polynomials.

This is intended as the mathematical realization of the -Joy operator concept:
the "return path" (critical points define where basin boundaries originate)
meets the "forward path" (roots are the attractors). The iteration targets
structure related to its own basin geometry.

**Novelty status**: Strong candidate pending narrower literature verification.
Newton maps are a mature area of complex dynamics with existing work on
modified variants (memory terms, relaxation, etc.). The specific
root-critical coupling described here has not surfaced in our searches, but
categorical novelty claims require deeper review.

---

## 1. The Core Iteration

### 1.1 Base Formula

For polynomial P(z) with first derivative P'(z) and second derivative P''(z):

$$z_{n+1} = z_n - \lambda\!\left[(1-\gamma)\frac{P(z_n)}{P'(z_n)} + \gamma\frac{P'(z_n)}{P''(z_n)}\right]$$

| Symbol | Name | Range | Role |
|--------|------|-------|------|
| lambda | Damping | (0, 2] | Standard Newton relaxation |
| gamma | Joy coupling | [0, 1] | Root <-> critical point interpolation |

- At **gamma = 0**: standard damped Newton on P. Fixed points = roots of P
- At **gamma = 1**: Newton on P'. Fixed points = critical points of P (zeros of P')
- At **0 < gamma < 1**: coupled system with emergent fixed points

### 1.2 Full Explaino-Joy Formula (with Phoenix memory)

$$z_{n+1} = z_n - \lambda\!\left[(1-\gamma)\frac{P(z_n)}{P'(z_n)} + \gamma\frac{P'(z_n)}{P''(z_n)}\right] + \mu \cdot z_{n-1}$$

| Symbol | Name | Range | Role |
|--------|------|-------|------|
| mu | Phoenix memory | complex, |mu| < 1 | Temporal self-reference (z_{n-1} feedback) |

---

## 2. Fixed-Point Analysis

### 2.1 What Are the Fixed Points?

At a fixed point z* of the base iteration (Section 1.1):

$$(1-\gamma)\frac{P(z^*)}{P'(z^*)} + \gamma\frac{P'(z^*)}{P''(z^*)} = 0$$

Multiplying through by P'(z*) P''(z*) (assuming non-degenerate):

$$(1-\gamma)\, P(z^*)\, P''(z^*) \;+\; \gamma\, [P'(z^*)]^2 = 0$$

### 2.2 Degree-4 Polynomial: Fixed-Point Count Upper Bound

For P(z) = c_4 z^4 + c_3 z^3 + c_2 z^2 + c_1 z + c_0:

- P(z) P''(z): degree 4+2 = degree 6
- [P'(z)]^2: degree 3 squared = degree 6
- Combined equation F_gamma(z) = (1-gamma) P P'' + gamma (P')^2: degree 6

| gamma | Equation | Fixed-point count upper bound |
|-------|----------|-------------------------------|
| 0 | P(z) = 0 | 4 (roots of P) |
| 1 | P'(z) = 0 | 3 (critical points of P) |
| 0 < gamma < 1 | F_gamma(z) = 0 | up to 6 |

Whether all 6 are attracting (vs. repelling or neutral) depends on local
stability analysis at each fixed point. The transition from 4 fixed points
(gamma=0) to 3 (gamma=1) involves bifurcations as gamma varies.

### 2.3 The gamma = 1/2 Special Case

At gamma = 1/2:

$$P(z)\,P''(z) + [P'(z)]^2 = 0$$

Note that:

$$\frac{d}{dz}[P(z)\,P'(z)] = [P'(z)]^2 + P(z)\,P''(z)$$

So the fixed points at gamma = 1/2 are exactly the **critical points of the
product P(z) P'(z)** — the points where the product of the function and its
derivative achieves extrema.

### 2.4 Connection to Newton Map Derivative

The derivative of Newton's map N_P(z) = z - P(z)/P'(z) is:

$$N_P'(z) = \frac{P(z)\,P''(z)}{[P'(z)]^2}$$

Our fixed-point equation F_gamma(z) = 0 can be rewritten as:

$$N_P'(z^*) = -\frac{\gamma}{1-\gamma}$$

**The Joy Newton fixed points are points where Newton's map has a specific
derivative value determined by gamma.** This is a level-set condition on the
Jacobian of Newton's method.

| gamma | N_P'(z*) | Interpretation |
|-------|----------|----------------|
| 0 | 0 | Superattracting fixed points (roots of P) |
| 0.1 | -1/9 | Near-root with small repulsion |
| 0.5 | -1 | Parabolic-like (derivative = -1) |
| 0.9 | -9 | Strongly repelling in Newton |
| 1 | -inf | Critical points (P' = 0) |

**Interesting direction**: The coupled map targets points determined by a
level-set condition on the Newton map derivative, which may produce
boundary-related attractors not present in standard Newton. Whether this
constitutes a formal Fatou-Julia interpolation requires further analysis.

---

## 3. Novelty Assessment

### 3.1 Literature Search Vectors (Pending Verification)

1. **"coupled Newton critical point iteration"**
2. **"Newton fractal derivative polynomial coupling"**
3. **"interpolation root-finding critical point attractor"**
4. **"level set Newton map derivative fractal"**
5. **"Dynamics of Newton maps"** — Cambridge paper (Ergodic Theory, 2018) covers modified Newton dynamics broadly

### 3.2 What Exists vs. What This Adds

| Known method | Uses P'' | Fixed points = roots of P? | Fixed points = critical points of P? |
|---|---|---|---|
| Newton | No | Yes | No |
| Halley | Yes (acceleration) | Yes | No |
| Schroeder | Yes (acceleration) | Yes | No |
| Householder | Yes (higher-order) | Yes | No |
| **Joy Newton** | **Yes (coupling)** | **Yes (gamma=0)** | **Yes (gamma=1)** |

All known higher-order root-finding methods (Halley, Schroeder, Householder)
use P'' to **accelerate convergence to roots of P**. Their fixed points are
always the roots of P. Joy Newton uses P'' to create a different fixed-point
set.

### 3.3 Candidate Novelty Claims (To Be Verified)

For 0 < gamma < 1, the Joy Newton iteration has:

1. A **different fixed-point count upper bound** than either Newton or derivative-Newton (up to 6 for degree 4)
2. **Fixed points that coincide with level sets of N_P'(z)**, a structure not targeted by standard Newton variants
3. At gamma=1/2, fixed points are critical points of P(z)P'(z)
4. Basin boundaries that **bifurcate** as gamma varies

---

## 4. Connection to the Explaino / Joy / Carl Framework

### 4.1 Joy Pairing (Semantic Dual)

In Newton fractal dynamics:
- **Forward path**: Newton iteration finds roots of P (basin interiors)
- **Return path**: Critical points of P define where basin boundaries originate

Joy Newton pairs these: gamma controls how much the iteration targets the
structure that defines its own boundaries. At gamma = 0, the iteration is
pure root-finding. At gamma > 0, it also targets boundary-generating structure.

### 4.2 Helmholtz / Radiance Connection

The Newton flow field divergence is:

$$\nabla \cdot \mathbf{v} = -(1 - N_P'(z))$$

Joy Newton fixed points satisfy N_P'(z) = -gamma/(1-gamma). At gamma = 1/2
this gives N_P'(z) = -1, so div(v) = -(1-(-1)) = -2 (not zero — correcting
an error in the earlier draft). The divergence-free condition N_P'(z) = 1
corresponds to gamma = -1, which is outside the [0,1] range. The connection
to incompressible flow is therefore not as direct as initially claimed, but
the level-set structure itself remains interesting.

---

## 5. Implementation Plan

### 5.1 Infrastructure Already Available

| Component | Status | File |
|---|---|---|
| P, P', P'' evaluation | DONE | polynomial_eval_real_coeffs.h: PolyEvalRealCoeffsDeg4D2 |
| Seed-parameterized roots | DONE | fractal_derived_fields.cpp: ExplainoShapeForSeed |
| WedgeTween interpolation | DONE | explaino_seed_curve.h |
| Phoenix memory pattern | DONE | fractal_renderer.cu: explaino_phoenix |
| Momentum pattern | DONE | fractal_renderer.cu: explaino_inertial |
| Float32 + Float64 paths | DONE | All explaino modes have dual-precision |
| Warp start function | DONE | explaino_warp_start / _d |

### 5.2 New Components Needed

| Component | Effort | Description |
|---|---|---|
| FractalType::explaino_joy | Trivial | New enum value |
| KernelParams::joy_coupling | Trivial | New float field, default 0.0 |
| Kernel iteration | Small | ~50 lines, mirrors explaino_phoenix structure |
| P'' guard | Trivial | Guard |P''|^2 < 1e-30 (fall back to pure Newton term) |
| UI binding | Small | Schema binding for joy_coupling slider |
| CLI/probe support | Small | ParseFractalType + probe runner entry |
| Headless test | Medium | Continuity test (small gamma delta -> small fixed-point delta) |

### 5.3 Kernel Pseudocode (Float32 Path)

```cpp
z = explaino_warp_start(coord, seed, phase, strength);
Cx zPrev = z;

for (int it = 0; it < maxIter; ++it) {
    Cx P, dP, d2P;
    poly_eval_real_coeffs_deg4_d2(coeffs, z, &P, &dP, &d2P);

    float pAbs = cx_abs(P);
    if (pAbs < eps) break;

    // Standard Newton term: P(z)/P'(z)
    float dAbs2 = cx_abs2(dP);
    if (dAbs2 < 1e-20f) break;
    Cx newtonStep = cx_div(P, dP);

    // Joy dual term: P'(z)/P''(z)
    Cx joyStep = {0.0f, 0.0f};
    float d2Abs2 = cx_abs2(d2P);
    if (d2Abs2 > 1e-20f) {
        joyStep = cx_div(dP, d2P);
    }

    // Combined step: (1-gamma)*Newton + gamma*JoyDual
    float oneMinusGamma = 1.0f - joyCoupling;
    Cx combinedStep = cx_add(
        cx_scale(newtonStep, oneMinusGamma),
        cx_scale(joyStep, joyCoupling));

    // Apply with damping and Phoenix memory
    Cx zNext = cx_add(
        cx_sub(z, cx_scale(combinedStep, userDamp)),
        cx_mul(pConst, zPrev));

    zPrev = z;
    z = zNext;

    if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
}
```

### 5.4 Test Strategy (TDD)

1. **Continuity test**: At gamma=0, verify output matches explaino exactly
2. **Continuity test**: Small gamma increment (0.0 -> 0.01) produces small fixed-point shift
3. **Fixed-point count test**: At gamma=0, converges to 4 roots. At gamma=1, converges to 3 critical points
4. **Degenerate guard test**: When P'' = 0 at some z, iteration does not diverge
5. **Seed tween**: Verify smooth behavior across integer seed boundaries at gamma=0.3
