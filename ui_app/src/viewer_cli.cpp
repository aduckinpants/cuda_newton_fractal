#include "viewer_cli.h"
#include "cli_args.h"
#include "enum_id_utils.h"

#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <sstream>

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

static bool TryParseFlashlightArgs(const std::vector<std::string>& args, ViewerCliArgs* out) {
    if (!TryStr(args, "--flashlight-probe", &out->have_flashlight_probe_path, &out->flashlight_probe_path)) return false;
    out->flashlight_probe = HasArg(args, "--flashlight-probe");
    if (out->flashlight_probe && !out->have_flashlight_probe_path) return false;
    if (!TryInt(args, "--flashlight-ticks", &out->have_flashlight_ticks, &out->flashlight_ticks)) return false;
    if (!TryDouble(args, "--flashlight-radius", &out->have_flashlight_radius, &out->flashlight_radius)) return false;
    if (!TryDouble(args, "--flashlight-zoom-radius", &out->have_flashlight_zoom_radius, &out->flashlight_zoom_radius)) return false;
    if (!TryDouble(args, "--flashlight-warp", &out->have_flashlight_warp, &out->flashlight_warp)) return false;
    out->flashlight_closure_last = HasArg(args, "--flashlight-closure") || HasArg(args, "--flashlight-closure-last");

    std::string flashlightFractalTypeId;
    bool haveFlashlightFractalTypeId = false;
    if (!TryStr(args, "--flashlight-fractal-type", &haveFlashlightFractalTypeId, &flashlightFractalTypeId)) return false;
    if (haveFlashlightFractalTypeId) {
        out->have_flashlight_fractal_type = TryParseFractalTypeId(flashlightFractalTypeId, &out->flashlight_fractal_type);
        if (!out->have_flashlight_fractal_type) return false;
    }
    return true;
}

static bool TryParseFiniteDoubleText(const std::string& text, double* outValue);

static bool TryParseUiAutomationSetControlValueSpec(
    const std::string& spec,
    std::string* outControlId,
    double* outValue) {
    const std::size_t equals = spec.find('=');
    if (equals == std::string::npos || equals == 0 || equals + 1 >= spec.size()) {
        return false;
    }
    double parsedValue = 0.0;
    if (!TryParseFiniteDoubleText(spec.substr(equals + 1), &parsedValue)) {
        return false;
    }
    if (outControlId) {
        *outControlId = spec.substr(0, equals);
    }
    if (outValue) {
        *outValue = parsedValue;
    }
    return true;
}

static bool TryParseRuntimeWalkArgs(const std::vector<std::string>& args, ViewerCliArgs* out) {
    if (!TryStr(args, "--runtime-walk-request-json", &out->have_runtime_walk_request_json, &out->runtime_walk_request_json_path)) return false;
    if (!TryStr(args, "--load-runtime-walk-request-json", &out->have_runtime_walk_viewer_request_json, &out->runtime_walk_viewer_request_json_path)) return false;
    if (!TryStr(args, "--load-runtime-walk-fits", &out->have_runtime_walk_viewer_fits_path, &out->runtime_walk_viewer_fits_path)) return false;
    out->open_color_pipeline_window_on_startup = HasArg(args, "--open-color-pipeline-window");
    out->open_equation_pack_workbench_on_startup = HasArg(args, "--open-equation-pack-workbench");
    if (!TryStr(args, "--equation-pack-workbench-pack-json", &out->have_equation_pack_workbench_pack_json, &out->equation_pack_workbench_pack_json_path)) return false;
    if (!TryStr(args, "--ui-automation-report-json", &out->have_ui_automation_report_json, &out->ui_automation_report_json_path)) return false;
    if (!TryStr(args, "--ui-automation-command-json", &out->have_ui_automation_command_json, &out->ui_automation_command_json_path)) return false;
    if (!TryStr(args, "--ui-automation-click-control-id", &out->have_ui_automation_click_control_id, &out->ui_automation_click_control_id)) return false;
    std::string setControlValueSpec;
    bool haveSetControlValueSpec = false;
    if (!TryStr(args, "--ui-automation-set-control-value", &haveSetControlValueSpec, &setControlValueSpec)) return false;
    if (haveSetControlValueSpec) {
        out->have_ui_automation_set_control_value = TryParseUiAutomationSetControlValueSpec(
            setControlValueSpec,
            &out->ui_automation_set_control_id,
            &out->ui_automation_set_control_value);
        if (!out->have_ui_automation_set_control_value) return false;
    }
    return true;
}

static std::vector<std::string> SplitColorPipelineActionSpec(const std::string& spec) {
    std::vector<std::string> tokens;
    std::size_t start = 0;
    while (start <= spec.size()) {
        const std::size_t next = spec.find(':', start);
        if (next == std::string::npos) {
            tokens.push_back(spec.substr(start));
            break;
        }
        tokens.push_back(spec.substr(start, next - start));
        start = next + 1;
    }
    return tokens;
}

static std::string LowercaseAscii(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

static bool TryParseNonNegativeIntText(const std::string& text, int* outValue) {
    if (!outValue || text.empty()) {
        return false;
    }
    char* end = nullptr;
    errno = 0;
    const long value = std::strtol(text.c_str(), &end, 10);
    if (!end || end == text.c_str() || *end != '\0' || errno == ERANGE) {
        return false;
    }
    if (value < 0 || value > static_cast<long>((std::numeric_limits<int>::max)())) {
        return false;
    }
    *outValue = static_cast<int>(value);
    return true;
}

static bool TryParseFiniteDoubleText(const std::string& text, double* outValue) {
    if (!outValue || text.empty()) {
        return false;
    }
    char* end = nullptr;
    errno = 0;
    const double value = std::strtod(text.c_str(), &end);
    if (!end || end == text.c_str() || *end != '\0' || errno == ERANGE || !std::isfinite(value)) {
        return false;
    }
    *outValue = value;
    return true;
}

static bool TryParseMoveDirectionText(const std::string& text, int* outDirection) {
    if (!outDirection) {
        return false;
    }
    const std::string normalized = LowercaseAscii(text);
    if (normalized == "up" || normalized == "left" || normalized == "-1") {
        *outDirection = -1;
        return true;
    }
    if (normalized == "down" || normalized == "right" || normalized == "+1" || normalized == "1") {
        *outDirection = 1;
        return true;
    }
    return false;
}

static bool TryParseBoolText(const std::string& text, bool* outValue) {
    if (!outValue) {
        return false;
    }
    const std::string normalized = LowercaseAscii(text);
    if (normalized == "true" || normalized == "1") {
        *outValue = true;
        return true;
    }
    if (normalized == "false" || normalized == "0") {
        *outValue = false;
        return true;
    }
    return false;
}

static bool TryParseColorPipelineActionSpec(
    const std::string& spec,
    ColorPipelineHeadlessAction* outAction,
    std::string* outError) {
    if (!outAction) {
        if (outError) {
            *outError = "Advanced color action parsing requires output storage.";
        }
        return false;
    }

    const std::vector<std::string> tokens = SplitColorPipelineActionSpec(spec);
    if (tokens.empty() || tokens[0].empty()) {
        if (outError) {
            *outError = "Advanced color action specs require a non-empty verb.";
        }
        return false;
    }

    const std::string& verb = tokens[0];
    if (verb == "select_function") {
        if (tokens.size() != 4 || tokens[1].empty() || tokens[2].empty() || tokens[3].empty()) {
            if (outError) {
                *outError = "select_function expects lane_id:row_index:function_id";
            }
            return false;
        }
        ColorPipelineHeadlessSelectFunctionAction action;
        action.lane_id = tokens[1];
        if (!TryParseNonNegativeIntText(tokens[2], &action.row_index)) {
            if (outError) {
                *outError = "select_function requires a non-negative row index";
            }
            return false;
        }
        action.function_id = tokens[3];
        *outAction = std::move(action);
        return true;
    }

    if (verb == "add_row") {
        if ((tokens.size() != 2 && tokens.size() != 3) || tokens[1].empty()) {
            if (outError) {
                *outError = "add_row expects lane_id[:function_id]";
            }
            return false;
        }
        ColorPipelineHeadlessAddRowAction action;
        action.lane_id = tokens[1];
        if (tokens.size() == 3) {
            action.function_id = tokens[2];
        }
        *outAction = std::move(action);
        return true;
    }

    if (verb == "move_row") {
        if (tokens.size() != 4 || tokens[1].empty() || tokens[2].empty() || tokens[3].empty()) {
            if (outError) {
                *outError = "move_row expects lane_id:row_index:up|down";
            }
            return false;
        }
        ColorPipelineHeadlessMoveRowAction action;
        action.lane_id = tokens[1];
        if (!TryParseNonNegativeIntText(tokens[2], &action.row_index)) {
            if (outError) {
                *outError = "move_row requires a non-negative row index";
            }
            return false;
        }
        if (!TryParseMoveDirectionText(tokens[3], &action.direction)) {
            if (outError) {
                *outError = "move_row direction must be up, down, -1, or 1";
            }
            return false;
        }
        *outAction = std::move(action);
        return true;
    }

    if (verb == "remove_row") {
        if (tokens.size() != 3 || tokens[1].empty() || tokens[2].empty()) {
            if (outError) {
                *outError = "remove_row expects lane_id:row_index";
            }
            return false;
        }
        ColorPipelineHeadlessRemoveRowAction action;
        action.lane_id = tokens[1];
        if (!TryParseNonNegativeIntText(tokens[2], &action.row_index)) {
            if (outError) {
                *outError = "remove_row requires a non-negative row index";
            }
            return false;
        }
        *outAction = std::move(action);
        return true;
    }

    if (verb == "set_param") {
        if (tokens.size() != 6 || tokens[1].empty() || tokens[2].empty() || tokens[3].empty() || tokens[4].empty() || tokens[5].empty()) {
            if (outError) {
                *outError = "set_param expects lane_id:row_index:param_path:number|bool|enum:value";
            }
            return false;
        }
        ColorPipelineHeadlessSetParamAction action;
        action.lane_id = tokens[1];
        if (!TryParseNonNegativeIntText(tokens[2], &action.row_index)) {
            if (outError) {
                *outError = "set_param requires a non-negative row index";
            }
            return false;
        }
        action.param_path = tokens[3];
        const std::string valueType = LowercaseAscii(tokens[4]);
        if (valueType == "number") {
            double value = 0.0;
            if (!TryParseFiniteDoubleText(tokens[5], &value)) {
                if (outError) {
                    *outError = "set_param:number requires a finite numeric value";
                }
                return false;
            }
            action.value = value;
        } else if (valueType == "bool") {
            bool value = false;
            if (!TryParseBoolText(tokens[5], &value)) {
                if (outError) {
                    *outError = "set_param:bool requires true, false, 1, or 0";
                }
                return false;
            }
            action.value = value;
        } else if (valueType == "enum") {
            action.value = tokens[5];
        } else {
            if (outError) {
                *outError = "set_param value type must be number, bool, or enum";
            }
            return false;
        }
        *outAction = std::move(action);
        return true;
    }

    if (outError) {
        *outError = std::string("Unknown advanced color action verb '") + verb + "'";
    }
    return false;
}

static bool TryParseColorPipelineActionSpecs(
    const std::vector<std::string>& specs,
    std::vector<ColorPipelineHeadlessAction>* outActions,
    std::string* outError) {
    if (outActions) {
        outActions->clear();
    }
    if (outError) {
        outError->clear();
    }
    if (!outActions) {
        if (outError) {
            *outError = "Advanced color action parsing requires output storage.";
        }
        return false;
    }

    for (std::size_t index = 0; index < specs.size(); ++index) {
        ColorPipelineHeadlessAction action;
        std::string error;
        if (!TryParseColorPipelineActionSpec(specs[index], &action, &error)) {
            if (outError) {
                std::ostringstream message;
                message << "Invalid --color-pipeline-action[" << index << "] '" << specs[index] << "': " << error;
                *outError = message.str();
            }
            return false;
        }
        outActions->push_back(std::move(action));
    }
    return true;
}

static bool TryParseHeadlessProofArgs(const std::vector<std::string>& args, ViewerCliArgs* out) {
    if (!TryStr(args, "--load-state-json", &out->have_load_state_json, &out->load_state_json)) return false;

    std::vector<std::string> colorPipelineActionSpecs;
    if (!TryGetArgValues(args, "--color-pipeline-action", &colorPipelineActionSpecs)) return false;
    if (!colorPipelineActionSpecs.empty()) {
        std::string colorPipelineActionError;
        if (!TryParseColorPipelineActionSpecs(
                colorPipelineActionSpecs,
                &out->color_pipeline_headless_proof.actions,
                &colorPipelineActionError)) {
            if (!colorPipelineActionError.empty()) {
                std::fprintf(stderr, "%s\n", colorPipelineActionError.c_str());
            }
            return false;
        }
    }

    if (!TryInt(args, "--sidecar-apply-armed-step-count", &out->have_sidecar_apply_armed_step_count, &out->sidecar_apply_armed_step_count)) return false;
    if (!TryInt(args, "--sidecar-replay-mutation-history-count", &out->have_sidecar_replay_mutation_history_count, &out->sidecar_replay_mutation_history_count)) return false;
    if (!TryDouble(args, "--sidecar-pump-paced-loop-seconds", &out->have_sidecar_pump_paced_loop_seconds, &out->sidecar_pump_paced_loop_seconds)) return false;
    return true;
}

static bool TryParseMetadataHeadlessArgs(const std::vector<std::string>& args, ViewerCliArgs* out) {
    out->describe_functions = HasArg(args, "--describe-functions");
    out->describe_parameter_surface = HasArg(args, "--describe-parameter-surface");
    out->describe_explaino_axis_registry = HasArg(args, "--describe-explaino-axis-registry");
    out->validate_ui_salt_contract = HasArg(args, "--validate-ui-salt-contract");
    out->explore_recommend = HasArg(args, "--explore-recommend");
    if (!TryStr(args, "--describe-functions-json", &out->have_describe_functions_json, &out->describe_functions_json_path)) return false;
    if (!TryStr(args, "--describe-parameter-surface-json", &out->have_describe_parameter_surface_json, &out->describe_parameter_surface_json_path)) return false;
    if (!TryStr(args, "--describe-explaino-axis-registry-json", &out->have_describe_explaino_axis_registry_json, &out->describe_explaino_axis_registry_json_path)) return false;
    if (!TryStr(args, "--ui-salt-contract-json", &out->have_ui_salt_contract_json, &out->ui_salt_contract_json_path)) return false;
    if (!TryStr(args, "--ui-salt-contract-report-json", &out->have_ui_salt_contract_report_json, &out->ui_salt_contract_report_json_path)) return false;
    if (!TryStr(args, "--explore-recommend-json", &out->have_explore_recommend_json, &out->explore_recommend_json_path)) return false;
    return true;
}

int ParseViewerCli(const std::vector<std::string>& args, ViewerCliArgs* out) {
    *out = ViewerCliArgs{};

    // Mode flags
    out->validate_ui_only = HasArg(args, "--validate-ui");
    out->capture_diagnostic_only = HasArg(args, "--capture-diagnostic");
    out->capture_finding_only = HasArg(args, "--capture-finding");
    if (!TryParseMetadataHeadlessArgs(args, out)) return 1;
    if (!TryStr(args, "--diagnostics-out-dir", &out->have_diagnostics_out_dir, &out->diagnostics_out_dir)) return 1;
    if (!TryStr(args, "--out-dir", &out->have_diagnostics_out_dir_alias, &out->diagnostics_out_dir_alias)) return 1;
    if (out->have_diagnostics_out_dir && out->have_diagnostics_out_dir_alias &&
            out->diagnostics_out_dir != out->diagnostics_out_dir_alias) {
        std::fprintf(stderr, "--diagnostics-out-dir and --out-dir must match when both are provided\n");
        return 1;
    }
    if (!out->have_diagnostics_out_dir && out->have_diagnostics_out_dir_alias) {
        out->have_diagnostics_out_dir = true;
        out->diagnostics_out_dir = out->diagnostics_out_dir_alias;
    }

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

    if (!TryParseFlashlightArgs(args, out)) return 1;
    if (!TryParseRuntimeWalkArgs(args, out)) return 1;

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
    if (!TryParseHeadlessProofArgs(args, out)) return 1;

    // Finding overrides
    if (!TryStr(args, "--finding-group", &out->have_finding_group, &out->finding_group)) return 1;
    if (!TryStr(args, "--finding-why", &out->have_finding_why, &out->finding_why)) return 1;

    if (!TryParseSweepConfig(args, &out->sweep_config)) return 1;

    return 0;
}
