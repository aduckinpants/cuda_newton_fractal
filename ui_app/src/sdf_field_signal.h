#pragma once

#include "lens_sdf.h"

enum class SdfFieldSignalKind {
    signed_distance_px,
    lens_field_v2_response,
    inside_outside,
    boundary_band,
    normal_angle_radians,
    curvature_estimate,
};

struct SdfFieldSignalConfig {
    float boundary_band_px{2.0f};
};

struct SdfFieldSignalSample {
    bool ok{false};
    float signed_distance_px{0.0f};
    float lens_field_v2_response{0.5f};
    bool inside{false};
    float inside_outside{0.0f};
    float boundary_band{0.0f};
    float normal_angle_radians{0.0f};
    float curvature_estimate{0.0f};
};

float ResolveSdfBoundaryBandFromSignedDistancePx(float signed_distance_px, const SdfFieldSignalConfig& config);
float ResolveLensFieldV2ResponseFromSignedDistancePx(float signed_distance_px, float field_pixel_scale);

bool SampleSdfFieldSignals(
    const SdfFieldView& field,
    int x,
    int y,
    const SdfFieldSignalConfig& config,
    SdfFieldSignalSample& outSample);

float ResolveSdfFieldSignalValue(const SdfFieldSignalSample& sample, SdfFieldSignalKind kind);
