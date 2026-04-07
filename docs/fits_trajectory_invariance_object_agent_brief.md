# FITS trajectory invariance object — agent brief

## Purpose

Investigate whether a FITS trajectory can be transformed from a frame-by-frame sequence into a **single 3D object** that captures the stable structure of the underlying fractal program over time.

This is not a request to make a prettier movie.
This is a request to identify and encode **persistent structure under transport**.

The motivating idea is:

> Given a solved trajectory `F(x, y, t)`, what parts of the fractal remain stable, persistent, or coherently transformed over the whole voyage, and how can that be frozen into one navigable 3D artifact?

The desired end result is something closer to an **astro-stack plate / ghost sculpture / invariance hull** than a replay.

## Framing

Treat the FITS trajectory first as a time-indexed solved field:

- `F(x, y, t)` = scalar or multichannel solved state over image plane and tick index

The core research question is not “what are the three axes?” in the abstract.
The core research question is:

- What invariance or persistence fields can be derived from the trajectory?
- Which derived fields produce meaningful 3D geometry?
- Which constructions are practical and informative inside the current Salticid/FITS toolchain?

The initial goal is **not** to solve the most abstract version.
The initial goal is to build a disciplined ladder of constructions and determine which one produces the first serious result.

## Key concept: invariance under trajectory

We want a field over the 2D fractal plane that expresses how much each location survives the program.

Candidate meanings of “invariant” include:

1. **Exact persistence**
   - the local state remains exactly unchanged across ticks
2. **Tolerance persistence**
   - the local state changes only within a small epsilon band
3. **Qualitative persistence**
   - the local state remains in the same qualitative class
   - examples: same inside/outside state, same basin class, same band membership
4. **Coherent transport**
   - the local state changes, but in a structured way consistent with the program rather than chaotic drift

The first two are easiest to implement.
The third is likely more semantically interesting.
The fourth is the eventual richer direction.

## Core candidate derived fields

The agent should evaluate the following invariant/persistence fields on one or more trajectories.

### A. Temporal variance field

`V(x, y) = Var_t(F(x, y, t))`

Interpretation:
- low variance = stable / persistent
- high variance = churn / transient structure

Useful transform:
- `S_var(x, y) = 1 - normalized_variance`
- or `S_log(x, y) = -log(eps + V(x, y))`

### B. Delta-zero / delta-small persistence field

Using the delta stack directly, for each pixel compute:

- exact-zero fraction across steps
- epsilon-small fraction across steps

Examples:
- `S_zero(x, y) = fraction_t(|ΔF| == 0)`
- `S_eps(x, y) = fraction_t(|ΔF| < eps)`

This is likely the most direct first experiment because it uses the data product already available.

### C. Occupancy / membership field

Choose a predicate on the solved state and accumulate persistence of membership.

Examples:
- inside/outside set
- above threshold
- within a band
- in a chosen basin class

`O(x, y) = mean_t(1[predicate(F(x, y, t))])`

Interpretation:
- high occupancy = location persistently belongs to that structure/class
- low occupancy = intermittent or unstable membership

### D. Feature persistence field

Run a feature transform first, then compute persistence.

Examples:
- edge map persistence
- ridge/contour persistence
- boundary-distance persistence
- basin-ID persistence if available

This may better reveal “where the math folds” than raw pixel intensity does.

### E. Coherent-transport field (later)

A more advanced stage should attempt to distinguish:
- rigid/coherent movement
- deformation
- flicker / instability

Possible approaches:
- optical-flow-like correspondence on derived fields
- local cross-correlation neighborhoods over time
- structure-tensor style motion coherence
- feature matching in basin/edge space rather than raw intensity

This is not required for V1, but the agent should discuss feasibility.

## 3D object constructions to compare

The agent should not assume one “correct” 3D object exists.
Instead, it should build and compare a ladder of constructions.

### Construction 1 — Heightfield invariance sculpture (V1 recommended)

Base domain:
- `(x, y)`

Height:
- one chosen persistence/stability field, such as `S_zero`, `S_eps`, `S_var`, or `O`

Optional appearance:
- color by mean intensity, entropy, band class, or another auxiliary field
- opacity by occupancy or confidence

Why this is the recommended V1:
- simplest to implement
- easiest to inspect visually
- directly matches the “ghost plate with hard structure where the math folds” intuition
- easy to export as mesh or height map

### Construction 2 — Full `(x, y, t)` volume + isosurface

Treat the full trajectory or a derived field as a 3D scalar volume.

Examples:
- raw value volume
- thresholded membership volume
- delta magnitude volume
- feature-presence volume

Then extract isosurfaces / shells / projections.

Why this matters:
- produces literal “fractal spacetime fossil” geometry
- preserves more temporal structure
- could reveal tunnels / shells / folds that disappear in collapsed statistics

Risk:
- may be visually dense and harder to interpret than the heightfield version

### Construction 3 — Density / opacity ghost stack

Build a translucent volume where density or opacity is driven by occupancy/persistence.

Interpretation:
- hard bright structure = persistent geometry
- wispy regions = transient or intermittent structure

Why this matters:
- closest to the “astro stack average plate” intuition
- good for qualitative inspection and volumetric rendering

### Construction 4 — Lifted reduced-state trajectory object (later)

Instead of working in raw image coordinates, reduce the state trajectory into a lower-dimensional embedding and build an object there.

Possible bases:
- PCA of frame states
- PCA of orientation vectors
- reduced basis of Explaino state
- a chosen manifold coordinate system if one becomes justified

Possible object forms:
- polyline/tube through reduced space
- ribbon with thickness from local delta magnitude or entropy
- occupancy hull in reduced coordinates

This is promising but explicitly later-stage.
Do not start here unless the simpler constructions fail to produce meaningful signal.

## Recommended implementation ladder

The agent should treat this as a staged research line.

### Phase 1 — Data audit and assumptions

1. Identify the exact data products available from the current FITS trajectory format.
2. Confirm whether the trajectory contains:
   - raw frame values
   - deltas only
   - enough metadata to reconstruct absolute frames deterministically
   - single-channel or multichannel state
3. Confirm which trajectories are most suitable as first test cases.
4. Prefer one or two known highly coherent trajectories first.

Deliverable:
- short audit note of what can be computed immediately without changing the save format.

### Phase 2 — Derived-field prototypes

Implement computation for at least these first:
- temporal variance field
- exact-zero fraction field
- epsilon-small fraction field
- one occupancy field based on a simple threshold or class predicate

Deliverables:
- per-trajectory scalar maps
- PNG/FITS/debug artifacts for inspection
- summary stats per derived field

### Phase 3 — V1 3D object build

Build a first serious 3D object using the most informative of the Phase-2 fields.

Required outputs:
- heightfield mesh or equivalent
- preview renders from several viewpoints
- overlay images showing the source 2D field and the generated height/opacity map
- notes on whether the object captures persistent structure meaningfully

### Phase 4 — Compare alternate constructions

Compare at least:
- heightfield invariance sculpture
- occupancy/density ghost stack
- one `(x, y, t)` isosurface attempt

Do not overbuild.
The goal is to determine which family should become the serious follow-up path.

### Phase 5 — Evaluation and recommendation

Judge the constructions using explicit criteria.

## Evaluation criteria

The agent should evaluate each construction against the following.

### Signal quality

- Does it produce visible hard structure rather than mush?
- Does it preserve meaningful distinctions between persistent and transient regions?
- Does it appear to reflect real structure in the source trajectory rather than visualization artifacts?

### Interpretability

- Can a human explain what the object is showing?
- Can slices/projections back to 2D be related to the source frames?
- Does the object support useful inspection rather than only looking impressive?

### Robustness

- Does the object remain qualitatively stable under small threshold/isovalue changes?
- Does it survive resolution changes reasonably?
- Is it overly sensitive to normalization choices?

### Toolchain practicality

- Can it be produced inside the current repo/tooling with modest additions?
- Can it be exported to simple formats such as `.ply`, `.obj`, or stable image artifacts?
- Does it scale to larger trajectories without exploding memory/time?

### Semantic relevance

- Does the object actually speak to “invariance under transport”?
- Or is it only a decorative remapping of the trajectory?

## Suggested first concrete experiments

The agent should start with one simple, one moderate, and one richer experiment.

### Experiment A — Exact-zero invariance sculpture

Input:
- one FITS delta trajectory

Derived field:
- `S_zero(x, y)`

Object:
- heightfield mesh with height = `S_zero`
- optional color = mean frame intensity

Why:
- trivial to compute
- directly grounded in delta-stack semantics
- strongest match to the current idea of “what persisted through the voyage?”

### Experiment B — Epsilon-persistence + occupancy plate

Input:
- same trajectory

Derived fields:
- `S_eps(x, y)`
- `O(x, y)` for one simple predicate

Object:
- height = `S_eps`
- opacity or secondary color = `O`

Why:
- begins separating exact invariance from qualitative persistence

### Experiment C — Full volume membership shell

Input:
- reconstructed `F(x, y, t)`

Derived field:
- binary or soft membership volume over `(x, y, t)`

Object:
- isosurface or volumetric render

Why:
- tests whether the full spacetime object has more interesting fold/tunnel structure than the collapsed plate

## Questions the agent must answer

1. Which derived field best captures persistent structure for the chosen trajectories?
2. Is the best first 3D object a heightfield, a volume shell, or a density stack?
3. Which normalization/threshold choices matter most?
4. Can the current FITS format support this directly, or does it need a new helper/export pass?
5. Which benchmark trajectories should become canonical for this line of work?
6. Is there enough signal to justify a later reduced-state / Explaino-space lift?

## Constraints

The agent must respect these constraints.

- Do not turn this into a vague “maybe PCA maybe manifolds maybe art” brainstorm.
- Ground every recommendation in current repo/tooling reality.
- Prefer the simplest construction that meaningfully answers the invariance question.
- Do not assume the correct 3D axes are known a priori.
- Separate “easy now” from “interesting later.”
- Produce real implementation guidance, not only conceptual commentary.

## Required deliverables

The agent should produce:

1. A concise framing summary
2. A data/tooling feasibility note
3. A derived-field design note
4. A recommended V1 construction with implementation steps
5. One or two alternate constructions for comparison
6. A proposed file/artifact/output schema
7. A benchmark trajectory recommendation
8. Open questions / risks
9. A prioritized next-step task list

## One-sentence directive

Take the current FITS trajectory idea and turn it into a disciplined implementation proposal for building a single 3D invariance object that captures persistent fractal structure under transport, starting from practical derived fields over `F(x, y, t)` and recommending the best first construction for serious follow-up.