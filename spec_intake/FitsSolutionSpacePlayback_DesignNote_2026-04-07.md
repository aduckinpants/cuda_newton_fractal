# FITS Solution-Space Playback — Design Note

## Summary

The runtime already has a meaningful FITS round-trip seam, and that seam is not
generic image I/O. It is anchored in the Explaino orientation-vector model,
which itself is built from the original Explaino seed system.

That suggests a stronger follow-on idea than simple FITS export/import:

- load a saved Salt FITS payload
- treat it as a solved state-space artifact, not just a static frame
- replay the represented solution space over time inside the runtime
- render the moving fractal state as a deterministic sequence

Conceptually this is close to the Jacobian-space-ship idea, but with a larger
motion surface:

- Jacobian-space-ship: move along a local Jacobian-derived flow near one point
- FITS solution-space playback: move the whole fractal state through a saved
  orientation/solution description over time

This is a whole-state transport idea, not a local linearization toy.

## Why This Is Interesting

If the FITS payload already preserves enough orientation information to round-trip
the runtime state, then it may be rich enough to drive an animation or replay
mode where the runtime "walks" the saved solution structure.

Potential value:

1. It turns a saved scientific/runtime artifact into an executable scene.
2. It creates a deterministic replay surface for Explaino-derived spaces.
3. It may provide a bridge between static archival output and exploratory viewer
   motion.
4. It gives us a new operator seam: score or analyze how a saved solved space
   evolves when replayed rather than only when re-sampled from scratch.

## Core Idea

Given a saved FITS artifact that already round-trips through Explaino orientation
vectors, define a playback mode that reconstructs a sequence of runtime states.

At a high level, each playback step would do something like:

1. Decode the FITS payload into an orientation/state representation.
2. Interpret that representation as either:
   - absolute states over time, or
   - a base state plus time-varying deltas / transport rules.
3. Apply the derived motion back onto the runtime state:
   - view center
   - rotation
   - zoom / scale
   - possibly seed / phase / drift fields if the FITS schema supports them
4. Render the resulting frame.

The key distinction is that playback should move the global fractal state, not
just perturb a local Jacobian at one coordinate.

## Good V1 Shape

The smallest useful version is probably:

- Explaino-family only at first
- offline playback from one FITS file
- deterministic time parameter `t in [0, 1]`
- apply playback to view transform first
- leave deeper parameter evolution optional until the file semantics are clearer

That gives a bounded first milestone:

"Can we load a saved FITS and drive a stable camera/state replay path from it?"

## Two Plausible Playback Models

### Model A — Camera/Frame Transport

Treat the FITS artifact as describing how to move the viewer through a solved
space.

Playback affects mainly:

- `ViewState.center`
- `ViewState.rotation_degrees`
- `ViewState.zoom` / `log2_zoom`

This is the safer first model because it preserves the current fractal formula
and mainly changes how we traverse the saved space.

### Model B — State Transport

Treat the FITS artifact as describing the evolving solved state itself.

Playback may affect:

- view transform
- Explaino combined seed
- seed drift / phase
- root spread / warp strength
- other derived Explaino controls if encoded by the FITS payload

This is more powerful, but it requires a very clear contract about what the FITS
artifact actually stores and which fields are authoritative.

## Relationship To Existing Explaino Seams

This idea naturally sits on top of existing Explaino machinery already present
in the repo:

- combined-seed representation (`ExplainoSeedCombined`, `ExplainoSeedSetCombined`)
- seed normalization and drift
- Explaino-derived polynomial reconstruction from seed/orientation-like inputs
- deterministic headless/runtime sampling surfaces

That matters because it means this is not purely speculative UI glitter. There
is already a real state model that can support deterministic replay if the FITS
payload exposes the right variables.

## Key Open Questions

These need a deeper follow-up before implementation:

1. What exactly is authoritative in the FITS round-trip payload?
   Is it image data plus metadata, or a richer solved-space representation?

2. Are the Explaino orientation vectors sufficient by themselves to reconstruct a
   time path, or do we need extra basis/delta fields?

3. Should playback move only camera/view state in V1, or also move seed/phase
   state?

4. Is the playback path best interpreted as:
   - a continuous interpolation problem,
   - a discrete frame stack,
   - or a transport field defined over the saved solution space?

5. How much of this is Explaino-specific, and how much can be generalized into a
   broader runtime playback surface later?

## Likely First Technical Slice

When we come back for the deeper dive, the right first slice is probably not UI.
It is a contract-analysis task:

1. inventory the current FITS save/load fields
2. identify exactly how Explaino orientation vectors are encoded
3. decide what subset can drive deterministic replay
4. define a minimal playback state structure
5. add a headless proof-of-concept that maps `t -> reconstructed runtime state`

Only after that should we decide whether playback belongs in:

- the viewer UI
- a headless CLI exporter
- reality-toolkit orchestration
- or all three

## Working Thesis

Yes, this looks like a legitimate product/research idea.

If the current FITS round-trip is truly built on Explaino orientation vectors,
then a saved FITS may be more like a compressed solved-space checkpoint than a
mere output image. If so, replaying that state over time could become a new
runtime mode: a whole-fractal solution-space ship rather than a local Jacobian
ship.

That is worth a deeper design pass.