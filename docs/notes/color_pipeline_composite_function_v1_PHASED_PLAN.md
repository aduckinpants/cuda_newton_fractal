# Color Pipeline Composite Function V1 Campaign

## Explicit User Asks

- [x] Preserve the full composite-function design and deferred boundary in checked-in repo truth before implementation.
- [x] Add a real reusable composite-function contract, not a one-off preset or pseudo-primitive.
- [x] Route every public Color Pipeline apply path through one prepared, atomic draft transaction.
- [x] Keep state replay backward compatible by persisting expanded primitive rows as authority.
- [x] Ship and qualify one Shape composite, `unit_contours_v1`, with exact parity against its manual primitive stack.
- [x] Use actual public no-mouse function-picker and parameter controls for runtime proof.
- [x] Harden and stop before more composites, adapters, graph UI, or user-authored packs; checkpoint/push are the remaining closure mechanics.

## Current Phase

Campaign implementation and validation are complete. `unit_contours_v1` is materialized through the separate composite contract, every public draft path uses generic preparation, replay authority remains expanded primitives, and the fresh published runtime passes exact manual-stack parity plus existing recipe/function preservation. Preplanned sliced work is exhausted; stop for replan before more product mutation.

## Phase Checklist

- [x] Phase 0 - lock plan/contract, restore the bootstrap bridge, truth-sync shipped work, and freeze deterministic primitive baseline evidence.
- [x] Slice A - materialize and validate `viewer.composite_function_contract.v1`.
- [x] Slice B - route all public application paths through generic Resolve/Prepare/Commit authority.
- [x] Slice C - add backward-compatible wrapper projection, load flattening, and truthful active/draft/composite receipts.
- [x] Slice D - implement and qualify `unit_contours_v1` through the public UI path.
- [x] Hardening - distrust-first review, regression repairs, focused/full native proof, and fresh published-runtime proof.
- [x] Closure mechanics - validators, receipts, checkpoint, push, rearward review, and clean tree.
- [x] Stop/replan - no additional composite, adapter, graph UI, or pack work under this campaign.

## Accepted Base And Entry Gate

- Accepted base: clean rearward-reviewed `7c4c0a7c86ff1dec690976cdcb8943e9ae6da5ba`.
- The preceding function-expansion campaign is merged and pushed before this contract becomes active.
- Existing typed ports, resolver routes, recipe application, state IO, capture/replay, and public no-mouse function controls are prerequisite authority.
- A shared authority or binding defect stops the campaign for bounded repair.
- A composite-specific defect blocks that composite gate without invalidating already-proven shared slices.

## Contract Locks

### Composite Identity

- Composite functions live in a separate materialized contract with schema id `viewer.composite_function_contract.v1`.
- They are not primitive function descriptors and do not inflate or collide with the primitive function catalog.
- V1 topology is `lane_local_function`, Shape lane only, one typed input, one typed output, linear acyclic primitive-only internals.
- V1 forbids nesting, adapters, cross-lane edges, arbitrary branches, and user-authored runtime packs.
- Each composite and internal node has an explicit semantic id. Position-derived ids are forbidden.
- One composite wrapper per lane is supported in V1.
- Fully expanded primitive rows, including neighboring primitive rows, must not exceed the existing lane capacity of eight.

### Type And Output Guarantees

- Every internal primitive must have known typed ports and an explicit runtime output-domain guarantee.
- The V1 pilot uses only finite `scalar.unit -> scalar.unit` primitives.
- `offset_scale` is excluded because its current output-domain guarantee is ambiguous and can exceed unit range.
- No implicit or explicit adapter participates in V1.

### Parameter Addressing

- Exposed parameter ids and primitive `descriptor_parameter_id` values are unique and version-stable within their descriptor contracts.
- Every internal parameter is either mapped exactly once or fixed explicitly.
- Mapping formula is:
  `target = clamp(exposed_value * scale + offset, optional_min, optional_max)`.
- Unknown targets, duplicate mappings, unmapped parameters, nonfinite values, invalid ranges, or inconsistent defaults fail materialization.

### Canonical Hash

- Composite fingerprints use SHA-256 after legacy alias normalization, explicit default expansion, author-order node preservation, parameter-id sorting inside each node, and typed-value normalization.
- Binary64 values use deterministic `.17g` text normalization.
- Display labels, tooltips, and other display-only text are excluded.
- Primitive descriptor fingerprints are included so dependency drift invalidates wrapper reconstruction.

## Phase 0 - Baseline Lock And Truth Sync

Before changing application or metadata authority:

1. Restore the temporary prior-contract bootstrap widening after the new contract is active.
2. Mark the function-expansion campaign shipped in `spec_intake/_STATUS.md`.
3. Freeze the manual primitive reference:
   - Source: `smooth_escape_ramp`
   - Shape rows: `repeat`, then `smooth_window`
   - Palette: `gradient_three_stop_v1`
   - Grading: `levels_gamma_v1`
4. Record materialized rows and params, state JSON, graph/application receipt, frame hash, backend, precision, and bounded timing.
5. Use a deterministic small scene and published runtime so later exact parity is meaningful.

The baseline witness is evidence, not a new authority format.

## Slice A - Composite Metadata Contract

Add viewer-local UI-Salt authoring and deterministic materialization for composite descriptors.

Required validation:

- Reject duplicate composite/node/parameter ids.
- Reject primitive id collisions.
- Reject unknown primitive functions, lane mismatches, cycles, branches, nesting, adapters, unknown types, missing output guarantees, over-capacity expansion, and incomplete parameter maps.
- Canonicalize aliases before hashing.
- Prove deterministic materialization and hash stability.
- Keep visible UI and runtime behavior unchanged in Slice A.

## Slice B - Generic Prepared Draft Transaction

Introduce one generic application seam used by:

- ordinary row-editor auto-apply;
- recipe Apply;
- loaded-draft Apply;
- headless/no-mouse application;
- composite Apply.

Required API behavior:

1. Resolve projection into candidate primitive rows.
2. Expand composite wrappers in author order.
3. Validate types, mappings, capabilities, routes, finite values, and expanded capacity.
4. Apply only to cloned params/window state.
5. Construct active, draft, and composite receipts from the prepared object.
6. Commit through non-failing host-state replacement only.

All allocation, validation, receipt construction, and callbacks occur before the first authoritative mutation. Commit performs only pre-owned `noexcept` swaps/assignments plus dirty/interacted signaling.

Failed preparation may update rejection diagnostics after failure, but must not mutate live params, dirty/frame state, generation counters, active receipts, or interaction state.

Add test-only fault injection after Resolve and before Commit. It is not persisted, not user-visible, and not a production fallback.

## Slice C - Projection, State, And Receipt Authority

- `state.json.color_pipeline_draft` remains expanded primitive-row replay authority.
- Add optional `color_pipeline_composite_projection` containing wrapper id/version/hash/params, stable wrapper row id, expanded row ids, and expansion fingerprint.
- Keep the current state version.
- On load, reconstruct a wrapper only when the installed contract and expanded rows match exactly.
- If the contract is missing or mismatched, load/replay the primitive rows and flatten with a warning; never fail replay solely because a wrapper is unavailable.
- `Reset From Current Color` clears wrapper provenance and produces primitive editor rows.
- Dirty comparison uses canonical expanded-row fingerprints.

Receipt split:

- Active execution receipt is built only from committed expanded runtime rows and has a new explicit schema version.
- Draft receipt describes current editor projection and can truthfully report a rejected draft without claiming it executed.
- `viewer.color_pipeline_composite_receipt.v1` maps wrapper identity to internal nodes, parameter mappings, expanded rows, contract hashes, status, and committed generation.
- Regression lock: after Applied A and rejected Draft B, active receipt remains A while draft/rejection receipt describes B.

## Slice D - unit_contours_v1

Add one Shape composite:

```text
unit_contours_v1
  repeat
  -> smooth_window
```

Exposed parameters:

| Parameter | Range | Default |
| --- | ---: | ---: |
| `composite.frequency` | `0.25..24` | `6` |
| `composite.offset` | `-1..1` | `0.10` |
| `composite.window_width` | `0.01..0.95` | `0.35` |
| `composite.softness` | `0..0.5` | `0.08` |

Mappings:

- `composite.frequency -> repeat/shape.frequency`
- `composite.offset -> repeat/shape.phase`
- `composite.window_width -> smooth_window/shape.width`
- `composite.softness -> smooth_window/shape.softness`
- `smooth_window/shape.center = 0.5` fixed

The normal UI picker shows `Unit Contours` as one Shape function with four wrapper parameters. Runtime execution remains the two expanded primitive rows.

## Exact Parity Standard

- Same deterministic scene, dimensions, backend, precision, frame/tick, warm-up, and capture format.
- Existing primitive reference and composite expansion must produce bit-identical frame bytes and frame hash.
- Materialized primitive rows and normalized parameters must match exactly.
- Any tolerance requires identifying and approving the exact nondeterministic operation first; no tolerance is pre-authorized.

## Test Plan

### Native

- Materializer accepts the locked composite and rejects every invalid contract class above.
- Composite/manual expansion and pixel evaluation are exact.
- Total expanded Shape count eight succeeds; nine rejects atomically.
- Disabled wrappers are inactive and neighboring primitive order is stable.
- Every public application path uses generic preparation.
- Fault injection and route/capacity failures leave authoritative state untouched.
- Old state loads unchanged; new state round-trips projection; missing contract flattens without replay drift.
- Active/draft/composite receipts remain truthful across success, rejection, reset, load, and replay.
- Existing recipes, all 39 pre-batch functions, the ten-function batch, SDF/root-aware routes, and capture/state rails remain green.

### Runtime / No Mouse

- Publish the viewer.
- Use the actual Shape function picker to select `unit_contours_v1`.
- Set every wrapper parameter through public controls and prove sensitivity.
- Prove selector identity and wrapper report identity.
- Compare against the manual primitive reference with exact frame hash.
- Capture Finding, reload `state.json`, and replay with matching pixels.
- Prove fallback flattening by replaying with wrapper metadata unavailable in a focused test-only runtime fixture.
- No physical mouse automation.

## Hostile Audit

- Status: complete
- Outcome: multiple real authority, state, tooling, and public-control defects were regressed and repaired; final full-native and published-runtime rails are green.

The closeout audit must answer:

- Did any public Apply path bypass generic preparation?
- Can a failed preparation change active rows, frame, generations, or active receipt?
- Can a wrapper leak into replay-authority rows?
- Can active receipts describe draft state?
- Can stale primitive metadata reconstruct a wrapper?
- Can expansion exceed runtime capacity after neighboring rows are counted?
- Did any implicit adapter or unsafe output-domain claim enter V1?
- Did visible UI controls remain inert or map to the wrong internal parameter?
- Did existing recipes, SDF, root-aware, capture, or replay behavior regress?

## Audit Passes

- [x] Pass 1 - reviewed each landed slice as suspect; found capacity, wrapper-count, grouped-link, and same-file patch defects.
- [x] Pass 2 - found headless metadata initialization/lookup bypasses, stale catalog expectations, and prepared-commit pointer invalidation; added regressions and repaired them.
- [x] Pass 3 - re-read the repaired state after the state-key/hash-domain and composite-parameter visibility repairs; no additional real defect found, and focused, full-native, and fresh published-runtime rails are green.

## Audit Findings

- [x] Slice A finding: invalid-fixture tests could pass after a no-op source mutation because they asserted only process failure. The tests now prove each fixture changes and reject traceback-based unrelated failures.
- [x] Capacity was initially applied to every primitive lane rather than only composite-expanded lanes; focused capacity tests now preserve existing primitive behavior while bounding wrapper expansion.
- [x] Loader and editor disagreed about the one-wrapper-per-lane V1 lock; both now reject duplicate wrappers consistently.
- [x] The grouped native test topology omitted the metadata parser needed by new headless authority; build linkage and the full helper rail now cover it.
- [x] A same-file runtime-test patch accidentally displaced earlier assertions; the final test retains baseline, capture/replay, public picker, parameter sensitivity, and receipt checks together.
- [x] Headless dispatch occurred before materialized metadata initialization and headless `set_param` resolved primitives only; startup ordering and composite descriptor lookup now use the same authority as the viewer.
- [x] Schema-binding tests encoded stale pre-batch catalog counts/order; they now assert the full current materialized catalog.
- [x] Prepared commit replaced row storage and invalidated active numeric-control pointers between drag frames; preparation now preserves compatible draft storage and regression tests prove pointer stability across repeated edits.
- [x] Runtime proof initially read the wrong projection array key and compared SHA-256 headless hashes with FNV-1a live-view hashes; proof now compares like-for-like hash domains and the actual `items` projection schema.
- [x] Final hostile finding: the row parameter collector bypassed the composite-aware visibility predicate, leaving all wrapper parameters inert/invisible despite valid metadata. It now uses lane-aware composite authority; native coverage and public no-mouse edits prove all four controls are reachable and active.
- [x] Third pass found no further product defect. `git diff --check`, full native helpers, exact staged metadata hash, and the fresh 11-case published-runtime matrix are green.

## Proof Ledger

- [x] Contract validates.
- [x] Plan sync passes.
- [x] Code-quality baseline passes at `93/100`.
- [x] Baseline primitive witness is frozen from the exact-head published runtime.
- [x] Composite materializer tests pass (`57 passed`).
- [x] Core (`4466`), window (`587`), schema/binding, state, capture, and report focused native rails pass.
- [x] Full native helper rail passes from current source in `2106.758s`; rebuilt `test_headless_modes` reports `391 passed, 0 failed`.
- [x] Runtime publishes successfully from final product source in `1295.823s`.
- [x] Staged standalone composite contract SHA-256 exactly matches source (`38E94FE142AF477FCA81C5140B6FA21BB3A32D68AC1281773AE0C3061334A316`).
- [x] Public no-mouse composite proof passes with exact manual primitive-stack parity and all four wrapper controls active.
- [x] Existing recipe/function preservation runtime matrix passes (`11 passed`).
- [x] Hostile audit validator passes with real findings and a clean re-audit.
- [x] Validation and contract proof receipts are written during checkpoint closure.
- [x] Rearward review is required to be `ok` before final closeout.
- [x] Branch is pushed and tree is clean before final closeout.

## Deferred Boundary

After `unit_contours_v1` and campaign hardening, preplanned sliced work is exhausted. Stop for replan before:

- `phase_ribbons` or additional composites;
- cross-lane materials or composite nesting;
- new adapters or output-domain repair for `offset_scale`;
- graph editor UI;
- user-authored composite packs;
- Salticid runtime execution.
