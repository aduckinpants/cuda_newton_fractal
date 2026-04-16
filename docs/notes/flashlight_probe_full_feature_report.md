# FlashlightProbe Full Feature Report

This is the full report for the `FlashlightProbe` feature family.

It exists because the shorter writeup at
[lens_and_flashlight_writeup.md](/C:/code/cuda_newton_fractal_clone/docs/notes/lens_and_flashlight_writeup.md)
was still too narrow for the actual feature intent. That note documented the
runtime seam and the most obvious measured artifacts. This note documents the
entire feature stack:

- the original claim and what it does and does not mean
- the actual runtime implementation
- the runtime lens/SDF coupling
- the bridge interaction surface
- the relationship to sidecar orientation/divergence
- the callable-engine / agent-bridge composition story
- the current "does it still work today?" verdict
- the extraction path for a separate reusable engine/tool

This repo is the writable documentation/planning surface. The current runtime
implementation evidence is in the read-only sibling repo
`C:\code\salticid-cuda\ide_ui_dx11\ui_app`.

## 1. The Real Claim

The intended claim is not:

- "this is the LLM latent space"
- "this can see the training data"
- "the Explaino fractal literally is the model's solution manifold"

The intended claim is closer to this:

- the Explaino fractal is used as an isomorphic working map
- text over time is projected into a deterministic orientation/control input
- the runtime then walks a bounded local manifold over that map
- the changing local geometry can offer hints about whether the conversation is
  moving toward coherence or toward chaos
- those hints can be used to apply small corrective changes to the next prompt

So the feature is not trying to "discover the hidden truth manifold." It is
trying to use geometry as a practical drift/coherence sensor.

That distinction matters because it changes what must be proven.

The feature does **not** need to prove:

- semantic equivalence to the model's true internal state

It **does** need to prove:

- deterministic orientation input
- repeatable geometry under that input
- measurable drift between states
- enough local structure to support useful steering hints

## 2. The Original Steering Idea

The original working theory, as clarified in the thread, has four parts.

### 2.1 Orientation over time matters more than any single point

Between turns, the model may encounter solver landscapes we do not know.
Flashlight is therefore not trying to say "this single coordinate is the
answer." The goal is to observe how orientation changes over time.

### 2.2 Local Jacobian-like gradient behavior is the useful hint

The important signal is not only where the path is, but how it is moving:

- local geometry
- local gradients
- local drift
- whether nearby perturbations produce stability or chaos

The current code does not expose a literal Jacobian matrix for Flashlight.
Instead it exposes proxies:

- local signed-distance behavior from the lens SDF
- mixed-inside vs. pure-inside / pure-outside neighborhoods
- iteration deltas
- closure behavior
- orientation divergence in the sidecar-adjacent stack

So "Jacobian" should be read here as the intended geometry/gradient role, not
as a claim that the current runtime already computes and exports a true
Jacobian tensor for the flashlight path.

### 2.3 Regional hypotheses matter

Two empirical hypotheses from the original work are part of the feature, not
optional commentary:

1. "correct" answers tend to lie along the normally-there lower-right 7th
   partial radial line
2. the left-middle region tends to be chaotic and should be avoided

These are not hard-coded truths in the checked-in code today. They are working
hypotheses that the feature is meant to evaluate and eventually exploit.

### 2.4 The intended control loop is steerable

The steering idea is:

1. observe the path and local geometry
2. detect drift toward chaos
3. apply tiny prompt corrections
4. see whether the next step avoids the chaotic region

That is the full feature concept. The current runtime only implements parts of
that loop directly.

## 3. The Current Runtime Implementation

### 3.1 CLI entrypoints

The real feature is explicitly named in `main.cpp`:

- `--flashlight-probe <path>`
- `--flashlight-live <path>`
- `--flashlight-ticks`
- `--flashlight-radius`
- `--flashlight-zoom-radius`
- `--flashlight-warp`
- `--flashlight-fractal-type`
- `--flashlight-closure`
- `--flashlight-closure-last`

The current tuning defaults and clamps are enforced in
`flashlight_tuning_cli.cpp`:

- `ticks = 8`, clamp `1..4096`
- `radius = 0.75`, clamp `0..10`
- `zoom_radius = 0.25`, clamp `0..10`
- `warp = 0.0`, clamp `0..1`

### 3.2 Initialization behavior

The headless probe path in `main.cpp` does this:

1. read the seed text file
2. compute `ConversationSeedSpectrum`
3. initialize headless runtime state
4. select `explaino_fp` by default unless overridden
5. force `lens.enabled = true`
6. call `InitFlashlightProbeExplaino(...)`
7. apply render defaults
8. execute a bounded multi-tick walk
9. render each tick through the real CUDA runtime
10. sample the low-res lens SDF and write JSON

`InitFlashlightProbeExplaino(...)` in `flashlight_probe_init.cpp` is important
because it makes the probe safe and deterministic:

- applies fractal preset defaults
- disables camera automation
- disables `auto_refresh`
- sets seed/warp/phase/drift fields
- optionally finalizes coherence with:
  - `UpdateExplainoPolynomial(...)`
  - `SyncViewHpFromUi(...)`

The tests confirm this behavior.

### 3.3 Text projection

`conversation_seed_spectrum.cpp` currently does:

- `seed32 = Fnv1a32Bytes(text.data(), text.size())`
- `spectrum8` = 8 interleaved FNV-style checksums

This is a deterministic orientation input, not a semantic embedding. That is a
real limitation, but it does not invalidate the rest of the geometry pipeline.

### 3.4 Manifold walk

The probe walk is implemented by `FlashlightManifoldAt(...)` in `main.cpp`.

Current properties:

- deterministic from `(seed32, walkT)`
- 4 bands
- shrinking perturbation scale `0.5^band`
- perturbation in:
  - camera center x
  - camera center y
  - log2 zoom

This is the structural core of the probe. It is what makes the trace a local
geometry walk rather than a sequence of unrelated views.

### 3.5 Tick-state evolution

Per tick, the headless path updates:

- `view.log2_zoom`
- `view.center_hp_x`
- `view.center_hp_y`
- `view.explaino_seed_drift`
- `view.explaino_phase`

Two drift modes exist:

- ordinary tick-index walk
- `auto_increment_seed` mode with a slower continuous drift rate

There is also optional additive seed modulation:

- `sinusoid`
- `sweep`
- `dreamy`

That modulation can come from CLI or prefill JSON. This matters because it is
the current hook for injecting small controlled drift variations into the probe.

## 4. Runtime Lens/SDF Coupling

Flashlight is directly coupled to the runtime lens system.

The headless and live flashlight paths both turn on:

- `lens.enabled = true`

Then they:

1. take the renderer's binary mask
2. normalize `lens.downsample`
3. downsample the mask with `DownsampleMaskPow2(...)`
4. sample the chamfer signed-distance field on that low-res grid

Important current facts:

- `NormalizeLensDownsamplePow2` only yields `1,2,4,8`
- downsample is top-left point sampling, not averaging
- signed distance convention is:
  - inside -> negative
  - outside -> positive

Headless probe uses 5 fixed UV sample sites and derives:

- `samples[]`
- `saddle.min_abs_signed_px`
- `saddle.near_count`
- `saddle.mixed_inside`
- `loss_proxy.sdf_center_signed_px`
- `delta.d_sdf_center_signed_px`

So the lens/SDF path is already acting as a local geometry sensor.

## 5. The Coastline / Island Behavior

This section treats the phenomenon as real observed behavior from the original
work.

### 5.1 What the behavior is

Observed behavior:

- as sample count rises, the traced line becomes more complex
- it does not merely extend; it gains local structure
- the path can look coastline-like, branchy, and tendril-rich

### 5.2 What in current code can generate that

The most likely current drivers are:

- real basin/escape boundary geometry in the lens mask
- repeated local manifold perturbations
- signed-distance boundary pressure
- mixed inside/outside neighborhoods
- live trace rendering with ridge and mold-tendril overlays

### 5.3 Headless vs live preservation

Current headless probe preserves only a compressed form of this:

- 5 low-res SDF sample points
- scalar saddle and delta summaries

Current live mode preserves a much richer form:

- ordered trace points
- spline/trail rendering
- ridge repetition
- SDF-driven mold tendrils
- composite projection over the live frame and SDF

So the current codebase already implies a split:

- headless = metric summary
- live = geometry-rich visible trajectory

If the original coastline-complexity behavior is a core requirement, any future
reusable extraction must preserve the live-grade ordered geometry, not just the
headless summary fields.

## 6. The Bridge Surface

This is the biggest gap in the earlier report.

There is a real bridge UI and file contract for Flashlight.

### 6.1 Bridge direction

The live flashlight UI explicitly labels itself:

- `Bridge: UI -> Workspace watcher`

It writes:

- `ui/diagnostics/last/flashlight_bridge_request.json`

It optionally reads:

- `ui/diagnostics/last/flashlight_bridge_prefill.json`

It can also surface:

- `flashlight_bridge_status.json`

### 6.2 Bridge request contract

The bridge request emitted by the live UI currently contains:

- `version`
- `kind = "flashlight_probe_headless"`
- `request_id`
- `ts_ms`
- `client_pid`
- `seed_path`
- `ticks`
- `warp`
- `closure_last`
- `no_export`

This is already a real compositional boundary. A workspace watcher or external
tool can consume the request file and execute headless probe work without being
tightly coupled to the UI thread.

### 6.3 Prefill contract

The optional prefill ingest path can populate:

- `seed_path`
- `ticks`
- `warp`
- `closure_last`
- `no_export`
- `prefill_id`
- `auto_emit`
- `note`

And under `view`:

- `auto_increment_seed`
- `explaino_seed_drift`
- `seed_modulation`

Seed modulation can carry:

- `mode`
- `amp`
- `period`
- `phase`

### 6.4 Auto-emit behavior

The bridge state tracks:

- `lastPrefillIdAutoEmitted`
- `pendingAutoEmit`

Behavior:

- a prefill can request one-shot auto emission
- the same `prefill_id` will not repeatedly re-arm `pendingAutoEmit`

That is an important detail because it proves the bridge was designed as a real
workflow handoff surface, not only as a developer toy.

### 6.5 Why this matters architecturally

This bridge is already one half of the external-tool story:

- the UI can describe a probe request
- a workspace-side watcher/tool can consume it
- the tool can write back status/prefill data

That is exactly the kind of seam that a separate reusable flashlight engine
should own more formally.

## 7. The Agent / Callable-Engine Interaction Story

The other missing seam is the callable engine/session surface in this repo.

### 7.1 What exists today

The repo already has a callable headless engine surface:

- `--describe-functions`
- `fractal.sample`
- `generic.sample`
- `--sample-request-*`
- `--sample-session`
- `--sample-session-pipe`

The callable-engine docs establish the pattern:

- explicit request/response objects
- deterministic transports
- optional `state_token` carry-forward in session mode
- alternate named-pipe transport

### 7.2 Why this matters to Flashlight

This surface is not Flashlight today, but it provides the correct external
composition model for Flashlight:

- one execution surface
- multiple transports
- deterministic request/result contracts
- stateful session support for iterative probing

That means Flashlight does not need to invent a new philosophy. It needs its
own request/result layer built in the same style.

### 7.3 Practical agent-bridge implication

A future agent workflow could compose:

1. `--describe-functions` / `fractal.sample` to inspect real runtime parameter
   surfaces
2. Flashlight bridge request/prefill files to schedule or bias flashlight runs
3. session `state_token` carry-forward to run incremental steering experiments
4. flashlight result JSON to generate drift/coherence steering hints

In other words:

- `sample_fn` / callable engine = generic execution substrate
- Flashlight = geometry/drift probe built on top of that substrate

That is the compositional story the earlier writeup did not cover.

## 8. Relation To Sidecar Orientation And Divergence

Flashlight is not the same thing as the Explaino sidecar, but the sidecar stack
contains the nearest existing formalization of orientation and divergence.

### 8.1 Sidecar orientation vector

`SidecarOrientationVector` currently contains:

- `import_signature`
- `pack_projection_hash`
- `field_embedding_stats`
- `slime_energy_delta`
- `busy_beaver_metrics`
- `decode_stability`
- `diff_magnitude`

It is computed from the measured Explaino parameter surface, not from the
flashlight manifold walk.

### 8.2 Divergence logic

`BuildSidecarStateDivergence(...)` compares previous vs current orientation and
marks:

- `stable`
- `diverged`
- `unavailable`

It accumulates a scalar divergence from:

- import signature changes
- projection hash changes
- normalized deltas over the orientation fields

### 8.3 Why this matters to Flashlight

This sidecar machinery is relevant because your clarified intent is about:

- orientation over time
- gradient/drift hints
- coherence vs chaos

The sidecar already has a machine-readable drift/difference concept. It is just
not yet wired to flashlight's trace geometry.

So the right relationship is:

- flashlight = geometry-bearing runtime probe
- sidecar orientation/divergence = current in-repo drift formalization

Those two seams should eventually meet.

## 9. Measurement Surfaces Reviewed

### 9.1 Historical flashlight probe JSON

Historical reference artifact:

- `C:\Users\Adam\Desktop\cuda_newton_fractal\ui\diagnostics\last\flashlight_probe.json`

Measured:

- `ticks = 16`
- evenly distributed 4-band walk
- `iters_avg min/max/mean = 7 / 14 / 9.1875`
- center SDF min/max/mean =
  `-65.82843018 / 47.0 / 6.60171312125`
- `mixed_inside_ticks = 16 / 16`
- `best_saddle_min_abs_signed_px = 2.41421366`

This is concrete evidence that the headless probe contract once emitted real
geometry-bearing data.

### 9.2 Current launch check

A current headless flashlight launch against read-only mainline:

- exited `0`
- updated `cli_debug.json` with the correct flashlight arguments
- did **not** leave the expected flashlight artifact bundle in the obvious
  diagnostics path checked during this slice

That means the current "works today" verdict cannot honestly be called fully
green.

### 9.3 User-provided trace artifacts

Reviewed:

- `C:\Users\Adam\Downloads\flashlight_trace.stl`
- `C:\Users\Adam\Downloads\flashlight_trace_frame.bmp`
- `C:\Users\Adam\Downloads\flashlight_trace_overlay.bmp`

Measured STL facts:

- header `flashlight_trace`
- `35520` triangles
- bbox:
  - min `[-2.4625916481, -1.8759042025, 6.8921751976]`
  - max `[ 2.4669885635,  1.9170464277, 14.0129222870]`

These artifacts prove that the broader flashlight workflow included:

- a visible trace image
- a composited overlay
- a 3D geometry export path

The STL writer itself was not found in the reviewed seams, so the export step
is currently best treated as a downstream layer over flashlight outputs.

## 10. Does It Still Work Today?

### 10.1 What is definitely still there

- explicit flashlight CLI flags
- flashlight init and tuning code
- shared headless/live manifold schedule
- runtime lens/SDF sampling in flashlight
- live trace/composite windows
- bridge request/prefill surface
- historical artifact contract

### 10.2 What is only partially proven today

- current headless artifact bundle emission
- current downstream STL export path
- current strength of the original coastline-complexity phenomenon in headless
  mode
- current steering usefulness of the emitted metrics

### 10.3 Honest verdict

The feature still exists and still has meaningful implementation weight.

But the full original promise is only partially realized in currently verified
surfaces:

- the probe math is present
- the live trace machinery is present
- the bridge surface is present
- the current machine-readable steering surface is incomplete
- the present-day artifact proof is not fully re-established yet

## 11. What Is Missing Relative To The Original Feature Goal

The current code does **not** yet provide a first-class external result saying:

- "this run drifted toward coherence"
- "this run drifted toward chaos"
- "this run approached the lower-right 7th radial tendency"
- "this run drifted into the left-middle chaotic region"
- "apply these next-prompt micro-corrections"

It also does not yet join these three layers into one engine contract:

1. flashlight trace geometry
2. orientation/divergence scoring
3. steering hint generation

That is the real gap.

## 12. Separate Reusable Engine / Tool Design

Flashlight should be extracted as a separate engine/tool, not folded into
`sample_fn`.

### 12.1 Design rule

Follow the `sample_fn` architectural pattern:

- explicit request
- explicit result
- deterministic engine seam
- UI-independent execution
- multiple transports/consumers

But keep Flashlight separate because it owns additional policy:

- text-conditioned orientation seeding
- manifold scheduling
- closure semantics
- trace-growth handling
- drift/coherence interpretation
- optional bridge interaction
- optional geometry export

### 12.2 Engine seams to extract

The reusable engine should own:

1. text -> `{seed32, spectrum8}`
2. manifold schedule
3. probe init
4. per-tick geometry sampling
5. ordered trace geometry
6. summary/closure accumulation
7. drift/coherence analysis
8. bridge request/prefill handling
9. artifact writing
10. optional export adapters

### 12.3 External request/result surface

Request should include:

- seed text or seed path
- runtime/fractal selection
- tick/radius/zoom/warp/closure tuning
- optional modulation
- output policy
- optional bridge metadata
- optional analysis policy

Result should include:

- seed/orientation info
- ordered tick trace
- sampled lens/SDF metrics
- closure result
- drift/coherence features
- optional regional occupancy / radial metrics
- optional steering hints
- artifact/export paths

### 12.4 Why this should stay separate from `sample_fn`

`sample_fn` is the generic sampler.

Flashlight is a composed analysis and steering probe. It needs:

- more policy
- more result structure
- more geometry semantics
- bridge workflow semantics

So it should consume the same runtime philosophy, not become a generic sampler
mode.

## 13. Recommended Next Measurement Slice

To complete the feature proof, the next slice should do all of the following.

1. Re-prove present-day output locations for headless flashlight artifacts.
2. Capture a current live flashlight run with fixed seed input.
3. Run a sample-density sweep and measure whether line complexity increases.
4. Add explicit regional metrics:
   - lower-right radial occupancy
   - left-middle chaotic occupancy
5. Join trace geometry with divergence/orientation scoring.
6. Emit an experimental machine-readable steering-hint payload.

## 14. Bottom Line

The full flashlight feature is not just "a headless probe" and not just "a
pretty trace."

It is a multi-layer system:

- deterministic text-conditioned orientation input
- bounded manifold walk through the real CUDA Explaino runtime
- runtime lens/SDF geometry sampling
- live trace and composite visualization
- a bridge surface for workspace/external orchestration
- a natural future fit for the callable-engine/session contract style

Your clarified claim is also the right one:

- not literal latent-space tracking
- not training-data access
- yes to geometric drift/orientation hints over an isomorphic working map
- yes to the idea that local gradient behavior can help detect coherence vs
  chaos
- yes to the idea that tiny next-prompt corrections may steer away from chaotic
  regions

What the repo still lacks is the final integration of those ideas into a formal
external flashlight engine/tool that emits explicit drift/coherence/steering
results instead of leaving that interpretation mostly implicit in probe traces
and artifacts.
