#include "../src/color_pipeline_loaded_draft.h"

#define COLOR_PIPELINE_WINDOW_NO_IMGUI
#include "../src/color_pipeline_window.h"
#undef COLOR_PIPELINE_WINDOW_NO_IMGUI

#include <cmath>
#include <cstdio>
#include <string>

namespace {

int gPassed = 0;
int gFailed = 0;

void Check(bool condition, const char* message) {
    if (condition) {
        ++gPassed;
    } else {
        ++gFailed;
        std::printf("  FAIL: %s\n", message);
    }
}

bool Near(double actual, double expected, double eps = 1.0e-6) {
    return std::fabs(actual - expected) <= eps;
}

KernelParams SmoothEscapeParams() {
    KernelParams params{};
    params.coloring_mode = ColoringMode::smooth_escape;
    params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
    return params;
}

KernelParams FixtureFParams() {
    KernelParams params{};
    params.coloring_mode = ColoringMode::iteration_bands;
    params.color_pipeline = {
        ColorSignal::iteration_bands,
        ColorPalette::banded_escape,
        ColorGradingPreset::balance_void_default,
    };

    params.color_source_stack_count = 1;
    params.color_source_stack[0].signal = ColorSignal::iteration_bands;
    params.color_source_stack[0].params.band_count = 16;
    params.color_source_stack[0].params.softness = 1.0f;
    params.color_source_stack[0].params.blend_weight = 1.0f;
    params.color_iteration_band_count = 16;
    params.color_iteration_band_softness = 1.0f;

    params.color_shape = ColorPipelineShape::offset_scale;
    params.color_shape_stack_count = 1;
    params.color_shape_stack[0].shape = ColorPipelineShape::offset_scale;
    params.color_shape_stack[0].params.offset = 0.30939f;
    params.color_shape_stack[0].params.scale = 4.46989f;
    params.color_shape_offset = 0.30939f;
    params.color_shape_scale = 4.46989f;

    params.color_palette_stack_count = 1;
    params.color_palette_stack[0].palette = ColorPalette::banded_escape;
    params.color_palette_stack[0].params.band_emphasis = 1.5f;
    params.color_palette_stack[0].params.phase_offset = 0.0f;
    params.color_palette_stack[0].params.blend_weight = 1.0f;
    params.color_palette_stack[0].params.blend_mode = ColorPaletteBlendMode::normal;
    params.color_iteration_band_emphasis = 1.5f;
    params.color_iteration_band_palette_offset = 0.0f;

    params.color_grading_stack_count = 1;
    params.color_grading_stack[0].grading = ColorGradingPreset::balance_void_default;
    params.color_grading_stack[0].params.balance_void = 0.5f;
    params.color_grading_stack[0].params.chroma_tension = -0.3f;
    params.color_grading_stack[0].params.accent_bias = -0.67403f;
    params.color_balance_void = 0.5f;
    params.color_chroma_tension = -0.3f;
    params.color_accent_bias = -0.67403f;
    return params;
}

void TestAppliesLoadedDraftThroughExistingRuntimeAuthority() {
    KernelParams params = SmoothEscapeParams();
    ColorPipelineWindowState draft{};
    Check(SyncColorPipelineWindowFromLiveState(&draft, FractalType::mandelbrot, &params),
        "TestApply_SyncsBaseDraft");
    Check(SelectColorPipelineLaneFunction(&draft, 3, "neutral_finish"),
        "TestApply_SelectsNeutralFinish");
    Check(SetColorPipelineParamNumber(&draft.lanes[3].rows[0], "grade.saturation", 0.25),
        "TestApply_ChangesDraftSaturation");

    ColorPipelineLoadedDraftApplyResult result{};
    std::string error;
    Check(ApplyLoadedColorPipelineDraftToRuntime(
            &draft, FractalType::mandelbrot, &params, &result, &error),
        "TestApply_Succeeds");
    Check(error.empty(), "TestApply_LeavesNoError");
    Check(result.changed, "TestApply_ReportsChanged");
    Check(params.color_pipeline.grading == ColorGradingPreset::neutral_default,
        "TestApply_ChangesLiveGrading");
    Check(Near(params.color_saturation, 0.25),
        "TestApply_ChangesLiveSaturation");
    Check(params.color_grading_stack_count == 1 &&
            Near(params.color_grading_stack[0].params.saturation, 0.25),
        "TestApply_ChangesLiveGradingStack");
    Check(!HasColorPipelineDraftEdits(draft),
        "TestApply_ResyncsDraftToEffectiveRuntime");

    result.changed = true;
    Check(ApplyLoadedColorPipelineDraftToRuntime(
            &draft, FractalType::mandelbrot, &params, &result, &error),
        "TestApply_MatchingDraftSucceeds");
    Check(!result.changed, "TestApply_MatchingDraftReportsUnchanged");
}

void TestLoadedDraftNormalizesOnceToExactFloatOwner() {
    KernelParams params = SmoothEscapeParams();
    ColorPipelineWindowState draft{};
    Check(SyncColorPipelineWindowFromLiveState(&draft, FractalType::mandelbrot, &params),
        "TestFloatOwner_SyncsBaseDraft");
    Check(SelectColorPipelineLaneFunction(&draft, 3, "balance_void_grade"),
        "TestFloatOwner_SelectsBalanceVoidGrade");

    constexpr double requested = 0.123456789012345;
    const float expectedRuntime = static_cast<float>(requested);
    Check(SetColorPipelineParamNumber(&draft.lanes[3].rows[0], "grade.balance_void", requested),
        "TestFloatOwner_SetsOverPreciseDraftValue");

    ColorPipelineLoadedDraftApplyResult result{};
    std::string error;
    Check(ApplyLoadedColorPipelineDraftToRuntime(
            &draft, FractalType::mandelbrot, &params, &result, &error),
        "TestFloatOwner_AppliesDraft");
    Check(params.color_balance_void == expectedRuntime,
        "TestFloatOwner_RuntimeStoresBinary32Value");

    double emitted = 0.0;
    Check(TryGetColorPipelineParamNumber(
            draft.lanes[3].rows[0], "grade.balance_void", &emitted, &error),
        "TestFloatOwner_ReadsSynchronizedDraft");
    Check(emitted == static_cast<double>(expectedRuntime),
        "TestFloatOwner_SynchronizedDraftReportsExactBinary32Value");
    Check(emitted != requested,
        "TestFloatOwner_SynchronizedDraftDoesNotPreserveDiscardedDraftDigits");

    const double emittedBeforeReplay = emitted;
    const float runtimeBeforeReplay = params.color_balance_void;
    ColorPipelineLoadedDraftApplyResult replayResult{};
    Check(ApplyLoadedColorPipelineDraftToRuntime(
            &draft, FractalType::mandelbrot, &params, &replayResult, &error),
        "TestFloatOwner_ReappliesNormalizedDraft");
    emitted = 0.0;
    Check(TryGetColorPipelineParamNumber(
            draft.lanes[3].rows[0], "grade.balance_void", &emitted, &error) &&
            emitted == emittedBeforeReplay &&
            params.color_balance_void == runtimeBeforeReplay,
        "TestFloatOwner_NormalizedRuntimeAndDraftDoNotDriftAgain");
}

void TestAppliesFixtureFFourLaneCompositionWithAuthoritativeReadback() {
    KernelParams params = FixtureFParams();
    ColorPipelineWindowState draft{};
    Check(EnsureColorPipelineWindowInitialized(&draft),
        "TestFixtureF_InitializesDraft");
    Check(SelectColorPipelineLaneFunction(&draft, 0, "banded_signal"),
        "TestFixtureF_SelectsSource");
    Check(SetColorPipelineParamNumber(&draft.lanes[0].rows[0], "signal.band_count", 16.0) &&
            SetColorPipelineParamNumber(&draft.lanes[0].rows[0], "signal.softness", 1.0) &&
            SetColorPipelineParamNumber(&draft.lanes[0].rows[0], "signal.blend_weight", 1.0),
        "TestFixtureF_ConfiguresSource");
    Check(SelectColorPipelineLaneFunction(&draft, 1, "offset_scale") &&
            SetColorPipelineParamNumber(&draft.lanes[1].rows[0], "shape.offset", 0.30939) &&
            SetColorPipelineParamNumber(&draft.lanes[1].rows[0], "shape.scale", 4.46989),
        "TestFixtureF_ConfiguresShape");
    Check(SelectColorPipelineLaneFunction(&draft, 2, "banded_heatmap") &&
            SetColorPipelineParamNumber(&draft.lanes[2].rows[0], "palette.band_emphasis", 1.8) &&
            SetColorPipelineParamNumber(&draft.lanes[2].rows[0], "palette.phase_offset", 0.0) &&
            SetColorPipelineParamNumber(&draft.lanes[2].rows[0], "palette.blend_weight", 1.0) &&
            SetColorPipelineParamEnum(&draft.lanes[2].rows[0], "palette.blend_mode", "normal"),
        "TestFixtureF_ConfiguresPalette");
    Check(SelectColorPipelineLaneFunction(&draft, 3, "balance_void_grade") &&
            SetColorPipelineParamNumber(&draft.lanes[3].rows[0], "grade.balance_void", 0.5) &&
            SetColorPipelineParamNumber(&draft.lanes[3].rows[0], "grade.chroma_tension", -0.3) &&
            SetColorPipelineParamNumber(&draft.lanes[3].rows[0], "grade.accent_bias", -0.67403),
        "TestFixtureF_ConfiguresGrading");

    ColorPipelineWindowState probeDraft = draft;
    KernelParams probeParams = params;
    bool probeChanged = false;
    Check(ApplyColorPipelineDraftToLiveState(
            &probeDraft, FractalType::explaino_balance_void, &probeParams, &probeChanged),
        "TestFixtureF_AppliesThroughSharedDraftOwner");
    ColorPipelineLaneState probeLane;
    bool importSupported = false;
    std::string probeError;
    Check(TryBuildColorPipelineSourceLaneFromLive(
            probeParams, &probeLane, &importSupported, &probeError) && importSupported,
        "TestFixtureF_SourceReadbackSupported");
    probeLane = {};
    importSupported = false;
    Check(TryBuildColorPipelineShapeLaneFromLive(
            probeParams, &probeLane, &importSupported, &probeError) && importSupported,
        "TestFixtureF_ShapeReadbackSupported");
    probeLane = {};
    importSupported = false;
    Check(TryBuildColorPipelinePaletteLaneFromLive(
            probeParams, &probeLane, &importSupported, &probeError) && importSupported,
        "TestFixtureF_PaletteReadbackSupported");
    probeLane = {};
    importSupported = false;
    Check(TryBuildColorPipelineGradingLaneFromLive(
            probeParams, &probeLane, &importSupported, &probeError) && importSupported,
        "TestFixtureF_GradingReadbackSupported");

    ColorPipelineLoadedDraftApplyResult result{};
    std::string error;
    const bool applied = ApplyLoadedColorPipelineDraftToRuntime(
            &draft,
            FractalType::explaino_balance_void,
            &params,
            &result,
            &error);
    if (!applied) {
        std::printf("  Fixture F apply error: %s\n", error.c_str());
    }
    Check(applied,
        "TestFixtureF_AppliesWithAuthoritativeReadback");
    Check(error.empty(), "TestFixtureF_LeavesNoError");
    Check(result.changed, "TestFixtureF_ReportsChanged");
    Check(params.color_palette_stack_count == 1 &&
            params.color_palette_stack[0].palette == ColorPalette::banded_escape &&
            params.color_palette_stack[0].params.band_emphasis == 1.8f,
        "TestFixtureF_ChangesPaletteStackRuntimeOwner");
    Check(params.color_iteration_band_emphasis == 1.8f,
        "TestFixtureF_ChangesFlatPaletteMirror");
    double emitted = 0.0;
    Check(TryGetColorPipelineParamNumber(
            draft.lanes[2].rows[0], "palette.band_emphasis", &emitted, &error) &&
            emitted == static_cast<double>(1.8f),
        "TestFixtureF_ReadsBackExactNormalizedRuntimeValue");
    Check(!HasColorPipelineDraftEdits(draft),
        "TestFixtureF_ResyncsDraftToEffectiveRuntime");
}

void TestInvalidLoadedDraftFailsClosedWithValidationDetail() {
    KernelParams params = SmoothEscapeParams();
    ColorPipelineWindowState draft{};
    Check(SyncColorPipelineWindowFromLiveState(&draft, FractalType::mandelbrot, &params),
        "TestInvalid_SyncsBaseDraft");
    Check(SelectColorPipelineLaneFunction(&draft, 1, "offset_scale"),
        "TestInvalid_SelectsShape");
    Check(SetColorPipelineParamNumber(&draft.lanes[1].rows[0], "shape.scale", 0.01),
        "TestInvalid_SetsOutOfRangeScale");

    draft.validation_messages.push_back("stale validation from an earlier operation");

    ColorPipelineLoadedDraftApplyResult result{};
    std::string error;
    Check(!ApplyLoadedColorPipelineDraftToRuntime(
            &draft, FractalType::mandelbrot, &params, &result, &error),
        "TestInvalid_Fails");
    Check(!result.changed, "TestInvalid_DoesNotReportChanged");
    Check(!error.empty(), "TestInvalid_ReportsEngineValidationDetail");
    Check(error.find("stale validation") == std::string::npos,
        "TestInvalid_DoesNotReportStaleValidationDetail");
    Check(params.color_shape == ColorPipelineShape::identity,
        "TestInvalid_DoesNotMutateRuntime");
}

} // namespace

int main() {
    TestAppliesLoadedDraftThroughExistingRuntimeAuthority();
    TestLoadedDraftNormalizesOnceToExactFloatOwner();
    TestAppliesFixtureFFourLaneCompositionWithAuthoritativeReadback();
    TestInvalidLoadedDraftFailsClosedWithValidationDetail();
    std::printf("test_color_pipeline_loaded_draft: %d passed, %d failed\n", gPassed, gFailed);
    return gFailed > 0 ? 1 : 0;
}
