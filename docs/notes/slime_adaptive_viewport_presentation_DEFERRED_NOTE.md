# Deferred Note: Adaptive Viewport Presentation During Fast Slime Runs

Status: nice-to-have deferred design. Do not implement under current SDF, sidecar GA, or render-pacing slices without a fresh plan/contract.

## Summary

Fastest-speed slime exploration can invert the cost profile: the slime/sidecar decision loop becomes cheap relative to rendering, so the run spends most of its wall time drawing every intermediate mutation instead of exploring. In that regime, rendering every slime adjustment is no longer useful feedback; it is a tax on the exploration loop.

The proposed direction is adaptive viewport presentation throttling for autonomous/slime runs:

```text
slime/evaluation ticks continue at the requested speed
mutation history records every step
UI/control state remains authoritative
viewport rendering samples the latest state at a measured cadence
final stop/pause/completion always forces a current render
```

This is presentation throttling, not simulation throttling. It should never drop mutation records, skip policy decisions, or fake replay history.

## Problem Statement

Current fastest runs can enter a bad mode:

- slime policy chooses and applies mutations quickly;
- each mutation invalidates the viewport;
- full render cost approaches or exceeds the per-tick slime budget;
- rendering every intermediate state serializes the loop;
- the operator sees motion, but the exploration rate collapses.

This is especially likely in high-resolution, `float64`, SDF-heavy, or expensive Color Pipeline states. In those cases, a rendered frame can become stale before it finishes, and the user-facing frame is a sampled diagnostic view of a rapidly advancing autonomous process rather than the process itself.

## Desired Semantics

Separate four things that are currently easy to blur:

1. **Slime tick**: one autonomous policy/evaluation step.
2. **Mutation commit**: an authoritative state change plus trail record.
3. **Viewport presentation**: a rendered image of some committed state.
4. **Final settle render**: guaranteed current render at stop/pause/completion/user takeover.

The first two are correctness surfaces. The third is feedback. The fourth is the closure guarantee.

## Candidate Policy

Use measured render timing and the intended slime tick budget to choose a presentation cadence.

Simple formula:

```text
cadence_steps = ceil(render_ema_ms / slime_tick_budget_ms)
cadence_steps = clamp(cadence_steps, 1, max_cadence_steps)
```

Interpretation:

- If render is cheap relative to the slime tick, render every step.
- If render costs about five slime ticks, render every fifth committed mutation.
- If render is extreme, cap visual updates to a low but bounded feedback rate.

Suggested guardrails:

```text
max_cadence_steps: 20 or 30
max_visual_interval_ms: 250-500 ms for diagnostic feedback
force_render_on_pause_stop: true
force_render_on_user_input: true
force_render_before_capture: true
```

When the slime tick has no meaningful fixed interval because the user requested "fastest", derive a synthetic budget:

```text
effective_tick_budget_ms = max(observed_slime_tick_ema_ms, configured_min_tick_budget_ms)
```

This prevents division by near-zero and makes fastest mode behave as "run slime as fast as practical, render at a bounded diagnostic cadence."

## State Machine Sketch

```text
manual_mode:
  normal render pacing

autonomous_slime_running:
  apply every slime decision
  append every mutation record
  mark viewport dirty
  if should_present_now:
    render latest committed state
    clear deferred count
  else:
    increment deferred count
    keep viewport stale marker visible

autonomous_slime_paused/stopped/completed:
  force render latest committed state
  clear stale marker after render completes

user_takeover:
  cancel/defer autonomous presentation policy
  force latest-state render or restore normal manual render invalidation
```

## Diagnostics And Reporting

The feature is only trustworthy if it is visible in reports.

Candidate runtime report fields:

- `autonomous_viewport_presentation_enabled`
- `autonomous_viewport_cadence_steps`
- `autonomous_viewport_steps_since_render`
- `autonomous_viewport_deferred_frames`
- `autonomous_viewport_last_presented_step`
- `autonomous_viewport_latest_committed_step`
- `autonomous_viewport_stale`
- `autonomous_render_ema_ms`
- `autonomous_slime_tick_ema_ms`
- `autonomous_forced_final_render_count`

Suggested UI copy:

```text
Viewport deferred: showing step 120 / latest 137, rendering every 5 slime steps
```

This is important because the control/sidebar values may reflect newer state than the currently presented frame. That mismatch is acceptable only if it is explicit.

## Relationship To Existing Pacing

This is related to preview scaling/debounce but not the same thing.

Existing pacing tries to make renders cheaper. This proposal chooses whether to present a render at all for every autonomous mutation.

Both can compose:

- preview scaling can reduce the cost of each presented frame;
- presentation throttling can skip frames when even preview rendering is still too expensive;
- final settle can use requested/full quality.

Do not use this as a substitute for fixing broken render timing or SDF hot paths. It is a policy for autonomous modes where the render is no longer useful per step.

## Capture And Replay Boundary

Capture/replay is a correctness fence. Any future implementation must treat a deferred viewport frame as a UI diagnostic artifact only.

Rules:

- Capture Finding must force a latest committed state render before writing `frame.png`.
- Diagnostic capture must force a latest committed state render before writing image outputs.
- Replay proofs must compare captured/latest-state pixels, not the last deferred presentation frame.
- `state.json` and `fractal-state.json` must record authoritative mutation/render state, not the stale presentation marker alone.
- If a forced final render fails, capture must fail closed rather than saving the stale viewport.

This is non-negotiable because earlier capture/replay bugs were caused by confusing visible UI state with capture authority. This proposal would intentionally allow the viewport to lag, so capture authority must be separated before behavior changes.

## Applicability

Likely applies to:

- Explaino sidecar auto-demo paced loops;
- future slime policy GA runs;
- runtime-walk field slime visualization;
- any future autonomous exploration mode with mutation history.

Should not apply to:

- ordinary manual slider/camera edits;
- capture/finding export;
- replay proof paths unless explicitly testing the presentation policy;
- static benchmarking where every frame must correspond to every state.

## Contradictions And Show-Stoppers

- The viewport can temporarily disagree with the latest committed parameters. This must be reported clearly.
- If capture uses the currently displayed stale frame instead of forcing latest-state render, it would create a serious capture authority bug.
- If skipped renders are confused with skipped slime steps, mutation history becomes dishonest.
- If the policy is based on hardcoded `every 5th` rather than measured timing, it will be wrong across machines and scenes.
- If render requests queue rather than coalesce, throttling can make latency worse. Presentation should render the latest state and discard obsolete intermediate render requests.
- If sidecar/slime code assumes "mutation implies immediate visible render", this will expose hidden coupling and needs explicit repair.
- If final forced render is expensive, stop/pause can feel delayed. That delay is acceptable only if reported as settling/finalizing.
- Fastest mode may not have a stable tick budget; the policy needs an observed/sliding budget rather than division by zero.

## Future Slice Shape

Recommended order:

1. Measurement-only report slice.
   - Add counters for autonomous slime tick time, render time, steps per render, and stale viewport state.
   - No behavior change.
2. Presentation-cadence policy tests.
   - Pure unit tests for cadence selection, hysteresis, clamp behavior, fastest-mode fallback, and final render rules.
3. Headless/runtime proof with fake heavy render timing.
   - Prove mutation count continues while presented frame count drops.
   - Prove final render happens.
4. Live viewer integration for one autonomous mode.
   - Prefer the smallest sidecar/slime loop surface.
   - Keep normal manual interaction unchanged.
5. Broader autonomous-mode adoption.
   - Only after first integration proves reports, capture safety, and UI clarity.

## Proof Gates For Any Future Implementation

- Unit tests prove cadence computation for cheap, equal-cost, expensive, and extreme render cases.
- Runtime proof shows mutation history length equals slime steps, not rendered frames.
- Runtime proof shows presented frame count is lower than mutation count under heavy render timing.
- Runtime proof shows stop/pause/completion forces latest-state render.
- Capture Finding proof shows capture never uses a stale deferred viewport frame.
- Manual slider/camera tests prove normal interaction pacing is unchanged.
- Report fields identify stale/deferred viewport state and latest committed step.
