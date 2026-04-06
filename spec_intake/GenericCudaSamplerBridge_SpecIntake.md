# Generic CUDA Sampler Bridge — Spec Intake

## Vision

Turn the CUDA engine into a **fast, accurate, semi-arbitrary function sampler
service** that Salticid (or any caller) can invoke on demand.

The long-term idea has three layers:

1. **Described functions** — the engine advertises what callable functions it
   hosts, including parameter names, types, ranges, and output metrics, via a
   machine-readable descriptor surface. Callers never guess.

2. **Transpiler bridge** — Salt operators (or arbitrary code from other
   languages) can be transpiled through the existing Salticid CoreIR pipeline
   into CUDA kernel source, compiled, and registered as new described functions
   in the engine. The engine becomes a JIT-or-AOT execution surface for
   externally authored computation.

3. **Operator wrapping** — once a function is registered and described, a
   Salticid operator adapter can construct probe requests, invoke the engine,
   consume responses, and present the result as a first-class Salt operator.
   From the Salt pipeline's perspective, "run this fractal" and "run this
   transpiled CUDA kernel" look the same.

Together these mean: **Salt operator from any arbitrary\* existing code, via
CUDA**, decoupled from the Salticid substrate and its baggage.

\*With limitations: the transpiled code must be expressible as a pure function
over a described parameter domain, it must compile to valid CUDA, and the
parameter surface must be expressible as a flat or shallow descriptor.

## Why This Matters

- The current Salticid Python runtime is accurate but slow for numeric-heavy
  computation. A CUDA bridge gives operators access to GPU-speed evaluation
  without rewriting the operator in CUDA by hand.
- The current fractal probe sampler (Phases A-D, committed at `01435ea`) proves
  the subprocess JSON contract works. Generalizing from "fractal sampler" to
  "function sampler" is the natural next step.
- The Salticid transpiler already has a working `CoreIR -> C++` backend. Adding
  a `CoreIR -> CUDA` backend is a bounded extension, not a new system.
- The UI schema already carries rich parameter metadata (type, range, default,
  visibility, enum options). Extracting a runtime-queryable function descriptor
  from the same authority is a reuse story, not a new invention.

## Existing Seams to Build On

### CUDA engine side (this repo)

| Seam | What it gives us |
|------|------------------|
| Probe CLI (`--sample-request-stdin/json`) | Proven subprocess JSON contract |
| `fractal_probe_contract.h/.cpp` | Request/response schema, fail-fast parsing |
| `fractal_probe_runner.h/.cpp` | Host-side point/grid/sequence dispatch loop |
| UI schema JSON | Parameter descriptors: type, range, default, visibility, enum options |
| `BindingContext` | Runtime binding path -> struct pointer resolution |
| `schema_binding.h/.cpp` | Typed value get/set, predicate evaluation |
| `ui_schema.h/.cpp` | Schema loader, control metadata structs |

### Salticid transpiler side (salticid-cuda, read-only reference)

| Seam | What it gives us |
|------|------------------|
| `transpiler/core_ir.py` | `Expr`, `Call`, `Compose`, `Stencil` IR types |
| `transpiler/backends/cpp.py` | Working C++ codegen from CoreIR |
| `transpiler/backends/base.py` | `Backend` protocol — pluggable emitter interface |
| `transpiler/api.py` | `lower_to_core_ir()`, `transpile_to_text()` public API |
| `spec/operators.d/*.json` | Operator JSON specs with contract/kwargs metadata |
| `op_table.py` | Operator registry with `_Entry` (name, arity, meta, lower) |
| `operator_atlas.py` | Introspects 540+ operators with full kwargs via `inspect.signature()` |

### Architectural alignment

Both sides already use the same patterns:

- **Described parameters**: UI schema JSON on the engine side, operator JSON
  specs + `inspect.signature()` on the transpiler side.
- **Binding-path vocabulary**: `fractal.view.*`, `fractal.params.*` on the
  engine; `contract.kwargs` on the transpiler.
- **Fail-fast on unknown**: both sides reject unknown paths/fields.
- **Pluggable backends**: the transpiler has a `BackendRegistry`; the engine's
  probe runner dispatches per fractal family. Both patterns generalize.

## Architecture

### Layer 1: Described Function Surface

The engine should expose a `/describe` or `--describe-functions` verb that
returns a machine-readable catalog of all callable functions:

```json
{
  "engine_version": 1,
  "functions": [
    {
      "id": "fractal.sample",
      "name": "Fractal Point Sampler",
      "description": "Sample any registered fractal family over a parameter domain.",
      "parameters": [
        {
          "path": "fractal.view.fractal_type",
          "type": "enum",
          "options": ["newton", "explaino", "mandelbrot", "julia", ...],
          "default": "explaino",
          "required": true
        },
        {
          "path": "fractal.params.epsilon",
          "type": "float",
          "min": 1e-12,
          "max": 0.01,
          "step": 1e-06,
          "default": 1e-06,
          "applicable_when": {
            "op": "in",
            "path": "fractal.view.fractal_type",
            "value": "newton,nova,halley,explaino,..."
          }
        }
      ],
      "outputs": [
        { "name": "iterations", "type": "int" },
        { "name": "status", "type": "enum", "options": ["escaped","converged","bounded","pole","nonfinite","invalid_param"] },
        { "name": "final_z_x", "type": "double" },
        { "name": "final_z_y", "type": "double" },
        { "name": "final_abs2", "type": "double" },
        { "name": "residual", "type": "double", "nullable": true },
        { "name": "root_index", "type": "int", "nullable": true }
      ],
      "summary_metrics": [
        "mean_iterations", "escape_fraction", "converged_fraction",
        "nonfinite_fraction", "pole_fraction"
      ]
    }
  ]
}
```

Key rules:

- Derived from existing authorities (UI schema JSON + runtime structs), never a
  third editable source of truth.
- Parameter applicability predicates reuse the existing `visible_if` system.
- Output metrics are declared per function, not globally.
- Unknown function ids fail fast.

### Layer 2: Transpiler Bridge (CoreIR -> CUDA)

The Salticid transpiler already converts `CoreIR` to C++ via `cpp.py`. A new
`cuda.py` backend would:

1. Accept a `CoreIR` program (a `Compose` of `Call`/`Stencil`/`Apply` nodes).
2. Emit a **CUDA device function** instead of CPU C++.
3. Wrap the device function in a **host-callable kernel entry point** with a
   standard signature matching the probe runner's dispatch interface.
4. Generate a **function descriptor JSON** from the CoreIR parameter metadata,
   including parameter names, types, and ranges extracted from the operator
   specs.

The emitted CUDA kernel follows a standard contract:

```cpp
// Generated by transpiler bridge
__device__ void user_fn_001(
    double cx, double cy,           // sample point
    const UserFnParams& params,     // described parameter block
    int max_iter,                   // iteration budget
    SampleResult* out               // standard output struct
);

__global__ void user_fn_001_kernel(
    const double* coords_x,
    const double* coords_y,
    int n_points,
    const UserFnParams& params,
    int max_iter,
    SampleResult* results
);
```

The `UserFnParams` struct is generated from the function descriptor: one field
per described parameter, types matching the descriptor.

### Layer 3: Operator Wrapping

On the Salticid side, an operator adapter:

1. Calls `--describe-functions` to discover what the engine can sample.
2. Constructs a probe request using the function's described parameters.
3. Invokes the engine via the existing CLI contract.
4. Parses the response and presents it as a Salt operator output.

From the Salt pipeline:

```
input >> cuda_sample(fn="fractal.sample", params={epsilon: 1e-8, fractal_type: "nova"}) >> score
```

or for a transpiled function:

```
input >> cuda_sample(fn="user.my_kernel", params={alpha: 0.5, beta: 2.0}) >> analyze
```

Both use the same probe request contract. The engine doesn't care whether the
function was built-in or transpiled.

## Phased Staging

### Phase E1 — Describe surface for built-in fractal sampler

**Goal**: Engine can tell callers what it can sample and what parameters it takes.

- Add `--describe-functions` CLI verb to `fractal_ui.exe`.
- Generate function descriptor JSON from existing UI schema + runtime structs at
  startup (no new source of truth).
- The fractal sampler is the first (and initially only) described function.
- Headless test: parse the descriptor, validate it matches the UI schema.
- Exit criteria: `fractal_ui.exe --describe-functions` emits valid JSON that a
  Salticid adapter could consume to auto-construct probe requests.

### Phase E2 — Generic probe dispatch

**Goal**: The probe runner can dispatch to any described function, not just
hard-coded fractal families.

- Refactor `fractal_probe_runner.cpp` to use a function registry instead of a
  switch on `fractal_type`.
- Register the existing fractal sampler as the first provider in that registry.
- The probe request gains a `function_id` field (default: `"fractal.sample"`
  for backward compatibility).
- Unknown function ids fail fast.
- Exit criteria: existing probe CLI tests still pass with `function_id`
  defaulting to `"fractal.sample"`.

### Phase E3 — Transpiler CUDA backend (Salticid side)

**Goal**: The Salticid transpiler can emit CUDA device functions from CoreIR.

- Add `transpiler/backends/cuda.py` inheriting the `Backend` protocol.
- Start with a minimal subset: scalar math, conditionals, loops.
- Emit `__device__` functions with standard entry-point signature.
- Generate companion function descriptor JSON from CoreIR + operator spec
  metadata.
- Headless test: transpile a simple Salt operator to CUDA, verify it compiles.
- Exit criteria: `transpile_to_text(src, target='cuda')` emits compilable CUDA
  for at least one simple operator.

### Phase E4 — Dynamic function registration

**Goal**: The engine can load and register transpiled CUDA kernels at startup or
on demand.

- Define a plugin/module directory where compiled `.cubin` or `.ptx` files plus
  descriptor JSONs can be dropped.
- Engine scans the directory at startup, loads kernels, registers described
  functions.
- The probe runner can then dispatch to transpiled functions using the same
  request contract.
- Exit criteria: a transpiled kernel compiled from Phase E3 output can be
  placed in the plugin directory, appears in `--describe-functions`, and is
  callable via `--sample-request-stdin`.

### Phase E5 — Salticid operator adapter

**Goal**: Salt operators can call the CUDA engine as a black-box function
sampler.

- Python adapter in Salticid constructs probe requests from described-function
  metadata.
- Adapter handles subprocess management, JSON serialization, response parsing.
- Salt operator DSL surface: `cuda_sample(fn=..., params=...)`.
- Exit criteria: a Salt pipeline that includes a CUDA-backed operator runs
  end-to-end, with the engine handling the numeric evaluation.

### Phase E6 — End-to-end transpile-and-run

**Goal**: A Salt operator definition can be transpiled to CUDA, compiled,
registered, and invoked in one workflow.

- CLI or script orchestrates: transpile -> compile -> register -> sample.
- This is the "Salt operator from arbitrary code" workflow.
- Exit criteria: write a Salt operator, run a script, get CUDA-speed sampling
  results back through the standard probe contract.

## Design Constraints

### No third source of truth

- UI schema JSON is the UI layout authority.
- C++ structs are the runtime execution authority.
- Function descriptors must be derived from those, not hand-authored.
- Transpiler-generated descriptors derive from CoreIR + operator specs, which
  are the transpiler-side authorities.

### No implicit fallback

- Unknown function ids, parameter paths, or metric names fail fast.
- Transpiled kernels that fail to compile are not silently skipped — they are
  reported as registration errors.
- Callers that request a function not present in the descriptor get a clean
  error, not a default behavior.

### Standard kernel contract

Every callable function, built-in or transpiled, must:

- Accept sample points as `(cx, cy)` coordinate pairs (or a suitable
  generalization for higher-dimensional domains).
- Accept parameters via a described parameter block.
- Return results via a standard `SampleResult` struct.
- Report per-sample status using the existing enum vocabulary.

### Backward compatibility

- The existing probe CLI contract (Phases A-D) must continue working unchanged.
- `function_id` defaults to `"fractal.sample"` when absent.
- Existing tests must not break when the generic dispatch is added.

## Transpiler Bridge Details

### What changes in Salticid (handoff proposal, not direct edit)

| Component | Change |
|-----------|--------|
| `transpiler/backends/` | Add `cuda.py` backend |
| `transpiler/api.py` | Register `"cuda"` target in backend registry |
| `spec/operators.d/taxonomy.v1.json` | Add `"cuda_sample"` backend routing |
| New: `tools/cuda_bridge/` | Transpile -> compile -> register orchestration |
| New: `operators/cuda_sample.py` | Salt operator adapter for the engine |

### What changes in this repo (CUDA engine)

| Component | Change |
|-----------|--------|
| `main.cpp` | Add `--describe-functions` verb |
| New: `function_registry.h/.cpp` | Described-function registry |
| New: `function_descriptor.h/.cpp` | Descriptor types and JSON serialization |
| `fractal_probe_runner.cpp` | Refactor to dispatch via registry |
| `fractal_probe_contract.h/.cpp` | Add optional `function_id` to request |
| New: `plugin_loader.h/.cpp` | Load `.cubin`/`.ptx` + descriptor from directory |

### What the CUDA backend emits

Given a simple Salt operator:

```
mandelbrot_escape(power=2, max_iter=1000, cx=-0.75, cy=0.1)
```

The CUDA backend would emit:

```cuda
// Auto-generated from CoreIR
// Function ID: user.mandelbrot_escape_v1
// Parameters: power(int), max_iter(int), cx(double), cy(double)

struct mandelbrot_escape_v1_Params {
    int power;
    int max_iter;
    double cx;
    double cy;
};

__device__ void mandelbrot_escape_v1(
    double px, double py,
    const mandelbrot_escape_v1_Params& p,
    SampleResult* out)
{
    double zr = px, zi = py;
    int i = 0;
    for (; i < p.max_iter; ++i) {
        double zr2 = zr * zr, zi2 = zi * zi;
        if (zr2 + zi2 > 4.0) break;
        double tmp = zr2 - zi2 + p.cx;
        zi = 2.0 * zr * zi + p.cy;
        zr = tmp;
    }
    out->iterations = i;
    out->status = (i < p.max_iter) ? STATUS_ESCAPED : STATUS_BOUNDED;
    out->final_z_x = zr;
    out->final_z_y = zi;
    out->final_abs2 = zr * zr + zi * zi;
    out->residual = NAN;
    out->root_index = -1;
}
```

Plus companion descriptor:

```json
{
  "id": "user.mandelbrot_escape_v1",
  "parameters": [
    { "path": "power", "type": "int", "default": 2 },
    { "path": "max_iter", "type": "int", "default": 1000 },
    { "path": "cx", "type": "double", "default": -0.75 },
    { "path": "cy", "type": "double", "default": 0.1 }
  ],
  "outputs": [
    { "name": "iterations", "type": "int" },
    { "name": "status", "type": "enum" },
    { "name": "final_z_x", "type": "double" },
    { "name": "final_z_y", "type": "double" },
    { "name": "final_abs2", "type": "double" },
    { "name": "residual", "type": "double", "nullable": true },
    { "name": "root_index", "type": "int", "nullable": true }
  ],
  "source": "transpiled",
  "source_language": "salt",
  "kernel_entry": "mandelbrot_escape_v1_kernel"
}
```

## Scope Boundaries

### What this spec covers

- The described-function metadata surface (Layer 1).
- The transpiler CUDA backend concept (Layer 2).
- The operator wrapping pattern (Layer 3).
- Phased staging E1-E6.
- Design constraints and fail-fast rules.

### What this spec does NOT cover

- CUDA kernel optimization (occupancy, shared memory, warp divergence). That is
  a later concern once the bridge works at all.
- Multi-GPU dispatch or device selection beyond the existing `device_id`.
- Higher-dimensional sample domains beyond 2D `(cx, cy)`. The standard contract
  starts with 2D; generalization is a future extension.
- Runtime JIT compilation via NVRTC. Phase E4 assumes AOT `.cubin`/`.ptx`. JIT
  is a later convenience, not a V1 requirement.
- Security sandboxing of transpiled kernels. The trust model assumes all code
  loaded into the engine is trusted (same-machine, same-user).

## Open Questions

1. **Parameter domain generalization**: The current probe contract assumes 2D
   complex-plane sampling `(cx, cy)`. Transpiled functions may want 1D, 3D, or
   higher-dimensional domains. Should the standard contract be dimension-aware
   from the start, or should 2D be the V1 contract with a clear extension
   point?

2. **Iteration model**: Fractal sampling inherently iterates. Transpiled
   functions may be single-evaluation (no iteration loop). Should the standard
   `SampleResult` always include `iterations` and `status`, or should those be
   optional per function descriptor?

3. **Shared memory and state**: The current probe contract is stateless per
   request. Some transpiled kernels might benefit from persistent state (lookup
   tables, cached intermediate results). Is that in scope for V1 or deferred?

4. **Compilation target**: Should the CUDA backend emit `.cu` source for nvcc,
   or PTX for direct loading? `.cu` is simpler but requires nvcc at runtime;
   PTX is portable but harder to debug.

## Recommendation

Start with **Phase E1** (describe surface for built-in fractal sampler). It is
self-contained, testable, and immediately useful: a Salticid adapter can
auto-discover what the engine supports without hard-coding assumptions.

Phase E2 (generic dispatch) follows naturally and keeps the probe runner from
calcifying around fractal-only dispatch.

Phases E3-E6 are the transpiler bridge, which is cross-repo work. E1 and E2
can land in this repo independently and provide value even if the transpiler
bridge takes time to materialize.
