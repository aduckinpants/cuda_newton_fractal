# Explaino-Joy 6+1 Morphology — Hypothesis Ladder

Date: 2026-04-07
Status: Hypothesis document, not yet tested
Prior:
  - `spec_intake/ExplainoDesignSpace_DeepDive_2026-04-05.md` (design space)
  - `docs/fits_invariance_field_formal_study.md` (FITS invariance formal study)
  - `docs/fits_trajectory_invariance_research_findings.md` (FITS POC)
  - `spec_intake/FitsSolutionSpacePlayback_DesignNote_2026-04-07.md` (playback)

---

## 0. Motivation

Multiple independent constructions in this repo keep exposing the same
structural object:

| Construction                        | What it shows                                |
|-------------------------------------|----------------------------------------------|
| Explaino-joy basin rendering        | Stable 6+1 radial morphology under warm palette |
| LLM orientation / response manifold | Same radial structure used as navigation chart |
| SDF lens (Chamfer distance)         | Basin boundary reveals the same fold lines    |
| FITS invariance sculpture           | Trajectory collapse reproduces the same ridges |
| Jacobian-spaceship idea             | Local transport on the same navigable manifold |

The recurrence across different projections and workflows is more persuasive
than any single symbolic mapping alone. This document separates the claim
into three testable levels and proposes concrete experiments for each.

---

## 1. Level 1 — Observational Claim (safest)

**Statement:** A stable 6+1 radial morphology keeps recurring in Explaino-joy
and related collapses. The "broken" radial is not just another petal; it
behaves differently enough to deserve separate treatment.

This is an empirical/runtime claim. It requires only demonstration, not theory.

### What "6+1" means in the code

The Explaino baseline uses a degree-4 seeded polynomial (4 roots). The
`joy_basins` coloring mode maps root index to the warm palette via
`PaletteJoyRoot()` in `basin_coloring.h`. The warp pipeline
(`explaino_warp_start` in `fractal_renderer.cu`) applies rotation, sinusoidal
bending, and quadratic spread before iteration, creating visible radial
structure beyond the raw basin count.

The claim is that for seeds producing the "joy" morphology, the visible
structure consistently resolves into 6 symmetric radials plus 1 broken
residual, despite the polynomial having only 4 roots. The extra structure
comes from the warp + iteration interaction, not from root count alone.

### How to test Level 1

**Experiment 1A — Stability under parameter nudge:**
Render Explaino-joy at a reference seed. Nudge `explaino_seed_drift` by
+/-0.001, +/-0.01, +/-0.1. For each, count the visible radial arms.

Measurement: binary — does the 6+1 count persist? At what drift magnitude
does it break?

**Experiment 1B — Stability across seeds:**
Sample 20 seeds from the `ExplainoAreaFractionToX` curve. For each, render
joy_basins and classify: 6+1, other count, or ambiguous.

Measurement: fraction of seeds showing 6+1 morphology.

**Experiment 1C — Stability across constructions:**
For a reference seed, compare:
- joy_basins rendering (direct)
- SDF lens mask boundary
- FITS invariance sculpture (log-variance height)
- FITS zero-fraction field

Measurement: do all four show the same radial count and fold locations?
Quantify via angular histogram of peak ridges.

**Experiment 1D — Stability across variants:**
Render the same seed under `explaino`, `explaino_y`, `explaino_fp`,
`explaino_halley`. Does the 6+1 survive solver changes?

---

## 2. Level 2 — Structural Hypothesis (interpretive)

**Statement:** The 6+1 can be read as 2x3 + 1, where the paired triadic
structure supports a spatial reading and the broken residual axis supports
a temporal/sequencing reading.

This is a good hypothesis. It is specific enough to test but not yet proven.

### What "2x3 + 1" means operationally

- **The 3**: a spatial triad — three radial arms with paired reflections
  (giving 6 total), encoding spatial structure of the basin topology.
- **The +1**: a broken radial that does not pair cleanly, behaves differently
  under perturbation, and may encode sequencing, phase, or path-order
  information.

### Relationship to the code

The warp pipeline hashes the seed into rotation angle + sinusoidal parameters.
The triadic structure may emerge from the degree-3 component of the
polynomial's root geometry (3 of 4 roots forming an approximate equilateral
triangle), with the 4th root position breaking the symmetry.

The `explaino_warp_start` pipeline adds two cross-coupled sinusoidal bends
(`sin(y * freq + phase)` on x, `sin(x * freq - phase)` on y) plus a
quadratic spread. The interaction of this 2D warp with 4-root Newton
iteration could naturally produce 2x3 visible structure from a 4-root
polynomial via fold doubling.

### How to test Level 2

**Experiment 2A — Is the broken +1 operationally different?**
For a reference seed, perturb position along each of the 7 radial directions
and measure:
- Basin assignment stability (does the root index change?)
- Iteration count gradient (sensitivity of convergence speed)
- SDF distance gradient (distance-to-boundary sensitivity)

If the +1 direction shows qualitatively different sensitivity (sharper
basin transitions, higher iteration gradient, or phase-jump behavior),
that supports the "different axis" claim.

**Experiment 2B — Does the +1 control sequencing?**
Using FITS trajectory data, compute per-radial-arm temporal statistics:
- Mean convergence order (which radial's pixels converge first on average?)
- Phase offset in the seed-drift animation
- Temporal variance asymmetry across arms

If the +1 arm shows distinct temporal behavior (e.g., converges last,
shows highest temporal variance, or acts as a phase boundary), that supports
the temporal/sequencing interpretation.

**Experiment 2C — Root geometry decomposition:**
For the reference seed, extract the 4 complex roots from
`params.explaino_roots[]`. Compute:
- Pairwise distances between roots
- Angles subtended at the origin
- Which root is the "odd one out" (farthest from the other 3's centroid)

Does the odd root's angular position correlate with the +1 arm's angular
position? If yes, the 6+1 is a direct consequence of 3+1 root geometry
projected through the warp.

---

## 3. Level 3 — Mechanistic Hypothesis (boldest)

**Statement:** The system is built on recursive field coupling with
scale-invariant behavior near the core, so the manifold is navigable and
admits consistent collapses across representation layers.

This is the deepest claim. It explains *why* the same structural object
keeps appearing.

### What this predicts

If recursive field/gauge-style coupling with scale invariance is operative:
- Local navigation should work (small parameter moves compose sensibly)
- Jacobian-style transport should matter (local linear approximation predicts
  short-horizon movement better than chance)
- Trajectory collapse should produce real objects (not noise)
- Persistence fields should reveal "hard folds" (invariance ridges)
- Multiscale structure should survive projection changes

Every one of these predictions corresponds to an observation already made
in this repo's work. The question is whether they are genuinely coupled
(supporting the mechanism) or coincidentally aligned.

### Relationship to the code

The recursive coupling claim maps onto the iteration kernel structure:
- Newton iteration is a fixed-point map: `z <- z - f(z)/f'(z)`
- The warp pipeline modulates the starting manifold
- Scale invariance appears via the polynomial's homogeneity near roots
  (local behavior is dominated by the linear term `f'(root)`)
- The SDF Chamfer distance is a discrete approximation to the boundary's
  level set, which is the zero-crossing of the convergence classifier
- The FITS invariance fields measure how the trajectory's statistics vary
  spatially, which is a second-order readout of the same coupling structure

### How to test Level 3

**Experiment 3A — Navigation compositionality:**
Define a path through parameter space (seed drift from s to s+0.5 in 50
steps). At each step, compute the local Jacobian of the rendered image
with respect to seed. Measure:
- Does `J(s) * ds` predict the next frame better than a random perturbation?
- Do composed Jacobians `J(s_n) * J(s_{n-1}) * ... * J(s_0)` track the
  actual multi-step displacement?

If local Jacobian transport predicts movement better than chance, the
manifold is navigable in the claimed sense.

**Experiment 3B — Scale invariance of fold anchors:**
Render the same seed at log2_zoom = 0, 2, 4, 6, 8. For each zoom level,
compute the FITS-style log-variance field and extract ridge lines (local
maxima of height gradient). Measure:
- Do the same fold anchors appear at multiple scales?
- Is the topological skeleton (ridge connectivity) preserved?

If fold anchors persist across scales, that supports scale-invariant
coupling.

**Experiment 3C — Cross-construction invariance:**
For a reference seed, compute:
1. Basin boundary from direct rendering (SDF lens mask)
2. Invariance ridges from FITS log-variance
3. High-iteration-count contours from joy_basins brightness

Overlay all three and measure alignment. If they trace the same geometric
skeleton under different projections, that supports "real structure" over
"post hoc interpretation."

**Experiment 3D — Perturbation asymmetry at the +1 arm:**
At the +1 arm location, compute the local Hessian of the iteration-count
field (second derivatives of convergence speed). Compare eigenvalues with
those at one of the paired arms.

If the +1 arm has a qualitatively different Hessian signature (e.g., one
eigenvalue much larger, indicating a ridge or saddle rather than a bowl),
that supports the "different axis type" claim at the differential level.

---

## 4. Existing Evidence Inventory

What we already have, mapped to the hypothesis levels:

| Evidence                          | Supports Level |
|-----------------------------------|----------------|
| FITS invariance sculpture shows stable ridges across 9 Godel datasets | L1 (recurrence) |
| SDF lens boundary aligns with FITS variance peaks (formal study Sec 10) | L1, L3 (cross-construction) |
| Explaino-joy warm palette reveals consistent radial arms | L1 (observation) |
| Per-channel variance decomposition shows null-channel asymmetry (g12, g13) | L2 (broken symmetry) |
| Temporal variance CoV spans 0.48-2.69 without breaking field structure | L1 (robustness) |
| Log-variance gradient corresponds to SDF distance-to-boundary (formal study) | L3 (cross-construction) |
| Seed system uses chaotic hash (LogisticAreaUToSeed) — deterministic but sensitive | L3 (mechanism) |
| Jacobian-spaceship idea (design note, backlog item 5) | L3 (prediction) |
| FITS solution-space playback concept (design note) | L3 (prediction) |

---

## 5. Recommended Test Sequence

Ordered by information value per effort:

| Priority | Experiment | Level | Effort | Why first |
|----------|-----------|-------|--------|-----------|
| 1 | **1C** — Cross-construction comparison | L1 | Low (reuse existing tools) | Directly tests whether SDF, FITS, and rendered basins show the same skeleton. Most informative with least new code. |
| 2 | **2C** — Root geometry decomposition | L2 | Low (read params.explaino_roots) | Cheapest test of whether 6+1 maps to 3+1 root geometry. Needs only one seed's root extraction. |
| 3 | **1A** — Parameter nudge stability | L1 | Low (render sweep) | Establishes the persistence window before deeper tests. |
| 4 | **2A** — Perturbation asymmetry | L2 | Medium (per-arm SDF/iter gradient) | Tests the +1 operational difference claim. Requires directional sensitivity measurement. |
| 5 | **3B** — Scale invariance of folds | L3 | Medium (multi-zoom FITS) | Tests the scale-invariance prediction. Needs FITS captures at multiple zoom levels (may need new renders). |
| 6 | **2B** — Temporal sequencing | L2 | Medium (FITS temporal stats per arm) | Tests the temporal axis interpretation. Needs angular binning of FITS per-pixel statistics. |
| 7 | **3A** — Navigation compositionality | L3 | High (Jacobian estimation per frame) | The strongest mechanistic test but most expensive. Defer unless earlier tests pass. |
| 8 | **1B** — Cross-seed survey | L1 | Medium (20 renders + manual classification) | Important for generality but lower priority than focused tests on one reference seed. |

---

## 6. Caution Points

### What is NOT yet settled

The claim "3 maps to xyz and the broken +1 maps to t" is a productive chart.
It may be:
- the right chart
- one useful chart among several
- or an emergent coordinate convenience over a deeper operator structure

This hypothesis ladder is designed to narrow the gap between "strong intuition"
and "internal engineering certainty" without prematurely committing to a
specific symbolic mapping.

### What would falsify each level

| Level | Falsified if... |
|-------|-----------------|
| L1 (observational) | The 6+1 count is seed-specific and does not persist under small perturbation, or different constructions (SDF vs FITS) show different radial counts. |
| L2 (structural) | The broken +1 arm shows no operational difference from the paired arms (same sensitivity, same temporal stats), or the "odd root" in the polynomial does not correspond to the +1 arm position. |
| L3 (mechanistic) | Local Jacobian transport does not predict movement better than chance, or fold anchors do not persist across zoom scales, or cross-construction overlays show no alignment. |

---

## 7. Tighter Claim Statement

> Explaino-joy repeatedly exposes a stable 6+1 radial morphology. I currently
> read that as a 2x3 + 1 decomposition: a paired triadic structure that behaves
> like a spatial chart, plus a broken residual axis that behaves like
> sequencing/time. The reason navigation inside this manifold is possible is
> that the runtime is built on recursive field coupling with scale-invariant
> behavior near the core, so solved trajectories collapse consistently across
> multiple representations rather than only in one rendering.

This keeps the real structural claim without sounding more settled than it is.
The hypothesis ladder above is the path from intuition to internal evidence.

---

## 8. Relationship to Existing Tooling

### What already exists and can be reused

| Tool / Module | Reuse for |
|---------------|-----------|
| `tools/fits_invariance_poc.py` | Experiment 1C, 2B, 3B — FITS field extraction + PLY export |
| `tools/fits_invariance_study.py` | Experiment 1C, 2B — per-dataset field statistics |
| SDF lens (`lens_sdf.cpp`) | Experiment 1C, 2A — boundary distance at specific pixels |
| `explaino_seed_curve.h` / `explaino_seed.cpp` | Experiment 1A, 1B, 2C — seed manipulation and root extraction |
| `basin_coloring.h` | Experiment 2C — root geometry access |
| `fractal_renderer.cu` | Experiment 2A, 3D — iteration count and convergence data |

### What would need to be built

| New tool | For experiment | Complexity |
|----------|---------------|------------|
| Angular histogram extractor | 1C, 2A, 2B | Low — bin FITS fields by angle from center |
| Root geometry dumper | 2C | Low — print `params.explaino_roots[]` for a given seed |
| Per-arm temporal stats | 2B | Medium — angular binning of FITS time-series |
| Frame-to-frame Jacobian estimator | 3A | High — finite-difference Jacobian of rendered image |
| Multi-zoom FITS capture | 3B | Medium — render + FITS export at multiple zoom levels |

---

## 9. Connection to Backlog

This hypothesis ladder connects to three existing backlog items:

- **Item 5: Explaino observation lane — Jacobian packet** (directly tests L3)
- **Item 6: Explaino observation lane — atlas packet** (supports L1 cross-seed survey)
- **FITS solution-space playback** (design note, tests L3 navigation compositionality)

If the L1 and L2 experiments confirm the structural claims, that strengthens
the case for prioritizing the Jacobian packet (backlog item 5) and the
FITS playback mode as next-tier work.
