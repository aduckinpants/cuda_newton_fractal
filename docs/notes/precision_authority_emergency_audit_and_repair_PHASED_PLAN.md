# Precision Authority Emergency Audit And Repair Phased Plan

## Current Phase

Phase 4 - review remaining float32 storage and arithmetic owners against the evidence-driven promotion gate after the closed Phase 3D authoring checkpoint.

## Phase Checklist

- [x] Phase 0 - lock this engine-native plan and workflow-only contract, validate it, checkpoint it, and continue without product mutation.
- [x] Phase 1 - produce a deterministic system-wide precision matrix covering authoring, state I/O, declared runtime tiers, and actual recurrence arithmetic; stop at the evidence review gate.
- [x] Phase 2 - repair false runtime-tier declarations or dispatch only where the matrix and focused RED witnesses prove them.
  - [x] Phase 2A - repair ExplainO Y, ExplainO Julia, ExplainO Lambda, Multicorn, and the shared McMullen/Collatz owner; preserve fast arithmetic; prove canonical execution and published-runtime truth.
- [x] Phase 3 - repair numeric authoring and readback identity across the general schema route, Color Pipeline UI-Salt route, and `state.json` route.
  - [x] Phase 3A - replace the shared six-decimal general-double editor format with one source-owned binary64 round-trip format and prove the Rational Escape combined seed through the published viewer.
  - [x] Phase 3B - classify state-load numeric casts by destination owner and repair only proven state-I/O identity loss.
  - [x] Phase 3C - classify Color Pipeline contract carriers against runtime consumers and repair only proven authoring/readback loss.
  - [x] Phase 3D - repair the remaining shared general-schema binary32 typed-input and specialized camera typed-edit routes, preserving readable slider labels and engine-owned camera math.
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

### Phase 3A - General-Schema Double Authoring Identity

The Phase 1 inventory reduces this sub-slice to one shared editor owner serving four `slider_double` controls. All currently route through `RenderDoubleControl`, which hardcodes `%.6f`; the two combined-seed controls delegate to `RenderExplainoSeedDoubleControl` using that same format.

Required behavior:

- expose one C++ source-owned binary64 round-trip input/display format (`%.17g`) for the general-schema double route;
- use it for both direct-double and combined ExplainO seed widgets without changing slider ranges, steps, schema JSON, model storage, or derived seed decomposition;
- prove the captured Rational Escape witness `0.21797676384449005` survives format/parse and the combined edit route exactly;
- make the precision inventory derive the active double format from `schema_binding.cpp` rather than maintaining a Python-owned duplicate literal;
- publish and prove the exact control edit through the viewer automation path and engine-emitted state/readback evidence.

This sub-slice does not authorize camera formatting, Color Pipeline carriers, state JSON casts, model-width changes, or a general UI precision redesign. Those remain Phase 3 successor work after this checkpoint.

### Phase 3B - State-I/O Owner Classification And Exact Replay

The Phase 1 count of 149 explicit `static_cast<float>` expressions is not a repair count. Current source groups them under eight loader owners: the main `LoadDiagnosticsStateJson` route plus seven typed helpers for Lens, fixed float arrays, ExplainO roots, and Color Pipeline stack runtime parameters.

Required behavior:

- mechanically join every float conversion to its declared destination storage owner and fail the inventory on an unclassified or ambiguous destination;
- keep deliberate JSON-number-to-float conversions when the destination model member is float-backed;
- enumerate serialized binary64 owners separately and prove their load/save/reload identity with nontrivial sentinels;
- prove that a non-binary32 JSON spelling assigned to a float owner is normalized to the actual float value and that the emitted state reports that value exactly;
- add a published-runtime action-free witness covering exact binary64 state identity and visible float normalization;
- repair product code only if the owner join or runtime witness proves an actual destination-width mismatch.

This sub-slice does not authorize model-width promotion, state schema changes, camera UI changes, Color Pipeline draft-carrier work, formula changes, nonzero-warp work, state-tool mutation, or engine merge.

### Phase 3C - Color Pipeline Carrier And Consumer Truth

The Phase 1 count of 120 compiled Color Pipeline parameters is not a repair count. The exact compiled UI-Salt contract declares 94 float parameters, 8 integer parameters, 18 enum parameters, and zero double parameters. Numeric draft values share one double carrier before lowering into float- and int-backed runtime owners.

Required behavior:

- mechanically classify every compiled parameter by declared kind, shared draft carrier, editor branch, and runtime consumer owner without adding a handwritten parameter registry;
- retain the double draft carrier so exact JSON input remains inspectable before materialization;
- preserve the intentional float32 runtime owners and expose their normalization in the synchronized engine-emitted draft;
- replace the shared fixed five- and three-decimal float editor formats with one source-owned nine-significant-digit binary32 round-trip format;
- remove decimal rounding from live float import so draft readback preserves the exact binary32 value promoted to double;
- compare draft and runtime float state by exact stored identity rather than a fixed decimal tolerance so adjacent binary32 edits cannot disappear;
- prove a non-binary32 draft spelling normalizes once at the float runtime owner, appears normalized in the engine-emitted draft, and replays without further drift;
- preserve historical direct UI apply for tuples without live-snapshot reconstruction, but require explicit loaded-draft application to fail transactionally when authoritative normalized readback is unavailable;
- repair no descriptor, runtime storage width, function behavior, or contract taxonomy unless a focused witness proves a distinct mismatch.

This sub-slice does not authorize float-to-double promotion, Color Pipeline topology or semantic changes, UI-Salt contract taxonomy changes, general-schema or camera work, formula changes, nonzero-warp work, state-tool mutation, or engine merge.
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
- Phase 3A contract: `docs/contracts/precision_authority_general_double_authoring_p3a.contract.json`; checkpoint token `ck:cc990b2c`.
- Phase 3A RED: focused native compile failed because the canonical round-trip format owner did not exist.
- Phase 3A focused proof: `test_schema_binding` passes exact format/parse, direct-double automation, and captured combined-seed model-readback identity; eight inventory tests pass.
- Phase 3A inventory: 107 controls now classify as 86 authoring identity losses, 15 intentional mixed-precision routes, four truthful float32 routes, and two truthful float64 routes. The two combined seed controls are intentionally mixed because their fractional component remains float-backed.
- Phase 3A full native rail: all helper tests passed.
- Phase 3A published runtime: `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`, SHA-256 `0cc8b94ee13cffd5166fed5d464324cf2589725e0241ad317ac4ab9109cd2bb3`.
- Phase 3A runtime proof: the published Rational Escape control consumed `0.21797676384449005` and reported the exact same engine-model combined seed; one runtime test passed with no skips.
- Phase 3B contract: `docs/contracts/precision_authority_state_io_owner_classification_p3b.contract.json`; exact clean start `49295852034f5249728cd19827b196c8bb90f264`.
- Phase 3B starting hypothesis: 149 float conversions are grouped under eight loader owners and appear to target float-backed storage; six serialized double members bypass those casts. Deterministic source joins confirm five are exact binary64 owners while `explaino_seed` is canonicalized with float-backed drift by `ExplainoSeedNormalize`.
- Phase 3B focused proof: nine inventory tests and `test_diagnostics_state_io` pass; the native RED first exposed seed canonicalization, then the repaired witness proved canonical binary64 identity and explicit float normalization.
- Phase 3B full native rail: all helper tests passed.
- Phase 3B published runtime: `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`, SHA-256 `6cdc1b24e71983489b07a9aded7c993a21b87db75c9f26eece3bf49c2a82076c`.
- Phase 3B runtime proof: one action-free materialize/replay test passed; five binary64 owners were exact, the canonical ExplainO seed was stable, over-precise damping was emitted as its exact binary32 value, and replay frame hashes matched.
- Phase 3B code-quality audit: 93/100 with baseline check passed; no product runtime source changed.
- Phase 3C contract: `docs/contracts/precision_authority_color_pipeline_consumer_p3c.contract.json`; slice checkpoint token `ck:0c3749cb`.
- Phase 3C inventory: nine tests pass; 120 compiled descriptors reduce to 94 float, 8 integer, and 18 enum declarations through five shared runtime owner structures.
- Phase 3C focused proof: `test_color_pipeline_core` passes 3,329 assertions, `test_color_pipeline_window` passes 441 assertions, `test_color_pipeline_loaded_draft` passes 30 assertions, and the schema-binding and diagnostics-state-I/O targets pass.
- Phase 3C compatibility boundary: live control normalization preserves active draft storage; ordinary direct apply remains compatible when historical tuples lack snapshot reconstruction; explicit loaded-draft apply is transactional and fails closed without normalized readback.
- Phase 3C full native rail: all helper tests passed.
- Phase 3C published runtime: `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`, SHA-256 `1b4cb2565a34f6005e2eb0412468a0edd907ee011db15dc15905d91f0213d3a1`.
- Phase 3C runtime proof: over-precise grade.balance_void normalizes once to exact binary32, the emitted draft reports that value, the frame changes, and action-free replay preserves authoritative state and exact frame identity.
- Phase 3D continuity RED: after Phase 3C closure, the current inventory still reports 86 AUTHORING_IDENTITY_LOSS controls. Source review reduces them to one shared %.5f binary32 input spelling plus the specialized center/zoom camera typed-edit path; this is remaining Phase 3 work, not evidence for storage promotion.
- Phase 3D contract: `docs/contracts/precision_authority_general_float_camera_p3d.contract.json`; slice checkpoint token `ck:d8e1fdc5`.
- Phase 3D inventory: 107 numeric controls classify as 87 truthful binary32, 5 truthful binary64, and 15 intentional mixed-precision, with zero `AUTHORING_IDENTITY_LOSS` results.
- Phase 3D focused proof: ten inventory tests and the complete `test_schema_binding` target pass, including adjacent-binary32 text round-trip plus binary64 camera typed-input witnesses.
- Phase 3D full native rail: all helper tests passed.
- Phase 3D published runtime: `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`, SHA-256 `3773bf8df75f20c0c9ccca0aa546092658a152bd6aef8a0f39e66226fd72f77f`.
- Phase 3D runtime proof: an adjacent binary32 seed-drift edit, exact binary64 center edit, and engine-owned linear-zoom-to-log2 edit survive the published viewer automation route.

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
- [x] Pass 11 - found that the existing UI automation report proved command consumption but exposed no exact combined-seed readback; added one engine-model field and a published-viewer witness.
- [x] Pass 12 - the full native rail found that the standalone automation-report test target did not link the canonical seed owner; repaired build wiring instead of duplicating arithmetic.
- [x] Pass 13 - found that the first inventory repair overclaimed combined seed as truthful binary64 even though fractional drift remains float-backed; reclassified it as intentional mixed precision and added direct-versus-combined assertions.
- [x] Pass 14 - the closure validator rejected `Phase 3A complete` because it truthfully interpreted that phrase as whole-campaign closure while Phases 3B-5 remain open; repaired the current-phase wording to `checkpoint closed`.
- [x] Pass 15 - final source, diff, focused, full-native, publication, runtime, contract, and plan-sync re-read found no additional Phase 3A defect.
- [x] Pass 16 - the first Phase 3B native witness rejected the assumption that all six serialized double members are unrestricted exact owners; `ExplainoSeedNormalize` intentionally canonicalizes `explaino_seed` with float-backed drift.
- [x] Pass 17 - repaired the source-derived inventory and witnesses to distinguish five exact binary64 state owners from the one normalized mixed-precision seed owner; all 149 float conversion lines still resolve to declared float storage.
- [x] Pass 18 - final source, focused, full-native, publication, action-free replay, code-quality, contract, plan-sync, and diff review found no additional Phase 3B defect and no justified product cast or storage mutation.
- [x] Pass 19 - distrust-first review found that fixed `1e-6` draft and runtime equality tolerances could hide an adjacent-binary32 Color Pipeline edit even after the editor became round-trip capable.
- [x] Pass 20 - repaired shared editor spelling, decimal import rounding, exact state identity, normalized readback, transactional loaded-draft apply, and pointer-safe live control synchronization; reran all affected focused owners.
- [x] Pass 21 - final source, focused, full-native, publication, runtime, code-quality, contract, plan-sync, and diff review found no additional Phase 3C defect or justified storage-width promotion.
- [x] Pass 22 - reopening the supposedly closed Phase 3 found 86 remaining general-schema authoring-loss records; source review reduced them to the shared binary32 typed-input owner and the specialized camera typed-edit owner instead of 86 independent repairs.
- [x] Pass 23 - the first runtime fixture requested an adjacent float above the control hard maximum and correctly observed a clamp; the witness now uses adjacent binary32 values around 0.5 and passes.
- [x] Pass 24 - a short outer timeout orphaned one publication compiler before a retry started another. Both exact owned process trees were terminated, their artifacts invalidated, and one clean single-owner publication rerun passed.
- [x] Pass 25 - final source, diff, inventory, focused native, full native, clean publication, and published-runtime review found no additional Phase 3D defect or justified storage promotion.

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
- [x] UI automation command consumption was insufficient readback evidence; the report now exposes the canonical combined seed at 17 digits.
- [x] The automation-report test target needed the same `explaino_seed.cpp` owner as the product route; no duplicate combination formula was introduced.
- [x] `%.17g` repairs the shared editor text, but it does not widen combined-seed storage. The inventory now distinguishes direct double controls from double-plus-float combined seed routes.
- [x] Sub-slice closure wording must not use whole-plan completion vocabulary while successor phases remain open; the plan now says `checkpoint closed` and passes the deterministic sync gate.
- [x] A `double` member is not automatically an unrestricted binary64 replay authority. The Phase 3B RED witness exposed `explaino_seed` canonicalization; inventory now follows the normalization owner instead of classifying from declaration width alone.
- [x] Color Pipeline authoring identity cannot use a visual epsilon as state equality. An adjacent-binary32 RED witness now requires exact draft/runtime comparison while range and integer-validation tolerances remain unchanged.
- [x] Rebuilding the entire draft after every live edit invalidated active UI parameter pointers. The repaired control path canonicalizes value carriers in place while general apply retains topology-rebuilding synchronization.
- [x] Some historical valid tuples apply but cannot be reconstructed by the live-snapshot importer. Direct UI behavior remains compatible; explicit loaded-draft application refuses to commit without authoritative readback.
- [x] Runtime replay evidence must exclude documented volatile diagnostics such as `stats`; authoritative authoring state and decoded frame identity remain exact.
- [x] Phase 3 was prematurely marked closed while the inventory still contained 86 authoring-loss rows. The repaired continuity now closes the two shared owner seams and requires zero remaining authoring-loss classifications before Phase 4.
- [x] Runtime witnesses must respect the declared control range; an out-of-range adjacent-float input proved clamping, not round-trip identity.
- [x] A caller timeout can outlive an owned compiler child. Concurrent publication artifacts are invalid evidence; exact owned trees were cleaned and the canonical proof is the later single-owner clean rerun only.

## Notes

- The prior finding-enrichment E1 callable-centralization phase remains deferred. This emergency campaign does not silently absorb it.
- Phase 2A closes at its evidence checkpoint once validation, commit, receipt, rearward review, and push finish.
- Phase 4 evidence-driven precision promotion review is the next planned execution boundary after the Phase 3D checkpoint; no engine merge is authorized.
