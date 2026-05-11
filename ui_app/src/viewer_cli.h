#pragma once

#include <string>
#include <vector>

#include "fractal_types.h"
#include "headless_modes.h"
#include "sweep_player.h"

// Parsed CLI arguments for the viewer.  Produced by ParseViewerCli() and
// consumed by WinMain to decide mode and apply overrides.  This struct is
// headless-testable with no D3D11 or Win32 dependency.

struct ViewerCliArgs {
    // Mode flags
    bool validate_ui_only = false;
    bool capture_diagnostic_only = false;
    bool capture_finding_only = false;
    bool describe_functions = false;

    // Sample mode
    bool sample_request_stdin = false;
    bool sample_response_stdout = false;
    bool have_sample_request_json = false;
    bool have_sample_response_json = false;
    std::string sample_request_json_path;
    std::string sample_response_json_path;

    // Session mode (V2-B)
    bool sample_session = false;
    bool have_sample_session_pipe = false;
    std::string sample_session_pipe_name;

    // Describe-functions
    bool have_describe_functions_json = false;
    std::string describe_functions_json_path;

    // Headless advisor report
    bool explore_recommend = false;
    bool have_explore_recommend_json = false;
    std::string explore_recommend_json_path;

    // Flashlight headless probe
    bool flashlight_probe = false;
    bool have_flashlight_probe_path = false;
    std::string flashlight_probe_path;
    bool have_flashlight_ticks = false;
    int flashlight_ticks = 8;
    bool have_flashlight_radius = false;
    double flashlight_radius = 0.75;
    bool have_flashlight_zoom_radius = false;
    double flashlight_zoom_radius = 0.25;
    bool have_flashlight_warp = false;
    double flashlight_warp = 0.0;
    bool flashlight_closure_last = false;
    bool have_flashlight_fractal_type = false;
    FractalType flashlight_fractal_type = FractalType::explaino_fp;

    // Saved-runtime walk
    bool have_runtime_walk_request_json = false;
    std::string runtime_walk_request_json_path;
    bool have_runtime_walk_viewer_request_json = false;
    std::string runtime_walk_viewer_request_json_path;
    bool have_runtime_walk_viewer_fits_path = false;
    std::string runtime_walk_viewer_fits_path;
    bool open_color_pipeline_window_on_startup = false;
    bool have_ui_automation_report_json = false;
    std::string ui_automation_report_json_path;

    // Fractal type
    bool have_fractal_type = false;
    FractalType fractal_type = FractalType::newton;

    // Explaino overrides
    bool have_explaino_seed = false;
    double explaino_seed = 0.0;

    bool have_explaino_seed_b = false;
    double explaino_seed_b = 0.0;

    bool have_explaino_mix = false;
    double explaino_mix = 0.0;

    bool have_explaino_phase = false;
    double explaino_phase = 0.0;

    bool have_explaino_warp_strength = false;
    double explaino_warp_strength = 0.0;

    bool have_explaino_seed_drift = false;
    double explaino_seed_drift = 0.0;

    // Lambda overrides
    bool have_lambda_real = false;
    double lambda_real = 0.0;

    bool have_lambda_imag = false;
    double lambda_imag = 0.0;

    // Resolution overrides
    bool have_width = false;
    int width = 0;

    bool have_height = false;
    int height = 0;

    // State loading
    bool have_load_state_json = false;
    std::string load_state_json;

    // Headless advanced-color proof overrides
    ColorPipelineHeadlessProofConfig color_pipeline_headless_proof;

    // Headless sidecar proof overrides
    bool have_sidecar_apply_armed_step_count = false;
    int sidecar_apply_armed_step_count = 0;

    bool have_sidecar_replay_mutation_history_count = false;
    int sidecar_replay_mutation_history_count = 0;

    bool have_sidecar_pump_paced_loop_seconds = false;
    double sidecar_pump_paced_loop_seconds = 0.0;

    // Finding capture overrides
    bool have_finding_group = false;
    std::string finding_group;

    bool have_finding_why = false;
    std::string finding_why;

    // Sweep configuration
    SweepPlayerConfig sweep_config{};

    // Derived convenience: true if any sample-mode arg was present
    bool any_sample_mode_arg = false;
};

inline bool ValidateViewerCliModeConflicts(const ViewerCliArgs& cli) {
    const bool exploreRecommend = cli.explore_recommend || cli.have_explore_recommend_json;
    const bool flashlightProbe = cli.flashlight_probe || cli.have_flashlight_probe_path;
    const bool runtimeWalk = cli.have_runtime_walk_request_json;
    const bool runtimeWalkViewer = cli.have_runtime_walk_viewer_request_json || cli.have_runtime_walk_viewer_fits_path;
    const bool colorPipelineHeadlessProof = !cli.color_pipeline_headless_proof.actions.empty();
    if (cli.have_runtime_walk_viewer_request_json && cli.have_runtime_walk_viewer_fits_path) return false;
    if (cli.capture_diagnostic_only && cli.capture_finding_only) return false;
    if (colorPipelineHeadlessProof && !(cli.capture_diagnostic_only || cli.capture_finding_only)) return false;
    if (cli.validate_ui_only && (cli.capture_diagnostic_only || cli.capture_finding_only)) return false;
    if (exploreRecommend && (cli.validate_ui_only || cli.capture_diagnostic_only || cli.capture_finding_only)) return false;
    if (flashlightProbe && (cli.validate_ui_only || cli.capture_diagnostic_only || cli.capture_finding_only || exploreRecommend)) return false;
    if (runtimeWalk && (cli.validate_ui_only || cli.capture_diagnostic_only || cli.capture_finding_only || exploreRecommend || flashlightProbe || runtimeWalkViewer)) return false;
    return true;
}

inline SampleModeArgs BuildViewerCliSampleModeArgs(const ViewerCliArgs& cli) {
    SampleModeArgs args;
    args.request_stdin = cli.sample_request_stdin;
    args.response_stdout = cli.sample_response_stdout;
    if (cli.have_sample_request_json) {
        args.request_json_path = cli.sample_request_json_path;
    }
    if (cli.have_sample_response_json) {
        args.response_json_path = cli.sample_response_json_path;
    }
    args.conflict_validate_ui = cli.validate_ui_only;
    args.conflict_capture_diagnostic = cli.capture_diagnostic_only;
    args.conflict_capture_finding = cli.capture_finding_only;
    return args;
}

// Parse command-line args into a ViewerCliArgs struct.
// Returns 0 on success.  Returns a nonzero exit code on fatal parse error
// (e.g. --explaino-seed present but value missing/invalid).
int ParseViewerCli(const std::vector<std::string>& args, ViewerCliArgs* out);
