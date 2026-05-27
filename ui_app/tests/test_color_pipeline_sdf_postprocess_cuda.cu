#include "../src/color_pipeline_sdf_postprocess.h"
#include "../src/color_pipeline_sdf_postprocess_cuda.h"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace {

int g_passed = 0;
int g_failed = 0;

void Check(bool condition, const char* message) {
    if (condition) {
        ++g_passed;
    } else {
        ++g_failed;
        std::cerr << "  FAIL: " << message << "\n";
    }
}

SdfFieldResult MakeGradientField(int width, int height) {
    SdfFieldResult field;
    field.width = width;
    field.height = height;
    field.pixel_scale = 1.0f;
    field.sign_convention = SdfSignConvention::negative_inside_positive_outside;
    field.source_kind = SdfFieldSourceKind::mask_derived;
    field.signed_distance_px.resize(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const float fx = static_cast<float>(x) - static_cast<float>(width) * 0.5f;
            const float fy = static_cast<float>(y) - static_cast<float>(height) * 0.5f;
            field.signed_distance_px[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)] =
                std::sin(fx * 0.35f) * 4.0f + std::cos(fy * 0.25f) * 3.0f + (0.15f * fx) - (0.1f * fy);
        }
    }
    return field;
}

RenderSettings Render(int width, int height) {
    RenderSettings render{};
    render.resolution = {width, height};
    return render;
}

KernelParams SdfParams(ColorSignal signal) {
    KernelParams params{};
    params.coloring_mode = ColoringMode::smooth_escape;
    params.color_pipeline = {signal, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
    params.color_source_stack_count = 1;
    params.color_source_stack[0].signal = signal;
    params.color_source_stack[0].params.scale = signal == ColorSignal::sdf_signed_distance ? 0.05f : 1.0f;
    params.color_source_stack[0].params.bias = signal == ColorSignal::sdf_signed_distance ? 0.5f : 0.0f;
    params.color_source_stack[0].params.blend_weight = 1.0f;
    params.color_shape = ColorPipelineShape::identity;
    params.color_pipeline.palette = ColorPalette::cyclic_escape;
    params.color_heatmap_cycle_scale = 1.0f;
    params.color_heatmap_saturation = 1.0f;
    params.exposure = 1.0f;
    params.color_saturation = 1.0f;
    params.color_contrast = 1.0f;
    return params;
}

bool RunPostprocess(
    const SdfFieldResult& field,
    const RenderSettings& render,
    const KernelParams& params,
    SdfColorPipelinePostprocessBackend backend,
    std::vector<std::uint32_t>& outPixels,
    SdfColorPipelinePostprocessStats& outStats,
    int outputPixelStep = 1) {
    outPixels.assign(
        static_cast<std::size_t>(render.resolution.x) * static_cast<std::size_t>(render.resolution.y),
        0x12345678u);
    SdfColorPipelinePostprocessOptions options{};
    options.output_pixel_step = outputPixelStep;
    options.max_worker_threads = 1;
    options.backend_preference = backend;
    std::string error;
    const bool ok = ApplyLensSdfColorPipelinePostprocess(
        field.View(),
        render,
        params,
        outPixels.data(),
        &error,
        &outStats,
        &options);
    if (!ok) {
        std::cerr << "postprocess failed: " << error << "\n";
    }
    return ok;
}

void CheckCpuGpuParity(
    const char* label,
    const SdfFieldResult& field,
    const RenderSettings& render,
    const KernelParams& params,
    int outputPixelStep = 1,
    SdfColorPipelinePostprocessBackend cudaBackend = SdfColorPipelinePostprocessBackend::cuda_direct_scalar) {
    std::vector<std::uint32_t> cpuPixels;
    std::vector<std::uint32_t> gpuPixels;
    SdfColorPipelinePostprocessStats cpuStats{};
    SdfColorPipelinePostprocessStats gpuStats{};
    Check(RunPostprocess(field, render, params, SdfColorPipelinePostprocessBackend::cpu, cpuPixels, cpuStats, outputPixelStep),
        "CheckCpuGpuParity_CPU_Succeeds");
    Check(RunPostprocess(field, render, params, cudaBackend, gpuPixels, gpuStats, outputPixelStep),
        "CheckCpuGpuParity_GPU_Succeeds");
    Check(cpuPixels == gpuPixels, label);
    Check(gpuStats.backend_used == cudaBackend,
        "CheckCpuGpuParity_GPUReportsCudaBackend");
    Check(!gpuStats.backend_fallback_used,
        "CheckCpuGpuParity_GPUDoesNotReportFallback");
    if (cudaBackend == SdfColorPipelinePostprocessBackend::cuda_direct_scalar) {
        Check(gpuStats.neighborhood_sample_count == 0,
            "CheckCpuGpuParity_GPURemainsDirectOnly");
    }
    Check(gpuStats.filled_pixel_count == cpuStats.filled_pixel_count,
        "CheckCpuGpuParity_GPUFilledCountMatchesCPU");
}

void TestSupportedDirectScalarStacksUseCudaWithExactPixels() {
    const SdfFieldResult field = MakeGradientField(16, 12);
    CheckCpuGpuParity(
        "TestSupportedDirectScalarStacksUseCudaWithExactPixels_SignedDistancePixelsMatch",
        field,
        Render(16, 12),
        SdfParams(ColorSignal::sdf_signed_distance));
    CheckCpuGpuParity(
        "TestSupportedDirectScalarStacksUseCudaWithExactPixels_LensFieldV2PixelsMatch",
        field,
        Render(16, 12),
        SdfParams(ColorSignal::lens_field_v2_distance));
    KernelParams contrastedLensField = SdfParams(ColorSignal::lens_field_v2_distance);
    contrastedLensField.color_source_stack[0].params.lens_field_v2_sign_contrast = 0.85f;
    CheckCpuGpuParity(
        "TestSupportedDirectScalarStacksUseCudaWithExactPixels_LensFieldV2SignContrastPixelsMatch",
        field,
        Render(16, 12),
        contrastedLensField);
    SdfFieldResult scaledField = field;
    scaledField.pixel_scale = 4.0f;
    CheckCpuGpuParity(
        "TestSupportedDirectScalarStacksUseCudaWithExactPixels_LensFieldV2ScaledPixelsMatch",
        scaledField,
        Render(16, 12),
        SdfParams(ColorSignal::lens_field_v2_distance));
    CheckCpuGpuParity(
        "TestSupportedDirectScalarStacksUseCudaWithExactPixels_InsideOutsidePixelsMatch",
        field,
        Render(16, 12),
        SdfParams(ColorSignal::sdf_inside_outside));
    KernelParams boundary = SdfParams(ColorSignal::sdf_boundary_band);
    boundary.color_source_stack[0].params.sdf_boundary_width_px = 5.0f;
    CheckCpuGpuParity(
        "TestSupportedDirectScalarStacksUseCudaWithExactPixels_BoundaryBandPixelsMatch",
        field,
        Render(16, 12),
        boundary);
}

void TestDirectScalarMultiRowStackUsesCudaWithExactPixels() {
    const SdfFieldResult field = MakeGradientField(18, 10);
    KernelParams params = SdfParams(ColorSignal::sdf_signed_distance);
    params.color_source_stack_count = 3;
    params.color_source_stack[0].signal = ColorSignal::sdf_signed_distance;
    params.color_source_stack[0].params.scale = 0.05f;
    params.color_source_stack[0].params.bias = 0.45f;
    params.color_source_stack[0].params.blend_weight = 1.0f;
    params.color_source_stack[1].signal = ColorSignal::sdf_inside_outside;
    params.color_source_stack[1].params.scale = 0.75f;
    params.color_source_stack[1].params.bias = 0.1f;
    params.color_source_stack[1].params.blend_weight = 0.35f;
    params.color_source_stack[2].signal = ColorSignal::sdf_boundary_band;
    params.color_source_stack[2].params.sdf_boundary_width_px = 4.0f;
    params.color_source_stack[2].params.blend_weight = 0.6f;
    CheckCpuGpuParity(
        "TestDirectScalarMultiRowStackUsesCudaWithExactPixels_PixelsMatch",
        field,
        Render(18, 10),
        params);
}

void TestCudaMatchesCpuForDownsampledFieldAndOutputPixelStep() {
    const SdfFieldResult field = MakeGradientField(5, 4);
    CheckCpuGpuParity(
        "TestCudaMatchesCpuForDownsampledFieldAndOutputPixelStep_DownsampledFieldPixelsMatch",
        field,
        Render(11, 7),
        SdfParams(ColorSignal::sdf_signed_distance));
    KernelParams boundary = SdfParams(ColorSignal::sdf_boundary_band);
    boundary.color_source_stack[0].params.sdf_boundary_width_px = 4.0f;
    CheckCpuGpuParity(
        "TestCudaMatchesCpuForDownsampledFieldAndOutputPixelStep_OutputStepPixelsMatch",
        MakeGradientField(13, 9),
        Render(13, 9),
        boundary,
        2);
}

void TestUnsupportedStacksFallbackToCpuInAutoMode() {
    const SdfFieldResult field = MakeGradientField(8, 8);
    std::vector<std::uint32_t> autoPixels;
    SdfColorPipelinePostprocessStats autoStats{};
    KernelParams normalAngle = SdfParams(ColorSignal::sdf_normal_angle);
    Check(!ColorPipelineSdfPostprocessCanUseCudaDirectScalar(normalAngle),
        "TestUnsupportedStacksFallbackToCpuInAutoMode_NormalAnglePreflightRejectsCuda");
    Check(ColorPipelineSdfPostprocessCanUseCudaFieldSignal(normalAngle),
        "TestUnsupportedStacksFallbackToCpuInAutoMode_NormalAngleFieldSignalPreflightAcceptsCuda");
    Check(RunPostprocess(field, Render(8, 8), normalAngle, SdfColorPipelinePostprocessBackend::auto_backend, autoPixels, autoStats),
        "TestUnsupportedStacksFallbackToCpuInAutoMode_NormalAngleAutoSucceeds");
    Check(autoStats.backend_used == SdfColorPipelinePostprocessBackend::cuda_field_signal && !autoStats.backend_fallback_used,
        "TestUnsupportedStacksFallbackToCpuInAutoMode_NormalAngleUsesCudaFieldSignal");

    KernelParams rowStep = SdfParams(ColorSignal::sdf_signed_distance);
    rowStep.color_source_stack[0].params.sdf_sample_step = 2;
    Check(!ColorPipelineSdfPostprocessCanUseCudaDirectScalar(rowStep),
        "TestUnsupportedStacksFallbackToCpuInAutoMode_RowSampleStepPreflightRejectsCuda");
    Check(!ColorPipelineSdfPostprocessCanUseCudaFieldSignal(rowStep),
        "TestUnsupportedStacksFallbackToCpuInAutoMode_RowSampleStepFieldSignalPreflightRejectsCuda");
}

void TestExplicitUnsupportedGpuBackendsFailClosed() {
    const SdfFieldResult field = MakeGradientField(8, 8);
    const RenderSettings render = Render(8, 8);
    std::vector<std::uint32_t> pixels;
    SdfColorPipelinePostprocessStats stats{};

    KernelParams normalAngle = SdfParams(ColorSignal::sdf_normal_angle);
    Check(!RunPostprocess(
            field,
            render,
            normalAngle,
            SdfColorPipelinePostprocessBackend::cuda_direct_scalar,
            pixels,
            stats),
        "TestExplicitUnsupportedGpuBackendsFailClosed_NormalAngleDoesNotSilentlyRunCpuForDirectScalar");

    KernelParams rowStep = SdfParams(ColorSignal::sdf_normal_angle);
    rowStep.color_source_stack[0].params.sdf_sample_step = 2;
    Check(!RunPostprocess(
            field,
            render,
            rowStep,
            SdfColorPipelinePostprocessBackend::cuda_field_signal,
            pixels,
            stats),
        "TestExplicitUnsupportedGpuBackendsFailClosed_RowStepDoesNotSilentlyRunCpuForFieldSignal");

    KernelParams direct = SdfParams(ColorSignal::sdf_signed_distance);
    Check(!RunPostprocess(
            field,
            render,
            direct,
            SdfColorPipelinePostprocessBackend::cuda_field_signal,
            pixels,
            stats),
        "TestExplicitUnsupportedGpuBackendsFailClosed_DirectScalarDoesNotSilentlyUseFieldSignal");
}

void TestNeighborhoodFieldSignalsUseCudaWithExactPixels() {
    const SdfFieldResult field = MakeGradientField(17, 13);
    KernelParams normalAngle = SdfParams(ColorSignal::sdf_normal_angle);
    normalAngle.color_pipeline.palette = ColorPalette::phase_wheel;
    normalAngle.color_source_stack[0].params.scale = -0.75f;
    normalAngle.color_source_stack[0].params.bias = 0.35f;
    Check(ColorPipelineSdfPostprocessCanUseCudaFieldSignal(normalAngle),
        "TestNeighborhoodFieldSignalsUseCudaWithExactPixels_NormalAnglePreflightAcceptsCuda");
    CheckCpuGpuParity(
        "TestNeighborhoodFieldSignalsUseCudaWithExactPixels_NormalAnglePixelsMatch",
        field,
        Render(17, 13),
        normalAngle,
        1,
        SdfColorPipelinePostprocessBackend::cuda_field_signal);

    KernelParams curvature = SdfParams(ColorSignal::sdf_curvature);
    curvature.color_source_stack[0].params.scale = 0.3f;
    curvature.color_source_stack[0].params.bias = 0.5f;
    Check(ColorPipelineSdfPostprocessCanUseCudaFieldSignal(curvature),
        "TestNeighborhoodFieldSignalsUseCudaWithExactPixels_CurvaturePreflightAcceptsCuda");
    CheckCpuGpuParity(
        "TestNeighborhoodFieldSignalsUseCudaWithExactPixels_CurvaturePixelsMatch",
        field,
        Render(17, 13),
        curvature,
        1,
        SdfColorPipelinePostprocessBackend::cuda_field_signal);
}

void TestNeighborhoodMixedSdfStackUsesCudaWithExactPixels() {
    const SdfFieldResult field = MakeGradientField(9, 7);
    KernelParams params = SdfParams(ColorSignal::sdf_normal_angle);
    params.color_pipeline.palette = ColorPalette::phase_wheel;
    params.color_source_stack_count = 4;
    params.color_source_stack[0].signal = ColorSignal::sdf_normal_angle;
    params.color_source_stack[0].params.scale = 1.2f;
    params.color_source_stack[0].params.bias = -0.15f;
    params.color_source_stack[0].params.blend_weight = 1.0f;
    params.color_source_stack[1].signal = ColorSignal::sdf_curvature;
    params.color_source_stack[1].params.scale = 0.25f;
    params.color_source_stack[1].params.bias = 0.4f;
    params.color_source_stack[1].params.blend_weight = 0.45f;
    params.color_source_stack[2].signal = ColorSignal::sdf_boundary_band;
    params.color_source_stack[2].params.sdf_boundary_width_px = 5.0f;
    params.color_source_stack[2].params.sdf_gate = ColorPipelineSdfGateMode::boundary_band;
    params.color_source_stack[2].params.sdf_gate_width_px = 3.0f;
    params.color_source_stack[2].params.blend_weight = 0.3f;
    params.color_source_stack[3].signal = ColorSignal::sdf_signed_distance;
    params.color_source_stack[3].params.scale = 0.04f;
    params.color_source_stack[3].params.bias = 0.5f;
    params.color_source_stack[3].params.blend_weight = 0.2f;
    Check(ColorPipelineSdfPostprocessCanUseCudaFieldSignal(params),
        "TestNeighborhoodMixedSdfStackUsesCudaWithExactPixels_PreflightAcceptsCuda");
    CheckCpuGpuParity(
        "TestNeighborhoodMixedSdfStackUsesCudaWithExactPixels_PixelsMatch",
        field,
        Render(19, 11),
        params,
        1,
        SdfColorPipelinePostprocessBackend::cuda_field_signal);
    CheckCpuGpuParity(
        "TestNeighborhoodMixedSdfStackUsesCudaWithExactPixels_OutputStepPixelsMatch",
        field,
        Render(19, 11),
        params,
        2,
        SdfColorPipelinePostprocessBackend::cuda_field_signal);
}

} // namespace

int main()
{
    TestSupportedDirectScalarStacksUseCudaWithExactPixels();
    TestDirectScalarMultiRowStackUsesCudaWithExactPixels();
    TestCudaMatchesCpuForDownsampledFieldAndOutputPixelStep();
    TestUnsupportedStacksFallbackToCpuInAutoMode();
    TestExplicitUnsupportedGpuBackendsFailClosed();
    TestNeighborhoodFieldSignalsUseCudaWithExactPixels();
    TestNeighborhoodMixedSdfStackUsesCudaWithExactPixels();

    std::cout << "test_color_pipeline_sdf_postprocess_cuda: passed=" << g_passed
              << " failed=" << g_failed << "\n";
    return g_failed == 0 ? 0 : 1;
}
