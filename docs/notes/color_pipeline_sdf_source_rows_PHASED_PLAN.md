# Color Pipeline SDF Source Rows

## Current Phase

Closed - Color Pipeline SDF Source Rows.

## Phase Checklist

- [x] Phase 1 - create and lock this checked-in plan/contract
- [x] Phase 2 - add RED native proof that SDF source rows are absent or not live-authoritative
- [x] Phase 3 - add live-safe SDF source descriptors, enum ids, state IO, and Color Pipeline bridge rules
- [x] Phase 4 - add the Lens SDF-backed post-render Color Pipeline consumer without touching the core fractal kernel math
- [x] Phase 5 - publish runtime and prove no-mouse SDF source rows change the live frame while preserving Lens/Color Pipeline behavior
- [x] Phase 6 - hostile audit, repair findings, checkpoint, receipts, rearward review, push

## Explicit User Asks

- [done] Start the next SDF slice after live GPU Lens SDF integration.
- [done] Add Color Pipeline SDF source rows for signed distance, inside/outside, boundary band, normal angle, and curvature.
- [done] Keep the work modular and avoid a renderer monolith.
- [done] Do not expose visible Color Pipeline rows that do nothing.
- [done] Do not use OS mouse automation.

## Scope

In scope:

- Add five live-backed Color Pipeline Source rows: `sdf_signed_distance`, `sdf_inside_outside`, `sdf_boundary_band`, `sdf_normal_angle`, and `sdf_curvature`.
- Use the existing Lens SDF field producer as the first SDF source authority.
- Generate the Lens SDF field when either Lens visualization is enabled or the active Color Pipeline needs SDF source rows.
- Apply SDF source coloring through a bounded post-render consumer that reuses the current Color Pipeline shape, palette, and grading stack semantics.
- Fail closed for mixed SDF/non-SDF Source stacks unless the live consumer can honestly resolve every enabled Source row.
- Preserve the existing Lens aux visualization and GPU/CPU backend reporting.

Out of scope:

- New SDF-native fractal lanes.
- Viewport overlay productization.
- Authored SDF pack UI/live viewport integration.
- Dynamic CUDA kernel registration.
- Replacing `RenderFractalCUDA` or moving the Lens SDF field into the fractal kernel in this slice.
- Physical mouse or OS cursor automation.
- Claiming a performance improvement without a bounded witness.

## Proof Ledger

- Start authority: `master` at `665d7c6849c894e083460ab594509dba20622d84` has live GPU Lens SDF integration closed and rearward review `ok`.
- Boundary authority: the prior runtime-walk SDF plan explicitly deferred live Color Pipeline SDF rows because the CUDA color path had no SDF field input.
- RED 1: `color_pipeline_sdf_source_rows_red_core` failed before implementation because the Color Pipeline source catalog did not include the five SDF source rows.
- Harness repair: `test_color_pipeline_core`, `test_color_pipeline_window`, and `test_escape_time_coloring` were missing focused targets in `ui_app/build_tests_vsdevcmd.cmd`; those targets are now wired and validated.
- Implemented authority: `ColorSignal` now includes five SDF source signals, enum/state/capture ids round-trip them, and Color Pipeline core/window bridge code treats them as runtime-backed only with real SDF source-stack params.
- Implemented live consumer: `color_pipeline_sdf_postprocess.*` applies Lens SDF signed distance, inside/outside, boundary band, normal angle, and curvature through current Color Pipeline shape/palette/grading semantics after the base CUDA render.
- Headless repair: hostile review found headless diagnostic/finding capture bypassed the SDF postprocess; `RenderHeadlessFractalFrame(...)` now requests the mask, computes Lens SDF, and applies the same postprocess before capture.
- Runtime proof: `tests/test_fractal_runtime_color_pipeline_sdf_rows.py::test_color_pipeline_sdf_source_rows_are_live_backed_no_mouse` selects all five SDF Source rows with no mouse, verifies state/draft ownership, and proves scale/bias edits change published-runtime frame hashes.
- Preservation proof:
  - `test_color_pipeline_core`: passed=142 failed=0
  - `test_color_pipeline_window`: passed=172 failed=0
  - `test_color_pipeline_sdf_postprocess`: passed=17 failed=0
  - `test_diagnostics_state_io`: all passed
  - `test_lens_sdf`: all passed
  - `test_lens_sdf_cuda`: all passed
  - `test_flashlight_probe`: 21 passed, 0 failed
  - `test_runtime_walk_headless`: passed=48 failed=0
  - `test_escape_time_coloring`: all passed
  - runtime publish: `ui_app/build_vsdevcmd.cmd` succeeded
  - published runtime pytest: 1 passed with JUnit at `artifacts/pytest/color_pipeline_sdf_source_rows_runtime.junit.xml`

## Hostile Audit

- Status: complete
- Required posture: assume the rows are catalog-only, the rendered frame is still using the old smooth-escape source, the Lens SDF field is stale or missing when Lens UI is off, mixed Source stacks silently degrade to zeros, or the implementation regresses existing Color Pipeline rows until focused proof disproves each risk.

## Audit Passes

- [done] Pass 1 - re-read Color Pipeline catalog/bridge/state IO for fake runtime-backed row exposure.
- [done] Pass 2 - clean re-read of live render order and Lens SDF generation for stale or missing field consumption.
- [done] Pass 3 - clean re-read after repair with preservation rails for Lens SDF, existing Color Pipeline rows, runtime publish, and no-mouse proof.

## Audit Findings

- [done] Headless capture path rendered through `RenderFractalCUDA(...)` without requesting a mask or applying the SDF Color Pipeline postprocess. This would have let no-mouse headless proof disagree with the live render path. Fixed by adding `RenderHeadlessFractalFrame(...)` and rerunning the published runtime pytest with all five SDF rows.
- [done] Contract runtime proof node id and command were stale after the runtime test was broadened from one SDF row to all five rows. Fixed the contract and reran the pytest with the required JUnit artifact.
- [clean] Clean re-read found no mixed-stack silent fallback: native window and postprocess tests fail closed for SDF/non-SDF Source mixtures.
- [clean] Clean re-read confirmed the repaired state did not expose another Lens SDF producer regression in focused CPU/CUDA Lens SDF, flashlight, and runtime-walk rails.

## Action Hostile Review

- Action ID: color-pipeline-sdf-source-rows-start
- Suspected failure mode: adding Source catalog rows without a real Lens SDF-backed live consumer would create visible controls that do nothing or lie through the runtime-backed catalog.
- Correct owner/action: Color Pipeline Source catalog plus live bridge plus Lens SDF post-render consumer must move together, with fail-closed tests for unsupported mixtures.
- Proof surface: focused native Color Pipeline core/window/state tests, Lens SDF preservation rail, runtime publish, no-mouse published runtime proof, contract validation, plan sync, hostile-audit validation, code-quality baseline, and diff hygiene.
- Blocked action: SDF-native fractal lane, viewport overlay, authored-pack UI, renderer replacement, physical mouse automation, or broad Color Pipeline redesign.
