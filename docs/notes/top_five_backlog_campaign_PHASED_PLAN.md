# Top Five Backlog Campaign Planning

## Current Phase

Complete - top-five backlog campaign planned, audited, and validated.

## Phase Checklist

- [x] Phase 1 - create the detailed top-five campaign plan and lock the docs-only planning slice
- [x] Phase 2 - hostile-audit the five-step ordering, boundaries, dependencies, and proof rails
- [x] Phase 3 - validate contract, phased-plan sync, hostile audit, code-quality baseline, and diff hygiene
- [x] Phase 4 - checkpoint the docs-only campaign plan and record a clean stop point

## Explicit User Asks

- [done] Determine whether the top five backlog items are planned in enough detail.
- [done] If not, take the time to detail the five linear work steps.
- [done] Keep this as planning only, not implementation.

## Scope

In scope:

- Detailed implementation sequencing for the current top five backlog items:
  1. Diagnostics capture output paths.
  2. Lens SDF control truth plus SDF field substrate seed.
  3. Categorized selector plus view presets.
  4. Camera/dive behavior.
  5. Smooth-escape/color tuning.
- Per-step scope boundaries, likely seams, RED tests, proof gates, and closeout expectations.
- Backlog/status docs links so future sessions can find this plan.

Out of scope:

- Product code, tests, schema, renderer, runtime harness, Color Pipeline behavior, or live viewer behavior changes.
- Starting any implementation branch.
- Claiming any of the five work items are fixed.
- Pushing or merging implementation work.

## Campaign Answer

Current continuation order after the 2026-05-24 reprioritization:

1. Diagnostics capture output paths - closed in `docs/notes/diagnostics_capture_output_paths_PHASED_PLAN.md`.
2. Smooth-escape/color tuning - measurement inventory closed in `docs/notes/smooth_escape_color_measurement_PHASED_PLAN.md`; interior-tone repair closed in `docs/notes/smooth_escape_interior_tone_PHASED_PLAN.md`.
3. Highest-confidence smooth/color fixes from that measurement - continue only from measured evidence and keep low-luma/low-unique rows separate.
4. Authored SDF pack parser plus CPU reference - later SDF substrate work from `docs/notes/sdf_field_pack_near_term_TODO.md`.
5. CUDA SDF pack evaluator - after the parser/CPU reference is closed.

Deferred behind that order for now:

- Categorized selector and view presets.
- Camera and dive behavior.

Before this slice:

- Step 2 was detailed in `docs/notes/sdf_field_pack_near_term_TODO.md`.
- Steps 1, 3, 4, and 5 had backlog/known-issue notes but were not planned to the same execution-ready level.

After this slice, the intended linear order is:

1. Diagnostics capture output paths.
2. Lens SDF truth cleanup, then SDF field interface extraction.
3. Categorized selector, then view presets.
4. Camera/dive behavior, after view preset catalog metadata exists.
5. Smooth-escape/color tuning, after selector/preset work makes visual comparison easier.

Recommended branch model:

- Keep this document as the umbrella campaign plan.
- Use one implementation branch per major step unless a step is explicitly split below.
- Each implementation branch still needs its own checked-in plan/contract, RED, hostile audit, validation, checkpoint, receipts, and merge/push discipline.

## Step 1 - Diagnostics Capture Output Paths

Status: closed in `docs/notes/diagnostics_capture_output_paths_PHASED_PLAN.md`. The historical problem statement below is kept as the original plan record, not as current open work.

### Current Problem

`--capture-diagnostic` writes to `runtime/diagnostics/last/`, so rapid or concurrent diagnostic captures overwrite each other. This is a trust/tooling problem, not a renderer feature.

### Product Intent

- Preserve a convenient `runtime/diagnostics/last/` pointer or mirror if useful.
- Add a timestamped default output bundle for every diagnostic capture.
- Add an explicit `--out-dir` override for deterministic scripted output.
- Ensure diagnostics capture output is discoverable and machine-readable.

### Likely Seams

- `ui_app/src/diagnostics_capture.cpp`
- headless/CLI mode parsing around diagnostic capture
- runtime diagnostics paths under `runtime/diagnostics/`
- tests that invoke headless diagnostics capture
- any docs/help text for the diagnostic capture command

### Required REDs

- Two default diagnostic captures in one run or rapid sequential runs do not overwrite the first bundle.
- `--capture-diagnostic --out-dir <path>` writes exactly to the requested output path.
- The rolling `last` output, if retained, is clearly a convenience mirror and not the only archive.
- Invalid or unwritable `--out-dir` fails with a useful error instead of silently falling back.

### Implementation Shape

- Add a small output-path resolver for diagnostics capture.
- Default path should include a timestamp and enough collision protection to handle same-second runs.
- Keep existing consumers that expect `runtime/diagnostics/last/` working through a mirror, manifest, or explicit compatibility rule.
- Do not change finding capture, render capture, or viewport behavior in this slice.

### Proof Gates

- Focused native/unit tests for output path resolution.
- Focused headless/runtime test for default unique bundle and explicit `--out-dir`.
- Diff hygiene and code-quality baseline.

### Done Means

- No diagnostic capture is lost by default due to `last/` overwrite.
- Scripts can request a deterministic output directory.
- Compatibility behavior for `last/` is documented and tested.

## Step 2 - Lens SDF Control Truth And SDF Field Substrate Seed

Detailed source plan: `docs/notes/sdf_field_pack_near_term_TODO.md`.

### Step 2A - Lens SDF Truth Cleanup

Goal:

- Make `fractal.lens.downsample` truthful or hide/remove it.
- Preferred outcome: wire it as real behavior using the Salticid `lens_sdf_chamfer` pattern.

Likely seams:

- `ui_app/src/lens_sdf.h`
- `ui_app/src/lens_sdf.cpp`
- possible new `ui_app/src/lens_sdf_chamfer.*`
- `ui_app/src/main.cpp`
- `ui_app/src/flashlight_probe.cpp`
- `ui_app/src/runtime_walk_headless.cpp`
- `ui_app/tests/test_lens_sdf.cpp`
- schema/binding tests if visibility changes

Required REDs:

- Visible `lens.downsample` currently does not affect the live Lens SDF render path.
- Duplicate downsample helpers exist outside a shared lens utility surface.
- Salticid-style downsample normalization behavior is not covered locally.

Proof gates:

- Native downsample normalization and mask downsample tests.
- Exact signed-distance sample tests.
- Runtime or headless proof that changing `lens.downsample` changes Lens SDF work resolution or output metadata.
- Base fractal output remains unchanged by Lens SDF downsample changes.

### Step 2B - SDF Field Interface Extraction

Goal:

- Extract reusable scalar signed-distance field data from Lens SDF.
- Keep RGBA visualization as presentation only.

Required REDs:

- Current API makes downstream consumers parse or depend on debug RGBA instead of scalar field data.
- Single-sample helpers recompute full fields instead of consuming a reusable field result.

Proof gates:

- Existing Lens SDF tests stay green.
- New scalar field tests prove sign convention, dimensions, diagonal distance behavior, and invalid input failure.
- No authored SDF pack is introduced in this sub-slice.

### Step 2C - Lens Semantics Authority

Goal:

- Centralize what "inside" means per fractal/family.
- Preserve synthetic basin root-parity behavior but make it explicit and documented.

Proof gates:

- Every shipped `FractalType` maps to an explicit lens semantics descriptor.
- Future unsupported types fail closed or are visibly unsupported.
- Renderer/probe/report paths use one vocabulary.

### Done Means

- Lens controls are no longer ambiguous.
- Lens SDF is the first mask-derived field producer.
- Authored SDF packs remain a later producer, not mixed into the Lens cleanup.

## Step 3 - Categorized Selector And View Presets

### Current Problem

The catalog is growing, but the normal selector still behaves like a flat list with one canonical view per fractal. Users cannot easily browse by family or select known-good regions.

### Product Intent

- Make fractal selection easier without changing `FractalType` runtime authority.
- Add per-fractal view presets before adding many more fractal families.
- Preserve saved-state compatibility and user-customized camera state.

### Dependency

This step should build on the recently landed catalog/default authority slices:

- `docs/notes/fractal_catalog_authority_inventory_PHASED_PLAN.md`
- `docs/notes/fractal_view_defaults_catalog_migration_PHASED_PLAN.md`

### Slice 3A - Catalog Selector Metadata

Goal:

- Add catalog metadata for grouping/categorization and display names.
- Preserve the existing selected `FractalType` as public state.

Likely seams:

- catalog metadata introduced by the catalog authority slices
- `ui_app/src/fractal_types.h`
- schema/binding option export
- parameter descriptor/export surfaces
- selector tests

Required REDs:

- Every current fractal must have a category/group.
- Unknown or missing group metadata must fail closed in catalog validation.
- Selector identity must remain explicit after category metadata is introduced.

Proof gates:

- Catalog/native test enumerates every current type.
- Runtime no-mouse selector proof for representative types across categories.
- Existing parameter descriptor/export tests remain green.

### Slice 3B - View Preset Catalog

Goal:

- Add a `view_preset` control with per-fractal presets plus `Custom`.
- Selecting a preset applies center, zoom, and optional rotation.
- User camera edits move the preset state to `Custom` without losing the current fractal.

Likely seams:

- `ui_app/src/fractal_derived_fields.cpp`
- catalog view-default metadata
- schema JSON and schema binding
- diagnostics state save/load
- capture/finding state export
- runtime no-mouse control set path

Required REDs:

- There is no selectable `view_preset` for representative fractals.
- Switching fractal type has only one canonical view.
- Camera mutation after preset selection does not have a tested `Custom` authority path.

Proof gates:

- Native binding tests for preset selection and `Custom` transition.
- Per-fractal catalog test proving at least one default preset for every current type and multiple interesting presets for high-value families.
- Runtime no-mouse proof that selecting presets changes camera/view and keeps fractal selector identity.
- Capture/finding proof that captured state records the visible camera after preset selection.

### Slice 3C - User-Facing Selector UX

Goal:

- Expose the grouping in the normal left-panel flow.
- Avoid a second runtime source of truth for selected fractal.

Implementation options to evaluate during RED:

- Grouped combo labels if current UI supports them.
- A category filter plus fractal selector if grouping in one combo is not supported.
- A searchable/filtered selector only if supported by existing UI patterns.

Required proof:

- Selecting category/filter cannot silently change the current fractal unless the user selects a fractal.
- Existing automation can still select a fractal by id.
- Saved-state load remains based on fractal id, not display label.

### Done Means

- Catalog browsing is organized.
- Users can jump to multiple known-good views.
- This becomes the foundation for future new fractal additions.

## Step 4 - Camera And Dive Behavior

### Current Problem

`ApplyAutoDivePerFrame()` is still too primitive:

- not dt-aware
- shallow flat zoom model
- little or no acceleration with depth
- behavior modes are selectable but not meaningfully distinct

### Dependency

Do this after Step 3 so camera behavior can reference view preset/catalog metadata instead of adding more hardcoded camera special cases.

### Slice 4A - Extract Camera/Dive Model For Tests

Goal:

- Move the math that can be tested out of `main.cpp` without changing behavior.

Required REDs:

- Current auto-dive math lacks a focused deterministic unit rail.
- Frame-rate dependency is not isolated by a helper test.

Proof gates:

- Behavior-preserving tests around current defaults.
- No UI/runtime behavior change in this extraction slice.

### Slice 4B - dt-Aware Dive And Acceleration

Goal:

- Make dive speed stable across frame rates.
- Add bounded acceleration that increases usefulness without making navigation uncontrollable.

Required REDs:

- Same simulated elapsed time with different frame counts currently produces different zoom depth.
- Dive remains too shallow over a representative elapsed-time window.

Proof gates:

- Unit tests for dt invariance and bounded acceleration.
- Runtime no-mouse proof that auto-dive changes zoom at expected rate and remains interruptible.
- No NaN/Inf camera states.

### Slice 4C - First Real Behavior Mode

Goal:

- Implement one real mode first, preferably `complexity`.
- Classify every exposed behavior mode as implemented, intentionally hidden/disabled, or explicitly deferred with truthful UI text.
- Do not leave `orbit` or `entropy` as selectable stubs if the slice proves they still do nothing.

Complexity mode candidate:

- steer toward higher iteration or local detail/variance using bounded sampling.
- use existing sample/probe surfaces where possible.
- avoid adding a heavy per-frame analysis path without measurement.

Required REDs:

- Selecting `complexity` currently does not produce distinct behavior.
- Mode selection must be runtime/state-backed and not just UI text.

Proof gates:

- Focused native tests for steering decision logic.
- Runtime proof that `complexity` diverges from default mode under a deterministic scene.
- Performance witness that steering does not tank frame time.

### Done Means

- Auto-dive is frame-rate independent and useful.
- At least one behavior mode is honestly implemented.
- Remaining behavior modes are either implemented, hidden/disabled, or explicitly deferred without misleading the user.

## Step 5 - Smooth-Escape And Color Tuning

### Current Problem

The renderer has a better cyclic palette than the old flat ramp, but escape-time coloring still has remaining global assumptions:

- one global band period
- forced-black smooth-escape interiors repaired in `docs/notes/smooth_escape_interior_tone_PHASED_PLAN.md`
- weak deep-zoom normalization
- known low-tuning rows from K4 remain deferred polish

### Dependency

Do this after Step 3 so visual comparisons can use view presets. It does not have to wait for Step 4 unless camera/dive work is actively touching the same runtime path.

### Slice 5A - Measurement And Tuning Inventory

Goal:

- Measure current escape/smooth-signal ranges per representative family and preset.
- Decide which tuning knobs belong in catalog metadata versus renderer constants.

Required REDs:

- Current global smooth band period cannot be justified per family.
- Interior treatment is not classified by family.

Proof gates:

- Headless measurement report for representative escape-time families.
- No product behavior change in the measurement slice.

### Slice 5B - Catalog-Driven Escape Tuning Metadata

Goal:

- Move per-family color tuning constants into catalog/runtime metadata where appropriate.
- Preserve existing defaults unless a measured tuning change is explicitly part of the slice.

Likely seams:

- `ui_app/src/escape_time_coloring.h`
- `ui_app/src/fractal_renderer.cu`
- catalog metadata from prior steps
- renderer tests
- Color Pipeline tests

Required REDs:

- Representative families need different smooth band periods or normalization to avoid washed/flat output.
- Existing tests do not lock per-family tuning authority.

Proof gates:

- Renderer/native tests for per-family tuning dispatch.
- Color Pipeline focused rails remain green.
- Runtime screenshot/hash proof for representative presets.
- Bounded performance witness.

### Slice 5C - Interior And Low-Tuning Polish

Goal:

- Address the known polish rows without broad Color Pipeline redesign:
  - Collatz fast-escape black pixels if still visible.
  - "Neither" band exhaustion visibility.
  - visually degenerate Nova/Lambda defaults if view presets did not already solve them.

Required REDs:

- Each changed row gets a focused visual/hash/sample witness.
- If a row is aesthetic and not objectively testable, record the bounded witness and do not overclaim.

Proof gates:

- Runtime no-mouse proof on representative view presets.
- Color Pipeline rails.
- Renderer/sample tests where math routing is touched.

### Done Means

- Smooth-escape tuning is family-aware where measured evidence supports it.
- Color Pipeline behavior is preserved.
- Visual polish changes are bounded and test-backed.

## Linear Order Rationale

The five items are mostly independent, but this order reduces rework:

1. Diagnostics output paths are quick trust cleanup and do not depend on the viewer UI.
2. Lens SDF truth is a small control-truth bug and seeds the later SDF substrate.
3. Selector/view presets make future visual work easier to evaluate.
4. Camera/dive behavior should use the preset/catalog substrate instead of hardcoded destinations.
5. Smooth-escape tuning benefits from stable representative presets and should not be mixed with selector/camera refactors.

## Shared Guardrails

- No physical mouse automation.
- No broad renderer rewrite.
- No implicit fallback for unknown ids, paths, presets, or controls.
- Preserve saved-state compatibility unless a slice explicitly migrates state.
- Keep Color Pipeline regressions out of non-color slices.
- Do not claim a viewer-facing fix from helper-only proof.
- Each implementation branch needs its own plan/contract and receipts.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed before mutation.
- Source docs inspected: `DEFERRED_THREADS.md`, `KNOWN_ISSUES.md`, `spec_intake/_STATUS.md`, and `docs/notes/sdf_field_pack_near_term_TODO.md`.
- Planning doc: this file.
- Backlog/status links: `DEFERRED_THREADS.md` and `spec_intake/_STATUS.md` now point future sessions at this plan.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/top_five_backlog_campaign_planning.contract.json --out-json artifacts/validation/top_five_backlog_campaign_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile-audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/top_five_backlog_campaign_PHASED_PLAN.md --out-json artifacts/validation/top_five_backlog_campaign_hostile_audit.json` passed.
- Code-quality baseline: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/top_five_backlog_campaign_code_quality.json` passed with baseline check passed.
- Diff hygiene: `py -3.14 tools/viewer_host_run_logged_command.py --label top_five_backlog_campaign_diff_check --log artifacts/logs/top_five_backlog_campaign_diff_check.log --out-json artifacts/validation/top_five_backlog_campaign_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.
- Checkpoint: represented by this docs-only planning commit and the paired `HANDOFF_LOG.md` entry.

## Hostile Audit

- Status: complete

## Audit Passes

- [done] Pass 1 - verify each of the five items has a bounded implementation slice and proof rail.
- [done] Pass 2 - verify the order does not hide dependencies or mix unrelated work.
- [done] Pass 3 - verify the plan does not claim any implementation work is complete.
- [done] Pass 4 - clean re-read after repairing camera behavior-mode truth found no additional real issue found in the docs-only boundary.

## Audit Findings

- [done] Finding 1: the initial camera/dive plan allowed `orbit` and `entropy` to remain unchanged, which could preserve misleading selectable stubs; the plan now requires every exposed behavior mode to be implemented, hidden/disabled, or explicitly deferred with truthful UI text.
- [done] Clean re-audit: re-read the repaired state and no additional workflow mistake found; product code, tests, schema, renderer, runtime harness, Color Pipeline, and live viewer behavior remain untouched.

## Action Hostile Review

- Action ID: top-five-backlog-campaign-validation
- Suspected failure mode: A broad five-item plan could become vague campaign text or accidental implementation scope.
- Correct owner/action: Documentation-only campaign planning with per-step boundaries, REDs, proof gates, and future branch guidance.
- Proof surface: Contract validation, phased-plan sync, hostile-audit validation, code-quality baseline, and diff hygiene.
- Blocked action: Product code, tests, schema, renderer, runtime harness, Color Pipeline, or live viewer behavior changes.

## Notes

- This is not an implementation branch.
- Next implementation branch should start with Step 1 unless the user explicitly chooses another step.
