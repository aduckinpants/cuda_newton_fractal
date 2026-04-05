# Fractal Binding Surface v1 — Binding Map

All bindings are under `fractal.*`.

## View (host-side)

- `fractal.view.center.x` → `ViewState.center.x` (float)
- `fractal.view.center.y` → `ViewState.center.y` (float)
- `fractal.view.zoom` → `ViewState.zoom` (float)
- `fractal.view.rotation` → `ViewState.rotation_degrees` (float, optional; may be ignored by engine)
- `fractal.view.auto_refresh` → `ViewState.auto_refresh` (bool)
- `fractal.actions.render_once` → action: enqueue one render tick
- `fractal.actions.load_state` → action: prompt for `state.json` or `finding.json` and load it into runtime state
- `fractal.actions.capture_finding` → action: archive the current frame/state into the findings tree

## Fractal (kernel parameter pack)

- `fractal.params.max_iter` → `KernelParams.max_iter` (int)
- `fractal.params.epsilon` → `KernelParams.epsilon` (float)
- `fractal.params.poly_kind` → `KernelParams.poly_kind` (enum)
- `fractal.params.poly_coeffs.0` → `KernelParams.poly_coeffs[0]` (float, real)
- `fractal.params.poly_coeffs.1` → `KernelParams.poly_coeffs[1]` (float, real)
- `fractal.params.poly_coeffs.2` → `KernelParams.poly_coeffs[2]` (float, real)
- `fractal.params.poly_coeffs.3` → `KernelParams.poly_coeffs[3]` (float, real)
- `fractal.params.poly_coeffs.4` → `KernelParams.poly_coeffs[4]` (float, real)
- `fractal.params.coloring_mode` → `KernelParams.coloring_mode` (enum)
- `fractal.params.exposure` → `KernelParams.exposure` (float)

## Render (host + device)

- `fractal.render.resolution.x` → `RenderSettings.resolution.x` (int)
- `fractal.render.resolution.y` → `RenderSettings.resolution.y` (int)
- `fractal.render.block_size` → `RenderSettings.block_size` (int; engine validates/clamps)
- `fractal.render.device_id` → `RenderSettings.device_id` (int, optional; engine clamps)
- `fractal.render.benchmark` → `RenderSettings.benchmark` (bool)


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

- ractal.params.multibrot_power (int, >=2; used when ractal.view.fractal_type = multibrot)
