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
- `fractal.view.zoom` : float, range [0.1, 1000], step 0.01, default 1.0
- `fractal.view.rotation` : float degrees, range [-180, 180], step 0.1, default 0.0 (optional feature; binding exists, engine may ignore)
- `fractal.view.auto_refresh` : bool, default true
- `fractal.actions.render_once` : action (button)

## Fractal (kernel parameter pack)

- `fractal.params.max_iter` : int, range [1, 5000], step 1, default 500
- `fractal.params.epsilon` : float, range [1e-12, 1e-2], step 1e-6, default 1e-6
- `fractal.params.poly_kind` : enum {`z3_minus_1`, `z4_minus_1`, `custom`}, default `z3_minus_1`
- `fractal.params.poly_coeffs.0..4` : float array (real coefficients), visible only when `poly_kind == custom`
  - Ordering: coefficient `k` multiplies $z^k$.
  - Defaults represent $z^3 - 1$: [-1, 0, 0, 1, 0]
- `fractal.params.coloring_mode` : enum {`root_basin`, `iteration_count`, `smooth_escape`}, default `root_basin`
- `fractal.params.exposure` : float, range [0.1, 5.0], step 0.01, default 1.0

## Render (host + device)

- `fractal.render.resolution.x` : int, range [64, 4096], step 1, default 1024
- `fractal.render.resolution.y` : int, range [64, 4096], step 1, default 768
- `fractal.render.block_size` : int, range [32, 1024], step 32, default 256 (engine MUST validate/clamp to supported values)
- `fractal.render.device_id` : int, range [0, 7], step 1, default 0 (optional feature; engine clamps to available devices)
- `fractal.render.benchmark` : bool, default false


## Camera Behavior (View)

- ractal.view.camera_behavior (enum: manual|complexity|orbit|entropy|off)
- ractal.view.auto_dive (bool)
- ractal.view.dive_speed (float, per-frame scale)


## Multibrot

- ractal.params.multibrot_power (int, >=2; used when ractal.view.fractal_type = multibrot)
