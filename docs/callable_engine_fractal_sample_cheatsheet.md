# Fractal Sampler Cheatsheet

This is the practical can/cannot-do guide for the shipped `fractal.sample` surface.

If you need request-supplied formulas, read [docs/callable_engine_dynamic_function_cheatsheet.md](docs/callable_engine_dynamic_function_cheatsheet.md).
If you need the transport and registry reference, read [docs/callable_engine_surface.md](docs/callable_engine_surface.md).
If you want copy-paste JSON, read [docs/examples/callable_engine/README.md](docs/examples/callable_engine/README.md).

## Mental Model

- `fractal.sample` is the runtime-authoritative sampler for the shipped fractal engine.
- It samples real fractal families and real runtime parameters, not a separate toy evaluator.
- You drive it through binding paths such as `fractal.view.*`, `fractal.params.*`, and `fractal.render.*`.
- The safe way to discover applicable parameters is still `--describe-functions`.

This is the right surface when the question is:

- what does the real shipped fractal engine do here?
- what happens if I sweep an actual runtime parameter?
- how do two shipped fractal families or presets compare under the same probe shape?

## What `fractal.sample` Can Do Today

### 1. Sample shipped fractal families

The current callable surface already samples the shipped probe-supported fractal catalog.

That includes:

- root-finding families such as `newton`, `halley`, and the Explaino family
- escape-time families such as `mandelbrot` and related variants
- the probe-supported advanced catalog already advertised through `--describe-functions`

This is not limited to one or two demo types. The callable surface is meant to probe the real shipped family list.

### 2. Point and grid probes

You can sample in four shapes:

- `point_set`
- `grid`
- `sequence_point_set`
- `sequence_grid`

Use `point_set` when you care about a few exact seeds.
Use `grid` when you want a basin or region scan.

### 3. Real runtime parameter overrides

You can override actual runtime paths such as:

- `fractal.view.fractal_type`
- `fractal.view.zoom`
- `fractal.params.*`
- `fractal.render.*`

That means the probe path is good for:

- comparing fractal families
- sensitivity sweeps on shipped parameters
- checking basin continuity or convergence behavior
- driving sidecar and toolkit analysis off real runtime samples

### 4. Sequence sweeps

Normal sequence sweeps are axis-based:

- `zip_paths: true` means lockstep replay across matching value lists
- `zip_paths: false` means Cartesian expansion across axes

This is the main POC surface for questions like:

- how does `max_iter` change the sampled region?
- how do seed and drift evolve together?
- which sequence step gives the best summary metric?

### 5. Dedicated Explaino variant crossfade sweeps

`fractal.sample` has one extra sweep mode that `generic.sample` does not have:

- `sequence.mode = variant_crossfade`

Current support is intentionally narrow:

- `explaino_ripple`
- `explaino_splice`
- `explaino_vortex`
- `explaino_tension`

Current rules:

- `from_variant` and `to_variant` must be different
- `steps` must be an odd integer `>= 3`
- ordinary V1 sequence overrides may not vary `fractal.view.fractal_type`; this dedicated crossfade path exists for that reason

### 6. Runtime-native result metrics

Per-sample outputs currently exposed for `fractal.sample`:

- `iterations`
- `status`
- `final_z_x`
- `final_z_y`
- `final_abs2`
- `residual`
- `root_index`

Summary metrics currently exposed:

- `mean_iterations`
- `escape_fraction`
- `converged_fraction`
- `nonfinite_fraction`
- `pole_fraction`
- `best_sequence_index`

Some outputs are family-dependent.

Practical rule:

- `root_index` and `residual` are most meaningful on basin/root-finding families
- escape-time families still give valid `status`, `iterations`, and final-state outputs, but not every field is equally informative

## High-Level Request Recipe

1. Set `function_id` to `fractal.sample`.
2. Choose `point_set`, `grid`, `sequence_point_set`, or `sequence_grid`.
3. Set `fractal.view.fractal_type` explicitly in `overrides`.
4. Add any real runtime parameter overrides you care about.
5. Add `region` for grid shapes or `points` for point-set shapes.
6. Add `sequence.vary` if you want a sweep.
7. Request only the metrics you actually need.

Two useful habits:

- always set `function_id` explicitly even if older JSON paths sometimes defaulted it
- always set `fractal.view.fractal_type` explicitly so the request stays obvious when you come back later

## Current Hard Limits And Guard Rails

- shipped callable ids are only `fractal.sample` and `generic.sample`
- grids are capped at 4,000,000 points
- unknown `function_id` values fail fast
- unknown binding paths fail fast
- invalid enum ids fail fast
- unsupported `variant_crossfade` variants fail fast
- ordinary V1 sequence overrides may not vary `fractal.view.fractal_type`

## What It Cannot Do Yet

- define a brand-new formula in the request body
- create a new named fractal callable
- register a new fractal family through JSON alone
- vary `fractal.view.fractal_type` inside ordinary axis sweeps
- use `variant_crossfade` outside the currently supported Explaino variant set
- make unsupported UI-only or unknown binding paths work by fallback
- pretend every UI surface is automatically a good probe surface without checking `--describe-functions`

If your question is “sample the real shipped engine with real runtime knobs,” use `fractal.sample`.
If your question is “sample a custom complex expression that is not a shipped fractal family,” use `generic.sample`.

## Good Fit Vs Bad Fit

Use `fractal.sample` now when:

- you want to inspect a shipped fractal family at exact coordinates
- you want a deterministic grid scan over a real runtime configuration
- you want to sweep actual shipped runtime parameters
- you want Explaino variant crossfades over the dedicated supported set

Do not use `fractal.sample` when:

- you need a custom formula that is not already a shipped fractal family
- you need user-defined composition syntax instead of binding-path overrides
- you need dynamic function authoring rather than runtime parameter probing