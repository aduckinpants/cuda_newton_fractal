# Deferred Threads

Purpose: keep intentionally paused threads visible so they do not vanish between slices.

Active specs: see `spec_intake/_STATUS.md` for the current planning surface.
Agent protocol: see `AGENT_WORKING_PROTOCOL.md` for working rules.

Last reconciled: 2026-05-26 on `codex/color-pipeline-sdf-postprocess-performance` during the SDF postprocess performance slice.

## Current Difficulty / Reward Priority

This table is the current backlog order after reconciling the older deferred notes, the parameter functionality campaign, the capture/pacing repairs, the generic equation-pack pause README, and the modular Magnet/toolkit plan. It is planning guidance, not permission to skip a new plan/contract for implementation.

| Rank | Work thread | Difficulty | Reward | Current next gate |
|------|-------------|------------|--------|-------------------|
| 1 | Backlog truth refresh | Low | High trust | This docs-only slice reconciles stale planning text. |
| 2 | Diagnostics capture output paths | Low | Medium | Add timestamped or explicit `--out-dir` bundles so captures stop overwriting `runtime/diagnostics/last`; see `docs/notes/top_five_backlog_campaign_PHASED_PLAN.md`. |
| 3 | Lens SDF control truth + SDF field substrate seed | Low/Medium | Medium/High | First make `lens.downsample` truthful, then extract a reusable SDF field interface; see `docs/notes/sdf_field_pack_near_term_TODO.md`. |
| 4 | Categorized selector + view presets | Medium | High | Organize the growing catalog and add per-fractal view preset choices before more catalog growth; see `docs/notes/top_five_backlog_campaign_PHASED_PLAN.md`. |
| 5 | Camera/dive behavior | Medium | High | Make auto-dive dt-aware and implement at least one real behavior mode instead of flat zoom stubs; see `docs/notes/top_five_backlog_campaign_PHASED_PLAN.md`. |
| 6 | Smooth-escape/color tuning | Medium | Medium/High | Per-family color tuning and interior treatment, without reopening Color Pipeline architecture broadly; see `docs/notes/top_five_backlog_campaign_PHASED_PLAN.md`. |
| 7 | Color Pipeline composition follow-ups | Medium | High | UI-Salt backend authority and preset workflow truth are shipped; current composition cleanup removes visible draft/live-bridge wording. Remaining bounded follow-ups are boundary-masked phase source, SDF masks/gates, and function-library taxonomy/layout work. |
| 8 | SDF performance follow-up after preview policy | Medium/High | High | Preview postprocess quality policy is shipped; shared SDF Field Downsample is not per Source row/layer yet. Next choose between per-row/multi-field downsample design and GPU Color Pipeline postprocess with measured client evidence. |
| 9 | Generic equation-pack productization | Medium/High | High strategic | Persistence first, then catalog picker, authoring UX, Salticid adapter, performance profiling. |
| 10 | Callable/transpiler handoff | High | High strategic | Finish the handoff boundary without pretending dynamic backend execution already exists. |
| 11 | Catalog/family authority refactor | High | High | Move remaining family defaults, visibility, and validation out of brittle monolithic paths before larger families. |
| 12 | Perturbation deep-zoom expansion | High | Very high | Start with Mandelbrot/Multibrot recurrence proof and precision tests; do not generalize blindly. |
| 13 | New substrate families | Very high | High | Lyapunov, IFS, attractor density, 3D DE/raymarch, and explanation-state families each need their own rendering/state contracts. |

## 1. Viewer Responsiveness / Adaptive Render Recovery

Status: deferred follow-up; older April text is superseded by later capture/pacing repairs.

Current shipped state:
- Interaction preview scaling and one full-quality settle render remain.
- Later capture/pacing work made preview activation measurement-gated instead of unknown-timing aggressive.
- Slow measured interaction has no-mouse proof that it enters preview and then settles.
- Capture Finding now preserves visible aspect/camera at high resolution and forces standard/f64 output.
- The focused pacing helper target exists: `ui_app/build_tests_vsdevcmd.cmd test_viewer_render_pacing`.
- SDF realtime pacing telemetry now reports base render, SDF field, SDF postprocess, and SDF total timing.
- SDF postprocess signal specialization now avoids derivative/neighborhood sampling for scalar-only SDF Source rows.
- SDF preview postprocess quality policy now reduces CPU SDF source samples during interactive preview through a reportable postprocess pixel step; full-quality render and capture stay step 1.
- Full-quality SDF postprocess now reuses one computed source color per downsampled SDF field cell, so shared `SDF Field Downsample` reduces CPU postprocess samples without adding another persisted downsample authority or changing expanded pixels.

Remaining risk:
- The existing proof focuses on render dimensions, timing reports, and no-mouse camera/control edits. It does not fully prove whole-PC responsiveness or end-to-end input-to-frame latency under every extreme f64 workload.

Resume constraints:
- Do not tune live pacing policy by feel.
- Add instrumentation first if the user reports remaining responsiveness pain: capture input mutation time, render start/end, preview scale, target size, actual size, and UI report latency in one persistent viewer session.
- For SDF-specific slowness, specialize or move SDF postprocess work before changing debounce policy again.
- Keep capture-quality rendering separate from live-view responsiveness work.

Key references:
- `ui_app/src/viewer_render_pacing.h`
- `ui_app/src/viewer_render_pacing.cpp`
- `ui_app/src/main.cpp`
- `ui_app/tests/test_viewer_render_pacing.cpp`
- `tests/test_fractal_runtime_resolution_pacing.py`
- `docs/contracts/fps_debounce_measurement_PHASED_PLAN.md`
- `docs/notes/capture_finding_aspect_camera_PHASED_PLAN.md`

## 2. Explaino Design-Space Deep Dive

Status: deferred research / later family-growth planning.

Current shipped state:
- The core Explaino family expansion, selector/control repairs, `explaino_all` registry-axis behavior, parameter authority work through Step 9, and generated/internal editor authority are closed in the current parameter campaign chain.
- The immediate slider-authority campaign no longer owns new Explaino design-space research.

Remaining work:
- Future Explaino research should start from a concrete hypothesis or sample_fn/sidecar proof surface, not from broad historical archaeology.
- FITS/invariance/joy/LUT/atlas research remains outside this repo's immediate implementation path unless a fresh plan pulls one bounded thread in.

Resume constraints:
- Use Salticid / CLI / broker probing first where possible.
- Keep observation lanes separate from new `FractalType` growth.
- Do not treat legacy Explaino LUT or old sidecar narratives as shipped product behavior.

Planning sources:
- `spec_intake/ExplainoDesignSpace_DeepDive_2026-04-05.md`
- `spec_intake/ExplainoFamilyExpansion_V1_SpecIntake.md`
- `docs/contracts/parameter_functionality_campaign_PHASED_PLAN.md`

## 3. Common Fractal Catalog Expansion + 2-Layer Dropdown

Status: partially shipped; remaining work is mostly selector/view-preset product polish.

Shipped since the original deferred note:
- Spider and Collatz exploration controls were added through the parameter functionality campaign.
- Celtic Mandelbrot, Burning Ship, and Perpendicular Burning Ship fold/mix controls were added.
- Magnet Type I shipped as the first modular fractal-toolkit slice with explicit controls and proof.
- Multibrot real/complex exponent work shipped earlier in the QoL branch.

Remaining high-reward work:
- Make the fractal chooser easier to use as the catalog grows: grouped/categorized selector, clearer naming, and a default landing path that is intentional.
- Add a `view_preset` style control/catalog so each fractal has multiple interesting starting views instead of one hardcoded canonical view.
- Decide whether startup should default to baseline `explaino` with `joy_basins` coloring, as originally requested, or remain current behavior. Do not slip that into unrelated work.

Still not safe as a small catalog wave:
- Sierpinski gasket / carpet and Apollonian-style gaskets need an IFS / geometry contract.
- Menger sponge and Mandelbulb need 3D DE/raymarch camera, normal, lighting, and performance contracts.

Planning sources:
- `spec_intake/CommonFractalCatalog_Deferred_2026-04-06.md`
- `spec_intake/FractalTypeDropdown_and_MultiFractalKernel_SpecIntake.md`
- `spec_intake/FractalCatalog_WaveTwo_SpecIntake.md`
- `docs/notes/fractal_toolkit_modular_catalog_magnet_wave_PHASED_PLAN.md`

## 4. CUDA Catalog Refactor Before Break-Wall

Status: partially complete; still strategically important before larger family additions.

Completed seams:
- `escape_time_direct_formulas.h`
- `escape_time_specialized_formulas.h`
- `perturbation_reference_orbit.h`
- `escape_time_coloring.h`
- `explaino_collatz_formulas.h`
- `polynomial_eval_real_coeffs.h`
- `basin_coloring.h`
- `fractal_runtime_validation.h`
- sample/probe/callable surfaces through the K1-K5/V2 work listed in `spec_intake/_STATUS.md`

Remaining seams:
- Explicit family defaults / visibility / validation still need cleaner ownership outside monolithic switch pressure.
- The future categorized dropdown/view-preset work needs cleaner catalog metadata than historical hardcoded defaults.
- The all-fractal control-surface proof expansion remains open after animation applicability Phase 1.

Prepared next slices:
- Slice A: `docs/notes/fractal_catalog_authority_inventory_PHASED_PLAN.md` creates a behavior-preserving catalog metadata authority and fail-closed coverage for every current `FractalType`.
- Slice B: `docs/notes/fractal_view_defaults_catalog_migration_PHASED_PLAN.md` depends on Slice A and moves current view defaults into catalog authority without changing visible defaults.

Timing rule:
- Do this before 3D, IFS, distance-estimator, Mandelbulb, density-field work, or broad perturbation expansion.

## 5. Lens SDF Follow-Ups

Status: partially shipped substrate. Capture/replay authority, phase-signal metadata, Color Pipeline fractal-switch preservation, realtime pacing telemetry, SDF postprocess signal specialization, SDF preview postprocess quality policy, CUDA SDF postprocess backends, and live-only adaptive SDF field resolution are shipped.

Shipped since the original deferred note:
- `lens.downsample` control truth is repaired.
- Lens SDF scalar field authority is extracted.
- Family-aware Lens SDF mask semantics are explicit.
- Authored SDF pack parser and CPU reference are shipped.
- CUDA SDF pack evaluator and hardening are shipped.
- Flashlight probe/report and runtime-walk headless/report consumers now use source-neutral SDF signal sampling.
- CUDA-backed Lens SDF field producer is shipped with CPU fallback/reference and backend reporting.
- Live Color Pipeline SDF source rows are shipped for signed distance, inside/outside, boundary band, normal angle, and curvature.
- Capture Finding parity for active SDF Color Pipeline source-row pixels is shipped.
- Shared Lens Downsample visibility/control authority is shipped when Color Pipeline SDF rows use the field.
- Normal viewport SDF overlays for boundary, band, and field-debug modes are shipped.
- SDF Source row tuning controls, `sdf_boundary_band` boundary width, and the Color Pipeline Source-section SDF Field Downsample alias are shipped.
- Capture/replay authority is shipped: `state.json` serializes Lens replay settings, explicit state loading accepts arbitrary JSON filenames, and a fast SDF/non-SDF replay matrix is in place.
- Color Pipeline phase-signal metadata is shipped: `sdf_normal_angle` is classified as phase-like while signed distance, boundary band, and curvature remain scalar.
- Color Pipeline fractal-switch preservation is shipped: compatible fractal selector changes keep supported live/source-stack Color Pipeline state, while unsupported switches project to target defaults.
- SDF realtime pacing telemetry is shipped and identifies SDF postprocess as a measured FPS bottleneck.
- SDF postprocess signal specialization is shipped: scalar-only rows no longer pay for derivative neighborhoods, while derivative sources preserve their sampling behavior.
- SDF preview postprocess quality policy is shipped: shared field downsample remains the authority, interactive preview reports a stepped postprocess path, and full-quality render/capture stays step 1.
- Full-quality downsampled-field cell reuse is shipped: when the shared SDF field is lower resolution than the render target, the Color Pipeline postprocess computes once per field cell and fills the same render pixels that already mapped to that cell.

Active follow-up:
- Broader composition/preset UX now has preset workflow truth and visible implementation-wording cleanup covered by bounded slices. Keep later composition work split by topic instead of turning it into a broad redesign.
- The immediate composition repair for disabled-row compatibility/error authority and `sdf_curvature` plus `sdf_normal_angle` SDF-only Source-stack blending is shipped on `08e62b6`.
- Choose the next larger SDF performance/design slice explicitly before returning to authored-pack UI or SDF-native lanes: field-generation algorithm/caching work or per-row/multi-field downsample authority with measured client evidence.

Still deferred follow-ups:
- Add SDF-native selectable fractal lanes only after the field producer and consumer proof is stable.
- Add authored SDF pack UI/live viewport integration after the normal field consumers are stable.
- Keep treating Lens SDF as the first mask-derived SDF field producer, not as the place to embed authored SDF packs.
- Use `docs/notes/sdf_field_pack_near_term_TODO.md` as the detailed implementation sequencing surface.

Resume constraints:
- Do not mix this with organized selector/common-fractal implementation unless the selector slice directly surfaces stale Lens controls.
- Do not merge Salticid's analytic SDF operator pack wholesale into this repo. Port only bounded, reviewed pieces when a slice explicitly needs them.
- Keep mask-derived Lens SDF and authored analytic SDF pack authority separate; they meet at the field interface.
- Do not claim authored-pack live viewport integration or SDF-native lanes are shipped until the normal viewer path proves them.

## 6. CUDA-Resident sample_fn + Optimization Staging + Reflexive Sidecar

Status: core initiative closed; follow-ons only.

Current shipped state:
- K1-K5 are done.
- CLI Bridge V2-A through V2-G are done.
- `fractal.sample`, `generic.sample`, `--describe-functions`, sample/session transport, cost metadata, NDJSON streaming, and named-pipe alternate transport are represented in current docs.
- Broader socket/multi-client transport remains deferred.

Remaining work:
- Do not reopen K1-K5 as if the extraction is still active.
- Use the callable/transpiler handoff plan for later backend/kernel-registration work.

Key references:
- `spec_intake/CliBridgeV2_GpuSampleFn_SpecIntake.md`
- `docs/notes/callable_engine_surface_wrap_PHASED_PLAN.md`
- `tests/test_fractal_runtime_session.py`

## 7. CLI Bridge V2 Multi-Client / Socket Transport Follow-On

Status: deferred.

Why paused:
- V2-G landed as a bounded Windows named-pipe transport for one external client/session per process.
- The broader multi-client pipe fan-in or socket transport surface is not shipped.

Resume constraints:
- Do not advertise concurrent callers until the runtime supports multiple simultaneous clients or a socket equivalent.
- Add focused tests for per-client session isolation and concurrent connect/close behavior.
- Preserve the JSON line protocol and fail-fast semantics.

## 8. Generic CUDA Equation Pack Feature Vertical

Status: paused after initial product vertical; high strategic reward, medium/high difficulty.

Current shipped state:
- v1 AST packs, parser/lowerer, workbench scaffolding, normal `generic_equation_pack` dropdown lane, left Controls JSON/control flow, `SampleGenericFunction` main-viewport bridge, no-mouse automation, and Color Pipeline-backed viewport coloring are shipped.
- This is real formula execution through the existing Generic CUDA evaluator.
- Dynamic CUDA kernel registration, Salticid `sample_fn` lowering, and shared-CUDA ABI integration are not shipped.

Resume order:
1. Persistence: save and reload the active pack with viewer state.
2. Catalog: curated pack picker instead of requiring JSON paste every session.
3. Authoring UX: safer text/editor surface that still lowers to v1 AST pack authority.
4. Salticid adapter: lower the limited expression/composition surface into this schema.
5. Performance: profile where `SampleGenericFunction` is enough and where compiled kernels are required.
6. Mainline merge work: map schema/descriptor contracts into Salticid/shared-CUDA architecture.

Key reference:
- `docs/notes/generic_cuda_equation_pack_PAUSE_README.md`

## 9. Recommended Next Campaigns

1. Top-five backlog campaign: diagnostics capture output path, Lens SDF control truth/SDF field substrate seed, categorized selector/view presets, camera/dive behavior, and smooth-escape per-family tuning. Planning surface: `docs/notes/top_five_backlog_campaign_PHASED_PLAN.md`.
2. Viewer product polish campaign: any selector/view-preset follow-ons, startup/default choice, and camera/dive refinements not closed by the top-five campaign.
3. Visual tuning campaign: any smooth-escape or interior-treatment follow-ons not closed by the top-five campaign, without broad Color Pipeline redesign.
4. Measurement campaign if needed: end-to-end responsiveness telemetry before any more FPS/debounce policy changes.
5. Equation-pack productization campaign: persistence, catalog, authoring UX, then Salticid adapter boundary.
6. Engine substrate campaign: family authority refactor, perturbation proof expansion, then new-substrate fractals.

## 10. Explaino Replay Proof Expansion Follow-On

Status: deferred.

Current shipped state:
- Persisted `sidecar_mutation_history` round-trips through `state.json`, finding/state load, and headless/live mutation recording.
- Headless replay can deterministically reapply ordered persisted parameter targets via `--sidecar-replay-mutation-history-count`.
- Replay fails fast on out-of-range replay counts and preserves the persisted payload during headless capture.

Deferred follow-ups:
- Frame-delta replay proof.
- Live-window replay proof.

Resume constraints:
- Treat this as a new bounded thread, not a reopening of a half-finished feature.
- Prove frame-delta behavior with deterministic artifact or hash-based harness before attempting live-window proof.
- Keep `state.json` as the only replay authority.
