# Blind Hostile Review: Parameter Motion And Focus-Owned Scrub Unification

Reviewer: fresh read-only subagent `01a03a35-ddfb-7db2-8766-260ee16c0e57` (`Epicurus`)
Review mode: no inherited task history; repo and planning artifacts only
Verdict: REJECT AS IMPLEMENTATION-READY
Disposition: recorded without plan amendment; operator discussion is required before reconciliation

## Verdict

The direction is sound, but the planning-only contract cannot authorize implementation and several runtime ownership decisions are not decision-complete.

## Findings

### P0 - The current contract intentionally cannot authorize implementation

The contract is `workflow_only`, permits only documentation/artifact paths, and requires planning validators. The plan also withholds product authorization. Implementation needs a separate `viewer_first` contract, preferably bounded per implementation slice, with exact source/test scope, native build, runtime publish, published-runtime proof, and machine assertions.

References:
- `docs/contracts/parameter_motion_scrub_unification.contract.json:5`
- `docs/contracts/parameter_motion_scrub_unification.contract.json:7`
- `docs/contracts/parameter_motion_scrub_unification.contract.json:64`
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:3`

### P1 - Capability ownership is unspecified

The plan requires `scrubbable`, `motion_animatable`, semantic owner, allowed policies, and exclusion reasons, but the current schema model has none of those fields. The existing descriptor derives animatability from hard-coded mappings and the old dropdown. Without a named metadata owner, implementation can create another drifting target list.

Required amendment: define fail-closed schema metadata and canonical target IDs/aliases, value domain, setter/applicator route, bounds, allowed policies, and the exact generated audit against bindings and visibility.

References:
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:79`
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:116`
- `ui_app/src/ui_schema.h:28`
- `ui_app/src/fractal_parameter_surface_descriptor.cpp:204`

### P1 - Binding type is not necessarily the authoritative value domain or setter

Camera controls are schema floats while authoritative center/zoom state includes doubles and transformed/log2 values. ExplainO seed is a logical combined value requiring `ExplainoSeedSetCombined`. Existing edit routes perform parameter-specific synchronization and side effects. A generic pointer commit can narrow precision, update only a mirror, or skip semantic work. `rate_per_second` is undefined for transformed values such as zoom.

Required amendment: name setter/applicator classes and value domains. Add special-setter parity for camera high-precision state, combined seed, preset-coupled fields, and root-derived updates. Avoid assuming schema storage type is the semantic motion type.

References:
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:103`
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:206`
- `ui/fractal_binding_surface_v1.ui_schema.json:73`
- `ui_app/src/fractal_types.h:430`
- `ui_app/src/schema_binding.cpp:775`
- `ui_app/src/explaino_seed.cpp:19`

### P1 - Prepare/commit and stale-context rules conflict

The plan requires commit to be non-failing after prepare while tests reject stale context, but no state generation currently defines staleness. A selector or authority change between prepare and commit forces either stale mutation or a failing commit.

Required amendment: define token lifetime, context identity, no-op outcomes, and whether preparation plus commit are an indivisible same-frame operation. If a generation check is needed, scope and own it explicitly.

References:
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:105`
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:369`

### P1 - Competing automatic mutation authorities are omitted

Sweep playback, runtime-walk playback, sidecar auto-demo, auto-dive, and auto-max-iteration can overlap seed, camera, or iteration targets. Motion on `max_iter` can be overwritten by auto-max; zoom motion can compete with auto-dive; runtime walk independently consumes arrows and applies snapshots. Ordering would become frame-dependent.

Required amendment: add an explicit arbitration matrix for Parameter Motion versus auto-dive, auto-max-iter, sweep, runtime walk, sidecar auto-demo, state load, capture, lane change, and root-authority change. Either exclude conflicted targets while another owner is active or define one deterministic ownership/preemption policy.

References:
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:23`
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:68`
- `ui_app/src/main.cpp:1477`
- `ui_app/src/main.cpp:2659`
- `ui_app/src/main.cpp:3745`
- `ui_app/src/viewport_interaction.cpp:32`
- `ui_app/src/main.cpp:4372`

### P1 - Dynamic eligibility conflicts with the cache statement

Visibility and activity can change inside one lane when root authority, generated layout, or custom-coefficient state changes. The plan says inventory is rebuilt only for schema/lane changes. A cached target can remain active after its predicate becomes false.

Required amendment: separate a cacheable static descriptor inventory from dynamic availability. Reevaluate activity on every action/prepare or invalidate using every visibility dependency.

References:
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:118`
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:389`
- `ui/fractal_binding_surface_v1.ui_schema.json:800`
- `ui_app/src/fractal_parameter_surface_descriptor.cpp:130`

### P1 - The public focus/key proof path does not exist

The persistent harness can set values, select enums, click known rectangles, and pan, but it cannot focus a real ImGui item or inject key press/repeat/release. Numeric text editors use secondary IDs absent from the automation rectangle surface. A direct step command can pass while global keyboard routing remains broken.

Required amendment: specify and build a real ImGui focus/key-event automation surface, including active item/control reporting. Public proof must cover scrub focus, numeric text focus, popup/menu ownership, key repeat, release, and focus loss.

References:
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:175`
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:376`
- `ui_app/src/main.cpp:3527`
- `ui_app/src/schema_binding.cpp:1843`
- `ui_app/src/schema_binding.cpp:2075`

### P1 - Bounds and numeric motion behavior are incomplete

Seed has only UI bounds and `max_iter` has only a hard minimum, while the plan requires finite hard ranges or an undefined safe local limit. Integer wrap needs an inclusive period. Reflect needs repeated-crossing and exact-boundary rules. Sub-unit integer rates, large/stalled `dt`, accumulator resets, float ULP no-ops, and clamp no-change outcomes are undefined.

Required amendment: freeze boundary equations, inclusive/exclusive endpoints, multi-crossing behavior, accumulator/reset rules, maximum accepted `dt`, transformed-value units, and no-op/stop outcomes. Reconcile this with one-sided and intentionally unbounded parameters.

References:
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:164`
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:194`
- `ui/fractal_binding_surface_v1.ui_schema.json:819`
- `ui/fractal_binding_surface_v1.ui_schema.json:576`

### P1 - Pacing lifecycle and generation requirements lack current authority

Current pacing exposes `NoteViewerInteraction` and elapsed-time debounce, not explicit begin/update/end or state/frame generation counters. The plan requires exact terminal settle and reports target/settled generations. Adding global generations here may expand into the deferred external-state architecture.

Required amendment: define the bounded pacing-end proof using existing authority, or explicitly scope a minimal generation owner. Do not claim generations that the runtime cannot currently produce.

References:
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:215`
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:278`
- `ui_app/src/viewer_render_pacing.h:31`
- `ui_app/src/viewer_render_pacing.cpp:141`
- `docs/notes/parameter_motion_external_runtime_interface_DEFERRED.md:111`

### P1 - State migration lacks frozen wire shapes and precedence

Current state version 3 serializes seed auto-increment but not generic animation target/rate. The loader accepts versions 1-3 and optional seed fields can default from current in-memory state. The proposed generic legacy migration has no authentic persisted source. Missing, malformed, new-plus-legacy, and current-config replacement precedence are not fully specified.

Required amendment: freeze state version policy and exact semantics for versions 1/2/3/new, absent object, malformed object, new-plus-legacy fields, capture while running, and load reset. An absent object must not accidentally preserve current motion configuration if load is defined to replace it.

References:
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:234`
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:270`
- `ui_app/src/diagnostics_capture.cpp:1357`
- `ui_app/src/diagnostics_capture.cpp:1367`
- `ui_app/src/diagnostics_state_io.cpp:2316`

### P2 - Color Pipeline exclusion is ambiguous

The plan alternates between excluding Color Pipeline row/function animation and excluding any Color Pipeline owner. Normal Color controls such as exposure/saturation exist on the normal schema and can affect the active Color Pipeline.

Required amendment: state whether all Color-domain controls are excluded in v1, or only advanced row/graph controls. Define this by semantic owner metadata, not help text.

References:
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:42`
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:126`
- `ui/fractal_binding_surface_v1.ui_schema.json:1425`

### P2 - Digit-scrub POC evidence is not reproducible from the checkout

The POC is a required input and its concepts are marked reviewed, but the plan does not record its source path, version, or a decision table showing retained/deferred concepts.

Required amendment: add a checked-in source/provenance note and a compact keep/defer/reject table. Do not make the external POC a runtime/build dependency.

References:
- `docs/contracts/parameter_motion_scrub_unification.contract.json:20`
- `docs/notes/parameter_motion_scrub_unification_PHASED_PLAN.md:417`

## Missing Proof Paths

- Canonical target/alias court covering every numeric schema control and every exclusion.
- Special-setter parity for camera high-precision state, combined seed, preset side effects, and root-derived updates.
- Collision tests against auto-dive, auto-max-iter, sweep, runtime walk, and sidecar auto-demo.
- Actual ImGui focus tests for scrub surface, numeric text input, popup/menu, repeat, release, and focus loss.
- Float32 ULP, float64 seed, bounded integer, one-sided bounds, multi-crossing wrap/reflect, stalled-frame `dt`, and clamp no-op tests.
- Fast-render and preview-render lifecycle tests proving one terminal full-quality settle.
- State versions 1/2/3/new, absent/malformed/new-plus-legacy precedence, capture while running, and replay pixel identity.
- Published public-path tests proving selection/configuration are inert and no excluded Color targets leak.

## Strong Points To Preserve

- Explicit activation toggle.
- Focus-owned scrub invariant.
- No retained raw pointers.
- Fail-closed target eligibility.
- One normal motion engine.
- Stopped-on-load behavior.
- Capture/replay pixel authority.
- Public no-mouse proof requirement.
- Deliberate Color Pipeline and external-IPC deferment.

## Unproven From Current Checkout

- The intended eligible-target inventory.
- Any authentic persisted generic-animation legacy state.
- Global state/frame generation semantics.
- Digit-scrub POC provenance.

No tracked files were modified by the reviewer. This review records findings only; no findings have yet been accepted, rejected, or repaired in the governing plan.
