# Fractal Binding Surface v1 (UI + Engine Binding Contract)

This document is the normalized binding contract for the CUDA fractal explorer.

- Scope: what is controllable and how it binds (no UI rendering details).
- Invariants: no invented bindings; stable diff-friendly paths; deterministic visibility rules.

## Namespace

All bindings live under the `fractal.*` namespace.

## Panels

- View
- Fractal
- Render

## View (host-side)

- `fractal.view.center.x` : float, range [-2, 2], step 0.001, default 0.0
- `fractal.view.center.y` : float, range [-2, 2], step 0.001, default 0.0
- `fractal.view.zoom` : float, range [1e-12, 1e12], step 0.01, default 1.0
- `fractal.view.rotation` : float degrees, range [-180, 180], step 0.1, default 0.0 (optional feature; binding exists, engine may ignore)
- `fractal.view.auto_refresh` : bool, default false; continuously dispatches frames even when no state changed
- `fractal.view.auto_increment_seed` : bool, default false; Explaino-family only; advances the combined Explaino seed continuously through the existing drift+tween seam
- `fractal.view.explaino_seed_rate` : float, default 0.35; Explaino-family only; visible when auto-increment is enabled
- `fractal.actions.render_once` : action (button)
- `fractal.actions.load_state` : action (button; opens a saved `state.json` or `finding.json`)
- `fractal.actions.capture_finding` : action (button; archives the current frame/state into the findings tree)
- `fractal.view.fractal_type` enum includes `explaino_nova` as the first new Explaino expansion family

## Fractal (kernel parameter pack)

- `fractal.params.max_iter` : int, range [1, 5000], step 1, default 500
- `fractal.params.epsilon` : float, range [1e-12, 1e-2], step 1e-6, default 1e-6
- `fractal.params.explaino_seed` : double, range [-10, 10], step 0.001, Explaino-family only; primary combined seed control surfaced by the host as integer seed base + fractional drift
- `fractal.params.poly_kind` : enum {`z3_minus_1`, `z4_minus_1`, `custom`}, default `z3_minus_1`
- `fractal.params.poly_coeffs.0..4` : float array (real coefficients), visible only when `poly_kind == custom`
  - Ordering: coefficient `k` multiplies $z^k$.
  - Defaults represent $z^3 - 1$: [-1, 0, 0, 1, 0]
- `fractal.params.coloring_mode` : enum {`root_basin`, `iteration_count`, `smooth_escape`}, default `root_basin`
- `fractal.params.exposure` : float, range [0.1, 5.0], step 0.01, default 1.0
- `fractal.view.explaino_seed_drift` : float, range [0, 0.999], Explaino-family only; advanced fractional component of the combined Explaino seed
- `fractal.view.explaino_seed_tween` : bool, Explaino-family only; blends between neighboring seed-derived polynomials during fractional seed motion

## Render (host + device)

- `fractal.render.resolution.aspect_preset` : computed enum {`custom`, `1:1`, `4:3`, `16:9`, `16:10`, `21:9`}, default-derived `4:3`; writes update `resolution.x/y` and are not persisted separately
- `fractal.render.resolution.long_edge` : computed int, range [256, 4096], step 16, default-derived 2048; writes update the active aspect through `resolution.x/y`
- `fractal.render.resolution.x` : int, range [64, 4096], step 1, default 2048; runtime/saved-state authority, visible only when the computed aspect is `custom`
- `fractal.render.resolution.y` : int, range [64, 4096], step 1, default 1536; runtime/saved-state authority, visible only when the computed aspect is `custom`
- `fractal.render.block_size` : int, range [32, 1024], step 32, default 256 (engine MUST validate/clamp to supported values)
- `fractal.render.device_id` : int, range [0, 7], step 1, default 0 (optional feature; engine clamps to available devices)
- `fractal.render.benchmark` : bool, default false
- `fractal.render.interaction_debounce_ms` : int, default 200; adaptive preview settle window after the last interaction
- `fractal.render.preview_target_fps` : float, default 30; adaptive preview target while panning/zooming/editing
- `fractal.render.preview_min_scale` : float, default 0.5; minimum adaptive preview scale relative to Width/Height


## Camera Behavior (View)

- ractal.view.camera_behavior (enum: manual|complexity|orbit|entropy|off)
- ractal.view.auto_dive (bool)
- ractal.view.dive_speed (float, per-frame scale)


## Multibrot

- `fractal.params.multibrot_power_float` -> `KernelParams.multibrot_power_float` (float real exponent; hard range [0.01, 32], normal UI range [0.01, 12], default 3.0; visible when `fractal.view.fractal_type = multibrot`)
- `fractal.params.multibrot_power_imag` -> `KernelParams.multibrot_power_imag` (float imaginary exponent; range [-4, 4], default 0.0; visible when `fractal.view.fractal_type = multibrot`)
- `fractal.params.multibrot_power` remains the legacy compatibility alias for the Multibrot real exponent in float binding paths and the integer power binding used by `multicorn_power` on the `multicorn` lane.

## Collatz

- `fractal.params.collatz_transition_strength` -> `KernelParams.collatz_transition_strength` (float multiplier on the cosine transition term; hard range [0, 4], normal UI range [0, 2], default 1.0; visible when `fractal.view.fractal_type = collatz`)

## Fixed-Family Fold Mix

- `fractal.params.burning_ship_fold_mix` -> `KernelParams.burning_ship_fold_mix` (float mix from unfolded quadratic step to canonical Burning Ship abs-component fold; range [0, 1], default 1.0; visible when `fractal.view.fractal_type = burning_ship`)
- `fractal.params.celtic_abs_mix` -> `KernelParams.celtic_abs_mix` (float mix from signed quadratic real term to canonical Celtic absolute real term; range [0, 1], default 1.0; visible when `fractal.view.fractal_type = celtic_mandelbrot`)
- `fractal.params.perpendicular_fold_mix` -> `KernelParams.perpendicular_fold_mix` (float mix from signed real factor to canonical Perpendicular Burning Ship folded real factor; range [0, 1], default 1.0; visible when `fractal.view.fractal_type = perpendicular_burning_ship`)
