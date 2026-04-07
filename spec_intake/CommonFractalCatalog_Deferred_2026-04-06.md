# Research Packet

**Title:** Common Fractal Catalog Expansion + Two-Layer Dropdown

**Date:** 2026-04-06

**Status:** deferred planning source, not implementation-ready spec

## 0) Summary

The user wants the next catalog thread to do four things at once:
- make the fractal chooser easier to navigate with a 2-layer organization
- switch the startup default to baseline `explaino` with `joy_basins` coloring
- add a new wave of recognizable/common fractals that are still safe on the current substrate
- follow that bounded wave with CUDA refactor work before the renderer hits a structural break-wall

This note records the recommended guardrails so that the next slice stays small and does not accidentally mix a UI reorg, a catalog wave, an IFS experiment, and a 3D renderer rewrite into one task.

## 1) Important Clarification: "Explaino Joy"

The existing codebase already has the desired surface:
- fractal type: `explaino`
- coloring mode: `joy_basins`

Recommendation:
- change the future startup default to baseline `explaino` plus `joy_basins`
- do **not** create a separate `explaino_joy` fractal enum unless there is a later reason to freeze a full preset bundle as its own top-level catalog item

That keeps the runtime ontology clean and avoids inventing a second name for an existing pairing.

## 2) Current Architecture Constraints

The current renderer is still fundamentally:
- a 2D single-pass CUDA image generator
- one fractal-type switch with shared view mapping
- schema-driven UI visibility and binding rules
- no existing 3D raymarch / distance-estimator / lighting / normal pipeline

That means the safest next catalog wave is still another bounded set of 2D families that fit the existing local-register iteration model.

## 3) Recommended 2-Layer Dropdown Direction

The next UI reorg should be a category-plus-entry chooser, not a giant flat list.

Recommended top-level categories:
- Common
- Explaino
- Root-Finding
- Escape-Time
- Research / Advanced

Recommended near-term mapping:
- Common: Explaino, Mandelbrot, Julia, Burning Ship, Nova, Phoenix
- Explaino: explaino, explaino_y, explaino_fp, explaino_dual, explaino_nova, explaino_halley, explaino_collatz, explaino_lambda, explaino_rational, explaino_rational_escape
- Root-Finding: Newton, Halley
- Escape-Time: Mandelbrot, Julia, Burning Ship, Multibrot, Multicorn, Phoenix, Lambda, McMullen
- Research / Advanced: only items that are shipped but not intended as default discovery surfaces

Important rule:
- this should still have one canonical runtime `fractal_type`; the 2-layer UI is an organization surface, not a second source of truth

## 4) Safe Next Wave on the Current 2D Substrate

Recommended candidates for one bounded "common fractals" wave:

### 4.1 Spider
- visually recognizable and common in fractal browsers
- still a 2D escape-time family
- fits the existing single-pass per-pixel iteration model

### 4.2 Celtic Mandelbrot
- low-ceremony extension of the current Mandelbrot-side substrate
- keeps the camera and iteration controls familiar
- broadens the catalog with a recognizable variation rather than another obscure research branch

### 4.3 Perpendicular Burning Ship
- recognizable and visually distinct from classic Burning Ship
- close enough to the current escape-time path to remain a bounded addition

### 4.4 One Magnet family, preset-driven
- only if the parameter surface is kept explicit and narrow
- do not turn this into a generic rational-map free-for-all in the same slice

Wave-size rule:
- choose at most 2-4 additions from this list, not all possible variants at once

## 5) Requested Families That Need a Different Contract

These are real user interests, but they should not be mixed into the next bounded 2D wave.

### 5.1 Gaskets / Carpets / Apollonian-style sets
- Sierpinski gasket
- Sierpinski carpet
- Apollonian gasket

Reason to defer:
- they want either an IFS / subdivision / geometry contract or a dedicated non-standard iteration ontology
- they are not just another branch in the existing complex-plane formula switch

### 5.2 Sponges / Mandelbulb
- Menger sponge
- Mandelbulb

Reason to defer:
- these are 3D distance-estimator / raymarch style threads
- they imply new camera semantics, normals, shading, and likely a new performance model
- they should only start after a deliberate renderer/refactor decision, not as "just one more fractal"

## 6) Suggested Sequence

### Slice A — UI organization only
- categorized 2-layer chooser
- startup default changes to baseline `explaino` + `joy_basins`
- no new math yet

### Slice B — one bounded common-fractal wave
- pick 2-4 safe 2D additions from the list above
- keep schema and defaults explicit
- do not mix in 3D or IFS work

### Slice C — CUDA refactor immediately after the wave
- extract common family helpers
- reduce kernel-switch sprawl
- clean up type-specific defaults and validation seams

### Slice D — only then revisit gasket / sponge / Mandelbulb ambitions

## 7) Refactor Trigger

The user explicitly wants a refactor after the next wave so the renderer does not hit a painful break wall later.

That means the safe promise is:
- one bounded catalog wave first
- then refactor the CUDA catalog seams
- then decide whether IFS or 3D work is still worth the structural cost

## 8) Non-Goals For The Next Slice

The next catalog slice should not try to do all of the following at once:
- 2-layer chooser rework
- common 2D additions
- Explaino deep-dive observation work
- gasket / IFS support
- Mandelbulb / Menger sponge / 3D DE work
- major CUDA refactor

That is too much ontology change for one pauseable slice.