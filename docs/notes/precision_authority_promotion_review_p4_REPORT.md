# Precision Authority Phase 4 Promotion Review

## Decision

`NO_PRECISION_PROMOTION_AUTHORIZED`

Phase 4 reviewed the remaining float32 storage and parameter-consumer classes after execution truth and authoring identity were repaired. None satisfies the approved promotion gate. No C++, CUDA, schema, state, ABI, renderer, or runtime behavior changed in this phase.

This is not a claim that binary32 is universally sufficient. It is a narrower result: the current evidence does not justify widening another owner now.

## Gate

A promotion requires all of the following at once:

1. a reproducible user-visible or scientific defect;
2. understood storage, ABI, and state compatibility impact;
3. a measured material runtime benefit;
4. explicit float32 continuity plus performance and frame-delta review;
5. truthful state and published-runtime replay after the change.

Type width, a float64 iteration label, or hypothetical deep-zoom sensitivity cannot substitute for those witnesses.

## Reviewed Classes

- General-schema float storage: rejected. Phase 3D repaired the shared typed-input owner and left no residual authoring-loss record.
- State-I/O float destinations: rejected. All 149 conversions resolve to declared float-backed owners; removing the casts would redefine storage rather than repair untruthful loading.
- Color Pipeline float consumers: rejected. The 94 float descriptors now round-trip exact binary32 and no compiled double descriptor establishes a wider contract.
- Combined ExplainO seed fraction: deferred pending a residual reproducible defect. The triggering Rational Escape problem closed at authoring; changing the shared drift carrier would alter canonical normalization and state semantics.
- Camera float mirrors: rejected as non-authoritative. The actual center and zoom owners are already double/log2 double and the typed editor now reaches them directly.
- Float parameters used by float64 iteration routes: deferred pending a family-specific parameter-resolution witness. Double recurrence arithmetic does not automatically require double storage for every input constant.

## Important Non-Candidates

The known lack of perturbation support for several deep-zoom families is not evidence for a scalar storage promotion and remains outside this campaign. Nonzero ExplainO warp remains excluded. Neither issue is being hidden inside this decision.

## Evidence

- Phase 3D inventory: 87 truthful float32, 5 truthful float64, 15 intentional mixed-precision, zero authoring-identity loss.
- Phase 3B: 149 loader float conversions, zero unresolved destinations.
- Phase 3C: 94 Color Pipeline float descriptors, zero double descriptors, exact binary32 normalized readback.
- Phase 2A: every current dispatch owner has source-derived executed-arithmetic evidence, with focused CUDA and published-runtime witnesses for repaired owners.
- Machine-readable decision: `docs/notes/precision_authority_promotion_review_p4.json`.

## Re-entry Rule

A future promotion requires a new bounded plan naming one owner, one reproducible defect, compatibility impact, expected benefit, performance/frame measurements, rollback, and published replay. This report cannot authorize that future mutation by itself.
