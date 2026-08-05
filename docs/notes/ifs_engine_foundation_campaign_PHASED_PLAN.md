# IFS Engine Foundation Campaign

## Current Phase

Current bounded slice: Phase 4 complete - the restart-safe campaign authority and cross-repo published-runtime provider test repair are proven. IFS product implementation remains deferred; stop for a separate two-day-task replan.

## Phase Checklist

- [x] Phase 0 - recover live repo authority, rearward review, the exact P2 testing finding, and the attached final-vision report.
- [x] Phase 1 - lock a workflow-only preplanning contract without changing renderer or product behavior.
- [x] Phase 2 - publish this detailed bounded IFS campaign and sync deferred/status authority.
- [x] Phase 3 - add and run the mandatory state-tool provider-to-published-engine integration rail.
- [x] Phase 4 - complete hostile review and focused/full validation; checkpoint, receipts, push, and clean-tree proof are the closure commands for this committed state.
- [x] Pause reached - select a separate task that fits the remaining two-day window; do not begin IFS implementation automatically.

## Explicit User Asks

- [complete] Document the IFS and future Science Mode direction thoroughly enough for a later session to resume without chat history.
- [complete] Decompose the work into bounded slices with dependency order, proof gates, estimates, and explicit pause points.
- [complete] Fix the previously identified P2 gap: no mandatory automated test currently runs the state tool polynomial-model provider directly against the published engine.
- [complete] Do not begin the multi-week IFS engine implementation before the user's travel.
- [complete] Close this bounded work, then return to a smaller task that can be completed within two days.

## Scope Of This Slice

Allowed now:

- checked-in IFS campaign planning and status/deferred truth sync;
- one state-tool integration test/runner that invokes `ActiveModelRuntimeClient` against the published engine and an exact captured state;
- focused and full tests needed to prove that rail;
- workflow closure artifacts.

Forbidden now:

- IFS structs, kernels, histogram buffers, runtime operations, selectors, UI, presets, SDF bridges, or Science Mode bindings;
- modifications to `RenderFractalCUDA`, `KernelParams`, `FractalSampleResult`, or Color Pipeline behavior;
- claims that the attached report is already implemented;
- universal CPU/CUDA exact-pixel promises for arbitrary non-dyadic floating affine maps;
- choosing or starting the next two-day product task before this slice closes.

## Authority And Evidence

Current repository facts at campaign creation:

- Engine repository: clean `master` at `e6b90cc4fba09618957f861d904501d59ef5dbca`, synchronized with `origin/master`, rearward review `ok`.
- State-tool repository: clean `main` at `76b43e28d091aa6f2a5996565c2d99aaa045cf1b` at the prior review boundary; revalidate before mutation and closure.
- Published runtime authority: `D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe`, resolved through the normal runtime launcher/active-file surface.
- Vision reference: `C:/Users/Adam/Downloads/Salticid_Runtime_IFS_Science_Mode_Consolidated_Report_2026-07-30.md`.
- Existing engine is pointwise: one render thread/pixel calls the device sampler and writes RGBA/mask/iteration/source-signal outputs.
- Existing SDF producers are a useful architectural precedent for separate CPU/CUDA producer APIs and backend receipts, but their field math is not the IFS domain model.
- There is no engine-owned persistent density histogram/accumulation authority suitable for IFS today.
- Existing deferred docs correctly state that IFS needs its own substrate before an IFS-to-SDF bridge.

The attached report is planning input. Current code, tests, published runtime behavior, and future checked-in contracts remain implementation authority.

## Corrected Feasibility Position

A useful affine IFS vertical is feasible, but it is not a new `FractalType` plus one formula branch. It requires an accumulation renderer because a chaos-game orbit is a continuing stochastic process writing a shared density image, not one independent answer per viewport coordinate.

The architecture should therefore be:

```text
IFS system description
  -> deterministic logical sample assignment
  -> CPU or CUDA accumulation producer
  -> integer visit histogram + receipts
  -> deterministic normalization
  -> typed scalar density source
  -> existing Color Pipeline presentation
```

The initial vertical deliberately excludes graph-directed lineage, SDF conversion, nonlinear Flame variations, History Horn geometry, and Salticid runtime coupling.

## Determinism Lock

A fixed random seed alone is insufficient. Replay identity must include:

- algorithm/version id;
- ordered map ids and coefficients;
- normalized map-selection weights;
- chain count and exact logical chain/sample ranges;
- counter-based PRNG identity;
- initial point policy;
- burn-in count;
- viewport mapping and out-of-view policy;
- integer histogram dimensions and counter width;
- normalization policy;
- presentation recipe/state kept separate from accumulation identity.

Parity is split into two courts:

1. **Exact dyadic court**: dyadic coefficients and view mapping chosen so CPU/CUDA map selection, positions, bins, and histogram hashes can match exactly.
2. **General floating court**: exact map-selection/sample identity and deterministic per-backend hashes are required; CPU/CUDA geometry is judged by bounded occupied-cell, mass, centroid, and density error. Tiny floating differences at bin edges are not mislabeled as algorithm failure.

No early authoritative fields may use concurrent floating accumulation, last-writer identity, or last-map-to-touch.

## Campaign 1 - Ordinary Affine IFS Product Vertical

### Slice IFS-0 - Contract And Model Authority

Estimate: 1-2 engineering days.

Goal: define a renderer-independent V1 contract without product UI.

Deliverables:

- `IfsAffineMap2D`: stable map id, 2x2 matrix, translation, nonnegative weight.
- `IfsSystemDesc`: schema/version, ordered maps, system id/name, optional preset provenance.
- `IfsRenderSettings`: output dimensions, viewport transform, sample count, chain count, burn-in, seed, backend preference, counter policy.
- `IfsAccumulationIdentity`: canonical hashes for algorithm, maps, schedule, viewport, and normalization.
- `IfsDensityResult`: integer histogram, dimensions, in-view/out-of-view counts, overflow status, deterministic hashes.
- `IfsRenderReport`: requested/used backend, timings, sample accounting, contractivity results, and failure reason.

Admission rules:

- finite coefficients and weights only;
- `2 <= map_count <=` a locked V1 maximum selected from measured memory/dispatch evidence;
- at least one positive weight;
- deterministic normalization in ordered-map order;
- conservative V1 contractivity check `sigma_max(A) < 1` with explicit tolerance and report;
- overflow and allocation bounds checked before execution;
- invalid descriptors fail closed rather than renormalizing unknown data silently.

Proof:

- parser/model tests for duplicate ids, unknown fields, malformed matrices, nonfinite values, weight errors, map-count limits, contractivity accept/reject, and stable canonical hashes;
- no CUDA or UI claim yet.

Pause gate: if the model cannot represent all five initial court systems without preset-specific fields, replan before a kernel exists.

### Slice IFS-1 - Two Independent CPU References

Estimate: 2-3 engineering days.

Goal: establish independent geometry and sampling oracles.

Deliverables:

- deterministic CPU chaos-game accumulator using the exact V1 counter-based selection/schedule contract;
- finite-depth prefix/cylinder enumerator that does not reuse the chaos-game loop;
- CPU report and histogram serialization suitable for later CUDA parity.

Why two references:

- chaos game proves invariant-measure sampling and accumulation;
- prefix enumeration proves address geometry, map order, and bounded cylinders;
- agreement on support/geometry catches errors that duplicated chaos-game implementations would share.

Proof court:

- Sierpinski triangle;
- Sierpinski carpet;
- Barnsley fern;
- a verified rotated or overlapping dragon-family system;
- one engine-owned overlap stress system;
- exact hand-computable short schedules;
- burn-in, zero samples, out-of-view, and counter-overflow boundaries.

Pause gate: do not start CUDA until prefix and chaos-game evidence agree on the intended support for every preset.

### Slice IFS-2 - Deterministic CUDA Accumulation

Estimate: 3-5 engineering days.

Goal: add a dedicated CUDA producer, separate from `RenderFractalCUDA`.

Execution contract:

- independent logical chains with deterministic counter-based PRNG inputs;
- exact chain/sample range assignment independent of block scheduling;
- integer atomic per-pixel visit counters;
- explicit in-view, out-of-view, rejected, and overflow accounting;
- no per-sample host round trips;
- CPU fallback is explicit and reported, never silent.

Proof:

- exact dyadic CPU/CUDA histogram parity;
- exact map-selection sequence and sample accounting on all courts;
- deterministic repeated CUDA histogram hashes;
- bounded general-floating parity metrics;
- odd dimensions, tiny/large sample counts, multiple chain counts, unavailable CUDA, allocation failure, and overflow tests;
- sanitizer/code-quality checks at the new producer boundary.

Pause gate: if deterministic assignment requires renderer-global coupling, stop and revise the producer API rather than entering `fractal_renderer.cu` ad hoc.

### Slice IFS-3 - Headless Runtime Operation

Estimate: 2-3 engineering days.

Goal: publish the accumulator as an engine operation before adding a viewer lane.

Proposed operation: `ifs.accumulate`.

Request authority:

- versioned IFS descriptor or curated preset id;
- versioned render settings;
- explicit backend preference;
- explicit output/report paths or response encoding bounds.

Response authority:

- operation/request ids and success/failure;
- accumulation identity and hashes;
- backend/timings;
- histogram dimensions and accounting;
- normalization metadata;
- no Color Pipeline pixels presented as accumulation truth.

Proof:

- request schema rejection/fail-closed tests;
- CPU/CUDA parity courts through the published runtime;
- repeated request determinism;
- file/stdout equivalence where both are supported;
- runtime catalog visibility and malformed-input diagnostics;
- compact PNG/gallery tooling may consume outputs, but cannot become execution authority.

Pause gate: headless operation and receipts must be stable before a normal selector is added.

### Slice IFS-4 - Viewer `ifs_scene` Vertical

Estimate: 3-5 engineering days.

Goal: one normal selectable viewer lane that remains outside the pointwise escape renderer.

Product surface:

- append-only `FractalType::ifs_scene` only if current catalog authority confirms a selector is the correct product seam;
- curated preset dropdown for the five proof systems;
- controls for sample budget, chain count, burn-in, seed, and reset/reseed policy;
- exact viewport/camera semantics defined for accumulated world coordinates;
- typed `ifs_density` scalar source feeding the existing Color Pipeline;
- clear progress/full-quality state for accumulation;
- state save/load, Capture Finding, replay, report, and `fractal-state.json` authority.

Preservation:

- no entry into `RenderFractalCUDA` pointwise dispatch;
- no use of `KernelParams` as the IFS system model;
- ordinary fractal/SDF lanes unchanged;
- live preview degradation never leaks into full-quality capture/replay.

Proof:

- selector identity and visible-control authority;
- every control changes the appropriate identity/report/output or is intentionally hidden;
- preset switching resets/reuses accumulation only according to an explicit key;
- Color Pipeline changes presentation without changing histogram identity;
- no-mouse capture/replay hash proof;
- stale accumulation never survives a map, seed, camera, dimension, or schedule-key change.

Pause gate: client manual acceptance is required before performance policy work or additional presets.

### Slice IFS-5 - Realtime Pacing, Cache, And Hardening

Estimate: 3-4 engineering days.

Goal: make the proven lane usable for exploration without corrupting full-quality authority.

Work:

- split timings: validation, scheduling, CUDA accumulation, copy/readback, normalization, Color Pipeline, total;
- live-only adaptive sample budget and/or field resolution based on measured frame budget;
- one full-quality settle render after interaction;
- safe histogram reuse only under the complete accumulation identity;
- bounded GPU memory and integer-overflow policy;
- 1024/2048 representative performance witnesses;
- hostile review of reset, animation, capture, replay, and state transitions.

Acceptance:

- low-cost views are not degraded unnecessarily;
- expensive views show measured responsiveness improvement;
- settle/capture/replay use requested full-quality settings;
- before/after medians and p95 are recorded; no unsupported FPS claim;
- all five presets have deterministic proof packets.

## Mandatory Campaign Pause

Stop after IFS-5. The ordinary affine vertical is independently useful and must be reviewed in the client before graph-directed IFS, IFS-to-SDF, Flame, History Horn, or Science Mode work begins.

Estimated Campaign 1 effort: 14-22 engineering days for the six slices, plus manual acceptance/repair contingency. The earlier 16-27 day product estimate remains the safer calendar range under this repo's validation and hostile-review requirements.

## Campaign 2 - Graph-Directed And Address-Aware IFS

This campaign is preplanned but not authorized by Campaign 1.

### Slice GIFS-0 - Graph Contract

Estimate: 2-4 days.

- stable vertex and edge ids;
- edge-owned affine transforms;
- explicit adjacency and initial vertex;
- parallel edges preserved as distinct identities;
- conditional outgoing weights normalized per vertex;
- unreachable/dead-end/invalid graph diagnostics.

### Slice GIFS-1 - Prefix And Cylinder Authority

Estimate: 3-5 days.

- finite legal-prefix enumeration;
- cylinder bounds/geometry;
- address lineage and stable prefix ids;
- forbidden transition and parallel-edge courts;
- symbolic multiplicity kept separate from occupied pixels and density.

### Slice GIFS-2 - Deterministic Graph-Directed CUDA Sampling

Estimate: 4-7 days.

- chain state includes current graph vertex;
- deterministic conditional edge selection;
- exact edge sequence receipts in bounded courts;
- histogram identity remains schedule-independent;
- address/debug outputs are bounded and never last-writer authority.

### Slice GIFS-3 - Viewer Diagnostics And Productization

Estimate: 3-6 days.

- graph/prefix diagnostic modes;
- prefix-aware coloring only where typed source authority exists;
- capture/replay/report lineage;
- client review before any quotient or horn geometry.

Estimated Campaign 2 effort: 12-22 engineering days.

## Later Separate Campaigns

### IFS To SDF Bridge

Estimate: 3-6 days after Campaign 1.

- define whether the bridge consumes thresholded support, normalized density contours, or explicit finite-depth geometry;
- produce a real SDF field with backend/dimension/downsample receipts;
- keep IFS histogram identity and SDF field identity separate;
- do not infer exact set distance from a low-resolution density bitmap without labeling approximation.

### Flame And Nonlinear Variation Substrate

Not a small IFS extension. Requires a versioned nonlinear transform library, density/color accumulation semantics, numerical stability policy, and its own CPU/CUDA courts.

### ExplainO IFS

Only after deterministic IFS receipts exist. Candidate lenses may explore map weights, contractivity margin, address multiplicity, overlap, or density response, but ExplainO controls cannot substitute for the base IFS authority.

### Salticid Science Mode And History Geometry

Cross-repo, estimated in weeks rather than days. Runtime/checkpoint/fork authority, offline FITS analysis, quotient observers, and History Horn/Pinched Return geometry remain separate proof layers. Geometry is presentation/analysis, never runtime truth.

## Known Architectural Risks

1. **Pointwise-renderer contamination**: forcing IFS through `FractalSampleResult` or `RenderFractalCUDA` would create a misleading and brittle monolith.
2. **False determinism**: seed-only claims ignore scheduling, sample ranges, map order, viewport bins, normalization, and overflow.
3. **Floating bin-edge drift**: universal exact CPU/CUDA histogram parity is not defensible for arbitrary maps.
4. **Identity collapse**: histogram, normalized density, colored frame, symbolic addresses, and quotient classes are distinct authorities.
5. **Progressive-state ambiguity**: accumulation reset/reuse needs a complete key and explicit generation/sample-range receipts.
6. **UI-before-substrate**: a selector or graph editor cannot compensate for missing headless accumulation authority.
7. **SDF category error**: existing SDF infrastructure is an API precedent, not an IFS implementation.
8. **Scope collapse**: graph-directed, Flame, SDF hybrid, History Horn, and Science Mode must not be bundled into the affine foundation.

## Campaign Proof Matrix

| Claim | Minimum evidence |
|---|---|
| Descriptor is valid | parser/model unit courts and canonical hash |
| CPU geometry is credible | independent prefix and chaos-game agreement |
| CUDA is deterministic | repeated histogram hash plus exact schedule/accounting |
| CPU/CUDA match | exact dyadic court plus bounded general-floating parity |
| Runtime is authoritative | published `ifs.accumulate` request/response proof |
| Viewer is truthful | no-mouse selector/control/report/capture/replay proof |
| Realtime is improved | before/after medians and p95 on identical scenarios |
| SDF bridge is valid | separately versioned field identity and semantic parity |
| Graph lineage is valid | prefix/edge/parallel-edge and forbidden-transition courts |
| Science claim is valid | separate runtime/checkpoint/observer evidence, not geometry alone |

## P2 State-Tool Runtime Test Repair

The prior review found a concrete integration gap:

- state-tool provider tests use synthetic active-model receipts and sample responses;
- engine runtime tests exercise active-model and canonical sampling independently;
- no mandatory automated rail instantiates the state tool's `ActiveModelRuntimeClient` against the published engine and an exact state.

The repair must add a dedicated integration test/runner in `C:/code/cuda-fractal-engine-state-tool` that:

1. resolves the published runtime executable through the normal launcher/active-file authority;
2. uses an exact captured `explaino_rational_escape` state with zero warp;
3. calls `ActiveModelRuntimeClient.describe` against the real executable;
4. derives polynomial-model features from that real receipt;
5. calls `ActiveModelRuntimeClient.sample` against the same state/runtime binding;
6. asserts selector, state hash, executable hash, provider id, model id, denominator power, numeric backend, and finite returned samples;
7. fails rather than silently skips when invoked through the dedicated integration command;
8. remains outside ordinary hermetic unit discovery unless the published runtime/fixture are explicitly supplied, so CI without the external runtime does not lie or fail for environmental absence.

The checked-in validation command for this slice must run that dedicated non-skipping integration rail and the complete state-tool unit suite.

## Proof Ledger

- [complete] Exact P2 text recovered from the prior review task record.
- [complete] Attached IFS report reviewed against current pointwise renderer and deferred substrate docs.
- [complete] Determinism acceptance corrected into exact dyadic and bounded general-floating courts.
- [complete] State-tool repair committed and pushed as `80d0dd0` on `codex/published-runtime-provider-integration-test`.
- [complete] Direct state-tool provider-to-published-engine integration proof passed against the published CUDA/float64 runtime and exact captured denominator power `3` state.
- [complete] Complete state-tool suite passed: 223 tests in one shell.
- [complete] Three-pass hostile review found and repaired three concrete validation gaps.
- [complete] Final engine contract/plan/code-quality/diff/audit validation passed; checkpoint, receipts, push, clean tree, and rearward review are enforced closure commands.

## Hostile Audit

- Status: complete

Questions:

- Did this slice accidentally begin IFS implementation?
- Does every future slice have a bounded deliverable and a stop/replan gate?
- Did the plan keep accumulation, normalization, presentation, addresses, and quotient observations as separate authorities?
- Did it avoid an impossible universal exact floating CPU/CUDA promise?
- Does the P2 repair execute the real state-tool production client against the real published engine?
- Can the dedicated integration rail silently skip and still report success?
- Does ordinary CI remain hermetic while local/release validation makes the cross-repo rail mandatory?
- Are both repositories clean, committed, and pushed at closure?
- Is preplanned work exhaustion stated honestly?

## Audit Passes

- [complete] Pass 1 - found that duplicate JSON keys were accepted and backend/arithmetic were reported without being enforced; added regressions and fail-closed checks.
- [complete] Pass 2 - found that the derived provider denominator was reported but not compared to the exact captured-state denominator; added a mismatch regression and exact equality check.
- [complete] Pass 3 - clean re-read of the final runner, six focused tests, 223-test suite, real published-runtime receipt, plan scope, and both repository states found no further defect.
- [complete] Pass 4 - receipt preflight found that the direct phased-plan sync command emitted no parseable evidence; replaced it with a logged command and explicit contract assertion.
- [complete] Pass 5 - reran contract, plan-sync, hostile-audit, and diff evidence after the receipt repair with no additional defect.

## Audit Findings

- [complete] Workflow setup finding: the active-contract guard cannot directly create a missing checked-in plan/contract because contract validation requires scoped paths to exist. A temporary artifact-scoped bootstrap contract was required. This is tooling friction, not IFS product work; no guard bypass was used.
- [complete] State authority finding: duplicate keys in the captured-state JSON could make the integration witness ambiguous. The runner now uses duplicate-key-rejecting JSON parsing.
- [complete] Runtime authority finding: a CPU or non-float64 response could previously pass while merely being reported. The dedicated rail now requires CUDA and float64 iteration arithmetic.
- [complete] Provider/state binding finding: a provider derived with the wrong denominator could previously pass. The rail now requires exact equality between captured-state, provider metadata, and model denominator power.
- [complete] Receipt authority finding: direct phased-plan sync produced no parseable JSON evidence. The contract now requires a logged plan-sync command and asserts its machine receipt.

## Closeout And Next Boundary

This bounded slice is complete only when:

- the campaign plan and status/deferred pointers are checked in;
- the P2 cross-repo integration rail executes and passes without skip against the published runtime;
- the state-tool full unit suite remains green;
- both repositories are clean, committed, and pushed;
- engine contract validation, plan sync, hostile audit, receipts, and rearward review are green.

After closeout, preplanned IFS product implementation remains deferred. Stop and choose a smaller task whose implementation and proof can fit inside the remaining two-day window.


Preplanned sliced work for this bounded docs/test repair is exhausted; stop for replan before IFS product mutation.
