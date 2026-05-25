#include "../src/color_pipeline_sdf_postprocess.h"
#include "../src/color_pipeline_core.h"

#include <cmath>
#include <cstdio>
#include <cstdint>
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
        std::printf("  FAIL: %s\n", message);
    }
}

std::uint64_t HashFrame(const std::vector<std::uint32_t>& pixels) {
    std::uint64_t hash = 1469598103934665603ull;
    for (std::uint32_t pixel : pixels) {
        hash ^= static_cast<std::uint64_t>(pixel);
        hash *= 1099511628211ull;
    }
    return hash;
}

SdfFieldResult MakeTestField() {
    SdfFieldResult field;
    field.width = 4;
    field.height = 4;
    field.pixel_scale = 1.0f;
    field.signed_distance_px.resize(16);
    for (int y = 0; y < field.height; ++y) {
        for (int x = 0; x < field.width; ++x) {
            const float fx = static_cast<float>(x) - 1.5f;
            const float fy = static_cast<float>(y) - 1.5f;
            field.signed_distance_px[static_cast<std::size_t>(y * field.width + x)] =
                (fx * fy) + (0.25f * fx);
        }
    }
    return field;
}

KernelParams SdfParams(ColorSignal signal, ColorPalette palette = ColorPalette::cyclic_escape) {
    KernelParams params{};
    params.coloring_mode = palette == ColorPalette::phase_wheel ? ColoringMode::phase : ColoringMode::smooth_escape;
    params.color_pipeline = {signal, palette, palette == ColorPalette::phase_wheel ? ColorGradingPreset::phase_default : ColorGradingPreset::escape_default};
    params.color_source_stack_count = 1;
    params.color_source_stack[0].signal = signal;
    params.color_source_stack[0].params.scale = signal == ColorSignal::sdf_signed_distance ? 0.05f : 1.0f;
    params.color_source_stack[0].params.bias = signal == ColorSignal::sdf_signed_distance || signal == ColorSignal::sdf_curvature ? 0.5f : 0.0f;
    params.color_source_stack[0].params.blend_weight = 1.0f;
    params.color_shape = ColorPipelineShape::identity;
    params.color_pipeline.palette = palette;
    params.color_heatmap_cycle_scale = 1.0f;
    params.color_heatmap_saturation = 1.0f;
    params.exposure = 1.0f;
    params.color_saturation = 1.0f;
    params.color_contrast = 1.0f;
    return params;
}

void TestEachSdfSourceSignalPostprocessesFrame() {
    const SdfFieldResult field = MakeTestField();
    RenderSettings render{};
    render.resolution = {4, 4};
    const ColorSignal signals[] = {
        ColorSignal::sdf_signed_distance,
        ColorSignal::sdf_inside_outside,
        ColorSignal::sdf_boundary_band,
        ColorSignal::sdf_normal_angle,
        ColorSignal::sdf_curvature,
    };
    for (ColorSignal signal : signals) {
        KernelParams params = SdfParams(
            signal,
            signal == ColorSignal::sdf_normal_angle ? ColorPalette::phase_wheel : ColorPalette::cyclic_escape);
        std::vector<std::uint32_t> pixels(16, 0x12345678u);
        const std::uint64_t before = HashFrame(pixels);
        std::string error;
        Check(ColorPipelineUsesSdfSource(params), "TestEachSdfSourceSignalPostprocessesFrame_UsesSdfSource");
        Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, params, pixels.data(), &error),
            "TestEachSdfSourceSignalPostprocessesFrame_PostprocessSucceeds");
        Check(HashFrame(pixels) != before, "TestEachSdfSourceSignalPostprocessesFrame_FrameChanges");
    }
}

void TestBoundaryBandWidthChangesPostprocessFrame() {
    const SdfFieldResult field = MakeTestField();
    RenderSettings render{};
    render.resolution = {4, 4};

    KernelParams narrow = SdfParams(ColorSignal::sdf_boundary_band);
    narrow.color_source_stack[0].params.sdf_boundary_width_px = 0.5f;
    KernelParams wide = SdfParams(ColorSignal::sdf_boundary_band);
    wide.color_source_stack[0].params.sdf_boundary_width_px = 6.0f;

    std::vector<std::uint32_t> narrowPixels(16, 0x12345678u);
    std::vector<std::uint32_t> widePixels(16, 0x12345678u);
    std::string error;
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, narrow, narrowPixels.data(), &error),
        "TestBoundaryBandWidthChangesPostprocessFrame_NarrowSucceeds");
    error.clear();
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, wide, widePixels.data(), &error),
        "TestBoundaryBandWidthChangesPostprocessFrame_WideSucceeds");
    Check(HashFrame(narrowPixels) != HashFrame(widePixels),
        "TestBoundaryBandWidthChangesPostprocessFrame_WidthChangesFrame");
}

void TestNormalAnglePhaseOffsetChangesFrameWithoutReclassifyingScalarSdfSources() {
    const SdfFieldResult field = MakeTestField();
    RenderSettings render{};
    render.resolution = {4, 4};

    KernelParams baseNormalAngle = SdfParams(ColorSignal::sdf_normal_angle, ColorPalette::phase_wheel);
    KernelParams offsetNormalAngle = baseNormalAngle;
    offsetNormalAngle.color_source_stack[0].params.bias = 0.25f;

    std::vector<std::uint32_t> basePixels(16, 0x12345678u);
    std::vector<std::uint32_t> offsetPixels(16, 0x12345678u);
    std::string error;
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, baseNormalAngle, basePixels.data(), &error),
        "TestNormalAnglePhaseOffsetChangesFrame_BaseSucceeds");
    error.clear();
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, offsetNormalAngle, offsetPixels.data(), &error),
        "TestNormalAnglePhaseOffsetChangesFrame_OffsetSucceeds");
    Check(HashFrame(basePixels) != HashFrame(offsetPixels),
        "TestNormalAnglePhaseOffsetChangesFrame_PhaseOffsetMovesFrame");
    Check(color_pipeline_core::ColorPipelineSourceSignalKindForSignal(ColorSignal::sdf_normal_angle) ==
            color_pipeline_core::ColorPipelineSourceSignalKind::phase,
        "TestNormalAnglePhaseOffsetChangesFrame_NormalAngleClassifiedPhase");
    Check(color_pipeline_core::ColorPipelineSourceSignalKindForSignal(ColorSignal::sdf_signed_distance) ==
            color_pipeline_core::ColorPipelineSourceSignalKind::scalar,
        "TestNormalAnglePhaseOffsetChangesFrame_SignedDistanceRemainsScalar");
    Check(color_pipeline_core::ColorPipelineSourceSignalKindForSignal(ColorSignal::sdf_boundary_band) ==
            color_pipeline_core::ColorPipelineSourceSignalKind::scalar,
        "TestNormalAnglePhaseOffsetChangesFrame_BoundaryBandRemainsScalar");
}

void TestMixedSourceStackFailsClosed() {
    const SdfFieldResult field = MakeTestField();
    RenderSettings render{};
    render.resolution = {4, 4};
    KernelParams params = SdfParams(ColorSignal::sdf_signed_distance);
    params.color_source_stack_count = 2;
    params.color_source_stack[1].signal = ColorSignal::smooth_escape;
    params.color_source_stack[1].params.blend_weight = 0.5f;
    std::vector<std::uint32_t> pixels(16, 0);
    std::string error;
    Check(!ColorPipelineSourceStackIsSdfOnly(params, &error) &&
            error.find("mix") != std::string::npos,
        "TestMixedSourceStackFailsClosed_SourceStackClassified");
    error.clear();
    Check(!ApplyLensSdfColorPipelinePostprocess(field.View(), render, params, pixels.data(), &error) &&
            error.find("mix") != std::string::npos,
        "TestMixedSourceStackFailsClosed_PostprocessRejects");
}

} // namespace

int main() {
    TestEachSdfSourceSignalPostprocessesFrame();
    TestBoundaryBandWidthChangesPostprocessFrame();
    TestNormalAnglePhaseOffsetChangesFrameWithoutReclassifyingScalarSdfSources();
    TestMixedSourceStackFailsClosed();

    std::printf("test_color_pipeline_sdf_postprocess: passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
