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
    bool describe_parameter_surface = false;
    bool describe_fractal_catalog = false;
    bool describe_viewport_facts = false;
    bool describe_explaino_axis_registry = false;
    bool validate_ui_salt_contract = false;

    // Diagnostic capture output
    bool have_diagnostics_out_dir = false;
    std::string diagnostics_out_dir;
    bool have_diagnostics_out_dir_alias = false;
    std::string diagnostics_out_dir_alias;

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

    // Describe parameter surface
    bool have_describe_parameter_surface_json = false;
    std::string describe_parameter_surface_json_path;

    // Describe fractal catalog
    bool have_describe_fractal_catalog_json = false;
    std::string describe_fractal_catalog_json_path;

    // Describe exact viewport geometry for one loaded state
    bool have_describe_viewport_facts_json = false;
    std::string describe_viewport_facts_json_path;

    // Describe Explaino-axis registry
    bool have_describe_explaino_axis_registry_json = false;
    std::string describe_explaino_axis_registry_json_path;

    // UI-Salt materialized contract validation
    bool have_ui_salt_contract_json = false;
    std::string ui_salt_contract_json_path;
    bool have_ui_salt_contract_report_json = false;
    std::string ui_salt_contract_report_json_path;

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
    bool open_equation_pack_workbench_on_startup = false;
    bool have_equation_pack_workbench_pack_json = false;
    std::string equation_pack_workbench_pack_json_path;
    bool open_sdf_pack_panel_on_startup = false;
    bool have_sdf_pack_json = false;
    std::string sdf_pack_json_path;
    bool have_ui_automation_report_json = false;
    std::string ui_automation_report_json_path;
    bool have_ui_automation_command_json = false;
    std::string ui_automation_command_json_path;
    bool have_ui_automation_click_control_id = false;
    std::string ui_automation_click_control_id;
    bool have_ui_automation_set_control_value = false;
    std::string ui_automation_set_control_id;
    double ui_automation_set_control_value = 0.0;

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
    bool apply_loaded_color_pipeline_draft = false;

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
    const bool describeParameterSurface = cli.describe_parameter_surface || cli.have_describe_parameter_surface_json;
    const bool describeFractalCatalog = cli.describe_fractal_catalog || cli.have_describe_fractal_catalog_json;
    const bool describeViewportFacts = cli.describe_viewport_facts || cli.have_describe_viewport_facts_json;
    const bool describeExplainoAxisRegistry = cli.describe_explaino_axis_registry || cli.have_describe_explaino_axis_registry_json;
    const bool validateUiSaltContract = cli.validate_ui_salt_contract || cli.have_ui_salt_contract_json || cli.have_ui_salt_contract_report_json;
    const bool flashlightProbe = cli.flashlight_probe || cli.have_flashlight_probe_path;
    const bool runtimeWalk = cli.have_runtime_walk_request_json;
    const bool runtimeWalkViewer = cli.have_runtime_walk_viewer_request_json || cli.have_runtime_walk_viewer_fits_path;
    const bool colorPipelineHeadlessProof = !cli.color_pipeline_headless_proof.actions.empty();
    const bool loadedDraftIdentityOverride = cli.have_fractal_type || cli.have_explaino_seed || cli.sweep_config.enabled;
    const bool viewportStateMutation = cli.apply_loaded_color_pipeline_draft ||
        colorPipelineHeadlessProof || loadedDraftIdentityOverride ||
        cli.have_explaino_seed_b || cli.have_explaino_mix || cli.have_explaino_phase ||
        cli.have_explaino_warp_strength || cli.have_explaino_seed_drift ||
        cli.have_lambda_real || cli.have_lambda_imag || cli.have_width || cli.have_height;
    if (cli.have_runtime_walk_viewer_request_json && cli.have_runtime_walk_viewer_fits_path) return false;
    if (cli.have_diagnostics_out_dir && !cli.capture_diagnostic_only) return false;
    if (cli.capture_diagnostic_only && cli.capture_finding_only) return false;
    if (cli.apply_loaded_color_pipeline_draft &&
            (!cli.have_load_state_json || colorPipelineHeadlessProof || loadedDraftIdentityOverride)) {
        return false;
    }
    if (colorPipelineHeadlessProof && !(cli.capture_diagnostic_only || cli.capture_finding_only)) return false;
    if (describeViewportFacts && (!cli.have_load_state_json || viewportStateMutation ||
            cli.validate_ui_only || cli.capture_diagnostic_only || cli.capture_finding_only ||
            exploreRecommend || describeParameterSurface || describeFractalCatalog ||
            describeExplainoAxisRegistry || validateUiSaltContract || flashlightProbe ||
            runtimeWalk || runtimeWalkViewer || cli.sample_session || cli.any_sample_mode_arg ||
            cli.describe_functions || cli.have_describe_functions_json)) return false;
    if (cli.validate_ui_only && (cli.capture_diagnostic_only || cli.capture_finding_only)) return false;
    if (validateUiSaltContract && !cli.validate_ui_salt_contract) return false;
    if (validateUiSaltContract && (cli.validate_ui_only || cli.capture_diagnostic_only || cli.capture_finding_only ||
            exploreRecommend || describeParameterSurface || describeFractalCatalog || describeExplainoAxisRegistry || flashlightProbe ||
            runtimeWalk || runtimeWalkViewer || cli.sample_session || cli.any_sample_mode_arg)) return false;
    if (exploreRecommend && (cli.validate_ui_only || cli.capture_diagnostic_only || cli.capture_finding_only || describeParameterSurface || describeFractalCatalog || describeExplainoAxisRegistry || validateUiSaltContract)) return false;
    if (describeParameterSurface && (cli.validate_ui_only || cli.capture_diagnostic_only || cli.capture_finding_only || exploreRecommend || describeFractalCatalog || describeExplainoAxisRegistry || validateUiSaltContract || flashlightProbe || runtimeWalk || runtimeWalkViewer || cli.any_sample_mode_arg)) return false;
    if (describeFractalCatalog && (cli.validate_ui_only || cli.capture_diagnostic_only || cli.capture_finding_only || exploreRecommend || describeParameterSurface || describeExplainoAxisRegistry || validateUiSaltContract || flashlightProbe || runtimeWalk || runtimeWalkViewer || cli.sample_session || cli.any_sample_mode_arg || cli.describe_functions || cli.have_describe_functions_json)) return false;
    if (describeExplainoAxisRegistry && (cli.validate_ui_only || cli.capture_diagnostic_only || cli.capture_finding_only || exploreRecommend || describeFractalCatalog || validateUiSaltContract || flashlightProbe || runtimeWalk || runtimeWalkViewer || cli.any_sample_mode_arg)) return false;
    if (flashlightProbe && (cli.validate_ui_only || cli.capture_diagnostic_only || cli.capture_finding_only || exploreRecommend || describeExplainoAxisRegistry || validateUiSaltContract)) return false;
    if (runtimeWalk && (cli.validate_ui_only || cli.capture_diagnostic_only || cli.capture_finding_only || exploreRecommend || describeExplainoAxisRegistry || validateUiSaltContract || flashlightProbe || runtimeWalkViewer)) return false;
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
