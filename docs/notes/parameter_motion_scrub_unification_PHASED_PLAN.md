# Parameter Motion And Focus-Owned Scrub Unification Campaign

Status: planning draft reviewed and rejected as implementation-ready; product mutation is paused pending operator discussion and reconciliation.

## Current Phase

The fresh blind hostile review is recorded without silent amendment. The current phase is operator discussion of the findings, followed by a bounded reconciliation pass. Implementation must not begin from this draft.

## Explicit User Asks

- [x] Replace the fragile seed-only arrow scrub with a general mechanism for eligible normal numeric fractal/view parameters.
- [x] Prevent arrow-key scrubbing while a user edits an ordinary numeric or text control.
- [x] Separate target selection, scrub quantum, and motion rate from activation so configuration changes never mutate the target value.
- [x] Replace the weak existing animation controls with one small coherent Parameter Motion panel instead of adding another overlapping system.
- [x] Keep Color Pipeline row/function animation out of v1 and defer it toward a future programmable Salticid-oriented surface.
- [x] Reuse the useful behavioral ideas from the digit-scrub POC without importing its browser widget, arbitrary-base model, or source-patching machinery.
- [x] Define Capture Finding and state-load behavior now: configuration reloads, active motion never resumes automatically, and the loaded state settles once.
- [x] Preserve a future path to a versioned external runtime-state interface without implementing IPC in this campaign.
- [x] Review this plan through a fresh blind hostile subagent and discuss findings before implementation.

## Phase Checklist

- [x] Phase 0 - Inspect current scrub, animation, binding, state, pacing, and automation seams.
- [x] Phase 1 - Write the planning campaign and planning-only machine contract.
- [x] Phase 2 - Record the external runtime-state interface deferment and backlog truth.
- [x] Phase 3 - Run and record a fresh blind hostile subagent review.
- [ ] Phase 4 - Discuss findings and revise or accept the plan and implementation slice contracts.
- [ ] Pause before product mutation.

## Scope

This campaign replaces three partially overlapping authorities:

1. the global `ApplyArrowKeySeedScrub` key polling path;
2. `ApplyExplainoSeedDynamics` and `auto_increment_seed`;
3. `ApplyParamAnimDynamics` and the hard-coded `param_anim_target` option list.

The replacement is one provider-backed Parameter Motion system for eligible numeric controls on the normal fractal/view binding surface. It owns target discovery, focused scrub actions, continuous motion, boundary handling, state migration, reports, and interaction lifecycle.

### In Scope

- Numeric `float32`, `float64`, and integer parameters exposed through normal schema/binding authority.
- A data-driven eligibility/capability provider.
- A compact Parameter Motion UI panel.
- Focus-owned left/right scrub, explicit decrement/increment commands, and bounded horizontal drag.
- One continuous motion engine with a separate enable toggle.
- Scoped seed action migration onto the same step provider.
- State/capture/replay configuration semantics and legacy state migration.
- Native and published no-mouse regression matrices.

### Explicitly Out Of Scope

- Color Pipeline row/function animation or graph animation.
- Salticid execution or a programmable animation language.
- Arbitrary bases, exact rational editing, AST/source patching, or a browser-style per-digit strip.
- Enum, boolean, device, resolution, backend, capture, diagnostics, or other operational controls unless a later contract explicitly classifies one.
- Multiple simultaneous motion tracks, timelines, keyframes, curves, easing libraries, MIDI/OSC, or recording.
- A public external state IPC endpoint.
- Physical mouse automation.

## Repo-Grounded Starting Point

- `ApplyArrowKeySeedScrub` polls global left/right key state in `main.cpp`; it does not require a scrub control to own keyboard focus.
- `ApplyExplainoSeedDynamics` is a separate seed-only runtime path gated by `auto_increment_seed`.
- `ApplyParamAnimDynamics` is a second runtime path whose non-`none` target is also its activation state.
- The generic animation path resolves only float bindings and still has a special seed branch.
- `param_anim_target` is a hand-maintained schema option list, so eligibility is not derived from current lane/binding authority.
- Existing state capture persists at least the legacy seed-auto-increment fields, while generic motion configuration is not a complete replay contract.
- The runtime already has interaction/preview pacing, schema binding, visible-control reporting, no-mouse commands, and state/capture seams that this campaign must reuse.

## Product Invariants

1. Arrow keys never mutate a parameter unless the dedicated scrub surface owns focus and no text/numeric edit is active.
2. Selecting a target never changes its value.
3. Changing quantum, rate, direction, or boundary policy never changes its value.
4. Enabling motion is an explicit command distinct from selecting a target.
5. Exactly one runtime engine owns continuous parameter motion.
6. All mutations use the same typed provider and normal authoritative setter path.
7. A visible Parameter Motion target is writable, active for the current lane, and honestly classified as scrubbable and/or motion-animatable.
8. A target that becomes inactive or invalid stops motion before another mutation.
9. State load never resumes motion automatically.
10. Capture/replay parameter pixels remain authoritative; motion configuration is secondary control state.
11. The existing interaction preview/debounce path receives begin/update/end lifecycle events and produces one settled full-quality frame.
12. Color Pipeline remains outside the target catalog in v1.

## Authority Model

### Parameter Capability Descriptor

Introduce a read-only descriptor generated from current schema and binding authority, not a new hand-maintained target enum.

Required fields:

- stable `binding_path`;
- display label and owning section;
- owning selector/family visibility predicate;
- numeric storage kind: `float32`, `float64`, or bounded integer;
- `binding_resolves`;
- `writable`;
- `visible`;
- `active_for_current_lane`;
- `scrubbable`;
- `motion_animatable`;
- hard and UI bounds where defined;
- canonical schema step;
- allowed boundary policies;
- structured exclusion/fail-closed reason;
- semantic owner and change-notification route.

These fields are independent. In particular, a metadata flag analogous to `safe_to_scrub` must not double as proof that a runtime binding resolves or that live mutation is authorized.

### Typed Accessor

The provider exposes typed, checked operations:

- `DescribeEligibleTargets(context)`;
- `ReadNumericValue(binding_path, context)`;
- `PrepareNumericMutation(binding_path, requested_value, boundary_policy, context)`;
- `CommitPreparedNumericMutation(prepared, change_source)`.

Preparation performs binding lookup, active-lane validation, finite checks, type conversion, and boundary handling before authoritative mutation. Commit must be non-failing after successful preparation and must emit the existing dirty/interacted notification exactly once.

No raw pointer retained across frames is authoritative.

### Eligibility

A target appears in the v1 selector only when all are true:

- normal schema/binding path exists;
- numeric type is supported;
- binding resolves in the current context;
- writable is true;
- active for the current lane is true;
- at least one of `scrubbable` or `motion_animatable` is true;
- owner is not Color Pipeline or an excluded operational domain.

Hidden targets may remain selected in loaded legacy configuration for review, but they are reported unavailable and cannot mutate.

## Parameter Motion Panel V1

The old global animation block is removed from the normal UI and replaced with one small panel:

```text
Parameter Motion
  Target               [eligible parameter dropdown]
  Current Value        [focusable scrub surface]
  Quantum              [base step x 10^exponent]
  Step                  [-] [+]
  Boundary              [Clamp / Wrap / Reflect when supported]
  Motion Enabled       [toggle]
  Motion Rate          [signed value units per second]
```

### Stable Control IDs

- `parameter_motion.target.primary`
- `parameter_motion.value.scrub.primary`
- `parameter_motion.quantum_exponent.primary`
- `parameter_motion.step.decrement`
- `parameter_motion.step.increment`
- `parameter_motion.boundary_policy.primary`
- `parameter_motion.enabled.primary`
- `parameter_motion.rate.primary`

### Target Selection

- Selection changes configuration only.
- It reads and displays the current authoritative value.
- It does not enable motion.
- If the previously selected target is absent on a lane change, configuration remains visible as unavailable while motion is forced off.
- The dropdown ordering is deterministic: owning section order, schema order, then stable binding path.

### Quantum

- Each descriptor supplies a positive canonical base step.
- `quantum_exponent` is an integer in `[-9, 9]`.
- Effective scrub quantum is `base_step * 10^quantum_exponent`.
- Integer targets round the effective quantum away from zero to at least `1`.
- Nonfinite, zero, or unrepresentable effective quanta fail closed.
- Changing quantum never applies a step.

If a schema control lacks a positive step, the provider derives one deterministically as `max((ui_max-ui_min)/1000, type_epsilon_at_current_value)` and reports `step_source=derived`. This rule is frozen for v1 so target ordering and scrub behavior do not depend on UI frame timing.

### Focus And Keyboard Ownership

- `Current Value` is a dedicated focusable scrub surface, not the ordinary numeric text editor.
- Left/right key presses step by one effective quantum only while that item owns keyboard focus.
- ImGui active text/numeric input, menus, popups, or another active item suppress scrub dispatch.
- Key repeat is permitted only through the scrub surface's own repeat policy.
- `Shift` multiplies the quantum by `10`; `Alt` multiplies it by `0.1`; both together cancel to `1`.
- Ordinary numeric controls retain normal caret/navigation behavior and must never dispatch Parameter Motion steps.
- Loss of focus ends the current gesture and requests one settled render if a mutation occurred.

### Pointer Scrub

- Horizontal drag begins only on the scrub surface.
- Each complete logical drag interval emits an integer step count through the same prepared mutation operation.
- Residual pixels are gesture-local and never serialized.
- Vertical movement does not mutate the value.
- Pointer capture loss ends the gesture safely.
- Runtime proof uses in-process UI/action events; physical mouse automation remains forbidden.

### Boundary Policy

V1 supports:

- `clamp`: clamp to hard bounds; continuous motion stops when a further tick cannot change the value;
- `wrap`: wrap over a finite hard range;
- `reflect`: reflect at finite hard bounds while preserving overshoot direction.

Unbounded mutation is not a user-selectable boundary policy. A descriptor without finite hard bounds may be scrubbed only when it declares an explicit safe local limit; otherwise it fails closed as `finite_motion_bounds_missing`.

The target descriptor declares allowed policies. Loading an unsupported policy falls back to `clamp`, stops motion, and reports a migration warning.

### Continuous Motion

- `Motion Rate` is signed authoritative value units per second, not steps per second; this permits direct migration of existing rates.
- `Motion Enabled` is the only activation authority.
- The engine uses a double-precision accumulator and commits a typed target value at most once per frame.
- A zero/nonfinite rate, invalid target, inactive lane, failed binding, or exhausted clamp boundary prevents mutation and disables motion with a reportable reason.
- Changing target/rate/quantum/boundary while enabled first stops the prior gesture. Configuration changes do not implicitly re-enable motion.
- The old seed and generic animation runtime functions are removed or reduced to compatibility translation outside the frame loop; they must not remain parallel mutation paths.

## Interaction And Render Pacing Contract

Every scrub or motion mutation uses one lifecycle:

1. `begin` on first committed change;
2. `update` on subsequent committed changes;
3. `end` on key release, button completion, drag release/capture loss, motion disable, invalidation, or state load.

The lifecycle marks the viewer as interacting through the existing pacing seam. It must not create a second debounce clock. Ending a mutated gesture requests exactly one settled full-quality render. Selecting/configuring without mutation does not enter interaction or dirty the frame.

## Legacy Seed Actions

Scoped `Prev Seed` / `Next Seed` controls remain where they have clear domain meaning, but they route through the same descriptor and prepared-step authority.

- They use the canonical seed action delta defined by the seed descriptor, not the panel's current target.
- They do not enable continuous motion.
- They preserve scoped root-pattern ownership.
- No direct `ExplainOSeedSetCombined` action branch remains outside the common mutation provider after cutover.

## State, Capture, And Replay Contract

### New State Shape

`state.json` gains an optional `parameter_motion` object:

```json
{
  "target_binding_path": "fractal.params.explaino_seed",
  "quantum_exponent": 0,
  "rate_per_second": 0.25,
  "boundary_policy": "clamp",
  "enabled": false
}
```

Rules:

- The target's actual numeric value remains in its existing authoritative state field.
- Saved `enabled` is always `false` for replay authority.
- `fractal-state.json` and reports may record `was_running_at_capture` as derived review context, never as resume authority.
- Transient focus, key repeat, drag residual, timing accumulator, interaction generation, and pending settle state are not serialized.

### Load Semantics

Every state load performs this order:

1. end/stop current motion and any scrub gesture;
2. load authoritative fractal/view parameters;
3. load or migrate Parameter Motion configuration;
4. validate target and policy against the loaded lane;
5. force `enabled=false`;
6. issue one settled render for the loaded state.

Loading a Capture Finding therefore replaces the current scrub configuration, as the operator requested for v1, but never resumes the capture's motion. A future option to preserve the viewer's current motion setup across state load is explicitly deferred.

### Legacy Migration

- A legacy generic animation target maps to its canonical binding path and rate when resolvable.
- A legacy seed auto-increment configuration maps to the scoped/current seed binding and rate only when no resolvable generic target is present.
- If both legacy systems claim authority, generic target wins, motion loads disabled, and a structured `multiple_legacy_motion_authorities` warning is reported.
- Unknown or inactive legacy targets are retained as unavailable configuration text where practical, never silently redirected.
- Old state files remain loadable.

## Runtime Reports

The no-mouse report exposes:

- configuration target path and label;
- target availability and exclusion reason;
- numeric type and current authoritative value;
- base step, step source, exponent, and effective quantum;
- requested/effective boundary policy;
- requested rate;
- enabled state;
- stopped reason;
- gesture state;
- last mutation source;
- target state generation and last settled render generation;
- legacy migration warning when present.

Reports distinguish configuration from execution. They must not claim motion is active merely because a target is selected.

## Implementation Slices

### Slice 0 - Baseline And RED Matrix

- Freeze current behavior and public control IDs in reports.
- Add regressions proving the three reported failures: global arrows mutate seed during numeric editing, target selection couples to activation, and target eligibility is a hard-coded list.
- Inventory every current numeric schema/binding descriptor and classify inclusion/exclusion.
- Record current state/capture migration examples.

Gate: failures are reproducible and the provider inventory has no unexplained numeric rows.

### Slice 1 - Numeric Capability Provider

- Add typed descriptors and deterministic enumeration.
- Add read/prepare/commit APIs with finite/type/boundary checks.
- Add a generated audit comparing schema visibility, binding resolution, and eligibility.
- No UI cutover or continuous motion yet.

Gate: all eligible targets can be read and prepared; every excluded numeric target has a stable reason.

### Slice 2 - Pure Scrub Policy And Widget

- Add deterministic quantum, modifier, boundary, key-repeat, and drag-step policy helpers.
- Add the focus-owned scrub surface and explicit step commands behind an internal feature switch.
- Prove ordinary text/numeric editing suppresses scrub.
- Wire interaction begin/update/end without removing old animation yet.

Gate: direct UI action tests prove no mutation without scrub ownership and exactly one mutation per prepared step.

### Slice 3 - Unified Continuous Motion Engine

- Add the one runtime motion engine and double accumulator.
- Route motion through the typed provider and existing pacing lifecycle.
- Prove target/rate/config changes are inert until explicitly enabled.
- Prove lane changes and clamp exhaustion stop safely.

Gate: representative float32, float64, and integer targets animate deterministically with one mutation authority.

### Slice 4 - UI And Action Cutover

- Replace old Auto-Increment Seed / Animate Parameter controls with Parameter Motion.
- Remove the hard-coded target option list.
- Migrate scoped seed actions to common step authority.
- Remove global arrow polling and parallel per-frame animation calls.
- Keep one bounded kill switch only if required for rollout, with explicit report authority and removal milestone.

Gate: source scan and runtime reports show no normal-path dual motion authority.

### Slice 5 - State/Capture Migration

- Add the optional state object and legacy loader translation.
- Update Capture Finding review sidecar.
- Prove all loads stop motion and settle once.
- Prove old states load cleanly and new states round-trip configuration without resuming.

Gate: capture/replay pixels remain deterministic and configuration truth is honest.

### Slice 6 - Published Runtime Sweep And Hardening

- Publish once.
- Run data-driven no-mouse target sweeps across representative families and numeric types.
- Exercise the actual focus/action command surface, not direct helper calls.
- Hostile-review schema/provider drift, dead controls, action bypasses, pacing bypasses, and state resume hazards.
- Remove temporary rollout switch if all gates are green; otherwise report explicit fallback authority and stop.

Gate: the old three defects are impossible through the public runtime paths, and all visible controls are consumed or intentionally unavailable.

## Test Plan

### Native

- Provider tests for deterministic inventory, supported numeric types, current-lane activation, stable exclusion reasons, and no Color Pipeline entries.
- Prepare/commit tests for finite values, hard bounds, clamp/wrap/reflect, integer rounding, float precision, and stale-context rejection.
- Pure scrub-policy tests for quantum derivation, modifiers, key repeat, drag residual, and boundary transitions.
- UI tests proving focus ownership, text-edit suppression, configuration non-mutation, and stable control IDs.
- Motion tests proving one engine, signed rates, accumulator behavior, lane invalidation, and interaction lifecycle.
- State tests for new round-trip, legacy migrations, conflicting legacy authority, malformed config, and forced stopped load.
- Source/registry audit proving no hand-maintained target dropdown and no direct global seed-arrow mutation path remain.

### Runtime / No Mouse

- Launch the published viewer and use persistent in-process commands.
- Focus the scrub surface, issue left/right actions, and prove exactly the selected target changes.
- Focus an ordinary numeric text editor, issue left/right keys, and prove no motion target changes.
- Change target, quantum, rate, and policy; prove frame and target value remain unchanged.
- Enable motion; prove values and frames advance and adaptive preview engages when timing requires it.
- Disable/end; prove one settled full-quality render.
- Change fractal lane while active; prove motion stops before mutation of an inactive binding.
- Exercise at least seed double authority, a float formula parameter, an integer iteration parameter if classified eligible, and one root-field scoped control.
- Capture Finding, reload, and prove configuration reloads stopped with matching replay pixels.
- Prove Color Pipeline controls are absent from the target catalog.

### Performance

- The provider inventory is rebuilt only when schema/lane authority changes, not every frame.
- Motion commits at most one value per frame.
- Compare idle and enabled-motion frame timing against the pre-slice baseline.
- No performance claim without median/tail evidence; timing noise does not relax correctness.

## Risks And Stop Rules

1. Binding drift: stop if schema and runtime binding cannot share stable parameter identity.
2. Dual authority: stop if old and new frame-loop mutators coexist on the normal path.
3. Hidden mutation: stop if any configuration operation changes a target or dirty generation.
4. Focus ambiguity: stop if the harness cannot prove ordinary numeric editing owns arrows exclusively.
5. Type narrowing: stop if float64/double targets are forced through float-only bindings.
6. Boundary ambiguity: fail closed rather than infer unsafe unbounded motion.
7. Pacing bypass: stop if motion changes frames without the normal interaction lifecycle.
8. State surprise: stop if load can resume motion or preserve stale gesture state.
9. Scope creep: defer Color Pipeline and external IPC rather than weakening v1.

## Deferred Follow-Ups

- Programmable Color Pipeline animation through a future Salticid/graph surface.
- Multiple tracks, timelines, keyframes, modulation graphs, and recipe-local animation.
- Per-digit strip UI, exact rational/base editing, nonlinear scrub profiles, and source patching.
- External runtime-state named-pipe API; see `parameter_motion_external_runtime_interface_DEFERRED.md`.
- Preserve-current-motion-configuration option during state load.
- Operational-control motion classes and explicit unbounded policies.

## Proof Ledger

- [x] Existing seed scrub, seed dynamics, generic animation, schema, state, and report seams inspected.
- [x] Digit-scrub POC concepts reviewed and bounded for viewer use.
- [x] State/capture semantics locked at planning level.
- [x] Color Pipeline and external runtime API boundaries recorded.
- [x] Fresh blind hostile subagent review recorded.
- [ ] Findings discussed with the operator and plan revised or accepted.
- [x] Contract validation passed.
- [x] Phased-plan sync passed.
- [x] Hostile-audit validation passed.
- [x] Code-quality baseline passed.
- [x] Diff check passed.

## Hostile Audit

- Status: complete

The blind planning audit is complete. Its verdict rejected implementation readiness, and its findings await operator reconciliation; product implementation remains paused.

The reviewer must assume this plan recreates the old authority bug until disproven. It must inspect:

- whether eligibility is actually data-driven or merely another list;
- whether focus ownership can be proved through public UI events;
- whether a selected target can still imply activation;
- whether float64, integer, scoped root, and legacy seed paths remain truthful;
- whether state load can accidentally resume execution;
- whether pacing receives exact lifecycle events;
- whether old action controls bypass the provider;
- whether Color Pipeline or operational controls leak into v1;
- whether compatibility creates permanent dual authority;
- whether the planned test matrix tests public paths rather than helpers.

## Audit Passes

- [x] Blind pass 1 by a fresh subagent found one contract-state observation, eight implementation-blocking authority/specification gaps, and two bounded documentation ambiguities.
- [x] Evidence-fidelity pass confirmed the planning-only contract is an intentional gate and the reviewer's dirty-checkout rearward result is not evidence that clean base HEAD was unproven.
- [x] Clean re-read after review-record corrections confirmed the substantive P1/P2 findings are recorded, no silent design amendment was made, and product mutation remains paused.

## Audit Findings

- [x] Finding set recorded in `docs/notes/parameter_motion_scrub_unification_BLIND_HOSTILE_REVIEW.md` without silent plan revision.
- [x] Review precision finding: the P0 planning-contract observation is an expected implementation gate, not a reason to turn this documentation contract into a product contract.
- [x] Review precision finding: the cited `blocked_unproven` artifact was generated against the dirty in-progress planning checkout; clean base HEAD 6958f02 had passed rearward review at session start.
- [ ] Reconcile capability metadata ownership and authoritative setter/value-domain classes.
- [ ] Reconcile competing automatic mutators, dynamic availability, and actual ImGui focus/key automation.
- [ ] Reconcile exact boundary/accumulator/pacing/state-version semantics.
- [ ] Clarify the Color-domain exclusion and check in digit-scrub POC provenance/decision mapping.

## Planning Slice Closeout

This planning slice ends after the blind review is recorded, validation passes, the artifacts are checkpointed and pushed, and the operator has a concrete findings summary for discussion.

Product implementation is not authorized by this draft. Preplanned implementation slices exist above, but their execution is paused until the blind findings are reconciled and the implementation contract is explicitly accepted or revised.





