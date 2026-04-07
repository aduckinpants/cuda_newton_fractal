#pragma once

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

ESCAPE_TIME_COLOR_HD inline unsigned char EscapeTimeColorToneMap(unsigned char value, float exposure) {
    float x = (static_cast<float>(value) / 255.0f) * exposure;
    x = 1.0f - expf(-x);
    return static_cast<unsigned char>(EscapeTimeColorClamp(x, 0.0f, 1.0f) * 255.0f);
}

template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color ApplyFractalColorGrading(Color color, const KernelParams& params) {
    const float exposure = params.exposure < 0.0f ? 0.0f : params.exposure;
    color = EscapeTimeColorMake<Color>(
        EscapeTimeColorToneMap(color.x, exposure),
        EscapeTimeColorToneMap(color.y, exposure),
        EscapeTimeColorToneMap(color.z, exposure),
        255);

    color = EscapeTimeColorMake<Color>(
        static_cast<unsigned char>(EscapeTimeColorClamp(static_cast<float>(color.x) * params.color_tint_r, 0.0f, 255.0f)),
        static_cast<unsigned char>(EscapeTimeColorClamp(static_cast<float>(color.y) * params.color_tint_g, 0.0f, 255.0f)),
        static_cast<unsigned char>(EscapeTimeColorClamp(static_cast<float>(color.z) * params.color_tint_b, 0.0f, 255.0f)),
        color.w);

    const float lum = 0.299f * static_cast<float>(color.x) + 0.587f * static_cast<float>(color.y) + 0.114f * static_cast<float>(color.z);
    color = EscapeTimeColorMake<Color>(
        static_cast<unsigned char>(EscapeTimeColorClamp(lum + params.color_saturation * (static_cast<float>(color.x) - lum), 0.0f, 255.0f)),
        static_cast<unsigned char>(EscapeTimeColorClamp(lum + params.color_saturation * (static_cast<float>(color.y) - lum), 0.0f, 255.0f)),
        static_cast<unsigned char>(EscapeTimeColorClamp(lum + params.color_saturation * (static_cast<float>(color.z) - lum), 0.0f, 255.0f)),
        color.w);

    return EscapeTimeColorMake<Color>(
        static_cast<unsigned char>(EscapeTimeColorClamp(128.0f + params.color_contrast * (static_cast<float>(color.x) - 128.0f), 0.0f, 255.0f)),
        static_cast<unsigned char>(EscapeTimeColorClamp(128.0f + params.color_contrast * (static_cast<float>(color.y) - 128.0f), 0.0f, 255.0f)),
        static_cast<unsigned char>(EscapeTimeColorClamp(128.0f + params.color_contrast * (static_cast<float>(color.z) - 128.0f), 0.0f, 255.0f)),
        color.w);
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

// Cyclic iteration-band palette (8 distinct hues).
template <typename Color>
ESCAPE_TIME_COLOR_HD inline Color IterationBandColor(int iteration, int maxIter) {
    const float palette[8][3] = {
        {0.05f, 0.15f, 0.45f},
        {0.10f, 0.50f, 0.70f},
        {0.15f, 0.75f, 0.55f},
        {0.65f, 0.85f, 0.20f},
        {0.95f, 0.75f, 0.15f},
        {0.90f, 0.40f, 0.10f},
        {0.70f, 0.15f, 0.30f},
        {0.40f, 0.10f, 0.55f},
    };
    int idx = iteration % 8;
    float t = static_cast<float>(iteration) / static_cast<float>(maxIter);
    float bright = 1.0f - 0.4f * t;
    return EscapeTimeColorMake<Color>(
        static_cast<unsigned char>(EscapeTimeColorClamp(palette[idx][0] * bright, 0.0f, 1.0f) * 255.0f),
        static_cast<unsigned char>(EscapeTimeColorClamp(palette[idx][1] * bright, 0.0f, 1.0f) * 255.0f),
        static_cast<unsigned char>(EscapeTimeColorClamp(palette[idx][2] * bright, 0.0f, 1.0f) * 255.0f),
        255);
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
    (void)fractalType;

    const Color errorColor = EscapeTimeColorMake<Color>(255, 0, 255, 255);
    if (mode == ColoringMode::root_basin || mode == ColoringMode::joy_basins) {
        return errorColor;
    }

    if (mode == ColoringMode::phase) {
        float angle = atan2f(z.y, z.x);
        float h = (angle + 3.14159265f) / (2.0f * 3.14159265f);
        float v = escaped ? 0.85f : 0.25f;
        return HsvToRgb<Color>(h, 0.9f, v);
    }

    if (mode == ColoringMode::iteration_bands) {
        if (!escaped) return EscapeTimeColorMake<Color>(0, 0, 0, 255);
        return IterationBandColor<Color>(iteration, maxIter);
    }

    if (!escaped) {
        return EscapeTimeColorMake<Color>(0, 0, 0, 255);
    }

    if (mode == ColoringMode::iteration_count) {
        const float t = static_cast<float>(iteration) / static_cast<float>(maxIter);
        const unsigned char value = static_cast<unsigned char>(EscapeTimeColorClamp(t, 0.0f, 1.0f) * 255.0f);
        return EscapeTimeColorMake<Color>(64, value, static_cast<unsigned char>(255 - value), 255);
    }

    const float magnitude = EscapeTimeColorAbs(z);
    const float logZn = logf(fmaxf(magnitude, 1.0e-12f));

    float denom = logf(2.0f);
    if (fractalType == FractalType::multibrot && params.multibrot_power_float > 1.0f) {
        denom = logf(params.multibrot_power_float);
    }

    const float nu = static_cast<float>(iteration) + 1.0f - logf(fmaxf(logZn / denom, 1.0e-12f)) / denom;
    const float band = nu * 0.025f;
    const float frac = band - floorf(band);

    const float stops[6][3] = {
        {0.00f, 0.03f, 0.20f},
        {0.05f, 0.35f, 0.65f},
        {0.10f, 0.75f, 0.85f},
        {0.95f, 0.85f, 0.25f},
        {0.90f, 0.45f, 0.10f},
        {0.00f, 0.03f, 0.20f},
    };

    const float u5 = frac * 5.0f;
    int segment = static_cast<int>(u5);
    if (segment > 4) segment = 4;
    const float segT = u5 - static_cast<float>(segment);
    const float rf = stops[segment][0] + (stops[segment + 1][0] - stops[segment][0]) * segT;
    const float gf = stops[segment][1] + (stops[segment + 1][1] - stops[segment][1]) * segT;
    const float bf = stops[segment][2] + (stops[segment + 1][2] - stops[segment][2]) * segT;

    return EscapeTimeColorMake<Color>(
        static_cast<unsigned char>(EscapeTimeColorClamp(rf, 0.0f, 1.0f) * 255.0f),
        static_cast<unsigned char>(EscapeTimeColorClamp(gf, 0.0f, 1.0f) * 255.0f),
        static_cast<unsigned char>(EscapeTimeColorClamp(bf, 0.0f, 1.0f) * 255.0f),
        255);
}

#undef ESCAPE_TIME_COLOR_HD