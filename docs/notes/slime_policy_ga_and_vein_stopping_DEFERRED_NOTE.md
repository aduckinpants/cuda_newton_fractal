# Deferred Note: Slime Policy GA And Vein Stopping

Status: deferred research and harness work. Do not implement under current SDF or Color Pipeline slices.

## Summary

The promising idea is a genetic algorithm over slime behavior policy, not over final fractal parameters.

The slime should still perform its normal contract-driven information walk. The genetic algorithm would tune how the slime chooses and stops moves: which parameter to inspect, how aggressively to step, when to revisit, when to switch veins, and when to stop. This preserves the sidecar contract boundary: the slime sees the public parameter surface, measurement rows, information landscape, and replayable mutation trail; the GA only searches over strategy policy.

The related stopping feedback is the same family of work but should be documented as a separate first-class concern: the current sidecar can find and follow an information vein, but it is weak at detecting when the local vein is exhausted.

## Existing Repo Surfaces

Known current seams:

- `ui_app/src/explaino_sidecar_controller.h`
  - `SidecarAutoDemoControllerPolicy`
  - `SidecarAutoDemoMutationRecord`
  - paced-loop apply policy
- `ui_app/src/explaino_sidecar_action.h`
  - `SidecarActionRecommendation`
  - utility, information gradient, information curvature, uncertainty, stability, cost hint
- `ui_app/src/explaino_sidecar_measurement.h`
  - `SidecarMeasurementRow`
  - aggregate information gain, counterfactual witness, decode stability
- `ui_app/src/runtime_walk_field_slime.h`
  - `RuntimeWalkFieldSlimeConfig`
  - world-space slime movement policy knobs

Important distinction: Explaino sidecar policy and runtime-walk field slime policy are related but not identical. The first is parameter-space exploration. The second is world/image-field motion. A future campaign should keep them separate until both are proven independently.

## Genetic Policy Candidate

A first sidecar strategy genome could include:

- step-size multiplier
- exploration rate
- trail memory decay
- revisit penalty
- revisit bonus after landscape change
- gradient-follow weight
- curvature-follow weight
- novelty weight
- stability weight
- cost penalty weight
- parameter-subset priors
- mutation scale by parameter type
- patience before switching parameter
- max steps per local vein
- low-gain plateau window
- oscillation numeric-band threshold
- dormant-parameter reactivation threshold

A first runtime-walk field-slime strategy genome could include:

- finite-difference step
- step scale
- gradient sensitivity
- hysteresis
- min/max marble count
- adaptive marble pressure weights
- cluster-spread response
- export cadence
- dead-marble reseed policy

## Stopping And Switching Layers

Stopping should not mean "parameter is weak now." It should mean "the current local vein is exhausted under the current landscape."

Suggested layers:

1. `stopping_v0`: hard step/time budget.
2. `stopping_v1`: utility plateau detector: last N moves have low marginal gain.
3. `stopping_v2`: oscillation detector: same parameter bounces inside a tiny numeric band.
4. `stopping_v3`: vein exhaustion detector: a once-rich parameter now has decaying marginal gain.
5. `stopping_v4`: switch-before-stop: try another exposed parameter before ending the run.
6. `stopping_v5`: reactivation-aware stop: dormant parameters can become rich after the path changes.

The B-trace damping tail described by the user is the target symptom: repeated narrow bracketing around `explaino_damping` with low but nonzero utility should switch or go dormant instead of consuming the rest of the run.

## Fitness Shape

Do not optimize for raw entropy alone. It can reward noise.

Candidate fitness:

```text
 useful_information_gain
+ stable_novelty_gain
+ useful_gradient_discovery
+ late_parameter_reactivation_score
+ coverage_improvement
+ structure_stability
- noise_penalty
- degeneracy_penalty
- excessive_jump_penalty
- replay_failure_penalty
```

The special score is `late_parameter_reactivation_score`: reward a parameter that was low-value early but became useful after the slime changed the landscape.

Hard gate:

```text
if mutation trail cannot replay forward from the same start:
  fitness = 0
```

True backward replay should be treated as a stronger later gate, not assumed from the current mutation record.

## Contradictions And Show-Stoppers

- Current `SidecarAutoDemoMutationRecord` is not strong enough for a rigorous golden-thread proof. It stores label/path/type/target/utility, but not previous value, pre/post state hash, genome id, scene id, RNG seed, measurement hash, step index, or selection rationale.
- A GA can easily overfit to a small scene matrix or reward visual noise. Fitness must include stability and degeneracy penalties.
- Live autonomous mutation is too risky for v0. This should begin headless/offline and emit reports.
- Sidecar policy GA and runtime-walk field slime GA can be confused because both use "slime" language. Keep their genomes and evaluation scenes separate.
- Measurement cost may explode as `population * generations * scenes * repeats * steps * sidecar samples`.
- If the GA can see hidden state or bypass the public contract, it invalidates the architecture.

## Future Slice Shape

Suggested future order:

1. Trail receipt hardening.
   - Add enough immutable metadata to replay and audit a mutation path.
   - Keep old state load compatible.
2. Stopping policy v1-v3.
   - Plateau, oscillation, and local vein exhaustion detectors.
   - No GA yet.
3. Offline sidecar policy evaluator.
   - Fixed scene matrix, fixed seeds, deterministic reports.
4. Minimal GA v0.
   - Small population, small generations, no live viewer mutation.
5. Runtime-walk field slime evaluator.
   - Separate from sidecar parameter policy.
6. Productization only if offline reports are boringly deterministic.

## Proof Gates For Any Future Implementation

- Native tests for policy validation and stopping detectors.
- Replay test that fails if a trail cannot be reproduced from the same start state.
- Fixed-seed evaluator output stable across repeated runs.
- Scene matrix includes at least one trace with a known damping-tail exhaustion symptom.
- Fitness report breaks down gain, novelty, reactivation, stability, penalties, and replay status.
- No physical mouse automation.
- No live mutation path until the headless evaluator is proven.

