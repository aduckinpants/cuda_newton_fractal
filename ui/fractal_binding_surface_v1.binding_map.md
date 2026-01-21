# Fractal Binding Surface v1  Binding Map

All bindings are under `fractal.*`.

## View (host-side)

- `fractal.view.fractal_type`  `ViewState.fractal_type` (enum)
- `fractal.view.center.x`  `ViewState.center.x` (float)
- `fractal.view.center.y`  `ViewState.center.y` (float)
- `fractal.view.zoom`  `ViewState.zoom` (float)
- `fractal.view.rotation`  `ViewState.rotation_degrees` (float)
- `fractal.view.auto_refresh`  `ViewState.auto_refresh` (bool)

### Camera behavior

- `fractal.view.camera_behavior`  `ViewState.camera_behavior` (enum: manual|complexity|orbit|entropy|off)
- `fractal.view.auto_dive`  `ViewState.auto_dive` (bool)
- `fractal.view.dive_speed`  `ViewState.dive_speed` (float)

## Actions

- `fractal.actions.render_once`  action: enqueue one render tick
- `fractal.actions.reset_view`  action: reset center/zoom/rotation
- `fractal.actions.reset_all`  action: reset view + params + render defaults
- `fractal.actions.export_state`  action: write `..\ui\last_state.json`
- `fractal.actions.import_state`  action: load `..\ui\last_state.json` (strict; fail-fast)

## Fractal params (kernel parameter pack)

- `fractal.params.max_iter`  `KernelParams.max_iter` (int)
- `fractal.params.epsilon`  `KernelParams.epsilon` (float)
- `fractal.params.nova_alpha`  `KernelParams.nova_alpha` (float)
- `fractal.params.phoenix_p_real`  `KernelParams.phoenix_p_real` (float)
- `fractal.params.phoenix_p_imag`  `KernelParams.phoenix_p_imag` (float)

### Polynomial

- `fractal.params.poly_kind`  `KernelParams.poly_kind` (enum)
- `fractal.params.poly_coeffs.0`  `KernelParams.poly_coeffs[0]` (float)
- `fractal.params.poly_coeffs.1`  `KernelParams.poly_coeffs[1]` (float)
- `fractal.params.poly_coeffs.2`  `KernelParams.poly_coeffs[2]` (float)
- `fractal.params.poly_coeffs.3`  `KernelParams.poly_coeffs[3]` (float)
- `fractal.params.poly_coeffs.4`  `KernelParams.poly_coeffs[4]` (float)

### Coloring + grading

- `fractal.params.coloring_mode`  `KernelParams.coloring_mode` (enum)
- `fractal.params.exposure`  `KernelParams.exposure` (float)
- `fractal.params.color_saturation`  `KernelParams.color_saturation` (float)
- `fractal.params.color_contrast`  `KernelParams.color_contrast` (float)
- `fractal.params.color_tint_r`  `KernelParams.color_tint_r` (float)
- `fractal.params.color_tint_g`  `KernelParams.color_tint_g` (float)
- `fractal.params.color_tint_b`  `KernelParams.color_tint_b` (float)

### Explaino

- `fractal.params.explaino_seed`  `KernelParams.explaino_seed` (int)
- `fractal.params.explaino_warp_strength`  `KernelParams.explaino_warp_strength` (float)

### Escape-time extras

- `fractal.params.multibrot_power`  `KernelParams.multibrot_power` (int)

## Render (host + device)

- `fractal.render.resolution.x`  `RenderSettings.resolution.x` (int)
- `fractal.render.resolution.y`  `RenderSettings.resolution.y` (int)
- `fractal.render.block_size`  `RenderSettings.block_size` (int)
- `fractal.render.device_id`  `RenderSettings.device_id` (int)
- `fractal.render.benchmark`  `RenderSettings.benchmark` (bool)

## Artifacts (exe-relative)

- `..\ui\last_ui_error.json`  first UI/schema error (fail-fast)
- `..\ui\last_state.json`  last exported state
