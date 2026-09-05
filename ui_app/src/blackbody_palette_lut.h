#pragma once

#include "fractal_types.h"

#include <cmath>

struct BlackbodyPaletteLutRgb {
    float r;
    float g;
    float b;
};

#include "../../docs/reference/blackbody_chromaticity/blackbody_palette_lut_v1.inc"

inline constexpr BlackbodyPaletteLutRgb kBlackbodyPaletteLutV1Host[BLACKBODY_PALETTE_LUT_V1_ENTRY_COUNT] =
    BLACKBODY_PALETTE_LUT_V1_INITIALIZER;

#if defined(__CUDACC__)
static __device__ __constant__ BlackbodyPaletteLutRgb kBlackbodyPaletteLutV1Device[BLACKBODY_PALETTE_LUT_V1_ENTRY_COUNT] =
    BLACKBODY_PALETTE_LUT_V1_INITIALIZER;
#endif

#if defined(__CUDACC__)
#define BLACKBODY_PALETTE_HD __host__ __device__
#else
#define BLACKBODY_PALETTE_HD
#endif

inline constexpr float kBlackbodyTemperatureMinK = 800.0f;
inline constexpr float kBlackbodyTemperatureMaxK = 40000.0f;

BLACKBODY_PALETTE_HD inline float ClampBlackbodyTemperature(float value, float fallback) {
    if (!(value >= kBlackbodyTemperatureMinK && value <= kBlackbodyTemperatureMaxK)) {
        return fallback;
    }
    return value;
}

BLACKBODY_PALETTE_HD inline float BlackbodyPaletteLutCoordinateForTemperature(float temperatureK) {
    const float bounded = ClampBlackbodyTemperature(temperatureK, kBlackbodyTemperatureMinK);
    const float reciprocalMin = 1.0f / kBlackbodyTemperatureMinK;
    const float reciprocalMax = 1.0f / kBlackbodyTemperatureMaxK;
    const float q = ((1.0f / bounded) - reciprocalMin) / (reciprocalMax - reciprocalMin);
    return q * static_cast<float>(BLACKBODY_PALETTE_LUT_V1_ENTRY_COUNT - 1);
}

inline void RebuildBlackbodyPaletteRuntimeCache(ColorPipelinePaletteRuntimeParams* params) {
    if (!params) return;
    params->blackbody_temperature_0_k = ClampBlackbodyTemperature(params->blackbody_temperature_0_k, 1600.0f);
    params->blackbody_temperature_1_k = ClampBlackbodyTemperature(params->blackbody_temperature_1_k, 12000.0f);
    params->blackbody_lut_x0 = BlackbodyPaletteLutCoordinateForTemperature(params->blackbody_temperature_0_k);
    params->blackbody_lut_x1 = BlackbodyPaletteLutCoordinateForTemperature(params->blackbody_temperature_1_k);
    params->blackbody_lut_dx = params->blackbody_lut_x1 - params->blackbody_lut_x0;
}

BLACKBODY_PALETTE_HD inline BlackbodyPaletteLutRgb SampleBlackbodyPaletteRuntime(
    float signalValue,
    const ColorPipelinePaletteRuntimeParams& params) {
    const float input = isfinite(signalValue) ? fminf(1.0f, fmaxf(0.0f, signalValue)) : 0.0f;
    float coordinate = 0.0f;
    if (params.blackbody_temperature_mapping == BlackbodyTemperatureMapping::reciprocal_temperature) {
        coordinate = fmaf(input, params.blackbody_lut_dx, params.blackbody_lut_x0);
    } else {
        const float temperature = fmaf(
            input,
            params.blackbody_temperature_1_k - params.blackbody_temperature_0_k,
            params.blackbody_temperature_0_k);
        coordinate = BlackbodyPaletteLutCoordinateForTemperature(temperature);
    }
    coordinate = fminf(
        static_cast<float>(BLACKBODY_PALETTE_LUT_V1_ENTRY_COUNT - 1),
        fmaxf(0.0f, coordinate));
    int lower = static_cast<int>(floorf(coordinate));
    if (lower >= BLACKBODY_PALETTE_LUT_V1_ENTRY_COUNT - 1) {
        lower = BLACKBODY_PALETTE_LUT_V1_ENTRY_COUNT - 1;
    }
    const int upper = lower < BLACKBODY_PALETTE_LUT_V1_ENTRY_COUNT - 1 ? lower + 1 : lower;
    const float fraction = coordinate - static_cast<float>(lower);
#if defined(__CUDA_ARCH__)
    const BlackbodyPaletteLutRgb low = kBlackbodyPaletteLutV1Device[lower];
    const BlackbodyPaletteLutRgb high = kBlackbodyPaletteLutV1Device[upper];
#else
    const BlackbodyPaletteLutRgb low = kBlackbodyPaletteLutV1Host[lower];
    const BlackbodyPaletteLutRgb high = kBlackbodyPaletteLutV1Host[upper];
#endif
    return {
        fmaf(fraction, high.r - low.r, low.r),
        fmaf(fraction, high.g - low.g, low.g),
        fmaf(fraction, high.b - low.b, low.b),
    };
}

#undef BLACKBODY_PALETTE_LUT_V1_INITIALIZER
#undef BLACKBODY_PALETTE_HD
