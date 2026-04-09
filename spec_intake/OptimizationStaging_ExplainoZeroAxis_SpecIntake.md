# Optimization Staging — Explaino Zero Axis

Date: 2026-04-10 (revised 2026-04-09)
Status: Spec intake — refined with CUDA-resident sample_fn constraint and Carl lineage context
Prior:
  - All 4 Explaino solver variants landed (ripple 7152f44, splice b2234a4, vortex 41a5509, tension f7d8f52)
  - ExplainoDesignSpace_DeepDive_2026-04-05.md (Axis B — solver dynamics)
  - param_anim_dynamics.cpp (BindFloat property registry)
  - CliBridgeV2_GpuSampleFn_SpecIntake.md (K1-K5: CUDA kernel extraction)
  - REFLEXIVE_DISCOVERY_AND_CARL_LINEAGE.md (structural context)

---

## 0. Motivation

All four new Explaino solver variants (ripple, splice, vortex, tension) share a
structural invariant:

> When their unique parameter equals zero, the kernel reduces to pure Explaino
> Newton iteration.

| Variant | Parameter | Zero value | Effect at zero |
|---------|-----------|------------|----------------|
| ripple  | ripple_amplitude | 0.0 | No sinusoidal kick; pure Newton step |
| splice  | splice_offset | 0.0 | P_A == P_B; alternating polynomial = identity |
| vortex  | vortex_strength | 0.0 | No rotation; pure Newton step |
| tension | tension_strength | 0.0 | No secondary-root pull; pure Newton step |

This **common zero axis** means:

1. The pure Explaino Newton kernel is a degenerate case shared by all variants.
2. Any variant can be smoothly interpolated to/from any other variant by first
   collapsing to the shared zero axis, then expanding along the target axis.
3. The zero axis is the natural optimization staging surface — measure, compare,
   and profile all variants against the common Newton baseline.

## 1. What Optimization Staging Means

### 1.1 Kernel cost profiling along the zero axis

Each variant has different per-iteration cost:

| Variant | Extra work per iteration | Expected relative cost |
|---------|-------------------------|----------------------|
| ripple  | sin, atan2, normalize | ~1.15x Newton |
| splice  | Second polynomial eval (5 coefficients) | ~1.3x Newton |
| vortex  | atan2, rotation (complex multiply) | ~1.1x Newton |
| tension | Distance to all 4 roots, sort for 2nd-closest | ~1.5x Newton |

**Phase 1A**: Add a micro-benchmark harness that runs each variant kernel at
`param=0` and at `param=default`, measuring:
- Mean iterations to convergence
- Wall-clock GPU time per 1M samples
- Register pressure (from `--ptxas-options=-v`)

This gives the cost-of-variant table needed by the Explaino Reflexive sidecar
and the probe runner.

**Key integration point**: Phase 1A's micro-benchmark harness is the natural
validation surface for the CUDA-resident sample_fn extraction (CLI Bridge V2
spec, Phase K4). If K1-K3 land first, Phase 1A becomes a trivial consumer of
`fractal_sample_host_api()`. If Phase 1A lands first, its benchmark surface
becomes the extraction target for K1. Either ordering works — they converge
on the same `__device__` function.

### 1.2 Convergence basin equivalence at zero

**Phase 1B**: Headless probe test that samples a 256x256 grid at `param=0`
for all four variants plus baseline Explaino, then asserts:
- Root indices match exactly
- Iteration counts match within +/-1
- Residuals match within float epsilon

This proves the zero axis invariant holds and that the kernel branches are
correctly gated. This test already partially exists in the continuity test
files (`test_explaino_*_continuity.cpp`) as the "Newton-match" case, but it
tests single points. The grid version is the proper regression.

### 1.3 Parameter sensitivity profiling

**Phase 2A**: For each variant, sweep its unique parameter from 0 to max in
N steps and measure:
- Mean iteration count (convergence cost)
- Escape fraction (basin stability)
- Nonfinite fraction (numerical health)
- Root index distribution entropy (morphology diversity)

This builds the **parameter sensitivity table** — the data the Explaino-All
sidecar needs to generate smart slider ranges.

## 2. What Optimization Staging Enables

### 2.1 Cost-aware variant suggestion (gamma term)

In the Explaino Reflexive framing (see ExplainoAll spec), this cost table
becomes the gamma term in the action-selection objective:
a* = argmax EIG(a) - gamma*Cost(a). The sidecar uses this data to balance
information gain against GPU cost when choosing which param to demonstrate.

Concretely: "Tension produces the most diverse morphology in this
region but costs 1.5x. Vortex is nearly as interesting at 1.1x."

### 2.2 Smooth variant crossfade

Because all variants meet at zero, a crossfade between any two variants is:

```
t in [0, 1]:
  param_A = default_A * (1 - t)
  param_B = default_B * t
```

At `t=0`: variant A at full strength.
At `t=0.5`: both params near zero — effectively Newton.
At `t=1`: variant B at full strength.

This is a natural animation path and a well-defined probe sweep axis.

### 2.3 Multi-variant composition

Nothing prevents activating multiple variant parameters simultaneously:

```
ripple_amplitude = 0.1
vortex_strength = 0.2
```

The kernel currently only dispatches one variant at a time (from the
fractal_type enum). A future **Explaino-Composed** kernel could apply all
perturbations in a single iteration loop, enabled by non-zero parameters.

This is Phase 3 work and requires kernel refactoring, but the zero-axis
invariant guarantees it is mathematically well-defined: any subset of
parameters at zero cleanly removes that effect.

In Carl terms: this is the transition from single-hat dispatch to multi-hat
fusion in the brain fold. Each variant parameter is a hat emitting a
perturbation proposal. The composed kernel is the Brain Hat integrating all
proposals in a single step. The zero-axis invariant is the guarantee that
removing a hat (setting its param to zero) does not break the system.

## 3. Phased Plan

### Phase 1 — Measure (no kernel changes)

| Step | Description | Exit criteria |
|------|-------------|---------------|
| 1A | Add `test_explaino_zero_axis_equivalence.cpp` grid test | 256x256 grid matches baseline for all 4 variants |
| 1B | Add `benchmark_explaino_variants.cu` GPU micro-benchmark | Table of {variant, param_value, mean_iters, gpu_ms_per_1M} |
| 1C | Add `run_explaino_param_sensitivity.py` probe sweep script | CSV: variant x param_value x {mean_iters, escape_frac, nonfinite_frac, entropy} |

**Integration note**: 1A and 1B can share the CUDA-resident sample_fn if K1-K3
from CLI Bridge V2 land first. If not, they use direct kernel launch (same
code, just not yet extracted into the callable device function). Either way,
the measurement data is identical.

### Phase 2 — Expose (probe/descriptor changes)

| Step | Description | Exit criteria |
|------|-------------|---------------|
| 2A | Extend `FunctionParamDescriptor` with `cost_hint` field | Describe surface reports estimated relative cost per variant |
| 2B | Add `variant_crossfade` sequence mode to probe contract | Probe request can sweep between two variants via zero axis |
| 2C | Add `multi_metric` sensitivity report to describe surface | Machine-readable param sensitivity table for each variant |

**Dependency**: 2A-2C require CLI Bridge V2 Phase V2-B (keep-alive session)
for efficient repeated probing. Phase K3 (`fractal_sample_host_api`) is the
execution backend.

### Phase 3 — Compose (kernel changes)

| Step | Description | Exit criteria |
|------|-------------|---------------|
| 3A | Design `explaino_composed` kernel that reads all variant params | Spec + test contract, no implementation |
| 3B | Implement composed kernel with gated perturbation chain | All variant params active simultaneously; zero params = no work |
| 3C | Schema + probe + describe surface for composed variant | Full pipeline: UI, describe, probe, animate |
| 3D | Extract composed kernel into `fractal_sample_device()` | K1-K5 updated to include composed variant |

## 4. Dependencies

- Phase 1 has no hard external dependencies. Uses existing build/test/probe scripts.
  Optionally consumes K1-K3 if they land first (recommended).
- Phase 2 depends on CLI Bridge V2 Phases V2-B + K3 (`fractal_sample_host_api`).
- Phase 3 depends on Phase 2, K1-K5 (kernel extraction must be stable), and
  Explaino Reflexive sidecar for UX validation.
- The zero axis IS the reversibility/re-rooting guarantee from the Explaino
  math (Explain-o Math v1.1) and Carl's invariant-checking pattern
  (`contradiction_global` in the hat-rack formalization).
- The cost table from Phase 1 IS the gamma term in the Reflexive sidecar's
  action-selection objective: argmax EIG(a) - gamma*Cost(a).

## 5. Critical path integration

The recommended implementation order across all three specs:

```
K1-K3 (extract CUDA sample device fn)    <- enables everything
  |
  +-- Phase 1A-1C (measurement)          <- proves correctness + gets cost data
  |     |
  |     +-- Phase 2A-2C (expose)         <- describe surface gains cost/sensitivity
  |           |
  |           +-- R1-R2 (Reflexive)      <- sidecar consumes describe + probe
  |
  +-- V2-A thru V2-G (CLI session)       <- session protocol over sample host API
  |     |
  |     +-- R2-R3 (Reflexive)            <- sidecar uses session for demonstrations  
  |
  +-- K4-K5 (equivalence test + lib)     <- enables salticid-cuda integration
        |
        +-- G1-G4 (salticid adapter)     <- Salt operators call device fn
        |
        +-- Phase 3A-3D (composed)       <- multi-variant kernel
```

## 5. Non-Goals

- No new fractal types in Phase 1 or Phase 2.
- No kernel performance optimization (CUDA occupancy tuning, etc.) — just measurement.
- No UI changes in Phase 1.
- The composed kernel (Phase 3) is explicitly deferred until measurement proves it useful.
