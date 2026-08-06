# Curated Color Recipe Authority Campaign

## Explicit User Asks

- [x] Incorporate the accepted local and external hostile-review amendments before implementation.
- [x] Repair recipe application authority before shipping the curated recipe pack.
- [x] Preserve the current Color Pipeline layout and public selector/Apply workflow.
- [x] Keep external state-tool integration deferred; engine-native rails remain authoritative.
- [x] Execute Slice 0 baseline lock before product mutation.
- [ ] Continue through independent authority, Beauty, and recipe gates only while prerequisites remain green.

## Current Phase

Slice 4 - Existing Recipe Parity is active from clean, pushed, receipted, rearward-reviewed Slice 3 checkpoint ac07b2a92039987ff7975182329afabd8647b4be.

The hard-denial workflow blocks raw branch mutation but this checkout has no approved branch/merge wrapper. This campaign therefore starts as an explicitly stacked slice on codex/model-diagnostics-overlay-preplanning. Eventual upstream integration must preserve both closed planning campaigns.

Slices 0 through 3 are closed. Slice 4 adds prepared and committed recipe-application receipts, truthful graph-versus-fallback authority, and exact/modified/none/unknown-after-reload recipe provenance while preserving the existing six recipes bit-exactly. SDF Beauty metadata expansion remains Slice 5.

## Phase Checklist

- [x] Bootstrap, repo status, code-quality audit, and rearward review.
- [x] Accept the revised campaign direction and external amendments.
- [x] Checkpoint and lock this campaign plan/contract.
- [x] Slice 0: prove prerequisite precision and UI-to-engine bindings.
- [x] Slice 0: freeze recipes, Beauty, state, captures, reports, hashes, fallback, and timing.
- [x] Slice 0: hostile audit and focused validation.
- [x] Slice 0: checkpoint closure prepared; machine receipts and rearward review follow the committed state.
- [x] Slice 1: add RED proof for partial draft mutation and false success on rejected live Apply.
- [x] Slice 1: add Resolve/Prepare/Commit and route the public Apply path through it.
- [x] Slice 1: prove failed preparation has no authoritative draft/live/dirty/interaction side effects.
- [x] Slice 1: hostile audit and focused validation are complete; checkpoint closure is prepared and receipts/rearward review follow the committed state.
- [x] Slice 2: Canonical Recipe Contract adds semantic IDs, explicit source folds, stable parameter addresses, canonical hashes, normalized legacy aliases, and the closed adapter inventory.
- [x] Slice 3: add RED proof for selector/Prepare/report capability drift and stale snapshot rejection.
- [x] Slice 3: add one runtime-owned producer capability snapshot with stable machine IDs and generation.
- [x] Slice 3: share one applicability query across selector status, Prepare, automation, and Capture Finding.
- [x] Slice 3: separate static support, current field validity, and quality observations.
- [x] Slice 3: hostile audit and focused native/runtime validation are complete; checkpoint closure is prepared and receipts/rearward review follow the committed state.
- [x] Slice 4: add RED proof for missing committed receipt and ambiguous manual/reload provenance.
- [x] Slice 4: build the receipt completely during Prepare and commit it through non-failing state replacement.
- [x] Slice 4: report graph versus fallback authority and exact/modified/none/unknown-after-reload truthfully.
- [x] Slice 4: prove all six existing recipes remain bit-exact through public Apply, fallback, capture, and replay.
- [x] Slice 4: hostile audit and focused native/runtime validation.
- [ ] Slice 5: SDF Beauty Migration.
- [ ] Slice 6: New Recipe Qualification.
- [ ] Slice 7: Runtime Closure and replan.

## Scope Lock

In scope:

- Transactional recipe Apply authority.
- Canonical graph metadata, semantic IDs, parameter paths, hashes, and receipts.
- Runtime-owned capability snapshots and truthful applicability.
- Bit-exact current-recipe and Beauty migration.
- Independent qualification of Root Glow, Curvature Relief, and Lens Topography.
- Existing linear UI as a projection.

Out of scope:

- Visible graph editor, arbitrary graphs, new SDF ops, new fractal families, Salticid runtime, external state-tool integration, IFS, diagnostics-overlay product work, and physical mouse automation.

## Entry Gate

Before recipe behavior changes:

1. Record the exact accepted base.
2. Prove precision and binding authority for root metrics, SDF normal angle/curvature, Lens Field V2, recipe row parameters, seed, and auto-increment.
3. Freeze pre-change witnesses for every public recipe and Beauty.
4. Record renderer-only baseline timing separately from UI/capture overhead.

A shared authority, binding, transaction, type, or receipt defect stops the campaign for repair. A recipe-specific producer defect blocks only that recipe gate; independent gates continue.

## Canonical Recipe Graph Contract

### Ordered Source Fold

Maximum Source count is 8. Author order is execution order.

    accumulator_0 = evaluate(S0)
    accumulator_i = lerp(accumulator_(i-1), evaluate(Si),
                         clamp(Si.signal.blend_weight, 0, 1))

The first Source initializes the accumulator. Its blend canonicalizes to 1.0 and another explicit value is invalid recipe metadata. Blend is a Source descriptor parameter copied into the fold-edge receipt. Unknown, nonfinite, or out-of-range values fail before commit. Fold nodes and edges are explicit in prepared and committed receipts.

### Semantic Identity

Nodes use recipe-local semantic IDs such as source.normal_angle, source.lens_response, fold.lens_response, adapter.unit_cycle_as_phase_turns, shape.identity, palette.phase_wheel, and grading.phase_finish. Position IDs are not override authority. Duplicate IDs fail.

Every descriptor_parameter_id is unique and version-stable within its descriptor contract. Meaning changes require a descriptor or recipe version bump.

### Canonical Hashing

Hashing occurs after legacy alias normalization, recipe-order preservation, override sorting by node_id plus descriptor_parameter_id, typed-value normalization, and explicit expansion of every descriptor default. Display labels, descriptions, and tooltips are excluded. The hash includes recipe ID/version, functions, fold order, parameters, capability requirements, adapters, and output projection.

### Type And Adapter Locks

Canonical types include scalar.unit, scalar.signed, phase.turns, category.*, and color.rgb.

Historical phase.radians is accepted only as a mislabeled-turn alias. It normalizes to phase.turns without numeric conversion, emits a warning, and hashes canonically.

Lens Field V2 output is scalar.unit. sign_contrast remains an internal Source parameter and does not imply signed output.

The only executable campaign adapter is unit_cycle_as_phase_turns_v1: scalar.unit to phase.turns, no numeric operation, explicit edge consent required. No generic insertion is allowed. Beauty must receipt:

    fold.lens_response
    -> adapter.unit_cycle_as_phase_turns
    -> shape.identity

## Atomic Application Boundary

The public Apply path uses ResolveRecipe, PrepareColorPipelineApplication, then CommitPreparedColorPipelineApplication.

Preparation performs all validation, normalization, adapter/capability resolution, row/fold materialization, route checks, allocation checks, fingerprints, and receipts.

Commit performs only non-failing host-state replacement and dirty/interacted signaling with storage already owned by the prepared object. No allocation, validation, receipt construction, upload, observer callback, or other fallible action may occur after the first authoritative mutation. Prefer noexcept swaps/assignments.

Rejected preparation may update rejection diagnostics and selector UI state, but not authoritative draft/live state, dirty state, runtime generation, displayed frame, last successful application, or committed receipt.

## Capability Authority

One runtime-owned capability snapshot serves selector status, Prepare, reports, automation, and capture:

    producer_id
    producer_generation
    evaluator_id
    fractal_precision_tier
    color_metric_arithmetic_tier
    capabilities[]
    current_field_validity[]

Prepare rechecks the current snapshot. Static support, current validity, and visual-quality observations are separate. Unavailable recipes remain visible-disabled with stable reason codes and missing capability IDs.

## Root Glow Contract

root_log_proximity_v1 is dedicated, not an alias for overloaded root_proximity.

It uses the active renderer final iterate and selected authoritative root-pattern descriptor. It never host-re-solves coefficients.

    distance = minimum Euclidean distance from sample
               over every root in the selected authoritative root-pattern set
    value = -log2(max(proximity_scale * distance, 1e-12)) + proximity_bias

Output is scalar.signed. Missing roots fail closed. Receipts record root-pattern identity/hash, evaluator, fractal tier, metric tier, and narrowing. F64-to-F32 metric narrowing must be explicit and separately qualified.

## Provenance And Receipts

Reports separate last_recipe_application_request, current_recipe_match, and authoritative_pipeline_rows. Reload never fabricates historical recipe authorship. Exact matching uses canonical resolved fingerprints.

The committed receipt comes from the prepared object actually committed and contains recipe identity/version, metadata hash, capability snapshot, semantic nodes/folds, normalized overrides, approved adapters, final rows/params, runtime generation, authority, and fallback status.

## Fallback Lifecycle

color_pipeline.recipe_v2.force_legacy_recipe_tuple defaults false, is internal/test-only, is not persisted, is reported whenever active, is tested in CI, and never reports graph authority. It expires after all current recipes and Beauty are bit-exact under graph authority plus one subsequent checkpoint campaign without rollback use.

## Qualification Standard

Existing recipes and Beauty require bit-identical captured bytes at 256x192, identical backend/precision/tick, animation off, one warm-up, one measured frame, and exact SHA-256 equality.

New recipe gates:

- 100 percent finite pixels.
- normalized scalar p95-p05 at least 0.05.
- at least 8 occupied bins in a fixed 32-bin palette histogram.
- fewer than 90 percent of pixels in the two terminal bins.
- owning-parameter perturbation mean absolute normalized RGB change at least 0.01.
- stationary and unrelated-UI-only mean absolute RGB change exactly 0.

Use positive perturbations, or the same negative magnitude at the upper bound, and record actual values.

| Recipe | Fixed perturbations |
| --- | --- |
| Root Glow | proximity_bias +0.1; normalization scale +10 percent; glow +0.1 |
| Curvature Relief | curvature_bias +0.1; normalization scale +10 percent; heatmap cycle +0.2 |
| Lens Topography | sign_contrast +0.1; field_bias +0.1; heatmap cycle +0.2 |

Timing uses identical hardware/state/backend, five warm-ups, twenty renderer-only samples, median and p95. Reject median regression above max(10 percent, 3 times baseline MAD divided by baseline median). No performance-improvement claim is authorized.

## Slices And Independent Gates

### Slice 0 - Baseline Lock

Freeze every current recipe and Beauty: rows/params, metadata projection, state, deterministic capture/replay, hashes, authority report, normal Apply, forced legacy, and timing. Prove prerequisite bindings. No product changes.

### Slice 1 - Application Authority

Add Resolve/Prepare/Commit, route the public selector/Apply event through it, and prove rejected preparation has no authoritative side effects.

### Slice 2 - Canonical Recipe Contract

Add semantic IDs, descriptor parameter IDs, explicit folds, canonical hashing, type aliases, and the closed adapter inventory.

### Slice 3 - Capability And Applicability

Add one runtime-owned producer snapshot stored with the Color Pipeline window state and refreshed by the live runtime. The snapshot owns stable producer/evaluator/precision identity, static capability IDs, current field validity, and separately named quality observations.

One applicability query consumes that snapshot for selector status, Resolve/Prepare requirements, automation, and Capture Finding. Prepare rechecks the current snapshot and rejects stale or missing requirements with stable reason codes and missing capability IDs. Selector visibility may explain unavailable recipes, but UI inference is forbidden.

Static support answers whether the active producer can supply a required signal. Current validity reports whether the latest active evaluation actually produced finite authoritative data. Quality observations are descriptive only and cannot become hidden recipe capabilities in this slice.

No new recipe, graph execution, row behavior, pixel behavior, or replay authority is added in Slice 3.

### Slice 4 - Existing Recipe Parity (Gate A)

Move every existing recipe to prepared graph authority with bit-exact parity, fallback truth, and replay. The prepared object owns a complete application receipt before Commit. Commit only moves the already-owned window state and parameters.

Runtime/report truth distinguishes `last_recipe_application_request`, `current_recipe_match`, and authoritative rows. A successful graph Apply reports `exact`; a subsequent row edit reports `modified`; an unrelated row set reports `none`; a loaded state without persisted recipe provenance reports `unknown_after_reload`. The runtime never infers historical authorship from row similarity.

The receipt records recipe id/version, canonical metadata hash, capability snapshot identity/generation, graph/fallback authority, semantic node/fold/edge/adapter ids, committed row fingerprint, final authoritative rows, runtime generation, and fallback status. The receipt is generated from the prepared state that actually commits.

The internal fallback defaults off, is not persisted, and reports legacy authority whenever active. All six existing recipes require exact graph/fallback lane parity and exact same-backend capture/replay hashes.

No SDF Beauty metadata migration, new recipe, new visible control, or row arithmetic change belongs in Slice 4.

### Slice 5 - Beauty Migration (Gate B)

Represent Beauty's two-source fold and adapter explicitly and match frozen goldens bit-exactly.

### Slice 6 - Curated Qualification (Independent Gates C/D1/D2/D3)

Define Root Glow authority, then independently qualify Root Glow, Curvature Relief, and Lens Topography. A failed recipe remains visible-disabled and does not block the others.

### Slice 7 - Runtime Closure

Exercise the real selector/Apply command without physical mouse automation, verify committed receipts and replay truth, audit, and stop for replan.

## Proof Ledger

| Item | Status | Evidence |
| --- | --- | --- |
| Base | complete | clean 061ec748; rearward ok; campaign setup checkpoint be9516a |
| Existing graph/fallback substrate | observed | recipe_v2_graph and explicit fallback ID |
| Known authority defects | observed | phase tag, Root Proximity type/semantics, Lens V2 type, hidden Beauty expansion |
| Campaign plan/contract | complete | plan/contract validators green at be9516a |
| Slice 0 public Apply baseline | complete | test_public_recipe_apply_baseline_packet |
| Slice 0 capture/replay baseline | complete | six exact 256x192 SHA-256 capture/replay pairs and frozen state files |
| Slice 0 timing baseline | complete | five warm-ups plus twenty renderer samples per recipe |
| Slice 0 prerequisite authority | complete | Lens V2, SDF angle/curvature, root proximity f32/f64, and seed auto-increment runtime rails |
| Slice 0 fallback baseline | complete | all six recipe graph/legacy tuple projections match exactly |
| Validation and receipts | complete | focused native/runtime green; machine receipts are written against the committed checkpoint |
| Slice 1 transactional Apply | complete | Resolve/Prepare/Commit; non-failing commit assertions; native reject/success/manual-recovery rails; published public Apply proof; commit 6ddef4a6; receipts and rearward review are ok. |
| Slice 2 canonical recipe contract | complete | commit 1ae2f14; 43 materializer tests; 3364 Color Pipeline core assertions; published staged-contract and public Apply runtime proofs; receipts and rearward review are ok. |
| Slice 3 capability/applicability authority | complete | runtime-owned snapshot and shared query; 467 window assertions; report and finding-sidecar native rails; published public selector/Apply proof; current SDF validity proof; final checkpoint/receipts follow. |
| Slice 4 existing recipe parity | complete pending checkpoint | prepared/committed graph and fallback receipts; exact/modified/none/unknown-after-reload provenance; 3364 core and 477 window assertions; report/capture/state rails; six frozen live FNV and headless SHA-256 recipe goldens; published runtime and JUnit proof green. |

## Hostile Audit

- Status: complete
- Scope: Slice 4 prepared/committed receipt ownership, graph/fallback truth, rejected-Apply preservation, exact/modified/none/unknown-after-reload provenance, frozen-golden pixel parity, report/capture consistency, and state replay.
- Assume dual authority, false receipts, non-atomic Apply, stale capability snapshots, type reinterpretation, unstable parameter addresses, fabricated provenance, or weak qualification remains.

## Audit Passes

- [x] Pass 1 - Local hostile review exposed hidden Apply bypass, implicit folds, type drift, and false provenance risk.
- [x] Pass 2 - External hostile review required atomic Prepare/Commit, capability ownership, semantic IDs, frozen goldens, and independent gates.
- [x] Pass 3 - Clean re-read confirmed the repaired state locks scoped defect blocking, normative hashes, Beauty adapter receipts, Root Glow tiers, fixed perturbations, and non-failing commit.
- [x] Slice 0 pass 1 - the RED witness proved all-recipe public Apply/state/timing evidence was absent.
- [x] Slice 0 pass 2 - the first implementation exposed stale Apply-frame and re-Apply timing reports; the witness now settles with Render Once and measures fresh headless renders.
- [x] Slice 0 pass 3 - fallback review found only the default recipe was compared; all six current recipe projections now have exact graph/fallback parity.
- [x] Slice 1 pass 1 - the old public Apply path mutated the draft before live validation, returned success after rejection, marked interaction, and was hidden inside the ImGui-only test exclusion.
- [x] Slice 1 pass 2 - successful staging lacked direct non-mutation proof and stale output errors were not cleared; native tests now freeze Resolve/Prepare/Commit behavior.
- [x] Slice 1 pass 3 - runtime proof exposed one-frame rejection messages and manual Apply recovery left stale recipe errors; persistent rejection state now clears at the shared successful application boundary.
- [x] Slice 2 pass 1 - the first green native rail still concealed semantic metadata defects: phase.turns retained radians units/period, Lens Field V2 remained typed signed-distance, and the legacy alias was declared but not normalized.
- [x] Slice 2 pass 2 - projection hardening found malformed source-fold values lacked direct negative C++ proof; noncanonical first-source blend and output-node mismatch now fail closed in the native rail.
- [x] Slice 2 pass 3 - a clean re-read of canonical types, adapters, generated metadata, parser validation, projection, and published runtime staging found no additional Slice 2 defect.
- [x] Slice 3 pass 1 - RED review proved the selector/report surfaces had no runtime-owned capability snapshot and Prepare had no stale-snapshot or stable missing-capability rejection.
- [x] Slice 3 pass 2 - implementation review found the report helper omitted its recipe parser link input, the report fixture was only a partial Source lane, and the old runtime assertion expected vague `not allowed` text; each now has a focused repair and regression.
- [x] Slice 3 pass 3 - clean re-read and published public-path proof confirmed selector status, Prepare, automation, and Capture Finding use one snapshot/query; field validity and empty quality observations do not grant static support.
- [x] Slice 4 pass 1 - RED review proved committed application receipt and distinct manual/reload provenance were absent; early report/finding fixtures also falsely expected `exact` from invalid or stale live snapshots.
- [x] Slice 4 pass 2 - runtime review found the first proof only established current self-consistency, not parity with frozen pre-change pixels; it now requires all six Slice 0 live FNV and headless SHA-256 goldens. Follow-up review added rejection-after-success proof so an unavailable recipe cannot erase the prior receipt or generation.
- [x] Slice 4 pass 3 - clean re-read plus focused native, exact published-runtime, frozen-golden, Capture Finding, and reload proof found no additional Slice 4 defect.
- Later implementation audits remain pending for canonical graph authority, dual authority, capability snapshots, independent recipe gates, and fallback lifecycle.

## Audit Findings
- [x] Current phase-like metadata labels turn values as phase.radians.
- [x] Current root_proximity is typed scalar.unit despite unbounded logarithmic and producer-dependent semantics.
- [x] Lens Field V2 is typed as raw signed distance despite bounded response output.
- [x] Beauty metadata claims one Source while runtime special handling adds another.
- [x] Workflow lacks an approved branch/merge wrapper; this campaign is explicitly stacked.
- [x] Planning closure finding: one pending audit state for future Slice 0 caused rearward review to reject the completed planning checkpoint.
- [x] Public Apply command acknowledgement can precede the rendered recipe frame; baseline proof now requires one explicit full-quality settle render.
- [x] Reapplying an unchanged recipe does not force a render, so repeated Apply reports cannot be used as timing samples; baseline timing uses fresh headless renders.
- [x] Recipe option IDs are public command IDs but not always-visible rectangles; the witness waits for the selector then drives the scoped selection command.
- [x] Forced-legacy parity covered only default_smooth_escape; the native rail now compares all six current graph/legacy recipe projections.
- [x] The first packet revision paired a settled frame with the earlier Apply receipt; receipts and lane rows now come from the same settled report.
- [x] The broad native helper sweep exceeded 15 minutes and left an orphaned NVCC process; it was terminated and no broad-sweep success is claimed. Required focused core/window rails passed.
- [x] Recipe Apply committed draft changes before live-family validation and returned true after rejection; Resolve/Prepare now operate on copies and Commit is statically constrained to non-throwing assignments.
- [x] The recipe application helper lived inside the ImGui-only block, preventing native public-path regression coverage; only rendering controls remain ImGui-gated.
- [x] Recipe rejection diagnostics lasted one UI frame and survived later manual recovery; a persistent UI-only rejection field now remains reportable and clears on any successful application.
- [x] The first Slice 1 publish wrapper was cut off by an incorrectly short outer terminal timeout while the compiler child continued; the orphan completed, and all claimed publish evidence comes from later fully receipted runs.
- [x] The final normal runtime executable was locked by an existing process; the checked-in publish fallback staged fractal_ui_dev.exe and the active-runtime runtime lane passed against that fresh executable.
- [x] Slice 3 contract bootstrap deadlocked because the validator required the newly allowed capability header to exist before relocking; one empty header was created directly to break the cycle, then every product mutation returned to the guarded wrapper.
- [x] The first Slice 3 report build exposed a missing `json_min.cpp` link input now that the report calls the real recipe resolver; focused and full helper topology now declare the dependency.
- [x] The first report fixture contained only a Source lane, so recipe resolution failed before capability evaluation; the fixture now starts from the complete live pipeline and overlays its diagnostic Source row.
- [x] Existing runtime rejection proof asserted vague `not allowed` text; it now requires `missing_required_capability` plus the exact missing source capability while preserving frame and row state.
- [x] phase.turns initially retained units=radians and period=2*pi, while runtime phase signals are normalized cycles; metadata now declares units=turns and period=1, with no numeric state change.
- [x] The phase.radians compatibility alias was listed but authoring references were not normalized; typed signals, ports, adapter endpoints, and recipe-adapter endpoints now normalize to phase.turns during materialization.
- [x] Lens Field V2 remained typed scalar.sdf_signed_distance despite its bounded unit response; its descriptor and canonical output port now use scalar.unit while sign_contrast remains an internal source parameter.
- [x] The first focused native invocation incorrectly used an environment variable even though build_tests_vsdevcmd.cmd dispatches focused tests through positional arguments; that command started the broad suite and hit the outer timeout. All claimed native evidence comes from the corrected positional focused command.
- [x] The first Slice 2 runtime publish used a 240-second outer timeout while CUDA compilation remained active. No failure is claimed; the clean retry used the checked-in command with a 900-second ceiling and completed successfully in 724.6 seconds.
- [x] Slice 4 report and finding fixtures populated receipt rows without a valid matching live snapshot, so truthful provenance reported `modified`; fixtures now construct a genuinely committed state before asserting `exact`.
- [x] The first Slice 4 state-IO assertion was placed before the round-trip state existed and failed compilation; it now runs only after emitted state reload.
- [x] The first Slice 4 runtime proof compared current capture only to its own replay and could miss recipe pixel drift; it now requires all six frozen Slice 0 live and headless hashes.
- [x] Receipt preservation after a successful Apply followed by an unavailable recipe had no direct proof; the native rail now freezes receipt identity, generation, exact match, dirty state, and interaction state across rejection.

## Planned Validation Targets

- contract validation and phased-plan sync
- code-quality baseline
- focused Color Pipeline core/window native rails
- runtime publish
- published no-mouse public Apply and replay proof
- baseline witness validation
- hostile-audit validation and diff check

## Stop Point

Slice 4 implementation and hostile validation are complete pending checkpoint, machine receipts, rearward review, and push. Slice 5 SDF Beauty Migration is the next checked-in preplanned slice; preplanned sliced work is not exhausted.
