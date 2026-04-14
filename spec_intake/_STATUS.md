# Spec Intake — Status Index

Last updated: 2026-04-13

## Implemented

These specs are fully landed on the branch with tests, schema bindings, and probe coverage.

| Spec | Summary | Key Commits |
|------|---------|-------------|
| ExplainoFamilyExpansion_V1 | explaino_nova, explaino_dual, explaino_lambda, explaino_rational_escape + ripple, splice, vortex, tension | multiple |
| NonIntegerMultibrot_V1 | multibrot_power_float with [2,12] domain | landed 2026-04-06 |
| NovaFractal_V1 | Nova escape-time contract (coloring bug fixed) | Nova repair slice |
| PhoenixFractal_V1 | Phoenix with single preset (p=0.5667) | early branch |
| TranscendentalNewtonPresets_V1 | sin, exp-1, cosh Newton presets | early branch |
| FractalCatalog_WaveTwo | Lambda, Explaino-Lambda, Explaino-Rational-Escape | wave-two commits |
| FractalTypeDropdown_and_MultiFractalKernel | Multi-fractal kernel routing + grouped selector | schema extraction |
| GenericCudaSamplerBridge | Probe sampling API for all 27 types | probe/pipeline commits |
| RealtimeCliSampling_OperatorCallIn | Headless --sample-request-stdin/stdout contract | headless mode extraction |
| OptimizationStaging_ExplainoZeroAxis | Zero-axis measurement, benchmark/sensitivity exposure, variant crossfade, and internal composed Explaino routing | multiple |
| ExplainoAll_Reflexive | Reflexive sidecar, direct mutation, runtime proof closure, and continuity audit | multiple |
| ExplainoSidecarMutationReplay | Persisted sidecar mutation history plus deterministic ordered headless parameter replay from loaded `state.json`; future frame-delta/live replay proof is deferred | multiple |
| HeadlessExplorationAdvisor | Explaino-only `--explore-recommend-json` report mode that emits a deterministic next-best-observation artifact from the existing sidecar intelligence seams | advisor slice |
| LambdaQuadraticConjugacy_DesignNote | Lambda kept as FractalType::lambda_map (reserved word) | Lambda commits |
| CommonFractalCatalog_Deferred_2026-04-06 | Wave 1: Spider, Celtic MB, Perpendicular BS | common-fractal wave |
| No_Implicit_Fallback_General_Directive | Fail-fast for unknown bindings/enums/params | adopted throughout |

## Deferred — Future Fractal Toolkit Branch

These are planned for a dedicated fractal-toolkit branch, not this merge.

| Spec | Summary | Notes |
|------|---------|-------|
| FractalCatalog_AdvancedAdditions | Lyapunov, attractors, IFS, distance estimators, Mandelbulb | Needs architecture changes (CUDA refactor gate) |
| Color_Pipeline_TunePass_Directive | Escape-time smoothing, exposure refinement | Future polish phase |
| VisualTuning_PostProcess | Sharpen, bloom, depth-of-field | Spec-only; do not implement yet |
| Camera_Depth_UILayout_Notes | Dive-depth tuning, camera behavior stubs | Documented in KNOWN_ISSUES |

## Planning — Active Specs (need refinement before implementation)

| Spec | Summary | Dependencies |
|------|---------|-------------|
| CallableEngineSurfaceWrap | Active callable-surface generalization thread: advisor stdout symmetry plus the current callable-surface reference doc are landed, and Phase 3 now has a built-in callable registry acting as the single authority for shipped `function_id` values across describe-functions and probe dispatch. Remaining near-term work is to turn that static registry into the explicit generalization surface for more built-in callables without pretending dynamic kernel registration already exists yet. | Builds on RealtimeCliSampling_OperatorCallIn, GenericCudaSamplerBridge, and HeadlessExplorationAdvisor; near-term follow-on is registry/function-id generalization, not transpiler/backend execution yet |
| CliBridgeV2_GpuSampleFn | **K1-K5 ALL DONE. V2-A/V2-B/V2-C/V2-D/V2-E/V2-F/V2-G DONE.** CLI session V2 now has batch, keep-alive, state-token diffing, response cost metadata, NDJSON streaming for single/session requests, describe-functions sensitivity metadata, and a Windows named-pipe alternate transport. Broader multi-client/socket transport remains deferred. | No remaining active phases in this repo; broader multi-client/socket transport stays deferred |

### K4 Diagnostic Findings (investigation backlog, prioritized)

| ID | Finding | Priority | Disposition |
|----|---------|----------|------------|
| KF-1 | explaino_y: 0 avg iters, residual up to 27, 100% convergence — degenerate early-exit? | **High** | **DONE** (816b5d1) — root-snap + residual zero + regression test |
| KF-2 | collatz RGBA: fast-escape pixels render black | Low | Cosmetic; not a sample-path bug |
| KF-3 | "neither" band: 3-12% max_iter exhaustion across escape types | Deferred | Optimization target for future phase |
| KF-4 | nova/explaino_nova: 99.5% escape at 1 avg iter | Low | Parameter tuning — defaults may not showcase fractal |
| KF-5 | lambda_map/explaino_lambda: degenerate at default params | Low | Same as KF-4 |

## Research / Hypothesis — Deferred to Nine

These are research artifacts managed from the `nine` repo.

| Spec | Summary | Notes |
|------|---------|-------|
| ExplainoJoy_6Plus1_HypothesisLadder | 6+1 morphology hypothesis, FITS invariance | Needs experimental validation |
| FitsSolutionSpacePlayback_DesignNote | FITS replay as solved state-space artifact | Next phase after invariance study |
| ExplainoDesignSpace_DeepDive | Observation + family axes for Explaino work | Informs future expansion |

## Planning Reference — Completed / Historical

These specs served their planning purpose and are retained as context.

| Spec | Summary | Notes |
|------|---------|-------|
| ViewerHost_Backport_FeatureMatrix | Surgical backport plan from stale branch | Completed; see SEED_REFACTOR_RECONCILIATION |
| CudaCodeQualityCleanup_2026-04-07 | Active ratchet plan; consult code_quality_audit | Partially complete (score 95→ongoing) |
| SlimeCppStructuralAnalysis_MainlinePrework | Mainline salticid-cuda analysis | Out of scope for this repo |
