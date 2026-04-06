# Realtime CLI Sampling / Operator Call-In — Design Note

## Summary

The next reality-toolkit seam should be a headless sampling API, not another
viewer-frame workflow.

The goal is to let an external Salticid operator "call in" to the fractal
runtime as a deterministic probe service:

- no GUI required
- no image capture required for the primary path
- stable JSON request/response contract
- works across fractal families, not just Explaino
- supports parameter sweeps, combined-seed motion, and local probe regions near
  an interesting finding/sample area

This note defines the V1 contract and staging.

## Why This Exists

The current headless/runtime surfaces are:

- `--validate-ui`
- `--capture-diagnostic`
- `--capture-finding`

Those are useful, but they are still image-first. The current probe script in
`tools/reality_toolkit/scripts/run_fractal_param_probe_sweep.py` generates state
files and then captures frames. That is too heavy for operator-driven probing.

The live sweep regression is also proving brittle because it depends on a GUI
window visibly changing over time. That is exactly the wrong abstraction for a
machine operator.

We want a seam where a Salticid operator can do this loop:

1. choose a candidate region / parameter vector / seed motion path
2. call the fractal runtime headlessly
3. receive numeric sample results immediately as JSON
4. score or rank those results in Salticid
5. decide the next probe without touching viewer windows or image files

## Design Goals

1. Stateless subprocess first.
2. JSON over stdin/stdout is the primary operator contract.
3. File-based request/response is supported for manual workflows and debugging.
4. Same request shape works for all fractal families.
5. Unknown fields, enum ids, or invalid parameter combinations fail explicitly.
6. Numeric output uses machine-grade precision; do not rely on the current
   diagnostics `state.json` formatting precision.
7. The CLI should be callable either directly by `fractal_ui.exe` or through a
   small Python toolkit wrapper that Salticid can treat as a stable adapter.

## Non-Goals (V1)

- No long-lived server/broker requirement yet.
- No named-pipe or socket protocol in V1.
- No image output in the main probe path.
- No attempt to make the operator speak UI schema directly.
- No hidden fallback from sample mode to capture-diagnostic mode.

## Operator Call-In Model

### Preferred invocation

Use the runtime as a subprocess with request JSON on stdin and response JSON on
stdout:

```powershell
fractal_ui.exe --sample-request-stdin --sample-response-stdout
```

Why this is the right V1 seam:

- avoids long Windows command-line quoting issues for nested JSON
- makes Salticid integration straightforward
- keeps the runtime stateless and easy to supervise
- works locally first; can later be wrapped by a broker without redesigning the
  payload

### Secondary/manual invocation

For reproducibility and manual debugging:

```powershell
fractal_ui.exe --sample-request-json request.json --sample-response-json result.json
```

Both surfaces must use the same request/response schema.

## Proposed CLI Surface

### New headless verbs

- `--sample-request-stdin`
- `--sample-request-json <path>`
- `--sample-response-stdout`
- `--sample-response-json <path>`

Rules:

- exactly one request source is required
- at least one response sink is required
- sample mode is mutually exclusive with `--validate-ui`, `--capture-diagnostic`,
  and `--capture-finding`
- invalid combinations return non-zero and emit a machine-readable error payload

## Request Schema (V1)

```json
{
  "request_version": 1,
  "request_id": "probe-2026-04-06T15-30-00Z-001",
  "mode": "sequence_grid",
  "base_state": {
    "load_state_json": "D:/salt-fractal/cuda_newton_fractal_clone/findings/foo/state.json"
  },
  "overrides": [
    { "path": "fractal.view.fractal_type", "value": "explaino_lambda" },
    { "path": "fractal.params.explaino_warp_strength", "value": 0.25 },
    { "path": "fractal.params.lambda_real", "value": 2.9685855 },
    { "path": "fractal.params.lambda_imag", "value": -0.27446103 }
  ],
  "region": {
    "center_x": 0.5,
    "center_y": 0.0,
    "span_x": 0.15,
    "span_y": 0.15,
    "grid_width": 32,
    "grid_height": 32
  },
  "sequence": {
    "vary": [
      {
        "path": "fractal.params.explaino_seed",
        "values": [3.0, 3.25, 3.5, 3.75, 4.0]
      },
      {
        "path": "fractal.view.explaino_seed_drift",
        "values": [0.0, 0.1, 0.2, 0.3, 0.4]
      }
    ],
    "zip_paths": true
  },
  "metrics": [
    "iterations",
    "status",
    "final_z",
    "final_abs2",
    "residual",
    "summary_mean_iterations",
    "summary_escape_fraction",
    "summary_converged_fraction"
  ],
  "operator_context": {
    "source": "salticid",
    "operator": "parameter_probe_scout",
    "why": "rank local seed motion around a sampled finding region"
  }
}
```

## Response Schema (V1)

```json
{
  "response_version": 1,
  "request_id": "probe-2026-04-06T15-30-00Z-001",
  "ok": true,
  "runtime": {
    "exe_path": "D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe",
    "fractal_type": "explaino_lambda",
    "device_id": 0
  },
  "summary": {
    "sample_count": 5120,
    "mean_iterations": 341.2,
    "escape_fraction": 0.8125,
    "converged_fraction": 0.0,
    "nonfinite_fraction": 0.0,
    "pole_fraction": 0.0019,
    "best_sequence_index": 3
  },
  "sequence_results": [
    {
      "sequence_index": 0,
      "applied": {
        "fractal.params.explaino_seed": 3.0,
        "fractal.view.explaino_seed_drift": 0.0
      },
      "summary": {
        "mean_iterations": 288.6,
        "escape_fraction": 0.76,
        "converged_fraction": 0.0
      }
    }
  ],
  "samples": [
    {
      "sequence_index": 0,
      "grid_x": 0,
      "grid_y": 0,
      "coord_x": 0.425,
      "coord_y": 0.075,
      "iterations": 147,
      "status": "escaped",
      "final_z_x": -1.723,
      "final_z_y": 3.441,
      "final_abs2": 14.792,
      "residual": null,
      "root_index": null
    }
  ],
  "operator_context": {
    "source": "salticid",
    "operator": "parameter_probe_scout",
    "why": "rank local seed motion around a sampled finding region"
  },
  "error": null
}
```

## Modes

### 1. `point_set`

Sample an arbitrary list of coordinates.

Use when Salticid already has an explicit shortlist of points to inspect.

### 2. `grid`

Sample a rectangular region using a regular grid.

Use when the operator wants a local field measurement around a known finding or
sample area.

### 3. `sequence_grid`

Run the same grid repeatedly while varying one or more parameters.

This is the V1 replacement for the current image-heavy motion idea.

### 4. `sequence_point_set`

Run the same point set repeatedly while varying one or more parameters.

Use when Salticid is doing a tight local search around a few chosen coordinates.

## Family-Agnostic Sample Fields

Every sampled point should return these core fields:

- `iterations`
- `status`
- `final_z_x`
- `final_z_y`
- `final_abs2`

Optional fields by family:

- `residual`
  - meaningful for Newton-like / basin families
  - `null` for pure escape-time families when not defined
- `root_index`
  - meaningful only when a root assignment exists
  - `null` otherwise

### Sample status enum

V1 should use a finite machine enum, not freeform strings from random branches:

- `escaped`
- `converged`
- `bounded`
- `pole`
- `nonfinite`
- `invalid_param`

This keeps Salticid-side scoring logic simple.

## Parameter Override Model

Do not make the operator mutate C++ structs directly. Reuse the schema/runtime
binding path vocabulary where possible:

- `fractal.view.*`
- `fractal.params.*`
- `fractal.render.*`

This keeps the operator contract aligned with the host UI contract and reduces
the chance of a second source of truth.

Unknown binding paths must fail.

## Seed Motion Model

The operator should be able to probe combined seed motion directly without
special-casing Explaino internals.

V1 rule:

- allow overrides on both `fractal.params.explaino_seed` and
  `fractal.view.explaino_seed_drift`
- normalize via the existing Explaino seed helpers before dispatch
- echo back normalized values in the response summary

This preserves the existing combined-seed semantics and makes Salticid capable
of exploring both coarse seed jumps and fine drift motion.

## Precision Requirements

The current diagnostics bundle writes floats with default stream precision,
which is fine for human inspection but too lossy for operator contracts.

For the sample API:

- use explicit precision when writing JSON
- preserve enough digits to round-trip doubles/floats safely
- do not require Salticid to infer intent from rounded diagnostic snapshots

## Implementation Shape

Do not grow `ui_app/src/main.cpp` further than necessary.

Preferred extraction seams:

- `fractal_probe_contract.h/.cpp`
  - request parsing
  - response writing
  - fail-fast validation
- `fractal_probe_runner.h/.cu`
  - sample dispatch
  - grid/point-set sequencing
  - summary reduction
- `fractal_probe_metrics.h/.cpp`
  - aggregation helpers

The runtime should call those seams from `main.cpp`, but the protocol logic
should live outside the monolith.

## CUDA / Runtime Contract Notes

Today `RenderFractalCUDA()` returns:

- RGBA frame
- optional mask
- `RenderStats { last_render_ms, last_iters_avg, last_device_id }`

That is not enough for operator probing.

V1 needs a parallel probe path, likely something like:

```cpp
bool SampleFractalCUDA(
    const ViewState& view,
    const KernelParams& params,
    const ProbeRequest& request,
    ProbeResponse* outResponse,
    const char** outError);
```

This should not depend on image buffers.

## Salticid Integration Pattern

The cool part is not that Salticid launches a viewer. The cool part is that the
fractals become a callable measurement oracle.

Suggested adapter shape:

- Salticid operator constructs a request payload
- operator shells out to either:
  - `fractal_ui.exe --sample-request-stdin --sample-response-stdout`, or
  - `py -3.14 tools/reality_toolkit/scripts/run_fractal_sample_operator_call.py`
- adapter/runtime returns JSON
- operator scores the response and decides the next request

Why a Python adapter may still be useful even if the runtime supports stdin/json
directly:

- easier request construction from existing toolkit code
- path normalization / repo-root discovery lives in one place
- later can bridge to Salticid session/broker conventions without changing the
  runtime contract

## Fail-Fast Rules

- unknown request version => error
- unknown mode => error
- unknown binding path => error
- unknown fractal type / enum id => error
- missing request source or response sink => error
- sequence arrays with mismatched lengths in `zip_paths` mode => error
- invalid region dimensions or empty point sets => error
- unsupported metric names => error
- unsupported fractal/metric combination => explicit `null` or explicit error,
  never silent substitution

## Acceptance Criteria

1. A Salticid operator can invoke the runtime as a subprocess with stdin JSON and
   get stdout JSON back.
2. The runtime can sample at least one escape-time family and one Newton-like
   family through the same contract.
3. `sequence_grid` supports combined seed motion around a local region without
   producing image artifacts or requiring viewer windows.
4. Responses include enough precision and metadata to be ranked or compared by
   an external operator.
5. Invalid requests fail fast with machine-readable error payloads.

## Recommended Staging

### Phase A — Contract + host-only parser/validator

- add request/response schema types
- add CLI verb plumbing
- add parser/validator tests
- no CUDA sampling yet

### Phase B — Minimal probe runner

- support `point_set` for one family end-to-end
- JSON stdout/file round-trip tests
- fail-fast tests for invalid requests

### Phase C — Family-general sampling

- extend sampling path across escape-time + basin families
- add unified status/residual/root-index semantics

### Phase D — Sequence probing for motion/toolkit work

- add `sequence_grid`
- add toolkit adapter script
- add operator-oriented regression tests

## Recommendation

Build V1 around `stdin`/`stdout` JSON and `sequence_grid` first.

That gives us the shortest path from:

- today's image-heavy probe scripts

to:

- Salticid-driven parameter scouts that can "call in" to the fractal runtime as
  a numeric oracle.