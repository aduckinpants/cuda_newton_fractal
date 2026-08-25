# Parameter Motion And Focus-Owned Scrub Unification Campaign

Status: second blind hostile review completed; its findings are reconciled below. Product mutation remains unauthorized under this planning-only contract; reconciliation is complete and Slice 0 requires a new accepted `viewer_first` contract.

## Current Phase

The second fresh reviewer found no new scope-level showstopper, but rejected the draft on semantic-target identity, mutator cutover, continuous-time math, pacing ownership, state migration, observed-input proof, and descriptor migration. The operator's standing direction to choose the smallest extensible route resolves the product choices: canonical targets may have multiple presentations, continuous motion is rate-only, invalid frame time skips safely, and state loading is transactional and always stopped. The reconciliation reviewer accepted the architecture with mechanical amendments; those amendments and a final clean re-read are complete. Remaining work is checkpoint, receipts, rearward review, and push.

## Explicit User Asks

- [x] Replace fragile seed-only global arrow scrubbing with focus-owned scrubbing for explicitly eligible normal numeric parameters.
- [x] Prevent arrow-key mutation while ordinary numeric/text controls, menus, popups, or other widgets own input.
- [x] Separate target selection, quantum, rate, and policy configuration from explicit stepping or motion enablement.
- [x] Replace the old Auto-Increment Seed and generic Animate Parameter UI with one coherent Parameter Motion panel.
- [x] Keep all Color-domain controls out of v1; Color Pipeline is the future animation authority.
- [x] Preserve useful digit-scrub POC behavior through a repository-local reference snapshot without importing browser/runtime code.
- [x] Make state load replace motion configuration but always load stopped.
- [x] Preserve a future versioned external runtime-state interface without implementing IPC here.
- [x] Prefer the smallest route that remains extensible and does not create parallel authority.
- [x] Complete a second fresh blind hostile review and reconcile its findings without broadening v1.

## Phase Checklist

- [x] Phase 0 - Inspect current scrub, animation, binding, state, pacing, automation, and competing-mutator seams.
- [x] Phase 1 - Record the initial campaign and first blind hostile review.
- [x] Phase 2 - Narrow v1 and resolve first-review ownership, bounds, pacing, state, and test ambiguities.
- [x] Phase 3 - Check in bounded digit-scrub POC provenance and decisions.
- [x] Phase 4 - Run a different fresh blind hostile review.
- [x] Phase 5 - Apply the recorded repairs and complete a clean re-read.
- [ ] Pause before product mutation and create Slice 0's `viewer_first` contract.

## Scope Lock

V1 supports only:

- explicitly opted-in direct-linear `float32` parameters;
- explicitly opted-in direct-linear `float64` parameters;
- the combined ExplainO seed through its existing semantic setter adapter;
- one focused scrub target and one continuous motion track;
- normal fractal/view parameter owners whose values directly affect the current lane.

V1 excludes:

- every Color-domain control, including legacy simple Color controls on the main panel and all Color Pipeline rows/graph parameters;
- camera center, zoom, rotation, and transformed/high-precision view values;
- integer, enum, boolean, categorical, and discrete-index controls;
- render resolution, iteration limits, auto-max iteration, preview/pacing, device, precision, backend, capture, diagnostics, and other operational controls;
- derived, preset-coupled, or multi-field controls without a dedicated semantic setter adapter;
- arbitrary root-count/layout selection, although direct-linear scoped root float controls may opt in after setter proof;
- multiple tracks, timelines, easing, keyframes, recording, and programmable animation;
- external IPC, Salticid execution, and physical mouse automation.

This boundary is intentional. A later campaign can add another numeric/storage class only by extending the target court, semantic setter inventory, and public proof matrix.

## Repo-Grounded Starting Point

- `ApplyArrowKeySeedScrub` globally polls left/right key state and does not require scrub focus.
- `ApplyExplainoSeedDynamics` and `ApplyParamAnimDynamics` are separate frame-loop mutation authorities.
- A non-`none` generic animation target also acts as activation state.
- The generic path resolves float bindings and special-cases combined ExplainO seed.
- The target dropdown is a hand-maintained schema option list.
- Sweep, runtime walk, sidecar auto-demo, auto-dive, auto-max iteration, state load, manual edits, and seed actions are additional mutation owners that require arbitration.
- Existing pacing exposes interaction notification plus debounce rather than begin/end generations.
- State version 3 persists legacy seed auto-increment fields but does not persist generic animation target/rate.
- The published no-mouse harness cannot currently focus an ImGui item or inject key press/repeat/release through the real input path.

## Product Invariants

1. No parameter mutates unless an explicit step/drag event or enabled continuous-motion tick owns the action.
2. Selecting a target or changing configuration never mutates parameter value, dirty state, render state, or interaction state.
3. Left/right arrows mutate only while the dedicated scrub surface owns keyboard focus.
4. All authoritative mutations use a declared semantic setter class; schema storage type alone is insufficient.
5. Exactly one normal frame-loop engine owns continuous Parameter Motion.
6. Parameter Motion never runs concurrently with sweep, runtime walk, sidecar auto-demo, or another exclusive playback owner.
7. Static descriptor discovery may be cached; dynamic visibility, binding, lane activity, and ownership are checked on every prepare.
8. Motion stops before lane, authority, target, rate, policy, state-load, capture, or competing-mode transitions.
9. Every state load replaces configuration and leaves motion disabled.
10. Capture/replay parameter pixels remain authoritative; motion execution state is never replay authority.
11. Existing viewer interaction/debounce authority remains the only render-pacing clock.
12. Every Color-domain control is excluded from v1.

## Repository-Local POC Reference

Reference snapshot: `docs/reference/digit_scrub_poc_snapshot/PROVENANCE_AND_DECISIONS.md`.

The planning branch carries this bounded snapshot for its final checkpoint. It records source path, dirty working-tree provenance, source hashes, copied model/tests, focus/drag excerpt, postmortem, and keep/adapt/defer/reject decisions. It is reference-only and has no viewer build/runtime dependency.

## Configuration Owner

Add host-owned `ParameterMotionState`, separate from `ViewState`, `KernelParams`, renderer inputs, and Color Pipeline state.

Authoritative configuration fields:

- `target_id`;
- `quantum_exponent`;
- `rate_per_second`;
- `enabled`.

`boundary_policy` is canonical target metadata, not host configuration and not serialized state.

Transient fields:

- double accumulator/current intended value;
- focused/drag gesture state;
- drag residual;
- last tick timestamp;
- stopped reason;
- last mutation source;
- whether capture observed motion before stopping it.

Only authoritative configuration is serialized. Transient fields reset on construction, load, target/configuration change, and ownership transition.

## Schema-Owned Motion Metadata

Eligibility is explicit opt-in metadata in one canonical `motion_targets` registry owned by the UI schema contract. Absence means ineligible. Ordinary control descriptors may reference a canonical target through `motion_target_ref`; they do not duplicate the target definition.

Canonical target example:

```json
{
  "target_id": "fractal.params.explaino_seed",
  "label": "ExplainO Seed",
  "semantic_owner": "explaino_root_authority",
  "value_domain": "combined_explaino_seed",
  "setter_id": "explaino_combined_seed_v1",
  "scrubbable": true,
  "continuous": true,
  "base_step": 0.001,
  "bounds_policy": "finite_linear",
  "max_abs_rate": 4.0,
  "max_abs_step": 0.4
}
```

Presentation descriptor example:

```json
{
  "path": "fractal.params.explaino_seed",
  "motion_target_ref": "fractal.params.explaino_seed"
}
```

Contract rules:

- `target_id` is version-stable and unique in the canonical registry. Legacy target aliases live in one alias table and never appear as new save output.
- Multiple presentation descriptors may reference one canonical target. Presentation identity, section, visibility, and normal edit widgets remain distinct; they never create duplicate motion targets.
- V1 `semantic_owner` is the closed enum `fractal_formula | explaino_root_authority`. `color`, `camera`, `render_operational`, `diagnostics`, and unknown owners are forbidden.
- V1 `value_domain` is `direct_linear | combined_explaino_seed`.
- V1 `setter_id` is `direct_binding_f32_v1 | direct_binding_f64_v1 | explaino_combined_seed_v1`.
- The existing descriptor `step`, UI minimum, and UI maximum remain normal widget presentation metadata. Motion `base_step`, `bounds_policy`, `max_abs_rate`, and `max_abs_step` belong only to the canonical target.
- `hard_clamp` obtains finite hard bounds from the authoritative binding-validation contract, never from UI-only slider bounds. Registry load fails if binding hard bounds are absent or disagree across presentations.
- `base_step`, `max_abs_rate`, and `max_abs_step` are finite and positive.
- `bounds_policy` is `hard_clamp` or `finite_linear`.
- `finite_linear` is permitted only for a reviewed setter with explicit finite `max_abs_rate` and `max_abs_step`.
- `max_abs_rate * 0.1 <= max_abs_step` is required so every legal clamped continuous tick is also a legal setter step.
- UI visibility does not imply eligibility, and `scrubbable` does not imply binding or commit authority.

The schema parser rejects unknown fields/enums, duplicate canonical target IDs, dangling presentation refs, conflicting aliases, invalid bounds/policy combinations, unsupported setters, and Color-domain opt-ins.

## Capability Provider

The provider joins static schema metadata with live binding/context authority.

Static inventory, cacheable until schema reload:

- target id, label, section/order;
- semantic owner and value domain;
- setter id and numeric storage type;
- base step and safety limits;
- declared bounds policy;
- scrub/continuous capabilities.

Dynamic availability, evaluated on every list render and every mutation prepare:

- at least one presentation descriptor for the canonical target passes its current selector/lane visibility predicate;
- current root/custom/generated authority predicates;
- binding resolution and writability;
- competing owner state;
- setter-specific preconditions;
- current finite value;
- structured unavailable reason.

No raw pointer, ImGui item id, or resolved binding is retained across frames.
## Mutation Service And Setter Classes

Public operations:

- `DescribeParameterMotionTargets(context)`;
- `ReadParameterMotionValue(target_id, context)`;
- `TryApplyParameterMotionDelta(target_id, delta, context, source)`;
- `TryApplyParameterMotionAbsolute(target_id, value, context, source)`.

Prepare and commit are an indivisible same-thread operation inside each public mutation call. No prepared token escapes the call or frame. Preparation performs canonical-target resolution, presentation-aware dynamic availability, ownership, finite/rate/step, type, and bounds validation. After preparation, commit calls the declared setter and returns a non-failing mutation receipt. The normal frame loop folds that receipt into its existing `interactionChanged` aggregator; the service never calls pacing notification directly. A context transition cannot occur between the two stages.

Setter contracts:

### `direct_binding_f32_v1`

- Reads and writes the authoritative direct-linear float binding through one binding mutation API.
- Typed result is computed in double, range-checked, converted once, and compared to current float bits/value.
- If conversion yields no representable change, scrub reports `quantum_below_storage_resolution`; continuous motion retains its double intended-value accumulator until representable or stopped.
- No parameter-specific side effects are permitted unless the binding mutation API already owns them and the target court proves parity.

### `direct_binding_f64_v1`

- Same contract with double storage and no float narrowing.
- Transformed camera/log values remain excluded even if represented by doubles.

### `explaino_combined_seed_v1`

- Reads `ExplainoSeedCombined` and commits through `ExplainoSeedSetCombined`.
- Preserves scoped root-field seed authority where that helper already defines it.
- Does not imply that every seed-like or root-derived value is eligible.

A setter mismatch fails closed as `unsupported_motion_setter`. After cutover, this service is the only authority for motion-originated writes. Ordinary widgets, Reset All, selectors/defaults, automation edits, state load, presets, and manual actions retain their existing authorities but must stop motion before writing an eligible target.

## Target Court V1

Slice 0 generates a checked-in JSON/Markdown court over every numeric schema descriptor:

- canonical target id, aliases, and every presentation path;
- storage type;
- semantic owner;
- presentation visibility owners and merge result;
- motion metadata present/absent;
- setter id;
- scrub/continuous classification;
- bounds policy and limits;
- included/excluded verdict;
- stable exclusion reason.

Initial eligible candidates are limited to existing direct-linear formula/root float controls plus combined ExplainO seed. Camera, Color, integer, operational, derived, and preset-coupled rows must appear as explicit exclusions. No unexplained numeric descriptor may pass the court.

## Parameter Motion Panel V1

```text
Parameter Motion
  Target               [eligible parameter dropdown]
  Current Value        [focusable scrub surface]
  Quantum              [base step x 10^exponent]
  Step                  [-] [+]
  Motion Enabled       [toggle]
  Motion Rate          [signed value units per second]
```

Stable control ids:

- `parameter_motion.target.primary`
- `parameter_motion.value.scrub.primary`
- `parameter_motion.quantum_exponent.primary`
- `parameter_motion.step.decrement`
- `parameter_motion.step.increment`
- `parameter_motion.enabled.primary`
- `parameter_motion.rate.primary`

Boundary policy is descriptor-owned and reported but not user-selectable in v1. This removes unsupported wrap/reflect choices and prevents changing policy from becoming another state surface.

### Target Selection

- Changes configuration only and forces `enabled=false` before changing target.
- Reads/displays current authority after selection.
- Does not dirty or interact with the frame.
- An unavailable loaded target remains identifiable in configuration/reporting but cannot mutate.
- Ordering is deterministic: schema section order, schema row order, target id.

### Quantum

- `quantum_exponent` is integer `[-9, 9]`.
- Effective quantum is `base_step * 10^quantum_exponent`.
- Shift multiplies by `10`; Alt multiplies by `0.1`; both together cancel.
- Nonfinite/zero/excessive deltas fail before mutation.
- Changing exponent never applies a step.

### Focus And Key Ownership

- Current Value is a dedicated focusable scrub surface, not the ordinary numeric text editor.
- Left/right steps are dispatched only when that exact stable control id owns focus and no text input, popup, menu, or other active item owns capture.
- Key repeat uses ImGui's real queued input/repeat behavior.
- Key release or focus loss ends the scrub gesture.
- Ordinary numeric editor caret/navigation remains untouched.

### Pointer Gesture

- Horizontal drag begins only on the scrub surface.
- Complete logical intervals convert to integer step counts through the same mutation service.
- Residual pixels are gesture-local.
- Vertical movement, hover, and pointer movement without ownership are inert.
- Capture loss ends the gesture safely.
- Product tests use in-process ImGui events, never OS mouse automation.

## Boundary And Timing Mathematics

### Scrub and explicit step

Scrub is quantum-based. For one or more signed integer steps:

```text
delta = signed_step_count * base_step * 10^quantum_exponent
candidate = current_authoritative_value + delta
```

`hard_clamp` clamps `candidate` to finite inclusive binding hard bounds `[lo, hi]`. `finite_linear` requires finite `candidate` and `abs(delta) <= max_abs_step`. A typed candidate equal to current authority is a no-op. Scrub reports `clamp_no_change` or `quantum_below_storage_resolution` as appropriate. No wrap or reflection exists in v1.

### Continuous motion

Continuous motion is rate-based only; quantum does not quantize ticks. Quantum remains a scrub/step configuration. Changing it still stops motion before configuration mutation so all panel changes follow one inert rule.

On enable:

```text
intended_0 = current_authoritative_value
```

For each valid rendered-frame tick:

```text
dt_used = min(raw_dt, 0.1)
tick_delta = rate_per_second * dt_used
intended_next = intended_previous + tick_delta
```

Rules:

- `raw_dt` must be finite and positive. Invalid or nonpositive time skips the tick with `invalid_dt_skipped`, leaves the accumulator unchanged, and resets the timing anchor so no lost time is caught up; it does not stop motion.
- Registry validation requires `max_abs_rate * 0.1 <= max_abs_step`; therefore every legal clamped tick delta is legal.
- `hard_clamp` clamps `intended_next` to `[lo, hi]`, applies it through the absolute setter, then disables with `clamp_boundary_reached` once the intended value reaches a boundary in the active direction.
- `finite_linear` requires finite `intended_next`, finite typed conversion, and `abs(tick_delta) <= max_abs_step`; violation disables with a structured reason.
- Float32 representational no-ops retain the double intended accumulator. The first later representable absolute value commits once; float64 uses the same equation without narrowing.
- At most one authoritative value commits per rendered frame.
- Zero or nonfinite rate cannot enable motion.
- Accumulator resets to current authority on enable and is discarded on any stop. Target/rate/quantum change, manual edit, state load, lane/authority change, and ownership transition stop first, so no old accumulator crosses a context boundary.

## Ownership And Arbitration

Parameter Motion is exclusive, not compositional, in v1.

| Event / owner | Required behavior |
|---|---|
| Enable while sweep, runtime walk, or sidecar auto-demo is active | Deny enable with `exclusive_playback_active`. |
| Start sweep, runtime walk, or sidecar auto-demo while motion is active | Stop/end Parameter Motion first, then start the requested owner. |
| Auto-dive or auto-max iteration | Their camera/integer targets are excluded from the court; no shared target exists. |
| Manual widget or automation edit of an eligible target | Stop/end motion first, then apply the edit through its normal authority. |
| Reset All | Stop/end motion first, reset parameters and motion configuration, remain disabled. |
| Fractal selector/default or preset application | Stop/end motion before applying defaults/preset; reevaluate target availability and never redirect silently. |
| Manual sidecar decision/application | Treat as an exclusive writer: stop/end motion before it mutates state. |
| Prev/Next Seed while motion is active | Stop/end motion, then apply one seed action through `explaino_combined_seed_v1`. |
| Target/rate/quantum change | Stop/end motion before configuration update; remain disabled. |
| Lane/root authority change | Stop/end motion; reevaluate target availability; never redirect target silently. |
| State load | Stop/end immediately before parsing; load leaves disabled even if parsing fails. |
| Capture Finding | Stop/end, wait for normal settled full-quality frame, capture current value/configuration as stopped. |
| Application shutdown | Stop without further mutation or settle request. |

No owner may silently pause and later reactivate Parameter Motion.

## Render Pacing Contract

This campaign adds no global state/frame generation authority and no second debounce clock.

- Every committed scrub/continuous mutation returns one changed receipt to the normal frame-loop `interactionChanged` aggregator; that aggregator remains the sole caller of existing `NoteViewerInteraction`.
- Configuration-only/no-op/rejected operations return unchanged receipts and do not notify pacing.
- While commits continue, existing preview/adaptive pacing may engage.
- When preview engaged and commits stop, existing debounce policy produces exactly one terminal full-quality transition in the deterministic fixture.
- When the render stayed full quality and preview never engaged, no synthetic settle transition is required; proof instead shows full-quality mode remained continuous.
- Fast/no-preview and slow/preview courts observe existing quality-mode/report state after the last mutation. They add no generation id or second debounce clock.

## Public Focus/Input Automation Surface

Before claiming the original arrow-key defect fixed, extend the persistent no-mouse harness with real input-path commands:

- `focus_control(control_id)`;
- `clear_focus()`;
- `key_down(key, modifiers)`;
- `key_up(key, modifiers)`;
- bounded `key_press` convenience implemented as down/up events;
- bounded pointer press/move/release against a reported control rectangle for the scrub surface only.

Commands feed the viewer's ImGui input queue and normal frame loop. They must not call the motion service directly.

Automation requests and observed widget receipts are separate. A command is acknowledged only after the normal ImGui frame observes the requested condition.

Request receipt fields:

- command sequence id, requested control id/event, enqueue frame, and timeout frame.

Observed receipt fields:

- acknowledging frame;
- focused stable control id observed through `IsItemFocused`;
- active stable control id observed through `IsItemActive`;
- text-input ownership observed after entering the numeric text-edit subcontrol;
- popup/menu ownership;
- key down/repeat/up or pointer press/move/release observed by the owning widget;
- scrub gesture state;
- last motion mutation source/result.

Numeric editors must expose stable automation identities for both the outer numeric widget and its text-edit subcontrol. Proof must enter the real text-edit mode, observe text ownership, inject arrows through the normal queue, and show caret/edit behavior without scrub dispatch. Synthetic click-to-button promotion or direct setter calls do not satisfy this rail.

## State, Capture, And Replay V4

Bump emitted `state_version` to `4`; loaders accept versions 1 through 4.

Optional v4 object:

```json
{
  "parameter_motion": {
    "target_id": "fractal.params.explaino_seed",
    "quantum_exponent": 0,
    "rate_per_second": 0.25
  }
}
```

`boundary_policy` is descriptor-owned and is not serialized. Execution state is not replay authority; new output omits `enabled`.

### Transactional load rule

State loading stops motion and ends gestures before parsing. It parses and validates the complete document into staged fractal/view and motion objects. No authoritative fractal value or motion configuration is committed until the complete staged document is valid. Commit replaces both staged objects and is non-failing. On failure, prior fractal/view values and prior motion configuration remain, but execution remains disabled and transients are cleared.

### Version and precedence table

| Input | Motion configuration result | Warning/error |
|---|---|---|
| v4 valid `parameter_motion` | Replace target, quantum, and rate; remain disabled. | None. |
| v4 object contains boolean `enabled` | Same as valid v4; ignore requested execution and remain disabled. | `motion_execution_state_ignored`. |
| v4 missing `parameter_motion` | Reset to target `none`, default quantum `0`, default rate `0.001`; disabled. | None. |
| v4 malformed/unknown field/bad target type/nonfinite or out-of-contract value | Reject entire staged load; preserve prior values/configuration but stopped. | Exact JSON path error. |
| v4 has new object plus legacy seed fields | New object wins. | `legacy_motion_fields_ignored`. |
| v1-v3 has neither legacy seed field | Reset motion configuration to defaults; disabled. | None. |
| v1-v3 has either/both legacy seed fields | Configure combined ExplainO seed target, quantum `0`, and legacy rate if present, otherwise `0.001`; disabled regardless of legacy bool. | `legacy_seed_motion_migrated`; if legacy bool was true also `motion_execution_state_ignored`. |
| v1-v3 legacy bool is non-boolean or legacy rate is nonfinite, negative, or exceeds the seed target maximum | Reject entire staged load; preserve prior values/configuration but stopped. | Exact legacy JSON path error. |

No generic animation target/rate migration exists because those fields were not persisted. Actual parameter values remain in existing authoritative state fields. Focus, drag, accumulator, timing, ownership, and pending settle data are never serialized. `fractal-state.json` may report `was_running_before_capture`; it is review-only.

Capture Finding stops motion without changing the current parameter value. If preview had engaged, it waits for the normal settled full-quality frame; otherwise it captures the already-full-quality frame. Replay uses the captured parameter values and stopped configuration.

## Runtime Reports

Expose configuration and execution separately:

- target id/label;
- static inclusion classification;
- dynamic availability and reason;
- semantic owner, value domain, setter id, storage type;
- current authoritative value;
- base step, exponent, effective quantum;
- bounds policy and safety limits;
- configured rate and enabled state;
- exclusive-owner denial/stopped reason;
- gesture/focus ownership;
- last mutation source/result;
- legacy migration warning;
- capture `was_running_before_capture` only on review surfaces.

A selected target never implies active motion in reports.

## Implementation Slices

Each product slice requires its own checked-in `viewer_first` contract with exact mutation scope, native rails, runtime publish, published no-mouse proof, receipts, and rearward review. This planning contract never authorizes product mutation.

### Slice 0 - Baseline, RED Regressions, And Target Court

- Freeze current seed/global-arrow/animation behavior and state v3 examples.
- Add RED tests for arrow mutation during numeric editing, target-selection activation, and hard-coded target drift.
- Generate the complete numeric descriptor inclusion/exclusion court.
- Freeze conflicting-owner states and public automation limitations.
- Create Slice 1 contract only after the court is reviewed.

Gate: every numeric descriptor has an explained verdict; no product behavior changes.

### Slice 1 - Motion Metadata And Typed Mutation Service

- Add schema parser/validation for opt-in motion metadata.
- Add static/dynamic capability provider.
- Add `direct_binding_f32_v1`, `direct_binding_f64_v1`, and `explaino_combined_seed_v1` services.
- Add finite, bounds, ULP/no-op, safety-limit, and dynamic-invalidation tests.
- No UI or frame-loop cutover.

Gate: included targets mutate only through proven semantic setters; excluded targets fail with stable reasons.

### Slice 2 - Real Focus/Input Harness And Scrub Surface

- Add ImGui focus/key/pointer automation commands and report identity.
- Add pure quantum/gesture policy.
- Add focus-owned scrub surface and explicit step buttons behind an internal rollout switch.
- Prove ordinary numeric edit, popup/menu, focus loss, repeat, and drag capture behavior through the public frame/input path.

Gate: the original arrow-key regression is provably impossible without direct helper bypass.

### Slice 3 - Mutator Census, Legacy Cutover, And Locked Continuous Engine

- Generate a source/behavior census for every eligible-target writer: normal widgets, automation edits, Reset All, selector/default application, presets, Prev/Next Seed, state load, capture, sweep, runtime walk, sidecar decisions/auto-demo, and legacy frame-loop engines.
- Add host-owned `ParameterMotionState` and the continuous engine behind a forced-disabled internal rollout lock.
- Interlock every retained eligible-target writer so it stops motion before mutation.
- Remove old seed/generic continuous frame-loop mutators and global arrow polling from the normal path before the new engine may enable.
- Implement exact `dt`, absolute accumulator, stop, no-op, and exclusive-owner rules.
- Return mutation receipts to existing pacing aggregation; test fast/no-preview and slow/preview fixtures.

Gate: source audit shows no competing continuous owner, every retained writer is interlocked, and only then may the internal engine enable in tests.

### Slice 4 - UI And Public Authority Cutover

- Replace Auto-Increment Seed / Animate Parameter UI with Parameter Motion.
- Replace hard-coded target options with the canonical provider.
- Route Prev/Next Seed through the semantic seed setter.
- Rebase `fractal_parameter_surface_descriptor` animatable/motion reporting on the canonical provider; remove its old dropdown-derived authority.
- Enable the new engine only through the public panel after the Slice 3 cutover gate.
- Retain no fallback unless a separately reported, expiring rollout switch is justified by a concrete regression.

Gate: UI, public parameter descriptor, source audit, and runtime report show one canonical target inventory and one normal motion authority.

### Slice 5 - State V4, Capture, And Replay

- Implement exact v4 and v1-3 migration rules.
- Update state/capture/review reports.
- Prove capture while running stops first and replay matches pixels.
- Prove missing/malformed/new-plus-legacy precedence.

Gate: all loads are stopped and deterministic; no invented generic migration.

### Slice 6 - Published Runtime Matrix And Hardening

- Publish once.
- Sweep representative direct float32, direct float64, combined seed, dynamic-invalidation, and excluded-domain targets.
- Exercise actual focus/key/pointer paths.
- Re-run competing-owner, state, capture/replay, and pacing rails.
- Hostile review for dead controls, hidden lists, setter bypass, Color leakage, and dual frame-loop owners.

Gate: public runtime proves the three original defects closed and no new ownership ambiguity.

## Test Plan

### Native

- Canonical target registry validation, alias/presentation merge court, closed owners/domains/setters, metadata ownership, and Color exclusion.
- Generated target and mutator courts with zero unexplained numeric descriptors or eligible-target writers.
- Setter parity for direct float32/float64 and combined seed.
- Dynamic visibility/root-authority invalidation on every prepare.
- Hard clamp, finite-linear cross-field limits, scrub quantum, rate-only continuous equations, ULP no-op, valid/invalid/stalled `dt`, absolute accumulator/reset, and nonfinite handling.
- Arbitration tests for manual/automation edit, Reset All, selector/default/preset application, seed action, sidecar decisions, exclusive playbacks, state load, capture, lane/authority change, and shutdown.
- State versions 1/2/3/4 across every row of the migration table, including transactional whole-document failure and stopped state.
- Public parameter descriptor parity against the canonical provider.
- Source audit proving no global arrow scrub, hard-coded target list, uninterlocked writer, or parallel frame-loop mutator remains after cutover.

### Runtime / No Mouse

- Focus scrub surface and inject real key down/repeat/up; selected target changes.
- Focus ordinary numeric editor and inject arrows; caret/edit behavior occurs and no motion target changes.
- Open popup/menu and prove scrub suppression.
- Change target, exponent, and rate; prove target/frame/dirty/interacted state unchanged.
- Enable/disable continuous motion in fast/no-preview and slow/preview fixtures; require a terminal transition only when preview actually engaged.
- Activate each exclusive playback and prove deterministic stop/denial.
- Change dynamic root authority and prove target invalidates before mutation.
- Capture while running, reload, and prove stopped configuration plus pixel replay parity.
- Prove every Color-domain and operational target is absent with stable exclusion reason.

### Performance

- Static inventory builds only on schema reload.
- Dynamic availability checks are bounded and occur on list render/prepare.
- At most one motion commit per frame.
- Record idle-panel and active-motion median/tail timing; no improvement claim is required.

## Risks And Stop Rules

1. Stop if schema metadata and binding authority cannot share stable target identity.
2. Stop if an included target lacks an exact semantic setter/value domain.
3. Stop if public input automation cannot prove real ImGui focus ownership.
4. Stop if another automatic owner can write the same target concurrently.
5. Stop if state load or capture can reactivate motion.
6. Stop if Color, camera, integer, operational, or transformed targets leak into v1.
7. Stop rather than add wrap/reflect, generic source patching, or global generations incidentally.

## Deferred Follow-Ups

- Color Pipeline/Salticid programmable animation.
- Camera/transformed and integer motion setter classes.
- Wrap, reflect, and richer boundary policies.
- Multiple tracks, timelines, keyframes, modulation, and recording.
- Per-digit strip, arbitrary bases, exact rational editing, and source patching.
- External runtime-state named-pipe API: `docs/notes/parameter_motion_external_runtime_interface_DEFERRED.md`.
- Preserve-current-motion-configuration option during state load.

## Proof Ledger

- [x] Current motion and competing-owner seams inspected.
- [x] First blind hostile review recorded.
- [x] Narrowed v1 decisions incorporated.
- [x] Repository-local digit-scrub POC reference snapshot and decision map prepared for the planning checkpoint.
- [x] Second fresh blind hostile review recorded.
- [x] Unambiguous findings repaired and clean re-read complete.
- [x] Contract validation passed after final review.
- [x] Plan sync passed after final review.
- [x] Hostile-audit validation passed after final review.
- [x] Code-quality baseline and diff check passed.

## Hostile Audit

- Status: complete

Review questions:

- Is the opt-in metadata owner explicit and non-duplicative?
- Can every included value be mutated through an authoritative semantic setter without narrowing or mirror drift?
- Does the arbitration matrix cover every competing owner?
- Can public no-mouse events prove actual ImGui focus/key ownership?
- Are clamp, finite-linear, accumulator, and stalled-frame rules exact?
- Are pacing claims limited to existing authority?
- Are state v4 and legacy precedence exact and stopped?
- Are all Color/camera/integer/operational domains excluded?
- Can another implementation path recreate dual authority?

## Audit Passes

- [x] First blind review rejected the broad draft and identified setter, arbitration, input-harness, bounds, pacing, state, Color, and provenance gaps.
- [x] Reconciliation pass narrowed v1 and repaired each first-review category in planning.
- [x] Second fresh blind review found semantic-target, cutover, timing, pacing, state, input-proof, descriptor, contract, and provenance gaps.
- [x] Final clean re-read confirmed the repaired state; no additional real defect found.

## Audit Findings

- [x] First-review findings are preserved in `docs/notes/parameter_motion_scrub_unification_BLIND_HOSTILE_REVIEW.md`.
- [x] First-review reconciliation decisions are encoded in this revision and the repository-local POC snapshot.
- [x] Second-review findings are recorded and reconciled through canonical targets, complete writer cutover, rate-only exact math, central pacing receipts, transactional state rules, observed ImGui receipts, descriptor migration, and provenance repair.
- [x] Workflow finding: an intermediate scripted contract edit emitted malformed newline text; the contract was restored immediately, re-locked through `viewer_host_revise_contract.py`, and final JSON/schema/diff validation is green.
- [x] Final clean re-read of the repaired state and reconciliation review found no additional real issue.

## Planning Closeout

This planning campaign closes only after the second fresh review is recorded, unambiguous findings are repaired, validators pass, and the branch is checkpointed/pushed/rearward-`ok`.

Product implementation remains unauthorized. Slice 0 is the next preplanned product slice, but it requires a new accepted `viewer_first` contract after this planning closeout.
