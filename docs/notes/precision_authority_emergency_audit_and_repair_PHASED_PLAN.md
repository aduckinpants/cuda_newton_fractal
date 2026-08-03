# Precision Authority Emergency Audit And Repair Phased Plan

## Current Phase

Phase 2A complete - five source-proven float32-only dispatch owners covering six selectors now have truthful standard-tier arithmetic, canonical execution evidence, owner-level inventory attribution, and published-runtime proof. Phase 3 is the next approved execution boundary after the Phase 2A checkpoint.

## Phase Checklist

- [x] Phase 0 - lock this engine-native plan and workflow-only contract, validate it, checkpoint it, and continue without product mutation.
- [x] Phase 1 - produce a deterministic system-wide precision matrix covering authoring, state I/O, declared runtime tiers, and actual recurrence arithmetic; stop at the evidence review gate.
- [x] Phase 2 - repair false runtime-tier declarations or dispatch only where the matrix and focused RED witnesses prove them.
  - [x] Phase 2A - repair ExplainO Y, ExplainO Julia, ExplainO Lambda, Multicorn, and the shared McMullen/Collatz owner; preserve fast arithmetic; prove canonical execution and published-runtime truth.
- [ ] Phase 3 - repair numeric authoring and readback identity across the general schema route, Color Pipeline UI-Salt route, and `state.json` route.
- [ ] Phase 4 - make only evidence-driven storage or arithmetic precision promotions, preserving intentionally truthful float32 domains.
- [ ] Phase 5 - run full qualification, publish from an exact clean checkpoint, reconcile the provisional fractal `ui.salt` reality-contact note, and stop at the engine merge-approval boundary.

## Explicit User Asks

- [open] Audit the engine broadly enough to determine how severe float32/float64 authoring and runtime truth problems are across the system.
- [open] Repair proven precision-authority defects without blindly converting every float to double.
- [open] Cover all three authoring routes: general schema binding, Color Pipeline UI-Salt contract materialization, and `state.json` load/save/replay.
- [open] Distinguish what the UI claims, what storage preserves, what runtime tier resolution reports, and what arithmetic actually executes.
- [open] Preserve the current float32 route where it is truthful and measure intentional float64 changes rather than treating them as regressions.
- [open] Keep nonzero ExplainO warp out of this campaign. Fixtures use zero warp; no warp redesign or materialization work is authorized.
- [open] Keep the future `ui.salt` direction visible as a nonblocking design constraint, using the provisional pack-idea reality-contact examples as design evidence only.
- [open] Do not redesign the whole UI, implement the Salticid language, add WPF/designer infrastructure, or modify the state-tool repository in this campaign.
- [open] Do not merge the CUDA engine branch without separate user approval.

## Starting Authority

- Repository: `C:\code\cuda_newton_fractal_clone`.
- Branch: `codex/finding-enrichment-engine-authority-v1`.
- Exact starting commit: `0b928615d851d7c1ab00a7e121a9899adcff60bc`.
- Remote parity at start: local branch equals `origin/codex/finding-enrichment-engine-authority-v1`.
- Worktree and stash at start: clean worktree; no stashes reported.
- Rearward review: `artifacts/hooks/viewer_host_rearward_review/0b928615d851d7c1ab00a7e121a9899adcff60bc.json` reports `ok`.
- Published executable inherited from E0: `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`, SHA-256 `240987082532e8e7fbd9676f06b1314ed1bbdab3d8d6007bdc99fba249e42c83`.
- State-tool repository is frozen during engine work.
- Pack-idea reality-contact documentation is already clean on its own `main` checkpoint and is not engine runtime authority.

## Triggering Evidence

The Rational Escape captures around `explaino_seed = 0.217977` prove a concrete authoring-identity defect:

- nearby edits produce distinct runtime states and frames;
- the stored model combines a double seed base with float-backed drift and float-backed derived root/poly fields;
- the current combined seed UI displays only six decimal places;
- re-entering the displayed value can therefore change the authoritative state;
- `state.json` already emits enough digits to preserve the stored float value, so the first proven defect is not generic JSON truncation.

The same inspection also found a wider risk:

- `SampleTier::standard` is described as float64;
- the tier resolver advertises standard support broadly;
- the canonical sampler contains selector branches with heterogeneous `useFP64` handling and float-backed parameters;
- schema controls, Color Pipeline values, state loading, and runtime recurrence arithmetic do not currently expose one reviewed end-to-end precision classification.

This evidence authorizes an audit. It does not authorize a mass type conversion.

## Precision Authority Model

For every authorable numeric value and every selectable fractal runtime, the campaign distinguishes these layers:

```text
authored text or widget value
-> schema/contract-declared numeric kind and display/edit behavior
-> bound model member and storage width
-> state JSON parse/serialize behavior
-> requested sample tier
-> resolved numeric backend and iteration strategy
-> arithmetic actually executed by the selected recurrence/color path
-> engine-emitted state, report, frame, and replay evidence
```

No layer may stand in for another. In particular:

- a `slider_double` does not prove double-backed model storage;
- a double-backed member does not prove float64 recurrence arithmetic;
- a `standard` or `float64` receipt does not prove every selected branch consumes float64 operands;
- precise JSON spelling does not repair a lossy UI readback loop;
- a visually different frame does not by itself identify which layer is responsible.

## Authoring Routes In Scope

### General schema binding

Trace controls from `ui/fractal_binding_surface_v1.ui_schema.json` through schema parsing, control rendering, `BindFloat` or `BindDouble`, model storage, state I/O, parameter-surface export, and engine readback. Record display format and edit identity separately from storage type.

### Color Pipeline UI-Salt contract

Trace numeric parameter descriptors from the compiled/materialized UI-Salt contract through row draft value carriers, validation, serialization, loaded-draft application, and runtime consumers. The compiled contract remains authority; do not add a second precision table in C++ or Python.

### `state.json` loading and replay

Trace JSON number parsing, casts, range checks, model assignment, derived-field recomputation, deterministic serialization, engine-emitted state, and action-free replay. Existing `state.json` is authoritative replay state; no schema replacement is authorized.

## Phase 0 - Plan And Contract Lock

Scope is documentation and workflow authority only. Required closure:

1. checked-in plan and machine-readable contract;
2. exact repository and runtime starting identity;
3. planning contract validation and plan synchronization;
4. focused workflow-tool tests;
5. hostile review, clean checkpoint, receipts, rearward review, and push.

No product or test-source mutation belongs in Phase 0.

## Phase 1 - Deterministic Precision Matrix

Create one deterministic inventory generator and focused tests. It must derive facts from current source/schema/contracts rather than maintaining a hand-authored replacement registry.

Required matrix dimensions:

- selector or capability owner;
- serialized path/control ID/function parameter path;
- applicability and authorability status;
- UI control type and displayed/editable precision where statically knowable;
- binding route and model storage type;
- state-load conversion and state-save behavior;
- parameter-surface or UI-Salt-declared numeric kind/range;
- requested tier support flags;
- resolved backend claims;
- canonical sampler branch and whether float64 is actually consumed;
- derived-field narrowing or widening seams;
- current tests/witnesses;
- classification and confidence.

Required classifications:

```text
TRUTHFUL_FLOAT32
TRUTHFUL_FLOAT64
INTENTIONAL_MIXED_PRECISION
AUTHORING_IDENTITY_LOSS
STATE_IO_NARROWING
FALSE_RUNTIME_TIER_CLAIM
NEEDS_RUNTIME_WITNESS
UNSUPPORTED_OR_NONAUTHORABLE
```

The generator may emit `UNKNOWN` while evidence is incomplete, but it must fail closed on malformed source authorities and must not silently infer semantic equivalence from names.

Phase 1 stops at a review gate with:

- machine-readable matrix;
- concise reviewed Markdown report;
- prioritized candidate repair slices;
- exact unknowns and required witnesses;
- no product repair mixed into the inventory checkpoint.

## Phase 2 - Runtime Tier Truth

For each matrix row classified as a false tier claim or high-risk unknown:

1. add a focused RED witness proving the requested/resolved/actual mismatch;
2. choose the smallest truthful repair: implement the missing float64 route, narrow advertised support, or report an explicit fallback/unsupported result;
3. preserve float32 operation order and outputs unless a separately reviewed correction is intentional;
4. prove the selected runtime report matches actual arithmetic;
5. add representative published-runtime evidence.

Do not declare all selectors float64-capable because their coordinates enter through a double camera. Do not downgrade a truthful float64 path merely because some model parameters are intentionally float-backed.

### Phase 2A - Bounded False-Owner Repair

The Phase 1 count of 25 selectors without selector-named `useFP64` tokens is not a workload count. Shared predicates and the generic fallback already own most of those selectors. Source review narrows this slice to five dispatch owners covering six selectors:

```text
explaino_y
explaino_julia
explaino_lambda
multicorn
UsesSpecializedEscapeTimeFormula -> mcmullen, collatz
```

Required behavior:

- add a canonical `FractalSampleEvidence` field that records whether float64 iteration arithmetic actually executed, without changing `FractalSampleResult`;
- instrument real float64 iteration branches, not float64-only postprocessing such as nearest-root snapping;
- add RED native witnesses proving every target resolves to float64 and executes float64 iteration arithmetic under `standard`, while `fast` remains on the existing float32 route;
- implement the missing double routes with existing typed math helpers and preserve float32 operation order;
- update the inventory to attribute shared predicate/fallback owners and stop presenting selector count as repair count;
- publish and prove representative repaired selectors through the exact runtime.

No host-probe centralization, numeric authoring repair, state I/O work, Color Pipeline work, storage promotion, nonzero-warp work, state-tool mutation, or engine merge belongs in Phase 2A.

## Phase 3 - Numeric Authoring And Readback Identity

Repair only matrix-proven authoring defects. Required behavior:

- a displayed editable value must round-trip without changing state when the user makes no semantic change;
- editing must not promise precision the bound storage cannot retain;
- serialized values must retain the model value exactly enough for round trip;
- runtime normalization must be visible in engine-emitted state or receipts;
- general schema, Color Pipeline, and state-load routes must share model authority rather than separate coercion policy.

The Rational Escape combined seed is the first required witness. Wider fixes must be driven by mechanically identified peers and focused tests, not by a global decimal-format change made on intuition.

## Phase 4 - Evidence-Driven Precision Promotions

A field or arithmetic path may move from float32 to float64 only when all are established:

- the user-visible or scientific defect is reproducible;
- storage/ABI/state compatibility impact is understood;
- the runtime path benefits materially;
- float32 continuity expectations are explicit;
- representative performance and frame deltas are measured;
- state load/save and published-runtime replay remain truthful.

Broad struct conversion, ABI churn, and speculative deep-zoom rewrites are forbidden.

## Phase 5 - Qualification, Publication, And Reality-Contact Reconciliation

Run every current repository-mandatory focused, native, runtime, code-quality, hostile-audit, checkpoint, receipt, and rearward-review rail required by the actual mutations. Publish only from an exact clean product checkpoint and record executable hash plus representative runtime witnesses.

Then revisit the provisional fractal `ui.salt` reality-contact document in `C:\code\hat-rack-v2\salticid-pack-idea-staging` only to record what the engine audit actually proved. Do not keep it continuously synchronized during the engine campaign and do not turn it into runtime authority.

Stop at a clean pushed engine PR boundary. Merge still requires separate user approval.

## Future `ui.salt` Constraint

The future language direction is allowed to influence seam quality but not campaign scope:

- preserve stable semantic control identity;
- keep numeric kind, range, applicability, storage binding, and runtime evidence separable;
- prefer machine-derived projections over duplicated handwritten UI definitions;
- do not make current engine code depend on the provisional syntax examples;
- do not generate or parse new `.salt` files in this campaign.

## Mutation Boundaries

Allowed under phase-specific successor contracts:

- deterministic inventory tooling and tests;
- focused schema/binding/display repairs;
- focused state I/O repairs;
- focused tier resolver or canonical sampler repairs;
- narrow storage precision promotions proven by evidence;
- docs, fixtures, receipts, and publication proof.

Forbidden without a new user-approved plan:

- wholesale UI rewrite;
- wholesale float-to-double conversion;
- nonzero warp work;
- Color Pipeline redesign;
- state schema replacement;
- IFS, new fractal families, or perturbation expansion;
- state-tool product mutation;
- Salticid compiler/language implementation;
- engine merge.

## Proof Ledger

- Bootstrap: clean branch `codex/finding-enrichment-engine-authority-v1` at `0b928615d851d7c1ab00a7e121a9899adcff60bc`, remote parity `0/0`.
- Rearward review: `ok` for the starting commit.
- Existing Rational Escape E0 runtime: exact-checkpoint executable SHA-256 `240987082532e8e7fbd9676f06b1314ed1bbdab3d8d6007bdc99fba249e42c83`.
- Trigger captures: `2026-08-02/200930_615`, `202028_667`, and `202121_962` under the published runtime manual-capture tree.
- Source orientation: general schema exposes `BindFloat` and `BindDouble`; the UI schema contains four `slider_double` controls; `SampleTier::standard` maps to float64 direct; canonical sampler branches differ in explicit `useFP64` handling.
- Phase 0 contract validation: `precision_authority_emergency_audit_p0_contract.json` reports `ok: true`.
- Phase 0 plan sync: passed for both the predecessor handoff note and this successor plan.
- Phase 0 focused workflow tests: 30 passed, with the same two explicitly unrelated baseline tests deselected as in the preceding planning checkpoint.
- Phase 0 product mutation: none.
- Phase 0 checkpoint: `662f578bc54be31182ef00655f3470d52bf2648e`, receipted, rearward-reviewed `ok`, and pushed.
- Phase 1 contract: `docs/contracts/precision_authority_inventory_p1.contract.json`.
- Phase 1 deterministic generator: `tools/precision_authority_inventory.py` with eight focused tests in `tests/test_precision_authority_inventory.py`.
- Phase 1 machine matrix: `artifacts/precision_authority/phase1/matrix.json`; generated Markdown projection: `artifacts/precision_authority/phase1/matrix.md`.
- Phase 1 reviewed conclusions: `docs/notes/precision_authority_inventory_p1_REPORT.md`.
- Phase 1 source inventory: 107 numeric controls, including 90 authoring-identity risks, 120 compiled Color Pipeline parameters, 149 explicit state-load float casts requiring owner classification, and 51 selectors under a universally advertised `standard -> float64 direct` policy.
- Phase 1 runtime-tier result: 25 selectors lack selector-named direct `useFP64` branch evidence, but shared predicates/delegation/fallback make them witness targets rather than proven false claims.
- Phase 1 full Python suite on the dirty candidate: 581 passed, 19 failed, 23 skipped. The exact clean Phase 0 checkpoint `662f578bc54be31182ef00655f3470d52bf2648e` produced 572 passed, 20 failed, 23 skipped. Failure-set comparison found zero new failures; the only baseline-only failure was `test_runtime_sweep_changes_live_view_and_space_pauses_it`, which passed in the candidate run. The suite is not represented as green; raw logs are `artifacts/logs/precision_authority_p1_python_suite.log` and `artifacts/logs/precision_authority_p1_baseline_python_suite.log`.
- Phase 1 product mutation: none.
- Phase 2A contract: `docs/contracts/precision_authority_runtime_tier_p2a.contract.json`; slice checkpoint token `ck:de3d3067`.
- Phase 2A RED: the focused CUDA witness failed because `FractalSampleEvidence` did not expose executed-arithmetic identity.
- Phase 2A repair: ExplainO Y, ExplainO Julia, ExplainO Lambda, Multicorn, and the shared McMullen/Collatz owner retain their float32 fast branches and execute typed float64 recurrence arithmetic under standard.
- Phase 2A canonical native witness: `test_fractal_sample_kernel` reports 1,076 passed and zero failed; all six targets report false for fast and true for standard `used_float64_iteration_arithmetic`.
- Phase 2A inventory: eight focused tests pass; 51 selectors are attributed to 31 top-level dispatch owners, and no owner lacks the canonical static execution marker. Static attribution remains explicitly subordinate to the native execution witness.
- Phase 2A full native rail: all helper tests passed after the execution-evidence ABI test was updated to lock the intentional three-field layout.
- Phase 2A published runtime: `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`, SHA-256 `30d143a5baf81b7eb79ea46874eefe687ac08cf8d9e2e60dc1f4c3ae1ea288e9`.
- Phase 2A published-runtime matrix: six selectors passed fast/standard resolution and deterministic action-free standard replay in `tests/test_fractal_runtime_precision_tier_truth.py`.
- Phase 2A code-quality audit: 93/100 with baseline check passed; no new critical, error, or warning class was introduced.

## Hostile Audit

- Status: complete

Audit questions:

- Does the plan confuse storage width, display precision, JSON precision, requested tier, resolved tier, and executed arithmetic?
- Does it assume every mixed float/double path is defective?
- Could the inventory become a second hand-maintained precision registry?
- Does a proposed fix preserve intentional float32 behavior and ABI/state compatibility?
- Are the three authoring routes all traced to the same runtime owners?
- Does future `ui.salt` remain nonblocking design evidence rather than a product dependency?
- Is nonzero warp still excluded simply?
- Are state-tool mutation and engine merge still unauthorized?

## Audit Passes

- [x] Pass 1 - reviewed the Phase 0 plan and contract against live repository authority and found an invalid forward reference to a not-yet-created successor contract.
- [x] Pass 2 - removed the invalid forward reference, re-read the repaired state after contract validation and plan sync, and found no additional real defect.
- [x] Pass 3 - final clean re-read confirmed the repaired state separates UI display, storage, JSON, tier resolution, and executed arithmetic and does not authorize mass precision conversion, warp work, state-tool mutation, or engine merge.
- [x] Pass 4 - distrust-first review found that the first generator flattened specialized high-precision camera routes into ordinary float bindings and misclassified the composite resolution long-edge route as unresolved.
- [x] Pass 5 - added RED coverage, then repaired the generator to expose binding storage, editor carrier, authoritative storage, special edit route, source confidence, and unresolved cross-layer joins; the repaired inventory now recognizes camera, combined-seed, and aspect-preserving resolution ownership.
- [x] Pass 6 - final re-read confirmed that static sampler-token absence is reported only as a witness target, state-load casts and Color Pipeline carriers remain `NEEDS_RUNTIME_WITNESS`, no product repair is mixed into Phase 1, and Phase 2 remains blocked at the user review gate.
- [x] Pass 7 - distrust-first source review found that ExplainO Y initially tested convergence through a float-projected residual and that perturbation execution was not marked because it does not sit under `if (useFP64)`; both defects were repaired before the focused GREEN witness.
- [x] Pass 8 - the first full native rail found the intentional evidence-field addition conflicted with a stale exact two-field layout assertion; the test now locks the legacy prefix, witness offset, and padded three-field size, and the complete native rail passes.
- [x] Pass 9 - final marker review found mechanically inserted assignments inside nearest-root postprocessing blocks. Those assignments were removed so the witness records actual iteration arithmetic only; the generic direct-iteration fallback marker was retained and correctly indented.
- [x] Pass 10 - post-repair re-read plus focused, full-native, publication, and six-selector runtime reruns found no additional real defect.

## Audit Findings

- [x] The first Phase 0 contract listed `docs/contracts/precision_authority_inventory_p1.contract.json` before that successor file existed. Contract validation correctly blocked slice start. The repaired P0 contract names only existing paths; the P1 contract will be created through the repository's checked-in successor-lock procedure after this checkpoint.
- [x] The inventory could have become a second semantic registry if it treated static source parsing as runtime authority. The plan now requires source-derived facts, explicit confidence, `NEEDS_RUNTIME_WITNESS` for incomplete evidence, and runtime proof before behavioral repair.
- [x] The first generator treated schema `BindFloat` as the whole camera story. Current source instead reads and writes `center_hp_x`, `center_hp_y`, and `log2_zoom` through specialized routes, including a float editor for the double center authorities. Tests now prevent that authority compression.
- [x] The first generator called `fractal.render.resolution.long_edge` unresolved because it is not a direct `BindInt` pointer. Current source intentionally projects that integer through `GetIntValue` / `SetIntValue` into an aspect-preserving `int2` resolution. Tests now preserve that composite route.
- [x] A top-level selector branch containing `useFP64` is useful static evidence but not executed-arithmetic proof. Shared predicate branches and the generic fallback make token absence insufficient for `FALSE_RUNTIME_TIER_CLAIM`; the report keeps all such rows at the runtime-witness boundary.
- [x] The repository-wide Python suite is red on the exact clean Phase 0 checkpoint. The Phase 1 candidate introduced no new failure IDs, so closure records baseline equivalence rather than making a false all-green claim. Product repair for the inherited failures is outside this workflow-only slice.
- [x] Selector counts were misleading as repair counts. The owner-aware inventory now attributes helper predicates and the generic fallback mechanically from source, while retaining the native CUDA witness as behavioral authority.
- [x] The first Phase 2A execution witness marked float64 postprocessing as well as recurrence execution. Hostile review removed the postprocess-only assignments and reran every affected proof rail.
- [x] The widened evidence seam intentionally changes only `FractalSampleEvidence`; `FractalSampleResult`, renderer behavior, state contracts, and the fast float32 route remain unchanged.

## Notes

- The prior finding-enrichment E1 callable-centralization phase remains deferred. This emergency campaign does not silently absorb it.
- Phase 2A closes at its evidence checkpoint once validation, commit, receipt, rearward review, and push finish.
- Phase 3 numeric authoring and readback identity is the next approved execution boundary; no engine merge is authorized.
