#include "viewer_cli.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>

static int g_passed = 0;
static int g_failed = 0;

static void Check(bool cond, const char* name) {
    if (cond) {
        std::printf("  PASS: %s\n", name);
        ++g_passed;
    } else {
        std::printf("  FAIL: %s\n", name);
        ++g_failed;
    }
}

static std::vector<std::string> Args(std::initializer_list<const char*> list) {
    std::vector<std::string> result;
    result.push_back("fractal_ui.exe"); // argv[0]
    for (auto s : list) result.push_back(s);
    return result;
}

static void TestDefaultsNoArgs() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({}), &cli);
    Check(rc == 0, "TestDefaultsNoArgs_ReturnCode");
    Check(!cli.validate_ui_only, "TestDefaultsNoArgs_ValidateUi");
    Check(!cli.capture_diagnostic_only, "TestDefaultsNoArgs_CaptureDiag");
    Check(!cli.capture_finding_only, "TestDefaultsNoArgs_CaptureFinding");
    Check(!cli.describe_functions, "TestDefaultsNoArgs_DescribeFunctions");
    Check(!cli.have_fractal_type, "TestDefaultsNoArgs_FractalType");
    Check(!cli.have_explaino_seed, "TestDefaultsNoArgs_ExplainoSeed");
    Check(!cli.have_width, "TestDefaultsNoArgs_Width");
    Check(!cli.have_height, "TestDefaultsNoArgs_Height");
    Check(!cli.sweep_config.enabled, "TestDefaultsNoArgs_SweepDisabled");
    Check(!cli.any_sample_mode_arg, "TestDefaultsNoArgs_NotSampleMode");
}

static void TestValidateUi() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--validate-ui"}), &cli);
    Check(rc == 0, "TestValidateUi_ReturnCode");
    Check(cli.validate_ui_only, "TestValidateUi_Flag");
}

static void TestFractalType() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--fractal-type", "mandelbrot"}), &cli);
    Check(rc == 0, "TestFractalType_ReturnCode");
    Check(cli.have_fractal_type, "TestFractalType_Have");
    Check(cli.fractal_type == FractalType::mandelbrot, "TestFractalType_Value");
}

static void TestFractalTypeMissingValue() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--fractal-type"}), &cli);
    Check(rc != 0, "TestFractalTypeMissingValue_Fails");
}

static void TestFractalTypeUnknown() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--fractal-type", "nosuchfractal"}), &cli);
    Check(rc != 0, "TestFractalTypeUnknown_Fails");
}

static void TestExplainoSeed() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--explaino-seed", "42.5"}), &cli);
    Check(rc == 0, "TestExplainoSeed_ReturnCode");
    Check(cli.have_explaino_seed, "TestExplainoSeed_Have");
    Check(std::fabs(cli.explaino_seed - 42.5) < 1e-9, "TestExplainoSeed_Value");
}

static void TestExplainoSeedBadValue() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--explaino-seed", "notanumber"}), &cli);
    Check(rc != 0, "TestExplainoSeedBadValue_Fails");
}

static void TestExplainoSeedMissing() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--explaino-seed"}), &cli);
    Check(rc != 0, "TestExplainoSeedMissing_Fails");
}

static void TestExplainoPhase() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--explaino-phase", "1.5"}), &cli);
    Check(rc == 0, "TestExplainoPhase_ReturnCode");
    Check(cli.have_explaino_phase, "TestExplainoPhase_Have");
    Check(std::fabs(cli.explaino_phase - 1.5) < 1e-9, "TestExplainoPhase_Value");
}

static void TestResolutionOverrides() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--width", "800", "--height", "600"}), &cli);
    Check(rc == 0, "TestResolutionOverrides_ReturnCode");
    Check(cli.have_width && cli.width == 800, "TestResolutionOverrides_Width");
    Check(cli.have_height && cli.height == 600, "TestResolutionOverrides_Height");
}

static void TestResolutionBadWidth() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--width", "xyz"}), &cli);
    Check(rc != 0, "TestResolutionBadWidth_Fails");
}

static void TestSweepConfigComplete() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({
        "--sweep-seed-start", "1.0",
        "--sweep-seed-stop", "5.0",
        "--sweep-seed-step", "0.5",
        "--sweep-dwell-ms", "200",
        "--sweep-loop"
    }), &cli);
    Check(rc == 0, "TestSweepConfigComplete_ReturnCode");
    Check(cli.sweep_config.enabled, "TestSweepConfigComplete_Enabled");
    Check(std::fabs(cli.sweep_config.seed_start - 1.0) < 1e-9, "TestSweepConfigComplete_Start");
    Check(std::fabs(cli.sweep_config.seed_stop - 5.0) < 1e-9, "TestSweepConfigComplete_Stop");
    Check(std::fabs(cli.sweep_config.seed_step - 0.5) < 1e-9, "TestSweepConfigComplete_Step");
    Check(std::fabs(cli.sweep_config.dwell_seconds - 0.2) < 1e-9, "TestSweepConfigComplete_Dwell");
    Check(cli.sweep_config.loop, "TestSweepConfigComplete_Loop");
}

static void TestSweepConfigIncomplete() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({
        "--sweep-seed-start", "1.0",
        "--sweep-seed-stop", "5.0"
        // Missing --sweep-seed-step
    }), &cli);
    Check(rc != 0, "TestSweepConfigIncomplete_Fails");
}

static void TestSweepConfigZeroStepFails() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({
        "--sweep-seed-start", "1.0",
        "--sweep-seed-stop", "5.0",
        "--sweep-seed-step", "0.0"
    }), &cli);
    Check(rc != 0, "TestSweepConfigZeroStepFails_Fails");
}

static void TestSweepConfigNegativeStepFails() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({
        "--sweep-seed-start", "1.0",
        "--sweep-seed-stop", "5.0",
        "--sweep-seed-step", "-0.5"
    }), &cli);
    Check(rc != 0, "TestSweepConfigNegativeStepFails_Fails");
}

static void TestSweepConfigEqualBoundsFails() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({
        "--sweep-seed-start", "3.0",
        "--sweep-seed-stop", "3.0",
        "--sweep-seed-step", "0.5"
    }), &cli);
    Check(rc != 0, "TestSweepConfigEqualBoundsFails_Fails");
}

static void TestSweepConfigDescendingBoundsFails() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({
        "--sweep-seed-start", "5.0",
        "--sweep-seed-stop", "1.0",
        "--sweep-seed-step", "0.5"
    }), &cli);
    Check(rc != 0, "TestSweepConfigDescendingBoundsFails_Fails");
}

static void TestSampleModeStdio() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({
        "--sample-request-stdin",
        "--sample-response-stdout"
    }), &cli);
    Check(rc == 0, "TestSampleModeStdio_ReturnCode");
    Check(cli.sample_request_stdin, "TestSampleModeStdio_Stdin");
    Check(cli.sample_response_stdout, "TestSampleModeStdio_Stdout");
    Check(cli.any_sample_mode_arg, "TestSampleModeStdio_AnySampleMode");
}

static void TestSampleModeJsonPaths() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({
        "--sample-request-json", "req.json",
        "--sample-response-json", "resp.json"
    }), &cli);
    Check(rc == 0, "TestSampleModeJsonPaths_ReturnCode");
    Check(cli.have_sample_request_json, "TestSampleModeJsonPaths_HaveReq");
    Check(cli.sample_request_json_path == "req.json", "TestSampleModeJsonPaths_ReqPath");
    Check(cli.have_sample_response_json, "TestSampleModeJsonPaths_HaveResp");
    Check(cli.sample_response_json_path == "resp.json", "TestSampleModeJsonPaths_RespPath");
    Check(cli.any_sample_mode_arg, "TestSampleModeJsonPaths_AnySampleMode");
}

static void TestDescribeFunctionsJson() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({
        "--describe-functions-json", "out.json"
    }), &cli);
    Check(rc == 0, "TestDescribeFunctionsJson_ReturnCode");
    Check(cli.have_describe_functions_json, "TestDescribeFunctionsJson_Have");
    Check(cli.describe_functions_json_path == "out.json", "TestDescribeFunctionsJson_Path");
}

static void TestExploreRecommendJson() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({
        "--explore-recommend-json", "advisor.json"
    }), &cli);
    Check(rc == 0, "TestExploreRecommendJson_ReturnCode");
    Check(cli.have_explore_recommend_json, "TestExploreRecommendJson_Have");
    Check(cli.explore_recommend_json_path == "advisor.json", "TestExploreRecommendJson_Path");
}

static void TestExploreRecommendStdout() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({
        "--explore-recommend"
    }), &cli);
    Check(rc == 0, "TestExploreRecommendStdout_ReturnCode");
    Check(cli.explore_recommend, "TestExploreRecommendStdout_Flag");
}

static void TestFlashlightProbe() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({
        "--flashlight-probe", "seed.txt",
        "--flashlight-ticks", "9",
        "--flashlight-radius", "0.61",
        "--flashlight-zoom-radius", "0.19",
        "--flashlight-warp", "0.3",
        "--flashlight-fractal-type", "explaino_fp",
        "--flashlight-closure-last"
    }), &cli);
    Check(rc == 0, "TestFlashlightProbe_ReturnCode");
    Check(cli.flashlight_probe, "TestFlashlightProbe_Flag");
    Check(cli.have_flashlight_probe_path, "TestFlashlightProbe_HavePath");
    Check(cli.flashlight_probe_path == "seed.txt", "TestFlashlightProbe_Path");
    Check(cli.have_flashlight_ticks && cli.flashlight_ticks == 9, "TestFlashlightProbe_Ticks");
    Check(cli.have_flashlight_radius && std::fabs(cli.flashlight_radius - 0.61) < 1e-9, "TestFlashlightProbe_Radius");
    Check(cli.have_flashlight_zoom_radius && std::fabs(cli.flashlight_zoom_radius - 0.19) < 1e-9, "TestFlashlightProbe_ZoomRadius");
    Check(cli.have_flashlight_warp && std::fabs(cli.flashlight_warp - 0.3) < 1e-9, "TestFlashlightProbe_Warp");
    Check(cli.have_flashlight_fractal_type && cli.flashlight_fractal_type == FractalType::explaino_fp, "TestFlashlightProbe_FractalType");
    Check(cli.flashlight_closure_last, "TestFlashlightProbe_Closure");
}

static void TestFlashlightProbeMissingValue() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--flashlight-probe"}), &cli);
    Check(rc != 0, "TestFlashlightProbeMissingValue_Fails");
}

static void TestFlashlightProbeBadFractalType() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({
        "--flashlight-probe", "seed.txt",
        "--flashlight-fractal-type", "not_a_fractal"
    }), &cli);
    Check(rc != 0, "TestFlashlightProbeBadFractalType_Fails");
}

static void TestExploreRecommendJsonMissingValue() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--explore-recommend-json"}), &cli);
    Check(rc != 0, "TestExploreRecommendJsonMissingValue_Fails");
}

static void TestSampleSession() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({ "--sample-session" }), &cli);
    Check(rc == 0, "TestSampleSession_ReturnCode");
    Check(cli.sample_session, "TestSampleSession_Flag");
    // --sample-session is independent of the legacy sample mode args
    Check(!cli.any_sample_mode_arg, "TestSampleSession_NotAnySampleMode");
}

static void TestSampleSessionPipe() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({
        "--sample-session",
        "--sample-session-pipe", "session_pipe_a"
    }), &cli);
    Check(rc == 0, "TestSampleSessionPipe_ReturnCode");
    Check(cli.sample_session, "TestSampleSessionPipe_Session");
    Check(cli.have_sample_session_pipe, "TestSampleSessionPipe_HavePipe");
    Check(cli.sample_session_pipe_name == "session_pipe_a", "TestSampleSessionPipe_Name");
    Check(!cli.any_sample_mode_arg, "TestSampleSessionPipe_NotAnySampleMode");
}

static void TestSampleSessionPipeRequiresSession() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({
        "--sample-session-pipe", "session_pipe_a"
    }), &cli);
    Check(rc != 0, "TestSampleSessionPipeRequiresSession_Fails");
}

static void TestSampleSessionPipeMissingValue() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({
        "--sample-session",
        "--sample-session-pipe"
    }), &cli);
    Check(rc != 0, "TestSampleSessionPipeMissingValue_Fails");
}

static void TestSampleSessionConflictsWithSampleRequest() {
    // --sample-session + --sample-request-stdin should both parse, but WinMain validates conflicts
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({ "--sample-session", "--sample-request-stdin" }), &cli);
    Check(rc == 0, "TestSampleSessionConflict_ReturnCode");
    Check(cli.sample_session, "TestSampleSessionConflict_Session");
    Check(cli.sample_request_stdin, "TestSampleSessionConflict_Stdin");
}

static void TestDescribeFunctionsJsonMissingValue() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--describe-functions-json"}), &cli);
    Check(rc != 0, "TestDescribeFunctionsJsonMissingValue_Fails");
}

static void TestLoadStateJson() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--load-state-json", "state.json"}), &cli);
    Check(rc == 0, "TestLoadStateJson_ReturnCode");
    Check(cli.have_load_state_json, "TestLoadStateJson_Have");
    Check(cli.load_state_json == "state.json", "TestLoadStateJson_Path");
}

static void TestSidecarApplyArmedStepCount() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--sidecar-apply-armed-step-count", "2"}), &cli);
    Check(rc == 0, "TestSidecarApplyArmedStepCount_ReturnCode");
    Check(cli.have_sidecar_apply_armed_step_count, "TestSidecarApplyArmedStepCount_Have");
    Check(cli.sidecar_apply_armed_step_count == 2, "TestSidecarApplyArmedStepCount_Value");
}

static void TestSidecarApplyArmedStepCountMissingValue() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--sidecar-apply-armed-step-count"}), &cli);
    Check(rc != 0, "TestSidecarApplyArmedStepCountMissingValue_Fails");
}

static void TestSidecarReplayMutationHistoryCount() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--sidecar-replay-mutation-history-count", "2"}), &cli);
    Check(rc == 0, "TestSidecarReplayMutationHistoryCount_ReturnCode");
    Check(cli.have_sidecar_replay_mutation_history_count, "TestSidecarReplayMutationHistoryCount_Have");
    Check(cli.sidecar_replay_mutation_history_count == 2, "TestSidecarReplayMutationHistoryCount_Value");
}

static void TestSidecarReplayMutationHistoryCountMissingValue() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--sidecar-replay-mutation-history-count"}), &cli);
    Check(rc != 0, "TestSidecarReplayMutationHistoryCountMissingValue_Fails");
}

static void TestSidecarPumpPacedLoopSeconds() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--sidecar-pump-paced-loop-seconds", "0.25"}), &cli);
    Check(rc == 0, "TestSidecarPumpPacedLoopSeconds_ReturnCode");
    Check(cli.have_sidecar_pump_paced_loop_seconds, "TestSidecarPumpPacedLoopSeconds_Have");
    Check(std::fabs(cli.sidecar_pump_paced_loop_seconds - 0.25) < 1e-9, "TestSidecarPumpPacedLoopSeconds_Value");
}

static void TestSidecarPumpPacedLoopSecondsMissingValue() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--sidecar-pump-paced-loop-seconds"}), &cli);
    Check(rc != 0, "TestSidecarPumpPacedLoopSecondsMissingValue_Fails");
}

static void TestFindingGroup() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--finding-group", "test_group"}), &cli);
    Check(rc == 0, "TestFindingGroup_ReturnCode");
    Check(cli.have_finding_group, "TestFindingGroup_Have");
    Check(cli.finding_group == "test_group", "TestFindingGroup_Value");
}

static void TestExplainoWarpStrength() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--explaino-warp-strength", "0.75"}), &cli);
    Check(rc == 0, "TestExplainoWarpStrength_ReturnCode");
    Check(cli.have_explaino_warp_strength, "TestExplainoWarpStrength_Have");
    Check(std::fabs(cli.explaino_warp_strength - 0.75) < 1e-9, "TestExplainoWarpStrength_Value");
}

static void TestLambdaOverrides() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--lambda-real", "2.5", "--lambda-imag", "-0.3"}), &cli);
    Check(rc == 0, "TestLambdaOverrides_ReturnCode");
    Check(cli.have_lambda_real && std::fabs(cli.lambda_real - 2.5) < 1e-9, "TestLambdaOverrides_Real");
    Check(cli.have_lambda_imag && std::fabs(cli.lambda_imag - (-0.3)) < 1e-9, "TestLambdaOverrides_Imag");
}

static void TestCaptureDiagnostic() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--capture-diagnostic"}), &cli);
    Check(rc == 0, "TestCaptureDiagnostic_ReturnCode");
    Check(cli.capture_diagnostic_only, "TestCaptureDiagnostic_Flag");
}

static void TestCaptureFinding() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--capture-finding"}), &cli);
    Check(rc == 0, "TestCaptureFinding_ReturnCode");
    Check(cli.capture_finding_only, "TestCaptureFinding_Flag");
}

static void TestMultipleFlagsCompose() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({
        "--fractal-type", "explaino",
        "--explaino-seed", "7.0",
        "--explaino-phase", "0.5",
        "--width", "512",
        "--height", "512"
    }), &cli);
    Check(rc == 0, "TestMultipleFlagsCompose_ReturnCode");
    Check(cli.have_fractal_type && cli.fractal_type == FractalType::explaino, "TestMultipleFlagsCompose_FractalType");
    Check(cli.have_explaino_seed && std::fabs(cli.explaino_seed - 7.0) < 1e-9, "TestMultipleFlagsCompose_Seed");
    Check(cli.have_explaino_phase && std::fabs(cli.explaino_phase - 0.5) < 1e-9, "TestMultipleFlagsCompose_Phase");
    Check(cli.have_width && cli.width == 512, "TestMultipleFlagsCompose_Width");
    Check(cli.have_height && cli.height == 512, "TestMultipleFlagsCompose_Height");
}

static void TestExplainoSeedDrift() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--explaino-seed-drift", "0.25"}), &cli);
    Check(rc == 0, "TestExplainoSeedDrift_ReturnCode");
    Check(cli.have_explaino_seed_drift, "TestExplainoSeedDrift_Have");
    Check(std::fabs(cli.explaino_seed_drift - 0.25) < 1e-9, "TestExplainoSeedDrift_Value");
}

static void TestExplainoSeedB() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--explaino-seed-b", "3.14"}), &cli);
    Check(rc == 0, "TestExplainoSeedB_ReturnCode");
    Check(cli.have_explaino_seed_b, "TestExplainoSeedB_Have");
    Check(std::fabs(cli.explaino_seed_b - 3.14) < 1e-9, "TestExplainoSeedB_Value");
}

static void TestExplainoMix() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--explaino-mix", "0.6"}), &cli);
    Check(rc == 0, "TestExplainoMix_ReturnCode");
    Check(cli.have_explaino_mix, "TestExplainoMix_Have");
    Check(std::fabs(cli.explaino_mix - 0.6) < 1e-9, "TestExplainoMix_Value");
}

static void TestFindingWhy() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--finding-why", "test reason"}), &cli);
    Check(rc == 0, "TestFindingWhy_ReturnCode");
    Check(cli.have_finding_why, "TestFindingWhy_Have");
    Check(cli.finding_why == "test reason", "TestFindingWhy_Value");
}

static void TestDescribeFunctions() {
    ViewerCliArgs cli{};
    int rc = ParseViewerCli(Args({"--describe-functions"}), &cli);
    Check(rc == 0, "TestDescribeFunctions_ReturnCode");
    Check(cli.describe_functions, "TestDescribeFunctions_Flag");
}

int main() {
    TestDefaultsNoArgs();
    TestValidateUi();
    TestFractalType();
    TestFractalTypeMissingValue();
    TestFractalTypeUnknown();
    TestExplainoSeed();
    TestExplainoSeedBadValue();
    TestExplainoSeedMissing();
    TestExplainoPhase();
    TestResolutionOverrides();
    TestResolutionBadWidth();
    TestSweepConfigComplete();
    TestSweepConfigIncomplete();
    TestSweepConfigZeroStepFails();
    TestSweepConfigNegativeStepFails();
    TestSweepConfigEqualBoundsFails();
    TestSweepConfigDescendingBoundsFails();
    TestSampleModeStdio();
    TestSampleModeJsonPaths();
    TestExploreRecommendJson();
    TestExploreRecommendStdout();
    TestFlashlightProbe();
    TestFlashlightProbeMissingValue();
    TestFlashlightProbeBadFractalType();
    TestExploreRecommendJsonMissingValue();
    TestSampleSession();
    TestSampleSessionPipe();
    TestSampleSessionPipeRequiresSession();
    TestSampleSessionPipeMissingValue();
    TestSampleSessionConflictsWithSampleRequest();
    TestDescribeFunctionsJson();
    TestDescribeFunctionsJsonMissingValue();
    TestLoadStateJson();
    TestSidecarApplyArmedStepCount();
    TestSidecarApplyArmedStepCountMissingValue();
    TestSidecarReplayMutationHistoryCount();
    TestSidecarReplayMutationHistoryCountMissingValue();
    TestSidecarPumpPacedLoopSeconds();
    TestSidecarPumpPacedLoopSecondsMissingValue();
    TestFindingGroup();
    TestExplainoWarpStrength();
    TestLambdaOverrides();
    TestCaptureDiagnostic();
    TestCaptureFinding();
    TestMultipleFlagsCompose();
    TestExplainoSeedDrift();
    TestExplainoSeedB();
    TestExplainoMix();
    TestFindingWhy();
    TestDescribeFunctions();

    std::printf("test_viewer_cli: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}
