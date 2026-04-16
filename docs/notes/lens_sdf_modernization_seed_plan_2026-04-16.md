# Lens SDF Modernization Seed Plan (Preserved)

This file preserves the earlier modernization seed so the later Phase 2 planning pass does not need to reconstruct it from chat history.

It is intentionally high-level and superseded by the evidence-backed Phase 1 report in `docs/notes/lens_and_flashlight_writeup.md`.

## Preserved Seed

- Modernize the runtime lens from a debug-only mask/SDF side path into a first-class, color-integrated viewport overlay.
- Support the full current fractal catalog with explicit family-aware mask semantics.
- Replace the dead `lens.downsample` control with real behavior or a clearer surface.
- Improve performance by removing the worst host-side full-frame work from the hot path.
- Use the SDF modernization to prepare shared substrate for the later flashlight/probe modernization.

## Preserved Implementation Axes

- explicit lens semantics authority per `FractalType`
- viewport overlay instead of aux-window-only presentation
- palette-aware or family-aware color integration
- real lens resolution control
- GPU-first or GPU-resident postprocess direction
- shared internal seam for later flashlight/probe consumers

## Preserved Follow-On Targets

- color-integrated viewport overlay design
- real control-surface behavior for lens resolution/downsample
- full current fractal catalog support
- performance architecture direction
- flashlight/probe follow-on substrate and sequencing

## Historical Note

This preserved seed exists because the earlier planning direction was useful but not sufficiently evidence-backed. Phase 2 should use the detailed report as authority and use this file only as a fallback memory aid.
