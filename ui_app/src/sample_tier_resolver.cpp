#include "sample_tier_resolver.h"
#include "fractal_family_rules.h"

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
    // At deep zoom (log2_zoom > 20, roughly 10^6x), switch to float64.
    if (requestedTier == SampleTier::tier_auto) {
        if ((support & kSupport_Standard) && log2Zoom > 20.0) {
            return {NumericBackend::float64, IterationStrategy::direct};
        }
        return {NumericBackend::float32, IterationStrategy::direct};
    }

    // Unknown tier: degrade to fast.
    return {NumericBackend::float32, IterationStrategy::direct};
}
