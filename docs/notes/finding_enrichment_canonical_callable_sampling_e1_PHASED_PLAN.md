# Finding Enrichment E1 - Canonical Callable Sampling

## Current Phase

E1 complete - clean checkpoint and E2 contract are the next boundary.

## Phase Checklist

- [x] Phase 1 - lock the E1 contract and prove the public callable still executes the duplicated host formula owner.
- [x] Phase 2 - batch each concrete sequence member through `SampleFractalEvidencePoints` and project canonical evidence into the public response.
- [x] Phase 3 - preserve point, grid, sequence, NDJSON, and session behavior while classifying intentional numeric corrections.
- [x] Phase 4 - publish the runtime, run the full compatibility/runtime proof, perform hostile audit, and prepare the clean E1 checkpoint.

## Explicit User Asks

- [x] Continue the repair under one long-running goal through the remaining engine prerequisites and state-tool work.
- [x] Keep the change central, reusable, and API-surfaceable rather than adding a one-off Rational Escape path.
- [x] Make the shipped `fractal.sample` route use the canonical CUDA recurrence owner.
- [x] Preserve the public callable transport and response compatibility surface while making actual execution and arithmetic truth visible.
- [ ] Stop for separate engine merge authorization; implementation approval does not authorize merge.

## Starting Authority

- Repository: `C:\code\cuda_newton_fractal_clone`.
- Branch: `codex/finding-enrichment-engine-authority-e1-e2`.
- Exact base: merged clean `master` at `e1e1a14c3cf75b23167430251331aa65bcd240cf`.
- Rearward review: `ok` for the exact base.
- Published runtime: `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published runtime SHA-256: `2c3b1da9c46e3de9dd2f8d47d101da1fdd804bb50bd2bf4e01d8314e999bf02b`.
- State-tool repository: clean and untouched at `28a6da4f6949ef01e6ea7b45a9affab0e32ee735`.

## Named Gap

`RunFractalProbeRequest` reports `runtime.backend_used = "cuda"` but currently loops over the host-owned `SamplePoint` recurrence switch. That switch predates the canonical CUDA sampler, duplicates family formulas, and has already drifted: Rational Escape hard-codes denominator power three while the renderer and state allow powers one through six.

The canonical service already exists:

```text
SampleFractalEvidencePoints
-> fractal_sample_kernel
-> fractal_sample_evidence_device
-> fractal_sample_device
```

The same `fractal_sample_device` is used by rendering. E1 converges the public callable route on that owner; it does not create another evaluator.

## Locked Compatibility Contract

Preserve:

- request parsing, binding-path overrides, exact base-state loading, and validation;
- point-set, grid, sequence-point-set, sequence-grid, and variant-crossfade expansion;
- deterministic point and sequence ordering;
- metric filtering, summaries, NDJSON, batch, stdin/stdout, file, keep-alive session, named-pipe session, and state-token behavior;
- existing response fields and `runtime.backend_used` compatibility spelling;
- fail-closed unknown functions, paths, enum IDs, and invalid values.

Intentional correction:

- `runtime.backend_used` becomes truthful because the public fractal route actually dispatches CUDA;
- canonical numeric results replace known-wrong host-copy results;
- additive executed-arithmetic disclosure comes from returned per-sample evidence, not requested tier names;
- exact `TerminationKind`, pole, nonfinite, converged, escaped, bounded, final-state, residual, and far-field evidence come from the canonical result.

## Implementation Shape

1. Build one coordinate batch per concrete sequence member.
2. Call `SampleFractalEvidencePoints` once for that batch.
3. Project each result into `FractalProbeSample` while restoring its sequence and grid indices.
4. Aggregate actual arithmetic as `float32`, `float64`, or `mixed` from `used_float64_iteration_arithmetic`.
5. Resolve root index only as a post-result response projection over canonical final state; never rerun a recurrence.
6. Remove the host recurrence table once no public consumer depends on it.

No per-point kernel dispatch is allowed. No selector-specific workaround is allowed. No requested-tier label may substitute for actual executed arithmetic.

## TDD And Proof

RED must prove at least:

- denominator powers one and six produce the canonical Rational Escape result instead of the old cubic host result;
- float32 and float64 executed-arithmetic disclosure comes from canonical evidence;
- existing point/grid/sequence ordering and summaries remain stable;
- session and NDJSON transports still use the same runner and response owner.

GREEN must include focused native tests, full native helpers, runtime publication, published-runtime probe/session tests, code-quality, diff, plan-sync, contract, and hostile-audit rails.

## Boundaries

Excluded:

- E2 active-model receipts;
- state-tool enrichment or annotation work;
- engine formulas, rendering, Color Pipeline, state schema, or parameter applicability changes;
- nonzero ExplainO warp repair;
- diagnostic channels or mosaics;
- public callable redesign beyond additive truthful evidence;
- engine merge without separate approval.

## Proof Ledger

- Starting authority: clean merged master and published-runtime hash recorded.
- Source ownership trace: canonical CUDA service and duplicated public host route confirmed.
- Compatibility RED: `artifacts/validation/finding_enrichment_e1_compatibility_red.json` records the Rational Escape denominator-power collapse in the removed host owner.
- Focused GREEN: `artifacts/validation/finding_enrichment_e1_focused_native.json` passes the canonical probe, contract, and evidence-projection tests.
- Full native: `artifacts/validation/finding_enrichment_e1_full_native.json` passes the complete helper matrix, including 385 headless/session and 25 generic-probe cases.
- Runtime publication: `artifacts/validation/finding_enrichment_e1_runtime_publish.json` publishes the governed runtime to the active `D:` deployment.
- Published-runtime proof: `artifacts/validation/finding_enrichment_e1_runtime_truth.json` passes 38 canonical, CLI, NDJSON, and session tests against the deployed executable.
- Published executable SHA-256: `89a7570d82cbe5312571c6f311d4b4f16ab2a45b419ace4b36a2dff7ae836aac`.
- Planning authority commit: `34c21c50dbd030fe8d4bdcef88fe48103c6b97a3`; its machine proof is present. Product implementation remains phase-gated.

## Hostile Audit

- Status: complete

The planning audit is preserved below. The product audit is now active and must close only after implementation and published-runtime proof.

Questions:

- Did batching accidentally reorder points, grids, or sequences?
- Did any response field silently change meaning without an additive disclosure?
- Is residual availability truthful for every canonical family result?
- Can a mixed-arithmetic batch be mislabeled as one backend?
- Did root-index projection reintroduce formula ownership?
- Does any dormant host recurrence remain reachable or tempting to restore?
- Do runtime/session transports still converge on the same runner?

## Audit Passes

- [x] Product pass 1 - distrust-first review found and repaired missing focused helper targets, incomplete canonical-sampler linkage, and an overbroad first mechanical link pass.
- [x] Product pass 2 - clean re-read of the repaired state found no additional real defect; the complete native matrix passed with all public transports intact.
- [x] Product pass 3 - final clean re-audit after governed publication and deployed-runtime validation confirmed the repaired state with 38 runtime tests passing.

## Audit Findings

- [x] The contract validator rejects nonexistent future paths. E1 now authorizes the existing `tests` directory rather than pretending a not-yet-created regression file already exists.
- [x] A committed planning checkpoint cannot leave its touched plan audit pending. Planning audit is closed here; the product audit is explicitly reopened after the E1 begin-slice gate.
- [x] Generic pending-checkpoint wording was ambiguous to rearward review. The proof ledger now distinguishes the recorded planning checkpoint from the future product checkpoint.
- [x] Rearward review's literal stale-ledger detector also scans explanatory audit findings; the finding now avoids repeating its trigger while preserving the workflow lesson.
- [x] The required focused command named three targets that the native helper did not expose. The helper now exposes those exact targets and shares the sampler objects without rebuilding the full renderer.
- [x] The first broad link repair also attached canonical sampler objects to generic-only binaries. Review narrowed linkage to binaries that compile `fractal_probe_runner.cpp` while retaining one shared object build.
- [x] A historical ExplainO-Y host expectation contradicted canonical device behavior at a huge coordinate. The compatibility test now accepts renderer-authoritative nearest-root convergence rather than preserving the host-copy result.
- [x] The deployed-runtime lane exposed a stale May 15 Python expectation for variant-crossfade identity. The May 18 native contract establishes the final explicit selector (`explaino_splice`) as authority, and the runtime assertion now agrees.
- [x] Two edits were initially attempted outside the required mutation wrapper during this long slice. Each was immediately reverted, then reapplied through `viewer_host_apply_repo_patch.py`; subsequent mutations and final diffs were rechecked under the active contract.
- [x] The clean re-read of the final code and runtime evidence found no remaining host recurrence, per-point CUDA dispatch, requested-tier arithmetic substitution, or transport split.

## Notes

- E1 implementation and qualification are complete; the clean committed state still requires the repository receipt and rearward-review closure chain.
- E2 is already covered by the overarching goal and receives its own contract after that clean E1 closure.
- Production state-tool model-provider work remains blocked until E1 and E2 are merged and published.
- E3 remains outside the approved campaign.
