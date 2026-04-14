# Callable Engine Surface

This document is the current reference for the repo's callable headless engine surface.

For a practical can/cannot-do guide focused on dynamic function solving and sweep shapes, see [docs/callable_engine_dynamic_function_cheatsheet.md](docs/callable_engine_dynamic_function_cheatsheet.md).
For the matching real-runtime fractal probe guide, see [docs/callable_engine_fractal_sample_cheatsheet.md](docs/callable_engine_fractal_sample_cheatsheet.md).
For transport, batch, NDJSON, and session behavior, see [docs/callable_engine_transport_session_cheatsheet.md](docs/callable_engine_transport_session_cheatsheet.md).
For copy-paste request JSON, see [docs/examples/callable_engine/README.md](docs/examples/callable_engine/README.md).

It answers three questions:

1. what callable surfaces ship today
2. how to invoke them deterministically
3. where the current `generic.sample` expression/composition preview stops and the later generalized kernel-registration work begins

## Current Surfaces

### 1. Function discovery

Use the descriptor surface to discover callable function ids, parameter metadata, and output metrics.

CLI:

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --describe-functions
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --describe-functions-json D:\salt-fractal\cuda_newton_fractal_clone\artifacts\describe_functions.json
```

Shipped function ids today:

- `fractal.sample`
- `generic.sample`

The built-in callable registry is the authority for those shipped ids.
`--describe-functions` and headless `function_id` validation/dispatch now consume the same registry instead of carrying separate hardcoded lists.
That registry also owns the descriptor-builder callback for each shipped callable, so adding another built-in callable is now a single registry-surface change instead of a registry row plus a separate descriptor switch.

### 2. Stateless sample requests

Use the sample request/response contract for deterministic point, grid, and sequence sampling.

CLI transports:

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --sample-request-stdin --sample-response-stdout
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --sample-request-json request.json --sample-response-json result.json
```

Representative `fractal.sample` request:

```json
{
  "request_version": 1,
  "request_id": "fractal-grid-001",
  "function_id": "fractal.sample",
  "mode": "grid",
  "overrides": [
    {"path": "fractal.view.fractal_type", "value": "explaino_lambda"},
    {"path": "fractal.params.lambda_real", "value": 2.9685855},
    {"path": "fractal.params.lambda_imag", "value": -0.27446103}
  ],
  "region": {
    "center_x": 0.0,
    "center_y": 0.0,
    "span_x": 2.0,
    "span_y": 2.0,
    "grid_width": 8,
    "grid_height": 8
  },
  "metrics": ["iterations", "status", "summary_mean_iterations"]
}
```

### 3. Stateful session transport

Use the one-line JSON session protocol when one external caller wants to keep state tokens alive across multiple requests.

CLI:

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --sample-session
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --sample-session --sample-session-pipe my_session_pipe
```

### 4. Exploration advisor

Use the advisor surface when the input is a current or loaded runtime state and the output should be a deterministic next-best-observation report instead of sample data.

CLI:

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --load-state-json state.json --explore-recommend
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --load-state-json state.json --explore-recommend-json advisor.json
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --load-state-json state.json --explore-recommend --explore-recommend-json advisor.json
```

Important boundary:

- the advisor is a headless report mode, not a callable `function_id`
- it is Explaino-only today and intentionally fails fast on non-Explaino states

## Current Function Boundary

### `fractal.sample`

For a practical POC guide focused on the real runtime fractal probe surface, supported sweep shapes, and current limits, see [docs/callable_engine_fractal_sample_cheatsheet.md](docs/callable_engine_fractal_sample_cheatsheet.md).

This is the runtime-authoritative fractal sampler over the existing binding-path vocabulary:

- `fractal.view.*`
- `fractal.params.*`
- `fractal.render.*`

Callers should discover applicable parameters through `--describe-functions` rather than guessing.

### `generic.sample`

For a current POC cheatsheet focused on dynamic function solving, supported expression forms, and supported sweep modes, see [docs/callable_engine_dynamic_function_cheatsheet.md](docs/callable_engine_dynamic_function_cheatsheet.md).
Copy-paste request JSON lives under [docs/examples/callable_engine/README.md](docs/examples/callable_engine/README.md).

This is the current preview surface for request-supplied function expressions.

It already lets incoming sample requests carry a composed function body directly:

```json
{
  "request_version": 1,
  "request_id": "generic-compose-001",
  "function_id": "generic.sample",
  "mode": "point_set",
  "function": {
    "expression": "compose(sin(z), z^2 + c)",
    "params": {
      "c_real": -0.75,
      "c_imag": 0.1
    },
    "epsilon": 1e-8,
    "escape_radius": 16.0
  },
  "points": [
    {"x": 0.1, "y": 0.2}
  ],
  "metrics": ["value", "abs2", "derivative"]
}
```

What `generic.sample` is today:

- a shipped expression/evaluator preview surface
- suitable for request-supplied composed functions like `compose(...)` and `iterate(...)`
- useful for prototyping generic/lambda-style compositions from incoming requests

What `generic.sample` is not yet:

- dynamic kernel registration
- a registry of newly named described functions
- transpiled CUDA kernels loaded from external descriptors
- proof that every future lambda/kernel composition should become a first-class `function_id`

## Near-Term Wrap Rule

For the current branch, treat the callable surface as:

- discovery via `--describe-functions`
- deterministic sampling via `fractal.sample` and `generic.sample`
- deterministic state-review advice via `--explore-recommend`

Do not pretend the engine already supports arbitrary registered kernels. That belongs to the later registry/transpiler thread described in:

- [docs/notes/callable_engine_surface_wrap_PHASED_PLAN.md](docs/notes/callable_engine_surface_wrap_PHASED_PLAN.md)
- [spec_intake/GenericCudaSamplerBridge_SpecIntake.md](spec_intake/GenericCudaSamplerBridge_SpecIntake.md)
- [spec_intake/RealtimeCliSampling_OperatorCallIn_DesignNote.md](spec_intake/RealtimeCliSampling_OperatorCallIn_DesignNote.md)

Current Phase 3 stop point:

- the shipped callable ids, execution kinds, and descriptor builders are now registry-backed inside this repo
- the next step is the engine-side handoff boundary for later transpiler/kernel registration work
- dynamic kernel registration and transpiler-backed external function loading remain future work