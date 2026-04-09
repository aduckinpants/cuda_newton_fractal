#pragma once

#include "fractal_types.h"

#if defined(__CUDACC__)
#define FRACTAL_FAMILY_RULES_HD __host__ __device__
#else
#define FRACTAL_FAMILY_RULES_HD
#endif

FRACTAL_FAMILY_RULES_HD inline constexpr bool IsExplainoFamily(FractalType fractalType) {
    return fractalType == FractalType::explaino ||
        fractalType == FractalType::explaino_y ||
        fractalType == FractalType::explaino_fp ||
        fractalType == FractalType::explaino_nova ||
    fractalType == FractalType::explaino_halley ||
    fractalType == FractalType::explaino_dual ||
    fractalType == FractalType::explaino_mult ||
    fractalType == FractalType::explaino_phoenix ||
    fractalType == FractalType::explaino_transcendental ||
    fractalType == FractalType::explaino_inertial ||
    fractalType == FractalType::explaino_julia ||
    fractalType == FractalType::explaino_rational ||
    fractalType == FractalType::explaino_collatz ||
    fractalType == FractalType::explaino_lambda ||
    fractalType == FractalType::explaino_rational_escape ||
    fractalType == FractalType::explaino_joy ||
    fractalType == FractalType::explaino_fold;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool SupportsBasinColoring(FractalType fractalType) {
    return fractalType == FractalType::newton ||
        fractalType == FractalType::explaino ||
        fractalType == FractalType::explaino_y ||
        fractalType == FractalType::explaino_fp ||
    fractalType == FractalType::explaino_halley ||
    fractalType == FractalType::explaino_dual ||
    fractalType == FractalType::explaino_mult ||
    fractalType == FractalType::explaino_phoenix ||
    fractalType == FractalType::explaino_transcendental ||
    fractalType == FractalType::explaino_inertial ||
    fractalType == FractalType::explaino_rational ||
    fractalType == FractalType::explaino_collatz ||
    fractalType == FractalType::explaino_joy ||
    fractalType == FractalType::explaino_fold ||
    fractalType == FractalType::halley;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool IsEscapeTimeFamily(FractalType fractalType) {
    return fractalType == FractalType::nova ||
        fractalType == FractalType::mandelbrot ||
        fractalType == FractalType::julia ||
        fractalType == FractalType::burning_ship ||
        fractalType == FractalType::multibrot ||
        fractalType == FractalType::phoenix ||
        fractalType == FractalType::explaino_nova ||
        fractalType == FractalType::explaino_julia ||
        fractalType == FractalType::multicorn ||
        fractalType == FractalType::collatz ||
    fractalType == FractalType::mcmullen ||
    fractalType == FractalType::lambda_map ||
    fractalType == FractalType::explaino_lambda ||
        fractalType == FractalType::explaino_rational_escape ||
        fractalType == FractalType::spider ||
        fractalType == FractalType::celtic_mandelbrot ||
        fractalType == FractalType::perpendicular_burning_ship;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool LensMaskInsideForFractal(FractalType fractalType, bool converged, bool escaped) {
    if (SupportsBasinColoring(fractalType)) return converged;
    if (IsEscapeTimeFamily(fractalType)) return !escaped;
    return false;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool IsColoringModeAllowedForFractal(FractalType fractalType, ColoringMode mode) {
    if (SupportsBasinColoring(fractalType)) return true;
    return mode != ColoringMode::root_basin && mode != ColoringMode::joy_basins;
}

FRACTAL_FAMILY_RULES_HD inline constexpr ColoringMode DefaultColoringModeForFractal(FractalType fractalType) {
    return SupportsBasinColoring(fractalType) ? ColoringMode::joy_basins : ColoringMode::smooth_escape;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool DefaultAutoMaxIterForFractal(FractalType fractalType) {
    return fractalType == FractalType::nova || fractalType == FractalType::explaino_nova;
}

// Auto max-iter: scale iterations with zoom depth and fractal family.
// Escape-time fractals need more iterations at depth; basin types less so.
inline int ComputeAutoMaxIter(double log2_zoom, FractalType fractalType) {
    double depth = log2_zoom < 0.0 ? -log2_zoom : log2_zoom;
    int base, scale;
    if (fractalType == FractalType::collatz || fractalType == FractalType::explaino_collatz) {
        base = 300; scale = 80;
    } else if (fractalType == FractalType::nova || fractalType == FractalType::explaino_nova) {
        base = 400; scale = 120;
    } else if (IsEscapeTimeFamily(fractalType)) {
        base = 200; scale = 50;
    } else {
        base = 150; scale = 30;
    }
    int result = base + static_cast<int>(scale * depth);
    if (result < 100) result = 100;
    if (result > 5000) result = 5000;
    return result;
}

#undef FRACTAL_FAMILY_RULES_HD