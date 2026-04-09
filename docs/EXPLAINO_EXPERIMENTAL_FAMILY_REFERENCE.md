# Explaino Experimental Family Reference

Date: 2026-04-09
Branch: `feature/explaino-joy`

This documents the "experimental explaino" fractals — variants that modify
the Newton iteration step in creative or "wrong" ways. Each one shares the
Explaino seed/warp/polynomial infrastructure but changes **how the solver
step is computed** each iteration.

All experimental variants share these common features:
- Seeded polynomial from `explaino_seed` + `explaino_seed_drift`
- Initial coordinate warp via `explaino_warp_start` (phase, strength)
- Adaptive damping: `damp = userDamp / (1 + |step|)`
- Orbit pullback: if `|z|^2 > 16`, scale z back to radius 4
- Phoenix memory: `+ phoenix_p * z_{n-1}` (optional, defaults to 0)
- Best-iteration tracking: records the iteration where `|P(z)|` was smallest
- Convergence fallback: if not converged, snap to nearest root using bestIt

---

## Shipped Variants

### explaino_joy (enum 30)

**Commit:** 046015e  
**One-liner:** Root-critical coupled Newton — blends P/P' with P'/P''.

**Formula:**
```
combined = (1 - gamma) * P/P' + gamma * P'/P''
z_{n+1} = z - damp * combined + phoenix_p * z_{n-1}
```

**Key parameter:** `joy_coupling` (gamma) in [0, 1], default 0.5  
**Derivatives needed:** P, P', P'' (second derivative)

**What it does:** At gamma=0, standard Newton targeting polynomial roots.
At gamma=1, targets critical points (where P'=0) instead. Intermediate
values target both simultaneously, producing 5-6 visible basins from a
degree-4 polynomial — more attractors than roots.

**Visual character:** "Valley + ridge" interplay. Basin boundaries become
active attractors at moderate gamma, revealing structure that standard
Newton hides.

**Theory source:** JOY_NEWTON_FRACTAL_PROPOSAL.md — fixed points of the
blended map are level sets of the Newton map derivative N_P'(z).

---

### explaino_fold (enum 31)

**Commit:** 5de47a5  
**One-liner:** Burning Ship-style abs-value folding on the Newton step.

**Formula:**
```
step_newton  = P/P'
step_folded  = (|Re(step)|, |Im(step)|)
combined     = (1 - alpha) * step_newton + alpha * step_folded
z_{n+1}      = z - damp * combined + phoenix_p * z_{n-1}
```

**Key parameter:** `fold_coupling` (alpha) in [0, 1], default 0.5  
**Derivatives needed:** P, P' (first derivative only)

**What it does:** Component-wise absolute value reflects the Newton step's
real and imaginary parts into the first quadrant. This breaks the complex
analytic structure — the step direction is "folded" like the Burning Ship
fractal folds its escape-time iteration.

**Visual character:** Asymmetric basin boundaries. At alpha=1 (pure fold),
all four quadrants of each Newton step collapse to one, creating sharp
angular features and bilateral asymmetry where standard Newton is smooth.
Looks like a "crystalline" or "shattered" version of the baseline.

**Theory source:** Burning Ship family (abs-value folding applied to a
root-finding context rather than escape-time).

---

### explaino_bell (enum 32)

**Commit:** c451bc1  
**One-liner:** Measurement-reaction decomposition of the Newton step.

**Formula:**
```
step     = P/P'
pHat     = P / |P|                             # unit vector in P direction
dotPar   = Re(step * conj(pHat))              # scalar projection
s_par    = dotPar * pHat                       # measurement channel
combined = step - beta * s_par                 # attenuate measurement
z_{n+1}  = z - damp * combined + phoenix_p * z_{n-1}
```

**Key parameter:** `bell_coupling` (beta) in [0, 1], default 0.5  
**Derivatives needed:** P, P' (first derivative only)

**What it does:** Decomposes the Newton step into two orthogonal channels:
- **Parallel to P** (measurement/bulk): the component of the step aligned
  with the polynomial value's phase direction
- **Perpendicular to P** (reaction/spin): the remaining component

Beta attenuates the measurement channel. At beta=0, full Newton. At beta=1,
only the perpendicular/reaction component drives iteration — the solver
steps purely sideways relative to the P-field direction.

**Visual character:** Spiral/vortex patterns at basin boundaries. At
moderate beta, basins develop curved, swirling edges. At beta=1, the
iteration spirals around roots instead of approaching them directly.

**Theory source:** Salticid Bell No-Snap conjecture: "A measurement is the
enforced null-spin projection of a standing-wave tangent, and the recorded
value is the reaction of the field to that constraint." P-phase parallel =
bulk/inertial (Nand-Janus mass). Perpendicular = charge/spin.

---

## Proposed Variants (Not Yet Implemented)

### explaino_ripple (proposed enum 33)

**One-liner:** Standing-wave normal perturbation — sinusoidal kick
perpendicular to the Newton step.

**Formula:**
```
step  = P/P'
n_hat = i * step / |step|                      # normal to step direction
kick  = A * sin(2*pi*n/T + arg(P'(z)))         # standing-wave term
z_{n+1} = z - damp * step + kick * n_hat + phoenix_p * z_{n-1}
```

**Key parameter:** `ripple_amplitude` (A) in [0, 0.5], default 0.15  
**Derivatives needed:** P, P' (first derivative only)

**What it does:** After computing the standard Newton step, adds a
sinusoidal perturbation perpendicular to the step direction. The phase of
the sine wave depends on iteration count and the local derivative argument,
creating standing-wave interference patterns.

**Expected visual:** Concentric ring structure overlaid on basins — like
Chladni figures on a vibrating plate. At A=0, standard Newton. At moderate
A, rings appear at basin boundaries where the kick disrupts convergence.

**Theory source:** Nand-Janus standing-wave theory — "nodes of standing
waves on the computational manifold" (N=9/K=5 nonogram constraint system).

---

### explaino_splice (proposed enum 34)

**One-liner:** Alternating-polynomial interference — two different seeded
polynomials take turns driving each iteration.

**Formula:**
```
P_A = seeded polynomial from seed s
P_B = seeded polynomial from seed s + offset
if n % 2 == 0:  step = P_A(z) / P_A'(z)
else:            step = P_B(z) / P_B'(z)
z_{n+1} = z - damp * step + phoenix_p * z_{n-1}
```

**Key parameter:** `splice_offset` in [0, 2], default 0.5  
**Derivatives needed:** P, P' (first derivative only, but for two polynomials)

**What it does:** Instead of blending two polynomials (that is explaino_dual),
this alternates which one drives each iteration. Even iterations solve toward
P_A's roots; odd iterations solve toward P_B's roots. The two root landscapes
compete per-iteration, creating interference fringes.

**Expected visual:** Moire-like interference between two basin topologies.
At offset=0, P_A=P_B so standard Newton. At moderate offset, "beat patterns"
appear where the two topologies constructively/destructively interfere.

**Theory source:** Nand-Janus standing-wave theory (two counter-propagating
wave systems) + FITS trajectory invariance (temporal alternation reveals
hidden structure).

---

### explaino_vortex (proposed enum 35)

**One-liner:** Self-referential spin — the Newton step is rotated by its
own argument angle.

**Formula:**
```
step     = P/P'
theta    = arg(step)                            # angle of current step
rotation = exp(i * V * theta)                   # self-referential rotation
z_{n+1}  = z - damp * step * rotation + phoenix_p * z_{n-1}
```

**Key parameter:** `vortex_strength` (V) in [0, 1], default 0.3  
**Derivatives needed:** P, P' (first derivative only)

**What it does:** Rotates the Newton step by a fraction of its own angle.
This is self-referential — the step "leans into turns." Steps that already
point sideways get rotated further sideways; steps aimed directly at a root
get rotated off-target.

**Expected visual:** Spiral/swirl patterns at basin boundaries, like fluid
dynamics. At V=0, standard Newton. At V=0.5, moderate curling. At V=1,
every step is rotated by its full angle — maximally self-referential.

**Theory source:** Mass-charge-spin scratch (theory_nand_janus_mass_charge_spin_scratch.md)
— "spin = offset driving the solver." Bell theory (reaction as angular
projection). The vortex is the angular complement to Bell's linear
decomposition.

---

### explaino_tension (proposed enum 36)

**One-liner:** Root competition field — weak gravitational pull toward
competitor roots fattens basin boundaries.

**Formula:**
```
step    = P/P'
r_near  = closest_root(z)                       # convergence target
r_far   = second_closest_root(z)                # competitor
pull    = T * (r_far - z) / |r_far - z|^2       # inverse-distance gravity
z_{n+1} = z - damp * step + pull + phoenix_p * z_{n-1}
```

**Key parameter:** `tension_strength` (T) in [0, 0.1], default 0.02  
**Derivatives needed:** P, P' (first derivative only) + root positions

**What it does:** After computing the Newton step, adds a weak pull toward
the second-closest root. This creates a tug-of-war at basin boundaries:
the solver wants to converge to the nearest root, but the tension term
pulls it toward the competitor.

**Expected visual:** Wide, structured border regions instead of hairline
basin edges. At T=0, standard Newton. At small T, boundaries fatten into
"contested territories." Reveals the geometric skeleton of root competition.

**Theory source:** Bell No-Snap theory ("tensile" connection conjecture,
measurement as constraint on standing-wave tangent). The 6+1 hypothesis
paper (basin boundary as active structure, not passive edge).

---

## Shared Architecture

All experimental explaino fractals:
1. Live in `fractal_renderer.cu` as `else if (ft == FractalType::explaino_*)` blocks
2. Have matching host probe runner paths in `fractal_probe_runner.cpp`
3. Have headless continuity tests in `ui_app/tests/test_explaino_*_continuity.cpp`
4. Use enum values in `fractal_types.h` (30+)
5. Are wired into all string/enum mapping files (cli_args, diagnostics_capture,
   diagnostics_state_io, finding_archive_actions, safe_mode_schema, schema_binding)
6. Have UI schema entries (dropdown + parameter slider + visible_if lists)
7. Default to `joy_basins` coloring mode and 500 max iterations

## Parameter Summary

| Variant | Key Param | Range | Default | Derivatives |
|---------|-----------|-------|---------|-------------|
| joy | joy_coupling | [0, 1] | 0.5 | P, P', P'' |
| fold | fold_coupling | [0, 1] | 0.5 | P, P' |
| bell | bell_coupling | [0, 1] | 0.5 | P, P' |
| ripple | ripple_amplitude | [0, 0.5] | 0.15 | P, P' |
| splice | splice_offset | [0, 2] | 0.5 | P, P' (x2) |
| vortex | vortex_strength | [0, 1] | 0.3 | P, P' |
| tension | tension_strength | [0, 0.1] | 0.02 | P, P' + roots |
