# Dynamic Function Solving Cheatsheet

This is the practical can/cannot-do guide for the current callable POC.

If you need the transport and registry reference, read [docs/callable_engine_surface.md](docs/callable_engine_surface.md).
If you need session, batch, or NDJSON behavior, read [docs/callable_engine_transport_session_cheatsheet.md](docs/callable_engine_transport_session_cheatsheet.md).
If you want copy-paste request JSON, read [docs/examples/callable_engine/README.md](docs/examples/callable_engine/README.md).
If you need the short version: `generic.sample` lets you send a complex-valued expression in the request, evaluate it over points, grids, and sequence sweeps, and get value/derivative/summary data back. It does not yet create new named callables or dynamic CUDA kernels.

## Mental Model

- `fractal.sample` samples one of the shipped fractal families through the existing viewer binding vocabulary.
- `generic.sample` samples a request-supplied complex expression.
- `--describe-functions` tells you which callable ids ship today.
- `--sample-request-*` and `--sample-session` are transports, not new math surfaces.
- `--explore-recommend` is an Explaino-only advisor mode, not a dynamic function surface.

## What `generic.sample` Can Do Today

### 1. Direct evaluation

Evaluate a complex expression at one point or many points.

Good fits:

- `z^2 + z + 1`
- `exp(z)`
- `compose(z^2, z + 1)`

Use this when you want the value field itself, not a long iterative basin.

### 2. Iterated maps

Use `iterate(body, N)` to repeatedly apply a map starting from `z`.

That gives you a simple escape/convergence probe surface for maps such as:

- `iterate(z^2 + c, 50)`
- `iterate(exp(z) - 1, 60)`
- `iterate(compose(sin(z), z^2 + c), 40)`

The request-level `epsilon` and `escape_radius` decide when a sample is treated as `converged`, `escaped`, `bounded`, or `nonfinite`.

### 3. Newton and root-finding maps

The current POC is already useful for custom Newton-style solvers if you write the iteration explicitly.

Example:

- `iterate(z - (z^3 - 1) / (3 * z^2), 200)`

That means the dynamic function surface can already explore:

- polynomial Newton basins
- transcendental Newton steps
- custom fixed-point iterations

### 4. Parameterized families

You can provide free parameters inside `function.params`.

Rules:

- scalar params are referenced directly by name
- complex params are formed from `<name>_real` and `<name>_imag`, then referenced as `<name>`
- exponents after `^` can come from numeric literals or param names

Example idea:

- expression: `iterate(z^2 + c, 50)`
- params: `c_real`, `c_imag`

### 5. Point, grid, and sequence sweeps

You can sample the same expression in four shapes:

- `point_set`: explicit list of coordinates
- `grid`: one rectangular region sampled once
- `sequence_point_set`: replay the same point set across sequence steps
- `sequence_grid`: replay the same grid across sequence steps

Sequence sweeps are axis-based:

- `zip_paths: true` means lockstep sweep values
- `zip_paths: false` means Cartesian expansion

For `generic.sample`, the current useful sequence paths are numeric `function.params.*` entries such as:

- `function.params.c_real`
- `function.params.c_imag`
- `function.params.power`

### 6. Derivatives and summary metrics

Per-sample outputs currently exposed for `generic.sample`:

- `iterations`
- `status`
- `value_x`
- `value_y`
- `abs2`
- `derivative_x`
- `derivative_y`

Summary metrics currently exposed:

- `mean_iterations`
- `converged_fraction`
- `escape_fraction`
- `nonfinite_fraction`
- `mean_abs2`
- `diverged_fraction`

The derivative output is numerical. It is useful for probing local behavior, but it is not a symbolic derivative system.

## Expression Building Blocks

Supported variables:

- `z`
- `z_conj`

Supported operators:

- `+`
- `-`
- `*`
- `/`
- `^`

Supported unary functions:

- `sin(...)`
- `cos(...)`
- `exp(...)`
- `log(...)`
- `abs(...)`
- `conj(...)`

Supported higher-level helpers:

- `iterate(body, count)` where `count` is an integer literal or a bare scalar param name
- `compose(f, g)`

## High-Level Request Recipe

1. Set `function_id` to `generic.sample`.
2. Pick `point_set`, `grid`, `sequence_point_set`, or `sequence_grid`.
3. Put the math in `function.expression`.
4. Put free params in `function.params`.
5. If it is iterative, set `function.epsilon` and `function.escape_radius`.
6. Ask only for the outputs and summary metrics you actually need.
7. Add a `sequence` block if you want a sweep.

Optional execution control:

- use `execution.backend_preference` only when you need to pin the backend for debugging or parity work
- allowed values are `default`, `cpu`, and `cuda`
- current runtime policy resolves `default` to the CPU path
- successful responses report the actual executor in `runtime.backend_used`
- an explicitly requested unavailable backend fails fast; it does not silently fall back

Minimal Newton-style point-set sketch:

```json
{
  "request_version": 1,
  "request_id": "newton-z3m1",
  "function_id": "generic.sample",
  "mode": "point_set",
  "function": {
    "expression": "iterate(z - (z^3 - 1) / (3 * z^2), 200)",
    "epsilon": 1e-10,
    "escape_radius": 1000.0
  },
  "points": [
    {"x": 1.1, "y": 0.05},
    {"x": -0.4, "y": 0.9}
  ],
  "metrics": ["iterations", "status", "value", "derivative"]
}
```

Minimal parameter sweep sketch:

```json
{
  "request_version": 1,
  "request_id": "sweep-c-real",
  "function_id": "generic.sample",
  "mode": "sequence_grid",
  "function": {
    "expression": "iterate(z^2 + c, 50)",
    "params": {"c_real": 0.0, "c_imag": 0.0},
    "epsilon": 1e-8,
    "escape_radius": 4.0
  },
  "region": {
    "center_x": 0.0,
    "center_y": 0.0,
    "span_x": 2.0,
    "span_y": 2.0,
    "grid_width": 64,
    "grid_height": 64
  },
  "sequence": {
    "zip_paths": false,
    "vary": [
      {"path": "function.params.c_real", "values": [-2.0, -1.0, 0.0]}
    ]
  },
  "metrics": ["status", "summary_mean_iterations", "summary_diverged_fraction"]
}
```

## Current Hard Limits And Guard Rails

- shipped callable ids are only `fractal.sample` and `generic.sample`
- expression tree size is capped at 64 nodes
- stored parameter slots are capped at 32
- parser nesting depth is capped at 50
- `iterate(..., count)` requires a finite integer in `[1, 10000]` and fails fast otherwise
- grids are capped at 4,000,000 points
- malformed expressions and unknown function ids fail fast instead of falling back

## What It Cannot Do Yet

- create a new named `function_id`
- dynamically register, transpile, or load a new CUDA kernel from request data
- teach `--describe-functions` about your expression-specific free params
- use `sequence.mode = variant_crossfade` with `generic.sample`
- apply arbitrary `fractal.view.*` or `fractal.params.*` override paths inside the current `generic.sample` runner path
- define user lambdas, conditionals, piecewise branches, or user-defined functions
- define loops other than `iterate(...)`
- define multi-input, vector, or matrix-valued functions
- ask for symbolic derivatives, Jacobians, or Hessians
- rely on the public dynamic-function path being GPU-backed today

Two practical limitations matter a lot right now:

- `generic.sample` is effectively a stateless math surface, not a view-state-driven runtime surface
- the runtime-owned default path currently resolves to CPU, but the public request surface can now pin `execution.backend_preference` to `cpu` or `cuda` and the response reports `runtime.backend_used`
- the current generic gallery helper and `gallery_manifest.json` output path are temporary scaffolding for proof work; they are useful today, but they are not the future formal output contract

## Good Fit Vs Bad Fit

Use `generic.sample` now when:

- you want to explore a custom complex map quickly
- you want Newton or fixed-point basin sweeps over a few numeric params
- you want a sidecar math probe around an existing captured scene
- you want deterministic JSON output from a request-supplied formula

Do not treat it as the final backend when:

- you need a reusable named callable to show up as a new engine function
- you need runtime/kernel registration instead of request-supplied expressions
- you need `--describe-functions` to machine-advertise execution preferences; the override exists in the request contract today, but it is intentionally kept out of the normal math-parameter list

That broader handoff belongs to the later transpiler and kernel-registration thread. For the current branch, the right mental model is:

- `generic.sample` is the preview and stress-test surface for dynamic callable math
- its backend choice is a low-prominence execution control, not part of the math signature
- it is not yet the finished dynamic callable backend

## Copy-Paste Examples

- [docs/examples/callable_engine/generic_sample_newton_z3m1_point_set.json](docs/examples/callable_engine/generic_sample_newton_z3m1_point_set.json)
- [docs/examples/callable_engine/README.md](docs/examples/callable_engine/README.md)