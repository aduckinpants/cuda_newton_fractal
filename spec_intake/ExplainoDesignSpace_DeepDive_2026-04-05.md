# Research Packet

**Title:** Explaino Design Space Deep Dive

**Date:** 2026-04-05

**Status:** planning and research source, not implementation-ready spec

## 0) Executive Summary

The important shift is this:

Explaino is not just one fractal with a few variants.

It is a **design space** built from multiple axes that can be recombined:
- seeded root/topology generation
- pre-iteration warp / deformation
- update law / solver dynamics
- memory / feedback terms
- temporal continuation / transport law
- observation layer / coloring / metrics
- embedding / boundary geometry
- live operator coupling through Salticid and the broker-viewer flow

That means names like `Explaino-Nova`, `Explaino-Halley`, `Explaino-Phoenix`, and `Explaino-Transcendental` are only one slice of the space. There are equally important Explaino opportunities in topology, observation, and operator-bridge work.

## 1) What Already Exists

### 1.1 Shipped in the fractal engine clone
- `explaino` — seeded-warp Newton basin family
- `explaino_fp` — damped / fixed-point biased variant
- `explaino_y` — feedback-heavy / re-warped variant

### 1.2 Reviewed in `nine`
- legacy Explaino LUT colormap research is now mature
- the key result is that the legacy LUT is a scalar wedge/CDF/trig-lift seam, not the same thing as `joy_basins`
- the broker viewer already carries the legacy Explaino LUT path

### 1.3 Already present in Salticid operator space
- `seed_explaino`
- `explaino_tick`
- `explaino_joy_field`
- `time_curve_wedge_v1`

### 1.4 Already present in the agent/tooling workflow
- broker probe sweep patterns in `nine`
- Salticid CLI tools designed for agent-driven probing
- the `ndepend-salt` / static-audit tooling lane for bounded structure review

So some of the best next work is not inventing from nothing. It is aligning these already-existing seams and deciding what belongs in the fractal engine versus the operator/runtime/toolkit side.

## 2) Three Explaino Surfaces We Must Not Confuse

### 2.1 Explaino fractal family
This is the seeded procedural polynomial + warp + iteration family in the CUDA renderer.

### 2.2 Explaino legacy LUT colormap
This is the reviewed wedge/CDF/trig-lift scalar-to-RGB map discussed in `nine`.

### 2.3 Explaino operator family in Salticid
This is the live operator/runtime surface (`seed_explaino`, `explaino_tick`, `explaino_joy_field`) that can already be probed through CLI tools and broker flows.

The legacy LUT should likely become an **observation mode / colormap mode**, not a new fractal type.

## 3) The Real Design Axes

### Axis A — Seed / topology ontology
This is about how the root set or driving field is constructed before iteration starts.

Candidates:
- single scalar seed (current)
- dual-seed interpolation
- clustered / repeated roots
- constrained symmetry packs
- operator-driven seed bundles (for example the 13-channel bundle work already discussed in `nine`)

This axis is high value because it changes the geometric skeleton of the family, not just the coloring.

### Axis B — Update law / solver dynamics
This is the most obvious axis, and the one people usually mean when they say `Explaino-*`.

Candidates:
- Explaino-Newton (current baseline)
- Explaino-FP (current)
- Explaino-Y (current)
- Explaino-Nova
- Explaino-Halley
- Explaino-Phoenix
- Explaino-Transcendental Newton (`sin`, `exp-1`, `cosh` presets)
- Explaino-Rational
- Explaino-relaxed / alpha-controlled Newton
- Explaino-inertial / heavy-ball / previous-step memory

### Axis C — Memory / coupling term
This is separate from solver law and deserves its own category.

Candidates:
- none (current baseline)
- previous-z memory (Phoenix-like)
- previous-step memory (momentum)
- periodic snap / attractor bias (related to `explaino_fp`)
- live external field coupling from operator/runtime state

### Axis D — Observation layer
This is where the newly reviewed `nine` colormap work matters.

Candidates:
- `joy_basins` (current signature mode)
- legacy Explaino LUT colormap port
- Jacobian determinant / fold map
- step magnitude / condition-number heatmap
- orbit-trap overlays
- autocorrelation / texture-scale overlays
- convergence confidence / stability maps

This axis is critical because a large part of Explaino's value is interpretability, not just raw formula novelty.

### Axis E — Runtime coupling
This is about how Explaino is driven, explored, and validated live.

Candidates:
- standalone clone viewer only
- shared `nine` broker/viewer if session lifecycle is smooth
- Salticid CLI sweeps first, fractal engine second
- side-by-side operator/runtime view and fractal-engine view

### Axis F — Temporal continuation / transport law
This is about how Explaino moves, not just how it looks in a still frame.

Candidates:
- phase sweep only
- seed drift only
- seed tween policy
- dual-seed interpolation mode
- chirality-preserving continuation
- chirality-flipping continuation
- orbit-follow / anchored-pivot continuation

This matters because a large part of Explaino's identity shows up in motion and continuation behavior, not just static morphology.

### Axis G — Embedding / boundary geometry
This is not just camera framing. It changes the morphology class by changing the domain or boundary condition the family lives in.

Candidates:
- full plane
- circular / disk domain
- strip / wall / corridor domain
- cavity / capsule / standing-wave domain
- masked or constrained domains

This gives us a clean way to talk about plane Explaino, wall-city Explaino, cavity Explaino, or bounded standing-wave Explaino without pretending they are only solver variants.

## 4) Two Roadmap Lanes

The planning should now be explicitly split into two lanes.

### Lane A — Observation / instrumentation
These are not automatically new `FractalType` values.

#### 4.1 Explaino-LUT
Not a fractal type. A colormap/observation seam.

Why it matters:
- `nine` just finished the hard review work here
- it is likely portable
- it gives the fractal engine a second Explaino-native observation mode besides `joy_basins`
- it is the cleanest bridge between the operator/toolkit world and the viewer-host world

#### 4.2 Explaino-Jacobian
Treat Jacobian/condition structure as a first-class observation surface.

Why it matters:
- directly tied to the existing warp math
- useful for understanding where the family is creating folds or new boundaries
- likely high leverage once the engine has a stable observation lane beyond color alone

#### 4.3 Explaino-Atlas mode
A metric/analysis surface rather than a single picture.

Possible outputs:
- seed landscape atlas
- phase-strength heatmaps
- boundary density / autocorrelation summaries
- continuity/smoothness reports

Why it matters:
- it turns Explaino into a measurable research surface, not just an image generator
- it lines up well with the existing reality-toolkit and `ndepend-salt` investigation style

### Lane B — Solver / topology family growth
These are the families that are good candidates for future `Explaino-*` implementation work.

#### 4.4 Explaino-Nova
Seeded procedural topology with escape-time hybrid behavior.

Why it matters:
- strongest bridge between the repaired Nova work and the Explaino family
- likely to produce genuinely different morphology rather than just recoloring the same basins

#### 4.5 Explaino-Halley
Same seeded roots, higher-order solver.

Why it matters:
- excellent apples-to-apples comparison against baseline Explaino
- likely to reveal new convergence islands, boundary sharpness, and failure geometry

#### 4.6 Explaino-DualSeed
Two stable seed sources with an explicit mix parameter.

Why it matters:
- gives a controlled design surface instead of just neighboring drift
- useful for sweeps, presets, and future reality-toolkit dataset studies

#### 4.7 Explaino-Multiplicity
Clustered or repeated roots.

Why it matters:
- exposes solver pathologies and sensitivity directly
- especially useful for comparing Newton, FP, and Halley variants

#### 4.8 Explaino-Phoenix
Add previous-z memory to the seeded topology.

Why it matters:
- moves Explaino into recurrence-driven escape-time behavior
- likely very visually distinct

#### 4.9 Explaino-Transcendental
Use seeded topology but replace polynomial root-finding with transcendental function presets.

Why it matters:
- broadens Explaino beyond polynomial-only behavior
- should connect cleanly to the already-specced transcendental Newton line

#### 4.10 Explaino-Warp-Driven Julia
This is a bridge family where the Julia-style escape-time contract becomes the baseline and the Explaino warp/topology machinery drives the field.

Why it matters:
- it is a clear named bridge to an external family that deserves an explicit slot
- likely earlier and clearer than jumping straight to larger multi-axis hybrids

#### 4.11 Explaino-relaxed / alpha
Expose solver damping / relaxation as a first-class family seam.

Why it matters:
- cheap to add conceptually
- may reveal large visual changes with small math changes

#### 4.12 Explaino-inertial
Add previous-step momentum instead of previous-z recurrence.

Why it matters:
- more controlled than full Phoenix memory
- probably easier to compare directly against baseline Explaino and Explaino-Y

#### 4.13 Explaino-Rational
Seeded procedural topology with rational-map update laws.

Why it matters:
- pushes the family into disconnected dust / web / pole-driven geometries
- likely very rich, but riskier

#### 4.14 Explaino-OperatorBridge
Use Salticid operator outputs or field bundles to drive seeds, roots, or warp fields live.

Why it matters:
- this is where the broker/CLI/toolkit ecosystem becomes scientifically interesting
- it turns Explaino into a view onto live operator state, not just a standalone renderer

#### 4.15 Capstone hybrids
Examples:
- Newton-Warp-Phoenix tri-hybrids
- Halley-Warp memory hybrids
- multi-axis solver + topology + embedding combinations

Why they matter:
- they are likely rich
- but they should be treated as explicitly second-order work after the single-axis baselines are understood

## 5) What We Can Already Probe Before New CUDA Work

### In `nine`
- legacy Explaino LUT smoothness and seam behavior
- broker-driven Explaino smoothness sweeps
- current viewer-side LUT behavior

### In Salticid mainline
- `seed_explaino`
- `explaino_tick`
- `explaino_joy_field`
- the large existing corpus of `.salt` demos and falsification tests around warp, power, boundary scaling, and composition
- agent-friendly CLI tooling intended for probe-first exploration
- `ndepend-salt` / structural audit surfaces for bounded follow-up classification

This is important: many Explaino questions should be probed in operator/CLI land before becoming CUDA implementation work.

## 6) Candidate Packet Template

Before implementation, each named candidate should answer the same bounded packet questions:

1. What axis is actually changing?
2. What baseline stays fixed?
3. What morphology delta do we expect?
4. What observation modes are required to evaluate it?
5. What Salticid / CLI probes can test it before CUDA work?
6. What are the known failure modes / kill criteria?

And every candidate packet should include a short boundedness/recovery contract:
- bailout / termination rule
- singularity / branch handling
- safe parameter envelope
- failure phenotype examples
- recovery policy when the family walks into unstable wall-city / fold / singular regimes

## 7) Named External-Family Bridges / Pending Slots

These names are worth keeping visible even when they are not first-wave work.

- Explaino-McMullen
- Explaino-Lambda
- Explaino-Multicorn
- Explaino-Noninteger
- Explaino-Warp-Driven Julia
- Newton-Warp-Phoenix tri-hybrids

Guidance:
- McMullen / Lambda / Multicorn / Noninteger / Warp-Driven Julia should sit as named bridge slots
- the tri-hybrids should be treated as capstone hybrids, not first-wave baselines

## 8) Recommended Research/Spec Order

### Observation / instrumentation lane
1. Explaino-LUT port packet
2. Explaino-Jacobian
3. Explaino-Atlas

### Family lane
1. Explaino-Nova
2. Explaino-Halley
3. Explaino-DualSeed
4. Explaino-Multiplicity
5. Explaino-Phoenix
6. Explaino-Transcendental
7. Explaino-Warp-Driven Julia
8. Explaino-OperatorBridge
9. Capstone hybrids

That order balances:
- already-reviewed research
- visual distinctness
- scientific value
- implementation tractability

The rule is explicit now:
- do the single-axis baselines first
- do the cross-axis hybrids later

## 9) Naming Guidance

The clean naming rule is:
- use `Explaino-*` when the seeded/topology ontology stays central and the change is in solver/update/memory/observation/embedding
- do not call every observation mode a new fractal type

Examples:
- `Explaino-Nova` — yes, fractal family
- `Explaino-Halley` — yes, fractal family
- `Explaino-LUT` — likely a colormap / view mode, not a `FractalType`
- `Explaino-Jacobian` — likely an overlay or analysis mode, not a `FractalType`

## 10) High-Level Takeaway

The strongest next insight is that Explaino should be treated as a **matrix**, not a linear list.

The matrix dimensions are:
- topology
- solver law
- memory term
- temporal continuation
- observation mode
- embedding / boundary geometry
- runtime coupling

If we respect that matrix, the family can grow in a way that is both scientifically interesting and implementation-bounded.

If we ignore it, we will tend to pile unrelated concepts into one overloaded `Explaino` bucket and lose clarity fast.