#include "viewer_cli.h"

#include <cstdio>
#include <string>

static int g_passed = 0;
static int g_failed = 0;

static void Check(bool condition, const char* name) {
    if (condition) {
        std::printf("  PASS: %s\n", name);
        ++g_passed;
    } else {
        std::printf("  FAIL: %s\n", name);
        ++g_failed;
    }
}

static ViewerCliArgs AddColorPipelineProofAction(ViewerCliArgs cli) {
    ColorPipelineHeadlessSelectFunctionAction action;
    action.lane_id = "source";
    action.row_index = 0;
    action.function_id = "root_proximity";
    cli.color_pipeline_headless_proof.actions.push_back(action);
    return cli;
}

static void TestMainCliConflictValidationAllowsDefaultViewerMode() {
    ViewerCliArgs cli;
    Check(ValidateViewerCliModeConflicts(cli), "TestMainCliConflictValidationAllowsDefaultViewerMode_DefaultAllowed");
}

static void TestMainCliConflictValidationRejectsRuntimeWalkViewerDoubleSource() {
    ViewerCliArgs cli;
    cli.have_runtime_walk_viewer_request_json = true;
    cli.have_runtime_walk_viewer_fits_path = true;
    Check(!ValidateViewerCliModeConflicts(cli), "TestMainCliConflictValidationRejectsRuntimeWalkViewerDoubleSource_Blocked");
}

static void TestMainCliConflictValidationRejectsDualCaptureModes() {
    ViewerCliArgs cli;
    cli.capture_diagnostic_only = true;
    cli.capture_finding_only = true;
    Check(!ValidateViewerCliModeConflicts(cli), "TestMainCliConflictValidationRejectsDualCaptureModes_Blocked");
}

static void TestMainCliConflictValidationRequiresCaptureForColorPipelineProof() {
    ViewerCliArgs withoutCapture = AddColorPipelineProofAction(ViewerCliArgs{});
    Check(!ValidateViewerCliModeConflicts(withoutCapture), "TestMainCliConflictValidationRequiresCaptureForColorPipelineProof_NoCaptureBlocked");

    ViewerCliArgs withDiagnosticCapture = AddColorPipelineProofAction(ViewerCliArgs{});
    withDiagnosticCapture.capture_diagnostic_only = true;
    Check(ValidateViewerCliModeConflicts(withDiagnosticCapture), "TestMainCliConflictValidationRequiresCaptureForColorPipelineProof_DiagnosticAllowed");
}

static void TestMainCliConflictValidationRejectsValidateUiCaptureMix() {
    ViewerCliArgs cli;
    cli.validate_ui_only = true;
    cli.capture_finding_only = true;
    Check(!ValidateViewerCliModeConflicts(cli), "TestMainCliConflictValidationRejectsValidateUiCaptureMix_Blocked");
}

static void TestMainCliConflictValidationRejectsExploreAndFlashlightMix() {
    ViewerCliArgs cli;
    cli.explore_recommend = true;
    cli.flashlight_probe = true;
    Check(!ValidateViewerCliModeConflicts(cli), "TestMainCliConflictValidationRejectsExploreAndFlashlightMix_Blocked");
}

static void TestMainCliConflictValidationRejectsRuntimeWalkMixedModes() {
    ViewerCliArgs cli;
    cli.have_runtime_walk_request_json = true;
    cli.have_runtime_walk_viewer_fits_path = true;
    Check(!ValidateViewerCliModeConflicts(cli), "TestMainCliConflictValidationRejectsRuntimeWalkMixedModes_Blocked");
}

static void TestMainBuildsSampleModeArgsFromCli() {
    ViewerCliArgs cli;
    cli.sample_request_stdin = true;
    cli.sample_response_stdout = true;
    cli.have_sample_request_json = true;
    cli.sample_request_json_path = "request.json";
    cli.have_sample_response_json = true;
    cli.sample_response_json_path = "response.json";
    cli.validate_ui_only = true;
    cli.capture_diagnostic_only = true;
    cli.capture_finding_only = false;

    const SampleModeArgs args = BuildViewerCliSampleModeArgs(cli);
    Check(args.request_stdin, "TestMainBuildsSampleModeArgsFromCli_RequestStdin");
    Check(args.response_stdout, "TestMainBuildsSampleModeArgsFromCli_ResponseStdout");
    Check(args.request_json_path == "request.json", "TestMainBuildsSampleModeArgsFromCli_RequestPath");
    Check(args.response_json_path == "response.json", "TestMainBuildsSampleModeArgsFromCli_ResponsePath");
    Check(args.conflict_validate_ui, "TestMainBuildsSampleModeArgsFromCli_ValidateConflict");
    Check(args.conflict_capture_diagnostic, "TestMainBuildsSampleModeArgsFromCli_DiagnosticConflict");
    Check(!args.conflict_capture_finding, "TestMainBuildsSampleModeArgsFromCli_FindingConflict");
}

int main() {
    TestMainCliConflictValidationAllowsDefaultViewerMode();
    TestMainCliConflictValidationRejectsRuntimeWalkViewerDoubleSource();
    TestMainCliConflictValidationRejectsDualCaptureModes();
    TestMainCliConflictValidationRequiresCaptureForColorPipelineProof();
    TestMainCliConflictValidationRejectsValidateUiCaptureMix();
    TestMainCliConflictValidationRejectsExploreAndFlashlightMix();
    TestMainCliConflictValidationRejectsRuntimeWalkMixedModes();
    TestMainBuildsSampleModeArgsFromCli();

    std::printf("test_main: passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
