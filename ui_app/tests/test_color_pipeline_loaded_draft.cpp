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
    TestInvalidLoadedDraftFailsClosedWithValidationDetail();
    std::printf("test_color_pipeline_loaded_draft: %d passed, %d failed\n", gPassed, gFailed);
    return gFailed > 0 ? 1 : 0;
}
