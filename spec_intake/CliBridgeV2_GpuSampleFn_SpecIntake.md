# CLI Bridge V2 + CUDA-Resident sample_fn — Spec Intake

Date: 2026-04-10 (revised 2026-04-09)
Status: Spec intake — refined with CUDA-resident constraint and Carl lineage context
Prior:
  - GenericCudaSamplerBridge_SpecIntake.md (E1-E2 done, E3-E6 deferred)
  - RealtimeCliSampling_OperatorCallIn_DesignNote.md (V1 contract landed)
  - fractal_probe_contract.h (request/response schema)
  - function_descriptor.h (describe surface)
  - salticid-cuda transpiler (CoreIR, Backend protocol, op_table — read-only ref)
  - salticid-cuda shared CUDA core extraction (Phases 1-6 complete, Phase 7 checkpoint)
  - salticid-cuda native_public_cuda_single_layer_PHASED_PLAN.md (frozen, ready)
  - REFLEXIVE_DISCOVERY_AND_CARL_LINEAGE.md (structural context)

Design constraint (from project owner, 2026-04-09):
  **sample_fn will be fully on CUDA.** Not a future handoff — the primary
  execution surface. The CLI session is a coordination layer; the kernel
  runs on device.

---

## 0. Situation

One execution surface, two access paths.

The fractal sample_fn is a **CUDA kernel** — it runs on the GPU, always. The
CLI session and the Salticid operator surface are both coordination layers
that invoke the same device-resident evaluation.

| Access path | Consumer | Coordination | Latency target | Contract |
|-------------|----------|--------------|----------------|----------|
| **CLI session (V2)** | External software, data streams, agent tooling, Explaino Reflexive sidecar | JSON over stdin/stdout or named pipe; host manages session state | <100ms round-trip (dominated by JSON serialization, not compute) | Superset of V1 probe contract |
| **Salticid operator adapter** | `.salt` pipeline operators | CoreIR -> shared CUDA core -> device invocation; no host round-trip | <1ms per sample batch | CoreIR function via shared CUDA core |

Both paths invoke the **same CUDA kernel**. The kernel already exists in this
repo (`fractal_renderer.cu`). What is new:
1. Extracting the iteration kernel into a callable device function (not just a
   full-frame renderer)
2. Wiring the CLI session to invoke that device function with arbitrary param
   overrides (V2 session protocol)
3. Exposing the device function through the shared CUDA core ABI so Salticid
   operators can call it without subprocess spawn

This is the Carl pattern applied to the probe surface: the kernel is the
behavior function, the CLI session is the perception/cognition coordination,
and the shared CUDA core is the guard that ensures both paths run the same
computation.

---

## 1. CUDA-Resident sample_fn (This Repo)

### 1.1 What it is

A CUDA `__device__` function extracted from the existing iteration kernels in
`fractal_renderer.cu`, callable at GPU speed with arbitrary parameter
coordinates and overrides.

Current state: The **iteration logic exists** (all 27+ fractal types) but is
entangled with the full-frame renderer's pixel-mapping, coloring, and output
pipeline. The extraction surface is the iteration loop itself.

Salticid-cuda inflight context: The shared CUDA core extraction (Phases 1-6)
is nearly complete. The native C API will gain `SALT_RUNTIME_BACKEND_SHARED_CUDA`
backend selection. The bridge interpreter already evaluates CoreIR expression
trees on CUDA device. This means the plumbing for device-to-device invocation
is landing in the sibling repo — this repo provides the kernel.

### 1.2 Architecture sketch

```
This repo provides:
  fractal_sample_device()         <-- NEW: extracted __device__ function
    - Takes: KernelParams subset, complex coordinate, iteration budget
    - Returns: SampleResult {iterations, status, root_index, residual, final_z}
    - Pure computation, no pixel mapping, no coloring, no framebuffer

  fractal_sample_kernel<<<>>>()   <-- NEW: thin launch wrapper
    - Grid of sample points -> calls fractal_sample_device() per thread
    - Output: device buffer of SampleResult

  fractal_sample_host_api()       <-- NEW: host-callable C function
    - Accepts JSON-compatible param struct, sample coordinates
    - Launches fractal_sample_kernel, reads back results
    - Used by CLI session (V2) and by shared CUDA core ABI

Salticid-cuda consumes:
  Shared CUDA core links fractal_sample_kernel as a static/object target
  Native adapter exposes it through salt_runtime_create_ex(SHARED_CUDA)
  Salt operator adapter calls device function directly (no host round-trip)
```

### 1.3 Binding vocabulary

The device function's parameter surface uses the same binding-path
vocabulary as the CLI bridge (`fractal.params.epsilon`, `fractal.view.fractal_type`,
etc.) so that:
- Describe surfaces are compatible
- Parameter override semantics are identical
- Test fixtures can probe either surface with the same request shape

### 1.4 Design decisions (resolved)

| Question | Decision | Rationale |
|----------|----------|----------|
| How does the device function receive parameters? | (a) Flat struct passed by value | KernelParams already exists; subset extraction is the natural fit |
| How are results returned? | (a) Per-thread output to device buffer | Same pattern as existing renderer; SampleResult is a small struct |
| How is the device function registered? | (a) Static compile-time linking | The kernel lives in this repo; dynamic loading deferred to E4 |
| How does Salticid invoke it? | Via shared CUDA core ABI (`salt_runtime_create_ex`) | Shared core extraction landing in salticid-cuda makes this the natural path |
| What iteration budget does the device function get? | (b) Caller-specified | Probe use cases need explicit budget control |

### 1.5 Phased plan (this repo — kernel extraction)

| Phase | Description | Exit criteria | Status |
|-------|-------------|---------------|--------|
| K1 | Extract `fractal_sample_device()` from renderer | Device function compiles, matches renderer output for all 37 types | **DONE** (e3edb17) |
| K2 | Add `fractal_sample_kernel<<<>>>()` launch wrapper + `SampleFractalPoints()` host API | Grid launch produces correct SampleResult buffer; host API manages device memory | **DONE** (e3ac63e, net -1403 lines) |
| K3 | *(Delivered in K2)* | `SampleFractalPoints()` landed as part of K2 | **DONE** |
| K4 | Headless equivalence test: sample API vs. renderer | 256x256 grid for all 37 types; results match within float epsilon; per-type diagnostics | **DONE** (0e7dc32 + bf857be) |
| K5 | Shared CUDA core linkage target (static lib / object) | salticid-cuda can link against fractal_sample_kernel | |

### 1.5.1 K4 diagnostic findings (investigation backlog)

Discovered via 256x256 × 37-type equivalence test (~2.4M sample points).

| ID | Finding | Priority | Notes |
|----|---------|----------|-------|
| KF-1 | **explaino_y**: 0 avg iterations, 100% convergence, residual up to 27 | **High** (correctness) | Suspicious degenerate early-exit; may be unconditional convergence at default params. Investigate before V2 work relies on sample correctness. |
| KF-2 | **collatz RGBA gap**: some escaped pixels render as black | Low (cosmetic) | Very-fast-escape (1-2 iters) produces near-zero coloring values. Not a sample-path bug. |
| KF-3 | **"neither" band**: 3-12% of pixels exhaust max_iter across escape types | Deferred (optimization) | Set interior pixels that neither converge nor escape — wasted compute. Natural early-bailout optimization target for a future phase. |
| KF-4 | **nova/explaino_nova**: 99.5% escape, 1 avg iter at default params | Low (parameter tuning) | Extremely parameter-sensitive. Default params may not showcase the fractal well. |
| KF-5 | **lambda_map/explaino_lambda**: 0 avg iters, 98.9% escape at default params | Low (parameter tuning) | Degenerate at defaults; investigate whether default view/params are representative. |

### 1.6 Salticid-side adapter (handoff, not this repo)

| Phase | Description | Owner repo |
|-------|-------------|------------|
| G1 | Shared CUDA core gains fractal sample as a linked target | salticid-cuda |
| G2 | Native adapter wraps fractal_sample_host_api in C ABI | salticid-cuda |
| G3 | Salt operator adapter (`@cuda_sample(...)`) calls device function | salticid-cuda |
| G4 | End-to-end: Salt operator -> shared core -> device eval -> result | both repos |

---

## 2. CLI Bridge V2 (This Repo)

### 2.1 What V1 has

The current CLI bridge (landed, tested) supports:

- `--sample-request-stdin` / `--sample-response-stdout` (pipe mode)
- `--sample-request-json` / `--sample-response-json` (file mode)
- `--describe-functions` / `--describe-functions-json` (catalog)
- Request modes: `point_set`, `grid`, `sequence_point_set`, `sequence_grid`
- Overrides via binding paths
- Metric selection (summary-only vs. full per-sample)
- Operator audit trail (source, operator_name, why)
- Fail-fast on unknown function_id, binding paths, enum values

### 2.2 What V2 adds

V2 is motivated by three new consumers:

1. **External data streams** (realtime sensor/audio/control data driving
   fractal parameters as a live visualization surface)
2. **Agent tooling** (automated exploration, parameter sweeps, finding capture)
3. **Explaino Reflexive sidecar** (the engine explaining itself — needs fast,
   lightweight demonstration calls for Bayesian parameter importance measurement)

### 2.3 V2 feature matrix

| Feature | V1 | V2 | Notes |
|---------|----|----|-------|
| Single request/response | yes | yes | Unchanged |
| Batch request array | no | **yes** | Array of requests in single invocation; avoids subprocess spawn overhead |
| Persistent session (keep-alive) | no | **yes** | Long-running process accepts request stream on stdin; critical for realtime |
| Streaming results | no | **yes** | Progressive output for large grids (NDJSON, one line per sample/sequence) |
| State carry-forward | no | **yes** | Response includes state_token; next request can reference it; avoids re-parsing |
| Parameter diff | no | **yes** | Request specifies only changed overrides vs. previous state_token |
| Cost/budget metadata | no | **yes** | Response includes gpu_ms, sample_count, estimated_cost |
| Describe with sensitivity | no | **yes** | Describe response includes param sensitivity hints (from Optimization Staging) |
| Named pipe / socket transport | no | **yes** | Alternative to stdin/stdout for concurrent callers |
| Binary sample output | no | **maybe** | For very large grids; NDJSON may be sufficient |

### 2.4 Session protocol (V2 keep-alive)

```
Client spawns: fractal_ui.exe --sample-session
Client writes to stdin:
  {"session": "open", "request_id": "init"}
Engine responds:
  {"session": "ready", "state_token": "s0", "engine_version": 2}

Client writes:
  {"request_id": "r1", "function_id": "fractal.sample", "overrides": [...], "region": {...}, "metrics": ["iterations"]}
Engine responds:
  {"response_version": 2, "ok": true, "state_token": "s1", "summary": {...}, "samples": [...], "cost": {"gpu_ms": 3.2, "sample_count": 65536}}

Client writes (diff mode):
  {"request_id": "r2", "state_token": "s1", "overrides": [{"path": "fractal.params.ripple_amplitude", "value": {"kind": "number", "number_value": 0.3}}], "metrics": ["summary_mean_iterations"]}
Engine responds:
  {"response_version": 2, "ok": true, "state_token": "s2", "summary": {...}, "cost": {"gpu_ms": 1.1, "sample_count": 65536}}

Client writes:
  {"session": "close"}
Engine exits cleanly.
```

### 2.5 Streaming output (NDJSON)

For large grids, V2 supports newline-delimited JSON:

```
Client writes:
  {"request_id": "r3", ..., "output_mode": "ndjson"}
Engine writes (one line per sequence step or grid row):
  {"type": "sample_batch", "sequence_index": 0, "samples": [...]}
  {"type": "sample_batch", "sequence_index": 1, "samples": [...]}
  ...
  {"type": "summary", "request_id": "r3", "summary": {...}, "cost": {...}}
```

### 2.6 Phased plan (this repo)

| Phase | Description | Exit criteria |
|-------|-------------|---------------|
| V2-A | Batch request array | Single invocation processes N requests; N responses returned in order |
| V2-B | Keep-alive session mode | `--sample-session` verb; open/close lifecycle; state_token carry-forward |
| V2-C | Parameter diff mode | Request with state_token + sparse overrides; only changed params re-applied |
| V2-D | Cost metadata in response | gpu_ms, sample_count in every response |
| V2-E | NDJSON streaming output | `output_mode: "ndjson"` for progressive results |
| V2-F | Describe with sensitivity | Describe response carries param sensitivity table from Optimization Staging |
| V2-G | Named pipe / socket transport | Alternative transport for concurrent external consumers |

### 2.7 Dependency ordering

```
V2-A (batch) ← no deps, pure contract extension
V2-B (session) ← V2-A (batch is natural inside session)
V2-C (diff) ← V2-B (needs state_token from session)
V2-D (cost) ← no deps, can land alongside any phase
V2-E (NDJSON) ← V2-A or V2-B
V2-F (sensitivity) ← Optimization Staging Phase 2
V2-G (named pipe) ← V2-B (session lifecycle must exist first)
```

---

## 3. Shared Contract Invariants (Both Surfaces)

These rules apply to both the GPU-resident device function and the CLI bridge:

1. **Same binding-path vocabulary**: `fractal.params.*`, `fractal.view.*`
2. **Same metric names**: `iterations`, `status`, `final_z_x/y`, `residual`, `root_index`
3. **Same fail-fast policy**: unknown paths, unknown enum values, invalid ranges → error, not silent fallback
4. **Same describe surface**: both consumers can call `--describe-functions` or equivalent to discover the parameter surface
5. **Same sample status enum**: `escaped`, `converged`, `bounded`, `pole`, `nonfinite`, `invalid_param`

## 4. This Repo's Deliverables vs. Salticid Handoffs

| Deliverable | Owner |
|-------------|-------|
| CLI Bridge V2 session protocol (V2-A through V2-G) | **This repo** |
| CUDA-resident `fractal_sample_device()` kernel extraction (K1-K4) | **This repo** (DONE) |
| Shared CUDA core linkage target (K5) | **This repo** |
| Describe surface extensions (cost, sensitivity) | **This repo** |
| Plugin loader for dynamic .cubin registration (E4, deferred) | **This repo** |
| Shared CUDA core integration + native adapter | **salticid-cuda** (G1-G2) |
| Salt operator adapter | **salticid-cuda** (G3-G4) |

## 5. Dependency ordering (revised)

```
K1 (extract device fn) <- no deps
K2 (launch wrapper) <- K1
K3 (host API) <- K2
K4 (equivalence test) <- K3
V2-A (batch) <- K3 (batch invokes host API)
V2-B (session) <- V2-A
K5 (shared lib target) <- K4 (proven correct first)
V2-C (diff) <- V2-B
V2-D (cost) <- K4 (gpu_ms comes from kernel timing)
V2-E (NDJSON) <- V2-A or V2-B
V2-F (sensitivity) <- Optimization Staging Phase 2
V2-G (named pipe) <- V2-B
G1 (salticid link) <- K5
```

The kernel extraction phases (K1-K5) are now the critical path. CLI V2 session
work (V2-A onward) depends on K3 being available as the execution backend.

## 6. Non-Goals

- No CPU fallback for sample_fn — CUDA is the execution surface, period.
- No breaking changes to V1 contract; V2 is additive.
- No web/HTTP transport (subprocess JSON is the primary contract).
- No authentication or multi-tenant isolation (single-user assumption).
- No NVRTC JIT in Phase 1 — static linking through shared CUDA core first.
