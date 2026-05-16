# Fractal Sample Evidence Widening Staging

## Current Phase

Complete - the repo now carries a closed docs-only staging slice for the bounded engine lane that widens the fractal sample evidence contract without pretending to genericize the whole runtime. This slice did not implement `FractalSampleEvidence` on its own head; it translated the external prep packet into checked-in repo authority, wrote the reusable launch-anchor surface, and staged the bounded implementation order so later sessions could start from the repo instead of chat or external notes alone. Slices A, B, and C later landed on this branch, so this staging doc is now historical kickoff doctrine plus slice-order authority rather than the current implementation status surface. The live repo now carries `FractalSampleEvidence`, `BuildLegacySampleResult(...)`, `SampleFractalEvidencePoints(...)`, and one real paired-state/counterfactual widened consumer in the sidecar measurement lane while later consumer lanes remain deferred.

## Phase Checklist

- [x] Phase 1 - prove the current repo still lacks a checked-in sample-evidence widening packet even though external prep already exists
- [x] Phase 2 - write one checked-in sample-evidence launch-anchor document plus one bounded staged slice map while keeping later implementation prompts flexible
- [x] Phase 3 - validate the docs-only contract, hostile-review the staged packet, and close this staging slice cleanly without claiming the widened evidence seam is already implemented

## Explicit User Asks

- [done] Merge the closed Explaino-family branch stack to `master` and push it before starting the next lane.
- [done] Create a fresh branch for the next engine-oriented work.
- [done] Preload the next slice work in repo because that restart pattern has been working well.
- [done] Keep the older Explaino split-lane questions documented as deferred threads rather than reopening them now.
- [done] Treat this as docs-only staging work, not implementation work.

## Presumption Loop

The controlling risk is fake architecture progress through a broad refactor story. The final answer on the staging head was a bounded docs-only sample-evidence packet: the repo gained one checked-in launch anchor and one staged slice map for widening the sample evidence seam while preserving the then-current single-orbit contract as the shipped legacy projection.

The staged sample-evidence packet now fixes one checked-in slice order:

1. contract skeleton plus legacy projection helper
2. dual-return API plus device/kernel default legacy evidence wiring
3. one real widened consumer proof

An optional cleanup or enforcement slice exists only if the earlier slices leave truthful residual debt that cannot be absorbed in-slice.

This staging slice preserved kickoff flexibility until the live repo proved the first widened consumer. The current branch now chooses the paired-state/counterfactual sidecar lane first, and the packet still fences projection-and-flow, `ExplainO-BalanceVoid`, meta-basin, and DSL-style work out of that first-consumer proof.

## Presumption Evidence

- Bootstrap and repo reread on 2026-05-16 proved branch=`feature/fractal-sample-evidence-widening`, `HEAD=35bd22b`, and a clean tree after the closed Explaino-family stack was merged to `master` and a new branch was cut.
- A repo search before the staging mutation on head `35bd22b` proved the repo then had only `FractalSampleResult` as the checked-in fractal sample payload and lacked any checked-in `FractalSampleEvidence`, `SampleFractalEvidencePoints(...)`, or `BuildLegacySampleResult(...)` helper.
- `ui_app/src/fractal_sample_result.h`, `ui_app/src/fractal_sample_core.cu`, and `ui_app/src/fractal_sample_device.inl` proved the staging head sampling seam was single-orbit-shaped.
- `ui_app/src/generic_sample_core.h` and `ui_app/src/generic_sample_core.cu` proved the codebase already tolerated a second bounded sample contract without requiring a universal abstraction first.
- The external prep packet already existed in `D:\salt-output\explaino_novelty_analysis\20260511_152923_viewer_host_fractal_math_refresh_packet\07_SAMPLE_EVIDENCE_WIDENING_PLAN.md` and `08_SAMPLE_EVIDENCE_IMPLEMENTATION_CHECKLIST.md`, but before this staging slice the repo had no checked-in equivalent restart-safe packet.

## Slice Map

### Slice A - Contract Skeleton

Goal:

- define one widened evidence payload beside `FractalSampleResult`
- define one explicit legacy projection helper
- keep runtime behavior unchanged while the seam is being introduced

Must prove:

- one widened evidence type exists
- one explicit legacy projection helper exists
- no current caller behavior drift is claimed yet

Must not do:

- no renderer replacement
- no first consumer yet

### Slice B - Dual-Return API

Goal:

- add one widened host API
- preserve `SampleFractalPoints(...)` as the default legacy projection path
- teach the device/kernel path to emit default legacy evidence

Must prove:

- one widened host API exists
- default evidence stays legacy-compatible for current callers
- the widened seam is additive rather than a forced rewrite

Must not do:

- no whole-engine generic abstraction
- no meta-basin/controller redesign

### Slice C - First Consumer

Goal:

- choose exactly one real widened consumer
- prove the new evidence fields are justified by actual use

Must prove:

- one non-legacy consumer reads the widened payload meaningfully
- legacy render/runtime equivalence still holds
- the chosen evidence fields are no broader than needed

Must not do:

- no multi-consumer grab bag
- no jump directly to meta-basin, operator-itinerary, or DSL/program-space work

### Optional Cleanup

Only open this if slices A-C leave truthful residual debris too risky to absorb inside the bounded implementation slice.

## Launch Anchor

Future implementation prompts should point first at:

- [fractal_sample_evidence_widening_launch_anchor.md](/C:/code/cuda_newton_fractal_clone/docs/notes/fractal_sample_evidence_widening_launch_anchor.md)

That file is the reusable entry surface for:

- the bounded sample-evidence scope
- the staged slice order
- the required owner seams
- the first-consumer fence
- the required stale-plan gate

## Proof Ledger

- RED 1 landed truthfully on the staging head: the repo had no checked-in sample-evidence widening launch packet before this slice.
- RED 2 landed truthfully on the staging head: the repo still lacked any checked-in widened evidence payload or widened sample API, so the engine lane was still only external planning.
- `docs/notes/fractal_sample_evidence_widening_launch_anchor.md`, `docs/notes/fractal_sample_evidence_widening_staging_PHASED_PLAN.md`, and `docs/contracts/fractal_sample_evidence_widening_staging.contract.json` now exist as the checked-in sample-evidence widening packet.
- The packet keeps `FractalSampleResult` and `SampleFractalPoints(...)` explicit as the shipped legacy projection authority while staging the later widened seam.
- The packet keeps the first consumer bounded to paired-state/counterfactual or projection-and-flow and explicitly fences off meta-basin, operator-itinerary, and DSL/program-space work from the first implementation slice.
- Current repo note: slices A, B, and C later landed the bounded payload/helper, the widened host API/default evidence path, and the paired-state/counterfactual first consumer; use the launch anchor plus the slice-C plan for live implementation truth.

## Hostile Audit

- Status: complete
- Required posture: assume the packet is still trying to smuggle in a broad runtime abstraction, a renderer rewrite, or a premature new-fractal commitment until the checked-in docs disprove that suspicion.
- Current hostile result: the staged docs survive a clean reread without claiming the widened seam is already implemented or allowing the next prompt to collapse into “generic pure stack” rewrite language.

## Audit Passes

- [x] Pass 1 - prove the repo still lacks a checked-in sample-evidence widening packet before writing new doctrine.
- [x] Pass 2 - re-read the new packet as if it still over-hardcodes the future implementation or silently widens into whole-engine refactor work.
- [x] Pass 3 - after docs-only validation, clean reread the final packet as if it still claims the widened seam is already shipped or still lacks a bounded first-consumer fence.

## Audit Findings

- [x] Real staging gap fixed: the repo had external notes for sample-evidence widening but no checked-in restart-safe packet for the next engine lane.
- [x] Real scope-risk fixed: without a checked-in anchor, future prompts were more likely to drift back into “generic pure stack” rewrite language instead of the bounded seam widening the user actually wanted.
- [x] Clean re-read evidence: the repaired docs now keep the lane bounded to sample evidence widening, preserve the legacy projection truth, and leave the first widened consumer intentionally narrow.

## Notes

- Expected owner seams for later implementation:
  - `ui_app/src/fractal_sample_result.h`
  - `ui_app/src/fractal_sample_core.cu`
  - `ui_app/src/fractal_sample_device.inl`
  - `ui_app/src/fractal_renderer.cu`
  - `ui_app/src/generic_sample_core.h`
  - `ui_app/src/generic_sample_core.cu`
  - `ui_app/src/explaino_sidecar_cuda_sample_host.cpp`
  - `ui_app/src/explaino_sidecar_measurement.cpp`
  - `ui_app/src/runtime_walk_field_slime.cpp`
  - `ui_app/tests/test_fractal_sample_result.cpp`
  - `ui_app/tests/test_fractal_sample_core.cu`
  - `ui_app/tests/test_fractal_sample_kernel.cu`
  - `ui_app/tests/test_fractal_sample_equivalence.cu`
  - `ui_app/tests/test_fractal_renderer.cu`
  - `ui_app/tests/test_generic_sample_core.cu`
  - `ui_app/tests/test_generic_sample_parity.cu`
  - `ui_app/tests/test_explaino_sidecar_cuda_sample_host.cpp`
  - `ui_app/tests/test_explaino_sidecar_measurement.cpp`
  - `ui_app/tests/test_runtime_walk_field_slime.cpp`
  - `tests/test_fractal_runtime_explaino_escape_variants.py`
  - `tests/test_fractal_runtime_explaino_runtime_walk.py`
- Source seed for this staged packet:
  - `D:\salt-output\explaino_novelty_analysis\20260511_152923_viewer_host_fractal_math_refresh_packet\07_SAMPLE_EVIDENCE_WIDENING_PLAN.md`
  - `D:\salt-output\explaino_novelty_analysis\20260511_152923_viewer_host_fractal_math_refresh_packet\08_SAMPLE_EVIDENCE_IMPLEMENTATION_CHECKLIST.md`

## Resume Point

Closed. Historical staging only. The live restart surface is now [fractal_sample_evidence_widening_launch_anchor.md](/C:/code/cuda_newton_fractal_clone/docs/notes/fractal_sample_evidence_widening_launch_anchor.md) plus [fractal_sample_evidence_widening_slice_c_paired_state_counterfactual_witness_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/fractal_sample_evidence_widening_slice_c_paired_state_counterfactual_witness_PHASED_PLAN.md); use this staged slice map only for the original ordering and scope fence.

## Action Hostile Review

- Action ID: action-20260516-fractal-sample-evidence-widening-staging
- Suspected Failure Mode: the repo gets another vague future-ideas note that still invites a broad generic-engine rewrite or falsely implies the widened seam already exists.
- Correct Owner/Action: stage the bounded sample-evidence doctrine and slice order in checked-in docs, preserve legacy-projection truth, and keep later implementation contracts live and slice-specific.
- Proof Surface: this plan, the launch anchor, docs-only contract validation, phased-plan sync, hostile-audit validation, committed reread, and clean repo state.
- Blocked Action: shipping `FractalSampleEvidence`, shipping a widened API, shipping a first consumer, or widening into meta-basin/DSL/controller redesign from this staging slice.
