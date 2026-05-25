#include "sdf_field_signal.h"

#include <cmath>
#include <cstddef>

namespace {

bool IsValidField(const SdfFieldView& field) {
    if (!field.signed_distance_px || field.width <= 0 || field.height <= 0) return false;
    if (field.sign_convention != SdfSignConvention::negative_inside_positive_outside) return false;
    const std::size_t expectedCount = static_cast<std::size_t>(field.width) * static_cast<std::size_t>(field.height);
    return field.signed_distance_count == expectedCount;
}

int ClampInt(int value, int lo, int hi) {
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

bool SampleRaw(const SdfFieldView& field, int x, int y, float& outValue) {
    if (!IsValidField(field)) return false;
    if (x < 0 || y < 0 || x >= field.width || y >= field.height) return false;
    const std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(field.width) + static_cast<std::size_t>(x);
    outValue = field.signed_distance_px[index];
    return std::isfinite(outValue);
}

bool SampleRawClamped(const SdfFieldView& field, int x, int y, float& outValue) {
    return SampleRaw(
        field,
        ClampInt(x, 0, field.width - 1),
        ClampInt(y, 0, field.height - 1),
        outValue);
}

} // namespace

float ResolveSdfBoundaryBandFromSignedDistancePx(float signed_distance_px, const SdfFieldSignalConfig& config) {
    const float safeBandPx = std::isfinite(config.boundary_band_px) && config.boundary_band_px > 0.000001f
        ? config.boundary_band_px
        : 0.000001f;
    float boundaryBand = 1.0f - (std::fabs(signed_distance_px) / safeBandPx);
    if (boundaryBand < 0.0f) boundaryBand = 0.0f;
    if (boundaryBand > 1.0f) boundaryBand = 1.0f;
    return boundaryBand;
}

bool SampleSdfFieldSignals(
    const SdfFieldView& field,
    int x,
    int y,
    const SdfFieldSignalConfig& config,
    SdfFieldSignalSample& outSample) {
    outSample = SdfFieldSignalSample{};
    float center = 0.0f;
    if (!SampleRaw(field, x, y, center)) return false;

    float left = 0.0f;
    float right = 0.0f;
    float up = 0.0f;
    float down = 0.0f;
    if (!SampleRawClamped(field, x - 1, y, left) ||
        !SampleRawClamped(field, x + 1, y, right) ||
        !SampleRawClamped(field, x, y - 1, up) ||
        !SampleRawClamped(field, x, y + 1, down)) {
        return false;
    }

    const float boundaryBand = ResolveSdfBoundaryBandFromSignedDistancePx(center, config);

    const float gradX = 0.5f * (right - left);
    const float gradY = 0.5f * (down - up);
    const float gradMag2 = gradX * gradX + gradY * gradY;

    outSample.ok = true;
    outSample.signed_distance_px = center;
    outSample.inside = center < 0.0f;
    outSample.inside_outside = outSample.inside ? 1.0f : 0.0f;
    outSample.boundary_band = boundaryBand;
    outSample.normal_angle_radians = gradMag2 > 0.000000000001f ? std::atan2(gradY, gradX) : 0.0f;
    outSample.curvature_estimate = left + right + up + down - (4.0f * center);
    return std::isfinite(outSample.normal_angle_radians) && std::isfinite(outSample.curvature_estimate);
}

float ResolveSdfFieldSignalValue(const SdfFieldSignalSample& sample, SdfFieldSignalKind kind) {
    switch (kind) {
    case SdfFieldSignalKind::signed_distance_px: return sample.signed_distance_px;
    case SdfFieldSignalKind::inside_outside: return sample.inside_outside;
    case SdfFieldSignalKind::boundary_band: return sample.boundary_band;
    case SdfFieldSignalKind::normal_angle_radians: return sample.normal_angle_radians;
    case SdfFieldSignalKind::curvature_estimate: return sample.curvature_estimate;
    }
    return sample.signed_distance_px;
}
