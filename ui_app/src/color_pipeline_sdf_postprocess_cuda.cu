#include "color_pipeline_sdf_postprocess_cuda.h"

#include "escape_time_coloring.h"

#include <cuda_runtime.h>

#include <algorithm>
#include <cstddef>

namespace {

constexpr int kThreadsPerBlock = 256;
constexpr float kPi = 3.14159265358979323846f;
constexpr float kTwoPi = 2.0f * kPi;

struct Rgba8 {
    unsigned char x;
    unsigned char y;
    unsigned char z;
    unsigned char w;
};

int CeilDivHost(long long numerator, int denominator) {
    if (denominator <= 0 || numerator <= 0) {
        return 0;
    }
    return static_cast<int>((numerator + static_cast<long long>(denominator) - 1) /
        static_cast<long long>(denominator));
}

int NormalizePostprocessPixelStepHost(int value) {
    if (value <= 1) return 1;
    if (value <= 2) return 2;
    return 4;
}

std::size_t RenderPixelCount(const RenderSettings& render) {
    if (render.resolution.x <= 0 || render.resolution.y <= 0) {
        return 0;
    }
    return static_cast<std::size_t>(render.resolution.x) * static_cast<std::size_t>(render.resolution.y);
}

std::size_t FieldPixelCount(const SdfFieldView& field) {
    if (field.width <= 0 || field.height <= 0) {
        return 0;
    }
    return static_cast<std::size_t>(field.width) * static_cast<std::size_t>(field.height);
}

__device__ int ClampIntDevice(int value, int lo, int hi) {
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

__device__ int CeilDivDevice(long long numerator, int denominator) {
    if (denominator <= 0 || numerator <= 0) {
        return 0;
    }
    return static_cast<int>((numerator + static_cast<long long>(denominator) - 1) /
        static_cast<long long>(denominator));
}

__device__ float ResolveBoundaryBandDevice(float signedDistancePx, float boundaryWidthPx) {
    const float safeBandPx = isfinite(boundaryWidthPx) && boundaryWidthPx > 0.000001f
        ? boundaryWidthPx
        : 0.000001f;
    float boundaryBand = 1.0f - (fabsf(signedDistancePx) / safeBandPx);
    if (boundaryBand < 0.0f) boundaryBand = 0.0f;
    if (boundaryBand > 1.0f) boundaryBand = 1.0f;
    return boundaryBand;
}

__device__ float ResolveLensFieldV2ResponseDevice(float signedDistancePx, float fieldPixelScale) {
    const float safeScale = isfinite(fieldPixelScale) && fieldPixelScale > 0.000001f ? fieldPixelScale : 1.0f;
    const float fullResolutionSignedPx = isfinite(signedDistancePx) ? signedDistancePx * safeScale : 0.0f;
    float response = 0.5f + (fullResolutionSignedPx / (2.0f * 48.0f));
    if (response < 0.0f) response = 0.0f;
    if (response > 1.0f) response = 1.0f;
    return response;
}

__device__ bool ResolveDirectSdfSourceValueDevice(
    float center,
    float fieldPixelScale,
    const ColorPipelineSourceStackEntry& entry,
    float* outValue) {
    if (!outValue || !isfinite(center)) {
        return false;
    }

    float value = center;
    if (entry.signal == ColorSignal::lens_field_v2_distance) {
        value = ResolveLensFieldV2ResponseDevice(center, fieldPixelScale);
    } else if (entry.signal == ColorSignal::sdf_inside_outside) {
        value = center < 0.0f ? 1.0f : 0.0f;
    } else if (entry.signal == ColorSignal::sdf_boundary_band) {
        value = ResolveBoundaryBandDevice(
            center,
            EscapeTimeColorClamp(entry.params.sdf_boundary_width_px, 0.25f, 16.0f));
    } else if (entry.signal != ColorSignal::sdf_signed_distance) {
        return false;
    }

    value = value * entry.params.scale + entry.params.bias;
    if (entry.params.sdf_gate == ColorPipelineSdfGateMode::boundary_band) {
        const float mask = ResolveBoundaryBandDevice(
            center,
            EscapeTimeColorClamp(entry.params.sdf_gate_width_px, 0.25f, 16.0f));
        value *= EscapeTimeColorClamp(mask, 0.0f, 1.0f);
    }
    *outValue = value;
    return true;
}

__device__ ColorPipelineSourceStackEntry FlatSdfSourceEntryDevice(const KernelParams& params) {
    ColorPipelineSourceStackEntry entry;
    entry.signal = params.color_pipeline.signal;
    return entry;
}

__device__ bool ResolveDirectSdfPipelineSignalDevice(
    const float* field,
    int fieldWidth,
    int fieldHeight,
    int fx,
    int fy,
    float fieldPixelScale,
    const KernelParams& params,
    float* outSignal) {
    if (!field || !outSignal || fx < 0 || fy < 0 || fx >= fieldWidth || fy >= fieldHeight) {
        return false;
    }
    const float center = field[fy * fieldWidth + fx];
    const int sourceStackCount = ClampColorPipelineSourceStackCount(params.color_source_stack_count);
    if (sourceStackCount <= 0) {
        return ResolveDirectSdfSourceValueDevice(center, fieldPixelScale, FlatSdfSourceEntryDevice(params), outSignal);
    }

    float signal = 0.0f;
    if (!ResolveDirectSdfSourceValueDevice(center, fieldPixelScale, params.color_source_stack[0], &signal)) {
        return false;
    }
    for (int index = 1; index < sourceStackCount; ++index) {
        float nextSignal = 0.0f;
        if (!ResolveDirectSdfSourceValueDevice(center, fieldPixelScale, params.color_source_stack[index], &nextSignal)) {
            return false;
        }
        signal = EscapeTimeColorLerp(
            signal,
            nextSignal,
            EscapeTimeColorClamp(params.color_source_stack[index].params.blend_weight, 0.0f, 1.0f));
    }
    *outSignal = signal;
    return true;
}

__device__ bool SampleRawFieldDevice(
    const float* field,
    int fieldWidth,
    int fieldHeight,
    int fx,
    int fy,
    float* outValue) {
    if (!field || !outValue || fx < 0 || fy < 0 || fx >= fieldWidth || fy >= fieldHeight) {
        return false;
    }
    const float value = field[fy * fieldWidth + fx];
    if (!isfinite(value)) {
        return false;
    }
    *outValue = value;
    return true;
}

__device__ bool SampleRawFieldClampedDevice(
    const float* field,
    int fieldWidth,
    int fieldHeight,
    int fx,
    int fy,
    float* outValue) {
    return SampleRawFieldDevice(
        field,
        fieldWidth,
        fieldHeight,
        ClampIntDevice(fx, 0, fieldWidth - 1),
        ClampIntDevice(fy, 0, fieldHeight - 1),
        outValue);
}

struct SdfFieldSignalSampleDevice {
    float center{0.0f};
    float left{0.0f};
    float right{0.0f};
    float up{0.0f};
    float down{0.0f};
};

__device__ bool SampleFieldSignalDevice(
    const float* field,
    int fieldWidth,
    int fieldHeight,
    int fx,
    int fy,
    SdfFieldSignalSampleDevice* outSample) {
    if (!outSample ||
        !SampleRawFieldDevice(field, fieldWidth, fieldHeight, fx, fy, &outSample->center) ||
        !SampleRawFieldClampedDevice(field, fieldWidth, fieldHeight, fx - 1, fy, &outSample->left) ||
        !SampleRawFieldClampedDevice(field, fieldWidth, fieldHeight, fx + 1, fy, &outSample->right) ||
        !SampleRawFieldClampedDevice(field, fieldWidth, fieldHeight, fx, fy - 1, &outSample->up) ||
        !SampleRawFieldClampedDevice(field, fieldWidth, fieldHeight, fx, fy + 1, &outSample->down)) {
        return false;
    }
    return true;
}

__device__ bool ResolveFieldSdfSourceValueDevice(
    const SdfFieldSignalSampleDevice& sample,
    float fieldPixelScale,
    const ColorPipelineSourceStackEntry& entry,
    float* outValue) {
    if (!outValue) {
        return false;
    }

    float value = sample.center;
    if (entry.signal == ColorSignal::lens_field_v2_distance) {
        value = ResolveLensFieldV2ResponseDevice(sample.center, fieldPixelScale);
    } else if (entry.signal == ColorSignal::sdf_inside_outside) {
        value = sample.center < 0.0f ? 1.0f : 0.0f;
    } else if (entry.signal == ColorSignal::sdf_boundary_band) {
        value = ResolveBoundaryBandDevice(
            sample.center,
            EscapeTimeColorClamp(entry.params.sdf_boundary_width_px, 0.25f, 16.0f));
    } else if (entry.signal == ColorSignal::sdf_normal_angle) {
        const float gradX = 0.5f * (sample.right - sample.left);
        const float gradY = 0.5f * (sample.down - sample.up);
        const float gradMag2 = gradX * gradX + gradY * gradY;
        const float angle = gradMag2 > 0.000000000001f ? atan2f(gradY, gradX) : 0.0f;
        value = (angle + kPi) / kTwoPi;
    } else if (entry.signal == ColorSignal::sdf_curvature) {
        value = sample.left + sample.right + sample.up + sample.down - (4.0f * sample.center);
    } else if (entry.signal != ColorSignal::sdf_signed_distance) {
        return false;
    }

    value = value * entry.params.scale + entry.params.bias;
    if (entry.params.sdf_gate == ColorPipelineSdfGateMode::boundary_band) {
        const float mask = ResolveBoundaryBandDevice(
            sample.center,
            EscapeTimeColorClamp(entry.params.sdf_gate_width_px, 0.25f, 16.0f));
        value *= EscapeTimeColorClamp(mask, 0.0f, 1.0f);
    }
    *outValue = value;
    return isfinite(value);
}

__device__ bool ResolveFieldSdfPipelineSignalDevice(
    const float* field,
    int fieldWidth,
    int fieldHeight,
    int fx,
    int fy,
    float fieldPixelScale,
    const KernelParams& params,
    float* outSignal) {
    if (!field || !outSignal || fx < 0 || fy < 0 || fx >= fieldWidth || fy >= fieldHeight) {
        return false;
    }

    SdfFieldSignalSampleDevice sample{};
    if (!SampleFieldSignalDevice(field, fieldWidth, fieldHeight, fx, fy, &sample)) {
        return false;
    }

    const int sourceStackCount = ClampColorPipelineSourceStackCount(params.color_source_stack_count);
    if (sourceStackCount <= 0) {
        return ResolveFieldSdfSourceValueDevice(sample, fieldPixelScale, FlatSdfSourceEntryDevice(params), outSignal);
    }

    float signal = 0.0f;
    if (!ResolveFieldSdfSourceValueDevice(sample, fieldPixelScale, params.color_source_stack[0], &signal)) {
        return false;
    }
    for (int index = 1; index < sourceStackCount; ++index) {
        float nextSignal = 0.0f;
        if (!ResolveFieldSdfSourceValueDevice(sample, fieldPixelScale, params.color_source_stack[index], &nextSignal)) {
            return false;
        }
        signal = EscapeTimeColorLerp(
            signal,
            nextSignal,
            EscapeTimeColorClamp(params.color_source_stack[index].params.blend_weight, 0.0f, 1.0f));
    }
    *outSignal = signal;
    return true;
}

__device__ std::uint32_t PackRgbaDevice(Rgba8 color) {
    return static_cast<std::uint32_t>(color.x) |
        (static_cast<std::uint32_t>(color.y) << 8) |
        (static_cast<std::uint32_t>(color.z) << 16) |
        (static_cast<std::uint32_t>(color.w) << 24);
}

__device__ std::uint32_t ResolvePackedColorDevice(
    const float* field,
    int fieldWidth,
    int fieldHeight,
    int fx,
    int fy,
    float fieldPixelScale,
    const KernelParams& params) {
    float signal = 0.0f;
    if (!ResolveDirectSdfPipelineSignalDevice(field, fieldWidth, fieldHeight, fx, fy, fieldPixelScale, params, &signal)) {
        return 0xff000000u;
    }
    const float shapedSignal = ApplyColorPipelineShapeValue(signal, params);
    Rgba8 color = SampleProgrammableEscapeTimePalette<Rgba8>(shapedSignal, true, params);
    color = ApplyFractalColorGrading(color, params);
    return PackRgbaDevice(color);
}

__device__ std::uint32_t ResolveFieldPackedColorDevice(
    const float* field,
    int fieldWidth,
    int fieldHeight,
    int fx,
    int fy,
    float fieldPixelScale,
    const KernelParams& params) {
    float signal = 0.0f;
    if (!ResolveFieldSdfPipelineSignalDevice(field, fieldWidth, fieldHeight, fx, fy, fieldPixelScale, params, &signal)) {
        return 0xff000000u;
    }
    const float shapedSignal = ApplyColorPipelineShapeValue(signal, params);
    Rgba8 color = SampleProgrammableEscapeTimePalette<Rgba8>(shapedSignal, true, params);
    color = ApplyFractalColorGrading(color, params);
    return PackRgbaDevice(color);
}

__global__ void SdfDirectScalarFieldCellKernel(
    const float* field,
    int fieldWidth,
    int fieldHeight,
    int renderWidth,
    int renderHeight,
    float fieldPixelScale,
    KernelParams params,
    std::uint32_t* outRgba) {
    const int fieldCount = fieldWidth * fieldHeight;
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index >= fieldCount) {
        return;
    }
    const int fx = index % fieldWidth;
    const int fy = index / fieldWidth;
    const int yBegin = CeilDivDevice(static_cast<long long>(fy) * static_cast<long long>(renderHeight), fieldHeight);
    const int yEnd = CeilDivDevice(static_cast<long long>(fy + 1) * static_cast<long long>(renderHeight), fieldHeight);
    const int xBegin = CeilDivDevice(static_cast<long long>(fx) * static_cast<long long>(renderWidth), fieldWidth);
    const int xEnd = CeilDivDevice(static_cast<long long>(fx + 1) * static_cast<long long>(renderWidth), fieldWidth);
    const std::uint32_t packed = ResolvePackedColorDevice(field, fieldWidth, fieldHeight, fx, fy, fieldPixelScale, params);
    for (int yy = yBegin; yy < yEnd; ++yy) {
        const int rowOffset = yy * renderWidth;
        for (int xx = xBegin; xx < xEnd; ++xx) {
            outRgba[rowOffset + xx] = packed;
        }
    }
}

__global__ void SdfDirectScalarRenderBlockKernel(
    const float* field,
    int fieldWidth,
    int fieldHeight,
    int renderWidth,
    int renderHeight,
    int outputPixelStep,
    int blockCols,
    int blockRows,
    float fieldPixelScale,
    KernelParams params,
    std::uint32_t* outRgba) {
    const int blockCount = blockCols * blockRows;
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index >= blockCount) {
        return;
    }
    const int blockX = index % blockCols;
    const int blockY = index / blockCols;
    const int xBegin = blockX * outputPixelStep;
    const int yBegin = blockY * outputPixelStep;
    const int blockWidth = ClampIntDevice(renderWidth - xBegin, 1, outputPixelStep);
    const int blockHeight = ClampIntDevice(renderHeight - yBegin, 1, outputPixelStep);
    const int sampleX = ClampIntDevice(xBegin + blockWidth / 2, 0, renderWidth - 1);
    const int sampleY = ClampIntDevice(yBegin + blockHeight / 2, 0, renderHeight - 1);
    const int fx = ClampIntDevice(
        static_cast<int>((static_cast<long long>(sampleX) * static_cast<long long>(fieldWidth)) / renderWidth),
        0,
        fieldWidth - 1);
    const int fy = ClampIntDevice(
        static_cast<int>((static_cast<long long>(sampleY) * static_cast<long long>(fieldHeight)) / renderHeight),
        0,
        fieldHeight - 1);
    const std::uint32_t packed = ResolvePackedColorDevice(field, fieldWidth, fieldHeight, fx, fy, fieldPixelScale, params);
    for (int yy = yBegin; yy < yBegin + blockHeight; ++yy) {
        const int rowOffset = yy * renderWidth;
        for (int xx = xBegin; xx < xBegin + blockWidth; ++xx) {
            outRgba[rowOffset + xx] = packed;
        }
    }
}

__global__ void SdfFieldSignalFieldCellKernel(
    const float* field,
    int fieldWidth,
    int fieldHeight,
    int renderWidth,
    int renderHeight,
    float fieldPixelScale,
    KernelParams params,
    std::uint32_t* outRgba) {
    const int fieldCount = fieldWidth * fieldHeight;
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index >= fieldCount) {
        return;
    }
    const int fx = index % fieldWidth;
    const int fy = index / fieldWidth;
    const int yBegin = CeilDivDevice(static_cast<long long>(fy) * static_cast<long long>(renderHeight), fieldHeight);
    const int yEnd = CeilDivDevice(static_cast<long long>(fy + 1) * static_cast<long long>(renderHeight), fieldHeight);
    const int xBegin = CeilDivDevice(static_cast<long long>(fx) * static_cast<long long>(renderWidth), fieldWidth);
    const int xEnd = CeilDivDevice(static_cast<long long>(fx + 1) * static_cast<long long>(renderWidth), fieldWidth);
    const std::uint32_t packed = ResolveFieldPackedColorDevice(field, fieldWidth, fieldHeight, fx, fy, fieldPixelScale, params);
    for (int yy = yBegin; yy < yEnd; ++yy) {
        const int rowOffset = yy * renderWidth;
        for (int xx = xBegin; xx < xEnd; ++xx) {
            outRgba[rowOffset + xx] = packed;
        }
    }
}

__global__ void SdfFieldSignalRenderBlockKernel(
    const float* field,
    int fieldWidth,
    int fieldHeight,
    int renderWidth,
    int renderHeight,
    int outputPixelStep,
    int blockCols,
    int blockRows,
    float fieldPixelScale,
    KernelParams params,
    std::uint32_t* outRgba) {
    const int blockCount = blockCols * blockRows;
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index >= blockCount) {
        return;
    }
    const int blockX = index % blockCols;
    const int blockY = index / blockCols;
    const int xBegin = blockX * outputPixelStep;
    const int yBegin = blockY * outputPixelStep;
    const int blockWidth = ClampIntDevice(renderWidth - xBegin, 1, outputPixelStep);
    const int blockHeight = ClampIntDevice(renderHeight - yBegin, 1, outputPixelStep);
    const int sampleX = ClampIntDevice(xBegin + blockWidth / 2, 0, renderWidth - 1);
    const int sampleY = ClampIntDevice(yBegin + blockHeight / 2, 0, renderHeight - 1);
    const int fx = ClampIntDevice(
        static_cast<int>((static_cast<long long>(sampleX) * static_cast<long long>(fieldWidth)) / renderWidth),
        0,
        fieldWidth - 1);
    const int fy = ClampIntDevice(
        static_cast<int>((static_cast<long long>(sampleY) * static_cast<long long>(fieldHeight)) / renderHeight),
        0,
        fieldHeight - 1);
    const std::uint32_t packed = ResolveFieldPackedColorDevice(field, fieldWidth, fieldHeight, fx, fy, fieldPixelScale, params);
    for (int yy = yBegin; yy < yBegin + blockHeight; ++yy) {
        const int rowOffset = yy * renderWidth;
        for (int xx = xBegin; xx < xBegin + blockWidth; ++xx) {
            outRgba[rowOffset + xx] = packed;
        }
    }
}

struct CudaPostprocessBuffers {
    float* field{nullptr};
    std::uint32_t* rgba{nullptr};

    bool Allocate(std::size_t fieldBytes, std::size_t rgbaBytes) {
        return cudaMalloc(&field, fieldBytes) == cudaSuccess &&
            cudaMalloc(&rgba, rgbaBytes) == cudaSuccess;
    }

    void Free() {
        cudaFree(field);
        cudaFree(rgba);
        field = nullptr;
        rgba = nullptr;
    }

    ~CudaPostprocessBuffers() {
        Free();
    }
};

void FillCudaStats(
    SdfColorPipelinePostprocessStats* outStats,
    const KernelParams& params,
    int outputPixelStep,
    std::size_t sampleCount,
    std::size_t filledPixelCount) {
    if (!outStats) {
        return;
    }
    const int rowCount = std::max(1, ClampColorPipelineSourceStackCount(params.color_source_stack_count));
    outStats->direct_sample_count = sampleCount;
    outStats->neighborhood_sample_count = 0;
    outStats->source_direct_sample_count = sampleCount * static_cast<std::size_t>(rowCount);
    outStats->source_neighborhood_sample_count = 0;
    outStats->filled_pixel_count = filledPixelCount;
    outStats->output_pixel_step = outputPixelStep;
    outStats->worker_count = 1;
    outStats->backend_used = SdfColorPipelinePostprocessBackend::cuda_direct_scalar;
    outStats->backend_fallback_used = false;
}

bool SignalNeedsNeighborhoodHost(ColorSignal signal) {
    return signal == ColorSignal::sdf_normal_angle || signal == ColorSignal::sdf_curvature;
}

void CountSourceRowsForStats(const KernelParams& params, int* outDirectRows, int* outNeighborhoodRows) {
    int directRows = 0;
    int neighborhoodRows = 0;
    const int rowCount = ClampColorPipelineSourceStackCount(params.color_source_stack_count);
    if (rowCount <= 0) {
        if (SignalNeedsNeighborhoodHost(params.color_pipeline.signal)) {
            neighborhoodRows = 1;
        } else {
            directRows = 1;
        }
    } else {
        for (int index = 0; index < rowCount; ++index) {
            if (SignalNeedsNeighborhoodHost(params.color_source_stack[index].signal)) {
                ++neighborhoodRows;
            } else {
                ++directRows;
            }
        }
    }
    if (outDirectRows) *outDirectRows = directRows;
    if (outNeighborhoodRows) *outNeighborhoodRows = neighborhoodRows;
}

void FillCudaFieldSignalStats(
    SdfColorPipelinePostprocessStats* outStats,
    const KernelParams& params,
    int outputPixelStep,
    std::size_t sampleCount,
    std::size_t filledPixelCount) {
    if (!outStats) {
        return;
    }
    int directRows = 0;
    int neighborhoodRows = 0;
    CountSourceRowsForStats(params, &directRows, &neighborhoodRows);
    outStats->direct_sample_count = neighborhoodRows == 0 ? sampleCount : 0;
    outStats->neighborhood_sample_count = neighborhoodRows > 0 ? sampleCount : 0;
    outStats->source_direct_sample_count = sampleCount * static_cast<std::size_t>(directRows);
    outStats->source_neighborhood_sample_count = sampleCount * static_cast<std::size_t>(neighborhoodRows);
    outStats->filled_pixel_count = filledPixelCount;
    outStats->output_pixel_step = outputPixelStep;
    outStats->worker_count = 1;
    outStats->backend_used = SdfColorPipelinePostprocessBackend::cuda_field_signal;
    outStats->backend_fallback_used = false;
}

struct CudaDirectScalarRegistrar {
    CudaDirectScalarRegistrar() {
        RegisterSdfColorPipelineCudaDirectScalarBackend(ApplyLensSdfColorPipelinePostprocessCudaDirectScalar);
        RegisterSdfColorPipelineCudaFieldSignalBackend(ApplyLensSdfColorPipelinePostprocessCudaFieldSignal);
    }
};

CudaDirectScalarRegistrar gCudaDirectScalarRegistrar;

} // namespace

bool ApplyLensSdfColorPipelinePostprocessCudaDirectScalar(
    const SdfFieldView& field,
    const RenderSettings& render,
    const KernelParams& params,
    std::uint32_t* ioRgba,
    std::string* outError,
    SdfColorPipelinePostprocessStats* outStats,
    const SdfColorPipelinePostprocessOptions* options) {
    if (!ioRgba || render.resolution.x <= 0 || render.resolution.y <= 0) {
        if (outError) *outError = "CUDA direct scalar SDF postprocess received an invalid render target";
        return false;
    }
    if (!field.signed_distance_px || field.width <= 0 || field.height <= 0 ||
        field.signed_distance_count != FieldPixelCount(field)) {
        if (outError) *outError = "CUDA direct scalar SDF postprocess requires a valid Lens SDF field";
        return false;
    }
    if (!ColorPipelineSdfPostprocessCanUseCudaDirectScalar(params)) {
        if (outError) *outError = "CUDA direct scalar SDF postprocess does not support this Source stack";
        return false;
    }

    const std::size_t fieldCount = FieldPixelCount(field);
    const std::size_t renderCount = RenderPixelCount(render);
    if (fieldCount == 0 || renderCount == 0 ||
        fieldCount > static_cast<std::size_t>(0x7fffffff) ||
        renderCount > static_cast<std::size_t>(0x7fffffff)) {
        if (outError) *outError = "CUDA direct scalar SDF postprocess received unsupported dimensions";
        return false;
    }

    const std::size_t fieldBytes = fieldCount * sizeof(float);
    const std::size_t rgbaBytes = renderCount * sizeof(std::uint32_t);
    CudaPostprocessBuffers buffers;
    if (!buffers.Allocate(fieldBytes, rgbaBytes) ||
        cudaMemcpy(buffers.field, field.signed_distance_px, fieldBytes, cudaMemcpyHostToDevice) != cudaSuccess) {
        if (outError) *outError = "CUDA direct scalar SDF postprocess could not allocate or upload buffers";
        return false;
    }

    const int outputPixelStep = NormalizePostprocessPixelStepHost(options ? options->output_pixel_step : 1);
    std::size_t sampleCount = 0;
    if (outputPixelStep == 1 &&
        field.width <= render.resolution.x &&
        field.height <= render.resolution.y &&
        (field.width < render.resolution.x || field.height < render.resolution.y)) {
        sampleCount = fieldCount;
        const int blocks = static_cast<int>((fieldCount + kThreadsPerBlock - 1) / kThreadsPerBlock);
        SdfDirectScalarFieldCellKernel<<<blocks, kThreadsPerBlock>>>(
            buffers.field,
            field.width,
            field.height,
            render.resolution.x,
            render.resolution.y,
            field.pixel_scale,
            params,
            buffers.rgba);
    } else {
        const int blockRows = CeilDivHost(render.resolution.y, outputPixelStep);
        const int blockCols = CeilDivHost(render.resolution.x, outputPixelStep);
        sampleCount = static_cast<std::size_t>(blockRows) * static_cast<std::size_t>(blockCols);
        const int blocks = static_cast<int>((sampleCount + kThreadsPerBlock - 1) / kThreadsPerBlock);
        SdfDirectScalarRenderBlockKernel<<<blocks, kThreadsPerBlock>>>(
            buffers.field,
            field.width,
            field.height,
            render.resolution.x,
            render.resolution.y,
            outputPixelStep,
            blockCols,
            blockRows,
            field.pixel_scale,
            params,
            buffers.rgba);
    }

    if (cudaGetLastError() != cudaSuccess || cudaDeviceSynchronize() != cudaSuccess) {
        if (outError) *outError = "CUDA direct scalar SDF postprocess kernel failed";
        return false;
    }
    if (cudaMemcpy(ioRgba, buffers.rgba, rgbaBytes, cudaMemcpyDeviceToHost) != cudaSuccess) {
        if (outError) *outError = "CUDA direct scalar SDF postprocess could not download RGBA";
        return false;
    }

    FillCudaStats(outStats, params, outputPixelStep, sampleCount, renderCount);
    return true;
}

bool ApplyLensSdfColorPipelinePostprocessCudaFieldSignal(
    const SdfFieldView& field,
    const RenderSettings& render,
    const KernelParams& params,
    std::uint32_t* ioRgba,
    std::string* outError,
    SdfColorPipelinePostprocessStats* outStats,
    const SdfColorPipelinePostprocessOptions* options) {
    if (!ioRgba || render.resolution.x <= 0 || render.resolution.y <= 0) {
        if (outError) *outError = "CUDA field-signal SDF postprocess received an invalid render target";
        return false;
    }
    if (!field.signed_distance_px || field.width <= 0 || field.height <= 0 ||
        field.signed_distance_count != FieldPixelCount(field)) {
        if (outError) *outError = "CUDA field-signal SDF postprocess requires a valid Lens SDF field";
        return false;
    }
    if (!ColorPipelineSdfPostprocessCanUseCudaFieldSignal(params)) {
        if (outError) *outError = "CUDA field-signal SDF postprocess does not support this Source stack";
        return false;
    }

    const std::size_t fieldCount = FieldPixelCount(field);
    const std::size_t renderCount = RenderPixelCount(render);
    if (fieldCount == 0 || renderCount == 0 ||
        fieldCount > static_cast<std::size_t>(0x7fffffff) ||
        renderCount > static_cast<std::size_t>(0x7fffffff)) {
        if (outError) *outError = "CUDA field-signal SDF postprocess received unsupported dimensions";
        return false;
    }

    const std::size_t fieldBytes = fieldCount * sizeof(float);
    const std::size_t rgbaBytes = renderCount * sizeof(std::uint32_t);
    CudaPostprocessBuffers buffers;
    if (!buffers.Allocate(fieldBytes, rgbaBytes) ||
        cudaMemcpy(buffers.field, field.signed_distance_px, fieldBytes, cudaMemcpyHostToDevice) != cudaSuccess) {
        if (outError) *outError = "CUDA field-signal SDF postprocess could not allocate or upload buffers";
        return false;
    }

    const int outputPixelStep = NormalizePostprocessPixelStepHost(options ? options->output_pixel_step : 1);
    std::size_t sampleCount = 0;
    if (outputPixelStep == 1 &&
        field.width <= render.resolution.x &&
        field.height <= render.resolution.y &&
        (field.width < render.resolution.x || field.height < render.resolution.y)) {
        sampleCount = fieldCount;
        const int blocks = static_cast<int>((fieldCount + kThreadsPerBlock - 1) / kThreadsPerBlock);
        SdfFieldSignalFieldCellKernel<<<blocks, kThreadsPerBlock>>>(
            buffers.field,
            field.width,
            field.height,
            render.resolution.x,
            render.resolution.y,
            field.pixel_scale,
            params,
            buffers.rgba);
    } else {
        const int blockRows = CeilDivHost(render.resolution.y, outputPixelStep);
        const int blockCols = CeilDivHost(render.resolution.x, outputPixelStep);
        sampleCount = static_cast<std::size_t>(blockRows) * static_cast<std::size_t>(blockCols);
        const int blocks = static_cast<int>((sampleCount + kThreadsPerBlock - 1) / kThreadsPerBlock);
        SdfFieldSignalRenderBlockKernel<<<blocks, kThreadsPerBlock>>>(
            buffers.field,
            field.width,
            field.height,
            render.resolution.x,
            render.resolution.y,
            outputPixelStep,
            blockCols,
            blockRows,
            field.pixel_scale,
            params,
            buffers.rgba);
    }

    if (cudaGetLastError() != cudaSuccess || cudaDeviceSynchronize() != cudaSuccess) {
        if (outError) *outError = "CUDA field-signal SDF postprocess kernel failed";
        return false;
    }
    if (cudaMemcpy(ioRgba, buffers.rgba, rgbaBytes, cudaMemcpyDeviceToHost) != cudaSuccess) {
        if (outError) *outError = "CUDA field-signal SDF postprocess could not download RGBA";
        return false;
    }

    FillCudaFieldSignalStats(outStats, params, outputPixelStep, sampleCount, renderCount);
    return true;
}
