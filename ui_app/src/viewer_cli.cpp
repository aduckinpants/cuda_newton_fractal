#include "viewer_cli.h"
#include "cli_args.h"
#include "enum_id_utils.h"

// Helper: parse a double arg with the standard "present but invalid => fail" contract.
static bool TryDouble(const std::vector<std::string>& args, const char* flag, bool* haveOut, double* valOut) {
    *haveOut = TryParseDoubleArg(args, flag, valOut);
    if (HasArg(args, flag) && !*haveOut) return false;
    return true;
}

// Helper: parse an int arg.
static bool TryInt(const std::vector<std::string>& args, const char* flag, bool* haveOut, int* valOut) {
    *haveOut = TryParseIntArg(args, flag, valOut);
    if (HasArg(args, flag) && !*haveOut) return false;
    return true;
}

// Helper: parse a string-valued arg.
static bool TryStr(const std::vector<std::string>& args, const char* flag, bool* haveOut, std::string* valOut) {
    *haveOut = TryGetArgValue(args, flag, valOut);
    if (HasArg(args, flag) && !*haveOut) return false;
    return true;
}

static bool IsValidSweepSeedRange(double start, double stop, double step) {
    if (step <= 0.0) return false;
    if (!(start < stop)) return false;
    return true;
}

static bool TryParseSweepConfig(const std::vector<std::string>& args, SweepPlayerConfig* outConfig) {
    bool haveSweepStart = false, haveSweepStop = false, haveSweepStep = false;
    double sweepStart = 0.0, sweepStop = 0.0, sweepStep = 0.0;
    if (!TryDouble(args, "--sweep-seed-start", &haveSweepStart, &sweepStart)) return false;
    if (!TryDouble(args, "--sweep-seed-stop", &haveSweepStop, &sweepStop)) return false;
    if (!TryDouble(args, "--sweep-seed-step", &haveSweepStep, &sweepStep)) return false;

    int sweepDwellMs = 450;
    bool haveSweepDwell = false;
    if (!TryInt(args, "--sweep-dwell-ms", &haveSweepDwell, &sweepDwellMs)) return false;

    const bool sweepLoop = HasArg(args, "--sweep-loop");
    const bool haveAnySweep = haveSweepStart || haveSweepStop || haveSweepStep || haveSweepDwell || sweepLoop;
    if (!haveAnySweep) return true;
    if (!(haveSweepStart && haveSweepStop && haveSweepStep)) return false;
    if (!IsValidSweepSeedRange(sweepStart, sweepStop, sweepStep)) return false;

    if (outConfig) {
        outConfig->enabled = true;
        outConfig->seed_start = sweepStart;
        outConfig->seed_stop = sweepStop;
        outConfig->seed_step = sweepStep;
        outConfig->dwell_seconds = (double)sweepDwellMs / 1000.0;
        outConfig->loop = sweepLoop;
    }
    return true;
}

int ParseViewerCli(const std::vector<std::string>& args, ViewerCliArgs* out) {
    *out = ViewerCliArgs{};

    // Mode flags
    out->validate_ui_only = HasArg(args, "--validate-ui");
    out->capture_diagnostic_only = HasArg(args, "--capture-diagnostic");
    out->capture_finding_only = HasArg(args, "--capture-finding");
    out->describe_functions = HasArg(args, "--describe-functions");
    out->explore_recommend = HasArg(args, "--explore-recommend");
    out->flashlight_probe = HasArg(args, "--flashlight-probe");

    // Sample mode
    out->sample_request_stdin = HasArg(args, "--sample-request-stdin");
    out->sample_response_stdout = HasArg(args, "--sample-response-stdout");
    if (!TryStr(args, "--sample-request-json", &out->have_sample_request_json, &out->sample_request_json_path)) return 1;
    if (!TryStr(args, "--sample-response-json", &out->have_sample_response_json, &out->sample_response_json_path)) return 1;

    // Session mode (V2-B)
    out->sample_session = HasArg(args, "--sample-session");
    if (!TryStr(args, "--sample-session-pipe", &out->have_sample_session_pipe, &out->sample_session_pipe_name)) return 1;
    if (out->have_sample_session_pipe && !out->sample_session) return 1;

    int sampleSourceCount = (out->sample_request_stdin ? 1 : 0) + (out->have_sample_request_json ? 1 : 0);
    out->any_sample_mode_arg = sampleSourceCount > 0 || out->sample_response_stdout || out->have_sample_response_json;

    // Describe-functions JSON
    if (!TryStr(args, "--describe-functions-json", &out->have_describe_functions_json, &out->describe_functions_json_path)) return 1;
    if (!TryStr(args, "--explore-recommend-json", &out->have_explore_recommend_json, &out->explore_recommend_json_path)) return 1;
    if (!TryStr(args, "--flashlight-probe", &out->have_flashlight_probe_path, &out->flashlight_probe_path)) return 1;
    if (!TryStr(args, "--runtime-walk-request-json", &out->have_runtime_walk_request_json, &out->runtime_walk_request_json_path)) return 1;
    if (out->flashlight_probe && !out->have_flashlight_probe_path) return 1;
    if (!TryInt(args, "--flashlight-ticks", &out->have_flashlight_ticks, &out->flashlight_ticks)) return 1;
    if (!TryDouble(args, "--flashlight-radius", &out->have_flashlight_radius, &out->flashlight_radius)) return 1;
    if (!TryDouble(args, "--flashlight-zoom-radius", &out->have_flashlight_zoom_radius, &out->flashlight_zoom_radius)) return 1;
    if (!TryDouble(args, "--flashlight-warp", &out->have_flashlight_warp, &out->flashlight_warp)) return 1;
    out->flashlight_closure_last = HasArg(args, "--flashlight-closure") || HasArg(args, "--flashlight-closure-last");
    {
        std::string flashlightFractalTypeId;
        bool haveFlashlightFractalTypeId = false;
        if (!TryStr(args, "--flashlight-fractal-type", &haveFlashlightFractalTypeId, &flashlightFractalTypeId)) return 1;
        if (haveFlashlightFractalTypeId) {
            out->have_flashlight_fractal_type = TryParseFractalTypeId(flashlightFractalTypeId, &out->flashlight_fractal_type);
            if (!out->have_flashlight_fractal_type) return 1;
        }
    }

    // Fractal type
    out->have_fractal_type = TryParseFractalTypeArg(args, &out->fractal_type);
    if (HasArg(args, "--fractal-type") && !out->have_fractal_type) return 1;

    // Explaino overrides
    if (!TryDouble(args, "--explaino-seed", &out->have_explaino_seed, &out->explaino_seed)) return 1;
    if (!TryDouble(args, "--explaino-seed-b", &out->have_explaino_seed_b, &out->explaino_seed_b)) return 1;
    if (!TryDouble(args, "--explaino-mix", &out->have_explaino_mix, &out->explaino_mix)) return 1;
    if (!TryDouble(args, "--explaino-phase", &out->have_explaino_phase, &out->explaino_phase)) return 1;
    if (!TryDouble(args, "--explaino-warp-strength", &out->have_explaino_warp_strength, &out->explaino_warp_strength)) return 1;
    if (!TryDouble(args, "--explaino-seed-drift", &out->have_explaino_seed_drift, &out->explaino_seed_drift)) return 1;

    // Lambda overrides
    if (!TryDouble(args, "--lambda-real", &out->have_lambda_real, &out->lambda_real)) return 1;
    if (!TryDouble(args, "--lambda-imag", &out->have_lambda_imag, &out->lambda_imag)) return 1;

    // Resolution overrides
    if (!TryInt(args, "--width", &out->have_width, &out->width)) return 1;
    if (!TryInt(args, "--height", &out->have_height, &out->height)) return 1;

    // State loading
    if (!TryStr(args, "--load-state-json", &out->have_load_state_json, &out->load_state_json)) return 1;
    if (!TryInt(args, "--sidecar-apply-armed-step-count", &out->have_sidecar_apply_armed_step_count, &out->sidecar_apply_armed_step_count)) return 1;
    if (!TryInt(args, "--sidecar-replay-mutation-history-count", &out->have_sidecar_replay_mutation_history_count, &out->sidecar_replay_mutation_history_count)) return 1;
    if (!TryDouble(args, "--sidecar-pump-paced-loop-seconds", &out->have_sidecar_pump_paced_loop_seconds, &out->sidecar_pump_paced_loop_seconds)) return 1;

    // Finding overrides
    if (!TryStr(args, "--finding-group", &out->have_finding_group, &out->finding_group)) return 1;
    if (!TryStr(args, "--finding-why", &out->have_finding_why, &out->finding_why)) return 1;

    if (!TryParseSweepConfig(args, &out->sweep_config)) return 1;

    return 0;
}
