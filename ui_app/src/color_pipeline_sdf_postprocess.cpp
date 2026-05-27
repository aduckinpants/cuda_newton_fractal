#include "color_pipeline_sdf_postprocess.h"

#include "escape_time_coloring.h"
#include "sdf_field_signal.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <thread>
#include <vector>

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
constexpr std::size_t kParallelPostprocessSampleThreshold = 4096;
constexpr int kMaxAutomaticSdfPostprocessWorkers = 6;

SdfColorPipelinePostprocessBackendFn& RegisteredCudaDirectScalarBackend() {
    static SdfColorPipelinePostprocessBackendFn backendFn = nullptr;
    return backendFn;
}

SdfColorPipelinePostprocessBackendFn& RegisteredCudaFieldSignalBackend() {
    static SdfColorPipelinePostprocessBackendFn backendFn = nullptr;
    return backendFn;
}

struct PlannedSdfSourceRow {
    ColorSignal signal{ColorSignal::sdf_signed_distance};
    SdfFieldSignalKind kind{SdfFieldSignalKind::signed_distance_px};
    SdfFieldSignalConfig config{};
    ColorPipelineSdfGateMode gate{ColorPipelineSdfGateMode::none};
    float gate_width_px{2.0f};
    float lens_field_v2_sign_contrast{0.0f};
    float scale{1.0f};
    float bias{0.0f};
    float blend_weight{1.0f};
    int sample_step{1};
    bool needs_neighborhood{false};
};

struct SdfPostprocessExecutionPlan {
    std::array<PlannedSdfSourceRow, kColorPipelineMaxSourceStackCount> rows{};
    int row_count{0};
    bool use_direct_samples{true};
    bool uses_row_sample_steps{false};
    int output_pixel_step{1};
    int worker_count{1};
};

struct SdfRowSampleCache {
    int cached_step{1};
    int cached_qfy{-1};
    std::vector<float> values{};
    std::vector<unsigned char> valid{};
};

struct SdfPostprocessResolveScratch {
    std::array<SdfRowSampleCache, kColorPipelineMaxSourceStackCount> row_caches{};
};

int ClampIntLocal(int value, int lo, int hi) {
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

int CeilDivNonNegativeLocal(long long numerator, int denominator) {
    if (denominator <= 0) return 0;
    if (numerator <= 0) return 0;
    return static_cast<int>((numerator + static_cast<long long>(denominator) - 1) /
        static_cast<long long>(denominator));
}

int NormalizePostprocessPixelStep(int value) {
    if (value <= 1) return 1;
    if (value <= 2) return 2;
    return 4;
}

int NormalizeSdfSourceSampleStep(int value) {
    if (value <= 1) return 1;
    if (value > 8) return 8;
    return value;
}

int QuantizeSdfSampleCoordinate(int coord, int extent, int sampleStep) {
    if (extent <= 1 || sampleStep <= 1) {
        return ClampIntLocal(coord, 0, std::max(0, extent - 1));
    }
    const int blockBegin = (coord / sampleStep) * sampleStep;
    const int blockEnd = std::min(extent, blockBegin + sampleStep);
    return ClampIntLocal(blockBegin + (blockEnd - blockBegin) / 2, 0, extent - 1);
}

int SdfSampleBlockCount(int extent, int sampleStep) {
    return CeilDivNonNegativeLocal(extent, std::max(1, sampleStep));
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
    case ColorSignal::lens_field_v2_distance:
        return SdfFieldSignalKind::lens_field_v2_response;
    case ColorSignal::sdf_signed_distance:
    default:
        return SdfFieldSignalKind::signed_distance_px;
    }
}

SdfFieldSignalConfig SignalConfigForPlannedRow(const PlannedSdfSourceRow& row) {
    SdfFieldSignalConfig config{};
    if (row.signal == ColorSignal::sdf_boundary_band) {
        config.boundary_band_px = row.config.boundary_band_px;
    }
    return config;
}

SdfFieldSignalConfig SignalConfigForSourceEntry(const ColorPipelineSourceStackEntry& entry) {
    SdfFieldSignalConfig config{};
    if (entry.signal == ColorSignal::sdf_boundary_band) {
        config.boundary_band_px = EscapeTimeColorClamp(entry.params.sdf_boundary_width_px, 0.25f, 16.0f);
    }
    return config;
}

PlannedSdfSourceRow PlannedRowForSourceEntry(const ColorPipelineSourceStackEntry& entry) {
    PlannedSdfSourceRow row{};
    row.signal = entry.signal;
    row.kind = SignalKindForColorSignal(entry.signal);
    row.config = SignalConfigForSourceEntry(entry);
    row.gate = entry.params.sdf_gate;
    row.gate_width_px = EscapeTimeColorClamp(entry.params.sdf_gate_width_px, 0.25f, 16.0f);
    row.lens_field_v2_sign_contrast = EscapeTimeColorClamp(entry.params.lens_field_v2_sign_contrast, 0.0f, 1.0f);
    row.scale = entry.params.scale;
    row.bias = entry.params.bias;
    row.blend_weight = EscapeTimeColorClamp(entry.params.blend_weight, 0.0f, 1.0f);
    row.sample_step = NormalizeSdfSourceSampleStep(entry.params.sdf_sample_step);
    row.needs_neighborhood = SdfSignalNeedsNeighborhood(entry.signal);
    return row;
}

bool AppendPlannedSourceRow(SdfPostprocessExecutionPlan& plan, const ColorPipelineSourceStackEntry& entry) {
    if (plan.row_count < 0 || plan.row_count >= kColorPipelineMaxSourceStackCount) {
        return false;
    }
    const PlannedSdfSourceRow row = PlannedRowForSourceEntry(entry);
    plan.rows[plan.row_count] = row;
    plan.use_direct_samples = plan.use_direct_samples && !row.needs_neighborhood;
    plan.uses_row_sample_steps = plan.uses_row_sample_steps || row.sample_step > 1;
    ++plan.row_count;
    return true;
}

bool BuildSdfPostprocessExecutionPlan(
    const KernelParams& params,
    int outputPixelStep,
    SdfPostprocessExecutionPlan& outPlan,
    std::string* outError) {
    outPlan = {};
    outPlan.output_pixel_step = outputPixelStep;
    outPlan.use_direct_samples = true;
    outPlan.worker_count = 1;

    const int sourceStackCount = ClampColorPipelineSourceStackCount(params.color_source_stack_count);
    if (sourceStackCount <= 0) {
        if (!IsColorPipelineSdfSourceSignal(params.color_pipeline.signal)) {
            if (outError) *outError = "SDF Color Pipeline postprocess was requested without an SDF source signal";
            return false;
        }
        ColorPipelineSourceStackEntry entry;
        entry.signal = params.color_pipeline.signal;
        return AppendPlannedSourceRow(outPlan, entry);
    }

    bool hasSdf = false;
    bool hasNonSdf = false;
    for (int index = 0; index < sourceStackCount; ++index) {
        const bool isSdf = IsColorPipelineSdfSourceSignal(params.color_source_stack[index].signal);
        hasSdf = hasSdf || isSdf;
        hasNonSdf = hasNonSdf || !isSdf;
    }
    if (hasSdf && hasNonSdf) {
        if (outError) *outError = "SDF Color Pipeline source stacks cannot mix SDF and non-SDF Source rows";
        return false;
    }
    if (!hasSdf) {
        if (outError) *outError = "SDF Color Pipeline postprocess was requested without an SDF source signal";
        return false;
    }

    for (int index = 0; index < sourceStackCount; ++index) {
        if (!AppendPlannedSourceRow(outPlan, params.color_source_stack[index])) {
            if (outError) *outError = "SDF Color Pipeline source stack exceeds supported row count";
            return false;
        }
    }
    return outPlan.row_count > 0;
}

float ApplySdfGateToSourceValue(
    float value,
    float signedDistancePx,
    const ColorPipelineSourceStackEntry& entry) {
    if (entry.params.sdf_gate != ColorPipelineSdfGateMode::boundary_band) {
        return value;
    }
    SdfFieldSignalConfig gateConfig{};
    gateConfig.boundary_band_px = EscapeTimeColorClamp(entry.params.sdf_gate_width_px, 0.25f, 16.0f);
    const float mask = ResolveSdfBoundaryBandFromSignedDistancePx(signedDistancePx, gateConfig);
    return value * EscapeTimeColorClamp(mask, 0.0f, 1.0f);
}

float ApplyPlannedSdfGateToSourceValue(
    float value,
    float signedDistancePx,
    const PlannedSdfSourceRow& row) {
    if (row.gate != ColorPipelineSdfGateMode::boundary_band) {
        return value;
    }
    SdfFieldSignalConfig gateConfig{};
    gateConfig.boundary_band_px = row.gate_width_px;
    const float mask = ResolveSdfBoundaryBandFromSignedDistancePx(signedDistancePx, gateConfig);
    return value * EscapeTimeColorClamp(mask, 0.0f, 1.0f);
}

float ResolveSdfSourceValueFromSample(
    const SdfFieldSignalSample& sample,
    float fieldPixelScale,
    const ColorPipelineSourceStackEntry& entry) {
    float value = ResolveSdfFieldSignalValue(sample, SignalKindForColorSignal(entry.signal));
    if (entry.signal == ColorSignal::lens_field_v2_distance) {
        value = ResolveLensFieldV2ResponseFromSignedDistancePx(
            sample.signed_distance_px,
            fieldPixelScale,
            entry.params.lens_field_v2_sign_contrast);
    }
    if (entry.signal == ColorSignal::sdf_normal_angle) {
        value = (value + kPi) / kTwoPi;
    }
    value = value * entry.params.scale + entry.params.bias;
    return ApplySdfGateToSourceValue(value, sample.signed_distance_px, entry);
}

float ResolvePlannedSdfSourceValueFromSample(
    const SdfFieldSignalSample& sample,
    float fieldPixelScale,
    const PlannedSdfSourceRow& row) {
    float value = ResolveSdfFieldSignalValue(sample, row.kind);
    if (row.signal == ColorSignal::lens_field_v2_distance) {
        value = ResolveLensFieldV2ResponseFromSignedDistancePx(
            sample.signed_distance_px,
            fieldPixelScale,
            row.lens_field_v2_sign_contrast);
    }
    if (row.signal == ColorSignal::sdf_normal_angle) {
        value = (value + kPi) / kTwoPi;
    }
    value = value * row.scale + row.bias;
    return ApplyPlannedSdfGateToSourceValue(value, sample.signed_distance_px, row);
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

bool PlannedRowNeedsDistinctSignalSample(
    const PlannedSdfSourceRow& row,
    const SdfFieldSignalConfig& cachedConfig) {
    if (row.signal != ColorSignal::sdf_boundary_band) {
        return false;
    }
    return !BoundaryBandConfigMatches(SignalConfigForPlannedRow(row), cachedConfig);
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
    *outValue = ResolveSdfSourceValueFromSample(sample, field.pixel_scale, entry);
    return true;
}

bool ResolveDirectSdfSourceValueFromCenter(
    float center,
    float fieldPixelScale,
    const ColorPipelineSourceStackEntry& entry,
    float* outValue) {
    if (!outValue || SdfSignalNeedsNeighborhood(entry.signal)) {
        return false;
    }
    float value = center;
    if (entry.signal == ColorSignal::lens_field_v2_distance) {
        value = ResolveLensFieldV2ResponseFromSignedDistancePx(
            center,
            fieldPixelScale,
            entry.params.lens_field_v2_sign_contrast);
    } else if (entry.signal == ColorSignal::sdf_inside_outside) {
        value = center < 0.0f ? 1.0f : 0.0f;
    } else if (entry.signal == ColorSignal::sdf_boundary_band) {
        value = ResolveSdfBoundaryBandFromSignedDistancePx(center, SignalConfigForSourceEntry(entry));
    } else if (entry.signal != ColorSignal::sdf_signed_distance) {
        return false;
    }
    value = value * entry.params.scale + entry.params.bias;
    *outValue = ApplySdfGateToSourceValue(value, center, entry);
    return true;
}

bool ResolveDirectPlannedSdfSourceValueFromCenter(
    float center,
    float fieldPixelScale,
    const PlannedSdfSourceRow& row,
    float* outValue) {
    if (!outValue || row.needs_neighborhood) {
        return false;
    }
    float value = center;
    if (row.signal == ColorSignal::lens_field_v2_distance) {
        value = ResolveLensFieldV2ResponseFromSignedDistancePx(
            center,
            fieldPixelScale,
            row.lens_field_v2_sign_contrast);
    } else if (row.signal == ColorSignal::sdf_inside_outside) {
        value = center < 0.0f ? 1.0f : 0.0f;
    } else if (row.signal == ColorSignal::sdf_boundary_band) {
        value = ResolveSdfBoundaryBandFromSignedDistancePx(center, SignalConfigForPlannedRow(row));
    } else if (row.signal != ColorSignal::sdf_signed_distance) {
        return false;
    }
    value = value * row.scale + row.bias;
    *outValue = ApplyPlannedSdfGateToSourceValue(value, center, row);
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
        if (!ResolveDirectSdfSourceValueFromCenter(center, field.pixel_scale, FlatSdfSourceEntry(params), outSignal)) {
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
    if (!ResolveDirectSdfSourceValueFromCenter(center, field.pixel_scale, first, &signal)) {
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
        if (!ResolveDirectSdfSourceValueFromCenter(center, field.pixel_scale, entry, &nextSignal)) {
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

bool ResolveDirectPlannedSdfPipelineSignal(
    const SdfFieldView& field,
    int fx,
    int fy,
    const SdfPostprocessExecutionPlan& plan,
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
    if (plan.row_count <= 0) {
        if (outError) *outError = "SDF Color Pipeline postprocess was requested without an SDF source signal";
        return false;
    }

    float signal = 0.0f;
    if (!ResolveDirectPlannedSdfSourceValueFromCenter(center, field.pixel_scale, plan.rows[0], &signal)) {
        if (outError) *outError = "SDF Color Pipeline postprocess needs neighborhood sampling for this SDF source";
        return false;
    }
    for (int index = 1; index < plan.row_count; ++index) {
        float nextSignal = 0.0f;
        if (!ResolveDirectPlannedSdfSourceValueFromCenter(center, field.pixel_scale, plan.rows[index], &nextSignal)) {
            if (outError) *outError = "SDF Color Pipeline postprocess needs neighborhood sampling for this SDF source";
            return false;
        }
        signal = EscapeTimeColorLerp(signal, nextSignal, plan.rows[index].blend_weight);
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
    float signal = ResolveSdfSourceValueFromSample(cachedSample, field.pixel_scale, first);
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
        const float nextSignal = ResolveSdfSourceValueFromSample(*sample, field.pixel_scale, entry);
        signal = EscapeTimeColorLerp(
            signal,
            nextSignal,
            EscapeTimeColorClamp(entry.params.blend_weight, 0.0f, 1.0f));
    }
    *outSignal = signal;
    return true;
}

bool ResolvePlannedSdfPipelineSignal(
    const SdfFieldView& field,
    int fx,
    int fy,
    const SdfPostprocessExecutionPlan& plan,
    float* outSignal,
    std::string* outError) {
    if (!outSignal) {
        if (outError) *outError = "SDF Color Pipeline postprocess requires output storage";
        return false;
    }
    if (plan.row_count <= 0) {
        if (outError) *outError = "SDF Color Pipeline postprocess was requested without an SDF source signal";
        return false;
    }

    const SdfFieldSignalConfig cachedConfig = SignalConfigForPlannedRow(plan.rows[0]);
    SdfFieldSignalSample cachedSample;
    if (!SampleSdfFieldSignals(field, fx, fy, cachedConfig, cachedSample)) {
        if (outError) *outError = "SDF Color Pipeline postprocess could not sample the Lens SDF field";
        return false;
    }
    float signal = ResolvePlannedSdfSourceValueFromSample(cachedSample, field.pixel_scale, plan.rows[0]);
    for (int index = 1; index < plan.row_count; ++index) {
        const PlannedSdfSourceRow& row = plan.rows[index];
        const SdfFieldSignalSample* sample = &cachedSample;
        SdfFieldSignalSample distinctSample;
        if (PlannedRowNeedsDistinctSignalSample(row, cachedConfig)) {
            if (!SampleSdfFieldSignals(field, fx, fy, SignalConfigForPlannedRow(row), distinctSample)) {
                if (outError) *outError = "SDF Color Pipeline postprocess could not sample the Lens SDF field";
                return false;
            }
            sample = &distinctSample;
        }
        const float nextSignal = ResolvePlannedSdfSourceValueFromSample(*sample, field.pixel_scale, row);
        signal = EscapeTimeColorLerp(signal, nextSignal, row.blend_weight);
    }
    *outSignal = signal;
    return true;
}

bool ResolvePlannedSdfSourceRowValue(
    const SdfFieldView& field,
    int fx,
    int fy,
    const PlannedSdfSourceRow& row,
    float* outValue,
    std::string* outError,
    SdfColorPipelinePostprocessStats* outStats) {
    if (!outValue) {
        if (outError) *outError = "SDF Color Pipeline postprocess requires output storage";
        return false;
    }
    if (row.needs_neighborhood) {
        SdfFieldSignalSample sample;
        if (!SampleSdfFieldSignals(field, fx, fy, SignalConfigForPlannedRow(row), sample)) {
            if (outError) *outError = "SDF Color Pipeline postprocess could not sample the Lens SDF field";
            return false;
        }
        *outValue = ResolvePlannedSdfSourceValueFromSample(sample, field.pixel_scale, row);
        if (outStats) {
            ++outStats->source_neighborhood_sample_count;
        }
        return true;
    }

    float center = 0.0f;
    if (!SampleSdfCenterValue(field, fx, fy, &center) ||
        !ResolveDirectPlannedSdfSourceValueFromCenter(center, field.pixel_scale, row, outValue)) {
        if (outError) *outError = "SDF Color Pipeline postprocess could not sample the Lens SDF field";
        return false;
    }
    if (outStats) {
        ++outStats->source_direct_sample_count;
    }
    return true;
}

bool ResolveCachedPlannedSdfSourceRowValue(
    const SdfFieldView& field,
    int fx,
    int fy,
    const SdfPostprocessExecutionPlan& plan,
    int rowIndex,
    SdfPostprocessResolveScratch* scratch,
    float* outValue,
    std::string* outError,
    SdfColorPipelinePostprocessStats* outStats) {
    if (rowIndex < 0 || rowIndex >= plan.row_count) {
        if (outError) *outError = "SDF Color Pipeline postprocess row index is invalid";
        return false;
    }
    if (!scratch) {
        return ResolvePlannedSdfSourceRowValue(field, fx, fy, plan.rows[rowIndex], outValue, outError, outStats);
    }

    const PlannedSdfSourceRow& row = plan.rows[rowIndex];
    const int step = std::max(1, row.sample_step);
    const int qfx = QuantizeSdfSampleCoordinate(fx, field.width, step);
    const int qfy = QuantizeSdfSampleCoordinate(fy, field.height, step);
    if (step <= 1) {
        return ResolvePlannedSdfSourceRowValue(field, qfx, qfy, row, outValue, outError, outStats);
    }

    SdfRowSampleCache& cache = scratch->row_caches[static_cast<std::size_t>(rowIndex)];
    const int blockCount = SdfSampleBlockCount(field.width, step);
    if (cache.cached_step != step || static_cast<int>(cache.values.size()) != blockCount) {
        cache.cached_step = step;
        cache.cached_qfy = -1;
        cache.values.assign(static_cast<std::size_t>(blockCount), 0.0f);
        cache.valid.assign(static_cast<std::size_t>(blockCount), 0);
    }
    if (cache.cached_qfy != qfy) {
        cache.cached_qfy = qfy;
        std::fill(cache.valid.begin(), cache.valid.end(), 0);
    }

    const int blockX = ClampIntLocal(fx / step, 0, std::max(0, blockCount - 1));
    const std::size_t blockIndex = static_cast<std::size_t>(blockX);
    if (cache.valid[blockIndex]) {
        if (outValue) {
            *outValue = cache.values[blockIndex];
        }
        return true;
    }

    float value = 0.0f;
    if (!ResolvePlannedSdfSourceRowValue(field, qfx, qfy, row, &value, outError, outStats)) {
        return false;
    }
    cache.values[blockIndex] = value;
    cache.valid[blockIndex] = 1;
    if (outValue) {
        *outValue = value;
    }
    return true;
}

bool ResolveCachedPlannedSdfPipelineSignal(
    const SdfFieldView& field,
    int fx,
    int fy,
    const SdfPostprocessExecutionPlan& plan,
    SdfPostprocessResolveScratch* scratch,
    float* outSignal,
    std::string* outError,
    SdfColorPipelinePostprocessStats* outStats) {
    if (!outSignal) {
        if (outError) *outError = "SDF Color Pipeline postprocess requires output storage";
        return false;
    }
    if (plan.row_count <= 0) {
        if (outError) *outError = "SDF Color Pipeline postprocess was requested without an SDF source signal";
        return false;
    }

    float signal = 0.0f;
    if (!ResolveCachedPlannedSdfSourceRowValue(field, fx, fy, plan, 0, scratch, &signal, outError, outStats)) {
        return false;
    }
    for (int index = 1; index < plan.row_count; ++index) {
        float nextSignal = 0.0f;
        if (!ResolveCachedPlannedSdfSourceRowValue(field, fx, fy, plan, index, scratch, &nextSignal, outError, outStats)) {
            return false;
        }
        signal = EscapeTimeColorLerp(signal, nextSignal, plan.rows[index].blend_weight);
    }
    *outSignal = signal;
    return true;
}

int ResolveSdfPostprocessWorkerCount(
    std::size_t sampleCount,
    int workUnitCount,
    const SdfColorPipelinePostprocessOptions* options) {
    if (sampleCount == 0 || workUnitCount <= 1) {
        return 1;
    }
    const int requested = options ? options->max_worker_threads : 0;
    if (requested == 1) {
        return 1;
    }
    if (requested > 1) {
        return std::max(1, std::min({requested, kMaxAutomaticSdfPostprocessWorkers, workUnitCount}));
    }
    if (sampleCount < kParallelPostprocessSampleThreshold) {
        return 1;
    }
    const unsigned int hardware = std::thread::hardware_concurrency();
    if (hardware <= 2) {
        return 1;
    }
    const int preferred = static_cast<int>(hardware / 2);
    return std::max(1, std::min({preferred, kMaxAutomaticSdfPostprocessWorkers, workUnitCount}));
}

void AccumulatePostprocessStats(
    SdfColorPipelinePostprocessStats& total,
    const SdfColorPipelinePostprocessStats& next) {
    total.direct_sample_count += next.direct_sample_count;
    total.neighborhood_sample_count += next.neighborhood_sample_count;
    total.source_direct_sample_count += next.source_direct_sample_count;
    total.source_neighborhood_sample_count += next.source_neighborhood_sample_count;
    total.filled_pixel_count += next.filled_pixel_count;
}

void StampBackendStats(
    SdfColorPipelinePostprocessStats* outStats,
    SdfColorPipelinePostprocessBackend backend,
    bool fallbackUsed) {
    if (!outStats) {
        return;
    }
    outStats->backend_used = backend;
    outStats->backend_fallback_used = fallbackUsed;
}

std::uint32_t PackRgba(Rgba8 color) {
    return static_cast<std::uint32_t>(color.x) |
        (static_cast<std::uint32_t>(color.y) << 8) |
        (static_cast<std::uint32_t>(color.z) << 16) |
        (static_cast<std::uint32_t>(color.w) << 24);
}

bool ResolveAndFillSdfPostprocessBlock(
    const SdfFieldView& field,
    int fx,
    int fy,
    const KernelParams& params,
    const SdfPostprocessExecutionPlan& plan,
    SdfPostprocessResolveScratch* scratch,
    std::uint32_t* ioRgba,
    int renderWidth,
    int xBegin,
    int xEnd,
    int yBegin,
    int yEnd,
    std::string* outError,
    SdfColorPipelinePostprocessStats* outStats) {
    if (xBegin >= xEnd || yBegin >= yEnd) {
        return true;
    }
    float signal = 0.0f;
    const bool resolved = plan.uses_row_sample_steps
        ? ResolveCachedPlannedSdfPipelineSignal(field, fx, fy, plan, scratch, &signal, outError, outStats)
        : plan.use_direct_samples
        ? ResolveDirectPlannedSdfPipelineSignal(field, fx, fy, plan, &signal, outError)
        : ResolvePlannedSdfPipelineSignal(field, fx, fy, plan, &signal, outError);
    if (!resolved) {
        return false;
    }
    if (outStats) {
        if (plan.use_direct_samples) {
            ++outStats->direct_sample_count;
        } else {
            ++outStats->neighborhood_sample_count;
        }
        if (!plan.uses_row_sample_steps) {
            for (int index = 0; index < plan.row_count; ++index) {
                if (plan.rows[index].needs_neighborhood) {
                    ++outStats->source_neighborhood_sample_count;
                } else {
                    ++outStats->source_direct_sample_count;
                }
            }
        }
    }
    const float shapedSignal = ApplyColorPipelineShapeValue(signal, params);
    Rgba8 color = SampleProgrammableEscapeTimePalette<Rgba8>(shapedSignal, true, params);
    color = ApplyFractalColorGrading(color, params);
    const std::uint32_t packed = PackRgba(color);
    for (int yy = yBegin; yy < yEnd; ++yy) {
        const std::size_t rowOffset = static_cast<std::size_t>(yy) * static_cast<std::size_t>(renderWidth);
        for (int xx = xBegin; xx < xEnd; ++xx) {
            ioRgba[rowOffset + static_cast<std::size_t>(xx)] = packed;
            if (outStats) {
                ++outStats->filled_pixel_count;
            }
        }
    }
    return true;
}

bool RunDownsampledFieldPostprocessRows(
    const SdfFieldView& field,
    const RenderSettings& render,
    const KernelParams& params,
    const SdfPostprocessExecutionPlan& plan,
    int fyBegin,
    int fyEnd,
    std::uint32_t* ioRgba,
    std::string* outError,
    SdfColorPipelinePostprocessStats* outStats) {
    const int width = render.resolution.x;
    const int height = render.resolution.y;
    SdfPostprocessResolveScratch scratch{};
    for (int fy = fyBegin; fy < fyEnd; ++fy) {
        const int yBegin = CeilDivNonNegativeLocal(
            static_cast<long long>(fy) * static_cast<long long>(height),
            field.height);
        const int yEnd = CeilDivNonNegativeLocal(
            static_cast<long long>(fy + 1) * static_cast<long long>(height),
            field.height);
        for (int fx = 0; fx < field.width; ++fx) {
            const int xBegin = CeilDivNonNegativeLocal(
                static_cast<long long>(fx) * static_cast<long long>(width),
                field.width);
            const int xEnd = CeilDivNonNegativeLocal(
                static_cast<long long>(fx + 1) * static_cast<long long>(width),
                field.width);
            if (!ResolveAndFillSdfPostprocessBlock(
                    field,
                    fx,
                    fy,
                    params,
                    plan,
                    &scratch,
                    ioRgba,
                    width,
                    xBegin,
                    xEnd,
                    yBegin,
                    yEnd,
                    outError,
                    outStats)) {
                return false;
            }
        }
    }
    return true;
}

bool RunRenderBlockPostprocessRows(
    const SdfFieldView& field,
    const RenderSettings& render,
    const KernelParams& params,
    const SdfPostprocessExecutionPlan& plan,
    int blockYBegin,
    int blockYEnd,
    std::uint32_t* ioRgba,
    std::string* outError,
    SdfColorPipelinePostprocessStats* outStats) {
    const int width = render.resolution.x;
    const int height = render.resolution.y;
    const int outputPixelStep = plan.output_pixel_step;
    SdfPostprocessResolveScratch scratch{};
    for (int blockY = blockYBegin; blockY < blockYEnd; ++blockY) {
        const int y = blockY * outputPixelStep;
        const int blockHeight = ClampIntLocal(height - y, 1, outputPixelStep);
        const int sampleY = ClampIntLocal(y + blockHeight / 2, 0, height - 1);
        const int fy = ClampIntLocal((static_cast<long long>(sampleY) * field.height) / height, 0, field.height - 1);
        for (int x = 0; x < width; x += outputPixelStep) {
            const int blockWidth = ClampIntLocal(width - x, 1, outputPixelStep);
            const int sampleX = ClampIntLocal(x + blockWidth / 2, 0, width - 1);
            const int fx = ClampIntLocal((static_cast<long long>(sampleX) * field.width) / width, 0, field.width - 1);
            if (!ResolveAndFillSdfPostprocessBlock(
                    field,
                    fx,
                    fy,
                    params,
                    plan,
                    &scratch,
                    ioRgba,
                    width,
                    x,
                    x + blockWidth,
                    y,
                    y + blockHeight,
                    outError,
                    outStats)) {
                return false;
            }
        }
    }
    return true;
}

template <typename WorkerFn>
bool RunPostprocessWorkers(
    int workUnitCount,
    const SdfPostprocessExecutionPlan& plan,
    WorkerFn workerFn,
    std::string* outError,
    SdfColorPipelinePostprocessStats* outStats) {
    SdfColorPipelinePostprocessStats combined{};
    combined.output_pixel_step = plan.output_pixel_step;
    combined.worker_count = plan.worker_count;
    if (plan.worker_count <= 1) {
        bool ok = workerFn(0, workUnitCount, combined, outError);
        if (outStats) {
            *outStats = combined;
        }
        return ok;
    }

    std::vector<std::thread> threads;
    std::vector<SdfColorPipelinePostprocessStats> workerStats(static_cast<std::size_t>(plan.worker_count));
    std::vector<std::string> workerErrors(static_cast<std::size_t>(plan.worker_count));
    std::vector<unsigned char> workerOk(static_cast<std::size_t>(plan.worker_count), 1);
    threads.reserve(static_cast<std::size_t>(plan.worker_count));
    for (int workerIndex = 0; workerIndex < plan.worker_count; ++workerIndex) {
        const int begin = (workUnitCount * workerIndex) / plan.worker_count;
        const int end = (workUnitCount * (workerIndex + 1)) / plan.worker_count;
        workerStats[static_cast<std::size_t>(workerIndex)].output_pixel_step = plan.output_pixel_step;
        workerStats[static_cast<std::size_t>(workerIndex)].worker_count = 1;
        threads.emplace_back([&, workerIndex, begin, end]() {
            workerOk[static_cast<std::size_t>(workerIndex)] = workerFn(
                begin,
                end,
                workerStats[static_cast<std::size_t>(workerIndex)],
                &workerErrors[static_cast<std::size_t>(workerIndex)]) ? 1 : 0;
        });
    }
    for (std::thread& thread : threads) {
        thread.join();
    }
    for (int workerIndex = 0; workerIndex < plan.worker_count; ++workerIndex) {
        const std::size_t index = static_cast<std::size_t>(workerIndex);
        if (workerOk[index] == 0) {
            if (outError) *outError = workerErrors[index];
            if (outStats) {
                *outStats = combined;
            }
            return false;
        }
        AccumulatePostprocessStats(combined, workerStats[index]);
    }
    if (outStats) {
        *outStats = combined;
    }
    return true;
}

} // namespace

const char* SdfColorPipelinePostprocessBackendId(SdfColorPipelinePostprocessBackend backend) {
    switch (backend) {
    case SdfColorPipelinePostprocessBackend::cpu:
        return "cpu";
    case SdfColorPipelinePostprocessBackend::cuda_direct_scalar:
        return "cuda_direct_scalar";
    case SdfColorPipelinePostprocessBackend::cuda_field_signal:
        return "cuda_field_signal";
    case SdfColorPipelinePostprocessBackend::auto_backend:
        return "auto";
    }
    return "unknown";
}

void RegisterSdfColorPipelineCudaDirectScalarBackend(SdfColorPipelinePostprocessBackendFn backendFn) {
    RegisteredCudaDirectScalarBackend() = backendFn;
}

void RegisterSdfColorPipelineCudaFieldSignalBackend(SdfColorPipelinePostprocessBackendFn backendFn) {
    RegisteredCudaFieldSignalBackend() = backendFn;
}

bool IsColorPipelineSdfSourceSignal(ColorSignal signal) {
    return signal == ColorSignal::sdf_signed_distance ||
        signal == ColorSignal::sdf_inside_outside ||
        signal == ColorSignal::sdf_boundary_band ||
        signal == ColorSignal::sdf_normal_angle ||
        signal == ColorSignal::sdf_curvature ||
        signal == ColorSignal::lens_field_v2_distance;
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

bool ColorPipelineSdfPostprocessCanUseCudaDirectScalar(const KernelParams& params) {
    if (!ColorPipelineSdfPostprocessCanUseDirectSamples(params)) {
        return false;
    }
    const int sourceStackCount = ClampColorPipelineSourceStackCount(params.color_source_stack_count);
    for (int index = 0; index < sourceStackCount; ++index) {
        if (NormalizeSdfSourceSampleStep(params.color_source_stack[index].params.sdf_sample_step) > 1) {
            return false;
        }
    }
    return true;
}

bool ColorPipelineSdfPostprocessCanUseCudaFieldSignal(const KernelParams& params) {
    if (!ColorPipelineSourceStackIsSdfOnly(params)) {
        return false;
    }
    const int sourceStackCount = ClampColorPipelineSourceStackCount(params.color_source_stack_count);
    for (int index = 0; index < sourceStackCount; ++index) {
        if (NormalizeSdfSourceSampleStep(params.color_source_stack[index].params.sdf_sample_step) > 1) {
            return false;
        }
    }
    return true;
}

int ResolveSdfColorPipelinePostprocessOutputPixelStep(
    const KernelParams& params,
    bool previewActive,
    double previewScale,
    bool forceFullQuality) {
    if (forceFullQuality || !previewActive) {
        return 1;
    }
    if (ColorPipelineSdfPostprocessCanUseDirectSamples(params)) {
        return 1;
    }
    if (previewScale <= 0.35) {
        return 4;
    }
    return 2;
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
    const SdfColorPipelinePostprocessBackend requestedBackend = options
        ? options->backend_preference
        : SdfColorPipelinePostprocessBackend::auto_backend;
    bool cpuFallbackUsed = false;
    if (requestedBackend == SdfColorPipelinePostprocessBackend::cuda_direct_scalar ||
        requestedBackend == SdfColorPipelinePostprocessBackend::auto_backend) {
        if (ColorPipelineSdfPostprocessCanUseCudaDirectScalar(params)) {
            SdfColorPipelinePostprocessBackendFn cudaBackend = RegisteredCudaDirectScalarBackend();
            if (cudaBackend) {
                SdfColorPipelinePostprocessStats cudaStats{};
                std::string cudaError;
                if (cudaBackend(field, render, params, ioRgba, &cudaError, &cudaStats, options)) {
                    if (outStats) {
                        *outStats = cudaStats;
                    }
                    return true;
                }
                if (requestedBackend == SdfColorPipelinePostprocessBackend::cuda_direct_scalar) {
                    if (outError) *outError = cudaError.empty()
                        ? "CUDA direct scalar SDF Color Pipeline postprocess failed"
                        : cudaError;
                    return false;
                }
                cpuFallbackUsed = true;
                if (outError) {
                    outError->clear();
                }
            } else if (requestedBackend == SdfColorPipelinePostprocessBackend::cuda_direct_scalar) {
                if (outError) *outError = "CUDA direct scalar SDF Color Pipeline postprocess backend is not registered";
                return false;
            } else {
                cpuFallbackUsed = true;
            }
        } else if (requestedBackend == SdfColorPipelinePostprocessBackend::cuda_direct_scalar) {
            if (outError) *outError = "CUDA direct scalar SDF Color Pipeline postprocess does not support this Source stack";
            return false;
        }
    }
    if (requestedBackend == SdfColorPipelinePostprocessBackend::cuda_field_signal ||
        requestedBackend == SdfColorPipelinePostprocessBackend::auto_backend) {
        if (ColorPipelineSdfPostprocessCanUseCudaFieldSignal(params) &&
            !ColorPipelineSdfPostprocessCanUseCudaDirectScalar(params)) {
            SdfColorPipelinePostprocessBackendFn cudaBackend = RegisteredCudaFieldSignalBackend();
            if (cudaBackend) {
                SdfColorPipelinePostprocessStats cudaStats{};
                std::string cudaError;
                if (cudaBackend(field, render, params, ioRgba, &cudaError, &cudaStats, options)) {
                    if (outStats) {
                        *outStats = cudaStats;
                    }
                    return true;
                }
                if (requestedBackend == SdfColorPipelinePostprocessBackend::cuda_field_signal) {
                    if (outError) *outError = cudaError.empty()
                        ? "CUDA field-signal SDF Color Pipeline postprocess failed"
                        : cudaError;
                    return false;
                }
                cpuFallbackUsed = true;
                if (outError) {
                    outError->clear();
                }
            } else if (requestedBackend == SdfColorPipelinePostprocessBackend::cuda_field_signal) {
                if (outError) *outError = "CUDA field-signal SDF Color Pipeline postprocess backend is not registered";
                return false;
            } else {
                cpuFallbackUsed = true;
            }
        } else if (requestedBackend == SdfColorPipelinePostprocessBackend::cuda_field_signal) {
            if (outError) *outError = "CUDA field-signal SDF Color Pipeline postprocess does not support this Source stack";
            return false;
        } else if (!ColorPipelineSdfPostprocessCanUseCudaDirectScalar(params)) {
            cpuFallbackUsed = true;
        }
    }
    SdfPostprocessExecutionPlan plan;
    const int outputPixelStep = NormalizePostprocessPixelStep(options ? options->output_pixel_step : 1);
    if (!BuildSdfPostprocessExecutionPlan(params, outputPixelStep, plan, outError)) {
        return false;
    }

    const int width = render.resolution.x;
    const int height = render.resolution.y;
    if (outputPixelStep == 1 &&
        field.width <= width &&
        field.height <= height &&
        (field.width < width || field.height < height)) {
        const std::size_t sampleCount = static_cast<std::size_t>(field.width) * static_cast<std::size_t>(field.height);
        plan.worker_count = ResolveSdfPostprocessWorkerCount(sampleCount, field.height, options);
        const bool ok = RunPostprocessWorkers(
            field.height,
            plan,
            [&](int begin, int end, SdfColorPipelinePostprocessStats& localStats, std::string* localError) {
                return RunDownsampledFieldPostprocessRows(
                    field,
                    render,
                    params,
                    plan,
                    begin,
                    end,
                    ioRgba,
                    localError,
                    &localStats);
            },
            outError,
            outStats);
        StampBackendStats(outStats, SdfColorPipelinePostprocessBackend::cpu, cpuFallbackUsed);
        return ok;
    }

    const int blockRows = CeilDivNonNegativeLocal(height, outputPixelStep);
    const int blockCols = CeilDivNonNegativeLocal(width, outputPixelStep);
    const std::size_t sampleCount = static_cast<std::size_t>(blockRows) * static_cast<std::size_t>(blockCols);
    plan.worker_count = ResolveSdfPostprocessWorkerCount(sampleCount, blockRows, options);
    const bool ok = RunPostprocessWorkers(
        blockRows,
        plan,
        [&](int begin, int end, SdfColorPipelinePostprocessStats& localStats, std::string* localError) {
            return RunRenderBlockPostprocessRows(
                field,
                render,
                params,
                plan,
                begin,
                end,
                ioRgba,
                localError,
                &localStats);
        },
        outError,
        outStats);
    StampBackendStats(outStats, SdfColorPipelinePostprocessBackend::cpu, cpuFallbackUsed);
    return ok;
}
