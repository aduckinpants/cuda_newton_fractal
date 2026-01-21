# Fractal Binding Surface v1 (UI + Engine Binding Contract)

This document is the normalized binding contract for the CUDA fractal explorer.

- Scope: what is controllable and how it binds (no UI rendering details).
- Invariants: no invented bindings; stable diff-friendly paths.
- Diagnostics: on any UI/schema error the app fails-fast and writes a small JSON artifact.

## Namespace

All bindings live under the `fractal.*` namespace.

## View (host-side)

- `fractal.view.fractal_type` : enum {`newton`, `nova`, `explaino`, `explaino_y`, `explaino_fp`, `mandelbrot`, `julia`, `burning_ship`, `multibrot`, `phoenix`}
- `fractal.view.center.x` : float
- `fractal.view.center.y` : float
- `fractal.view.zoom` : float
- `fractal.view.rotation` : float degrees
- `fractal.view.auto_refresh` : bool

### Camera behavior

- `fractal.view.camera_behavior` : enum {`manual`, `complexity`, `orbit`, `entropy`, `off`}
- `fractal.view.auto_dive` : bool
- `fractal.view.dive_speed` : float (per-frame scale)

## Actions

- `fractal.actions.render_once` : action
- `fractal.actions.reset_view` : action
- `fractal.actions.reset_all` : action
- `fractal.actions.export_state` : action (writes `..\ui\last_state.json`)
- `fractal.actions.import_state` : action (loads `..\ui\last_state.json`, strict)

## Fractal params (kernel parameter pack)

- `fractal.params.max_iter` : int
- `fractal.params.epsilon` : float
- `fractal.params.nova_alpha` : float
- `fractal.params.phoenix_p_real` : float
- `fractal.params.phoenix_p_imag` : float
- `fractal.params.poly_kind` : enum {`z3_minus_1`, `z4_minus_1`, `custom`}
- `fractal.params.poly_coeffs.0..4` : float array (real coefficients)
- `fractal.params.multibrot_power` : int
- `fractal.params.coloring_mode` : enum {`root_basin`, `iteration_count`, `smooth_escape`, `joy_basins`}
- `fractal.params.exposure` : float

### Global grading (applies to all fractals)

- `fractal.params.color_saturation` : float
- `fractal.params.color_contrast` : float
- `fractal.params.color_tint_r` : float
- `fractal.params.color_tint_g` : float
- `fractal.params.color_tint_b` : float

### Explaino controls

- `fractal.params.explaino_seed` : int
- `fractal.params.explaino_warp_strength` : float

## Render (host + device)

- `fractal.render.resolution.x` : int
- `fractal.render.resolution.y` : int
- `fractal.render.block_size` : int
- `fractal.render.device_id` : int
- `fractal.render.benchmark` : bool

## Artifacts (exe-relative)

- `..\ui\last_ui_error.json` : first UI/schema error, fail-fast
- `..\ui\last_state.json` : last exported state
