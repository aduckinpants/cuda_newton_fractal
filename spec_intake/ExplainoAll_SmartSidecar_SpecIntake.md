# Explaino Reflexive — The Engine Explaining Itself

Date: 2026-04-10 (revised 2026-04-09)
Status: Spec intake — refined with CUDA-resident constraint and Carl lineage context
Prior:
  - Explain-o Math v1.1 (c:\code\Explain-o Math v1.1.md) — Bayesian/invariant framework
  - Explain-o Field System 2617 (c:\code\Explain-o Field System 2617.md) — recursive_state, divergence, SDF, energy, slime
  - ExplainO CPU Orientation Vector (c:\code\ExplainO CPU Orientation Vector.md) — 7-component structured state
  - OptimizationStaging_ExplainoZeroAxis_SpecIntake.md (zero axis measurement)
  - CliBridgeV2_GpuSampleFn_SpecIntake.md (CUDA-resident sample_fn + CLI session)
  - param_anim_dynamics.cpp (BindFloat resolution, no enum dispatch)
  - schema_binding.cpp (29-30 bindable float params, BindingContext)
  - function_descriptor.cpp (describe surface with param metadata)
  - ExplainoDesignSpace_DeepDive_2026-04-05.md (design axes A-G)
  - Carl / CarlOS V1 / hat-rack architecture (c:\code\wizard-test) — origin
  - REFLEXIVE_DISCOVERY_AND_CARL_LINEAGE.md (structural lineage)

---

## 0. The Shape

The sidecar is not a "smart parameter UI." It is Explaino applied to itself.
More precisely: it is the CarlBrain pattern instantiated as a single-tick
exploration agent over the fractal engine's parameter manifold.

In Carl terms:
- **Perception** = describe surface + probe results (what the engine looks like right now)
- **Cognition** = EIG computation + action selection (what to demonstrate next)
- **Behavior** = parameter change + GPU kernel invocation (the demonstration)
- The sidecar does NOT iterate (no Y combinator) — it runs one
  Perception -> Cognition -> Behavior pass per user interaction, exactly like
  Explaino runs one ExplainStep per explanation request.

The fractal engine is a concrete instance of the Explaino math framework
(Explain-o Math v1.1). Every structural element of the Explaino formalism maps
directly onto the engine's own architecture:

| Explaino Math (v1.1) | Engine Self-Application |
|---|---|
| Hypothesis space H | Parameter manifold: 29+ bindable floats x variant enum |
| Prior P_0(H) | Schema defaults (declared in describe surface) |
| Demonstration D_k | One probe call with a parameter perturbation |
| Information gain IG_k = D_KL(P_k, P_{k-1}) | Output delta from param nudge |
| Lens operator l_theta | Active zone projection: adaptive slider range |
| Action selection: argmax EIG - gamma*Cost | "Which param to sweep next?" where Cost = GPU time |
| Reversible path to root | Zero axis: any variant at param=0 -> pure Newton |
| Hamiltonian-connectedness | Variant crossfade through zero axis: every region reachable |
| No dead ends, no uncomputable states | Every param value in declared range produces a valid render |
| Closed under allowed operators | Parameter composition stays in the describable manifold |

And the Explaino Field System (2617) instantiates directly:

| Field System Operator | Engine Self-Application |
|---|---|
| recursive_state(text) | {ViewState, KernelParams}: the engine's current structured state |
| scalar_divergence(A, B) | Output delta between two parameter configurations |
| structural_score(state) | Morphology metric: convergence fraction, root index entropy |
| sdf(state) | Basin boundary geometry from the rendered fractal |
| energy(...) | Combined importance + cost landscape over the param manifold |
| slime_trace(...) | Auto-explorer's path through parameter space |

The zero axis is the formal "guaranteed re-rooting" invariant from the math:
every variant's unique parameter has a zero value that returns to the shared
Newton base state. This is not a UI convenience — it is the Hamiltonian-
connectedness property of the explanation-state manifold, instantiated in the
kernel architecture.

The describe surface is the hypothesis space made explicit. It does not merely
list parameters for convenience; it IS the structural declaration of H that
the Explaino math requires. This is why it must derive from the schema (single
source of truth) — a second editable source would break the invariant.

## 1. What This Changes From the Initial Draft

The initial "Smart Sidecar" draft was designed as an engineering convenience:
measure parameter importance, sort sliders, show cost badges. That design was
correct mechanically but missed the structural identity.

### What stays the same
- Describe-surface discovery (Schema -> param list)
- Micro-sweep measurement (probe calls -> importance scores)
- Adaptive slider ranges
- Zero-axis crossfade controls
- Cost annotations from Optimization Staging

### What changes

| Old framing | New framing |
|---|---|
| "Parameter importance score" | Expected information gain: EIG(a) = E[D_KL(posterior, prior)] |
| "Adaptive slider range" | Lens projection l_theta: the subspace that preserves mutual information with morphology |
| "Auto-exploration mode" | Active Bayesian action selection: a* = argmax EIG(a) - gamma*Cost(a) |
| "Micro-sweep" | Demonstration sequence D_1, ..., D_n |
| "Ranked param list" | Posterior belief state: which params have been demonstrated, which remain uncertain |
| "Cost badge" | The gamma term in the action-selection objective |
| "Zero axis reset" | Reversible path to root: re-rooting guarantee |
| "Variant crossfade" | Navigation on the Hamiltonian-connected explanation-state manifold |

### New capabilities this framing adds

1. **Demonstration budget**: The sidecar tracks total information delivered
   (sum IG_k = D_KL(P_n, P_0)) and can report "how well explored" the current
   parameter neighborhood is. This is the entropy reduction metric from the
   math: H[P_0] - H[P_n].

2. **Information efficiency**: Cost(demonstrations) / IG_total. The sidecar
   can compare exploration strategies: "sweeping ripple_amplitude gave 3x more
   information per GPU-ms than sweeping epsilon."

3. **Exploration completeness**: The sidecar knows which params have NOT been
   demonstrated near the current view. These are the high-uncertainty regions
   of the hypothesis space — the natural next targets.

4. **Structural divergence field**: When the user changes fractal type or seed,
   the sidecar computes scalar_divergence between old and new states. This is
   the Field System's divergence operator applied to the engine's own state
   transitions.

5. **Energy landscape visualization**: The combined importance/cost surface
   over the param manifold is literally the energy(...) function from the Field
   System. The sidecar can render this as a heatmap or profile.

## 2. Architecture

### 2.1 The reflexive loop

All demonstration calls go through the CUDA-resident `fractal_sample_device()`
(see CLI Bridge V2 spec, Phases K1-K3). The sidecar does NOT spawn subprocesses
or use the CLI bridge for its own micro-sweeps — it calls the sample host API
directly from the same process, at device speed.

The CLI session (V2-B keep-alive) is available for external consumers, but the
sidecar is an in-process consumer of the extracted kernel.

```
Engine state S = {ViewState, KernelParams}
  |
  v
recursive_state(S) --> structured param representation
  |
  v
Describe surface --> hypothesis space H with declared ranges, defaults, predicates
  |
  v
fractal_sample_host_api() --> demonstration sequence D_1..D_n (on CUDA, <1ms)
  |
  v
Information gain computation --> EIG per param, posterior update
  |
  v
Lens projection --> active zone per param (preserves MI with morphology target)
  |
  v
Action selection --> "which param to demonstrate next" (argmax EIG - gamma*Cost)
  |
  v
Presentation: ranked params, adaptive sliders, exploration budget, divergence field
  |
  v
User interaction --> param change --> S updates --> loop
```

### 2.2 The sidecar state IS an Explaino orientation vector

The CPUTransformOrientationV1 format (7 components) maps onto the sidecar's
internal state:

| Orientation component | Sidecar meaning |
|---|---|
| import_signature | Hash of current fractal_type + seed: "where in design-space this engine starts" |
| pack_projection_hash | Hash of applicable-param subset: "structural fingerprint of active param surface" |
| field_embedding_stats | Mean param importance: "how complex the current exploration surface is" |
| slime_energy_delta | Information gain from last demonstration batch: "how much we learned" |
| busy_beaver_metrics | Exploration richness: unique morphology classes seen / total probes |
| decode_stability | Divergence between predicted and actual output: "is our model of the param space accurate?" |
| diff_magnitude | Total param change since last orientation: "how far we've moved" |

This orientation vector is the sidecar's self-description. It can be saved,
compared, and replayed — connecting to the FITS solution-space playback idea.

### 2.3 What the sidecar renders

The sidecar is a secondary ImGui window that shows:

1. **Information budget panel**: Total IG delivered, per-param IG breakdown,
   info-efficiency ratio. Like a fuel gauge for exploration.

2. **Ranked param sliders**: Sorted by EIG (most informative at top).
   Slider extent = lens projection (active zone). Dead zones grayed out.
   Cost badge = gamma annotation from Optimization Staging.

3. **Exploration map**: Which params have been demonstrated (low uncertainty)
   vs. unexplored (high uncertainty). Simple heatmap or bar chart.

4. **Divergence indicator**: When the user changes fractal type or seed,
   shows the scalar_divergence magnitude of the state transition.

5. **Zero-axis control**: One slider that navigates the Hamiltonian-connected
   manifold between any two variants via the reversible zero-axis path.

6. **Orientation vector display**: The 7-component orientation of the current
   exploration state, shown as a compact badge or sparkline.

## 3. Phased Plan

### Phase R1 — Reflexive skeleton (Field System instantiation)

| Step | Description | Exit criteria |
|------|-------------|---------------|
| R1-A | Define SidecarOrientationVector struct (7 components mapping above) | Struct compiles, hash functions work |
| R1-B | Parse describe-functions catalog into hypothesis space representation | Sidecar discovers params, ranges, predicates programmatically |
| R1-C | ImGui window showing param list sorted by declared range width | Correct params displayed for each fractal type |
| R1-D | Compute orientation vector from current engine state | 7-component vector updates on state change |

### Phase R2 — Demonstration engine (CUDA-direct measurement)

| Step | Description | Exit criteria |
|------|-------------|---------------|
| R2-A | Wire sidecar to call `fractal_sample_host_api()` directly | Sidecar can invoke sample kernel in-process; no subprocess spawn |
| R2-B | Implement micro-sweep as demonstration sequence | +/-step for each applicable param; compute output deltas at device speed |
| R2-C | Compute EIG per param from demonstration results | Params ranked by expected information gain |
| R2-D | Track posterior state: cumulative IG, per-param uncertainty | Information budget panel functional |

### Phase R3 — Lens and action selection

| Step | Description | Exit criteria |
|------|-------------|---------------|
| R3-A | Compute lens projection (active zone) per param | Slider extent matches measured active zone |
| R3-B | Implement action selection: argmax EIG - gamma*Cost | Sidecar recommends next param to explore |
| R3-C | Auto-demonstration mode: sidecar runs action selection loop | Autonomous exploration with user-visible param changes |
| R3-D | Exploration completeness display | Map of demonstrated vs. uncertain params |

### Phase R4 — Full Field System instantiation

| Step | Description | Exit criteria |
|------|-------------|---------------|
| R4-A | Divergence field: state transitions produce scalar_divergence | Divergence indicator functional on fractal_type/seed change |
| R4-B | Energy landscape: combined importance/cost surface rendering | Heatmap or profile of param-space energy |
| R4-C | Slime trace: record auto-explorer path through param space | Path visualization overlaid on energy landscape |
| R4-D | Orientation vector persistence: save/load/compare exploration states | FITS-compatible round-trip of sidecar orientation |

## 4. Dependencies

| Phase | Depends on |
|-------|------------|
| R1 | Describe surface (landed, E1). No external deps. |
| R2 | CUDA-resident sample_fn: K1-K3 from CLI Bridge V2 spec (kernel extraction + host API) |
| R3 | R2 complete + Optimization Staging Phase 2 (cost data) |
| R4 | R3 complete. FITS round-trip is stretch goal. |

The critical enabler for R2 is the kernel extraction (K1-K3), not the CLI session
protocol. The sidecar is an in-process consumer — it calls the CUDA sample
function directly. The CLI session exists for external consumers (agents, data
streams, tooling).

## 5. Why This Matters Beyond the Sidecar

This is not just a UI feature. It is a proof that Explaino's mathematical
framework is self-hosting: the system designed for explaining things can
explain itself.

If this works, the same reflexive pattern applies to:
- Any Salticid operator explaining its own parameter surface
- Any CUDA kernel explaining its sensitivity landscape
- The transpiler explaining its own lowering decisions
- The runtime explaining its own performance characteristics

The fractal engine is the first concrete test case because it already has all
the required seams: described parameters, a probe surface, a cost model, and
a zero-axis reversibility guarantee. But the pattern is general.

The one-line takeaway from Explain-o Math v1.1 applies exactly:

> Explain-o is an active Bayesian/invariant-respecting system: choose
> representations and demonstrations that maximize useful belief updates
> while staying inside a globally navigable, reversible, computable
> explanation-state space.

The fractal engine's parameter manifold IS a globally navigable, reversible,
computable explanation-state space. The sidecar IS the active Bayesian agent.
The probe bridge IS the demonstration mechanism. The describe surface IS the
hypothesis space declaration.

## 6. Non-Goals

- Not a general-purpose Explaino Field System implementation (that lives in Salticid proper).
- Not a replacement for the main viewer (both coexist).
- Not an ML/inference system — measurement is empirical, not predicted.
- Not a symbolic math engine — the Bayesian vocabulary is structural framing,
  not a literal probabilistic inference implementation. EIG is measured by
  probe deltas, not computed from distribution objects.
- Not trying to reproduce the full Field System 2617 pipeline — the mapping
  is structural, using the same pattern at the C++ engine level.
