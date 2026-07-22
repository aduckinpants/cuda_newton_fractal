# Fractal Catalog And Viewport Continuity V1 Phased Plan

## Current Phase

Implementation complete - closure checkpoint, push, and ready PR stop at the separate engine merge-approval boundary.

## Phase Checklist

- [x] Phase 0 - locked this engine-native plan and contract from exact clean master.
- [x] Phase 1 - add deterministic engine-owned viewport facts with focused RED/GREEN proof.
- [x] Phase 2 - review and ground the direct native fractal families.
- [x] Phase 3 - review and ground the ExplainO and composed-analysis families.
- [x] Phase 4 - review and ground custom, SDF, and root-field consumer selectors; prove zero unavailable rows.
- [x] Phase 5 - publish, run all mandatory rails, complete hostile audit, checkpoint, push, and stop at the engine merge-approval boundary.

## Explicit User Asks

- [x] Cover every shipped fractal selector, not only McMullen.
- [x] Give downstream exploratory agents enough engine-owned mathematical context to reason about dynamics changes without treating every serialized value as active.
- [x] Add exact engine-owned viewport geometry so high-zoom dynamics changes can be paired with deliberate same-window, feature-tracking, or transition-survey camera intent.
- [x] Keep the work central, reusable, and API-surfaced rather than adding a one-off state-tool workaround.
- [x] Advance to the furthest clean review-ready checkpoint allowed overnight, but do not merge the CUDA engine pull request without separate user approval.

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
- Viewport RED receipts: `artifacts/validation/fractal_viewport_facts_red.json`, `fractal_viewport_cli_red.json`, and `fractal_viewport_finding_sidecar_red.json` record the expected pre-implementation failures.
- Focused GREEN receipts: `artifacts/validation/fractal_viewport_facts_green.json`, `fractal_viewport_cli_green.json`, `fractal_viewport_finding_sidecar_green.json`, `fractal_viewport_shared_mapping.json`, `fractal_viewport_phase1_repairs_fast.json`, and `fractal_viewport_renderer_exact_parity.json` cover the module, CLI, archive sidecar, and renderer-resident mapping.
- Full Phase 1 validation: `artifacts/validation/fractal_viewport_phase1_full_native_final.json`, `fractal_viewport_phase1_publish.json`, and `fractal_viewport_phase1_runtime.json` prove the completed native suite, published build, and published-runtime CLI.
- Phase 1 checkpoint: `598c1b9` records the renderer-resident viewport mapping, deterministic export, finding archive sidecar, and exact proof rails.
- Phase 2 RED/GREEN: `artifacts/validation/fractal_catalog_direct_families_red.json` records the expected unavailable-selector failure; `fractal_catalog_direct_families_green.json` proves the bounded direct-family coverage and sentence-ledger joins.
- Phase 2 coverage: 18 live selectors are now reviewed in catalog order, including the two pre-existing reviewed entries; 80 newly accepted sentence claims were checked against current repository-relative source symbols.
- Phase 2 full and published-runtime proof: `fractal_catalog_viewport_full_native.json`, `fractal_catalog_viewport_runtime_publish.json`, and `fractal_catalog_viewport_runtime_pytest.json` prove the full native suite, canonical runtime publish, and 8/8 runtime catalog/viewport checks.
- Phase 2 checkpoint: `8e1e813` records direct-family catalog coverage, current-source evidence, full native proof, and published-runtime verification.
- Phase 3 RED/GREEN: `artifacts/validation/fractal_catalog_explaino_composed_red.json` records the expected unavailable-selector failure; `fractal_catalog_explaino_composed_green.json` proves the 46-selector ExplainO/composed coverage and sensitive semantic assertions.
- Phase 3 coverage: 232 accepted claims cover 46 selectors; all 220 newly generated claims across 44 selectors resolve to current repository-relative source files and symbols.
- Phase 3 full and published-runtime proof: `fractal_catalog_viewport_full_native.json`, `fractal_catalog_viewport_runtime_publish.json`, and `fractal_catalog_viewport_runtime_pytest.json` prove the full native suite, canonical runtime publish, and 8/8 runtime checks with the exact 46-entry reviewed subset.
- Phase 3 checkpoint: `c613cf2` records 46 reviewed selectors, ExplainO/composed current-source evidence, full native proof, and published-runtime verification.
- Phase 4 RED/GREEN: `artifacts/validation/fractal_catalog_special_families_red.json` records the expected unavailable-selector failure; `fractal_catalog_special_families_green.json` proves all 51 live selectors reviewed with special-family authority-boundary assertions.
- Phase 4 coverage: 258 ledger records cover the complete live catalog; all 25 claims for the five newly reviewed custom/SDF/root-field selectors resolve to current source files and symbols.
- Phase 4 full and published-runtime proof: `fractal_catalog_viewport_full_native.json`, `fractal_catalog_viewport_runtime_publish.json`, and `fractal_catalog_viewport_runtime_pytest.json` prove the full native suite, canonical runtime publish, and 8/8 runtime checks with zero unavailable live rows.
- Phase 4 checkpoint: `a6f3c48d49ee0295c7b9f3aa560661d55d6cf6aa` is the exact all-selector product commit used for final publication.
- Final published executable: `D:\\salt-fractal\\cuda_newton_fractal_clone\\runtime\\fractal_ui.exe`, SHA-256 `6ac9ae7987942112c96986b96e7bafef489ab196ac6addb8f4ac9fad2b343a8e`.
- Final deterministic catalog: SHA-256 `8038ab867cd40dd4af6ca5b26aca11cd5e7c6b6a28816b00f7d4afdb4a4909fd`; 51 live entries, 51 reviewed, zero unavailable.
- Ready PR creation follows the final closure commit; merge remains explicitly unauthorized until separate user approval.
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

- [x] Pass 1 - hostile current-source and sentence-ledger audit rejected reuse of the generator's duplicate declaration block and confirmed each special selector's runtime authority boundary.
- [x] Pass 2 - re-read the repaired state after the first real Phase 4 finding; all five new selector claims resolve and zero live rows remain unavailable.
- [x] Pass 3 - no additional workflow mistake found in the final clean re-audit after full native and published-runtime proof.

## Audit Findings

- [x] The inherited active contract did not authorize creation of its successor plan/contract. A minimal contract-only bootstrap temporarily authorized the two exact successor paths, the successor contract was locked, and the inherited file was restored with no lingering diff.
- [x] The plan ties completion to live catalog coverage rather than treating the preflight count of 51 as a permanent hard-coded taxonomy.
- [x] The first capture integration wrote viewport facts after the archive script, which could leave a published finding whose metadata omitted the required sidecar. The repair now generates the exact bytes before archival and passes them through the established archive transaction, metadata, and readable artifact list.
- [x] The first shared mapping refactor used a higher-precision degree conversion and removed the renderer's center-add/subtract operation order. Although mathematically cleaner, both could change rotated or deep-zoom pixels. The final shared mapping preserves the historical `CUDART_PI_F / 180.0f` value, rotation branch, and arithmetic order exactly; the renderer-resident rail remains green.
- [x] The active contract initially omitted the existing Reality Toolkit archive paths required by the accepted finding-sidecar contract. The contract was revised narrowly to include `tools/reality_toolkit`, revalidated, and explicitly re-locked before those paths changed.
- [x] Historical prose initially misbound Collatz to the McMullen step symbol, described Spider as second-order rather than first-order in augmented `(z,c)` state, and overstated termination behavior. Current source and tests replaced those claims before they entered the compiled overlay.
- [x] Julia now names the configured serialized constant, Magnet names its unit-attractor epsilon-squared residual, and McMullen retains the negative-power term; direct-family wording does not infer visible contribution from configuration alone.
- [x] The first published-runtime Phase 2 run exposed a stale Python expectation for only the two pre-campaign reviewed entries. The repaired test now asserts all 18 reviewed selectors in exact live catalog order and passes 8/8 against the published executable.
- [x] The first attempt to repair that Python expectation bypassed the required patch wrapper. It was caught immediately, reverted before testing, and reapplied through `viewer_host_apply_repo_patch.py`; the final diff contains only wrapper-authorized mutation.
- [x] Existing `explaino_all` and `explaino_magnet_root_well` prose remained byte-for-byte unchanged while the new direct-family overlay was added; the runtime still has no dependency on the evidence ledger or historical files.
- [x] The first Phase 3 implementation patch duplicated the generated declaration include and failed compilation with redefined arrays. The duplicate block was removed through the authorized wrapper before GREEN; each generated selector now has one declaration and one reviewed-table row.
- [x] ExplainO Phoenix and Joy describe memory conditionally, Balance Void remains a dedicated first-order branch, Rational Escape preserves the denominator clamp from 1 through 6, and Spider remains first-order in augmented `(z,c)` state.
- [x] Counterfactual Pair describes different-root basin-swap classification without spatially localizing the synthetic result; Projection and Flow distinguishes peak/final transient pressure from a spatial force field.
- [x] The published-runtime test now asserts all 46 reviewed selectors in exact live catalog order and remains byte-deterministic across repeated stdout and file exports.
- [x] The Phase 4 generator output still contained the previously identified duplicate declaration include. The slice intentionally applied only the new compiled rows and ledger evidence, reusing the single reviewed-overlay declaration seam already in source.
- [x] Generic Equation Pack and SDF Pack Scene describe programmable/field substrates without inventing fixed formulas; exact loaded pack or scene authority remains required for concrete claims.
- [x] ExplainO Root SDF names the current root-circle, bridge-capsule, and phase-sine field construction without claiming an orbit recurrence or visible symmetry from serialized roots alone.
- [x] Mandelbrot and Multibrot root-trap selectors distinguish their base orbit dynamics from downstream root-field coloring, and continuous root proximity is not promoted into discrete basin evidence.
- [x] The published runtime now exports all 51 live catalog rows as reviewed in exact `kFractalCatalog` order, with zero unavailable shipped selectors and unchanged V1 schema/flag taxonomy.
- [x] Final closure reran focused catalog/viewport/CLI rails, the complete native helper suite, canonical publication, and the 8/8 published-runtime lane from the exact all-selector product checkpoint.
- [x] Runtime and catalog hashes were captured independently so the state-tool handoff can bind both executable identity and deterministic catalog content after the authorized merge.

## Notes

- The state-tool repository must remain clean and untouched throughout engine work.
- After a separately authorized engine merge, the next preplanned cross-repository phase is exact merged-master runtime publication followed by state-tool Packet V6 integration and representative high-zoom manual review.
