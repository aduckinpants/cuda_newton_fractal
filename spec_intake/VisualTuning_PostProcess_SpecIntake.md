# Visual Tuning / Post-Process Filters — Spec Intake

Status: spec-only (do not implement yet)

## Rationale

The current color pipeline (exposure, saturation, contrast, tint) covers basic
grading but lacks spatial/frequency-domain filters that are common in fractal
exploration tools. Adding a small set of image-space post-processes would
significantly improve visual tuning capability without changing the fractal
math.

## Proposed Filters

### 1. Sharpen (Unsharp Mask)

- Standard 3x3 or 5x5 unsharp mask applied to the final RGBA output.
- Parameter: `sharpen_amount` (float, 0.0 = off, 1.0 = strong).
- Runs in a separate CUDA kernel pass on the d_rgba buffer after the main
  render kernel.
- Cheap; single-pass convolution.

### 2. Sobel Edge Detection

- Computes gradient magnitude from the iteration-count or mask channel.
- Output: overlay or blend onto the fractal image.
- Parameters: `sobel_strength` (float), `sobel_blend` (float, 0 = off, 1 = full overlay).
- Could produce an "edge glow" effect that highlights basin boundaries.

### 3. Absolute Difference Filter (AbsDiff)

- Compares adjacent frames (temporal) or adjacent pixels (spatial) and outputs
  the absolute difference.
- Useful for motion/change detection during seed scrubbing.
- Parameter: `absdiff_mode` enum: `off`, `spatial`, `temporal`.
- Temporal mode requires a previous-frame buffer (one extra RGBA allocation).

### 4. Bloom / Glow

- Simple box-blur of bright pixels added back to the image.
- Parameters: `bloom_threshold` (float), `bloom_intensity` (float), `bloom_radius` (int).
- Two-pass separable blur (horizontal then vertical) for efficiency.

### 5. Gamma Correction

- Per-channel gamma curve applied at the end of the pipeline.
- Parameter: `gamma` (float, default 1.0, range 0.1-3.0).
- Trivial per-pixel operation.

## Implementation Notes

- All filters should be CUDA kernel post-passes on the d_rgba buffer.
- Each filter gets a checkbox enable + parameter sliders via schema.
- Pipeline order: render -> sharpen -> sobel overlay -> bloom -> gamma -> upload.
- Add to a new `post_process.cu` module; do not grow fractal_renderer.cu.
- Schema panel: add a "Post-Process" panel (order 26, between Color and Render).
- Binding paths: `fractal.postprocess.*`.

## Schema Sketch

```json
{
  "id": "postprocess",
  "label": "Post-Process",
  "order": 26,
  "controls": [
    { "id": "sharpen_amount", "type": "slider_float", "label": "Sharpen", "min": 0.0, "max": 2.0, "default": 0.0 },
    { "id": "sobel_strength", "type": "slider_float", "label": "Sobel Edges", "min": 0.0, "max": 1.0, "default": 0.0 },
    { "id": "sobel_blend", "type": "slider_float", "label": "Sobel Blend", "min": 0.0, "max": 1.0, "default": 0.5 },
    { "id": "bloom_enable", "type": "checkbox", "label": "Bloom", "default": false },
    { "id": "bloom_threshold", "type": "slider_float", "label": "Bloom Threshold", "min": 0.5, "max": 1.0, "default": 0.8 },
    { "id": "bloom_intensity", "type": "slider_float", "label": "Bloom Intensity", "min": 0.0, "max": 2.0, "default": 0.3 },
    { "id": "gamma", "type": "slider_float", "label": "Gamma", "min": 0.1, "max": 3.0, "default": 1.0 }
  ]
}
```

## Dependencies

- Requires a second RGBA buffer allocation for ping-pong post-processing.
- Temporal absdiff requires a previous-frame history buffer.
- No new third-party libs needed; all standard CUDA texture ops.
