# SDF Next Five Work Campaign

## Current Phase

Closed - planning slice checkpointed, receipt-backed, pushed, and ready for Step 1 execution.

## Phase Checklist

- [x] Phase 1 - create and lock the docs-only planning contract and phased plan.
- [x] Phase 2 - draft bounded slice plans through Step 5.
- [x] Phase 3 - hostile-review the slice order, proof gates, and Step 6 replan boundary.
- [x] Phase 4 - validate contract, plan sync, hostile audit, code quality, and diff hygiene.
- [x] Phase 5 - checkpoint the planning slice.

## Explicit User Asks

- [done] Plan out the next work in detail as bounded slices through the end of Step 5.
- [done] Treat Step 6 as the point where work should exhaust and replan instead of being pre-expanded.
- [done] Keep this as planning work; do not implement product changes under this planning contract.

## Scope

In scope:

- A concrete Step 1-5 campaign plan that future implementation sessions can execute one bounded slice at a time.
- Explicit proof gates and stop conditions for every step.
- A Step 6 replan boundary that prevents guessing beyond the next proven product state.

Out of scope:

- Product code changes.
- Merge-to-master execution.
- Runtime publish or no-mouse runtime proof.
- SDF-native fractal lane implementation.

## Campaign Rules

- Each implementation step below gets its own branch from the current `master`, its own checked-in phased plan, its own checked-in contract, hostile audit, receipts, push, and clean-tree closeout.
- Merge the previous step to `master` before opening the next step unless the step explicitly says it is docs-only and can safely remain on a planning branch.
- Any step that changes live viewer behavior must use the published runtime and no-mouse harness. Physical cursor automation remains forbidden.
- Any hot-path or performance-sensitive step must carry before/after numbers from the same witness shape. Qualitative "feels faster" text is not closure evidence.
- Full-quality capture/replay remains deterministic. Interactive preview may degrade quality only when the step explicitly owns that policy and proves settled/capture paths return to requested quality.
- The first SDF-native fractal lane is Step 6 material. Do not start it under Steps 1-5.

## Future Slice Artifact Names

Future sessions should create these checked-in artifacts before mutating each step. Step 3 and Step 5 are intentionally split into sub-slices because they can expose real blockers that should checkpoint cleanly instead of forcing one oversized change.

- Step 1 merge slice:
  - Plan: `docs/notes/sdf_postprocess_merge_to_master_PHASED_PLAN.md`
  - Contract: `docs/contracts/sdf_postprocess_merge_to_master.contract.json`
  - Branch: `master` after preflight from `codex/sdf-postprocess-measured-optimization`
- Step 2 roadmap truth sync:
  - Plan: `docs/notes/sdf_postprocess_roadmap_truth_sync_PHASED_PLAN.md`
  - Contract: `docs/contracts/sdf_postprocess_roadmap_truth_sync.contract.json`
  - Branch: `codex/sdf-postprocess-roadmap-truth-sync`
- Step 3A per-row field authority design and RED matrix:
  - Plan: `docs/notes/sdf_row_field_resolution_authority_reds_PHASED_PLAN.md`
  - Contract: `docs/contracts/sdf_row_field_resolution_authority_reds.contract.json`
  - Branch: `codex/sdf-row-field-resolution-authority-reds`
- Step 3B multi-field producer and cache:
  - Plan: `docs/notes/sdf_multi_field_producer_cache_PHASED_PLAN.md`
  - Contract: `docs/contracts/sdf_multi_field_producer_cache.contract.json`
  - Branch: `codex/sdf-multi-field-producer-cache`
- Step 3C row-local downsample UI/runtime productization:
  - Plan: `docs/notes/sdf_row_field_resolution_ui_runtime_PHASED_PLAN.md`
  - Contract: `docs/contracts/sdf_row_field_resolution_ui_runtime.contract.json`
  - Branch: `codex/sdf-row-field-resolution-ui-runtime`
  - Status: shipped; SDF Source rows expose row-local `Field Downsample` while `Inherit` preserves the shared `LensSettings::downsample` authority.
- Step 4 phase-safe normal-angle UX:
  - Plan: `docs/notes/sdf_normal_angle_beauty_phase_ux_PHASED_PLAN.md`
  - Contract: `docs/contracts/sdf_normal_angle_beauty_phase_ux.contract.json`
  - Branch: `codex/sdf-normal-angle-beauty-phase-ux`
  - Status: shipped on this branch; full-field SDF Normal Angle Diagnostic stays available and SDF Normal Angle Beauty applies boundary-band gating by default.
- Step 5A authored SDF pack field producer bridge:
  - Plan: `docs/notes/authored_sdf_pack_field_producer_bridge_PHASED_PLAN.md`
  - Contract: `docs/contracts/authored_sdf_pack_field_producer_bridge.contract.json`
  - Branch: `codex/authored-sdf-pack-field-producer-bridge`
  - Status: implemented on this branch; adds CPU/CUDA authored-pack field production into the shared SDF field surface without viewer UI, Color Pipeline consumption, or SDF-native lane changes.
- Step 5B authored SDF pack viewer UI integration:
  - Plan: `docs/notes/authored_sdf_pack_viewer_ui_integration_PHASED_PLAN.md`
  - Contract: `docs/contracts/authored_sdf_pack_viewer_ui_integration.contract.json`
  - Branch: `codex/authored-sdf-pack-viewer-ui-integration`
- Step 5C authored SDF pack Color Pipeline and overlay consumption:
  - Plan: `docs/notes/authored_sdf_pack_color_pipeline_overlay_PHASED_PLAN.md`
  - Contract: `docs/contracts/authored_sdf_pack_color_pipeline_overlay.contract.json`
  - Branch: `codex/authored-sdf-pack-color-pipeline-overlay`
- Step 6:
  - No implementation plan or contract is predeclared here. Step 6 must start from a fresh evidence review and replan.

## Step 1 - Merge Measured Postprocess Optimization To Master

Purpose:

- Move `codex/sdf-postprocess-measured-optimization` at `7ed70d6` onto `master` so the measured CUDA postprocess reuse work becomes the base for the next campaign.

Branch:

- Use `master`.
- Source branch: `codex/sdf-postprocess-measured-optimization`.

Bounded phases:

1. Preflight:
   - Verify `codex/sdf-postprocess-measured-optimization` is clean, pushed, and has rearward review `ok`.
   - Check out `master`, pull/reconcile origin, and verify whether `master` already contains `7ed70d6`.
   - If `master` has unrelated local or remote changes, stop and review before merging.
2. Merge:
   - Merge `codex/sdf-postprocess-measured-optimization` into `master`.
   - Do not squash unless the user explicitly asks; preserve the receipt-backed slice commit.
3. Focused merge validation:
   - Run contract/plan sync if touched by merge.
   - Run a lightweight diff/log check that `7ed70d6` is reachable from `master`.
   - If the merge is non-fast-forward or conflictful, run the focused SDF postprocess native and no-mouse rails before pushing.
4. Closeout:
   - Push `master`.
   - Confirm `master` clean and not ahead of origin.

Required proof:

- `git branch --contains 7ed70d6 master` or equivalent commit reachability proof.
- `git status --short --branch` showing clean `master` tracking origin with no local ahead state.
- Rearward review `ok` on the final `master` head after merge/push.

Stop conditions:

- Merge conflicts touching SDF postprocess, Color Pipeline, runtime reports, or pacing tests.
- `master` has unrelated local changes.
- Rearward review for `7ed70d6` is not `ok`.

## Step 2 - Post-Merge SDF Roadmap Truth Sync

Purpose:

- Repair stale roadmap/status text after Step 1 so the repo no longer says the current representative witness still "points back to postprocess review" as future work.
- Historically, Step 2 declared per-row/multi-field SDF downsample authority as the next active design choice; that Step 3 work is now shipped through `docs/notes/sdf_row_field_resolution_ui_runtime_PHASED_PLAN.md`.

Suggested branch:

- `codex/sdf-postprocess-roadmap-truth-sync`

Bounded phases:

1. Open a docs-only plan/contract for the truth-sync slice.
2. Search for stale postprocess-review wording in:
   - `spec_intake/_STATUS.md`
   - `DEFERRED_THREADS.md`
   - `KNOWN_ISSUES.md`
   - `docs/notes/sdf_field_pack_near_term_TODO.md`
   - any active SDF performance roadmap plans that still describe postprocess review as pending.
3. Update only roadmap/status truth:
   - mark repeated median SDF witness reporting shipped;
   - mark CUDA postprocess scratch-buffer reuse shipped;
   - record final median improvement range from Step 1's committed proof;
   - keep the witness recommendation wording honest if it remains conservative;
   - set per-row/multi-field SDF downsample authority as the then-next unresolved design/product step; after Step 3C, phase-safe normal-angle UX became Step 4 and is now shipped on the dedicated branch.
4. Hostile review for stale text:
   - grep for `points back to postprocess`, `postprocess review`, `current witness`, `current stage-split witness`, `deferred until the next measured design choice`, and `current representative witness`;
   - repair only text that is stale because of `7ed70d6`.
5. Validate and checkpoint.

Required proof:

- Plan sync and hostile-audit validation.
- Code-quality baseline.
- Diff check.
- Stale-phrase grep with expected remaining contexts documented if any phrase intentionally remains.
- Clean tree, commit, receipts, rearward review, push.

Stop conditions:

- Any docs imply per-row/multi-field downsample is already shipped.
- Any docs imply SDF-native lanes or authored SDF viewport integration are shipped.
- Any stale text remains that would mislead a fresh session about the next step.

## Step 3 - Per-Row / Multi-Field SDF Downsample Authority

Purpose:

- Let SDF Source composition layers choose field resolution intentionally instead of forcing the entire stack through one shared `LensSettings::downsample`.
- Preserve current behavior by default: existing states and rows inherit the shared Lens/SDF field downsample unless a row explicitly opts into another quality.

Suggested branch:

- `codex/sdf-row-field-resolution-authority`

Recommended bounded sub-slices:

Step 3 is a mini-campaign, not a single commit target. Execute 3A, 3B, and 3C as separate bounded slices using the artifact names above. If 3A proves the model is wrong, checkpoint that result and replan before 3B.

### Step 3A - Authority Model And RED Matrix

Goal:

- Choose and document the authority model before implementation.

Recommended model to test first:

- Add a source-row-local field-resolution policy in `ColorPipelineSourceRuntimeParams`.
- Default policy: `inherit_shared`, which uses existing `LensSettings::downsample`.
- Explicit row policy: `field_downsample = 1|2|4|8|16`.
- Runtime effective field key groups SDF rows by effective downsample. The renderer may generate multiple SDF fields only for distinct row groups that are actually enabled and actually SDF-backed.
- Put a hard cap on distinct live SDF fields per frame, initially `4`, with fail-closed validation/reporting if exceeded.

REDs:

- Existing SDF rows with no row-local field setting produce identical frame hashes and state JSON.
- Two enabled SDF rows with different row field downsample values report two effective field groups.
- Disabled rows do not request fields.
- Mixed SDF/non-SDF stacks keep existing fail-closed behavior.
- Capture/replay preserves row-local field authority exactly.
- No-mouse runtime report exposes requested shared downsample, row-local effective groups, field count, and per-group timing.

Step 3A exit:

- Authority design is checked in.
- REDs prove the current system cannot express per-row field resolution.
- No product behavior changes yet beyond tests/docs.

### Step 3B - Multi-Field Producer And Cache

Goal:

- Generate/cache one SDF field per distinct effective downsample group.

Implementation seams to inspect:

- `ui_app/src/main.cpp`
- `ui_app/src/lens_sdf*`
- `ui_app/src/color_pipeline_sdf_postprocess*`
- `ui_app/src/sdf_field_signal*`
- `ui_app/src/viewer_ui_automation_report*`
- diagnostics state/capture/replay paths

Required behavior:

- Current one-field path remains the fast path when all rows inherit the same downsample.
- Field cache keys include dimensions, fractal type, camera, precision, mask-affecting params, effective downsample, and Lens semantics.
- Color-only changes reuse field groups when mask authority is unchanged.
- Capture/finding/replay use requested full-quality row fields, not interactive degraded fields.
- Interactive adaptive field resolution can raise effective downsample per group, but must report both requested and adaptive values.

Tests:

- Native field-group planner tests.
- Native cache-key tests proving color-only reuse and camera/fractal/parameter invalidation.
- Native state IO tests proving old states default cleanly and new row-local fields round-trip.
- Runtime no-mouse test proving two-row SDF stack uses distinct field resolutions and changes frame hash as expected.
- Capture/replay matrix row with two distinct SDF field resolutions.

Performance proof:

- Before/after witness rows for:
  - single inherited field;
  - two SDF rows sharing one field;
  - two SDF rows with distinct fields;
  - high-res detail row plus coarse background row.
- Report field-generation, postprocess, SDF total, and last-frame medians.
- Do not claim this improves all cases; multi-field can cost more and should be presented as composition authority plus targeted quality/performance control.

### Step 3C - UI And Runtime Productization

Goal:

- Make row-local SDF field resolution understandable and safe in the Color Pipeline Source section.

UI behavior:

- Existing shared `SDF Field Downsample` remains available as the default shared authority.
- Each SDF Source row gets an advanced/local quality control only when the row is SDF-backed.
- Label should make the relationship explicit, e.g. `Field Downsample: Inherit / 1x / 2x / 4x / 8x / 16x`.
- Non-SDF rows do not show the control.
- Do not move or redesign the entire Color Pipeline layout in this step.

Runtime proof:

- No-mouse automation can set the per-row field downsample without physical mouse input.
- Row-local field changes can alter the frame hash when source stack makes them visible.
- Selector/fractal-switch preservation keeps compatible row-local fields and clears unsupported stale state.

Step 3 full closeout:

- Focused native rails.
- Runtime publish.
- No-mouse runtime row-field proof.
- Capture/replay authority proof.
- SDF performance witness with before/after median table.
- Hostile audit must explicitly answer whether shared Lens downsample remains the default authority and whether multi-field work silently regressed SDF rows, Lens Field v2, capture finding, or pacing.

Stop conditions:

- If the required multi-field cache/key model needs a broad renderer rewrite, stop after Step 3A with a design/red proof checkpoint and replan Step 3 before continuing.
- If performance gets materially worse for inherited one-field stacks, stop and repair before UI exposure.
- If capture/replay cannot faithfully reproduce row-local fields, do not close Step 3.

## Step 4 - Boundary-Masked / Phase-Safe Normal Angle UX

Status: shipped on `codex/sdf-normal-angle-beauty-phase-ux`.

Purpose:

- Preserve full-field `sdf_normal_angle` as a diagnostic phase view while adding a product-facing beauty path that avoids broad off-boundary branch/medial-axis sheets when users want normal-angle coloring near SDF boundaries.

Suggested branch:

- `codex/sdf-normal-angle-beauty-phase-ux`

Preconditions:

- Step 2 truth-sync merged.
- Step 3 either fully merged or explicitly deferred with a clean reason. If Step 3 lands row-local downsample, Step 4 must preserve it.

Bounded phases:

1. Review current phase metadata and source-row params:
   - confirm `sdf_normal_angle` is classified as phase;
   - confirm existing SDF Gate controls and boundary width behavior;
   - confirm full-field diagnostic preset still works.
2. Add REDs:
   - full-field `sdf_normal_angle` remains available and intentionally diagnostic;
   - boundary-masked normal-angle mode suppresses broad off-boundary phase planes;
   - phase offset or equivalent control moves the seam when full-field mode is active;
   - signed distance, boundary band, curvature, and normal smooth/root coloring do not show the same broad phase seam behavior under the matrix.
3. Implement the narrow UX:
   - either a source-local display mode (`full_field_diagnostic` vs `boundary_masked_beauty`) or a named preset that uses existing gate semantics;
   - keep existing SDF Gate and Gate Width controls authoritative if they already cover the behavior;
   - avoid broader Color Pipeline shape/palette redesign.
4. Runtime/capture proof:
   - no-mouse proof for diagnostic and boundary-masked modes;
   - capture/replay preserves the chosen mode and source params;
   - existing SDF composition compatibility remains green.
5. Documentation:
   - record that branch-cut/medial-axis planes are expected for full-field phase diagnostics, not a fractal kernel bug.

Required proof:

- Native Color Pipeline core/window/postprocess tests.
- Runtime publish.
- No-mouse SDF rows proof for full-field and boundary-masked normal angle.
- Capture/replay proof.
- Performance witness if the mode changes sampling cost.

Stop conditions:

- Do not remove or hide full-field diagnostic normal-angle behavior.
- Do not silently reinterpret all phase sources as scalar.
- Do not broaden into a full Color Pipeline function-library redesign.

## Step 5 - Authored SDF Pack UI / Viewport Integration

Purpose:

- Bring authored SDF packs from native/headless substrate into the normal viewer as a live field producer and Color Pipeline/overlay input, without yet adding an SDF-native fractal lane.

Suggested branch:

- `codex/authored-sdf-pack-viewport-integration`

Preconditions:

- Step 2 truth-sync merged.
- Step 3 and Step 4 either merged or explicitly deferred with clean compatibility notes.
- Existing SDF pack parser/CPU and CUDA evaluator plans remain current and green.

Recommended bounded sub-slices:

Step 5 is a mini-campaign, not a single commit target. Execute 5A, 5B, and 5C as separate bounded slices using the artifact names above. If a producer bridge or UI harness gap appears, checkpoint it before moving on to downstream consumption.

### Step 5A - Runtime Field Producer Bridge

Goal:

- Add an authored SDF pack field producer beside the mask-derived Lens SDF producer.

Behavior:

- Pack source is explicit: selected checked-in/example pack or loaded user pack path.
- Pack state includes pack id/path, params, region/view mapping, backend preference, backend used, and errors.
- CPU reference remains available for tests/fallback; CUDA evaluator is preferred when available.
- Producer emits the same SDF field interface consumed by overlay and Color Pipeline.

Tests:

- Native pack-to-field producer tests for circle/capsule/smooth union/repeat examples.
- CPU/CUDA parity for representative viewport grids.
- Error tests for invalid pack, missing params, unsupported op, oversized AST, and nonfinite values.

### Step 5B - Viewer UI Integration

Goal:

- Expose authored SDF pack loading/selection through the normal viewer flow without creating a new fractal type.

UI behavior:

- Add a bounded SDF Pack source/control surface in the existing left panel or Color Pipeline-adjacent flow.
- Loaded pack controls become normal visible numeric controls with no OS mouse dependency.
- Default viewer remains unchanged when no SDF pack is loaded.
- Pack state survives save/load and capture/replay.

Tests:

- Schema/binding tests for pack controls.
- Diagnostics state IO tests for pack state round-trip and old-state defaults.
- Runtime no-mouse proof for loading/selecting an example pack and changing a pack control.

### Step 5C - Color Pipeline And Overlay Consumption

Goal:

- Let authored SDF pack fields feed existing SDF Source rows and viewport overlay modes.

Behavior:

- Field source choice is explicit in reports: `mask_derived_lens_sdf` vs `authored_sdf_pack`.
- Existing SDF Source rows (`signed_distance`, `inside_outside`, `boundary_band`, `normal_angle`, `curvature`, Lens Field v2 where applicable) consume the selected field source only when compatible.
- Incompatible combinations fail closed with useful messages.
- Capture Finding captures the same pixels the viewport shows for authored pack field sources.

Runtime proof:

- No-mouse runtime proof for SDF pack field driving Color Pipeline frame hash.
- No-mouse overlay proof for pack field boundary/band/debug modes.
- Capture/replay matrix with one authored pack source.

Performance proof:

- Baseline and after witness for pack field generation and postprocess, with CPU/CUDA backend reported.
- Do not claim authored pack viewport is realtime for all packs; claim only bounded examples measured by the witness.

Step 5 full closeout:

- Native parser/evaluator/field producer rails.
- Schema/binding and diagnostics state IO rails.
- Runtime publish.
- No-mouse pack UI/control proof.
- No-mouse Color Pipeline and overlay proof.
- Capture/replay authority proof.
- Performance witness with numbers.
- Hostile audit must explicitly answer whether this stayed a field-producer integration rather than silently becoming an SDF-native fractal lane.

Stop conditions:

- If normal viewer integration needs a full new fractal lane to work, stop and replan before Step 6.
- If pack controls cannot be driven through the existing no-mouse automation surface, repair the harness before claiming UI integration.
- If CUDA pack backend cannot match CPU within established tolerance, keep CPU-only/product-limited and document before proceeding.

## Step 6 Boundary - Exhaust Work And Replan

Step 6 is not pre-planned as implementation here.

At Step 6, stop and replan from current evidence. The likely candidate is the first SDF-native selectable fractal lane, but it must be chosen only after Steps 1-5 prove:

- `master` carries the postprocess optimization and roadmap truth.
- Per-row/multi-field SDF downsample authority is either shipped or deliberately deferred with a clean reason.
- Normal-angle phase UX has a stable diagnostic/beauty split.
- Authored SDF packs can produce fields in the normal viewer path with state, capture/replay, Color Pipeline, overlay, and performance proof.

Step 6 replan questions:

- Which first SDF-native lane gives the best visual payoff without requiring a renderer monolith or 3D raymarcher?
- Should it be `sdf_recursive_fold_2d`, `sdf_smooth_lattice_2d`, `sdf_apollonian_field_2d`, or another bounded 2D field lane?
- What controls and examples prove it is meaningfully different from a Color Pipeline field overlay?
- What FPS/performance floor is acceptable for the first lane?
- Does it belong in the normal fractal dropdown immediately, or behind an experimental category gate?

## Proof Ledger

- Start authority: `codex/sdf-postprocess-measured-optimization` at `7ed70d6`, clean and pushed to origin.
- Rearward review: `ok` for `7ed70d6`.
- Planning slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "SDF next five work campaign planning" --profile native --plan docs/notes/sdf_next_five_work_campaign_PHASED_PLAN.md --contract docs/contracts/sdf_next_five_work_campaign.contract.json` appended `ck:b744029d` and locked the active contract.
- Current planning deliverable: drafted bounded plans for Step 1 merge, Step 2 roadmap truth sync, Step 3 per-row/multi-field SDF downsample authority, Step 4 boundary-masked/phase-safe normal-angle UX, Step 5 authored SDF pack viewport integration, and Step 6 replan boundary.
- Contract validation: passed with `artifacts/validation/sdf_next_five_work_campaign_contract.json`.
- Plan sync: passed with the direct plan-sync rail and then converted to receipt-parseable logged-command evidence at `artifacts/logs/sdf_next_five_work_campaign_plan_sync.log`.
- Hostile-audit validation: passed with `artifacts/validation/sdf_next_five_work_campaign_hostile_audit.json`.
- Code-quality baseline: passed with score `93/100` and no critical/error findings in `artifacts/validation/sdf_next_five_work_campaign_code_quality.json`.
- Diff hygiene: passed with `artifacts/validation/sdf_next_five_work_campaign_diff_check.json`.
- Checkpoint: committed as `48bb7e1` with receipt-shape follow-up `90d6cd6`.
- Push/clean-tree proof: `codex/sdf-postprocess-measured-optimization` pushed to origin at `90d6cd6`; rearward review reported `ok`.

## Hostile Audit

- Status: done
- Did this plan preserve the actual next Step 1-5 scope instead of substituting a smaller cleanup list? Yes. It covers merge, roadmap truth, per-row/multi-field SDF downsample authority, phase-safe normal-angle UX, and authored SDF pack viewport integration.
- Did this plan keep Step 6 as a real replan boundary? Yes. Step 6 has no predeclared implementation artifact and requires a fresh evidence review.
- Did this plan avoid implementation, merge, or runtime claims under a workflow-only planning contract? Yes. It only edited planning/contract surfaces and opened no product code changes.
- Did this plan require performance numbers for any hot-path step? Yes. Steps 3, 4 when sampling cost changes, and 5 all require before/after or bounded witness numbers.
- Did this plan keep SDF-native lanes out of Steps 1-5? Yes. SDF-native selectable fractal work is explicitly Step 6 material.

## Audit Passes

- [x] Pass 1 - draft-order audit found the campaign was detailed but did not pin exact plan/contract filenames for future Step 3 and Step 5 sub-slices.
- [x] Pass 2 - proof-gate audit repaired the artifact gap and confirmed each hot-path/product-visible step requires validation, runtime proof, and measured performance evidence where relevant.
- [x] Pass 3 - clean re-read of the repaired state found no additional real defect in the Step 1-5 ordering, Step 6 boundary, or docs-only scope.

## Audit Findings

- [done] Finding 1 - real workflow gap: the initial plan named Step 3 and Step 5 sub-slices but did not provide exact future plan/contract artifact names, which would make a future session invent names and risk widening scope. Repaired by adding the Future Slice Artifact Names section and explicitly requiring Step 3A/3B/3C and Step 5A/5B/5C to execute as separate bounded slices.
- [done] Finding 2 - clean re-read confirmed the repaired state keeps product implementation, master merge, runtime publish, and SDF-native lane work out of this workflow-only planning slice.
