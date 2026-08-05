# Model Diagnostics Overlay Capability Campaign

Status: documentation-only preplanning; product implementation is paused.

## Current Phase

Documentation planning, truth sync, and hostile validation are complete. Product implementation is paused. No viewer, renderer, capture, or analyzer behavior is authorized by this plan.

## Explicit User Asks

- [x] Document the general scientific/math diagnostics overlay idea in enough detail for a later session to resume without chat history.
- [x] Treat one-provider or one-fractal usefulness as a near showstopper.
- [x] Plan bounded slices, comparable to the IFS campaign, instead of beginning a large implementation now.
- [x] Preserve an optional annotated Capture Finding path without changing the canonical frame.
- [x] Pause after checked-in planning, validation, commit, push, and rearward review.

## Scope

This campaign plans a generally useful Model Diagnostics surface for every public fractal selector. It does not require every family to expose identical mathematics. It requires every selector to declare what mathematical model it owns, which coordinate spaces its facts inhabit, which facts can be derived truthfully, and why unavailable facts are unavailable.

The external engine-state tool remains a parity reference and future consumer. It is not a runtime dependency of the viewer.

### Forbidden In This Documentation Slice

- Product code or test mutation.
- A new viewer button, panel, overlay renderer, model solver, or capture artifact.
- Python execution from the viewer.
- Pretending the single existing polynomial provider covers the catalog.
- Drawing a fact in a viewport whose coordinate space does not match.
- Changing canonical Capture Finding pixels, state schema, or replay authority.

## Phase Checklist

- [x] Phase 0 - Inspect live catalog, model-provider, viewport-transform, capture, and external-tool authority.
- [x] Phase 1 - Lock the general capability architecture and coordinate-space rules.
- [x] Phase 2 - Define bounded implementation campaigns and per-slice gates.
- [x] Phase 3 - Sync the existing diagnostics spec, status, known-issue, and deferred surfaces.
- [x] Phase 4 - Run planning validation, hostile audit, code-quality baseline, and diff check.
- [x] Pause before any product implementation.

Commit, push, receipts, and rearward review are repository closure evidence, not preclaimed plan checklist state.

## Repo-Grounded Starting Point

- The public selector catalog contains 51 entries with append-only enum ids 0 through 50.
- The current active-model seam has one concrete provider, `polynomial_over_power_escape.v1`, for the zero-warp `explaino_rational_escape` case.
- The viewer already owns rotation-aware pixel-center complex mapping and a complex-plane basis in `fractal_viewport_facts`.
- Capture Finding already separates canonical frame/state authority from review sidecars.
- The external state tool already demonstrates deterministic derivation and annotation of critical, fixed, and singular points for the polynomial-over-power provider.
- Existing probe/sample capability is not universal enough to assume that one orbit diagnostic can be enabled blindly for all selectors.

These facts make the feature feasible, but they do not justify a quick one-family overlay.

## General Usefulness Lock

A public diagnostics UI is not allowed until all current selectors pass a catalog coverage court.

For each selector, the court must record:

1. model class;
2. diagnostic target and coordinate space;
3. exact, sampled, projected, heuristic, or unavailable evidence status;
4. at least one meaningful diagnostic capability;
5. provider and runtime authority;
6. projection eligibility for the active viewport;
7. cost/lifecycle policy;
8. structured reason for unsupported features.

A selector may expose fewer facts than another selector. It may not silently inherit mathematically false facts.

## Coordinate-Space Contract

Every feature carries an explicit space id:

- `parameter_plane`: the rendered pixel is a parameter such as Mandelbrot `c`.
- `dynamical_plane`: the rendered pixel is an initial state such as Julia `z0`.
- `field_world_plane`: authored SDF/field coordinates.
- `root_layout_plane`: root-pattern geometry before an explicit projection.
- `state_space_projection`: a declared 2D projection of higher-dimensional state.
- `screen_space`: UI-only measurements and labels.
- `nonspatial_fact`: facts that belong in tables/text and must not be drawn.

A point is drawable only when its feature space can be transformed into the current viewport space by a declared exact or explicitly lossy adapter. Sharing complex-number storage is not sufficient. For example, a dynamical critical point must not be drawn directly on a Mandelbrot parameter-plane viewport without a defined relation.

## Diagnostic Model Classes

Initial classification families:

1. Polynomial/root solvers: Newton, Nova, Halley, Schroeder, Householder, root-field consumers.
2. Holomorphic parameter-plane iterations: Mandelbrot, Multibrot, Lambda, Spider, Phoenix-like parameter views where applicable.
3. Holomorphic dynamical-plane and rational maps: Julia, rational escape, Magnet, McMullen.
4. Non-holomorphic folded maps: Burning Ship, Celtic, perpendicular/fold variants.
5. Stateful recurrences: Phoenix, Collatz, memory or second-order maps.
6. Transcendental maps: trigonometric/exponential Newton and related lanes.
7. Scalar-field producers: Lens SDF, Lens Field v2, authored SDF packs, `sdf_pack_scene`, Root SDF.
8. Generic equation packs: AST iterate/direct models with declared variables and parameters.
9. Composite/comparison lanes: ExplainO composites and counterfactual views with multiple model owners.

The future catalog audit must classify selectors from runtime/catalog authority and fail when a new selector lacks a classification.

## Typed Feature Receipt

Future providers emit `viewer.model_diagnostic_feature_set.v1`.

Required envelope:

- selector id and public fractal type;
- provider id/version;
- model class;
- target id;
- viewport space id;
- state and model fingerprints;
- derivation backend and precision;
- compute status and structured errors;
- features;
- scalar/text facts;
- timing and cache status.

Feature geometry kinds:

- `point`
- `polyline`
- `line`
- `circle`
- `vector`
- `contour`
- `region`
- `text_fact`

Each feature records:

- stable feature id;
- semantic kind;
- source space;
- optional target space and adapter id;
- epistemic status;
- coordinates/data;
- label and style role;
- residual/error where applicable;
- provenance and authority hash.

Epistemic statuses:

- `exact_runtime_authority`
- `exact_symbolic`
- `numerical_exact_equation`
- `sampled_runtime`
- `lossy_projection`
- `heuristic`
- `unavailable`

## Minimum Useful Coverage By Family

- Polynomial/root: roots, poles/singularities, critical points where defined, residuals, selected orbit.
- Parameter-plane holomorphic: parameter marker/facts, critical-orbit summary, escape/convergence facts; dynamical points remain nonspatial unless projected deliberately.
- Dynamical/rational: critical/fixed/pole points where derivable, selected orbit, residuals.
- Folded/non-holomorphic: selected orbit, fold-event markers, escape/convergence facts; no fake complex derivative facts.
- Stateful: state tuple, selected trajectory in a declared projection, cycle/escape facts.
- SDF/field: signed-distance sample, boundary/normal/curvature facts where backed, field bounds and producer authority.
- Generic packs: AST/node authority, selected sample/orbit when the pack declares an iterative model, otherwise field/direct facts.
- Composite: per-component facts and comparison relations; no merged authority that erases owners.

## Campaign 1 - Capability Foundation

### MD-0 Catalog Coverage Court (1-2 engineering days)

- Generate the selector-to-model-class/capability matrix from catalog and runtime authority.
- Add a machine validator that rejects missing selectors, duplicate ownership, invalid spaces, and capabilities without providers.
- Record current unsupported facts with reasons.
- No UI.

Gate: all 51 current selectors are classified.

### MD-1 Target, Binding, And Receipt Envelope (2-3 days)

- Add a model-diagnostic target contract independent of mouse state.
- Bind target, state hash, viewport facts, provider version, and precision.
- Add deterministic JSON serialization and cache key.
- No provider expansion beyond fixtures.

Gate: a target can be captured/replayed without reading live mutable UI state.

### MD-2 Universal Iteration-Evidence Substrate (3-5 days)

- Add bounded selected-pixel orbit/state tracing for compatible iterative models.
- Support stateful tuples and fold events through typed samples rather than pretending all traces are complex scalar orbits.
- Fail closed for direct fields and unsupported evaluator paths.
- Add runtime cost limits and cancellation.

Gate: every iterative selector either emits a truthful bounded trace or a tested structured denial.

### MD-3 Root And Polynomial Providers (3-5 days)

- Generalize root, critical, pole, fixed-point, and residual facts for polynomial/root families.
- Preserve captured/root-layout authority where it exists.
- Cross-check the external state-tool provider against the native receipt.

Gate: root-family facts match runtime authority and coordinate spaces.

### MD-4 Analytic And Rational Providers (4-7 days)

- Add family providers for Julia/Multibrot/Lambda/Magnet/McMullen/rational maps where equations are authoritative.
- Separate parameter-plane facts from dynamical-plane facts.
- Bound numerical solving and report residuals.

Gate: no point is projected across spaces implicitly.

### MD-5 Folded, Stateful, And Transcendental Providers (4-7 days)

- Add typed fold-event and state-tuple diagnostics.
- Add only mathematically valid derivative/fixed-point facts.
- Classify unavailable symbolic facts honestly.

Gate: non-holomorphic and stateful lanes are useful without fake holomorphic metadata.

### MD-6 Field, Generic-Pack, And Composite Providers (4-7 days)

- Cover SDF/field producers, generic AST packs, and multi-owner composites.
- Reuse field capability authority for signed-distance/normal/curvature facts.
- Preserve component ownership in comparison/composite receipts.

Gate: every selector has at least one useful capability or a product decision is required before UI work.

Campaign 1 estimate: 21-36 engineering days, depending on how much evaluator instrumentation can be shared safely.

## Campaign 2 - Product Surface

Campaign 2 cannot start until Campaign 1 coverage gates are green.

### UI-0 Diagnostics Window Shell (2-3 days)

- Add a button near `Load FITS...`.
- Add a compact resizable panel with target selection, capability list, compute status, and feature toggles.
- Capability-driven controls only; unavailable features show precise reasons.

### UI-1 Live Viewport Overlay (3-5 days)

- Render typed features through one overlay projection layer.
- Support labels, point/line geometry, visibility, and bounded update policy.
- Recompute on settled state by default; allow explicit refresh and cancellation.
- Never steal the mouse or require OS-cursor automation.

### UI-2 Annotated Capture Finding (3-5 days)

- Keep canonical `frame.png` unchanged.
- Optionally emit `frame-annotated.png` plus `math-overlay.json`.
- Bind both to state/frame/provider hashes.
- Replay proof must reproduce both canonical pixels and overlay receipt.

### UI-3 Hardening And External Consumer Cutover (2-4 days)

- No-mouse runtime matrix across every model class.
- Performance/cancellation/cache tests.
- Teach the external state tool to consume the shared receipt where useful; retain independent parity checks.
- Hostile audit of every selector capability.

Campaign 2 estimate: 10-17 engineering days.

## Later Campaigns

Deferred beyond the initial product vertical:

- region selection and multi-target comparison;
- symbolic differentiation or automatic differentiation;
- richer contour/basin/vector-field overlays;
- Science Mode corpus export;
- agent-optimized diagnostic packets;
- graph UI integration;
- 3D/state-space visualization.

## Performance And Lifecycle Rules

- Diagnostics computation is opt-in and off by default.
- Live render cadence must not wait on expensive solvers.
- Provider work uses immutable state snapshots, cancellation tokens, and bounded budgets.
- Cache keys include selector, state/model hash, target, provider version, precision, and viewport facts.
- Settled-state recompute is the default. Interaction may show stale-marked prior results, not block the viewport.
- Capture waits only when the operator explicitly requests annotated output.
- All timings are reported; no performance claim without measured witnesses.

## Required Proof Matrix

Future implementation must cover:

- every catalog selector in the coverage court;
- each coordinate-space class;
- exact and numerical provider receipts;
- structured unsupported cases;
- target/state replay;
- viewport projection parity including rotation/aspect;
- stale/cancelled work rejection;
- canonical capture preservation;
- annotated capture hash/provenance;
- no-mouse UI action coverage;
- external state-tool parity for shared cases.

## Risks And Mitigations

1. False universality: catalog court before UI.
2. Coordinate-space lies: explicit typed spaces and adapters.
3. One giant provider switch: registry ownership and per-family providers.
4. Solver stalls: bounded work, cancellation, settled-state scheduling.
5. Stale overlays: immutable state/model fingerprints.
6. Capture ambiguity: canonical and annotated artifacts remain separate.
7. Python runtime dependency: native contracts; external tool remains outside viewer.
8. Hand-maintained selector drift: generated catalog coverage validator.
9. Misleading numerical roots: residual/error and epistemic status.
10. UI overload: capability-driven compact panel.
11. Renderer coupling: overlay remains a consumer, never a fractal kernel path.
12. Agent-only usefulness: same receipt serves operator UI, capture review, and tooling.

## Proof Ledger

- [x] Current selector count and provider limitation inspected from live repo authority.
- [x] Viewport and capture seams inspected.
- [x] External state-tool provider and annotation path reviewed as parity evidence.
- [x] Existing diagnostics research spec reconciled as a narrower predecessor.
- [x] Status, deferred, and known-issue surfaces synchronized.
- [x] Contract validation passed.
- [x] Phased-plan sync passed.
- [x] Hostile-audit validation passed.
- [x] Code-quality baseline passed.
- [x] Diff check passed.

Final commit, receipts, push, and rearward review are recorded by their machine-owned repository surfaces.

## Hostile Audit

- Status: complete

The audit assumes the plan is misleading until disproven. It specifically asks:

- Does this accidentally authorize a one-provider UI?
- Does it confuse parameter, dynamical, field, root-layout, or screen spaces?
- Is every current/future selector forced through a catalog capability court?
- Can unavailable mathematics fail closed without making the panel useless?
- Does Capture Finding retain canonical pixel/replay authority?
- Does the viewer acquire a Python or state-tool runtime dependency?
- Are estimates and pause points explicit?
- Is product mutation still forbidden?

## Audit Passes

- [x] Pass 1 found a coordinate-space design defect: complex storage alone was insufficient to justify projection. Repaired with explicit spaces/adapters.
- [x] Pass 2 found a coverage defect: a selected-orbit vertical cannot be assumed universal because direct fields and some evaluator paths lack that contract. Repaired with typed trace/denial gates.
- [x] Pass 3 clean re-read confirmed no additional scope leak after separating capability foundation from product UI/capture campaigns.

## Audit Findings

- [x] Finding 1: A quick critical-point overlay would be useful only for a narrow provider and would create false product generality. Resolution: MD-0 through MD-6 gate UI work.
- [x] Finding 2: The older diagnostics spec did not make catalog-wide coordinate-space classification a prerequisite. Resolution: it now points to this campaign as the governing general-coverage plan.
- [x] Finding 3: Annotating the canonical finding frame would damage replay/review authority. Resolution: canonical frame remains unchanged; annotation is a sibling artifact.
- [x] Finding 4: The predecessor diagnostics spec retained a stale final recommendation to begin with a narrow Newton/ExplainO UI. Resolution: replace it with the MD-0 catalog-court restart boundary.
- [x] Clean re-read of the repaired planning surfaces found no additional real defect.

## Action Hostile Review

- Action ID: model-diagnostics-overlay-preplanning-truth-sync-5
- Suspected Failure Mode: Documentation could describe the polished panel before proving broad model capability, leading future work directly into a one-family demo.
- Correct Owner/Action: Synchronize status, known-issue, deferred, and predecessor-spec surfaces around the capability-first gate.
- Proof Surface: checked-in plan/contract plus contract, plan-sync, hostile-audit, code-quality, and diff-check receipts.
- Blocked Action: no product source, test, schema, UI, renderer, capture, or analyzer mutation.

## Closeout And Pause

This bounded slice ends after documentation checkpoint, receipts, push, and rearward review.

Preplanned sliced work for this bounded documentation slice is exhausted; stop for replan before Model Diagnostics product mutation.
