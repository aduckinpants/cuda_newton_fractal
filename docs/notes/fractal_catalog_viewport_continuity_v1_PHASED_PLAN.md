# Fractal Catalog And Viewport Continuity V1 Phased Plan

## Current Phase

Phase 1 - add deterministic engine-owned viewport facts with focused RED/GREEN proof.

## Phase Checklist

- [x] Phase 0 - locked this engine-native plan and contract from exact clean master.
- [ ] Phase 1 - add deterministic engine-owned viewport facts with focused RED/GREEN proof.
- [ ] Phase 2 - review and ground the direct native fractal families.
- [ ] Phase 3 - review and ground the ExplainO and composed-analysis families.
- [ ] Phase 4 - review and ground custom, SDF, and root-field consumer selectors; prove zero unavailable rows.
- [ ] Phase 5 - publish, run all mandatory rails, complete hostile audit, checkpoint, push, and stop at the engine merge-approval boundary.

## Explicit User Asks

- [open] Cover every shipped fractal selector, not only McMullen.
- [open] Give downstream exploratory agents enough engine-owned mathematical context to reason about dynamics changes without treating every serialized value as active.
- [open] Add exact engine-owned viewport geometry so high-zoom dynamics changes can be paired with deliberate same-window, feature-tracking, or transition-survey camera intent.
- [open] Keep the work central, reusable, and API-surfaced rather than adding a one-off state-tool workaround.
- [open] Advance to the furthest clean review-ready checkpoint allowed overnight, but do not merge the CUDA engine pull request without separate user approval.

## Starting Authority

- Repository: `C:\\code\\cuda_newton_fractal_clone`
- Branch: `codex/fractal-catalog-viewport-continuity-v1`
- Exact starting commit: `337d66c8e2571f59b5757a27497ebe38717e30df`
- Starting base: exact clean `origin/master`
- Rearward review: `artifacts/hooks/viewer_host_rearward_review/337d66c8e2571f59b5757a27497ebe38717e30df.json` reports `ok`.
- Live selector authority: `ui_app/src/fractal_catalog.h::kFractalCatalog`, observed at 51 rows during preflight.
- Existing descriptive export: schema V1 at `--describe-fractal-catalog`, currently reviewed for two selectors and unavailable for the other live rows.
- Renderer camera authority: `ui_app/src/fractal_renderer.cu::fractal_kernel`, using `center_hp_x`, `center_hp_y`, `log2_zoom`, render dimensions, aspect, and rotation.
- Historical editorial seed: `D:\\salt-output\\explaino_novelty_analysis\\20260612_000000_viewer_host_fractal_math_refresh_packet_doc_update_working\\fractal_catalog_current.json`; it is review evidence only and never runtime authority.

## Bounded Goal

Complete the existing engine-owned descriptive catalog for every row in the live catalog and expose the exact renderer camera mapping as a deterministic viewport-facts artifact. The result lets a downstream agent combine:

```text
selected recurrence and termination semantics
+ applicable controls
+ exact captured viewport geometry
+ the visible frame and field notes
-> an evidence-disciplined dynamics and camera override
```

McMullen is the first high-zoom acceptance witness, not a special implementation path.

## Public Contract A - Descriptive Catalog Coverage

The existing commands and schema remain stable:

```text
fractal_ui.exe --describe-fractal-catalog
fractal_ui.exe --describe-fractal-catalog-json <path>
```

Every current and future `kFractalCatalog` row must have `description_status: "reviewed"` before this campaign closes. `description_status: "unavailable"` remains a valid schema value for forward-compatible fail-soft consumers, but the focused coverage test fails while any shipped row is unavailable.

Each reviewed description retains the existing V1 fields:

1. `math_summary`
2. `recurrence_or_field_model`
3. `state_order`
4. `termination_or_classification`
5. `interpretation_notes`
6. concise repository-relative `source_refs`

Descriptions must distinguish recurrence dynamics, downstream coloring or field consumption, and capture-specific causal evidence. They must not claim visible contribution from a nonzero control, infer basins from a continuous proximity field, infer visible symmetry from serialized roots, spatially localize global statistics, or claim exact self-similarity from one frame.

Shared family prose may be compiled from reusable reviewed templates, but every exported selector/field/sentence must retain a deterministic selector-scoped evidence mapping. Runtime code must not parse the evidence ledger or historical JSON.

## Public Contract B - Viewport Facts V1

Add one central headless/testable module that computes the same camera mapping used by `fractal_kernel`. Expose it through:

```text
fractal_ui.exe --describe-viewport-facts --load-state-json <state.json>
fractal_ui.exe --describe-viewport-facts-json <path> --load-state-json <state.json>
```

Finding capture also writes the same bytes as:

```text
fractal-viewport-facts.json
```

The capture sidecar is derived from the exact `ViewState` and capture `RenderSettings` used for the frame. The CLI derives facts from the complete loaded state and its serialized render dimensions. Stdout emits only deterministic UTF-8 JSON; file mode uses the established same-directory `.tmp` replacement convention; diagnostics use stderr.

Schema root:

```json
{
  "schema_version": 1,
  "mapping_id": "cuda_fractal_renderer_pixel_center_v1",
  "selected_fractal_type": "mcmullen",
  "render": {},
  "camera": {},
  "local_frame": {},
  "complex_pixel_basis": {},
  "continuous_edge_corners": [],
  "pixel_center_corners": [],
  "axis_aligned_complex_bounds": {},
  "fit_model": {}
}
```

The deterministic fixed-order payload contains:

- render width, height, and aspect ratio;
- exact serialized `center_hp_x`, `center_hp_y`, `log2_zoom`, resolved linear zoom, and rotation;
- local horizontal/vertical half spans and full spans;
- complex-plane step vectors for one image pixel in X and Y;
- magnitudes in complex units per pixel;
- continuous frame-edge corners after rotation;
- actual corner pixel-center coordinates after rotation;
- the axis-aligned complex bounding box of the rotated continuous frame;
- the engine mapping equations and inverse fit equations needed to frame a set of predicted feature points with a declared margin.

The formulas must be mechanically identical to renderer authority:

```text
aspect = width / height
zoom = 2^log2_zoom
base = 2 / zoom
local_half_width = base * aspect
local_half_height = base
nx = ((px + 0.5 + sample_offset_x) / width - 0.5) * 2
ny = ((py + 0.5 + sample_offset_y) / height - 0.5) * 2
local = (nx * base * aspect, ny * base)
complex = center + rotate(local, rotation_degrees)
```

For an inverse fit, feature points are transformed into camera-local coordinates around the proposed center. With positive half extents including the caller's margin:

```text
max_zoom_x = 2 * aspect / required_local_half_width
max_zoom_y = 2 / required_local_half_height
fit_zoom = min(max_zoom_x, max_zoom_y)
fit_log2_zoom = log2(fit_zoom)
```

The engine reports this mapping; it does not choose subjects, predict mathematical feature locations, or auto-move the camera.

## Dynamics And Viewport Interpretation Contract

The future state-tool packet will apply one general rule across all selectors:

- Color-only changes preserve the exact camera unless the user separately asks to reframe.
- Every non-color dynamics change at meaningful zoom carries an explicit camera intent: `same_window_comparison`, `feature_tracking`, or `transition_survey`.
- Small numerical parameter changes must never be described as visually small merely from their magnitude.
- Feature tracking must estimate the subject before and after the change. Unique continuation may recenter; split, merge, disappearance, or ambiguity must frame the branch set or transition neighborhood rather than claim one unchanged object.
- If the feature location cannot be derived from attached engine authority and transparent mathematics, the agent must state that limitation and choose an honest wider comparison frame instead of fabricating a precise camera.
- Exact frame fitting uses the attached viewport facts and image aspect, never a Python-maintained universal camera formula.

Selector descriptions supply the recurrence-specific background needed to decide what may move or bifurcate. Viewport facts supply geometry only. The state tool will later render this rule; this engine slice does not edit the state-tool repository.

## Sentence-Level Evidence

Extend `docs/fractal_descriptive_catalog_evidence.v1.json` so every exported sentence maps to one or more accepted records containing:

```text
claim_id
selector
description_field
sentence_index
claim_text
source_file
source_symbol
classification: direct | derived | editorial_paraphrase
review_disposition: accepted | rejected | superseded
```

Current engine source and tests outrank historical prose. Historical text may seed wording but cannot establish current truth. The ledger remains build/test authority only. Public `source_refs` derive from accepted mappings and remain repository-relative.

The review must cover all live selectors, including direct escape/root families, every ExplainO/composed variant, analysis lanes, generic equation packs, SDF lanes, and root-field consumers. Descriptions for programmable lanes must describe their substrate and authority boundary rather than pretending one fixed user-authored formula exists.

## Mutation Surface

Allowed:

- compiled descriptive metadata and reusable family templates;
- the tracked sentence evidence ledger and historical audit updates;
- a central deterministic viewport-facts computation/serializer;
- headless CLI and finding-capture exposure of that module;
- focused tests, build wiring, docs, receipts, and publication proof.

Forbidden without a revised user-approved engine plan:

- fractal formulas, samplers, termination behavior, rendering pixels, or camera behavior;
- state JSON serialization/deserialization semantics;
- parameter applicability or UI schema changes;
- Color Pipeline behavior;
- selector, family, capability, or runtime-flag taxonomy changes;
- automatic feature detection, camera movement, or aesthetic scoring;
- runtime dependency on docs, the ledger, or the historical side folder.

## TDD, Dirty Experiments, And Rollback

- Each behavioral phase starts with a focused failing native or runtime test before implementation.
- Implement the minimum change that closes the current RED, then refactor only covered seams.
- One family batch may be dirty at a time; it must end in a coherent checkpoint before the next batch.
- If current source cannot safely ground a sentence, record the unresolved claim and stop that batch rather than laundering historical prose into runtime authority.
- If exact viewport facts require changing renderer mapping or state semantics, stop for plan revision.
- Rollback uses the last coherent branch checkpoint through approved wrappers; never reset or overwrite unrelated work.

## Validation And Closure

Run every current repository-mandated rail plus the commands locked in `docs/contracts/fractal_catalog_viewport_continuity_v1.contract.json`. Minimum proof includes:

- contract validation and phased-plan sync;
- focused descriptive-catalog and viewport-facts native tests;
- CLI parse/conflict tests;
- deterministic stdout/file/capture-sidecar byte proof;
- all-selector reviewed/evidence coverage with zero unavailable live rows;
- renderer-mapping parity tests for unrotated, rotated, rectangular, high-zoom, and invalid states;
- full native helper suite;
- runtime build/publish and published-runtime pytest;
- representative finding capture proof;
- code-quality baseline, diff check, hostile audit, commit, receipts, rearward review, push, and clean tree.

The engine feature stops at a clean pushed pull request. This plan and the overnight goal do not authorize merging that pull request. State-tool integration is blocked until the user separately approves the engine merge and exact merged-master runtime publication is complete.

## Proof Ledger

- Repository/bootstrap/status: clean `master` at `337d66c8e2571f59b5757a27497ebe38717e30df`, equal to `origin/master`.
- Rearward review: current-HEAD artifact reports `ok`.
- Live catalog count: 51 rows observed from `kFractalCatalog`; completion is mechanically tied to the live count, not hard-coded to 51.
- Existing reviewed coverage: 2 reviewed, 49 unavailable before this campaign.
- Renderer mapping source: `ui_app/src/fractal_renderer.cu::fractal_kernel` lines implementing aspect, `exp2(log2_zoom)`, pixel-center normalization, center, and rotation.
- Historical audit: existing V1 audit records 48 historical entries, one `lambda_map -> lambda` editorial alias, and three newer unseeded live rows; all claims require current-source reconfirmation.
- RED receipts: pending.
- Focused GREEN receipts: pending.
- Full validation receipts: pending.
- PR and clean-tree proof: pending.
- Slice lock: `ck:ae7df1e6`; `fractal_catalog_viewport_continuity_v1` is the active locked contract.
- Phase 0 proof: contract validator, phased-plan sync, 43 workflow-tool tests, and `git diff --check` passed.

## Hostile Audit

- Status: complete

Audit questions:

- Did any description copy stale historical claims without current source proof?
- Are recurrence, coloring, and field-consumer semantics kept distinct?
- Can a live selector still export `unavailable`, duplicate another identity, or bypass evidence coverage?
- Can renderer mapping and viewport facts drift independently?
- Are rotation, aspect, pixel centers, continuous edges, and high-zoom precision all represented honestly?
- Does the runtime load docs or historical evidence?
- Did the slice mutate formulas, pixels, camera behavior, state contracts, or Color Pipeline behavior?
- Did any checkpoint claim publication, integration, or merge authority it did not have?

## Audit Passes

- [x] Pass 1 - hostile contract and authority audit found the inherited-contract successor bootstrap gap and confirmed exact clean master, current rearward approval, live catalog ownership, and renderer mapping ownership.
- [x] Pass 2 - re-read the repaired state and confirmed the contract excludes formulas, rendering behavior, state semantics, parameter applicability, Color Pipeline behavior, taxonomy changes, and state-tool edits.
- [x] Pass 3 - a second clean re-read found no additional workflow mistake; the inherited contract is restored byte-for-byte and the new contract is locked.

## Audit Findings

- [x] The inherited active contract did not authorize creation of its successor plan/contract. A minimal contract-only bootstrap temporarily authorized the two exact successor paths, the successor contract was locked, and the inherited file was restored with no lingering diff.
- [x] The plan ties completion to live catalog coverage rather than treating the preflight count of 51 as a permanent hard-coded taxonomy.

## Notes

- The state-tool repository must remain clean and untouched throughout engine work.
- After a separately authorized engine merge, the next preplanned cross-repository phase is exact merged-master runtime publication followed by state-tool Packet V6 integration and representative high-zoom manual review.
