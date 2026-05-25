# Spec Intake - Status Index

Last updated: 2026-05-23

## Implemented

These specs are fully landed on the branch with tests, schema bindings, and probe coverage.

| Spec | Summary | Key Commits |
|------|---------|-------------|
| ExplainoFamilyExpansion_V1 | explaino_nova, explaino_dual, explaino_lambda, explaino_rational_escape + ripple, splice, vortex, tension | multiple |
| NonIntegerMultibrot_V1 | multibrot real exponent accepts `[0.01,32]` with normal UI range `[0.01,12]`, plus `multibrot_power_imag` complex exponent support `[-4,4]` | QoL multibrot exponent slice |
| NovaFractal_V1 | Nova escape-time contract (coloring bug fixed) | Nova repair slice |
| PhoenixFractal_V1 | Phoenix with runtime-backed `phoenix_p_real` / `phoenix_p_imag` controls; named preset library remains future polish | parameter functionality campaign |
| TranscendentalNewtonPresets_V1 | sin, exp-1, cosh Newton presets | early branch |
| FractalCatalog_WaveTwo | Lambda, Explaino-Lambda, Explaino-Rational-Escape | wave-two commits |
| FractalTypeDropdown_and_MultiFractalKernel | Multi-fractal kernel routing + grouped selector foundation | schema extraction |
| GenericCudaSamplerBridge | Probe sampling API for all currently supported types | probe/pipeline commits |
| RealtimeCliSampling_OperatorCallIn | Headless --sample-request-stdin/stdout contract | headless mode extraction |
| OptimizationStaging_ExplainoZeroAxis | Zero-axis measurement, benchmark/sensitivity exposure, variant crossfade, and internal composed Explaino routing | multiple |
| ExplainoAll_Reflexive | Reflexive sidecar, direct mutation, runtime proof closure, and continuity audit | multiple |
| ExplainoSidecarMutationReplay | Persisted sidecar mutation history plus deterministic ordered headless parameter replay from loaded `state.json`; future frame-delta/live replay proof is deferred | multiple |
| HeadlessExplorationAdvisor | Explaino-only `--explore-recommend-json` report mode that emits a deterministic next-best-observation artifact from the existing sidecar intelligence seams | advisor slice |
| LambdaQuadraticConjugacy_DesignNote | Lambda kept as `FractalType::lambda_map` because `lambda` is reserved | Lambda commits |
| CommonFractalCatalog_Deferred_2026-04-06 | Wave 1 plus exposed exploration controls: Spider feedback, Collatz transition strength, and fold/mix controls for Burning Ship, Celtic Mandelbrot, and Perpendicular Burning Ship | parameter functionality campaign |
| FractalToolkit_MagnetWave | Magnet Type I landed as the first modular fractal-toolkit slice with direct escape-time formula seams, controls, sample/runtime proof, and scan guardrails | Magnet toolkit slice |
| GenericCudaEquationPack_V1 | v1 AST packs, native lowerer, workbench, normal dropdown lane, left Controls JSON flow, main viewport bridge through `SampleGenericFunction`, and Color Pipeline-backed viewport coloring | equation-pack vertical |
| No_Implicit_Fallback_General_Directive | Fail-fast for unknown bindings/enums/params | adopted throughout |

## Deferred - Future Fractal Toolkit Branch

These are planned for dedicated future branches, not this docs reconciliation slice.

| Spec | Summary | Notes |
|------|---------|-------|
| FractalCatalog_AdvancedAdditions | Lyapunov, attractors, IFS, distance estimators, Mandelbulb | Needs architecture changes and separate render/state contracts |
| PerturbationDeepZoomExpansion | Extend precision beyond Mandelbrot / Julia | Start with recurrence-specific proof, likely Mandelbrot/Multibrot first |
| Color_Pipeline_TunePass_Directive | Escape-time smoothing, exposure refinement, per-family color tuning | Future polish phase; do not fold into unrelated schema/control work |
| VisualTuning_PostProcess | Sharpen, bloom, depth-of-field | Spec-only; do not implement yet |
| Camera_Depth_UILayout_Notes | Dive-depth tuning, camera behavior stubs, view preset UX | Documented in `KNOWN_ISSUES.md` and `DEFERRED_THREADS.md` |
| GenericEquationPack_Productization | Pack persistence, catalog picker, authoring UX, Salticid adapter, performance profiling | Resume from `docs/notes/generic_cuda_equation_pack_PAUSE_README.md` |
| CliBridgeV2_MultiClientSocket | Multi-client pipe fan-in or socket transport | V2-G one-client named-pipe transport is shipped; concurrency is not |

## Planning - Active Or Follow-Up Specs

| Spec | Summary | Dependencies |
|------|---------|-------------|
| FractalExtensions_PoC | PoC phases are complete: manifest-driven `fractal_extensions` can capture prepared scene states, attach `generic.sample` math sidecars, run analysis, and produce gallery proof. The remaining decision is whether repeated gallery-proof audits need a dedicated checked-in reporter script. | Builds on GenericCudaSamplerBridge, RealtimeCliSampling_OperatorCallIn, finding_capture, and finding_analyzer; current stop point is in `docs/notes/fractal_extensions_poc_PHASED_PLAN.md` |
| CallableEngineSurfaceWrap | Active callable-surface generalization thread: advisor stdout symmetry, callable reference docs, and built-in callable registry authority are landed. The next phase is the handoff boundary into the later transpiler/kernel-registration thread. | Builds on RealtimeCliSampling_OperatorCallIn, GenericCudaSamplerBridge, and HeadlessExplorationAdvisor; backend execution is not yet dynamic kernel registration |
| CliBridgeV2_GpuSampleFn | **K1-K5 ALL DONE. V2-A/V2-B/V2-C/V2-D/V2-E/V2-F/V2-G DONE.** CLI session V2 has batch, keep-alive, state-token diffing, response cost metadata, NDJSON streaming, describe-functions sensitivity metadata, and Windows named-pipe alternate transport. Broader multi-client/socket transport remains deferred. | No remaining active phases in this repo; broader transport stays deferred |
| FractalControlSurfaceRepairCampaign | Phase 1 animation applicability is closed. Remaining phases are descriptor/test-harness expansion, high-confidence Julia/Nova control-surface policy, and broader no-mouse proof expansion. Later parameter campaign work closed many related issues but did not automatically close every phase in that older campaign plan. | See `docs/notes/fractal_control_surface_repair_campaign_PHASED_PLAN.md` and the parameter functionality campaign docs |
| DeferredBacklogReprioritization | Current docs-only reconciliation of deferred issues by difficulty-to-reward ratio. | See `docs/notes/deferred_backlog_reprioritization_PHASED_PLAN.md` |
| FractalCatalogAuthoritySlices | Prepared Slice A and Slice B for catalog/default authority cleanup before further catalog growth or perturbation expansion. | Slice A: `docs/notes/fractal_catalog_authority_inventory_PHASED_PLAN.md`; Slice B: `docs/notes/fractal_view_defaults_catalog_migration_PHASED_PLAN.md` |
| SdfFieldPackNearTermPlanning | SDF modernization roadmap is partially shipped: Lens SDF control truth, scalar field authority, Lens semantics, authored SDF pack CPU reference, CUDA evaluator/hardening, GPU Lens SDF backend, flashlight/runtime-walk headless/report SDF signal consumption, live Color Pipeline SDF source rows, Capture Finding parity, normal viewport SDF overlay, and SDF Source row customization are done. Active follow-up: capture/replay authority smoke matrix for Lens-backed SDF Color Pipeline captures. Deferred: phase-signal metadata, authored-pack UI/live viewport integration, and SDF-native fractal lanes. | See `docs/notes/sdf_field_pack_near_term_TODO.md` and `docs/notes/sdf_field_pack_near_term_planning_PHASED_PLAN.md` |
| TopFiveBacklogCampaignPlanning | Detailed docs-only plan for the next five linear cleanup/polish items: diagnostics capture output paths, Lens SDF truth/SDF field substrate, categorized selector/view presets, camera/dive behavior, and smooth-escape/color tuning. | See `docs/notes/top_five_backlog_campaign_PHASED_PLAN.md` |

### K4 Diagnostic Findings (investigation backlog, prioritized)

| ID | Finding | Priority | Disposition |
|----|---------|----------|------------|
| KF-1 | explaino_y: 0 avg iters, residual up to 27, 100% convergence - degenerate early-exit? | **High** | **DONE** (816b5d1) - root-snap + residual zero + regression test |
| KF-2 | collatz RGBA: fast-escape pixels render black | Low | **DONE** - repaired by `docs/notes/smooth_escape_collatz_luma_PHASED_PLAN.md`; published inventory now reports `collatz` and `explaino_collatz_direct` black fraction `0.000` |
| KF-3 | "neither" band: 3-12% max_iter exhaustion across escape types | Deferred | Optimization/color tuning target for future phase |
| KF-4 | nova/explaino_nova: 99.5% escape at 1 avg iter | Low | Parameter/default tuning - not a control-authority bug |
| KF-5 | lambda_map/explaino_lambda: degenerate at default params | Low | Same as KF-4 |

## Research / Hypothesis - Deferred to Nine

These are research artifacts managed from the `nine` repo or future dedicated research branches.

| Spec | Summary | Notes |
|------|---------|-------|
| ExplainoJoy_6Plus1_HypothesisLadder | 6+1 morphology hypothesis, FITS invariance | Needs experimental validation |
| FitsSolutionSpacePlayback_DesignNote | FITS replay as solved state-space artifact | Next phase after invariance study |
| ExplainoDesignSpace_DeepDive | Observation + family axes for Explaino work | Informs future expansion; not current implementation authority |

## Planning Reference - Completed / Historical

These specs served their planning purpose and are retained as context.

| Spec | Summary | Notes |
|------|---------|-------|
| ViewerHost_Backport_FeatureMatrix | Surgical backport plan from stale branch | Completed; see SEED_REFACTOR_RECONCILIATION |
| CudaCodeQualityCleanup_2026-04-07 | Code quality ratchet plan | Partially complete; current score and baselines come from bootstrap/code_quality_audit |
| SlimeCppStructuralAnalysis_MainlinePrework | Mainline `salticid-cuda` analysis | Out of scope for this repo |
