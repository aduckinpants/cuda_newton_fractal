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

constexpr float kPi = 3.14159265358979323846f;
constexpr float kTwoPi = 2.0f * kPi;
constexpr float kBoundaryBandConfigEpsilon = 1.0e-6f;

int ClampIntLocal(int value, int lo, int hi) {
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

int NormalizePostprocessPixelStep(int value) {
    if (value <= 1) return 1;
    if (value <= 2) return 2;
    return 4;
}

bool SdfSignalNeedsNeighborhood(ColorSignal signal) {
    return signal == ColorSignal::sdf_normal_angle || signal == ColorSignal::sdf_curvature;
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

float ResolveSdfSourceValueFromSample(
    const SdfFieldSignalSample& sample,
    const ColorPipelineSourceStackEntry& entry) {
    float value = ResolveSdfFieldSignalValue(sample, SignalKindForColorSignal(entry.signal));
    if (entry.signal == ColorSignal::sdf_normal_angle) {
        value = (value + kPi) / kTwoPi;
    }
    return value * entry.params.scale + entry.params.bias;
}

bool BoundaryBandConfigMatches(const SdfFieldSignalConfig& left, const SdfFieldSignalConfig& right) {
    return std::fabs(left.boundary_band_px - right.boundary_band_px) <= kBoundaryBandConfigEpsilon;
}

bool EntryNeedsDistinctSignalSample(
    const ColorPipelineSourceStackEntry& entry,
    const SdfFieldSignalConfig& cachedConfig) {
    if (entry.signal != ColorSignal::sdf_boundary_band) {
        return false;
    }
    return !BoundaryBandConfigMatches(SignalConfigForSourceEntry(entry), cachedConfig);
}

bool SampleSdfCenterValue(const SdfFieldView& field, int fx, int fy, float* outValue) {
    if (!outValue || !field.signed_distance_px || fx < 0 || fy < 0 || fx >= field.width || fy >= field.height) {
        return false;
    }
    const std::size_t index = static_cast<std::size_t>(fy) * static_cast<std::size_t>(field.width) +
        static_cast<std::size_t>(fx);
    const float value = field.signed_distance_px[index];
    if (!std::isfinite(value)) {
        return false;
    }
    *outValue = value;
    return true;
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
    *outValue = ResolveSdfSourceValueFromSample(sample, entry);
    return true;
}

bool ResolveDirectSdfSourceValueFromCenter(
    float center,
    const ColorPipelineSourceStackEntry& entry,
    float* outValue) {
    if (!outValue || SdfSignalNeedsNeighborhood(entry.signal)) {
        return false;
    }
    float value = center;
    if (entry.signal == ColorSignal::sdf_inside_outside) {
        value = center < 0.0f ? 1.0f : 0.0f;
    } else if (entry.signal == ColorSignal::sdf_boundary_band) {
        value = ResolveSdfBoundaryBandFromSignedDistancePx(center, SignalConfigForSourceEntry(entry));
    } else if (entry.signal != ColorSignal::sdf_signed_distance) {
        return false;
    }
    *outValue = value * entry.params.scale + entry.params.bias;
    return true;
}

ColorPipelineSourceStackEntry FlatSdfSourceEntry(const KernelParams& params) {
    ColorPipelineSourceStackEntry entry;
    entry.signal = params.color_pipeline.signal;
    return entry;
}

bool ResolveDirectSdfPipelineSignal(
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
    float center = 0.0f;
    if (!SampleSdfCenterValue(field, fx, fy, &center)) {
        if (outError) *outError = "SDF Color Pipeline postprocess could not sample the Lens SDF field";
        return false;
    }

    const int sourceStackCount = ClampColorPipelineSourceStackCount(params.color_source_stack_count);
    if (sourceStackCount <= 0) {
        if (!IsColorPipelineSdfSourceSignal(params.color_pipeline.signal)) {
            if (outError) *outError = "SDF Color Pipeline postprocess was requested without an SDF source signal";
            return false;
        }
        if (!ResolveDirectSdfSourceValueFromCenter(center, FlatSdfSourceEntry(params), outSignal)) {
            if (outError) *outError = "SDF Color Pipeline postprocess needs neighborhood sampling for this SDF source";
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
    if (!ResolveDirectSdfSourceValueFromCenter(center, first, &signal)) {
        if (outError) *outError = "SDF Color Pipeline postprocess needs neighborhood sampling for this SDF source";
        return false;
    }
    for (int index = 1; index < sourceStackCount; ++index) {
        const ColorPipelineSourceStackEntry& entry = params.color_source_stack[index];
        if (!IsColorPipelineSdfSourceSignal(entry.signal)) {
            if (outError) *outError = "SDF Color Pipeline source stack mixes SDF and non-SDF Source rows";
            return false;
        }
        float nextSignal = 0.0f;
        if (!ResolveDirectSdfSourceValueFromCenter(center, entry, &nextSignal)) {
            if (outError) *outError = "SDF Color Pipeline postprocess needs neighborhood sampling for this SDF source";
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
    const SdfFieldSignalConfig cachedConfig = SignalConfigForSourceEntry(first);
    SdfFieldSignalSample cachedSample;
    if (!SampleSdfFieldSignals(field, fx, fy, cachedConfig, cachedSample)) {
        if (outError) *outError = "SDF Color Pipeline postprocess could not sample the Lens SDF field";
        return false;
    }
    float signal = ResolveSdfSourceValueFromSample(cachedSample, first);
    for (int index = 1; index < sourceStackCount; ++index) {
        const ColorPipelineSourceStackEntry& entry = params.color_source_stack[index];
        if (!IsColorPipelineSdfSourceSignal(entry.signal)) {
            if (outError) *outError = "SDF Color Pipeline source stack mixes SDF and non-SDF Source rows";
            return false;
        }
        const SdfFieldSignalSample* sample = &cachedSample;
        SdfFieldSignalSample distinctSample;
        if (EntryNeedsDistinctSignalSample(entry, cachedConfig)) {
            if (!SampleSdfFieldSignals(field, fx, fy, SignalConfigForSourceEntry(entry), distinctSample)) {
                if (outError) *outError = "SDF Color Pipeline postprocess could not sample the Lens SDF field";
                return false;
            }
            sample = &distinctSample;
        }
        const float nextSignal = ResolveSdfSourceValueFromSample(*sample, entry);
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

bool ColorPipelineSdfPostprocessCanUseDirectSamples(const KernelParams& params) {
    const int sourceStackCount = ClampColorPipelineSourceStackCount(params.color_source_stack_count);
    if (sourceStackCount > 0) {
        for (int index = 0; index < sourceStackCount; ++index) {
            const ColorSignal signal = params.color_source_stack[index].signal;
            if (!IsColorPipelineSdfSourceSignal(signal) || SdfSignalNeedsNeighborhood(signal)) {
                return false;
            }
        }
        return true;
    }
    return IsColorPipelineSdfSourceSignal(params.color_pipeline.signal) &&
        !SdfSignalNeedsNeighborhood(params.color_pipeline.signal);
}

bool ApplyLensSdfColorPipelinePostprocess(
    const SdfFieldView& field,
    const RenderSettings& render,
    const KernelParams& params,
    std::uint32_t* ioRgba,
    std::string* outError,
    SdfColorPipelinePostprocessStats* outStats,
    const SdfColorPipelinePostprocessOptions* options) {
    if (!ioRgba || render.resolution.x <= 0 || render.resolution.y <= 0) {
        if (outError) *outError = "SDF Color Pipeline postprocess received an invalid render target";
        return false;
    }
    if (field.width <= 0 || field.height <= 0 || !field.signed_distance_px) {
        if (outError) *outError = "SDF Color Pipeline postprocess requires a valid Lens SDF field";
        return false;
    }
    const int outputPixelStep = NormalizePostprocessPixelStep(options ? options->output_pixel_step : 1);
    if (outStats) {
        *outStats = {};
        outStats->output_pixel_step = outputPixelStep;
    }
    if (!ColorPipelineSourceStackIsSdfOnly(params, outError)) {
        return false;
    }

    const bool useDirectSamples = ColorPipelineSdfPostprocessCanUseDirectSamples(params);
    const int width = render.resolution.x;
    const int height = render.resolution.y;
    for (int y = 0; y < height; y += outputPixelStep) {
        const int blockHeight = ClampIntLocal(height - y, 1, outputPixelStep);
        const int sampleY = ClampIntLocal(y + blockHeight / 2, 0, height - 1);
        const int fy = ClampIntLocal((static_cast<long long>(sampleY) * field.height) / height, 0, field.height - 1);
        for (int x = 0; x < width; x += outputPixelStep) {
            const int blockWidth = ClampIntLocal(width - x, 1, outputPixelStep);
            const int sampleX = ClampIntLocal(x + blockWidth / 2, 0, width - 1);
            const int fx = ClampIntLocal((static_cast<long long>(sampleX) * field.width) / width, 0, field.width - 1);
            float signal = 0.0f;
            const bool resolved = useDirectSamples
                ? ResolveDirectSdfPipelineSignal(field, fx, fy, params, &signal, outError)
                : ResolveSdfPipelineSignal(field, fx, fy, params, &signal, outError);
            if (!resolved) {
                return false;
            }
            if (outStats) {
                if (useDirectSamples) {
                    ++outStats->direct_sample_count;
                } else {
                    ++outStats->neighborhood_sample_count;
                }
            }
            const float shapedSignal = ApplyColorPipelineShapeValue(signal, params);
            Rgba8 color = SampleProgrammableEscapeTimePalette<Rgba8>(shapedSignal, true, params);
            color = ApplyFractalColorGrading(color, params);
            const std::uint32_t packed = PackRgba(color);
            for (int yy = y; yy < y + blockHeight; ++yy) {
                const std::size_t rowOffset = static_cast<std::size_t>(yy) * static_cast<std::size_t>(width);
                for (int xx = x; xx < x + blockWidth; ++xx) {
                    ioRgba[rowOffset + static_cast<std::size_t>(xx)] = packed;
                    if (outStats) {
                        ++outStats->filled_pixel_count;
                    }
                }
            }
        }
    }
    return true;
}
