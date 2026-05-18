#include "sample_tier_resolver.h"
#include "fractal_family_rules.h"

namespace {

bool BasinColorSignalNeedsStandard(ColorSignal signal) {
    switch (signal) {
    case ColorSignal::smooth_escape:
    case ColorSignal::root_proximity:
        return true;
    default:
        return false;
    }
}

bool BasinColoringNeedsStandard(FractalType fractalType, const KernelParams& params) {
    return SupportsBasinColoring(fractalType) &&
        params.coloring_mode == ColoringMode::smooth_escape &&
        BasinColorSignalNeedsStandard(params.color_pipeline.signal);
}

bool ExplainoLegacyProjectionSmoothEscapeStaysFast(
    FractalType requestedFractalType,
    FractalType runtimeFractalType,
    const KernelParams& params) {
    if (params.coloring_mode != ColoringMode::smooth_escape ||
        params.color_pipeline.signal != ColorSignal::smooth_escape) {
        return false;
    }
    return requestedFractalType == runtimeFractalType &&
        IsExplainoLegacyProjectionSelector(requestedFractalType);
}

double AutoStandardThresholdLog2(FractalType fractalType) {
    switch (fractalType) {
    case FractalType::julia:
    case FractalType::explaino_julia:
        return 16.0;
    case FractalType::mandelbrot:
    case FractalType::burning_ship:
    case FractalType::multibrot:
    case FractalType::phoenix:
    case FractalType::multicorn:
    case FractalType::mcmullen:
    case FractalType::lambda_map:
    case FractalType::collatz:
    case FractalType::explaino_lambda:
    case FractalType::explaino_phoenix:
    case FractalType::explaino_rational_escape:
        return 18.0;
    default:
        return 20.0;
    }
}

} // namespace

uint32_t GetSampleTierSupport(FractalType ft) {
    // Every family supports fast (float32 direct) — that is the existing path.
    uint32_t flags = kSupport_Fast;

    // Phase A: Newton-family and basin-coloring families gain float64 direct.
    // Escape-time families that already have perturbation (mandelbrot, julia)
    // also support standard via float64 direct, though their deep-zoom path
    // will eventually be perturbation (Phase B).
    if (SupportsBasinColoring(ft)) {
        // All Newton/Explaino root-finding families.
        flags |= kSupport_Standard;
    }

    // Escape-time families: standard (float64 direct) for all.
    // Their existing perturbation path is handled separately (not tier-gated yet).
    if (!SupportsBasinColoring(ft)) {
        flags |= kSupport_Standard;
    }

    return flags;
}

ResolvedEvalMode ResolveSampleEvalMode(
    FractalType fractalType,
    SampleTier requestedTier,
    double log2Zoom)
{
    uint32_t support = GetSampleTierSupport(fractalType);

    // Resolve the requested tier.
    if (requestedTier == SampleTier::fast) {
        return {NumericBackend::float32, IterationStrategy::direct};
    }

    if (requestedTier == SampleTier::standard) {
        if (support & kSupport_Standard) {
            return {NumericBackend::float64, IterationStrategy::direct};
        }
        // Degrade to fast if standard not supported.
        return {NumericBackend::float32, IterationStrategy::direct};
    }

    // tier_auto: select based on zoom depth.
    // At moderate zoom, float32 is sufficient and faster.
    // Escape-time families need to promote earlier than Newton-family paths,
    // especially Julia and Mandelbrot exploration views where smear becomes
    // visible before the old one-size-fits-all threshold.
    if (requestedTier == SampleTier::tier_auto) {
        if ((support & kSupport_Standard) && log2Zoom > AutoStandardThresholdLog2(fractalType)) {
            return {NumericBackend::float64, IterationStrategy::direct};
        }
        return {NumericBackend::float32, IterationStrategy::direct};
    }

    // Unknown tier: degrade to fast.
    return {NumericBackend::float32, IterationStrategy::direct};
}

ResolvedEvalMode ResolveSampleEvalModeForRender(
    FractalType fractalType,
    const KernelParams& params,
    SampleTier requestedTier,
    double log2Zoom)
{
    const FractalType runtimeFractalType = ResolveExplainoRuntimeFractalType(fractalType, params);
    const uint32_t support = GetSampleTierSupport(runtimeFractalType);
    if (requestedTier == SampleTier::tier_auto &&
        ExplainoLegacyProjectionSmoothEscapeStaysFast(fractalType, runtimeFractalType, params)) {
        return {NumericBackend::float32, IterationStrategy::direct};
    }
    if (requestedTier == SampleTier::tier_auto &&
        (support & kSupport_Standard) &&
        BasinColoringNeedsStandard(runtimeFractalType, params)) {
        return {NumericBackend::float64, IterationStrategy::direct};
    }
    return ResolveSampleEvalMode(runtimeFractalType, requestedTier, log2Zoom);
}
