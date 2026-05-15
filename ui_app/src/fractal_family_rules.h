#pragma once

#include "fractal_types.h"

#include <string_view>

#if defined(__CUDACC__)
#define FRACTAL_FAMILY_RULES_HD __host__ __device__
#else
#define FRACTAL_FAMILY_RULES_HD
#endif

enum class ExplainoAxisParamSlot : int {
    ripple_amplitude = 0,
    splice_offset = 1,
    vortex_strength = 2,
    tension_strength = 3,
    balance_void = 4,
    symmetry_tension = 5,
    field_curvature = 6,
};

struct ExplainoAxisDescriptor {
    const char* axis_id;
    const char* binding_path;
    FractalType carrier_fractal_type;
    float default_value;
    ExplainoAxisParamSlot slot;
};

FRACTAL_FAMILY_RULES_HD inline constexpr FractalType ExplainoCanonicalFractalType() {
    return FractalType::explaino_all;
}

inline constexpr ExplainoAxisDescriptor kExplainoAxisRegistry[] = {
    {"ripple_amplitude", "fractal.params.ripple_amplitude", FractalType::explaino_ripple, 0.15f, ExplainoAxisParamSlot::ripple_amplitude},
    {"splice_offset", "fractal.params.splice_offset", FractalType::explaino_splice, 0.5f, ExplainoAxisParamSlot::splice_offset},
    {"vortex_strength", "fractal.params.vortex_strength", FractalType::explaino_vortex, 0.3f, ExplainoAxisParamSlot::vortex_strength},
    {"tension_strength", "fractal.params.tension_strength", FractalType::explaino_tension, 0.02f, ExplainoAxisParamSlot::tension_strength},
    {"balance_void", "fractal.params.balance_void", FractalType::explaino_balance_void, 0.0f, ExplainoAxisParamSlot::balance_void},
    {"symmetry_tension", "fractal.params.symmetry_tension", FractalType::explaino_balance_void, 0.0f, ExplainoAxisParamSlot::symmetry_tension},
    {"field_curvature", "fractal.params.field_curvature", FractalType::explaino_balance_void, 0.0f, ExplainoAxisParamSlot::field_curvature},
};

FRACTAL_FAMILY_RULES_HD inline constexpr bool IsExplainoComposedAxisCarrier(FractalType fractalType) {
    return fractalType == FractalType::explaino_ripple ||
        fractalType == FractalType::explaino_splice ||
        fractalType == FractalType::explaino_vortex ||
        fractalType == FractalType::explaino_tension;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool HasExplainoComposedAxisPerturbation(const KernelParams& params) {
    return params.ripple_amplitude != 0.0f ||
        params.splice_offset != 0.0f ||
        params.vortex_strength != 0.0f ||
        params.tension_strength != 0.0f;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool HasExplainoBalanceVoidPerturbation(const KernelParams& params) {
    return params.balance_void != 0.0f ||
        params.symmetry_tension != 0.0f ||
        params.field_curvature != 0.0f;
}

FRACTAL_FAMILY_RULES_HD inline constexpr FractalType ResolveExplainoRuntimeFractalType(
    FractalType fractalType,
    const KernelParams& params) {
    if (fractalType == ExplainoCanonicalFractalType()) {
        if (HasExplainoComposedAxisPerturbation(params)) {
            return FractalType::explaino_ripple;
        }
        if (HasExplainoBalanceVoidPerturbation(params)) {
            return FractalType::explaino_balance_void;
        }
        return FractalType::explaino;
    }
    if (IsExplainoComposedAxisCarrier(fractalType) && !HasExplainoComposedAxisPerturbation(params)) {
        return FractalType::explaino;
    }
    if (fractalType == FractalType::explaino_balance_void && !HasExplainoBalanceVoidPerturbation(params)) {
        return FractalType::explaino;
    }
    return fractalType;
}

inline const ExplainoAxisDescriptor* FindExplainoAxisDescriptor(std::string_view axisId) {
    for (const auto& axis : kExplainoAxisRegistry) {
        if (axisId == axis.axis_id) {
            return &axis;
        }
    }
    return nullptr;
}

inline float* ResolveExplainoAxisValue(KernelParams& params, ExplainoAxisParamSlot slot) {
    switch (slot) {
    case ExplainoAxisParamSlot::ripple_amplitude:
        return &params.ripple_amplitude;
    case ExplainoAxisParamSlot::splice_offset:
        return &params.splice_offset;
    case ExplainoAxisParamSlot::vortex_strength:
        return &params.vortex_strength;
    case ExplainoAxisParamSlot::tension_strength:
        return &params.tension_strength;
    case ExplainoAxisParamSlot::balance_void:
        return &params.balance_void;
    case ExplainoAxisParamSlot::symmetry_tension:
        return &params.symmetry_tension;
    case ExplainoAxisParamSlot::field_curvature:
        return &params.field_curvature;
    }
    return nullptr;
}

inline void ResetExplainoAxisRegistryValues(KernelParams& params) {
    for (const auto& axis : kExplainoAxisRegistry) {
        float* value = ResolveExplainoAxisValue(params, axis.slot);
        if (value) {
            *value = 0.0f;
        }
    }
}

inline void ApplyExplainoAxisRegistryDefaults(FractalType fractalType, KernelParams& params) {
    ResetExplainoAxisRegistryValues(params);
    for (const auto& axis : kExplainoAxisRegistry) {
        if (axis.carrier_fractal_type == fractalType) {
            float* value = ResolveExplainoAxisValue(params, axis.slot);
            if (value) {
                *value = axis.default_value;
            }
        }
    }
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool IsExplainoFamily(FractalType fractalType) {
    return fractalType == ExplainoCanonicalFractalType() ||
        fractalType == FractalType::explaino || fractalType == FractalType::explaino_y ||
        fractalType == FractalType::explaino_fp || fractalType == FractalType::explaino_nova ||
        fractalType == FractalType::explaino_halley || fractalType == FractalType::explaino_dual ||
        fractalType == FractalType::explaino_mult || fractalType == FractalType::explaino_phoenix ||
        fractalType == FractalType::explaino_transcendental || fractalType == FractalType::explaino_inertial ||
        fractalType == FractalType::explaino_julia || fractalType == FractalType::explaino_rational ||
        fractalType == FractalType::explaino_collatz || fractalType == FractalType::explaino_lambda ||
        fractalType == FractalType::explaino_rational_escape || fractalType == FractalType::explaino_joy ||
        fractalType == FractalType::explaino_fold || fractalType == FractalType::explaino_bell ||
        fractalType == FractalType::explaino_ripple || fractalType == FractalType::explaino_splice ||
        fractalType == FractalType::explaino_vortex || fractalType == FractalType::explaino_tension ||
        fractalType == FractalType::explaino_balance_void;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool SupportsBasinColoring(FractalType fractalType) {
    return fractalType == FractalType::newton || fractalType == ExplainoCanonicalFractalType() ||
        fractalType == FractalType::explaino ||
        fractalType == FractalType::explaino_y || fractalType == FractalType::explaino_fp ||
        fractalType == FractalType::explaino_halley || fractalType == FractalType::explaino_dual ||
        fractalType == FractalType::explaino_mult || fractalType == FractalType::explaino_phoenix ||
        fractalType == FractalType::explaino_transcendental || fractalType == FractalType::explaino_inertial ||
        fractalType == FractalType::explaino_rational || fractalType == FractalType::explaino_collatz ||
        fractalType == FractalType::explaino_joy || fractalType == FractalType::explaino_fold ||
        fractalType == FractalType::explaino_bell || fractalType == FractalType::explaino_ripple ||
        fractalType == FractalType::explaino_splice || fractalType == FractalType::explaino_vortex ||
        fractalType == FractalType::explaino_tension || fractalType == FractalType::explaino_balance_void ||
        fractalType == FractalType::halley;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool IsEscapeTimeFamily(FractalType fractalType) {
    return fractalType == FractalType::nova || fractalType == FractalType::mandelbrot ||
        fractalType == FractalType::julia || fractalType == FractalType::burning_ship ||
        fractalType == FractalType::multibrot || fractalType == FractalType::phoenix ||
        fractalType == FractalType::explaino_nova || fractalType == FractalType::explaino_julia ||
        fractalType == FractalType::multicorn || fractalType == FractalType::collatz ||
        fractalType == FractalType::mcmullen || fractalType == FractalType::lambda_map ||
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

FRACTAL_FAMILY_RULES_HD inline constexpr ColorPipelineSelection ColorPipelineForLegacyMode(ColoringMode mode) {
    switch (mode) {
    case ColoringMode::root_basin:
        return {ColorSignal::root_index, ColorPalette::root_classic, ColorGradingPreset::basin_default};
    case ColoringMode::iteration_count:
        return {ColorSignal::iteration_count, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
    case ColoringMode::smooth_escape:
        return {ColorSignal::smooth_escape, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
    case ColoringMode::joy_basins:
        return {ColorSignal::root_index, ColorPalette::joy, ColorGradingPreset::basin_default};
    case ColoringMode::phase:
        return {ColorSignal::phase_angle, ColorPalette::phase_wheel, ColorGradingPreset::phase_default};
    case ColoringMode::iteration_bands:
        return {ColorSignal::iteration_bands, ColorPalette::banded_escape, ColorGradingPreset::bands_default};
    }
    return {};
}

FRACTAL_FAMILY_RULES_HD inline constexpr ColorPipelineSelection DefaultColorPipelineForFractal(FractalType fractalType) {
    return ColorPipelineForLegacyMode(DefaultColoringModeForFractal(fractalType));
}

inline constexpr ColorPipelineSelection kSelectableColorPipelines[] = {
    ColorPipelineForLegacyMode(ColoringMode::root_basin),
    ColorPipelineForLegacyMode(ColoringMode::iteration_count),
    ColorPipelineForLegacyMode(ColoringMode::smooth_escape),
    ColorPipelineForLegacyMode(ColoringMode::joy_basins),
    ColorPipelineForLegacyMode(ColoringMode::phase),
    ColorPipelineForLegacyMode(ColoringMode::iteration_bands),
    {ColorSignal::escape_magnitude, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default},
    {ColorSignal::orbit_stripe, ColorPalette::phase_wheel, ColorGradingPreset::phase_default},
    {ColorSignal::root_proximity, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default},
    {ColorSignal::smooth_escape, ColorPalette::explaino_cmap, ColorGradingPreset::escape_default},
    {ColorSignal::escape_magnitude, ColorPalette::explaino_cmap, ColorGradingPreset::escape_default},
    {ColorSignal::root_proximity, ColorPalette::explaino_cmap, ColorGradingPreset::escape_default},
};

FRACTAL_FAMILY_RULES_HD inline constexpr bool TryLegacyColoringModeForPipeline(
    const ColorPipelineSelection& pipeline,
    ColoringMode* outMode) {
    if (pipeline.signal == ColorSignal::root_index &&
        pipeline.palette == ColorPalette::root_classic &&
        pipeline.grading == ColorGradingPreset::basin_default) {
        if (outMode) *outMode = ColoringMode::root_basin;
        return true;
    }
    const bool isEscapeLikeGrading = pipeline.grading == ColorGradingPreset::escape_default ||
        pipeline.grading == ColorGradingPreset::neutral_default ||
        pipeline.grading == ColorGradingPreset::tone_map_default ||
        pipeline.grading == ColorGradingPreset::glow_default ||
        pipeline.grading == ColorGradingPreset::balance_void_default;
    const bool isPhaseLikeGrading = pipeline.grading == ColorGradingPreset::phase_default ||
        pipeline.grading == ColorGradingPreset::neutral_default ||
        pipeline.grading == ColorGradingPreset::balance_void_default;
    const bool isBandLikeGrading = pipeline.grading == ColorGradingPreset::bands_default ||
        pipeline.grading == ColorGradingPreset::neutral_default ||
        pipeline.grading == ColorGradingPreset::balance_void_default;
    if (pipeline.signal == ColorSignal::iteration_count &&
        pipeline.palette == ColorPalette::cyclic_escape &&
        isEscapeLikeGrading) {
        if (outMode) *outMode = ColoringMode::iteration_count;
        return true;
    }
    if (pipeline.signal == ColorSignal::smooth_escape &&
        pipeline.palette == ColorPalette::cyclic_escape &&
        isEscapeLikeGrading) {
        if (outMode) *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (pipeline.signal == ColorSignal::root_index &&
        pipeline.palette == ColorPalette::joy &&
        pipeline.grading == ColorGradingPreset::basin_default) {
        if (outMode) *outMode = ColoringMode::joy_basins;
        return true;
    }
    if (pipeline.signal == ColorSignal::phase_angle &&
        pipeline.palette == ColorPalette::phase_wheel &&
        isPhaseLikeGrading) {
        if (outMode) *outMode = ColoringMode::phase;
        return true;
    }
    if (pipeline.signal == ColorSignal::iteration_bands &&
        pipeline.palette == ColorPalette::banded_escape &&
        isBandLikeGrading) {
        if (outMode) *outMode = ColoringMode::iteration_bands;
        return true;
    }
    return false;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool IsColorSignalAllowedForFractal(
    FractalType fractalType,
    ColorSignal signal) {
    if (signal == ColorSignal::root_index || signal == ColorSignal::root_proximity) {
        return SupportsBasinColoring(fractalType);
    }
    return true;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool TryMirroredColoringModeForPipeline(
    const ColorPipelineSelection& pipeline,
    ColoringMode* outMode) {
    if (TryLegacyColoringModeForPipeline(pipeline, outMode)) {
        return true;
    }
    const bool isEscapeLikeGrading = pipeline.grading == ColorGradingPreset::escape_default ||
        pipeline.grading == ColorGradingPreset::neutral_default ||
        pipeline.grading == ColorGradingPreset::tone_map_default ||
        pipeline.grading == ColorGradingPreset::glow_default ||
        pipeline.grading == ColorGradingPreset::balance_void_default;
    const bool isPhaseLikeGrading = pipeline.grading == ColorGradingPreset::phase_default ||
        pipeline.grading == ColorGradingPreset::neutral_default ||
        pipeline.grading == ColorGradingPreset::balance_void_default;
    if (pipeline.signal == ColorSignal::escape_magnitude &&
        pipeline.palette == ColorPalette::cyclic_escape &&
        isEscapeLikeGrading) {
        if (outMode) *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (pipeline.signal == ColorSignal::orbit_stripe &&
        pipeline.palette == ColorPalette::phase_wheel &&
        isPhaseLikeGrading) {
        if (outMode) *outMode = ColoringMode::phase;
        return true;
    }
    if (pipeline.signal == ColorSignal::root_proximity &&
        pipeline.palette == ColorPalette::cyclic_escape &&
        isEscapeLikeGrading) {
        if (outMode) *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (pipeline.signal == ColorSignal::smooth_escape &&
        pipeline.palette == ColorPalette::explaino_cmap &&
        isEscapeLikeGrading) {
        if (outMode) *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (pipeline.signal == ColorSignal::escape_magnitude &&
        pipeline.palette == ColorPalette::explaino_cmap &&
        isEscapeLikeGrading) {
        if (outMode) *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (pipeline.signal == ColorSignal::root_proximity &&
        pipeline.palette == ColorPalette::explaino_cmap &&
        isEscapeLikeGrading) {
        if (outMode) *outMode = ColoringMode::smooth_escape;
        return true;
    }
    return false;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool IsColorPipelineAllowedForFractal(
    FractalType fractalType,
    const ColorPipelineSelection& pipeline) {
    ColoringMode mode = ColoringMode::root_basin;
    return IsColorSignalAllowedForFractal(fractalType, pipeline.signal) &&
        TryMirroredColoringModeForPipeline(pipeline, &mode) &&
        IsColoringModeAllowedForFractal(fractalType, mode);
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