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
        fractalType == FractalType::explaino_nova;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool SupportsBasinColoring(FractalType fractalType) {
    return fractalType == FractalType::newton ||
        fractalType == FractalType::explaino ||
        fractalType == FractalType::explaino_y ||
        fractalType == FractalType::explaino_fp;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool IsEscapeTimeFamily(FractalType fractalType) {
    return fractalType == FractalType::nova ||
        fractalType == FractalType::mandelbrot ||
        fractalType == FractalType::julia ||
        fractalType == FractalType::burning_ship ||
        fractalType == FractalType::multibrot ||
        fractalType == FractalType::phoenix ||
        fractalType == FractalType::explaino_nova;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool IsColoringModeAllowedForFractal(FractalType fractalType, ColoringMode mode) {
    if (SupportsBasinColoring(fractalType)) return true;
    return mode != ColoringMode::root_basin && mode != ColoringMode::joy_basins;
}

FRACTAL_FAMILY_RULES_HD inline constexpr ColoringMode DefaultColoringModeForFractal(FractalType fractalType) {
    return SupportsBasinColoring(fractalType) ? ColoringMode::joy_basins : ColoringMode::smooth_escape;
}

#undef FRACTAL_FAMILY_RULES_HD