#include "../src/color_pipeline_sdf_postprocess.h"
#include "../src/color_pipeline_core.h"
#include "../src/sdf_field_signal.h"

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

SdfFieldResult MakeLargeTestField(int width, int height) {
    SdfFieldResult field;
    field.width = width;
    field.height = height;
    field.pixel_scale = 1.0f;
    field.signed_distance_px.resize(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const float fx = (static_cast<float>(x) - static_cast<float>(width) * 0.5f) * 0.125f;
            const float fy = (static_cast<float>(y) - static_cast<float>(height) * 0.5f) * 0.125f;
            field.signed_distance_px[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)] =
                std::sin(fx) * std::cos(fy) * 6.0f + (0.25f * fx) - (0.15f * fy);
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
        ColorSignal::lens_field_v2_distance,
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

void TestLensFieldV2ResponseDoesNotAliasRawSignedDistance() {
    SdfFieldResult field = MakeTestField();
    field.pixel_scale = 1.0f;
    RenderSettings render{};
    render.resolution = {4, 4};

    KernelParams rawSigned = SdfParams(ColorSignal::sdf_signed_distance);
    rawSigned.color_source_stack[0].params.scale = 1.0f;
    rawSigned.color_source_stack[0].params.bias = 0.0f;
    KernelParams lensField = SdfParams(ColorSignal::lens_field_v2_distance);
    lensField.color_source_stack[0].params.scale = 1.0f;
    lensField.color_source_stack[0].params.bias = 0.0f;

    std::vector<std::uint32_t> rawPixels(16, 0x12345678u);
    std::vector<std::uint32_t> lensPixels(16, 0x12345678u);
    std::string error;
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, rawSigned, rawPixels.data(), &error),
        "TestLensFieldV2ResponseDoesNotAliasRawSignedDistance_RawSucceeds");
    error.clear();
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, lensField, lensPixels.data(), &error),
        "TestLensFieldV2ResponseDoesNotAliasRawSignedDistance_LensSucceeds");
    Check(HashFrame(rawPixels) != HashFrame(lensPixels),
        "TestLensFieldV2ResponseDoesNotAliasRawSignedDistance_FramesDiffer");
}

void TestLensFieldV2SignContrastChangesFrameAndZeroPreservesLegacyResponse() {
    SdfFieldResult field = MakeLargeTestField(8, 8);
    field.pixel_scale = 2.0f;
    RenderSettings render{};
    render.resolution = {8, 8};

    KernelParams zeroContrast = SdfParams(ColorSignal::lens_field_v2_distance);
    zeroContrast.color_source_stack[0].params.scale = 1.0f;
    zeroContrast.color_source_stack[0].params.bias = 0.0f;
    zeroContrast.color_source_stack[0].params.lens_field_v2_sign_contrast = 0.0f;
    KernelParams strongContrast = zeroContrast;
    strongContrast.color_source_stack[0].params.lens_field_v2_sign_contrast = 0.85f;

    std::vector<std::uint32_t> zeroPixels(64, 0x12345678u);
    std::vector<std::uint32_t> strongPixels(64, 0x12345678u);
    std::string error;
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, zeroContrast, zeroPixels.data(), &error),
        "TestLensFieldV2SignContrastChangesFrame_ZeroSucceeds");
    error.clear();
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, strongContrast, strongPixels.data(), &error),
        "TestLensFieldV2SignContrastChangesFrame_StrongSucceeds");
    Check(HashFrame(zeroPixels) != HashFrame(strongPixels),
        "TestLensFieldV2SignContrastChangesFrame_StrongContrastChangesFrame");
    Check(std::fabs(ResolveLensFieldV2ResponseFromSignedDistancePx(8.0f, 2.0f, 0.0f) -
            ResolveLensFieldV2ResponseFromSignedDistancePx(8.0f, 2.0f)) <= 0.0001f,
        "TestLensFieldV2SignContrastChangesFrame_ZeroContrastPreservesLegacyHelper");
}

std::uint64_t RenderSdfSourceHashWithIdentityHeatmap(
    const SdfFieldResult& field,
    const RenderSettings& render,
    ColorSignal signal) {
    KernelParams params = SdfParams(signal, ColorPalette::cyclic_escape);
    params.color_source_stack[0].params.scale = 1.0f;
    params.color_source_stack[0].params.bias = 0.0f;
    if (signal == ColorSignal::sdf_boundary_band) {
        params.color_source_stack[0].params.sdf_boundary_width_px = 3.0f;
    }

    std::vector<std::uint32_t> pixels(
        static_cast<std::size_t>(render.resolution.x) * static_cast<std::size_t>(render.resolution.y),
        0x12345678u);
    std::string error;
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, params, pixels.data(), &error),
        "RenderSdfSourceHashWithIdentityHeatmap_PostprocessSucceeds");
    return HashFrame(pixels);
}

void TestSdfSourceDistinctnessMatrixRejectsRawSignedDistanceAliases() {
    SdfFieldResult field = MakeLargeTestField(8, 8);
    field.pixel_scale = 4.0f;
    RenderSettings render{};
    render.resolution = {8, 8};
    const std::uint64_t rawHash = RenderSdfSourceHashWithIdentityHeatmap(
        field,
        render,
        ColorSignal::sdf_signed_distance);

    struct ExpectedDistinctSource {
        ColorSignal signal;
        const char* message;
    };
    const ExpectedDistinctSource expectedDistinctSources[] = {
        {ColorSignal::sdf_inside_outside, "TestSdfSourceDistinctnessMatrix_InsideOutsideDoesNotAliasRaw"},
        {ColorSignal::sdf_boundary_band, "TestSdfSourceDistinctnessMatrix_BoundaryBandDoesNotAliasRaw"},
        {ColorSignal::sdf_normal_angle, "TestSdfSourceDistinctnessMatrix_NormalAngleDoesNotAliasRaw"},
        {ColorSignal::sdf_curvature, "TestSdfSourceDistinctnessMatrix_CurvatureDoesNotAliasRaw"},
        {ColorSignal::lens_field_v2_distance, "TestSdfSourceDistinctnessMatrix_LensFieldV2DoesNotAliasRaw"},
    };
    for (const ExpectedDistinctSource& expected : expectedDistinctSources) {
        const std::uint64_t sourceHash = RenderSdfSourceHashWithIdentityHeatmap(
            field,
            render,
            expected.signal);
        Check(sourceHash != rawHash, expected.message);
    }
}

void TestLensFieldV2ResponseHonorsFieldPixelScaleWithoutChangingRawSignedDistance() {
    SdfFieldResult field1x = MakeTestField();
    field1x.pixel_scale = 1.0f;
    SdfFieldResult field4x = field1x;
    field4x.pixel_scale = 4.0f;

    RenderSettings render{};
    render.resolution = {4, 4};

    KernelParams lensField = SdfParams(ColorSignal::lens_field_v2_distance);
    lensField.color_source_stack[0].params.scale = 1.0f;
    lensField.color_source_stack[0].params.bias = 0.0f;
    KernelParams rawSigned = SdfParams(ColorSignal::sdf_signed_distance);
    rawSigned.color_source_stack[0].params.scale = 1.0f;
    rawSigned.color_source_stack[0].params.bias = 0.0f;

    std::vector<std::uint32_t> lens1xPixels(16, 0x12345678u);
    std::vector<std::uint32_t> lens4xPixels(16, 0x12345678u);
    std::vector<std::uint32_t> raw1xPixels(16, 0x12345678u);
    std::vector<std::uint32_t> raw4xPixels(16, 0x12345678u);
    std::string error;
    Check(ApplyLensSdfColorPipelinePostprocess(field1x.View(), render, lensField, lens1xPixels.data(), &error),
        "TestLensFieldV2ResponseHonorsFieldPixelScale_Lens1xSucceeds");
    error.clear();
    Check(ApplyLensSdfColorPipelinePostprocess(field4x.View(), render, lensField, lens4xPixels.data(), &error),
        "TestLensFieldV2ResponseHonorsFieldPixelScale_Lens4xSucceeds");
    error.clear();
    Check(ApplyLensSdfColorPipelinePostprocess(field1x.View(), render, rawSigned, raw1xPixels.data(), &error),
        "TestLensFieldV2ResponseHonorsFieldPixelScale_Raw1xSucceeds");
    error.clear();
    Check(ApplyLensSdfColorPipelinePostprocess(field4x.View(), render, rawSigned, raw4xPixels.data(), &error),
        "TestLensFieldV2ResponseHonorsFieldPixelScale_Raw4xSucceeds");
    Check(HashFrame(lens1xPixels) != HashFrame(lens4xPixels),
        "TestLensFieldV2ResponseHonorsFieldPixelScale_LensUsesPixelScale");
    Check(HashFrame(raw1xPixels) == HashFrame(raw4xPixels),
        "TestLensFieldV2ResponseHonorsFieldPixelScale_RawIgnoresPixelScale");
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

void TestNormalAngleBoundaryGateMasksFullFieldDiagnostic() {
    const SdfFieldResult field = MakeTestField();
    RenderSettings render{};
    render.resolution = {4, 4};

    KernelParams fullField = SdfParams(ColorSignal::sdf_normal_angle, ColorPalette::phase_wheel);
    KernelParams gated = fullField;
    gated.color_source_stack[0].params.sdf_gate = ColorPipelineSdfGateMode::boundary_band;
    gated.color_source_stack[0].params.sdf_gate_width_px = 0.75f;
    KernelParams wideGate = gated;
    wideGate.color_source_stack[0].params.sdf_gate_width_px = 6.0f;

    std::vector<std::uint32_t> fullPixels(16, 0x12345678u);
    std::vector<std::uint32_t> gatedPixels(16, 0x12345678u);
    std::vector<std::uint32_t> widePixels(16, 0x12345678u);
    std::string error;
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, fullField, fullPixels.data(), &error),
        "TestNormalAngleBoundaryGateMasksFullFieldDiagnostic_FullFieldSucceeds");
    error.clear();
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, gated, gatedPixels.data(), &error),
        "TestNormalAngleBoundaryGateMasksFullFieldDiagnostic_GatedSucceeds");
    error.clear();
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, wideGate, widePixels.data(), &error),
        "TestNormalAngleBoundaryGateMasksFullFieldDiagnostic_WideGateSucceeds");
    Check(HashFrame(fullPixels) != HashFrame(gatedPixels),
        "TestNormalAngleBoundaryGateMasksFullFieldDiagnostic_GateChangesFrame");
    Check(HashFrame(gatedPixels) != HashFrame(widePixels),
        "TestNormalAngleBoundaryGateMasksFullFieldDiagnostic_GateWidthChangesFrame");
}

void TestNormalAngleGateWidthIsInactiveWhenGateIsOff() {
    const SdfFieldResult field = MakeTestField();
    RenderSettings render{};
    render.resolution = {4, 4};

    KernelParams narrowUnused = SdfParams(ColorSignal::sdf_normal_angle, ColorPalette::phase_wheel);
    narrowUnused.color_source_stack[0].params.sdf_gate = ColorPipelineSdfGateMode::none;
    narrowUnused.color_source_stack[0].params.sdf_gate_width_px = 0.25f;
    KernelParams wideUnused = narrowUnused;
    wideUnused.color_source_stack[0].params.sdf_gate_width_px = 16.0f;

    std::vector<std::uint32_t> narrowPixels(16, 0x12345678u);
    std::vector<std::uint32_t> widePixels(16, 0x12345678u);
    std::string error;
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, narrowUnused, narrowPixels.data(), &error),
        "TestNormalAngleGateWidthIsInactiveWhenGateIsOff_NarrowSucceeds");
    error.clear();
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, wideUnused, widePixels.data(), &error),
        "TestNormalAngleGateWidthIsInactiveWhenGateIsOff_WideSucceeds");
    Check(HashFrame(narrowPixels) == HashFrame(widePixels),
        "TestNormalAngleGateWidthIsInactiveWhenGateIsOff_WidthIgnoredWhenGateOff");
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

void TestBoundaryBandWidthStillAffectsMixedSdfStack() {
    const SdfFieldResult field = MakeTestField();
    RenderSettings render{};
    render.resolution = {4, 4};

    KernelParams narrow = SdfParams(ColorSignal::sdf_signed_distance);
    narrow.color_source_stack_count = 2;
    narrow.color_source_stack[1].signal = ColorSignal::sdf_boundary_band;
    narrow.color_source_stack[1].params.blend_weight = 0.75f;
    narrow.color_source_stack[1].params.sdf_boundary_width_px = 0.5f;

    KernelParams wide = narrow;
    wide.color_source_stack[1].params.sdf_boundary_width_px = 6.0f;

    std::vector<std::uint32_t> narrowPixels(16, 0x12345678u);
    std::vector<std::uint32_t> widePixels(16, 0x12345678u);
    std::string error;
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, narrow, narrowPixels.data(), &error),
        "TestBoundaryBandWidthStillAffectsMixedSdfStack_NarrowSucceeds");
    error.clear();
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, wide, widePixels.data(), &error),
        "TestBoundaryBandWidthStillAffectsMixedSdfStack_WideSucceeds");
    Check(HashFrame(narrowPixels) != HashFrame(widePixels),
        "TestBoundaryBandWidthStillAffectsMixedSdfStack_BoundaryConfigNotLost");
}

void TestScalarOnlySdfStackUsesDirectSamplesWithoutNeighborhood() {
    const SdfFieldResult field = MakeTestField();
    RenderSettings render{};
    render.resolution = {4, 4};

    KernelParams params = SdfParams(ColorSignal::sdf_signed_distance);
    params.color_source_stack_count = 3;
    params.color_source_stack[0].signal = ColorSignal::sdf_signed_distance;
    params.color_source_stack[0].params.scale = 0.05f;
    params.color_source_stack[0].params.bias = 0.5f;
    params.color_source_stack[0].params.blend_weight = 1.0f;
    params.color_source_stack[1].signal = ColorSignal::sdf_inside_outside;
    params.color_source_stack[1].params.blend_weight = 0.25f;
    params.color_source_stack[2].signal = ColorSignal::sdf_boundary_band;
    params.color_source_stack[2].params.blend_weight = 0.5f;
    params.color_source_stack[2].params.sdf_boundary_width_px = 4.0f;

    std::vector<std::uint32_t> pixels(16, 0x12345678u);
    std::string error;
    SdfColorPipelinePostprocessStats stats{};
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, params, pixels.data(), &error, &stats),
        "TestScalarOnlySdfStackUsesDirectSamplesWithoutNeighborhood_PostprocessSucceeds");
    Check(stats.direct_sample_count == 16,
        "TestScalarOnlySdfStackUsesDirectSamplesWithoutNeighborhood_DirectSamplesEveryPixel");
    Check(stats.neighborhood_sample_count == 0,
        "TestScalarOnlySdfStackUsesDirectSamplesWithoutNeighborhood_NoNeighborhoodSamples");
}


void TestDownsampledFieldReducesPostprocessSamplesWithoutChangingFullQualityPixels() {
    SdfFieldResult field = MakeTestField();
    field.pixel_scale = 4.0f;
    RenderSettings render{};
    render.resolution = {8, 8};

    KernelParams params = SdfParams(ColorSignal::sdf_normal_angle, ColorPalette::phase_wheel);
    std::vector<std::uint32_t> pixels(64, 0x12345678u);
    std::string error;
    SdfColorPipelinePostprocessStats stats{};
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, params, pixels.data(), &error, &stats),
        "TestDownsampledFieldReducesPostprocessSamplesWithoutChangingFullQualityPixels_PostprocessSucceeds");
    Check(stats.output_pixel_step == 1,
        "TestDownsampledFieldReducesPostprocessSamplesWithoutChangingFullQualityPixels_DefaultStepOne");
    Check(stats.neighborhood_sample_count == 16,
        "TestDownsampledFieldReducesPostprocessSamplesWithoutChangingFullQualityPixels_SamplesOnePerFieldCell");
    Check(stats.filled_pixel_count == 64,
        "TestDownsampledFieldReducesPostprocessSamplesWithoutChangingFullQualityPixels_FillsEveryRenderPixel");

    RenderSettings fieldResolutionRender{};
    fieldResolutionRender.resolution = {4, 4};
    std::vector<std::uint32_t> fieldPixels(16, 0x12345678u);
    SdfColorPipelinePostprocessStats fieldStats{};
    error.clear();
    Check(ApplyLensSdfColorPipelinePostprocess(
            field.View(),
            fieldResolutionRender,
            params,
            fieldPixels.data(),
            &error,
            &fieldStats),
        "TestDownsampledFieldReducesPostprocessSamplesWithoutChangingFullQualityPixels_FieldReferenceSucceeds");
    std::vector<std::uint32_t> expected(64, 0u);
    for (int y = 0; y < 8; ++y) {
        const int fy = (y * field.height) / 8;
        for (int x = 0; x < 8; ++x) {
            const int fx = (x * field.width) / 8;
            expected[static_cast<std::size_t>(y * 8 + x)] =
                fieldPixels[static_cast<std::size_t>(fy * field.width + fx)];
        }
    }
    Check(pixels == expected,
        "TestDownsampledFieldReducesPostprocessSamplesWithoutChangingFullQualityPixels_ExpandedPixelsMatchReference");
}

void TestDownsampledFieldUnevenRenderSizeMatchesReference() {
    SdfFieldResult field = MakeTestField();
    field.pixel_scale = 2.0f;
    RenderSettings render{};
    render.resolution = {7, 5};

    KernelParams params = SdfParams(ColorSignal::sdf_signed_distance);
    std::vector<std::uint32_t> pixels(35, 0x12345678u);
    std::string error;
    SdfColorPipelinePostprocessStats stats{};
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, params, pixels.data(), &error, &stats),
        "TestDownsampledFieldUnevenRenderSizeMatchesReference_PostprocessSucceeds");
    Check(stats.direct_sample_count == 16,
        "TestDownsampledFieldUnevenRenderSizeMatchesReference_SamplesOnePerFieldCell");
    Check(stats.filled_pixel_count == 35,
        "TestDownsampledFieldUnevenRenderSizeMatchesReference_FillsEveryRenderPixel");

    RenderSettings fieldResolutionRender{};
    fieldResolutionRender.resolution = {4, 4};
    std::vector<std::uint32_t> fieldPixels(16, 0x12345678u);
    error.clear();
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), fieldResolutionRender, params, fieldPixels.data(), &error),
        "TestDownsampledFieldUnevenRenderSizeMatchesReference_FieldReferenceSucceeds");
    std::vector<std::uint32_t> expected(35, 0u);
    for (int y = 0; y < render.resolution.y; ++y) {
        const int fy = (y * field.height) / render.resolution.y;
        for (int x = 0; x < render.resolution.x; ++x) {
            const int fx = (x * field.width) / render.resolution.x;
            expected[static_cast<std::size_t>(y * render.resolution.x + x)] =
                fieldPixels[static_cast<std::size_t>(fy * field.width + fx)];
        }
    }
    Check(pixels == expected,
        "TestDownsampledFieldUnevenRenderSizeMatchesReference_ExpandedPixelsMatchReference");
}

void TestPreviewPixelStepReducesSamplesAndFillsFrame() {
    const SdfFieldResult field = MakeTestField();
    RenderSettings render{};
    render.resolution = {4, 4};

    KernelParams params = SdfParams(ColorSignal::sdf_normal_angle, ColorPalette::phase_wheel);
    std::vector<std::uint32_t> pixels(16, 0x12345678u);
    const std::uint64_t before = HashFrame(pixels);
    std::string error;
    SdfColorPipelinePostprocessStats stats{};
    SdfColorPipelinePostprocessOptions options{};
    options.output_pixel_step = 2;
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, params, pixels.data(), &error, &stats, &options),
        "TestPreviewPixelStepReducesSamplesAndFillsFrame_PostprocessSucceeds");
    Check(stats.output_pixel_step == 2,
        "TestPreviewPixelStepReducesSamplesAndFillsFrame_ReportsStepTwo");
    Check(stats.neighborhood_sample_count == 4,
        "TestPreviewPixelStepReducesSamplesAndFillsFrame_SamplesOnePerBlock");
    Check(stats.filled_pixel_count == 16,
        "TestPreviewPixelStepReducesSamplesAndFillsFrame_FillsEveryPixel");
    Check(HashFrame(pixels) != before,
        "TestPreviewPixelStepReducesSamplesAndFillsFrame_FrameChangesFromSentinel");
}

void TestNormalAngleRequiresNeighborhoodSamples() {
    const SdfFieldResult field = MakeTestField();
    RenderSettings render{};
    render.resolution = {4, 4};

    KernelParams params = SdfParams(ColorSignal::sdf_normal_angle, ColorPalette::phase_wheel);
    std::vector<std::uint32_t> pixels(16, 0x12345678u);
    std::string error;
    SdfColorPipelinePostprocessStats stats{};
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, params, pixels.data(), &error, &stats),
        "TestNormalAngleRequiresNeighborhoodSamples_PostprocessSucceeds");
    Check(stats.direct_sample_count == 0,
        "TestNormalAngleRequiresNeighborhoodSamples_NoDirectOnlySamples");
    Check(stats.neighborhood_sample_count == 16,
        "TestNormalAngleRequiresNeighborhoodSamples_NeighborhoodSamplesEveryPixel");
}

void TestSdfRowSampleStepReducesHeavyRowSourceSamples() {
    const SdfFieldResult field = MakeLargeTestField(8, 8);
    RenderSettings render{};
    render.resolution = {8, 8};

    KernelParams full = SdfParams(ColorSignal::sdf_normal_angle, ColorPalette::phase_wheel);
    full.color_source_stack_count = 2;
    full.color_source_stack[0].signal = ColorSignal::sdf_signed_distance;
    full.color_source_stack[0].params.scale = 0.05f;
    full.color_source_stack[0].params.bias = 0.5f;
    full.color_source_stack[0].params.blend_weight = 1.0f;
    full.color_source_stack[1].signal = ColorSignal::sdf_normal_angle;
    full.color_source_stack[1].params.scale = 1.0f;
    full.color_source_stack[1].params.bias = 0.0f;
    full.color_source_stack[1].params.blend_weight = 0.75f;

    KernelParams coarse = full;
    coarse.color_source_stack[1].params.sdf_sample_step = 4;

    std::vector<std::uint32_t> fullPixels(64, 0x12345678u);
    std::vector<std::uint32_t> coarsePixels(64, 0x12345678u);
    std::string error;
    SdfColorPipelinePostprocessStats fullStats{};
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, full, fullPixels.data(), &error, &fullStats),
        "TestSdfRowSampleStepReducesHeavyRowSourceSamples_FullSucceeds");
    Check(fullStats.source_neighborhood_sample_count == 64,
        "TestSdfRowSampleStepReducesHeavyRowSourceSamples_FullSamplesNormalAngleEveryPixel");

    SdfColorPipelinePostprocessStats coarseStats{};
    error.clear();
    Check(ApplyLensSdfColorPipelinePostprocess(field.View(), render, coarse, coarsePixels.data(), &error, &coarseStats),
        "TestSdfRowSampleStepReducesHeavyRowSourceSamples_CoarseSucceeds");
    Check(coarseStats.source_neighborhood_sample_count <= 4,
        "TestSdfRowSampleStepReducesHeavyRowSourceSamples_CoarseSamplesNormalAngleByBlock");
    Check(coarseStats.source_direct_sample_count == fullStats.source_direct_sample_count,
        "TestSdfRowSampleStepReducesHeavyRowSourceSamples_DirectBaseRowStaysFullResolution");
    Check(coarseStats.filled_pixel_count == 64,
        "TestSdfRowSampleStepReducesHeavyRowSourceSamples_CoarseStillFillsFrame");
    Check(HashFrame(coarsePixels) != HashFrame(fullPixels),
        "TestSdfRowSampleStepReducesHeavyRowSourceSamples_CoarseChangesRequestedQuality");
}

void TestParallelPostprocessMatchesForcedSerialPixelsAndStats() {
    const SdfFieldResult field = MakeLargeTestField(32, 24);
    RenderSettings render{};
    render.resolution = {32, 24};

    KernelParams params = SdfParams(ColorSignal::sdf_normal_angle, ColorPalette::phase_wheel);
    params.color_source_stack_count = 2;
    params.color_source_stack[0].signal = ColorSignal::sdf_normal_angle;
    params.color_source_stack[0].params.scale = 1.25f;
    params.color_source_stack[0].params.bias = 0.1f;
    params.color_source_stack[0].params.blend_weight = 1.0f;
    params.color_source_stack[1].signal = ColorSignal::sdf_curvature;
    params.color_source_stack[1].params.scale = 0.5f;
    params.color_source_stack[1].params.bias = 0.5f;
    params.color_source_stack[1].params.blend_weight = 0.35f;

    std::vector<std::uint32_t> serialPixels(static_cast<std::size_t>(render.resolution.x) * static_cast<std::size_t>(render.resolution.y), 0x12345678u);
    std::vector<std::uint32_t> parallelPixels = serialPixels;
    std::string error;
    SdfColorPipelinePostprocessStats serialStats{};
    SdfColorPipelinePostprocessOptions serialOptions{};
    serialOptions.max_worker_threads = 1;
    Check(ApplyLensSdfColorPipelinePostprocess(
            field.View(), render, params, serialPixels.data(), &error, &serialStats, &serialOptions),
        "TestParallelPostprocessMatchesForcedSerialPixelsAndStats_SerialSucceeds");
    Check(serialStats.worker_count == 1,
        "TestParallelPostprocessMatchesForcedSerialPixelsAndStats_SerialWorkerCountOne");

    SdfColorPipelinePostprocessStats parallelStats{};
    SdfColorPipelinePostprocessOptions parallelOptions{};
    parallelOptions.max_worker_threads = 4;
    error.clear();
    Check(ApplyLensSdfColorPipelinePostprocess(
            field.View(), render, params, parallelPixels.data(), &error, &parallelStats, &parallelOptions),
        "TestParallelPostprocessMatchesForcedSerialPixelsAndStats_ParallelSucceeds");
    Check(parallelStats.worker_count > 1 && parallelStats.worker_count <= 4,
        "TestParallelPostprocessMatchesForcedSerialPixelsAndStats_ParallelWorkerCountBounded");
    Check(parallelPixels == serialPixels,
        "TestParallelPostprocessMatchesForcedSerialPixelsAndStats_ParallelPixelsMatchSerial");
    Check(parallelStats.direct_sample_count == serialStats.direct_sample_count &&
            parallelStats.neighborhood_sample_count == serialStats.neighborhood_sample_count &&
            parallelStats.filled_pixel_count == serialStats.filled_pixel_count,
        "TestParallelPostprocessMatchesForcedSerialPixelsAndStats_ParallelStatsMatchSerial");
}

void TestPreviewPixelStepPolicyKeepsScalarDirectStacksFullResolution() {
    KernelParams scalar = SdfParams(ColorSignal::sdf_signed_distance);
    Check(ResolveSdfColorPipelinePostprocessOutputPixelStep(scalar, true, 0.25, false) == 1,
        "TestPreviewPixelStepPolicyKeepsScalarDirectStacksFullResolution_SignedDistanceStepOne");

    KernelParams scalarStack = SdfParams(ColorSignal::sdf_signed_distance);
    scalarStack.color_source_stack_count = 3;
    scalarStack.color_source_stack[1].signal = ColorSignal::sdf_inside_outside;
    scalarStack.color_source_stack[1].params.blend_weight = 0.5f;
    scalarStack.color_source_stack[2].signal = ColorSignal::sdf_boundary_band;
    scalarStack.color_source_stack[2].params.blend_weight = 0.5f;
    scalarStack.color_source_stack[2].params.sdf_boundary_width_px = 4.0f;
    Check(ResolveSdfColorPipelinePostprocessOutputPixelStep(scalarStack, true, 0.25, false) == 1,
        "TestPreviewPixelStepPolicyKeepsScalarDirectStacksFullResolution_ScalarStackStepOne");

    KernelParams heavy = SdfParams(ColorSignal::sdf_normal_angle, ColorPalette::phase_wheel);
    Check(ResolveSdfColorPipelinePostprocessOutputPixelStep(heavy, true, 0.25, false) == 4,
        "TestPreviewPixelStepPolicyKeepsScalarDirectStacksFullResolution_NormalAngleCanStepFour");
    Check(ResolveSdfColorPipelinePostprocessOutputPixelStep(heavy, true, 0.50, false) == 2,
        "TestPreviewPixelStepPolicyKeepsScalarDirectStacksFullResolution_NormalAngleCanStepTwo");
    Check(ResolveSdfColorPipelinePostprocessOutputPixelStep(heavy, true, 0.25, true) == 1,
        "TestPreviewPixelStepPolicyKeepsScalarDirectStacksFullResolution_ForceFullQualityStepOne");
    Check(ResolveSdfColorPipelinePostprocessOutputPixelStep(heavy, false, 0.25, false) == 1,
        "TestPreviewPixelStepPolicyKeepsScalarDirectStacksFullResolution_NoPreviewStepOne");
}

} // namespace

int main() {
    TestEachSdfSourceSignalPostprocessesFrame();
    TestBoundaryBandWidthChangesPostprocessFrame();
    TestLensFieldV2ResponseDoesNotAliasRawSignedDistance();
    TestLensFieldV2SignContrastChangesFrameAndZeroPreservesLegacyResponse();
    TestLensFieldV2ResponseHonorsFieldPixelScaleWithoutChangingRawSignedDistance();
    TestSdfSourceDistinctnessMatrixRejectsRawSignedDistanceAliases();
    TestNormalAnglePhaseOffsetChangesFrameWithoutReclassifyingScalarSdfSources();
    TestNormalAngleBoundaryGateMasksFullFieldDiagnostic();
    TestNormalAngleGateWidthIsInactiveWhenGateIsOff();
    TestMixedSourceStackFailsClosed();
    TestBoundaryBandWidthStillAffectsMixedSdfStack();
    TestScalarOnlySdfStackUsesDirectSamplesWithoutNeighborhood();
    TestNormalAngleRequiresNeighborhoodSamples();
    TestDownsampledFieldReducesPostprocessSamplesWithoutChangingFullQualityPixels();
    TestDownsampledFieldUnevenRenderSizeMatchesReference();
    TestPreviewPixelStepReducesSamplesAndFillsFrame();
    TestSdfRowSampleStepReducesHeavyRowSourceSamples();
    TestParallelPostprocessMatchesForcedSerialPixelsAndStats();
    TestPreviewPixelStepPolicyKeepsScalarDirectStacksFullResolution();

    std::printf("test_color_pipeline_sdf_postprocess: passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
