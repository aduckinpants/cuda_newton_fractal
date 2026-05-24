#pragma once

#include "basin_coloring.h"
#include "explaino_seed_curve.h"
#include "fractal_family_rules.h"
#include "fractal_types.h"

#include <cmath>

#if defined(__CUDACC__)
#define ESCAPE_TIME_COLOR_HD __host__ __device__
#else
#define ESCAPE_TIME_COLOR_HD
#endif

template <typename Scalar>
ESCAPE_TIME_COLOR_HD inline Scalar EscapeTimeColorClamp(Scalar value, Scalar lo, Scalar hi) {
    return value < lo ? lo : (value > hi ? hi : value);
}

ESCAPE_TIME_COLOR_HD inline float EscapeTimeColorLerp(float a, float b, float t) {
    return a + (b - a) * t;
}

ESCAPE_TIME_COLOR_HD inline float EscapeTimeColorSmoothstep(float edge0, float edge1, float x) {
    if (edge1 <= edge0) {
        return x >= edge1 ? 1.0f : 0.0f;
    }
    const float t = EscapeTimeColorClamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color EscapeTimeColorMake(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) {
    return {r, g, b, a};
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color EscapeTimeColorMulRgb(Color color, float scale) {
    scale = EscapeTimeColorClamp(scale, 0.0f, 1.5f);
    const int r = static_cast<int>(roundf(static_cast<float>(color.x) * scale));
    const int g = static_cast<int>(roundf(static_cast<float>(color.y) * scale));
    const int b = static_cast<int>(roundf(static_cast<float>(color.z) * scale));
    return EscapeTimeColorMake<Color>(
        static_cast<unsigned char>(EscapeTimeColorClamp(r, 0, 255)),
        static_cast<unsigned char>(EscapeTimeColorClamp(g, 0, 255)),
        static_cast<unsigned char>(EscapeTimeColorClamp(b, 0, 255)),
        255);
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color EscapeTimeColorBlendRgb(Color base, Color target, float t) {
    t = EscapeTimeColorClamp(t, 0.0f, 1.0f);
    const int r = static_cast<int>(roundf(EscapeTimeColorLerp(static_cast<float>(base.x), static_cast<float>(target.x), t)));
    const int g = static_cast<int>(roundf(EscapeTimeColorLerp(static_cast<float>(base.y), static_cast<float>(target.y), t)));
    const int b = static_cast<int>(roundf(EscapeTimeColorLerp(static_cast<float>(base.z), static_cast<float>(target.z), t)));
    return EscapeTimeColorMake<Color>(
        static_cast<unsigned char>(EscapeTimeColorClamp(r, 0, 255)),
        static_cast<unsigned char>(EscapeTimeColorClamp(g, 0, 255)),
        static_cast<unsigned char>(EscapeTimeColorClamp(b, 0, 255)),
        target.w);
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color HsvToRgb(float h, float s, float v);

ESCAPE_TIME_COLOR_HD inline unsigned char EscapeTimeColorToneMap(unsigned char value, float exposure) {
    float x = (static_cast<float>(value) / 255.0f) * exposure;
    x = 1.0f - expf(-x);
    return static_cast<unsigned char>(EscapeTimeColorClamp(x, 0.0f, 1.0f) * 255.0f);
}

ESCAPE_TIME_COLOR_HD inline float ResolveFractalGradeExposure(const KernelParams& params) {
    float exposure = params.exposure < 0.0f ? 0.0f : params.exposure;
    if (params.color_pipeline.grading == ColorGradingPreset::escape_default) {
        exposure *= EscapeTimeColorClamp(params.color_contrast_lift_exposure, 0.1f, 3.0f);
    }
    return exposure;
}

ESCAPE_TIME_COLOR_HD inline float ResolveFractalGradeSaturation(const KernelParams& params) {
    float saturation = params.color_saturation;
    if (params.color_pipeline.grading == ColorGradingPreset::escape_default) {
        saturation *= EscapeTimeColorClamp(params.color_contrast_lift_saturation, 0.0f, 2.0f);
    }
    return saturation;
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color ApplyFractalColorTint(Color color, const KernelParams& params) {
    return EscapeTimeColorMake<Color>(
        static_cast<unsigned char>(EscapeTimeColorClamp(static_cast<float>(color.x) * params.color_tint_r, 0.0f, 255.0f)),
        static_cast<unsigned char>(EscapeTimeColorClamp(static_cast<float>(color.y) * params.color_tint_g, 0.0f, 255.0f)),
        static_cast<unsigned char>(EscapeTimeColorClamp(static_cast<float>(color.z) * params.color_tint_b, 0.0f, 255.0f)),
        color.w);
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color ApplyFractalColorSaturation(Color color, float saturation) {
    const float lum = 0.299f * static_cast<float>(color.x) + 0.587f * static_cast<float>(color.y) + 0.114f * static_cast<float>(color.z);
    return EscapeTimeColorMake<Color>(
        static_cast<unsigned char>(EscapeTimeColorClamp(lum + saturation * (static_cast<float>(color.x) - lum), 0.0f, 255.0f)),
        static_cast<unsigned char>(EscapeTimeColorClamp(lum + saturation * (static_cast<float>(color.y) - lum), 0.0f, 255.0f)),
        static_cast<unsigned char>(EscapeTimeColorClamp(lum + saturation * (static_cast<float>(color.z) - lum), 0.0f, 255.0f)),
        color.w);
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color ApplyFractalColorContrast(Color color, float contrast) {
    return EscapeTimeColorMake<Color>(
        static_cast<unsigned char>(EscapeTimeColorClamp(128.0f + contrast * (static_cast<float>(color.x) - 128.0f), 0.0f, 255.0f)),
        static_cast<unsigned char>(EscapeTimeColorClamp(128.0f + contrast * (static_cast<float>(color.y) - 128.0f), 0.0f, 255.0f)),
        static_cast<unsigned char>(EscapeTimeColorClamp(128.0f + contrast * (static_cast<float>(color.z) - 128.0f), 0.0f, 255.0f)),
        color.w);
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color ApplyFractalColorGradingPass(
    Color color,
    const KernelParams& params,
    float exposure,
    float saturation,
    float contrast) {
    color = EscapeTimeColorMake<Color>(
        EscapeTimeColorToneMap(color.x, exposure),
        EscapeTimeColorToneMap(color.y, exposure),
        EscapeTimeColorToneMap(color.z, exposure),
        255);
    return ApplyFractalColorContrast(
        ApplyFractalColorSaturation(
            ApplyFractalColorTint(color, params),
            saturation),
        contrast);
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color ApplyFractalColorToneMapFinishPass(
    Color color,
    const KernelParams& params,
    float exposure,
    float saturation,
    float contrast) {
    color = ApplyFractalColorContrast(
        ApplyFractalColorSaturation(
            ApplyFractalColorTint(color, params),
            saturation),
        contrast);
    return EscapeTimeColorMake<Color>(
        EscapeTimeColorToneMap(color.x, exposure),
        EscapeTimeColorToneMap(color.y, exposure),
        EscapeTimeColorToneMap(color.z, exposure),
        255);
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color ApplyFractalColorGlowHighlight(Color color, float glow) {
    const float clampedGlow = EscapeTimeColorClamp(glow, 0.0f, 2.0f);
    const auto applyChannel = [clampedGlow](unsigned char value) {
        const float normalized = static_cast<float>(value) / 255.0f;
        const float highlight = normalized * normalized;
        const float boosted = static_cast<float>(value) +
            (255.0f - static_cast<float>(value)) * highlight * (clampedGlow * 0.5f);
        return static_cast<unsigned char>(EscapeTimeColorClamp(boosted, 0.0f, 255.0f));
    };
    return EscapeTimeColorMake<Color>(
        applyChannel(color.x),
        applyChannel(color.y),
        applyChannel(color.z),
        color.w);
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color ApplyFractalColorGlowFinishPass(
    Color color,
    const KernelParams& params,
    float exposure,
    float saturation,
    float contrast,
    float glow) {
    return ApplyFractalColorGlowHighlight(
        ApplyFractalColorGradingPass(color, params, exposure, saturation, contrast),
        glow);
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color ApplyFractalColorBalanceVoidGradePass(
    Color color,
    float balanceVoid,
    float chromaTension,
    float accentBias) {
    const float clampedBalanceVoid = EscapeTimeColorClamp(balanceVoid, -1.0f, 1.0f);
    const float clampedChromaTension = EscapeTimeColorClamp(chromaTension, -1.0f, 1.0f);
    const float clampedAccentBias = EscapeTimeColorClamp(accentBias, -1.0f, 1.0f);

    const float red = static_cast<float>(color.x) / 255.0f;
    const float green = static_cast<float>(color.y) / 255.0f;
    const float blue = static_cast<float>(color.z) / 255.0f;
    const float luminance = red * 0.299f + green * 0.587f + blue * 0.114f;
    const float accent = (luminance - 0.5f) * 2.0f;
    const float chromaScale = 1.0f + clampedChromaTension * (0.35f + 0.35f * fabsf(accent));
    const float warmShift = clampedBalanceVoid * (0.12f + 0.08f * (1.0f - fabsf(accent)));
    const float accentLift = clampedAccentBias * accent * 0.18f;

    const float balancedRed = EscapeTimeColorClamp(
        luminance + (red - luminance) * chromaScale + warmShift + accentLift,
        0.0f,
        1.0f);
    const float balancedGreen = EscapeTimeColorClamp(
        luminance + (green - luminance) * (1.0f - clampedChromaTension * 0.18f) - accentLift * 0.35f,
        0.0f,
        1.0f);
    const float balancedBlue = EscapeTimeColorClamp(
        luminance + (blue - luminance) * chromaScale - warmShift - accentLift,
        0.0f,
        1.0f);

    return EscapeTimeColorMake<Color>(
        static_cast<unsigned char>(EscapeTimeColorClamp(roundf(balancedRed * 255.0f), 0.0f, 255.0f)),
        static_cast<unsigned char>(EscapeTimeColorClamp(roundf(balancedGreen * 255.0f), 0.0f, 255.0f)),
        static_cast<unsigned char>(EscapeTimeColorClamp(roundf(balancedBlue * 255.0f), 0.0f, 255.0f)),
        color.w);
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color ApplyFractalColorGradingStackRow(
    Color color,
    const KernelParams& params,
    const ColorPipelineGradingStackEntry& gradingEntry) {
    float exposure = params.exposure < 0.0f ? 0.0f : params.exposure;
    float saturation = params.color_saturation;
    float contrast = params.color_contrast;
    float glow = params.color_glow;
    bool toneMapFinish = false;
    bool glowFinish = false;
    if (gradingEntry.grading == ColorGradingPreset::escape_default) {
        exposure *= EscapeTimeColorClamp(gradingEntry.params.exposure, 0.1f, 3.0f);
        saturation *= EscapeTimeColorClamp(gradingEntry.params.saturation, 0.0f, 2.0f);
        contrast *= EscapeTimeColorClamp(gradingEntry.params.contrast, 0.0f, 3.0f);
    } else if (gradingEntry.grading == ColorGradingPreset::phase_default ||
               gradingEntry.grading == ColorGradingPreset::bands_default) {
        exposure *= EscapeTimeColorClamp(gradingEntry.params.exposure, 0.1f, 3.0f);
        saturation = EscapeTimeColorClamp(gradingEntry.params.saturation, 0.0f, 2.0f);
        contrast = EscapeTimeColorClamp(gradingEntry.params.contrast, 0.0f, 3.0f);
    } else if (gradingEntry.grading == ColorGradingPreset::neutral_default ||
               gradingEntry.grading == ColorGradingPreset::tone_map_default) {
        exposure = EscapeTimeColorClamp(gradingEntry.params.exposure, 0.1f, 3.0f);
        saturation = EscapeTimeColorClamp(gradingEntry.params.saturation, 0.0f, 2.0f);
        contrast = EscapeTimeColorClamp(gradingEntry.params.contrast, 0.0f, 3.0f);
        toneMapFinish = gradingEntry.grading == ColorGradingPreset::tone_map_default;
    } else if (gradingEntry.grading == ColorGradingPreset::glow_default) {
        exposure = EscapeTimeColorClamp(gradingEntry.params.exposure, 0.1f, 3.0f);
        saturation = EscapeTimeColorClamp(gradingEntry.params.saturation, 0.0f, 2.0f);
        contrast = EscapeTimeColorClamp(gradingEntry.params.contrast, 0.0f, 3.0f);
        glow = EscapeTimeColorClamp(gradingEntry.params.glow, 0.0f, 2.0f);
        glowFinish = true;
    } else if (gradingEntry.grading == ColorGradingPreset::balance_void_default) {
        return ApplyFractalColorBalanceVoidGradePass(
            color,
            gradingEntry.params.balance_void,
            gradingEntry.params.chroma_tension,
            gradingEntry.params.accent_bias);
    }
    if (toneMapFinish) {
        return ApplyFractalColorToneMapFinishPass(color, params, exposure, saturation, contrast);
    }
    if (glowFinish) {
        return ApplyFractalColorGlowFinishPass(color, params, exposure, saturation, contrast, glow);
    }
    return ApplyFractalColorGradingPass(color, params, exposure, saturation, contrast);
}

ESCAPE_TIME_COLOR_HD inline int ClampedColorGradingStackCount(const KernelParams& params) {
    return params.color_grading_stack_count > kColorPipelineMaxGradingStackCount ?
        kColorPipelineMaxGradingStackCount : params.color_grading_stack_count;
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color ApplyFractalColorGradingStack(Color color, const KernelParams& params, int gradingStackCount) {
    for (int index = 0; index < gradingStackCount; ++index) {
        color = ApplyFractalColorGradingStackRow(color, params, params.color_grading_stack[index]);
    }
    return color;
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color ApplyFractalColorPipelinePresetGrading(
    Color color, const KernelParams& params, float exposure, float saturation) {
    if (params.color_pipeline.grading == ColorGradingPreset::tone_map_default) {
        return ApplyFractalColorToneMapFinishPass(color, params, exposure, saturation, params.color_contrast);
    }
    if (params.color_pipeline.grading == ColorGradingPreset::glow_default) {
        return ApplyFractalColorGlowFinishPass(color, params, params.exposure < 0.0f ? 0.0f : params.exposure,
            params.color_saturation, params.color_contrast, params.color_glow);
    }
    if (params.color_pipeline.grading == ColorGradingPreset::balance_void_default) {
        return ApplyFractalColorBalanceVoidGradePass(
            color, params.color_balance_void, params.color_chroma_tension, params.color_accent_bias);
    }
    return ApplyFractalColorGradingPass(color, params, exposure, saturation, params.color_contrast);
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color ApplyFractalColorGrading(Color color, const KernelParams& params) {
    const int gradingStackCount = ClampedColorGradingStackCount(params);
    if (gradingStackCount > 0) {
        return ApplyFractalColorGradingStack(color, params, gradingStackCount);
    }
    return ApplyFractalColorPipelinePresetGrading(
        color, params, ResolveFractalGradeExposure(params), ResolveFractalGradeSaturation(params));
}

    ESCAPE_TIME_COLOR_HD inline float ApplyColorPipelineShapeValue(float value, const KernelParams& params, float repeatWrap = 1.0f);

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color MakePhaseAngleColor(float angle, bool brightValue, const KernelParams& params) {
    const float twoPi = 6.28318530717958647692f;
    const float wrapCycles = params.color_phase_wrap_cycles < 0.5f ? 0.5f : params.color_phase_wrap_cycles;
    float hue = ((angle + params.color_phase_signal_offset) / twoPi) * wrapCycles;
    hue = ApplyColorPipelineShapeValue(hue, params);
    hue += (params.color_phase_palette_offset / twoPi);
    const float value = brightValue ? 0.85f : 0.25f;
    return HsvToRgb<Color>(hue, 0.9f, value);
}

ESCAPE_TIME_COLOR_HD inline float ResolvePhaseOrbitSignal(float angle, const KernelParams& params) {
    const float twoPi = 6.28318530717958647692f;
    const float wrapCycles = params.color_phase_wrap_cycles < 0.5f ? 0.5f : params.color_phase_wrap_cycles;
    return ((angle + params.color_phase_signal_offset) / twoPi) * wrapCycles;
}

ESCAPE_TIME_COLOR_HD inline float ResolvePhaseOrbitSignal(float angle, const ColorPipelineSourceRuntimeParams& sourceParams) {
    const float twoPi = 6.28318530717958647692f;
    const float wrapCycles = sourceParams.wrap_cycles < 0.5f ? 0.5f : sourceParams.wrap_cycles;
    return ((angle + sourceParams.phase_offset) / twoPi) * wrapCycles;
}

ESCAPE_TIME_COLOR_HD inline float ResolveIterationRatioSignal(int iteration, int maxIter) {
    const int safeMaxIter = maxIter > 0 ? maxIter : 1;
    return static_cast<float>(iteration) / static_cast<float>(safeMaxIter);
}

ESCAPE_TIME_COLOR_HD inline float ResolveEscapeMagnitudeSignal(float magnitude, const KernelParams& params) {
    return logf(1.0f + fmaxf(magnitude, 0.0f)) * EscapeTimeColorClamp(params.color_escape_magnitude_scale, 0.25f, 4.0f) +
        EscapeTimeColorClamp(params.color_escape_magnitude_bias, -1.0f, 1.0f);
}

ESCAPE_TIME_COLOR_HD inline float ResolveEscapeMagnitudeSignal(float magnitude, const ColorPipelineSourceRuntimeParams& sourceParams) {
    return logf(1.0f + fmaxf(magnitude, 0.0f)) * EscapeTimeColorClamp(sourceParams.magnitude_scale, 0.25f, 4.0f) +
        EscapeTimeColorClamp(sourceParams.magnitude_bias, -1.0f, 1.0f);
}

ESCAPE_TIME_COLOR_HD inline float ComputeEscapeTimeNu(
    FractalType fractalType,
    int iteration,
    float magnitude,
    const KernelParams& params);

struct SmoothEscapeFamilyTuning {
    float scale;
    float bias;
    bool finite_magnitude_fallback;
    float angle_mix;
};

ESCAPE_TIME_COLOR_HD inline SmoothEscapeFamilyTuning ResolveSmoothEscapeFamilyTuning(FractalType fractalType) {
    switch (fractalType) {
    case FractalType::collatz:
    case FractalType::explaino_collatz_direct:
        return {4.0f, 0.18f, true, 0.0f};
    case FractalType::nova:
    case FractalType::explaino_nova:
        return {8.0f, 0.10f, false, 0.0f};
    case FractalType::mcmullen:
        return {5.0f, 0.08f, false, 0.0f};
    case FractalType::explaino_rational_escape:
        return {5.0f, 0.08f, false, 0.30f};
    case FractalType::magnet:
        return {5.0f, 0.08f, false, 0.45f};
    default:
        return {1.0f, 0.0f, false, 0.0f};
    }
}

ESCAPE_TIME_COLOR_HD inline float ResolveSmoothEscapeBaseSignal(
    FractalType fractalType,
    int iteration,
    float magnitude,
    float angle,
    const KernelParams& params) {
    const SmoothEscapeFamilyTuning tuning = ResolveSmoothEscapeFamilyTuning(fractalType);
    const float safeMagnitude = tuning.finite_magnitude_fallback && !isfinite(magnitude) ? 100.0f : magnitude;
    float angleSignal = 0.0f;
    if (tuning.angle_mix > 0.0f && isfinite(angle)) {
        const float twoPi = 6.28318530717958647692f;
        angleSignal = (angle + 3.14159265358979323846f) / twoPi;
    }
    return ComputeEscapeTimeNu(fractalType, iteration, safeMagnitude, params) * 0.025f * tuning.scale +
        angleSignal * tuning.angle_mix +
        tuning.bias;
}

ESCAPE_TIME_COLOR_HD inline float ResolveSmoothEscapeBaseSignal(
    FractalType fractalType,
    int iteration,
    float magnitude,
    const KernelParams& params) {
    return ResolveSmoothEscapeBaseSignal(fractalType, iteration, magnitude, 0.0f, params);
}

ESCAPE_TIME_COLOR_HD inline float ResolveSmoothEscapeSignal(
    FractalType fractalType,
    int iteration,
    float magnitude,
    float angle,
    const KernelParams& params) {
    return ResolveSmoothEscapeBaseSignal(fractalType, iteration, magnitude, angle, params) *
        EscapeTimeColorClamp(params.color_smooth_escape_scale, 0.25f, 4.0f) +
        EscapeTimeColorClamp(params.color_smooth_escape_bias, -1.0f, 1.0f);
}

ESCAPE_TIME_COLOR_HD inline float ResolveSmoothEscapeSignal(
    FractalType fractalType,
    int iteration,
    float magnitude,
    const KernelParams& params) {
    return ResolveSmoothEscapeSignal(fractalType, iteration, magnitude, 0.0f, params);
}

ESCAPE_TIME_COLOR_HD inline float ResolveSmoothEscapeSignal(
    FractalType fractalType,
    int iteration,
    float magnitude,
    float angle,
    const ColorPipelineSourceRuntimeParams& sourceParams,
    const KernelParams& params) {
    return ResolveSmoothEscapeBaseSignal(fractalType, iteration, magnitude, angle, params) *
        EscapeTimeColorClamp(sourceParams.scale, 0.25f, 4.0f) +
        EscapeTimeColorClamp(sourceParams.bias, -1.0f, 1.0f);
}

ESCAPE_TIME_COLOR_HD inline float ResolveSmoothEscapeSignal(
    FractalType fractalType,
    int iteration,
    float magnitude,
    const ColorPipelineSourceRuntimeParams& sourceParams,
    const KernelParams& params) {
    return ResolveSmoothEscapeSignal(fractalType, iteration, magnitude, 0.0f, sourceParams, params);
}

ESCAPE_TIME_COLOR_HD inline float ResolveOrbitStripeSignal(float angle, const KernelParams& params) {
    return 0.5f + 0.5f * sinf(
        angle * EscapeTimeColorClamp(params.color_orbit_stripe_frequency, 0.25f, 12.0f) +
        EscapeTimeColorClamp(params.color_orbit_stripe_phase, -3.14159265358979323846f, 3.14159265358979323846f));
}

ESCAPE_TIME_COLOR_HD inline float ResolveOrbitStripeSignal(float angle, const ColorPipelineSourceRuntimeParams& sourceParams) {
    return 0.5f + 0.5f * sinf(
        angle * EscapeTimeColorClamp(sourceParams.stripe_frequency, 0.25f, 12.0f) +
        EscapeTimeColorClamp(sourceParams.stripe_phase, -3.14159265358979323846f, 3.14159265358979323846f));
}

ESCAPE_TIME_COLOR_HD inline float ResolveBandedIterationSignal(
    int iteration,
    int maxIter,
    const ColorPipelineSourceRuntimeParams& sourceParams) {
    const float baseSignal = ResolveIterationRatioSignal(iteration, maxIter);
    const int bandCount = sourceParams.band_count < 2 ? 2 : sourceParams.band_count;
    const float rawBand = baseSignal * static_cast<float>(bandCount);
    const float baseBand = floorf(rawBand);
    const float frac = rawBand - baseBand;
    const float softness = EscapeTimeColorClamp(sourceParams.softness, 0.0f, 1.0f);
    const float blend = softness <= 0.0f
        ? (frac >= 0.5f ? 1.0f : 0.0f)
        : EscapeTimeColorSmoothstep(0.5f - 0.5f * softness, 0.5f + 0.5f * softness, frac);
    return (baseBand + blend) / static_cast<float>(bandCount);
}

ESCAPE_TIME_COLOR_HD inline float ResolveEscapeFamilySignal(
    ColorSignal signal,
    FractalType fractalType,
    int iteration,
    int maxIter,
    float magnitude,
    float angle,
    const KernelParams& params) {
    if (signal == ColorSignal::smooth_escape) {
        return ResolveSmoothEscapeSignal(fractalType, iteration, magnitude, angle, params);
    }
    if (signal == ColorSignal::escape_magnitude) {
        return ResolveEscapeMagnitudeSignal(magnitude, params);
    }
    return ResolveIterationRatioSignal(iteration, maxIter);
}

ESCAPE_TIME_COLOR_HD inline float ResolveEscapeFamilySignal(
    ColorSignal signal,
    FractalType fractalType,
    int iteration,
    int maxIter,
    float magnitude,
    float angle,
    const ColorPipelineSourceRuntimeParams& sourceParams,
    const KernelParams& params) {
    if (signal == ColorSignal::smooth_escape) {
        return ResolveSmoothEscapeSignal(fractalType, iteration, magnitude, angle, sourceParams, params);
    }
    if (signal == ColorSignal::escape_magnitude) {
        return ResolveEscapeMagnitudeSignal(magnitude, sourceParams);
    }
    if (signal == ColorSignal::iteration_bands) {
        return ResolveBandedIterationSignal(iteration, maxIter, sourceParams);
    }
    return ResolveIterationRatioSignal(iteration, maxIter);
}

ESCAPE_TIME_COLOR_HD inline float ResolveAngularSignal(ColorSignal signal, float angle, const KernelParams& params) {
    return signal == ColorSignal::phase_angle
        ? ResolvePhaseOrbitSignal(angle, params)
        : ResolveOrbitStripeSignal(angle, params);
}

ESCAPE_TIME_COLOR_HD inline float ResolveAngularSignal(
    ColorSignal signal,
    float angle,
    const ColorPipelineSourceRuntimeParams& sourceParams) {
    return signal == ColorSignal::phase_angle
        ? ResolvePhaseOrbitSignal(angle, sourceParams)
        : ResolveOrbitStripeSignal(angle, sourceParams);
}

template <typename Complex>
ESCAPE_TIME_COLOR_HD inline float EscapeTimeColorAbs(Complex value) {
    return sqrtf(value.x * value.x + value.y * value.y);
}

// HSV to RGB (h in [0,1], s in [0,1], v in [0,1]).
template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color HsvToRgb(float h, float s, float v) {
    h = h - floorf(h);  // wrap to [0,1]
    float c = v * s;
    float x = c * (1.0f - fabsf(fmodf(h * 6.0f, 2.0f) - 1.0f));
    float m = v - c;
    float r, g, b;
    int sector = static_cast<int>(h * 6.0f);
    switch (sector % 6) {
    case 0: r = c; g = x; b = 0; break;
    case 1: r = x; g = c; b = 0; break;
    case 2: r = 0; g = c; b = x; break;
    case 3: r = 0; g = x; b = c; break;
    case 4: r = x; g = 0; b = c; break;
    default: r = c; g = 0; b = x; break;
    }
    return EscapeTimeColorMake<Color>(
        static_cast<unsigned char>(EscapeTimeColorClamp(r + m, 0.0f, 1.0f) * 255.0f),
        static_cast<unsigned char>(EscapeTimeColorClamp(g + m, 0.0f, 1.0f) * 255.0f),
        static_cast<unsigned char>(EscapeTimeColorClamp(b + m, 0.0f, 1.0f) * 255.0f),
        255);
}

struct EscapeTimeColorRgb {
    float r;
    float g;
    float b;
};

ESCAPE_TIME_COLOR_HD inline EscapeTimeColorRgb EscapeTimeColorApplySaturation(EscapeTimeColorRgb rgb, float saturation) {
    const float lum = 0.299f * rgb.r + 0.587f * rgb.g + 0.114f * rgb.b;
    rgb.r = EscapeTimeColorClamp(lum + saturation * (rgb.r - lum), 0.0f, 1.0f);
    rgb.g = EscapeTimeColorClamp(lum + saturation * (rgb.g - lum), 0.0f, 1.0f);
    rgb.b = EscapeTimeColorClamp(lum + saturation * (rgb.b - lum), 0.0f, 1.0f);
    return rgb;
}

ESCAPE_TIME_COLOR_HD inline EscapeTimeColorRgb SampleIterationBandPalette(float rawBand, int bandCount, float softness) {
    const float palette[8][3] = {
        {0.05f, 0.15f, 0.45f}, {0.10f, 0.50f, 0.70f}, {0.15f, 0.75f, 0.55f}, {0.65f, 0.85f, 0.20f},
        {0.95f, 0.75f, 0.15f}, {0.90f, 0.40f, 0.10f}, {0.70f, 0.15f, 0.30f}, {0.40f, 0.10f, 0.55f},
    };
    int bandIndex = static_cast<int>(floorf(rawBand)) % bandCount;
    if (bandIndex < 0) bandIndex += bandCount;
    const int paletteIndex = bandIndex % 8;
    const int nextPaletteIndex = (paletteIndex + 1) % 8;
    const float frac = rawBand - floorf(rawBand);
    const float blend = softness <= 0.0f
        ? 0.0f
        : EscapeTimeColorSmoothstep(0.5f - 0.5f * softness, 0.5f + 0.5f * softness, frac);
    return {
        EscapeTimeColorLerp(palette[paletteIndex][0], palette[nextPaletteIndex][0], blend),
        EscapeTimeColorLerp(palette[paletteIndex][1], palette[nextPaletteIndex][1], blend),
        EscapeTimeColorLerp(palette[paletteIndex][2], palette[nextPaletteIndex][2], blend),
    };
}

ESCAPE_TIME_COLOR_HD inline EscapeTimeColorRgb ApplyIterationBandEmphasis(EscapeTimeColorRgb rgb, float emphasis) {
    const float lum = 0.299f * rgb.r + 0.587f * rgb.g + 0.114f * rgb.b;
    rgb.r = EscapeTimeColorClamp(lum + emphasis * (rgb.r - lum), 0.0f, 1.0f);
    rgb.g = EscapeTimeColorClamp(lum + emphasis * (rgb.g - lum), 0.0f, 1.0f);
    rgb.b = EscapeTimeColorClamp(lum + emphasis * (rgb.b - lum), 0.0f, 1.0f);
    return rgb;
}

ESCAPE_TIME_COLOR_HD inline ColorPipelineShapeRuntimeParams LegacyColorPipelineShapeRuntimeParams(const KernelParams& params) {
    ColorPipelineShapeRuntimeParams shapeParams;
    shapeParams.offset = params.color_shape_offset;
    shapeParams.scale = params.color_shape_scale;
    shapeParams.repeat_frequency = params.color_shape_repeat_frequency;
    shapeParams.repeat_phase = params.color_shape_repeat_phase;
    shapeParams.posterize_steps = params.color_shape_posterize_steps;
    shapeParams.posterize_mix = params.color_shape_posterize_mix;
    shapeParams.bias = params.color_shape_bias;
    shapeParams.gain = params.color_shape_gain;
    shapeParams.window_center = params.color_shape_window_center;
    shapeParams.window_width = params.color_shape_window_width;
    shapeParams.window_softness = params.color_shape_window_softness;
    return shapeParams;
}

ESCAPE_TIME_COLOR_HD inline float ApplyPosterizeShapeValue(
    float value,
    const ColorPipelineShapeRuntimeParams& params,
    float repeatWrap) {
    const int steps = params.posterize_steps < 2 ? 2 : (params.posterize_steps > 24 ? 24 : params.posterize_steps);
    const float mix = EscapeTimeColorClamp(params.posterize_mix, 0.0f, 1.0f);
    const float safeWrap = repeatWrap > 0.0f ? repeatWrap : 1.0f;
    const float quantized = floorf((value / safeWrap) * static_cast<float>(steps)) / static_cast<float>(steps) * safeWrap;
    return value + (quantized - value) * mix;
}

ESCAPE_TIME_COLOR_HD inline float ApplyRepeatShapeValue(
    float value,
    const ColorPipelineShapeRuntimeParams& params,
    float repeatWrap) {
    value = value * EscapeTimeColorClamp(params.repeat_frequency, 0.25f, 24.0f) +
        EscapeTimeColorClamp(params.repeat_phase, -1.0f, 1.0f);
    repeatWrap = repeatWrap > 0.0f ? repeatWrap : 1.0f;
    value = fmodf(value, repeatWrap);
    if (value < 0.0f) {
        value += repeatWrap;
    }
    return value;
}

ESCAPE_TIME_COLOR_HD inline float ApplyRepeatShapeValue(float value, const KernelParams& params, float repeatWrap) {
    return ApplyRepeatShapeValue(value, LegacyColorPipelineShapeRuntimeParams(params), repeatWrap);
}

ESCAPE_TIME_COLOR_HD inline float ApplyMirrorRepeatShapeValue(
    float value,
    const ColorPipelineShapeRuntimeParams& params,
    float repeatWrap) {
    const float safeWrap = repeatWrap > 0.0f ? repeatWrap : 1.0f;
    const float repeated = ApplyRepeatShapeValue(value, params, safeWrap);
    const float normalized = repeated / safeWrap;
    const float mirrored = normalized <= 0.5f ? normalized * 2.0f : (1.0f - normalized) * 2.0f;
    return mirrored * safeWrap;
}

ESCAPE_TIME_COLOR_HD inline float ApplyBiasCurveUnitValue(float value, float bias) {
    const float safeBias = EscapeTimeColorClamp(bias, 0.0f, 1.0f);
    if (safeBias <= 0.0f) {
        return 0.0f;
    }
    if (safeBias >= 1.0f) {
        return 1.0f;
    }
    const float denominator = ((1.0f / safeBias) - 2.0f) * (1.0f - value) + 1.0f;
    return denominator != 0.0f ? value / denominator : value;
}

ESCAPE_TIME_COLOR_HD inline float ApplyGainCurveUnitValue(float value, float gain) {
    const float safeGain = EscapeTimeColorClamp(gain, 0.0f, 1.0f);
    if (value < 0.5f) {
        return 0.5f * ApplyBiasCurveUnitValue(value * 2.0f, 1.0f - safeGain);
    }
    return 1.0f - 0.5f * ApplyBiasCurveUnitValue(2.0f - value * 2.0f, 1.0f - safeGain);
}

ESCAPE_TIME_COLOR_HD inline float ApplyBiasGainShapeValue(
    float value,
    const ColorPipelineShapeRuntimeParams& params,
    float repeatWrap) {
    const float safeWrap = repeatWrap > 0.0f ? repeatWrap : 1.0f;
    const float segment = floorf(value / safeWrap);
    float local = value - segment * safeWrap;
    if (local < 0.0f) {
        local += safeWrap;
    }
    float normalized = EscapeTimeColorClamp(local / safeWrap, 0.0f, 1.0f);
    normalized = ApplyBiasCurveUnitValue(normalized, params.bias);
    normalized = ApplyGainCurveUnitValue(normalized, params.gain);
    return segment * safeWrap + normalized * safeWrap;
}

ESCAPE_TIME_COLOR_HD inline float EscapeTimeColorWrapPositive(float value, float wrap) {
    wrap = wrap > 0.0f ? wrap : 1.0f;
    value = fmodf(value, wrap);
    if (value < 0.0f) {
        value += wrap;
    }
    return value;
}

ESCAPE_TIME_COLOR_HD inline float EscapeTimeColorWrappedDistance(float local, float center, float wrap) {
    float delta = local - center;
    if (delta > 0.5f * wrap) {
        delta -= wrap;
    } else if (delta < -0.5f * wrap) {
        delta += wrap;
    }
    return fabsf(delta);
}

ESCAPE_TIME_COLOR_HD inline float ApplySmoothWindowMask(float distance, float halfWidth, float softness) {
    if (softness <= 0.0f) {
        return distance <= halfWidth ? 1.0f : 0.0f;
    }
    const float inner = halfWidth > softness ? halfWidth - softness : 0.0f;
    const float outer = halfWidth + softness;
    if (outer <= 0.0f) {
        return 0.0f;
    }
    const float mask = 1.0f - EscapeTimeColorSmoothstep(inner, outer, distance);
    return EscapeTimeColorClamp(mask, 0.0f, 1.0f);
}

ESCAPE_TIME_COLOR_HD inline float ApplySmoothWindowShapeValue(
    float value,
    const ColorPipelineShapeRuntimeParams& params,
    float repeatWrap) {
    const float safeWrap = repeatWrap > 0.0f ? repeatWrap : 1.0f;
    const float local = EscapeTimeColorWrapPositive(value, safeWrap);
    const float center = EscapeTimeColorClamp(params.window_center, 0.0f, 1.0f) * safeWrap;
    const float width = EscapeTimeColorClamp(params.window_width, 0.0f, 1.0f) * safeWrap;
    const float softness = EscapeTimeColorClamp(params.window_softness, 0.0f, 1.0f) * safeWrap;
    if (width >= safeWrap) {
        return safeWrap;
    }
    const float distance = EscapeTimeColorWrappedDistance(local, center, safeWrap);
    const float halfWidth = 0.5f * width;
    return ApplySmoothWindowMask(distance, halfWidth, softness) * safeWrap;
}

ESCAPE_TIME_COLOR_HD inline float ApplyColorPipelineShapeRowValue(
    float value,
    ColorPipelineShape shape,
    const ColorPipelineShapeRuntimeParams& params,
    float repeatWrap) {
    if (shape == ColorPipelineShape::offset_scale) {
        value += EscapeTimeColorClamp(params.offset, -2.0f, 2.0f);
        value *= EscapeTimeColorClamp(params.scale, 0.1f, 8.0f);
    } else if (shape == ColorPipelineShape::repeat) {
        value = ApplyRepeatShapeValue(value, params, repeatWrap);
    } else if (shape == ColorPipelineShape::posterize) {
        value = ApplyPosterizeShapeValue(value, params, repeatWrap);
    } else if (shape == ColorPipelineShape::mirror_repeat) {
        value = ApplyMirrorRepeatShapeValue(value, params, repeatWrap);
    } else if (shape == ColorPipelineShape::bias_gain_curve) {
        value = ApplyBiasGainShapeValue(value, params, repeatWrap);
    } else if (shape == ColorPipelineShape::smooth_window) {
        value = ApplySmoothWindowShapeValue(value, params, repeatWrap);
    }
    return value;
}

ESCAPE_TIME_COLOR_HD inline float ApplyColorPipelineShapeValue(float value, const KernelParams& params, float repeatWrap) {
    int shapeStackCount = params.color_shape_stack_count;
    if (shapeStackCount > kColorPipelineMaxShapeStackCount) {
        shapeStackCount = kColorPipelineMaxShapeStackCount;
    }
    if (shapeStackCount > 0) {
        for (int index = 0; index < shapeStackCount; ++index) {
            value = ApplyColorPipelineShapeRowValue(
                value,
                params.color_shape_stack[index].shape,
                params.color_shape_stack[index].params,
                repeatWrap);
        }
        return value;
    }
    return ApplyColorPipelineShapeRowValue(
        value,
        params.color_shape,
        LegacyColorPipelineShapeRuntimeParams(params),
        repeatWrap);
}

ESCAPE_TIME_COLOR_HD inline float ResolveSmoothEscapeHeatmapBand(float nu, const KernelParams& params) {
    float band = nu * 0.025f;
    if (params.color_pipeline.signal == ColorSignal::smooth_escape) {
        band = band * EscapeTimeColorClamp(params.color_smooth_escape_scale, 0.25f, 4.0f) +
            EscapeTimeColorClamp(params.color_smooth_escape_bias, -1.0f, 1.0f);
    }
    band = ApplyColorPipelineShapeValue(band, params);
    if (params.color_pipeline.palette == ColorPalette::cyclic_escape) {
        band *= EscapeTimeColorClamp(params.color_heatmap_cycle_scale, 0.25f, 4.0f);
    }
    return band;
}

ESCAPE_TIME_COLOR_HD inline EscapeTimeColorRgb SampleEscapeHeatmap(float band, float saturation) {
    const float frac = band - floorf(band);
    const float stops[6][3] = {
        {0.00f, 0.03f, 0.20f}, {0.05f, 0.35f, 0.65f}, {0.10f, 0.75f, 0.85f},
        {0.95f, 0.85f, 0.25f}, {0.90f, 0.45f, 0.10f}, {0.00f, 0.03f, 0.20f},
    };
    const float u5 = frac * 5.0f;
    int segment = static_cast<int>(u5);
    if (segment > 4) segment = 4;
    const float segT = u5 - static_cast<float>(segment);
    EscapeTimeColorRgb rgb{
        stops[segment][0] + (stops[segment + 1][0] - stops[segment][0]) * segT,
        stops[segment][1] + (stops[segment + 1][1] - stops[segment][1]) * segT,
        stops[segment][2] + (stops[segment + 1][2] - stops[segment][2]) * segT,
    };
    rgb = EscapeTimeColorApplySaturation(rgb, EscapeTimeColorClamp(saturation, 0.0f, 2.0f));
    return rgb;
}

ESCAPE_TIME_COLOR_HD inline EscapeTimeColorRgb SampleEscapeHeatmap(float band, const KernelParams& params) {
    const float saturation = params.color_pipeline.palette == ColorPalette::cyclic_escape
        ? params.color_heatmap_saturation
        : 1.0f;
    return SampleEscapeHeatmap(band, saturation);
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color EscapeTimeColorFromRgb(EscapeTimeColorRgb rgb) {
    return EscapeTimeColorMake<Color>(
        static_cast<unsigned char>(EscapeTimeColorClamp(rgb.r, 0.0f, 1.0f) * 255.0f),
        static_cast<unsigned char>(EscapeTimeColorClamp(rgb.g, 0.0f, 1.0f) * 255.0f),
        static_cast<unsigned char>(EscapeTimeColorClamp(rgb.b, 0.0f, 1.0f) * 255.0f),
        255);
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color IterationBandColor(int iteration, int maxIter, const KernelParams& params);

template <typename Color>
ESCAPE_TIME_COLOR_HD inline bool TryMakeEscapeTimeBasinErrorColor(ColoringMode mode, Color* outColor) {
    if (!outColor) {
        return false;
    }
    if (mode != ColoringMode::root_basin && mode != ColoringMode::joy_basins) {
        return false;
    }
    *outColor = EscapeTimeColorMake<Color>(255, 0, 255, 255);
    return true;
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color MakeIterationCountEscapeTimeColor(int iteration, int maxIter) {
    const float t = static_cast<float>(iteration) / static_cast<float>(maxIter);
    const unsigned char value = static_cast<unsigned char>(EscapeTimeColorClamp(t, 0.0f, 1.0f) * 255.0f);
    return EscapeTimeColorMake<Color>(64, value, static_cast<unsigned char>(255 - value), 255);
}

template <typename Color, typename Complex>
ESCAPE_TIME_COLOR_HD inline bool TryMakeDiscreteEscapeTimeColor(FractalType, ColoringMode mode, bool escaped, int iteration, int maxIter, Complex z, const KernelParams& params, Color* outColor) {
    if (!outColor) return false;
    if (TryMakeEscapeTimeBasinErrorColor(mode, outColor)) return true;
    ColoringMode exactLegacyMode = ColoringMode::root_basin;
    if (!TryLegacyColoringModeForPipeline(params.color_pipeline, &exactLegacyMode) || exactLegacyMode != mode) {
        return false;
    }
    if (mode == ColoringMode::phase) {
        *outColor = MakePhaseAngleColor<Color>(atan2f(z.y, z.x), escaped, params);
        return true;
    }
    if (mode == ColoringMode::iteration_bands) {
        if (escaped) {
            *outColor = IterationBandColor<Color>(iteration, maxIter, params);
        } else {
            *outColor = EscapeTimeColorMake<Color>(0, 0, 0, 255);
        }
        return true;
    }
    if (mode == ColoringMode::iteration_count) {
        *outColor = escaped
            ? MakeIterationCountEscapeTimeColor<Color>(iteration, maxIter)
            : EscapeTimeColorMake<Color>(0, 0, 0, 255);
        return true;
    }
    return false;
}

ESCAPE_TIME_COLOR_HD inline float ComputeEscapeTimeNu(
    FractalType fractalType,
    int iteration,
    float magnitude,
    const KernelParams& params) {
    const float logZn = logf(fmaxf(magnitude, 1.0e-12f));
    float denom = logf(2.0f);
    if (fractalType == FractalType::multibrot && params.multibrot_power_float > 1.0f) {
        denom = logf(params.multibrot_power_float);
    }
    return static_cast<float>(iteration) + 1.0f - logf(fmaxf(logZn / denom, 1.0e-12f)) / denom;
}

template <typename Complex>
ESCAPE_TIME_COLOR_HD inline bool TryResolveRootProximityDistance(Complex z, const KernelParams& params, float* outDistance) {
    float nearestDistanceSquared = 0.0f;
    if (params.explaino_root_count > 0) {
        nearestDistanceSquared = static_cast<float>(NearestRootDistanceSquaredList(z, params.explaino_roots, params.explaino_root_count));
    } else {
        const int polynomialRootCount = ResolvePolynomialRootCount(params.poly_kind);
        if (polynomialRootCount <= 0) {
            return false;
        }
        nearestDistanceSquared = static_cast<float>(NearestRootDistanceSquaredUnitRoots(z, polynomialRootCount));
    }
    if (outDistance) {
        *outDistance = sqrtf(fmaxf(nearestDistanceSquared, 0.0f));
    }
    return true;
}

template <typename Complex>
ESCAPE_TIME_COLOR_HD inline float ResolveRootProximitySignal(Complex z, const KernelParams& params) {
    float distance = 0.0f;
    if (!TryResolveRootProximityDistance(z, params, &distance)) {
        return 0.0f;
    }
    const float scale = EscapeTimeColorClamp(params.color_root_proximity_scale, 0.25f, 8.0f);
    const float bias = EscapeTimeColorClamp(params.color_root_proximity_bias, -1.0f, 1.0f);
    const float safeDistance = fmaxf(distance, 1.0e-12f);
    return -log2f(fmaxf(scale * safeDistance, 1.0e-12f)) + bias;
}

template <typename Complex>
ESCAPE_TIME_COLOR_HD inline float ResolveRootProximitySignal(
    Complex z,
    const KernelParams& params,
    const ColorPipelineSourceRuntimeParams& sourceParams) {
    float distance = 0.0f;
    if (!TryResolveRootProximityDistance(z, params, &distance)) {
        return 0.0f;
    }
    const float scale = EscapeTimeColorClamp(sourceParams.proximity_scale, 0.25f, 8.0f);
    const float bias = EscapeTimeColorClamp(sourceParams.proximity_bias, -1.0f, 1.0f);
    const float safeDistance = fmaxf(distance, 1.0e-12f);
    return -log2f(fmaxf(scale * safeDistance, 1.0e-12f)) + bias;
}

template <typename Complex>
ESCAPE_TIME_COLOR_HD inline bool TryResolveColorPipelineRootSample(
    FractalType fractalType,
    Complex z,
    const KernelParams& params,
    int* outRootIndex,
    int* outRootCount) {
    if (fractalType == FractalType::projection_and_flow) {
        const int projectionRootCount =
            params.projection_and_flow_root_family == ProjectionAndFlowRootFamily::quartic_unit_roots ? 4 : 3;
        const int projectionClassCount = projectionRootCount * 4 + 1;
        if (outRootCount) *outRootCount = projectionClassCount;
        if (outRootIndex) *outRootIndex = NearestRootIndexUnitRoots(z, projectionClassCount);
        return true;
    }
    const int polynomialRootCount = ResolvePolynomialRootCount(params.poly_kind);
    const bool useCustomRoots = polynomialRootCount == 0 &&
        IsExplainoFamily(fractalType) &&
        params.explaino_root_count > 0;
    if (useCustomRoots) {
        if (outRootCount) *outRootCount = params.explaino_root_count;
        if (outRootIndex) *outRootIndex = NearestRootIndexList(z, params.explaino_roots, params.explaino_root_count);
        return true;
    }
    if (polynomialRootCount <= 0) {
        return false;
    }
    if (outRootCount) *outRootCount = polynomialRootCount;
    if (outRootIndex) *outRootIndex = NearestRootIndexUnitRoots(z, polynomialRootCount);
    return true;
}

ESCAPE_TIME_COLOR_HD inline float ResolveColorPipelineRootSignal(int rootIndex, int rootCount) {
    if (rootIndex < 0 || rootCount <= 0) {
        return 0.0f;
    }
    return (static_cast<float>(rootIndex) + 0.5f) / static_cast<float>(rootCount);
}

ESCAPE_TIME_COLOR_HD inline int ClampColorPipelineSourceStackCount(int count) {
    if (count < 0) {
        return 0;
    }
    return count > kColorPipelineMaxSourceStackCount ? kColorPipelineMaxSourceStackCount : count;
}

ESCAPE_TIME_COLOR_HD inline float ResolveBasinResidualMetric(float residual);

ESCAPE_TIME_COLOR_HD inline float ResolveBasinSmoothEscapeSignal(
    float residual,
    const ColorPipelineSourceRuntimeParams& sourceParams) {
    return ResolveBasinResidualMetric(residual) * 0.05f *
        EscapeTimeColorClamp(sourceParams.scale, 0.25f, 4.0f) +
        EscapeTimeColorClamp(sourceParams.bias, -1.0f, 1.0f);
}

template <typename Complex>
ESCAPE_TIME_COLOR_HD inline float ResolveColorPipelineSourceStackEntryEscapeSignal(
    FractalType fractalType,
    int iteration,
    int maxIter,
    Complex z,
    float magnitude,
    float angle,
    const KernelParams& params,
    const ColorPipelineSourceStackEntry& entry) {
    if (entry.signal == ColorSignal::root_proximity) {
        return ResolveRootProximitySignal(z, params, entry.params);
    }
    if (entry.signal == ColorSignal::phase_angle ||
        entry.signal == ColorSignal::orbit_stripe) {
        return ResolveAngularSignal(entry.signal, angle, entry.params);
    }
    if (entry.signal == ColorSignal::root_index) {
        int rootIndex = -1;
        int rootCount = 0;
        if (TryResolveColorPipelineRootSample(fractalType, z, params, &rootIndex, &rootCount)) {
            return ResolveColorPipelineRootSignal(rootIndex, rootCount);
        }
        return 0.0f;
    }
    return ResolveEscapeFamilySignal(
        entry.signal,
        fractalType,
        iteration,
        maxIter,
        magnitude,
        angle,
        entry.params,
        params);
}

ESCAPE_TIME_COLOR_HD inline int ResolveShapedColorPipelineRootIndex(int rootIndex, int rootCount, const KernelParams& params) {
    if (rootIndex < 0 || rootCount <= 0) {
        return -1;
    }
    const float shapedSignal = ApplyColorPipelineShapeValue(
        ResolveColorPipelineRootSignal(rootIndex, rootCount),
        params);
    float wrappedSignal = shapedSignal - floorf(shapedSignal);
    if (wrappedSignal < 0.0f) {
        wrappedSignal += 1.0f;
    }
    int shapedIndex = static_cast<int>(wrappedSignal * static_cast<float>(rootCount));
    if (shapedIndex >= rootCount) {
        shapedIndex = rootCount - 1;
    }
    return shapedIndex;
}

template <typename Complex>
ESCAPE_TIME_COLOR_HD inline float ResolveProgrammableEscapeTimeSignal(
    FractalType fractalType,
    int iteration,
    int maxIter,
    Complex z,
    const KernelParams& params) {
    const int sourceStackCount = ClampColorPipelineSourceStackCount(params.color_source_stack_count);
    if (sourceStackCount > 0) {
        const float magnitude = EscapeTimeColorAbs(z);
        const float angle = atan2f(z.y, z.x);
        float blendedSignal = ResolveColorPipelineSourceStackEntryEscapeSignal(
            fractalType,
            iteration,
            maxIter,
            z,
            magnitude,
            angle,
            params,
            params.color_source_stack[0]);
        for (int index = 1; index < sourceStackCount; ++index) {
            const ColorPipelineSourceStackEntry& entry = params.color_source_stack[index];
            const float nextSignal = ResolveColorPipelineSourceStackEntryEscapeSignal(
                fractalType,
                iteration,
                maxIter,
                z,
                magnitude,
                angle,
                params,
                entry);
            blendedSignal = EscapeTimeColorLerp(
                blendedSignal,
                nextSignal,
                EscapeTimeColorClamp(entry.params.blend_weight, 0.0f, 1.0f));
        }
        return blendedSignal;
    }
    if (params.color_pipeline.signal == ColorSignal::root_proximity) {
        return ResolveRootProximitySignal(z, params);
    }
    if (params.color_pipeline.signal == ColorSignal::phase_angle ||
        params.color_pipeline.signal == ColorSignal::orbit_stripe) {
        return ResolveAngularSignal(params.color_pipeline.signal, atan2f(z.y, z.x), params);
    }
    if (params.color_pipeline.signal == ColorSignal::root_index) {
        int rootIndex = -1;
        int rootCount = 0;
        if (TryResolveColorPipelineRootSample(fractalType, z, params, &rootIndex, &rootCount)) {
            return ResolveColorPipelineRootSignal(rootIndex, rootCount);
        }
        return 0.0f;
    }
    return ResolveEscapeFamilySignal(
        params.color_pipeline.signal,
        fractalType,
        iteration,
        maxIter,
        EscapeTimeColorAbs(z),
        atan2f(z.y, z.x),
        params);
}

ESCAPE_TIME_COLOR_HD inline float EscapeTimeColorExplainoSeed(float value) {
    double wrapped = static_cast<double>(value) - std::floor(static_cast<double>(value));
    if (wrapped < 0.0) {
        wrapped += 1.0;
    }
    return static_cast<float>(ExplainoWedgeTween(wrapped));
}

ESCAPE_TIME_COLOR_HD inline EscapeTimeColorRgb SampleExplainoSeedChannels(float basePhase) {
    return {
        EscapeTimeColorExplainoSeed(basePhase),
        EscapeTimeColorExplainoSeed(basePhase + 0.33f),
        EscapeTimeColorExplainoSeed(basePhase + 0.66f),
    };
}

ESCAPE_TIME_COLOR_HD inline EscapeTimeColorRgb ApplyExplainoColorfulness(EscapeTimeColorRgb baseRgb, float colorfulness) {
    const float twoPi = 6.28318530717958647692f;
    const EscapeTimeColorRgb vividRgb{
        0.5f + 0.5f * sinf(twoPi * baseRgb.r),
        0.5f + 0.5f * cosf(twoPi * baseRgb.g),
        0.5f + 0.5f * sinf((2.0f * twoPi) * baseRgb.b),
    };
    const float blend = EscapeTimeColorClamp(colorfulness, 0.0f, 1.0f);
    return {
        EscapeTimeColorLerp(baseRgb.r, vividRgb.r, blend),
        EscapeTimeColorLerp(baseRgb.g, vividRgb.g, blend),
        EscapeTimeColorLerp(baseRgb.b, vividRgb.b, blend),
    };
}

ESCAPE_TIME_COLOR_HD inline EscapeTimeColorRgb SampleExplainoCmap(
    float signalValue,
    const ColorPipelinePaletteRuntimeParams& paletteParams) {
    const float basePhase = signalValue * EscapeTimeColorClamp(paletteParams.seed_scale, 0.25f, 4.0f) +
        EscapeTimeColorClamp(paletteParams.seed_phase, -1.0f, 1.0f);
    return ApplyExplainoColorfulness(
        SampleExplainoSeedChannels(basePhase),
        paletteParams.colorfulness);
}

ESCAPE_TIME_COLOR_HD inline ColorPipelinePaletteRuntimeParams LegacyColorPipelinePaletteRuntimeParams(const KernelParams& params) {
    ColorPipelinePaletteRuntimeParams paletteParams;
    paletteParams.cycle_scale = params.color_heatmap_cycle_scale;
    paletteParams.saturation = params.color_pipeline.palette == ColorPalette::cyclic_escape
        ? params.color_heatmap_saturation
        : 1.0f;
    paletteParams.phase_offset = params.color_phase_palette_offset;
    paletteParams.band_emphasis = params.color_iteration_band_emphasis;
    if (params.color_pipeline.palette == ColorPalette::banded_escape) {
        paletteParams.phase_offset = params.color_iteration_band_palette_offset;
    }
    paletteParams.seed_scale = params.color_explaino_palette_seed_scale;
    paletteParams.seed_phase = params.color_explaino_palette_seed_phase;
    paletteParams.colorfulness = params.color_explaino_palette_colorfulness;
    return paletteParams;
}

ESCAPE_TIME_COLOR_HD inline EscapeTimeColorRgb SampleExplainoCmap(float signalValue, const KernelParams& params) {
    return SampleExplainoCmap(signalValue, LegacyColorPipelinePaletteRuntimeParams(params));
}

ESCAPE_TIME_COLOR_HD inline EscapeTimeColorRgb SampleColorPipelinePaletteRowRgb(
    float signalValue,
    bool escaped,
    const KernelParams& params,
    ColorPalette palette,
    const ColorPipelinePaletteRuntimeParams& paletteParams) {
    const float twoPi = 6.28318530717958647692f;
    if (palette == ColorPalette::explaino_cmap) {
        return SampleExplainoCmap(signalValue, paletteParams);
    }
    if (palette == ColorPalette::phase_wheel) {
        const float hue = signalValue + (EscapeTimeColorClamp(paletteParams.phase_offset, -3.14159265358979323846f, 3.14159265358979323846f) / twoPi);
        const float wrapped = hue - floorf(hue);
        const float c = (escaped ? 0.85f : 0.25f) * 0.9f;
        const float x = c * (1.0f - fabsf(fmodf(wrapped * 6.0f, 2.0f) - 1.0f));
        const float m = (escaped ? 0.85f : 0.25f) - c;
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        const int sector = static_cast<int>(wrapped * 6.0f) % 6;
        if (sector == 0) { r = c; g = x; }
        else if (sector == 1) { r = x; g = c; }
        else if (sector == 2) { g = c; b = x; }
        else if (sector == 3) { g = x; b = c; }
        else if (sector == 4) { r = x; b = c; }
        else { r = c; b = x; }
        return EscapeTimeColorApplySaturation(
            {r + m, g + m, b + m},
            EscapeTimeColorClamp(paletteParams.saturation, 0.0f, 2.0f));
    }
    if (palette == ColorPalette::banded_escape) {
        const int bandCount = params.color_iteration_band_count < 2 ? 2 : params.color_iteration_band_count;
        const float softness = EscapeTimeColorClamp(params.color_iteration_band_softness, 0.0f, 1.0f);
        const float paletteOffset = (EscapeTimeColorClamp(paletteParams.phase_offset, -3.14159265358979323846f, 3.14159265358979323846f) / twoPi) * static_cast<float>(bandCount);
        EscapeTimeColorRgb rgb = SampleIterationBandPalette(signalValue * static_cast<float>(bandCount) + paletteOffset, bandCount, softness);
        return ApplyIterationBandEmphasis(rgb, EscapeTimeColorClamp(paletteParams.band_emphasis, 0.0f, 2.0f));
    }
    const float heatmapBand = palette == ColorPalette::cyclic_escape
        ? signalValue * EscapeTimeColorClamp(paletteParams.cycle_scale, 0.25f, 4.0f)
        : signalValue;
    const float saturation = palette == ColorPalette::cyclic_escape ? paletteParams.saturation : 1.0f;
    return SampleEscapeHeatmap(heatmapBand, saturation);
}

ESCAPE_TIME_COLOR_HD inline EscapeTimeColorRgb BlendColorPipelinePaletteRgb(
    EscapeTimeColorRgb baseRgb,
    EscapeTimeColorRgb nextRgb,
    const ColorPipelinePaletteStackEntry& nextEntry) {
    const float weight = EscapeTimeColorClamp(nextEntry.params.blend_weight, 0.0f, 1.0f);
    return {
        EscapeTimeColorLerp(baseRgb.r, nextRgb.r, weight),
        EscapeTimeColorLerp(baseRgb.g, nextRgb.g, weight),
        EscapeTimeColorLerp(baseRgb.b, nextRgb.b, weight),
    };
}

ESCAPE_TIME_COLOR_HD inline int ClampColorPipelinePaletteStackCount(int count) {
    if (count < 0) {
        return 0;
    }
    return count > kColorPipelineMaxPaletteStackCount ? kColorPipelineMaxPaletteStackCount : count;
}

ESCAPE_TIME_COLOR_HD inline EscapeTimeColorRgb SampleColorPipelinePaletteStackRgb(
    float signalValue,
    bool escaped,
    const KernelParams& params) {
    const int paletteStackCount = ClampColorPipelinePaletteStackCount(params.color_palette_stack_count);
    EscapeTimeColorRgb rgb = SampleColorPipelinePaletteRowRgb(
        signalValue,
        escaped,
        params,
        params.color_palette_stack[0].palette,
        params.color_palette_stack[0].params);
    for (int index = 1; index < paletteStackCount; ++index) {
        const ColorPipelinePaletteStackEntry& nextEntry = params.color_palette_stack[index];
        const EscapeTimeColorRgb nextRgb = SampleColorPipelinePaletteRowRgb(
            signalValue,
            escaped,
            params,
            nextEntry.palette,
            nextEntry.params);
        rgb = BlendColorPipelinePaletteRgb(rgb, nextRgb, nextEntry);
    }
    return rgb;
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color SampleProgrammableEscapeTimePalette(float signalValue, bool escaped, const KernelParams& params) {
    if (ClampColorPipelinePaletteStackCount(params.color_palette_stack_count) > 0) {
        return EscapeTimeColorFromRgb<Color>(SampleColorPipelinePaletteStackRgb(signalValue, escaped, params));
    }
    return EscapeTimeColorFromRgb<Color>(SampleColorPipelinePaletteRowRgb(
        signalValue,
        escaped,
        params,
        params.color_pipeline.palette,
        LegacyColorPipelinePaletteRuntimeParams(params)));
}

ESCAPE_TIME_COLOR_HD inline float ResolveSmoothEscapeInteriorStrength(const KernelParams& params) {
    const float strength = isfinite(params.color_smooth_escape_interior_strength)
        ? params.color_smooth_escape_interior_strength
        : 0.2f;
    return EscapeTimeColorClamp(strength, 0.0f, 1.0f);
}

ESCAPE_TIME_COLOR_HD inline bool ShouldApplySmoothEscapeInteriorStrength(const KernelParams& params) {
    return params.color_pipeline.signal == ColorSignal::smooth_escape &&
        ClampColorPipelineSourceStackCount(params.color_source_stack_count) == 0;
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color ApplySmoothEscapeInteriorStrength(Color fullInteriorColor, const KernelParams& params) {
    const float strength = ResolveSmoothEscapeInteriorStrength(params);
    if (strength >= 0.999f) {
        return fullInteriorColor;
    }
    const Color mutedInteriorColor = SampleProgrammableEscapeTimePalette<Color>(0.0f, false, params);
    return EscapeTimeColorBlendRgb(mutedInteriorColor, fullInteriorColor, strength);
}

// Cyclic iteration-band palette (8 distinct hues).
template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color IterationBandColor(int iteration, int maxIter, const KernelParams& params) {
    const int safeMaxIter = maxIter > 0 ? maxIter : 1;
    const int bandCount = params.color_iteration_band_count < 2 ? 2 : params.color_iteration_band_count;
    const float softness = EscapeTimeColorClamp(params.color_iteration_band_softness, 0.0f, 1.0f);
    const float emphasis = EscapeTimeColorClamp(params.color_iteration_band_emphasis, 0.0f, 2.0f);
    const float twoPi = 6.28318530717958647692f;
    const float paletteOffset = (params.color_iteration_band_palette_offset / twoPi) * static_cast<float>(bandCount);
    const float t = static_cast<float>(iteration) / static_cast<float>(safeMaxIter);
    const float signalBand = ApplyColorPipelineShapeValue(t * static_cast<float>(bandCount), params, static_cast<float>(bandCount));
    EscapeTimeColorRgb rgb = SampleIterationBandPalette(signalBand + paletteOffset, bandCount, softness);
    rgb = ApplyIterationBandEmphasis(rgb, emphasis);
    const float bright = 1.0f - 0.4f * t;
    return EscapeTimeColorMake<Color>(
        static_cast<unsigned char>(EscapeTimeColorClamp(rgb.r * bright, 0.0f, 1.0f) * 255.0f),
        static_cast<unsigned char>(EscapeTimeColorClamp(rgb.g * bright, 0.0f, 1.0f) * 255.0f),
        static_cast<unsigned char>(EscapeTimeColorClamp(rgb.b * bright, 0.0f, 1.0f) * 255.0f),
        255);
}

ESCAPE_TIME_COLOR_HD inline float ResolveBasinResidualMetric(float residual) {
    return -logf(fmaxf(residual, 1.0e-12f));
}

ESCAPE_TIME_COLOR_HD inline float ResolveProjectionAndFlowSmoothEscapeResidual(
    FractalType fractalType,
    float residual,
    const KernelParams& params) {
    if (fractalType != FractalType::projection_and_flow) {
        return residual;
    }
    const float threshold = params.projection_and_flow_pressure_threshold;
    if (!isfinite(threshold) || threshold <= 0.0f) {
        return residual;
    }
    return fmaxf(residual, 0.0f) / fmaxf(threshold, 1.0e-12f);
}

ESCAPE_TIME_COLOR_HD inline float ResolveBasinSmoothEscapeSignal(float residual, const KernelParams& params) {
    return ResolveBasinResidualMetric(residual) * 0.05f *
        EscapeTimeColorClamp(params.color_smooth_escape_scale, 0.25f, 4.0f) +
        EscapeTimeColorClamp(params.color_smooth_escape_bias, -1.0f, 1.0f);
}

template <typename Complex>
ESCAPE_TIME_COLOR_HD inline float ResolveColorPipelineSourceStackEntryBasinSignal(
    FractalType fractalType,
    int iteration,
    int maxIter,
    Complex z,
    float magnitude,
    float angle,
    float residual,
    const KernelParams& params,
    const ColorPipelineSourceStackEntry& entry) {
    if (entry.signal == ColorSignal::root_proximity) {
        return ResolveRootProximitySignal(z, params, entry.params);
    }
    if (entry.signal == ColorSignal::phase_angle ||
        entry.signal == ColorSignal::orbit_stripe) {
        return ResolveAngularSignal(entry.signal, angle, entry.params);
    }
    if (entry.signal == ColorSignal::root_index) {
        int rootIndex = -1;
        int rootCount = 0;
        if (TryResolveColorPipelineRootSample(fractalType, z, params, &rootIndex, &rootCount)) {
            return ResolveColorPipelineRootSignal(rootIndex, rootCount);
        }
        return 0.0f;
    }
    if (entry.signal == ColorSignal::smooth_escape) {
        return ResolveBasinSmoothEscapeSignal(
            ResolveProjectionAndFlowSmoothEscapeResidual(fractalType, residual, params),
            entry.params);
    }
    if (entry.signal == ColorSignal::escape_magnitude) {
        return ResolveEscapeMagnitudeSignal(magnitude, entry.params);
    }
    if (entry.signal == ColorSignal::iteration_bands) {
        return ResolveBandedIterationSignal(iteration, maxIter, entry.params);
    }
    return ResolveIterationRatioSignal(iteration, maxIter);
}

template <typename Complex>
ESCAPE_TIME_COLOR_HD inline float ResolveProgrammableBasinSignal(
    FractalType fractalType,
    int iteration,
    int maxIter,
    Complex z,
    float residual,
    const KernelParams& params) {
    const int sourceStackCount = ClampColorPipelineSourceStackCount(params.color_source_stack_count);
    if (sourceStackCount > 0) {
        const float magnitude = EscapeTimeColorAbs(z);
        const float angle = atan2f(z.y, z.x);
        float blendedSignal = ResolveColorPipelineSourceStackEntryBasinSignal(
            fractalType,
            iteration,
            maxIter,
            z,
            magnitude,
            angle,
            residual,
            params,
            params.color_source_stack[0]);
        for (int index = 1; index < sourceStackCount; ++index) {
            const ColorPipelineSourceStackEntry& entry = params.color_source_stack[index];
            const float nextSignal = ResolveColorPipelineSourceStackEntryBasinSignal(
                fractalType,
                iteration,
                maxIter,
                z,
                magnitude,
                angle,
                residual,
                params,
                entry);
            blendedSignal = EscapeTimeColorLerp(
                blendedSignal,
                nextSignal,
                EscapeTimeColorClamp(entry.params.blend_weight, 0.0f, 1.0f));
        }
        return blendedSignal;
    }
    if (params.color_pipeline.signal == ColorSignal::root_proximity) {
        return ResolveRootProximitySignal(z, params);
    }
    if (params.color_pipeline.signal == ColorSignal::phase_angle ||
        params.color_pipeline.signal == ColorSignal::orbit_stripe) {
        return ResolveAngularSignal(params.color_pipeline.signal, atan2f(z.y, z.x), params);
    }
    if (params.color_pipeline.signal == ColorSignal::root_index) {
        int rootIndex = -1;
        int rootCount = 0;
        if (TryResolveColorPipelineRootSample(fractalType, z, params, &rootIndex, &rootCount)) {
            return ResolveColorPipelineRootSignal(rootIndex, rootCount);
        }
        return 0.0f;
    }
    if (params.color_pipeline.signal == ColorSignal::smooth_escape) {
        return ResolveBasinSmoothEscapeSignal(
            ResolveProjectionAndFlowSmoothEscapeResidual(fractalType, residual, params),
            params);
    }
    if (params.color_pipeline.signal == ColorSignal::escape_magnitude) {
        return ResolveEscapeMagnitudeSignal(EscapeTimeColorAbs(z), params);
    }
    return ResolveIterationRatioSignal(iteration, maxIter);
}

ESCAPE_TIME_COLOR_HD inline bool ShouldUseProgrammableColorForUnescapedSample(const KernelParams& params) {
    return params.color_pipeline.signal == ColorSignal::root_proximity ||
        params.color_pipeline.signal == ColorSignal::smooth_escape ||
        params.color_pipeline.palette == ColorPalette::phase_wheel;
}

template <typename Color, typename Complex>
ESCAPE_TIME_COLOR_HD inline Color MakeProgrammableBasinColor(
    FractalType fractalType,
    bool colorable,
    bool converged,
    int iteration,
    int maxIter,
    Complex z,
    float residual,
    const KernelParams& params) {
    if (!colorable &&
        !ShouldUseProgrammableColorForUnescapedSample(params)) {
        return EscapeTimeColorMake<Color>(0, 0, 0, 255);
    }
    const float shapedSignal = ApplyColorPipelineShapeValue(
        ResolveProgrammableBasinSignal(fractalType, iteration, maxIter, z, residual, params),
        params);
    return SampleProgrammableEscapeTimePalette<Color>(shapedSignal, converged, params);
}

template <typename Color, typename Complex>
ESCAPE_TIME_COLOR_HD inline Color MakeProgrammableBasinColor(
    FractalType fractalType,
    bool converged,
    int iteration,
    int maxIter,
    Complex z,
    float residual,
    const KernelParams& params) {
    return MakeProgrammableBasinColor<Color>(
        fractalType,
        converged,
        converged,
        iteration,
        maxIter,
        z,
        residual,
        params);
}

template <typename Color, typename Complex>
ESCAPE_TIME_COLOR_HD inline Color MakeEscapeTimeBaseColor(
    FractalType fractalType,
    ColoringMode mode,
    bool escaped,
    int iteration,
    int maxIter,
    Complex z,
    const KernelParams& params) {
    Color discreteColor{};
    if (TryMakeDiscreteEscapeTimeColor(fractalType, mode, escaped, iteration, maxIter, z, params, &discreteColor)) {
        return discreteColor;
    }
    if (!escaped && !ShouldUseProgrammableColorForUnescapedSample(params)) {
        return EscapeTimeColorMake<Color>(0, 0, 0, 255);
    }
    const float shapedSignal = ApplyColorPipelineShapeValue(
        ResolveProgrammableEscapeTimeSignal(fractalType, iteration, maxIter, z, params),
        params);
    const Color color = SampleProgrammableEscapeTimePalette<Color>(shapedSignal, escaped, params);
    if (!escaped && ShouldApplySmoothEscapeInteriorStrength(params)) {
        return ApplySmoothEscapeInteriorStrength(color, params);
    }
    return color;
}

#undef ESCAPE_TIME_COLOR_HD
