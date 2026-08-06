# Root Glow Source Measurement Slice

## Explicit User Asks

- [x] Bound the first commitment to measurement and existing-function tuning.
- [x] Keep the first run within an expected 2-4 hour window.
- [x] Export and measure the authoritative Root Glow source distribution before palette/grading inference.
- [x] Stop for review after classifying the next seam; do not add a new transfer function in this slice.

## Current Phase

Closure is active after published-runtime measurement classified the current source and signed-unit mapping as `existing_mapping_viable`. Product behavior remains unchanged and Root Glow remains disabled.

## Phase Checklist

- [x] Bootstrap, repo status, and rearward review.
- [x] Create and lock this bounded measurement plan/contract.
- [x] Add RED proof that current runtime evidence exposes only image-domain luminance, not raw Root Glow source values.
- [x] Add an optional diagnostic-only raw Source signal summary for `root_log_proximity_v1`.
- [x] Measure deterministic source and post-Shape distributions across the locked scene matrix.
- [x] Test the existing source bias, signed-unit scale/bias, palette cycle, and glow controls using fixed perturbations.
- [x] Classify the next seam without enabling Root Glow or adding a new transfer function.
- [x] Hostile audit and focused native/runtime validation are complete; execute the standard checkpoint, receipts, push, and rearward-review closure sequence.

## Scope Lock

In scope:

- Diagnostic-only measurement of the actual `root_log_proximity_v1` Source signal emitted by the active renderer.
- Finite count, nonfinite count, min/max, mean, standard deviation, p01/p05/p50/p95/p99, and a fixed histogram.
- A separately labelled summary after the existing `signed_unit_map_v1` Shape.
- Stable report authority fields for producer, source id, root-pattern ref/hash, evaluator, fractal tier, color-metric tier, and any narrowing.
- Fixed-perturbation tuning evidence for existing controls only.
- Small deterministic no-mouse runtime scenes and report artifacts.

Out of scope:

- Enabling the Root Glow recipe.
- Adding a new Shape, Source, Palette, adapter, or recipe.
- Changing root-distance/logarithm semantics.
- Changing visible UI layout or controls.
- Graph editor, SDF work, fractal families, Salticid runtime, external state-tool integration, or physical mouse automation.

## Measurement Contract

The diagnostic measures values before Palette and Grading. It must distinguish:

1. `source_raw`: the value returned by `root_log_proximity_v1` after its owning Source parameters and before Shape.
2. `shape_output`: the value after `signed_unit_map_v1` and before Palette.

The runtime report must not infer these distributions from RGB or luminance. Image-domain metrics remain useful but are labelled separately.

The fixed 32-bin histogram ranges are `[-16, 16]` for `source_raw` and `[0, 1]` for `shape_output`, with explicit below-range and above-range counts.

The report is diagnostic-only and opt-in through the no-mouse/report path. Normal live rendering, capture pixels, state authority, and recipe availability do not change.

## Locked Scene Matrix

Use 256x192, animation off, fixed camera, fixed seed, fixed precision, and the published runtime:

1. `explaino_magnet_root_well`, legacy quartic Dynamics Root Field.
2. `explaino_magnet_root_well`, regular N-gon Dynamics Root Field with count 11.
3. One second currently supported root-aware lane selected from existing runtime capability truth, with its exact state recorded in the evidence packet.

Each scene records raw Source, shaped Source, image metrics, frame hash, state hash, capability snapshot, and root-pattern consumer receipt.

## Existing-Control Perturbations

Predeclare and do not tune after seeing results:

- Source proximity bias: `+0.10`.
- Signed-unit scale: `+10%` of the candidate value, bounded to the descriptor range.
- Signed-unit bias: `+0.10`.
- Heatmap cycle: `+0.20`.
- Glow strength: one stable descriptor step or `+0.10` when the descriptor step is not explicit.

Record both source/shape distribution deltas and mean absolute normalized RGB change.

## Decision Gate

Close with exactly one primary classification:

- `existing_mapping_viable`: raw and shaped distributions plus fixed sensitivities show an existing-function candidate worth a later enablement slice.
- `transfer_mapping_needed`: raw Source has useful spread, but the existing signed-unit mapping compresses or saturates it.
- `source_metric_revision_needed`: raw Source itself is degenerate or inadequately sensitive across the scene matrix.
- `measurement_unproven`: diagnostic authority or deterministic runtime proof is not trustworthy.

This slice never enables Root Glow. Any transfer-function or source-metric work requires a new checked-in replan.

## Proof Ledger

| Item | Status | Evidence |
| --- | --- | --- |
| Base | complete | clean pushed head b7ab9c8; rearward review ok |
| Prior image-domain qualification | complete | 5/32 occupied proxy bins; fixed RGB sensitivities below 0.01 |
| Raw Source authority | complete | `viewer.color_source_measurement.v1` is emitted from the renderer Source-signal sidecar before Palette/Grading |
| Existing-control matrix | complete | `artifacts/root_glow_source_measurement/root_glow_source_measurement.json` |
| Classification | complete | `existing_mapping_viable`; no transfer function justified by current evidence |

## Hostile Audit

- Status: complete
- Scope: diagnostic source authority, opt-in behavior, arithmetic-tier truth, scene determinism, RGB-proxy separation, and no recipe enablement.
- Result: diagnostic-only measurement is isolated to the automation-report path; pure single-source eligibility is enforced; nonfinite accounting and precision narrowing are explicit; published runtime and preservation rails are green.

## Audit Passes

- [x] Pass 1 - RED review found the current raw-source observability gap.
- [x] Pass 2 - implementation review found and repaired compile-order, test-authority, nonfinite-accounting, multi-source-authority, and precision-reporting defects.
- [x] Pass 3 - clean re-read confirmed the repaired state through published no-mouse measurement and prior-qualification preservation proof; no additional real defect found.

## Audit Findings

- [x] Workflow bootstrap deadlock required two empty planning placeholders before the prior contract could authorize the new plan paths; no product file was mutated outside the guarded wrapper.
- [x] Current qualification reported image-domain luminance only and could not locate Source-versus-Shape compression; the opt-in Source-signal receipt closes that observability gap.
- [x] The first native build called `WriteHashOrNull` before declaration; a forward declaration and focused compile rail now cover it.
- [x] The focused report test used retired recipe authority and omitted the materialized metadata loader; the target now links/installs the real generated contract.
- [x] Nonfinite raw values were not counted in the shaped domain; both domains now account for every input sample.
- [x] A multi-source stack could have mislabeled one row as complete post-Shape authority; measurement now fails closed unless the active Source stack is exactly one `root_log_proximity_v1` row.
- [x] The first report did not identify f64-to-f32 color-metric narrowing; `color_metric_narrowing` now reports it explicitly.
- [x] The three-scene matrix found raw p05-p95 spreads `5.704578`, `6.382809`, and `2.518976`; shaped spreads `0.662401`, `0.791868`, and `0.453416`; shaped occupancy `26`, `22`, and `32` bins.
- [x] Fixed RGB sensitivities were source bias `0.003509`, shape scale `0.001444`, shape bias `0.029882`, heatmap cycle `0.024936`, and glow `0.000832`. The source/mapping are viable, but source bias, shape scale, and glow are weak at the locked perturbations.

## Planned Validation Targets

- contract validation and phased-plan sync
- code-quality baseline
- focused escape-time coloring and automation-report native rails
- runtime publish
- published no-mouse raw-source measurement matrix
- prior Root Glow qualification preservation
- hostile-audit validation and diff check

## Stop Point

Decision Gate classification is `existing_mapping_viable`. Root Glow remains disabled. The next defensible work is a separate default-tuning/control-pruning and qualification slice, not a transfer-function slice.

Preplanned sliced work is exhausted; stop for replan before more product mutation.

