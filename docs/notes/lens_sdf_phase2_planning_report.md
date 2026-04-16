# Lens SDF / Flashlight Phase 2 Planning Report

This document converts the Phase 1 technical report into a planning/reporting surface for the later implementation plan.

It does not prescribe code edits line-by-line. Instead it locks the planning direction, identifies the most defensible architectural moves, and narrows the implementation space so the final Phase 3 plan can be decision-complete without redoing the research.

## 1. Executive Direction

The Phase 1 report supports five strong conclusions:

1. The runtime lens already has full shipped-catalog classification coverage, so the modernization problem is not "add enum support first."
2. The current runtime lens is architecturally a debug side path, not a first-class overlay.
3. The current control surface is misleading because `fractal.lens.downsample` is user-visible but functionally dead.
4. Basin-family lens semantics are useful but partially hidden because the real runtime rule is synthetic root parity rather than literal convergence.
5. The runtime lens and the current Explaino-sidecar "flashlight" are still separate systems and need shared substrate before broader probe/guidance work can feel coherent.

That means the modernization effort should be framed as:

- a runtime lens productization pass
- plus a semantics-extraction pass
- plus a later flashlight/probe reuse pass

Not as a narrow SDF math cleanup.

## 2. Recommended Modernization Goals

### 2.1 Runtime Lens Goals

The runtime lens should become:

- color-integrated with the main viewport
- explicit in semantics per fractal family
- performance-aware
- controlled by a truthful UI surface

The runtime lens should stop being:

- aux-window-only
- grayscale-by-default
- full-resolution host recompute by construction
- partially defined by hidden renderer exceptions

### 2.2 Flashlight / Probe Goals

The later flashlight/probe work should reuse:

- the same family semantics authority
- the same notion of what boundary or partition is being visualized
- the same classification vocabulary for "what this lens means"

It should not inherit:

- the old ambiguity where `lens` means both image-space SDF and parameter-space guidance with no formal bridge

## 3. Recommended Architecture Direction

### 3.1 Introduce One Lens Semantics Authority

The implementation plan should center on one explicit lens semantics layer that answers, per `FractalType`:

- is the type lensable
- what family it belongs to for lens purposes
- what "inside" means
- whether the partition is literal or synthetic
- what explanatory label should be attached to that semantics

Minimum semantics buckets implied by Phase 1:

- basin partition via synthetic root parity
- escape-time bounded/not-escaped partition
- explicit unsupported bucket for future additions if not yet classified

This authority should become the runtime source of truth rather than splitting truth between:

- `LensMaskInsideForFractal(...)`
- renderer-side basin override logic

### 3.2 Separate Semantics From Presentation

The final design should split:

- semantics: what region or partition is being measured
- field generation: how the binary mask / SDF / distance band is computed
- presentation: how it is composited, colored, or exposed to other tools

This matters because the current code conflates:

- a useful synthetic basin boundary
- a debug-style grayscale SDF presentation

The synthetic semantics are not the problem. The product surface around them is.

### 3.3 Make the Viewport Overlay the Primary Surface

Recommended default direction:

- main viewport overlay becomes the primary lens experience
- aux windows remain optional diagnostics

This preserves debugging value without forcing users into disconnected side windows for the main feature.

The viewport overlay should be boundary-first, not full-screen grayscale replacement.

Meaning:

- preserve the fractal image as the main visual payload
- add boundary/field cues over it
- use opacity and blend controls rather than replacing the rendered image with a second flat image

## 4. Recommended Control-Surface Direction

### 4.1 Keep `enabled`, Replace the Lie

`fractal.lens.enabled` is real and should survive.

`fractal.lens.downsample` should not survive unchanged unless it becomes real behavior.

The Phase 3 implementation plan should choose one of two honest options:

1. keep `downsample`, but wire it into actual mask/SDF resolution behavior
2. replace it with a clearer control such as `resolution_scale` or `overlay_quality`

The control surface should also likely add:

- overlay opacity
- overlay mode
- optional debug visibility toggles for aux surfaces

### 4.2 Recommended Overlay Modes

The planning direction that best fits current evidence is a small bounded mode set:

- `boundary`
  - emphasize zero-crossing / contour only
- `band`
  - show a narrow signed-distance band around the partition
- `field_debug`
  - keep a fuller field-style diagnostic view for development

This keeps the user-facing surface intentional while preserving diagnostic depth.

## 5. Recommended Performance Direction

### 5.1 What Must Change

The current hot path pays for:

- full-res mask copy to host
- host-side SDF generation
- host-side RGBA conversion
- extra texture upload

The implementation plan should treat that as unacceptable for the final product path.

### 5.2 Preferred Direction

The most defensible planning direction is:

- compute the lens field at a lens-specific resolution
- avoid full-resolution host-side SDF recomputation as the normal path
- move toward GPU-first or GPU-resident postprocess generation

This does not require the Phase 3 plan to commit to the exact shader/kernel split yet, but it does mean the final plan should reject any "just keep CPU chamfer and overlay it" endpoint.

### 5.3 Transitional Allowance

If the implementation needs a temporary transitional seam:

- keep CPU SDF generation only as a testable or compatibility seam
- do not let it remain the default steady-state overlay path

## 6. Recommended Color-Integration Direction

### 6.1 Preserve Fractal Color as Ground Truth

The fractal image should remain the base layer.

The overlay should communicate lens structure without destroying:

- basin palette cues
- escape-time banding
- exposure and grading choices already made by the renderer

### 6.2 Treat Overlay Color as Semantic, Not Decorative

Recommended principles:

- boundary color should communicate "lens feature" distinctly from the base fractal color
- overlay intensity should be controlled by opacity and distance band, not by replacing the whole viewport with gray
- field-debug mode can remain more artificial, but the default surface should feel like a real viewer tool

### 6.3 Avoid Over-Coupling to Existing Coloring Modes

The overlay should be compatible with the existing coloring modes, but it should not force separate per-mode overlay code unless Phase 3 evidence shows that is necessary.

Planning default:

- one overlay system that composites over any current base coloring mode
- no bespoke overlay implementation per palette mode in the first modernization pass

## 7. Flashlight / Probe Follow-On Strategy

### 7.1 What Phase 2 Should Lock

The later flashlight/probe work should be sequenced after the runtime lens semantics and presentation extraction, not before.

Reason:

- right now the probe/guidance story does not share a formal semantics contract with the runtime lens
- building broader flashlight/probe behavior first would likely duplicate or fork the same family logic again

### 7.2 Shared Substrate To Expose

The Phase 3 implementation plan should assume a reusable internal surface that can answer:

- which lens semantics apply to the current fractal
- whether the partition is literal or synthetic
- what explanatory label to show to users or tooling
- what mask/SDF mode is being visualized

That gives the future flashlight/probe report or implementation a stable substrate without requiring it to consume the entire rendering pipeline.

### 7.3 Explaino Sidecar Boundary

The current Explaino sidecar should stay behaviorally separate during the runtime lens modernization pass.

Meaning:

- do not try to unify the sidecar controller logic with the runtime overlay in the same implementation wave
- do extract shared semantics vocabulary so the future follow-on can speak about both systems consistently

## 8. Recommended Implementation Sequencing

The later Phase 3 implementation plan should likely sequence work as:

1. extract and centralize lens semantics authority
2. wire truthful lens control-surface behavior
3. add viewport overlay presentation while preserving aux diagnostics
4. reduce host-side overhead / introduce lower-cost field generation path
5. add broader flashlight/probe reuse seam

This ordering is safer than starting with presentation polish because it avoids redoing UI work around unstable semantics and performance foundations.

## 9. Validation and Reporting Strategy For Phase 3

The implementation plan should require proof in three categories:

### 9.1 Semantics Proof

- every shipped `FractalType` maps to an explicit lens semantics entry
- basin synthetic partition behavior is intentional and documented
- no future type silently falls into a false fallback

### 9.2 Presentation Proof

- overlay renders in the main viewport
- aux diagnostics remain available when enabled
- current base fractal color remains visible under the overlay

### 9.3 Performance Proof

- no unnecessary lens work when disabled
- real resolution-control behavior when enabled
- measurable reduction in post-render host work compared with the current design

## 10. Decisions Phase 3 Still Needs To Finalize

This report intentionally narrows the space without finishing every decision.

The final implementation plan still needs to choose:

- exact public control names for the lens resolution/quality surface
- exact overlay mode list and defaults
- exact GPU-vs-CPU field generation boundary
- exact first-pass scope for the flashlight/probe follow-on seam

## 11. Bottom Line

Phase 1 established that the lens is broader than its old design reputation but still architecturally immature.

Phase 2 therefore recommends a modernization path with four explicit commitments:

- make semantics explicit
- make the control surface truthful
- make the viewport overlay primary
- make the hot path cheaper

And one explicit sequencing rule:

- do not attempt broad flashlight/probe unification until the runtime lens semantics substrate exists.
