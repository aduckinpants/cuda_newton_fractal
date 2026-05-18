# Fractal Sample Evidence Widening Launch Anchor

Use this file as the checked-in authority surface for the bounded engine lane that widens the fractal sample contract beyond the current single-orbit result without pretending to redesign the whole runtime.

Purpose:

- keep future prompts short without making them underspecified
- force kickoff through checked-in repo authority instead of chat reconstruction
- preserve the bounded sample-evidence scope fence even if the exact kickoff wording changes later

## Non-Negotiable Identity Rules

- This lane is not a generic whole-engine refactor pass.
- The current `FractalSampleResult` contract remains shipped and authoritative on the current head.
- `SampleFractalPoints(...)` must remain available as the default legacy projection path unless live code evidence later proves a narrower truthful transition.
- The first implementation slice must widen the evidence seam, not replace the renderer contract or force a universal runtime abstraction.
- One real non-legacy consumer is enough for the first implementation lane. More than one is scope drift unless the repo state later proves otherwise.
- Do not jump directly to meta-basin, operator-itinerary, or DSL/program-space work from the first sample-evidence slice.

## Scope Fence

Future sample-evidence widening slices may:

- add one bounded widened evidence payload beside `FractalSampleResult`
- add one bounded host API that returns widened evidence
- preserve `SampleFractalPoints(...)` as a thin legacy projection adapter
- teach the device/kernel path to emit default legacy evidence
- prove one real non-legacy consumer that reads the widened payload

Future sample-evidence widening slices must not:

- genericize the whole fractal engine
- replace the renderer contract wholesale
- reopen shipped advanced-color foundation work
- turn sidecar/controller state into the fractal object in the first slice
- treat this lane as a reason to skip proof, stale-plan gates, or slice contracts

## Staged Slice Order

Follow this order unless live repo evidence proves a narrower truthful split is better:

1. contract skeleton plus legacy projection helper
2. dual-return API plus device/kernel default legacy evidence wiring
3. one real widened consumer proof
4. bounded projection-and-flow witness
5. `ExplainO-BalanceVoid` viability / owner-seam proof or a truthful stop-here closure on the current branch
6. optional cleanup or enforcement only if the earlier slices leave truthful residual debt after the viability answer

Do not start with the widened consumer before the widened evidence seam exists.
Do not start with cleanup before the widened seam and the bounded widened-consumer lanes are both real.

## Current Lane Status

- The current repo keeps `FractalSampleResult` as the shipped legacy sample payload, and `SampleFractalPoints(...)` remains the shipped legacy projection API for existing callers.
- `ui_app/src/fractal_sample_result.h` now carries one bounded `FractalSampleEvidence` payload plus one `BuildLegacySampleResult(...)` helper; the widened payload remains bounded to `sample_coord` plus nested `legacy_result`.
- `ui_app/src/fractal_types.h` plus `ui_app/src/fractal_sample_core.cu` now expose `SampleFractalEvidencePoints(...)` as the widened host API and make `SampleFractalPoints(...)` a thin legacy projection adapter over default evidence-emitting sample-kernel output.
- The current repo now has one real widened consumer in the bounded sidecar paired-state/counterfactual lane: `ui_app/src/explaino_sidecar_measurement.cpp` reads `FractalSampleEvidence` through `SidecarMeasurementHost::SampleEvidence(...)` and records paired `minus_counterfactual` / `plus_counterfactual` witness metrics.
- The current repo now has a second real widened consumer in the bounded sidecar projection-and-flow lane: `ui_app/src/explaino_sidecar_lens.cpp` and `ui_app/src/explaino_sidecar_window.cpp` consume the paired counterfactual witness through `projection_flow_bias` to drive live lens guidance and active-zone asymmetry on the window state path.
- `ui_app/tests/test_explaino_sidecar_measurement.cpp` and `ui_app/tests/test_explaino_sidecar_window.cpp` now prove the bounded post-projection question closes as stop-here / non-promotion on this branch: legacy `explaino_balance_void` canonicalizes onto the existing generic `explaino_all` sidecar measurement and window seams on the bounded slice-A/B payload, with zero projection-flow coordinate displacement and no legacy sample-host fallback.
- The widened consumers still reuse the bounded slice-A/B payload only: no new evidence fields were needed beyond `sample_coord` plus nested `legacy_result`, no distinct `ExplainO-BalanceVoid` widened owner seam is proved, and the sample-evidence widening lane therefore stops here on this branch unless a future repo-grounded consumer emerges.
- The repo already has a bounded adjacent pattern in `generic_sample_core.*`, which proves a second sample contract can exist without forcing a universal abstraction first.
- The external prep packet under `D:\salt-output\explaino_novelty_analysis\20260511_152923_viewer_host_fractal_math_refresh_packet` is now translated into this checked-in packet so future sessions can start from repo authority instead of external notes alone.

## Group Under Review

The bounded engine seam for this packet is:

- widened fractal sample evidence

The key questions this lane must answer are:

- what exact evidence is needed beyond `FractalSampleResult`
- whether that extra evidence is generic enough to justify its own bounded struct
- how to preserve legacy callers truthfully while adding the widened seam
- whether the chosen first real non-legacy consumer is strong enough to justify any later widened-consumer work
- whether the widened seam improves future extensibility without lying about generality

## Preferred Consumer Order

The widened consumers should be chosen in this order unless live repo evidence proves otherwise:

1. bounded paired-state / counterfactual witness - landed
2. bounded projection-and-flow witness - landed
3. `ExplainO-BalanceVoid` viability / owner-seam proof - closed on this branch as stop-here / non-promotion because the live code did not prove a distinct widened owner seam or richer emitted evidence need
4. optional cleanup or enforcement only if truthful residual debt remains after the viability answer

Do not jump directly to:

- Explaino-native meta-basin
- Operator-Itinerary
- Program-Space / DSL

Those are later research or deeper engine lanes, not the first proof that the widened seam is real.

## Required Owner Seams

Every future sample-evidence widening kickoff must inspect and name the current seams before mutation:

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
- the narrowest truthful runtime/viewer witnesses under `tests/`

Kickoff must explicitly report:

- where the current single-orbit contract is still authoritative
- whether any internal consumer already assumes richer evidence informally
- what exact new fields are required for the first widened consumer
- whether legacy callers can stay unchanged
- what specific file or API will become the new widened-evidence authority

## Mandatory Kickoff Floor

Every future sample-evidence widening prompt should still require, at minimum:

- mandatory bootstrap reads of this launch anchor and the staged slice plan
- `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8`
- `py -3.14 tools/viewer_host_repo_status.py`
- a fresh slice plan/contract before mutation
- an explicit first RED/proving step
- a hostile review posture
- the post-closeout stale-plan gate on the committed head

The prompt may stay short as long as it points here and the staged plan while still forcing the session to report repo state, active lock, chosen slice, first RED, and expected owner seams.

## Per-Slice Success Rules

### Slice A - Contract Skeleton

Must prove:

- one widened evidence payload exists beside `FractalSampleResult`
- one explicit legacy projection helper exists
- no current caller behavior drift is claimed yet

Current bounded implementation on this head:

- `FractalSampleEvidence` adds only `sample_coord` beside nested `legacy_result`
- `BuildLegacySampleResult(...)` returns the unchanged legacy projection surface
- `SampleFractalPoints(...)` remains the only shipped sample API

### Slice B - Dual-Return API

Must prove:

- one widened host API exists
- `SampleFractalPoints(...)` still works as a truthful legacy projection path
- default widened evidence matches legacy semantics for current callers

### Slice C - First Consumer

Must prove:

- one real non-legacy consumer reads the widened payload meaningfully
- the chosen evidence fields are justified by actual use
- legacy render/runtime equivalence stays intact

### Slice D - Projection And Flow

Must prove:

- one second real bounded consumer reads the existing widened evidence meaningfully on a downstream projection surface
- the chosen projection-and-flow signal is justified by actual live use rather than speculative field growth
- `SampleFractalPoints(...)` and `FractalSampleResult` remain the shipped legacy projection surfaces

### Slice E - ExplainO-BalanceVoid Viability / Stop-Here

Must prove:

- whether `ExplainO-BalanceVoid` is or is not the next truthful bounded widened-consumer lane after projection-and-flow
- what exact owner seam would need widened evidence if that lane is real, or that no such distinct owner seam is currently proved
- whether the existing widened payload is sufficient without richer emitted evidence
- that `SampleFractalPoints(...)` and `FractalSampleResult` remain the shipped legacy projection surfaces

Current bounded implementation on this head:

- `ui_app/tests/test_explaino_sidecar_measurement.cpp` proves legacy `explaino_balance_void` parameter sweeps canonicalize onto the existing generic `explaino_all` measurement lane on `sample_coord + legacy_result`
- `ui_app/tests/test_explaino_sidecar_window.cpp` proves the same canonicalized BalanceVoid rows survive onto the generic window/lens owner seam with zero projection-flow bias and no legacy sample-host fallback
- no distinct BalanceVoid-specific widened owner seam or richer emitted evidence requirement is proved on this branch, so the widening lane stops here absent future repo-grounded evidence

## Post-Closeout Gate

Every future sample-evidence widening slice must re-read its active phased plan on the committed head and fail closed if the plan still contains stale pre-closeout language or untruthful open asks.

## Start Here

Use this packet when resuming or auditing from the current bounded stop point:

- [fractal_sample_evidence_widening_explaino_balance_void_viability_owner_seam_proof_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/fractal_sample_evidence_widening_explaino_balance_void_viability_owner_seam_proof_PHASED_PLAN.md)
- [fractal_sample_evidence_widening_projection_and_flow_witness_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/fractal_sample_evidence_widening_projection_and_flow_witness_PHASED_PLAN.md)
- [fractal_sample_evidence_widening_staging_PHASED_PLAN.md](/C:/code/cuda_newton_fractal_clone/docs/notes/fractal_sample_evidence_widening_staging_PHASED_PLAN.md)
