#include "color_pipeline_sdf_postprocess.h"

#include "escape_time_coloring.h"
#include "sdf_field_signal.h"

#include <cmath>
#include <cstddef>

namespace {

struct Rgba8 {
    unsigned char x;
    unsigned char y;
    unsigned char z;
    unsigned char w;
};

constexpr float kTwoPi = 6.28318530717958647692f;

int ClampIntLocal(int value, int lo, int hi) {
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

SdfFieldSignalKind SignalKindForColorSignal(ColorSignal signal) {
    switch (signal) {
    case ColorSignal::sdf_inside_outside:
        return SdfFieldSignalKind::inside_outside;
    case ColorSignal::sdf_boundary_band:
        return SdfFieldSignalKind::boundary_band;
    case ColorSignal::sdf_normal_angle:
        return SdfFieldSignalKind::normal_angle_radians;
    case ColorSignal::sdf_curvature:
        return SdfFieldSignalKind::curvature_estimate;
    case ColorSignal::sdf_signed_distance:
    default:
        return SdfFieldSignalKind::signed_distance_px;
    }
}

SdfFieldSignalConfig SignalConfigForSourceEntry(const ColorPipelineSourceStackEntry& entry) {
    SdfFieldSignalConfig config{};
    if (entry.signal == ColorSignal::sdf_boundary_band) {
        config.boundary_band_px = EscapeTimeColorClamp(entry.params.sdf_boundary_width_px, 0.25f, 16.0f);
    }
    return config;
}

bool ResolveSdfSourceValue(
    const SdfFieldView& field,
    int fx,
    int fy,
    const ColorPipelineSourceStackEntry& entry,
    float* outValue) {
    if (!outValue) {
        return false;
    }
    SdfFieldSignalSample sample;
    if (!SampleSdfFieldSignals(field, fx, fy, SignalConfigForSourceEntry(entry), sample)) {
        return false;
    }
    float value = ResolveSdfFieldSignalValue(sample, SignalKindForColorSignal(entry.signal));
    if (entry.signal == ColorSignal::sdf_normal_angle) {
        value = (value + 3.14159265358979323846f) / kTwoPi;
    }
    *outValue = value * entry.params.scale + entry.params.bias;
    return true;
}

ColorPipelineSourceStackEntry FlatSdfSourceEntry(const KernelParams& params) {
    ColorPipelineSourceStackEntry entry;
    entry.signal = params.color_pipeline.signal;
    return entry;
}

bool ResolveSdfPipelineSignal(
    const SdfFieldView& field,
    int fx,
    int fy,
    const KernelParams& params,
    float* outSignal,
    std::string* outError) {
    if (!outSignal) {
        if (outError) *outError = "SDF Color Pipeline postprocess requires output storage";
        return false;
    }

    const int sourceStackCount = ClampColorPipelineSourceStackCount(params.color_source_stack_count);
    if (sourceStackCount <= 0) {
        if (!IsColorPipelineSdfSourceSignal(params.color_pipeline.signal)) {
            if (outError) *outError = "SDF Color Pipeline postprocess was requested without an SDF source signal";
            return false;
        }
        if (!ResolveSdfSourceValue(field, fx, fy, FlatSdfSourceEntry(params), outSignal)) {
            if (outError) *outError = "SDF Color Pipeline postprocess could not sample the Lens SDF field";
            return false;
        }
        return true;
    }

    const ColorPipelineSourceStackEntry& first = params.color_source_stack[0];
    if (!IsColorPipelineSdfSourceSignal(first.signal)) {
        if (outError) *outError = "SDF Color Pipeline source stack contains a non-SDF first Source row";
        return false;
    }
    float signal = 0.0f;
    if (!ResolveSdfSourceValue(field, fx, fy, first, &signal)) {
        if (outError) *outError = "SDF Color Pipeline postprocess could not sample the Lens SDF field";
        return false;
    }
    for (int index = 1; index < sourceStackCount; ++index) {
        const ColorPipelineSourceStackEntry& entry = params.color_source_stack[index];
        if (!IsColorPipelineSdfSourceSignal(entry.signal)) {
            if (outError) *outError = "SDF Color Pipeline source stack mixes SDF and non-SDF Source rows";
            return false;
        }
        float nextSignal = 0.0f;
        if (!ResolveSdfSourceValue(field, fx, fy, entry, &nextSignal)) {
            if (outError) *outError = "SDF Color Pipeline postprocess could not sample the Lens SDF field";
            return false;
        }
        signal = EscapeTimeColorLerp(
            signal,
            nextSignal,
            EscapeTimeColorClamp(entry.params.blend_weight, 0.0f, 1.0f));
    }
    *outSignal = signal;
    return true;
}

std::uint32_t PackRgba(Rgba8 color) {
    return static_cast<std::uint32_t>(color.x) |
        (static_cast<std::uint32_t>(color.y) << 8) |
        (static_cast<std::uint32_t>(color.z) << 16) |
        (static_cast<std::uint32_t>(color.w) << 24);
}

} // namespace

bool IsColorPipelineSdfSourceSignal(ColorSignal signal) {
    return signal == ColorSignal::sdf_signed_distance ||
        signal == ColorSignal::sdf_inside_outside ||
        signal == ColorSignal::sdf_boundary_band ||
        signal == ColorSignal::sdf_normal_angle ||
        signal == ColorSignal::sdf_curvature;
}

bool ColorPipelineUsesSdfSource(const KernelParams& params) {
    const int sourceStackCount = ClampColorPipelineSourceStackCount(params.color_source_stack_count);
    if (sourceStackCount > 0) {
        for (int index = 0; index < sourceStackCount; ++index) {
            if (IsColorPipelineSdfSourceSignal(params.color_source_stack[index].signal)) {
                return true;
            }
        }
        return false;
    }
    return IsColorPipelineSdfSourceSignal(params.color_pipeline.signal);
}

bool ColorPipelineSourceStackIsSdfOnly(const KernelParams& params, std::string* outError) {
    bool hasSdf = false;
    bool hasNonSdf = false;
    const int sourceStackCount = ClampColorPipelineSourceStackCount(params.color_source_stack_count);
    if (sourceStackCount > 0) {
        for (int index = 0; index < sourceStackCount; ++index) {
            const bool isSdf = IsColorPipelineSdfSourceSignal(params.color_source_stack[index].signal);
            hasSdf = hasSdf || isSdf;
            hasNonSdf = hasNonSdf || !isSdf;
        }
    } else {
        hasSdf = IsColorPipelineSdfSourceSignal(params.color_pipeline.signal);
        hasNonSdf = !hasSdf;
    }
    if (hasSdf && hasNonSdf) {
        if (outError) *outError = "SDF Color Pipeline source stacks cannot mix SDF and non-SDF Source rows";
        return false;
    }
    return hasSdf;
}

bool ApplyLensSdfColorPipelinePostprocess(
    const SdfFieldView& field,
    const RenderSettings& render,
    const KernelParams& params,
    std::uint32_t* ioRgba,
    std::string* outError) {
    if (!ioRgba || render.resolution.x <= 0 || render.resolution.y <= 0) {
        if (outError) *outError = "SDF Color Pipeline postprocess received an invalid render target";
        return false;
    }
    if (field.width <= 0 || field.height <= 0 || !field.signed_distance_px) {
        if (outError) *outError = "SDF Color Pipeline postprocess requires a valid Lens SDF field";
        return false;
    }
    if (!ColorPipelineSourceStackIsSdfOnly(params, outError)) {
        return false;
    }

    const int width = render.resolution.x;
    const int height = render.resolution.y;
    for (int y = 0; y < height; ++y) {
        const int fy = ClampIntLocal((static_cast<long long>(y) * field.height) / height, 0, field.height - 1);
        for (int x = 0; x < width; ++x) {
            const int fx = ClampIntLocal((static_cast<long long>(x) * field.width) / width, 0, field.width - 1);
            float signal = 0.0f;
            if (!ResolveSdfPipelineSignal(field, fx, fy, params, &signal, outError)) {
                return false;
            }
            const float shapedSignal = ApplyColorPipelineShapeValue(signal, params);
            Rgba8 color = SampleProgrammableEscapeTimePalette<Rgba8>(shapedSignal, true, params);
            color = ApplyFractalColorGrading(color, params);
            ioRgba[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)] =
                PackRgba(color);
        }
    }
    return true;
}
