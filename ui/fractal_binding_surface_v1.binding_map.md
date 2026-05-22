# Fractal Binding Surface v1 — Binding Map

All bindings are under `fractal.*`.

## View (host-side)

- `fractal.view.center.x` → `ViewState.center.x` (float)
- `fractal.view.center.y` → `ViewState.center.y` (float)
- `fractal.view.zoom` → `ViewState.zoom` (float)
- `fractal.view.rotation` → `ViewState.rotation_degrees` (float, optional; may be ignored by engine)
- `fractal.view.auto_refresh` → `ViewState.auto_refresh` (bool)
- `fractal.view.auto_increment_seed` → `ViewState.auto_increment_seed` (bool; Explaino-family seed motion)
- `fractal.view.explaino_seed_drift` → `ViewState.explaino_seed_drift` (float; advanced fractional component of the combined Explaino seed)
- `fractal.view.explaino_seed_rate` → `ViewState.explaino_seed_rate` (float; per-second Explaino seed motion rate)
- `fractal.view.explaino_seed_tween` → `ViewState.explaino_seed_tween` (bool; tween neighboring seed-derived Explaino polynomials)
- `fractal.actions.render_once` → action: enqueue one render tick
- `fractal.actions.load_state` → action: prompt for `state.json` or `finding.json` and load it into runtime state
- `fractal.actions.capture_finding` → action: archive the current frame/state into the findings tree
- `fractal.params.nova_alpha` → `KernelParams.nova_alpha` (used by both `nova` and `explaino_nova`)

## Fractal (kernel parameter pack)

- `fractal.params.max_iter` → `KernelParams.max_iter` (int)
- `fractal.params.epsilon` → `KernelParams.epsilon` (float)
- `fractal.params.explaino_seed` → host-combined Explaino seed surface (`KernelParams.explaino_seed` integer base plus `ViewState.explaino_seed_drift` fractional component)
- `fractal.params.poly_kind` → `KernelParams.poly_kind` (enum)
- `fractal.params.poly_coeffs.0` → `KernelParams.poly_coeffs[0]` (float, real)
- `fractal.params.poly_coeffs.1` → `KernelParams.poly_coeffs[1]` (float, real)
- `fractal.params.poly_coeffs.2` → `KernelParams.poly_coeffs[2]` (float, real)
- `fractal.params.poly_coeffs.3` → `KernelParams.poly_coeffs[3]` (float, real)
- `fractal.params.poly_coeffs.4` → `KernelParams.poly_coeffs[4]` (float, real)
- `fractal.params.coloring_mode` → `KernelParams.coloring_mode` (enum)
- `fractal.params.exposure` → `KernelParams.exposure` (float)

## Render (host + device)

- `fractal.render.resolution.aspect_preset` -> computed from `RenderSettings.resolution.x/y`; fixed-preset writes update `resolution.x/y`, and `custom` is derived from non-preset dimensions
- `fractal.render.resolution.long_edge` -> computed from `RenderSettings.resolution.x/y`; writes preserve the active computed aspect and update `resolution.x/y`
- `fractal.render.resolution.x` -> `RenderSettings.resolution.x` (int; runtime/saved-state authority, visible only for computed `custom` aspect)
- `fractal.render.resolution.y` -> `RenderSettings.resolution.y` (int; runtime/saved-state authority, visible only for computed `custom` aspect)
- `fractal.render.block_size` → `RenderSettings.block_size` (int; engine validates/clamps)
- `fractal.render.device_id` → `RenderSettings.device_id` (int, optional; engine clamps)
- `fractal.render.benchmark` → `RenderSettings.benchmark` (bool)
- `fractal.render.interaction_debounce_ms` → `RenderSettings.interaction_debounce_ms` (int)
- `fractal.render.preview_target_fps` → `RenderSettings.preview_target_fps` (float)
- `fractal.render.preview_min_scale` → `RenderSettings.preview_min_scale` (float)


## Camera Behavior (View)

- ractal.view.camera_behavior (enum: manual|complexity|orbit|entropy|off)
- ractal.view.auto_dive (bool)
- ractal.view.dive_speed (float, per-frame scale)


## UI Actions / Future Types

- ractal.actions.reset_view (action: resets center/zoom/rotation)
- ractal.params.fractal_type (enum: newton|mandelbrot|julia; only newton implemented today)


## Reset Actions

- ractal.actions.reset_all (action: resets view + params + render defaults)


## Multibrot

- `fractal.params.multibrot_power_float` -> `KernelParams.multibrot_power_float` (float real exponent; hard range [0.01, 32], normal UI range [0.01, 12], default 3.0; visible when `fractal.view.fractal_type = multibrot`)
- `fractal.params.multibrot_power_imag` -> `KernelParams.multibrot_power_imag` (float imaginary exponent; range [-4, 4], default 0.0; visible when `fractal.view.fractal_type = multibrot`)
- `fractal.params.multibrot_power` remains the legacy compatibility alias for the Multibrot real exponent in float binding paths and the integer power binding used by `multicorn_power` on the `multicorn` lane.
