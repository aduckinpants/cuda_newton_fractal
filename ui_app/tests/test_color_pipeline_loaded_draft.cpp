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
    TestInvalidLoadedDraftFailsClosedWithValidationDetail();
    std::printf("test_color_pipeline_loaded_draft: %d passed, %d failed\n", gPassed, gFailed);
    return gFailed > 0 ? 1 : 0;
}
