# Deferred Note: Capture-Only Supersampling And Color Tuning

Status: deferred rendering/export quality work. Do not implement under current SDF or sidecar slices.

## Summary

The user-provided review for the `2026-06-10/090737_424__perpendicular_burning_ship` capture argues that the image is not primarily limited by missing anti-aliasing. Anti-aliasing would likely soften thin high-contrast filaments and subpixel shimmer, but the dominant visual harshness is from the fractal signal and Color Pipeline shaping: smooth-escape coloring plus repeated color-shape bands on a large `4096x2560` `float64` capture.

The near-term conclusion is:

1. Per-family color and smooth-escape tuning likely buys more for this specific frame.
2. Capture-only supersampling is feasible and useful for final findings.
3. Live viewer AA should be last because it fights the realtime/pacing goal.

## Product Boundary

Capture-only supersampling should be a finding/export-quality setting, not a live exploration feature.

The default live viewer should remain single-sample or otherwise realtime-first unless a later measured slice proves a cheap live AA path. Capture AA should be explicit, bounded, and reflected in capture metadata.

## Candidate Capture-Only Design

Add an export-quality mode for Capture Finding / diagnostic capture:

- `aa_mode`: `off`, `ssaa_2x2`, possibly `ssaa_3x3` later.
- Render at higher internal resolution.
- Downsample to requested final capture dimensions.
- Save the final frame as the normal capture image.
- Record AA mode and internal render dimensions in `state.json`, `finding.json`, and `fractal-state.json`.

For v1, prefer simple 2x2 SSAA:

- Easy to reason about.
- Roughly 4x fractal work.
- This keeps a 0.5 second capture plausibly around 2 seconds before postprocess overhead.

Avoid 4x4 as the default:

- Roughly 16x sample count.
- Likely too expensive for high-resolution `float64` finding captures.

## Color Tuning Direction

For the cited capture, color tuning may matter more than AA:

- repeated palette/shape bands can dominate perceived harshness;
- broad cyan/gold/brown bands are not stair-step artifacts;
- AA does not recover hidden detail from repeated color shaping;
- per-family defaults can choose gentler repeat frequency, palette saturation, contrast, or smoothstep ranges.

Future color work should stay separate from AA implementation:

- one slice for capture-quality SSAA mechanics;
- one slice for per-family color/smooth-escape tuning;
- one comparison harness that can show A/B captures without mixing the causes.

## Contradictions And Show-Stoppers

- AA is not a universal "make it pretty" switch. It will not fix banding created by Color Pipeline repeat/mirror/posterize shaping.
- Live viewer AA is in tension with the realtime exploration priority, especially for `float64` and high-resolution captures.
- SSAA multiplies render cost and may multiply SDF postprocess cost unless the postprocess is carefully staged.
- Capture/replay parity can be broken if AA settings affect pixels but are not serialized.
- Downsampling color space matters. Naive downsample in the wrong space can shift the look.
- Auto max-iterations and precision selection must remain capture-authoritative; AA should not accidentally change the sampled view or family defaults.
- If AA is applied after palette/grading rather than before, the result differs from multisampling the fractal signal itself. That needs an explicit decision.

## Future Slice Shape

Suggested order:

1. Capture-quality documentation/spec.
   - Decide whether SSAA samples full fractal+color per subpixel or downsamples final RGBA only.
   - Define metadata fields.
2. Capture-only SSAA 2x2.
   - No live viewer UI toggle.
   - Small fixture tests plus one no-mouse capture proof.
3. A/B report tool.
   - Same state, AA off/on, hashes and timing.
   - Store comparison thumbnails or metrics.
4. Per-family color tuning.
   - Use capture comparisons to decide whether tuning, not AA, is the better fix.
5. Live AA feasibility only after the above.

## Proof Gates For Any Future Implementation

- Native tests prove AA settings serialize/load/default cleanly.
- Capture Finding proof shows AA mode changes final pixels while preserving view/camera/aspect.
- `fractal-state.json` records AA mode and internal render dimensions.
- Timing report records AA cost.
- Full-quality capture remains deterministic for the same state and AA mode.
- Live viewer pacing tests stay unchanged unless a dedicated live-AA slice is opened.

