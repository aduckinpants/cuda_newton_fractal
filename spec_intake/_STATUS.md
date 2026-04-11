# Spec Intake — Status Index

Last updated: 2026-04-10

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
| OptimizationStaging_ExplainoZeroAxis | Zero-axis measurement, cost/sensitivity tables, variant crossfade; Phase 1 feeds kernel extraction validation | No external deps (Phase 1); CUDA sample_fn K1-K3 (Phase 2) |
| CliBridgeV2_GpuSampleFn | **K1-K5 ALL DONE. V2-A/V2-B/V2-C/V2-D/V2-E/V2-G DONE.** CLI session V2 now has batch, keep-alive, state-token diffing, response cost metadata, NDJSON streaming for single/session requests, and a Windows named-pipe alternate transport. | V2-F (describe with sensitivity) is the remaining active CLI Bridge V2 slice and still depends on Optimization Staging Phase 2 |
| ExplainoAll_Reflexive | Engine explaining itself: CarlBrain single-tick instance over param manifold; in-process CUDA sample calls | CUDA sample_fn K1-K3 (done) + Optimization Staging Phase 2 |

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
