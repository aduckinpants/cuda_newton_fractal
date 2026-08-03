# Finding Enrichment Engine Authority V1 Phased Plan

## Current Phase

Phase 1 / E0 is complete at product checkpoint `bdbffea018485aeaf69ca1a50bbfd81caf561d74`. The published runtime was rebuilt from that exact clean checkpoint and its tracked Rational Escape witness passed. E1 remains deferred pending explicit user review and authorization.

## Phase Checklist

- [x] Phase 0 - lock this engine-native campaign plan, record source-to-binary provenance status, validate the planning contract, checkpoint, push, and stop for review.
- [x] Phase 1 / E0 - repair Rational Escape numeric truth in the canonical device sampler with controlled float32/float64 evidence.
- [ ] Phase 2 / E1 - make the shipped callable sampling route use the canonical CUDA sampling owner and classify any retained compatibility route explicitly.
- [ ] Phase 3 / E2 - add a deterministic state-bound active-model receipt and bounded canonical point evaluation for supported model providers.
- [ ] Phase 4 - merge under separate authorization, publish from exact merged master, and produce the engine-to-tool handoff.
- [ ] Phase 5 / E3 - only under a later approved plan, add diagnostic-channel rendering required by a full diagnostic mosaic.

## Explicit User Asks

- [x] Design a slotted enrichment authority that can grow across fractal families instead of a Rational Escape-only feature.
- [x] Keep deterministic mathematics and truthful recurrence evidence engine-owned; keep solving, annotation layout, cache, mosaic assembly, disclosure, and packet prose tool-owned.
- [x] Treat `explaino_warp_strength != 0` simply: the provider is unavailable because the rendered subject cannot currently be trusted. Do not implement, model, or test nonzero warp beyond a trivial unsupported-status check.
- [x] Verify the current Rational Escape float32/float64 mismatch and distinguish current source truth from behavior proven in the packet-bound published executable.
- [x] Reassess V9 token-cost work against enrichment instead of assuming every prior optimization remains mandatory.
- [x] Begin only with the engine-native planning slice; no engine product code is authorized by this checkpoint.
- [x] Do not merge an engine pull request without separate user approval.

## Starting Authority

- Repository: `C:\code\cuda_newton_fractal_clone`
- Branch: `codex/finding-enrichment-engine-authority-v1`
- Exact starting commit: `09d5664b77116b716f83dd8df1085e88596498d0`
- Starting base: exact clean `master`, equal to `origin/master`
- Rearward review: `artifacts/hooks/viewer_host_rearward_review/09d5664b77116b716f83dd8df1085e88596498d0.json` reports `ok`.
- Published executable inspected during Phase 0: `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`
- Published executable SHA-256 observed during Phase 0: `ae329398693a5872faced0fa6f9cf57868788fc975b07ece5150954ac4face78`
- The executable has no embedded file or product version and no colocated commit receipt was found. Its exact source commit is therefore unproven until an exact-head rebuild/publication receipt is produced.
- State-tool repository remained clean and untouched during engine inspection.

## Bounded Goal

Establish reusable engine authorities that let a downstream finding-enrichment tool consume truthful, state-bound mathematics without duplicating recurrence semantics:

```text
complete captured state
+ exact runtime identity
-> selected active-model receipt
+ bounded canonical evaluation evidence
-> tool-owned solving, annotations, contact sheets, and packet disclosure
```

The first provider is Rational Escape because it exposed a concrete numeric and callable-path authority defect. The architecture must allow later providers and a common-subset layer without forcing every family into one formula model.

## Authority Architecture

### Engine-owned

- active model identity selected from the complete loaded state;
- exact serialized parameters that participate in that model;
- numeric backend actually used;
- canonical point or subsample evaluation through the renderer-owned recurrence implementation;
- truthful termination classification and raw numeric evidence;
- deterministic state/runtime binding and explicit unsupported reasons.

### State-tool-owned

- analytic solving derived from engine receipts;
- presentation-neutral annotation records;
- annotation rendering and annotation-render receipts;
- alternate-render contact sheets;
- later diagnostic-mosaic assembly;
- caches and cache manifests;
- disclosure profiles and packet prose;
- V9 orchestration, token accounting, and model-routing policy.

### Presentation and identity separation

Analysis identity is a function of immutable input state, runtime identity, provider version, and analysis settings. Disclosure profile is not part of that identity. A separate disclosure manifest selects which immutable outputs enter a packet. Presentation-neutral annotation records are distinct from raster annotation receipts and output-image hashes.

## Phase 0 - Provenance, Contract Lock, And Planning Checkpoint

This phase is documentation and contract work only.

Required evidence:

1. live repository authority, exact branch/head, clean tree, remote parity, and rearward review;
2. published executable path and hash;
3. an explicit `SOURCE_PROVEN` versus `PACKET_BINARY_PROVEN` distinction for every claimed defect;
4. source traces for the renderer, canonical CUDA point sampler, legacy host probe route, result/evidence types, and current CLI surfaces;
5. a checked-in native plan and machine-readable contract;
6. planning-contract validation, plan sync, hostile audit, checkpoint, push, and clean tree.

Phase 0 does not claim that the published executable contains the source defect merely because current source does. Exact packet-bound behavior requires an executable publication receipt or a controlled runtime witness.

## Phase 1 / E0 - Rational Escape Numeric Truth Repair

### Goal

Make Rational Escape honor the resolved numeric backend in the canonical device recurrence while retaining the existing truthful float32 path and intentionally correcting float64 behavior.

### Current source finding

`ui_app/src/fractal_sample_device.inl` selects `useFP64` globally and most families branch on it. The `FractalType::explaino_rational_escape` branch currently uses only float coordinates and float polynomial helpers. It clamps denominator power to 1 through 6 but does not branch on `useFP64` and does not emit a truthful family-specific termination kind. This is `SOURCE_PROVEN`; published-executable behavior remains separately qualified.

### Allowed mutation

- canonical device recurrence and narrow reusable helpers;
- result/evidence fields needed for truthful termination and numeric evidence;
- focused native and runtime tests;
- exact-head build/publication proof after implementation.

### Required behavior

- preserve the current float32 branch where it is already truthful;
- implement the corresponding float64 branch with the same recurrence and denominator-power clamp;
- classify at least pole/nonfinite, escape-radius, and max-iteration outcomes truthfully without laundering them into an unrelated category;
- expose meaningful raw evidence such as final complex value, iteration count, denominator magnitude or pole metric, and escape magnitude where authoritative;
- do not claim exact final rendered-pixel equivalence from one point sample under SSAA or color aggregation;
- do not change warp behavior. Provider availability is false whenever serialized `explaino_warp_strength` is nonzero.

### TDD and review

Start with controlled RED tests spanning float32, float64, denominator powers, near-pole, escape, nonfinite, and capped iteration cases. Review expected float64 image changes as intentional truth correction, not as a compatibility regression. Any unexpected float32 image change is a blocker.

## Phase 2 / E1 - Canonical Callable Sampling

### Goal

Make every authoritative callable point sample reuse the same canonical CUDA recurrence owner as rendering.

### Current source finding

`ui_app/src/fractal_sample_core.cu::SampleFractalEvidencePoints` and `ui_app/src/fractal_renderer.cu` both call `fractal_sample_device`. `ui_app/src/fractal_probe_runner.cpp::SamplePoint` separately reimplements family math on the host; its Rational Escape branch hard-codes a cubic denominator and can diverge from serialized denominator power. The probe response currently labels the runtime backend `cuda` while this route evaluates through the host implementation. These are `SOURCE_PROVEN` ownership and truth defects.

### Required behavior

- the shipped `fractal.sample` route delegates to the canonical sampling service;
- request/response schemas, point-set/sequence/grid semantics, NDJSON behavior, and deterministic ordering remain compatible unless a separately approved contract changes them;
- execution backend and numeric backend are reported distinctly and truthfully;
- any retained host route is explicitly classified as compatibility-only, non-authoritative, or removed after consumer review;
- no second recurrence table or selector-specific workaround is added.

### Gate

E1 is merged and published before a production state-tool provider may consume canonical evaluation evidence.

## Phase 3 / E2 - Active-Model Receipt And Provider Slot

### Goal

Expose a deterministic, state-bound model receipt plus bounded evaluation requests without making the engine responsible for analysis presentation.

### Candidate public surface

The engine-native implementation plan must lock exact CLI/API spelling after current CLI convention review. The accepted semantics are:

```text
complete loaded state
-> provider selection
-> deterministic active-model receipt or explicit unavailable result

complete loaded state + bounded point list
-> canonical evaluation evidence bound to the same provider and runtime
```

### Common receipt envelope

- schema version and provider ID/version;
- selected fractal type and exact state hash;
- runtime executable identity and numeric backend;
- model applicability status and machine-readable unavailable reason;
- exact participating serialized fields and values;
- recurrence/model description identifiers grounded in compiled engine authority;
- provider-specific payload;
- deterministic fixed ordering and no volatile machine-local fields.

### Rational Escape provider

Initial provider ID: `polynomial_over_power_escape.v1`.

It may expose:

- real polynomial coefficients in declared order;
- denominator power after the engine's existing clamp semantics;
- escape radius, epsilon, iteration cap, and other actually participating values;
- canonical critical or singular points only when mechanically implied by the active model;
- explicit unsupported status when warp is nonzero.

It must not solve orbit equations, infer visible feature identity, place annotations, choose camera moves, or claim that a canonical point caused a visible pixel feature.

### Provider scalability

Provider selection is slotted by live fractal/model authority. Later providers may share a common subset for state binding, numeric backend, evaluation, critical/singular points, and termination evidence. Unsupported families fail explicitly; they are not forced through Rational Escape vocabulary.

## Phase 4 - Merge, Publication, And Engine-To-Tool Handoff

Each implementation slice receives its own checked-in contract, RED/GREEN proof, mandatory repository rails, hostile audit, checkpoint, and clean tree. Engine PR merges require separate user approval.

After authorized merges:

1. fast-forward exact local master;
2. rerun mandatory post-merge authority checks;
3. build and publish from exact merged master;
4. record commit, executable SHA-256, runtime identity, and public-output hashes;
5. smoke-test new surfaces from the published executable;
6. confirm engine tree clean and state-tool tree untouched.

Production state-tool model-provider work is blocked until E0, E1, and E2 are merged and published. E3 is not a prerequisite for the basic model provider or annotation workflow.

## Phase 5 / E3 - Deferred Engine Diagnostic Channels

This phase is not authorized by the current planning checkpoint and requires a new user-approved engine plan.

It may expose a small reviewed set of engine-owned diagnostic channels for a later diagnostic mosaic. Channel semantics must be current-source grounded, deterministic, and distinct from ordinary alternate renderings. The exact tile set is intentionally provisional.

A low-cost alternate-render contact sheet is state-tool work and may precede E3. It is an observation aid, not proof of a diagnostic channel's internal semantics.

## Warp Rule

Warp is deliberately simple in this campaign:

```text
explaino_warp_strength == 0
-> provider may be available if all other gates pass

explaino_warp_strength != 0
-> provider unavailable: current rendered model cannot be trusted for this analysis
```

No warp-aware materialization, correction, dedicated acceptance fixture, or extended architecture is authorized. A trivial unsupported-status unit test is sufficient.

## Rational Escape Precision Repair Policy

The float64 change is an intentional truth correction, not a promise of pixel continuity. The implementation must:

- preserve the float32 path where appropriate;
- compare controlled float32 output before and after and fail on unintended drift;
- measure expected float64 state/sample/frame deltas;
- review near-pole sensitivity and termination changes explicitly;
- publish only after current repository rails and a representative runtime witness pass.

## State-Tool V9 Reassessment

Enrichment changes the token economics but does not eliminate the cost problem. Before V9 implementation, classify each existing cost item:

- `retained`: still required after deterministic enrichment;
- `reduced`: enrichment shrinks the required model context or reasoning;
- `replaced`: deterministic authority removes the model task entirely;
- `orthogonal`: transport/session economics remain unchanged.

Likely retained or orthogonal items include dollar-denominated pre-dispatch budgeting, context-tier avoidance, resource deduplication, provider-file reuse, cache accounting, and model-tier ablation. Deterministic math facts, active parameter extraction, and basic critical/singular point computation may be reduced or replaced. No cost claim is accepted without measured token and dollar receipts.

## Mutation Boundaries

Allowed only after the corresponding implementation phase receives a new locked contract:

- Rational Escape numeric truth in the canonical sampler;
- canonical callable routing;
- narrow result/evidence extensions;
- deterministic active-model receipt and bounded evaluation CLI/API;
- focused tests, docs, build wiring, receipts, and publication proof.

Forbidden without a new user-approved plan:

- unrelated formulas, samplers, rendering, UI, or Color Pipeline behavior;
- state JSON schema or parameter applicability changes;
- warp implementation or repair;
- solver, annotation renderer, cache, contact-sheet, mosaic, disclosure, or packet-prose logic in the engine;
- automatic feature detection, camera mutation, aesthetic scoring, or causal claims from image appearance;
- broad callable API redesign;
- state-tool mutation before the engine handoff gate.

## TDD, Dirty Experiments, And Rollback

- Behavioral work begins with focused RED evidence.
- One phase and one coherent dirty experiment may be active at a time.
- Use only repository-authorized mutation wrappers and validation rails.
- If canonicalization requires a public request/response break, stop for contract revision.
- If truthful float64 repair unexpectedly changes float32 output, stop and classify before continuing.
- If provider truth requires modeling nonzero warp, report unsupported and continue only with zero-warp fixtures.
- Rollback uses the last coherent checkpoint; do not reset or overwrite unrelated work.

## Validation And Closure

Phase 0 runs only its workflow-only contract rails. Later behavioral contracts must lock every mandatory current repository rail, including focused native tests, full native build/test, published runtime proof, code-quality baseline, diff check, hostile audit, checkpoint receipts, rearward review, push, and clean tree.

The campaign does not authorize engine merge. At the end of each implementation branch, stop at a clean pushed PR pending separate approval.

## Proof Ledger

- Bootstrap/status: clean `master` at `09d5664b77116b716f83dd8df1085e88596498d0`, equal to `origin/master`.
- Rearward review: current-head artifact reports `ok`.
- Published executable: SHA-256 `ae329398693a5872faced0fa6f9cf57868788fc975b07ece5150954ac4face78`; embedded file/product version absent; exact source commit unproven.
- Canonical renderer trace: `ui_app/src/fractal_renderer.cu` calls `fractal_sample_device`.
- Canonical CUDA point trace: `ui_app/src/fractal_sample_core.cu::SampleFractalEvidencePoints` calls the same `fractal_sample_device` and resolves sample numeric tier.
- Rational Escape source defect: `ui_app/src/fractal_sample_device.inl` ignores `useFP64` in the Rational Escape branch and uses float polynomial/denominator arithmetic.
- Result limitation: `ui_app/src/fractal_sample_result.h` has no pole-specific termination value; Rational Escape currently leaves termination at the generic default.
- Legacy callable divergence: `ui_app/src/fractal_probe_runner.cpp::SamplePoint` duplicates family math and hard-codes the Rational Escape denominator as cubic.
- Callable ownership inconsistency: the host probe response labels the runtime backend `cuda` despite evaluating through `SamplePoint`.
- Existing centralization seam: current source already exposes `SampleFractalEvidencePoints`; E1 should converge the public callable route on it rather than create a new evaluator.
- Current report authority: `.local/reviews/finding_enrichment_and_v9_reassessment_2026-08-02.md` in the state-tool repository, SHA-256 `57271aa7a38ff3b9621f80be0159680340d42685b9ba466e2e10905ae02f8417`; this is advisory planning input, not engine runtime authority.
- Phase 0 contract validation and phased-plan sync passed against the locked successor contract.
- Focused workflow-tool rail: 30 passed, 2 explicitly deselected after the unfiltered run exposed unrelated baseline failures in stale FITS workbench expectations and a stale 47-selector contract count versus the live 51-selector catalog.
- The inherited catalog contract was restored byte-for-byte after successor lock; product source has no Phase 0 diff.
- E0 RED: `artifacts/validation/finding_enrichment_e0_red_sample_kernel.json` records the expected missing `TerminationKind::pole` compile failure.
- E0 focused GREEN: `finding_enrichment_e0_sample_kernel.json` reports 1,052 passed CUDA sample assertions; `finding_enrichment_e0_fractal_types.json` reports 140 passed type/ordinal assertions.
- E0 full native: `finding_enrichment_e0_full_native.json` reports the complete helper suite passed.
- E0 product checkpoint: `bdbffea018485aeaf69ca1a50bbfd81caf561d74` (`Repair Rational Escape numeric truth`).
- E0 exact-checkpoint publication: `finding_enrichment_e0_runtime_publish_exact_checkpoint.json` records a successful build and publish from the clean product checkpoint; the published executable SHA-256 is `240987082532e8e7fbd9676f06b1314ed1bbdab3d8d6007bdc99fba249e42c83`.
- E0 published runtime: `finding_enrichment_e0_runtime_truth_exact_checkpoint.json` passes the tracked zero-warp Rational Escape fixture against that exact-checkpoint executable with deterministic, distinct float32/float64 frames.
- E0 raw witness: `.local/finding_enrichment_e0/postchange_b550c769/runtime-witness.json` records float32 frame continuity `8290c8cf...e046ebd == 8290c8cf...e046ebd` and controlled tier hashes `cc92e38c...743c9bb` versus `d989246a...e873439`.

## Hostile Audit

- Status: complete

Audit questions:

- Did the plan confuse current source evidence with behavior proven in the published packet-bound executable?
- Does the provider architecture scale beyond Rational Escape without inventing a universal formula model?
- Is nonzero warp handled as a simple unsupported gate rather than an implementation campaign?
- Are annotation data, annotation rendering, analysis identity, and disclosure identity kept separate?
- Does E1 converge on the canonical sampler rather than create another route?
- Could active-model receipts drift from the complete loaded state or runtime identity?
- Did the plan accidentally move solving, caching, mosaics, or packet prose into the engine?
- Are E0, E1, and E2 explicitly merged and published prerequisites for production state-tool integration?
- Is float64 correction reviewed honestly while float32 continuity remains protected?
- Did the planning slice mutate product code or claim merge authority?

## Audit Passes

- [x] Pass 1 - separated source-proven defects, packet-binary proof, provider/runtime authority, and tool-owned presentation responsibilities.
- [x] Pass 2 - clean re-read after contract validation, plan sync, and the focused workflow-tool baseline found no additional real defect in the repaired planning state.
- [x] Pass 3 - final clean re-read confirmed the repaired state does not authorize product mutation or engine merge and preserves the state-tool repository untouched.
- [x] Pass 4 - reviewed the E0 diff for float32 operation-order continuity, an actual double-precision branch, append-only termination identity, and absence of host-probe mutation.
- [x] Pass 5 - re-read the repaired E0 state after focused and full native GREEN; no additional real defect was found in enum consumers or canonical sampler ownership.
- [x] Pass 6 - clean re-read of published-runtime evidence confirmed the inert 729x fixture was not forced and the tracked 2^16 zoom fixture proves deterministic tier separation.
- [x] Pass 7 - rebuilt and published from exact clean product checkpoint `bdbffea018485aeaf69ca1a50bbfd81caf561d74`, hashed the deployed executable, and reran the tracked runtime witness successfully.

## Audit Findings

- [x] The inherited completed contract did not authorize its successor plan/contract. The documented minimal contract-only bootstrap precedent named the two successor paths, locked this successor, and restored the inherited contract byte-for-byte.
- [x] Initial source-to-binary provenance was incomplete because the published executable did not identify its source commit. E0 closes its behavioral claim through an external exact-head publication receipt: clean product checkpoint `bdbffea018485aeaf69ca1a50bbfd81caf561d74` produced executable SHA-256 `240987082532e8e7fbd9676f06b1314ed1bbdab3d8d6007bdc99fba249e42c83`, followed immediately by the passing tracked runtime witness.
- [x] Rational Escape has two distinct source defects: numeric-tier divergence in the canonical device sampler and an independently duplicated host callable implementation. E0 and E1 remain separate bounded phases.
- [x] The current termination enum does not name pole termination. The E0 contract must choose a truthful compatible extension rather than reuse an unrelated status silently.
- [x] A point/subsample result is not exact final pixel authority under SSAA and color aggregation. Public wording and tests preserve that boundary.
- [x] V9 cost work remains economically necessary but must be reassessed item by item after deterministic enrichment is concrete.
- [x] The unfiltered workflow-tool run exposed two unrelated baseline failures. They remain visible and out of scope; the focused planning rail excludes only those exact tests and passes its other 30 tests.
- [x] The first published-runtime fixture at approximately 729x was visually inert and produced identical tier frames. It was classified as inert, then replaced by the same tracked state at `log2_zoom = 16`, where the tier effect is deterministic and visible.
- [x] Building the runtime fixture from a generic ExplainO baseline produced an invalid Rational Escape state. The repair promotes the exact zero-warp capture state into `tests/fixtures/rational_escape_numeric_truth_v1/state.json`; production does not depend on the external capture path.
- [x] The pre-change binary normalized a loaded `fast` state to `standard`, confirming the Phase 0 source-to-binary provenance warning. Float32 continuity is therefore proven with the old and new binaries' shared default low-zoom command, not by pretending that loaded-state comparison was authoritative.
- [x] E0 changes only the canonical device sampler and shared termination serialization. The duplicated host `SamplePoint` implementation remains unchanged and explicitly deferred to E1.

## Notes

- The state-tool repository remains clean and untouched during engine planning and implementation.
- E0 was explicitly authorized after Phase 0 review and is complete. The next approved boundary is user review; E1 product mutation is not authorized by this checkpoint.
- On 2026-08-02 the user authorized a separate emergency precision-authority successor campaign. E1 remains deferred; the successor begins with a system-wide precision inventory rather than callable-sampling mutation.
- No CUDA engine merge is authorized by this plan or by approval to implement E0.
