#pragma once

#include "fractal_types.h"

// Per-family tier support query.
uint32_t GetSampleTierSupport(FractalType ft);

// Central capability resolver: maps (family, requested tier, zoom) to a
// concrete (backend, strategy) pair.  If the requested tier is not supported
// for the given family, the resolver degrades cleanly to the best available.
ResolvedEvalMode ResolveSampleEvalMode(
    FractalType fractalType,
    SampleTier requestedTier,
    double log2Zoom);
