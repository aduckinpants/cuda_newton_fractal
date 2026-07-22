#pragma once

#include <cmath>

#if defined(__CUDACC__)
#define FRACTAL_VIEWPORT_HD __host__ __device__
#else
#define FRACTAL_VIEWPORT_HD
#endif

struct FractalViewportMappingTransform {
    double aspect_ratio{1.0};
    double resolved_zoom{1.0};
    double base_half_height{2.0};
    bool has_rotation{false};
    double cosine{1.0};
    double sine{0.0};
};

struct FractalViewportMappedPoint {
    double real{0.0};
    double imag{0.0};
};

FRACTAL_VIEWPORT_HD inline FractalViewportMappingTransform BuildFractalViewportMappingTransform(
    int width,
    int height,
    double log2Zoom,
    double rotationDegrees) {
    FractalViewportMappingTransform transform;
    transform.aspect_ratio = height > 0 ? static_cast<double>(width) / static_cast<double>(height) : 1.0;
    transform.resolved_zoom = ::fmax(1.0e-300, ::exp2(log2Zoom));
    transform.base_half_height = 2.0 / transform.resolved_zoom;
    transform.has_rotation = rotationDegrees != 0.0;
    // Preserve the renderer's historical float CUDART_PI_F / 180.0f conversion exactly.
    const double radians = rotationDegrees * 0.01745329238474369049072265625;
    transform.cosine = ::cos(radians);
    transform.sine = ::sin(radians);
    return transform;
}

FRACTAL_VIEWPORT_HD inline FractalViewportMappedPoint MapFractalViewportLocalPoint(
    const FractalViewportMappingTransform& transform,
    double centerReal,
    double centerImag,
    double localReal,
    double localImag) {
    const double unrotatedReal = centerReal + localReal;
    const double unrotatedImag = centerImag + localImag;
    if (!transform.has_rotation) {
        return {unrotatedReal, unrotatedImag};
    }
    const double centeredReal = unrotatedReal - centerReal;
    const double centeredImag = unrotatedImag - centerImag;
    return {
        centerReal + centeredReal * transform.cosine - centeredImag * transform.sine,
        centerImag + centeredReal * transform.sine + centeredImag * transform.cosine,
    };
}

FRACTAL_VIEWPORT_HD inline FractalViewportMappedPoint MapFractalViewportSample(
    const FractalViewportMappingTransform& transform,
    double centerReal,
    double centerImag,
    int width,
    int height,
    int pixelX,
    int pixelY,
    double sampleOffsetX,
    double sampleOffsetY) {
    const double normalizedX =
        ((static_cast<double>(pixelX) + 0.5 + sampleOffsetX) / static_cast<double>(width) - 0.5) * 2.0;
    const double normalizedY =
        ((static_cast<double>(pixelY) + 0.5 + sampleOffsetY) / static_cast<double>(height) - 0.5) * 2.0;
    return MapFractalViewportLocalPoint(
        transform,
        centerReal,
        centerImag,
        normalizedX * transform.base_half_height * transform.aspect_ratio,
        normalizedY * transform.base_half_height);
}

#undef FRACTAL_VIEWPORT_HD
