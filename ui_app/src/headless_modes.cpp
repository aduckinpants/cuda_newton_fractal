#include "headless_modes.h"

#include <Windows.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>

#include "explaino_sidecar_controller.h"
#include "explaino_exploration_advisor.h"
#include "explaino_sidecar_measurement.h"
#include "explaino_sidecar_window.h"
#include "fractal_derived_fields.h"
#include "fractal_family_rules.h"
#include "fractal_probe_runner.h"
#include "function_descriptor.h"
#include "json_min.h"
#include "schema_binding.h"
#include "ui_schema.h"
#include "viewer_schema_load.h"

namespace {

void PrepareHeadlessSidecarState(ViewState& view, KernelParams& params) {
    if (IsExplainoFamily(view.fractal_type)) {
        UpdateExplainoPolynomial(view, params, nullptr);
    }
    if (view.auto_max_iter) {
        params.max_iter = ComputeAutoMaxIter(view.log2_zoom, view.fractal_type);
    }
}

SidecarAutoDemoControllerDecision BuildReplayDecision(
    const SidecarAutoDemoMutationRecord& record) {
    SidecarAutoDemoControllerDecision decision;
    decision.status = SidecarAutoDemoControllerStatus::apply_ready;
    decision.label = record.label;
    decision.path = record.path;
    decision.type = record.type;
    decision.utility = record.utility;
    decision.target_value = record.target_value;
    decision.has_target_value = true;
    decision.should_mutate = true;
    return decision;
}

bool RebuildHeadlessSidecarState(
    ViewState& view,
    KernelParams& params,
    const EngineFunctionCatalog& engineCatalog,
    BindingContext& bind,
    SidecarMeasurementHost& measurementHost,
    const SidecarAutoDemoControllerPolicy& sidecarControllerPolicy,
    SidecarOrientationVector& loadedOrientationBaseline,
    bool& loadedOrientationBaselineValid,
    ExplainoSidecarWindowState& sidecarState,
    bool& sidecarStateValid,
    SidecarBudgetState& sidecarBudgetState,
    bool& sidecarBudgetStateValid,
    std::string* outError) {
    PrepareHeadlessSidecarState(view, params);

    const bool usingLoadedOrientationBaseline = !sidecarStateValid && loadedOrientationBaselineValid;
    const SidecarOrientationVector* previousOrientation = nullptr;
    if (sidecarStateValid && sidecarState.has_orientation) {
        previousOrientation = &sidecarState.orientation;
    } else if (usingLoadedOrientationBaseline) {
        previousOrientation = &loadedOrientationBaseline;
    }

    ExplainoSidecarWindowState nextState;
    if (!BuildExplainoSidecarWindowState(
            engineCatalog,
            bind,
            &measurementHost,
            sidecarBudgetStateValid ? &sidecarBudgetState : nullptr,
            sidecarStateValid ? &sidecarState.completeness : nullptr,
            previousOrientation,
            (sidecarStateValid && !sidecarState.trace.function_id.empty()) ? &sidecarState.trace : nullptr,
            &sidecarControllerPolicy,
            &nextState,
            outError)) {
        return false;
    }

    sidecarState = std::move(nextState);
    if (!sidecarState.budget.function_id.empty()) {
        sidecarBudgetState = sidecarState.budget;
        sidecarBudgetStateValid = true;
    } else {
        sidecarBudgetState = {};
        sidecarBudgetStateValid = false;
    }
    sidecarStateValid = true;
    if (usingLoadedOrientationBaseline && sidecarState.has_orientation) {
        loadedOrientationBaseline = {};
        loadedOrientationBaselineValid = false;
    }
    return true;
}

void RecordAppliedSidecarMutation(
    const SidecarAutoDemoControllerDecision& decision,
    SidecarAutoDemoMutationHistory& sidecarMutationHistory,
    bool& sidecarMutationHistoryValid) {
    if (!sidecarMutationHistoryValid) {
        sidecarMutationHistory.clear();
        sidecarMutationHistoryValid = true;
    }
    sidecarMutationHistory.push_back(BuildSidecarAutoDemoMutationRecord(decision));
}

bool ApplyHeadlessArmedSteps(
    int stepCount,
    ViewState& view,
    KernelParams& params,
    const EngineFunctionCatalog& engineCatalog,
    BindingContext& bind,
    SidecarMeasurementHost& measurementHost,
    const SidecarAutoDemoControllerPolicy& sidecarControllerPolicy,
    SidecarOrientationVector& loadedOrientationBaseline,
    bool& loadedOrientationBaselineValid,
    SidecarAutoDemoMutationHistory& sidecarMutationHistory,
    bool& sidecarMutationHistoryValid,
    ExplainoSidecarWindowState& sidecarState,
    bool& sidecarStateValid,
    SidecarBudgetState& sidecarBudgetState,
    bool& sidecarBudgetStateValid,
    std::string* outError) {
    for (int stepIndex = 0; stepIndex < stepCount; ++stepIndex) {
        if (!RebuildHeadlessSidecarState(
                view,
                params,
                engineCatalog,
                bind,
                measurementHost,
                sidecarControllerPolicy,
                loadedOrientationBaseline,
                loadedOrientationBaselineValid,
                sidecarState,
                sidecarStateValid,
                sidecarBudgetState,
                sidecarBudgetStateValid,
                outError)) {
            return false;
        }

        if (!sidecarState.controller_decision.should_mutate) {
            return true;
        }

        bool changed = false;
        if (!ApplySidecarAutoDemoControllerDecision(sidecarState.controller_decision, bind, &changed, outError)) {
            return false;
        }
        if (!changed) {
            return true;
        }

        RecordAppliedSidecarMutation(
            sidecarState.controller_decision,
            sidecarMutationHistory,
            sidecarMutationHistoryValid);

        if (!RebuildHeadlessSidecarState(
                view,
                params,
                engineCatalog,
                bind,
                measurementHost,
                sidecarControllerPolicy,
                loadedOrientationBaseline,
                loadedOrientationBaselineValid,
                sidecarState,
                sidecarStateValid,
                sidecarBudgetState,
                sidecarBudgetStateValid,
                outError)) {
            return false;
        }
    }
    return true;
}

bool PumpHeadlessPacedLoop(
    double totalSeconds,
    ViewState& view,
    KernelParams& params,
    const EngineFunctionCatalog& engineCatalog,
    BindingContext& bind,
    SidecarMeasurementHost& measurementHost,
    const SidecarAutoDemoControllerPolicy& sidecarControllerPolicy,
    SidecarOrientationVector& loadedOrientationBaseline,
    bool& loadedOrientationBaselineValid,
    SidecarAutoDemoMutationHistory& sidecarMutationHistory,
    bool& sidecarMutationHistoryValid,
    ExplainoSidecarWindowState& sidecarState,
    bool& sidecarStateValid,
    SidecarBudgetState& sidecarBudgetState,
    bool& sidecarBudgetStateValid,
    std::string* outError) {
    constexpr double kHeadlessLoopTickSeconds = 1.0 / 60.0;

    if (!RebuildHeadlessSidecarState(
            view,
            params,
            engineCatalog,
            bind,
            measurementHost,
            sidecarControllerPolicy,
            loadedOrientationBaseline,
            loadedOrientationBaselineValid,
            sidecarState,
            sidecarStateValid,
            sidecarBudgetState,
            sidecarBudgetStateValid,
            outError)) {
        return false;
    }

    SidecarAutoDemoLoopState loopState{};
    double elapsedSeconds = 0.0;
    while (elapsedSeconds + 1.0e-12 < totalSeconds) {
        const double remainingSeconds = totalSeconds - elapsedSeconds;
        const double deltaSeconds = remainingSeconds < kHeadlessLoopTickSeconds
            ? remainingSeconds
            : kHeadlessLoopTickSeconds;
        elapsedSeconds += deltaSeconds;

        bool shouldApply = false;
        if (!AdvanceSidecarAutoDemoLoop(
                sidecarState.controller_decision,
                sidecarControllerPolicy,
                deltaSeconds,
                false,
                &loopState,
                &shouldApply,
                outError)) {
            return false;
        }

        if (!shouldApply) {
            continue;
        }

        bool changed = false;
        if (!ApplySidecarAutoDemoControllerDecision(sidecarState.controller_decision, bind, &changed, outError)) {
            return false;
        }
        ResetSidecarAutoDemoLoopState(&loopState);
        if (!changed) {
            continue;
        }

        RecordAppliedSidecarMutation(
            sidecarState.controller_decision,
            sidecarMutationHistory,
            sidecarMutationHistoryValid);

        if (!RebuildHeadlessSidecarState(
                view,
                params,
                engineCatalog,
                bind,
                measurementHost,
                sidecarControllerPolicy,
                loadedOrientationBaseline,
                loadedOrientationBaselineValid,
                sidecarState,
                sidecarStateValid,
                sidecarBudgetState,
                sidecarBudgetStateValid,
                outError)) {
            return false;
        }
    }

    return true;
}

} // namespace

bool ReplayLoadedSidecarMutationHistory(
    int replayMutationHistoryCount,
    ViewState& view,
    KernelParams& params,
    BindingContext& bind,
    const SidecarAutoDemoMutationHistory& sidecarMutationHistory,
    std::string* outError) {
    if (outError) outError->clear();

    if (replayMutationHistoryCount < 0) {
        if (outError) *outError = "sidecar headless proof replay_mutation_history_count must be >= 0";
        return false;
    }
    if (replayMutationHistoryCount == 0) {
        return true;
    }
    if (sidecarMutationHistory.empty()) {
        if (outError) *outError = "sidecar headless proof replay_mutation_history_count requires persisted sidecar mutation history";
        return false;
    }

    const size_t replayCount = static_cast<size_t>(replayMutationHistoryCount);
    if (replayCount > sidecarMutationHistory.size()) {
        if (outError) {
            *outError = "sidecar headless proof replay_mutation_history_count exceeds persisted sidecar mutation history length";
        }
        return false;
    }

    for (size_t index = 0; index < replayCount; ++index) {
        bool changed = false;
        const SidecarAutoDemoControllerDecision decision = BuildReplayDecision(sidecarMutationHistory[index]);
        if (!ApplySidecarAutoDemoControllerDecision(decision, bind, &changed, outError)) {
            return false;
        }
        PrepareHeadlessSidecarState(view, params);
    }

    return true;
}

// --- File I/O utilities ---

std::string ReadTextFile(const char* path) {
    std::ifstream f(path, std::ios::in | std::ios::binary);
    if (!f) return {};
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

bool TryReadTextFileExact(const std::string& path, std::string* outText, std::string* outError) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        if (outError) *outError = "Failed to open sample request file: " + path;
        return false;
    }
    std::ostringstream text;
    text << file.rdbuf();
    if (!file.good() && !file.eof()) {
        if (outError) *outError = "Failed to read sample request file: " + path;
        return false;
    }
    if (outText) *outText = text.str();
    return true;
}

bool ReadStdinText(std::string* outText, std::string* outError) {
    std::ostringstream text;
    text << std::cin.rdbuf();
    if (!std::cin.good() && !std::cin.eof()) {
        if (outError) *outError = "Failed to read sample request from stdin";
        return false;
    }
    if (outText) *outText = text.str();
    return true;
}

bool WriteTextFileExact(const std::string& path, const std::string& text, std::string* outError) {
    std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file) {
        if (outError) *outError = "Failed to open sample response file for write: " + path;
        return false;
    }
    file.write(text.data(), static_cast<std::streamsize>(text.size()));
    if (!file.good()) {
        if (outError) *outError = "Failed to write sample response file: " + path;
        return false;
    }
    return true;
}

// --- Probe helpers ---

FractalProbeResponse BuildProbeErrorResponse(const std::string& requestId,
    const std::string& exePath,
    const FractalProbeOperatorContext& operatorContext,
    const std::string& error) {
    FractalProbeResponse response;
    response.request_id = requestId;
    response.ok = false;
    response.runtime.exe_path = exePath;
    response.operator_context = operatorContext;
    response.error = error;
    return response;
}

bool EmitProbeResponse(const std::string& responseJson,
    bool toStdout,
    const std::string& responsePath,
    std::string* outError) {
    if (toStdout) {
        if (std::fwrite(responseJson.data(), 1, responseJson.size(), stdout) != responseJson.size()) {
            if (outError) *outError = "Failed to write JSON output to stdout";
            return false;
        }
        std::fflush(stdout);
    }
    if (!responsePath.empty()) {
        if (!WriteTextFileExact(responsePath, responseJson, outError)) return false;
    }
    return true;
}

bool UsesGridRows(FractalProbeMode mode) {
    return mode == FractalProbeMode::grid;
}

bool EmitSampleModeResponse(const SampleModeArgs& args,
    const std::string& responseText,
    std::string* outError) {
    return EmitProbeResponse(responseText,
        args.response_stdout,
        args.response_json_path,
        outError);
}

std::string BuildNdjsonProbeResponse(const FractalProbeRequest& request,
    const FractalProbeResponse& response,
    const std::string& stateToken) {
    std::ostringstream out;
    const bool groupByRow = UsesGridRows(request.mode);
    size_t batchStart = 0;
    bool wroteLine = false;

    while (batchStart < response.samples.size()) {
        const int batchSequenceIndex = response.samples[batchStart].sequence_index;
        const int batchRowIndex = groupByRow ? response.samples[batchStart].grid_y : -1;
        size_t batchEnd = batchStart + 1;
        while (batchEnd < response.samples.size()) {
            const FractalProbeSample& sample = response.samples[batchEnd];
            if (sample.sequence_index != batchSequenceIndex) break;
            if (groupByRow && sample.grid_y != batchRowIndex) break;
            ++batchEnd;
        }

        if (wroteLine) out << "\n";
        out << SerializeFractalProbeNdjsonSampleBatchJson(
            response.request_id,
            response.function_id,
            batchSequenceIndex,
            batchRowIndex,
            std::vector<FractalProbeSample>(response.samples.begin() + batchStart,
                response.samples.begin() + batchEnd),
            response.metric_selection);
        wroteLine = true;
        batchStart = batchEnd;
    }

    if (wroteLine) out << "\n";
    out << SerializeFractalProbeNdjsonSummaryJson(response, stateToken);
    return out.str();
}

int RunSingleSampleRequest(const json_min::Value& requestValue,
    const SampleModeArgs& args,
    const std::string& exePath) {
    std::string error;
    FractalProbeRequest request;
    if (!ParseFractalProbeRequestFromValue(requestValue, &request, &error)) {
        FractalProbeResponse response = BuildProbeErrorResponse(
            std::string(), exePath, FractalProbeOperatorContext{}, error);
        std::string emitError;
        if (!EmitSampleModeResponse(args, SerializeFractalProbeResponseJson(response), &emitError)) {
            std::fprintf(stderr, "%s\n", emitError.c_str());
        }
        return 1;
    }

    FractalProbeResponse response;
    if (!RunFractalProbeRequest(request, exePath, &response, &error)) {
        response = BuildProbeErrorResponse(request.request_id, exePath, request.operator_context, error);
    }

    const std::string responseText =
        request.output_mode == FractalProbeOutputMode::ndjson && response.ok
        ? BuildNdjsonProbeResponse(request, response, std::string())
        : SerializeFractalProbeResponseJson(response);

    std::string emitError;
    if (!EmitSampleModeResponse(args, responseText, &emitError)) {
        std::fprintf(stderr, "%s\n", emitError.c_str());
        return 1;
    }
    return response.ok ? 0 : 1;
}

int RunBatchSampleRequests(const json_min::Array& requestArray,
    const SampleModeArgs& args,
    const std::string& exePath) {
    bool anyFailed = false;
    std::string batchJson = "[";
    for (size_t i = 0; i < requestArray.size(); ++i) {
        FractalProbeRequest request;
        std::string reqError;
        FractalProbeResponse response;
        if (!ParseFractalProbeRequestFromValue(requestArray[i], &request, &reqError)) {
            std::string failedRequestId;
            if (requestArray[i].is_object()) {
                auto idIt = requestArray[i].as_object().find("request_id");
                if (idIt != requestArray[i].as_object().end() && idIt->second.is_string()) {
                    failedRequestId = idIt->second.as_string();
                }
            }
            response = BuildProbeErrorResponse(failedRequestId, exePath, FractalProbeOperatorContext{}, reqError);
            anyFailed = true;
        } else if (request.output_mode == FractalProbeOutputMode::ndjson) {
            response = BuildProbeErrorResponse(
                request.request_id,
                exePath,
                request.operator_context,
                "output_mode ndjson is not allowed inside batch request arrays");
            anyFailed = true;
        } else {
            std::string runError;
            if (!RunFractalProbeRequest(request, exePath, &response, &runError)) {
                response = BuildProbeErrorResponse(request.request_id, exePath, request.operator_context, runError);
                anyFailed = true;
            }
        }
        if (i > 0) batchJson += ",";
        batchJson += SerializeFractalProbeResponseJson(response);
    }
    batchJson += "]";

    std::string emitError;
    if (!EmitSampleModeResponse(args, batchJson, &emitError)) {
        std::fprintf(stderr, "%s\n", emitError.c_str());
        return 1;
    }
    return anyFailed ? 1 : 0;
}

// --- Headless mode dispatch ---

int RunSampleMode(const SampleModeArgs& args, const std::string& exePath) {
    const bool haveRequestJson = !args.request_json_path.empty();
    const bool haveResponseJson = !args.response_json_path.empty();
    const int sourceCount = (args.request_stdin ? 1 : 0) + (haveRequestJson ? 1 : 0);

    std::string error;
    std::string requestText;

    // --- Validate args ---
    if (sourceCount != 1) {
        error = "sample mode requires exactly one request source";
    } else if (!args.response_stdout && !haveResponseJson) {
        error = "sample mode requires at least one response sink";
    } else if (args.conflict_validate_ui || args.conflict_capture_diagnostic || args.conflict_capture_finding) {
        error = "sample mode is mutually exclusive with --validate-ui, --capture-diagnostic, and --capture-finding";
    } else if (!(args.request_stdin
            ? ReadStdinText(&requestText, &error)
            : TryReadTextFileExact(args.request_json_path, &requestText, &error))) {
        // error already set by the read function
    } else {
        // --- Parse root JSON to detect object vs array ---
        json_min::ParseResult parsed = json_min::Parse(requestText);
        if (!parsed.error.empty()) {
            error = parsed.error;
        } else if (parsed.value.is_object()) {
            return RunSingleSampleRequest(parsed.value, args, exePath);
        } else if (parsed.value.is_array()) {
            return RunBatchSampleRequests(parsed.value.as_array(), args, exePath);
        } else {
            error = "Probe request root must be an object or array";
        }
    }

    // --- Error fallback (pre-parse or structural errors) ---
    FractalProbeResponse response = BuildProbeErrorResponse(
        std::string(), exePath, FractalProbeOperatorContext{}, error);
    const std::string responseJson = SerializeFractalProbeResponseJson(response);
    std::string emitError;
    if ((args.response_stdout || haveResponseJson) &&
        !EmitSampleModeResponse(args, responseJson, &emitError)) {
        std::fprintf(stderr, "%s\n", emitError.c_str());
    }
    std::fprintf(stderr, "%s\n", error.c_str());
    return 1;
}

int RunDescribeFunctionsMode(bool toStdout, const std::string& jsonPath,
    const std::vector<std::string>& schemaCandidates) {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
    BindingContext bind;
    bind.view = &view;
    bind.params = &params;
    bind.render = &render;
    bind.lens = &lens;

    SchemaLoadResult schemaResult = LoadAndValidateViewerSchema(schemaCandidates, bind, false);
    if (schemaResult.fatal_error) {
        if (!schemaResult.warning.empty()) {
            std::fprintf(stderr, "%s\n", schemaResult.warning.c_str());
        }
        return 1;
    }
    if (!schemaResult.warning.empty()) {
        std::fprintf(stderr, "%s\n", schemaResult.warning.c_str());
    }

    EngineFunctionCatalog catalog = BuildEngineCatalog(schemaResult.schema);
    std::string catalogJson = SerializeEngineCatalogJson(catalog);

    std::string emitError;
    if (!EmitProbeResponse(catalogJson, toStdout, jsonPath, &emitError)) {
        std::fprintf(stderr, "%s\n", emitError.c_str());
        return 1;
    }
    return 0;
}

int RunExploreRecommendMode(
    bool toStdout,
    const std::string& jsonPath,
    ViewState& view,
    KernelParams& params,
    RenderSettings& render,
    const EngineFunctionCatalog& engineCatalog,
    BindingContext& bind,
    SidecarMeasurementHost& measurementHost) {
    if (!toStdout && jsonPath.empty()) {
        std::fprintf(stderr, "explore-recommend mode requires at least one output sink\n");
        return 1;
    }

    PrepareHeadlessSidecarState(view, params);

    ExplainoExplorationAdvisorReport report;
    std::string error;
    if (!BuildExplainoExplorationAdvisorReport(engineCatalog, bind, measurementHost, &report, &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    const std::string reportJson = SerializeExplainoExplorationAdvisorReportJson(report);
    std::string emitError;
    if (!EmitProbeResponse(reportJson, toStdout, jsonPath, &emitError)) {
        std::fprintf(stderr, "%s\n", emitError.c_str());
        return 1;
    }
    return 0;
}

bool HasSidecarHeadlessProofActions(const SidecarHeadlessProofConfig& config) {
    return config.apply_armed_step_count > 0 ||
        config.replay_mutation_history_count > 0 ||
        config.pump_paced_loop_seconds > 0.0;
}

bool ApplyHeadlessSidecarProofActions(
    const SidecarHeadlessProofConfig& config,
    ViewState& view,
    KernelParams& params,
    const EngineFunctionCatalog& engineCatalog,
    BindingContext& bind,
    SidecarMeasurementHost& measurementHost,
    const SidecarAutoDemoControllerPolicy& sidecarControllerPolicy,
    SidecarOrientationVector& loadedOrientationBaseline,
    bool& loadedOrientationBaselineValid,
    SidecarAutoDemoMutationHistory& sidecarMutationHistory,
    bool& sidecarMutationHistoryValid,
    ExplainoSidecarWindowState& sidecarState,
    bool& sidecarStateValid,
    SidecarBudgetState& sidecarBudgetState,
    bool& sidecarBudgetStateValid,
    std::string* outError) {
    if (outError) outError->clear();

    if (config.apply_armed_step_count < 0) {
        if (outError) *outError = "sidecar headless proof apply_armed_step_count must be >= 0";
        return false;
    }
    if (config.replay_mutation_history_count < 0) {
        if (outError) *outError = "sidecar headless proof replay_mutation_history_count must be >= 0";
        return false;
    }
    if (!std::isfinite(config.pump_paced_loop_seconds) || config.pump_paced_loop_seconds < 0.0) {
        if (outError) *outError = "sidecar headless proof pump_paced_loop_seconds must be finite and >= 0";
        return false;
    }

    if (config.replay_mutation_history_count > 0) {
        if (!sidecarMutationHistoryValid) {
            if (outError) *outError = "sidecar headless proof replay_mutation_history_count requires loaded sidecar mutation history";
            return false;
        }
        if (!ReplayLoadedSidecarMutationHistory(
                config.replay_mutation_history_count,
                view,
                params,
                bind,
                sidecarMutationHistory,
                outError)) {
            return false;
        }

        sidecarState = {};
        sidecarStateValid = false;
        sidecarBudgetState = {};
        sidecarBudgetStateValid = false;

        if (config.apply_armed_step_count == 0 && config.pump_paced_loop_seconds == 0.0) {
            if (!RebuildHeadlessSidecarState(
                    view,
                    params,
                    engineCatalog,
                    bind,
                    measurementHost,
                    sidecarControllerPolicy,
                    loadedOrientationBaseline,
                    loadedOrientationBaselineValid,
                    sidecarState,
                    sidecarStateValid,
                    sidecarBudgetState,
                    sidecarBudgetStateValid,
                    outError)) {
                return false;
            }
        }
    }

    if (config.apply_armed_step_count > 0) {
        if (!ApplyHeadlessArmedSteps(
                config.apply_armed_step_count,
                view,
                params,
                engineCatalog,
                bind,
                measurementHost,
                sidecarControllerPolicy,
                loadedOrientationBaseline,
                loadedOrientationBaselineValid,
                sidecarMutationHistory,
                sidecarMutationHistoryValid,
                sidecarState,
                sidecarStateValid,
                sidecarBudgetState,
                sidecarBudgetStateValid,
                outError)) {
            return false;
        }
    }

    if (config.pump_paced_loop_seconds > 0.0) {
        if (!PumpHeadlessPacedLoop(
                config.pump_paced_loop_seconds,
                view,
                params,
                engineCatalog,
                bind,
                measurementHost,
                sidecarControllerPolicy,
                loadedOrientationBaseline,
                loadedOrientationBaselineValid,
                sidecarMutationHistory,
                sidecarMutationHistoryValid,
                sidecarState,
                sidecarStateValid,
                sidecarBudgetState,
                sidecarBudgetStateValid,
                outError)) {
            return false;
        }
    }

    return true;
}

// --- Session mode (V2-B / V2-C) ---

// --- V2-C: Override merge ---

std::vector<FractalProbeOverride> MergeOverrides(
    const std::vector<FractalProbeOverride>& base,
    const std::vector<FractalProbeOverride>& diff) {
    if (diff.empty()) return base;
    if (base.empty()) return diff;

    // Copy base, then apply diff: replace matching paths, append new ones.
    std::vector<FractalProbeOverride> merged = base;
    for (const auto& d : diff) {
        bool replaced = false;
        for (auto& m : merged) {
            if (m.path == d.path) {
                m.value = d.value;
                replaced = true;
                break;
            }
        }
        if (!replaced) {
            merged.push_back(d);
        }
    }
    return merged;
}

static std::string MakeStateToken(int counter) {
    return "s" + std::to_string(counter);
}

// Escape a string for JSON embedding (minimal: quotes, backslash, control chars).
static std::string JsonEscapeString(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", (unsigned char)c);
                    out += buf;
                } else {
                    out += c;
                }
        }
    }
    return out;
}

static std::string BuildSessionError(const std::string& error) {
    return "{\"ok\":false,\"error\":\"" + JsonEscapeString(error) + "\"}";
}

// Compact pretty-printed JSON to a single line (for session NDJSON output).
static std::string CompactJson(const std::string& json) {
    std::string result;
    result.reserve(json.size());
    bool inString = false;
    bool prevBackslash = false;
    for (size_t i = 0; i < json.size(); ++i) {
        char c = json[i];
        if (inString) {
            result += c;
            if (c == '\\' && !prevBackslash) { prevBackslash = true; continue; }
            if (c == '"' && !prevBackslash) inString = false;
            prevBackslash = false;
        } else {
            if (c == '"') { inString = true; result += c; }
            else if (c == '\n' || c == '\r' || c == ' ' || c == '\t') { /* skip whitespace outside strings */ }
            else { result += c; }
        }
    }
    return result;
}

std::string ProcessSessionLine(const std::string& line,
    bool* sessionOpen,
    bool* sessionDone,
    int* stateTokenCounter,
    SessionOverrideMap* accumulatedOverrides,
    const std::string& exePath) {

    // Skip empty/whitespace lines
    bool allWhitespace = true;
    for (char c : line) {
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') { allWhitespace = false; break; }
    }
    if (line.empty() || allWhitespace) return {};

    // Parse JSON
    json_min::ParseResult parsed = json_min::Parse(line);
    if (!parsed.error.empty()) {
        if (!*sessionOpen) {
            *sessionDone = true;
            return BuildSessionError("session not open and line is not valid JSON: " + parsed.error);
        }
        return BuildSessionError("JSON parse error: " + parsed.error);
    }

    if (!parsed.value.is_object()) {
        if (!*sessionOpen) {
            *sessionDone = true;
            return BuildSessionError("session not open and line is not a JSON object");
        }
        return BuildSessionError("session line must be a JSON object");
    }

    const auto& obj = parsed.value.as_object();

    // Check for session control message
    auto sessIt = obj.find("session");
    if (sessIt != obj.end() && sessIt->second.is_string()) {
        const std::string& verb = sessIt->second.as_string();

        if (verb == "open") {
            if (*sessionOpen) {
                return BuildSessionError("session already open");
            }
            *sessionOpen = true;
            std::string token = MakeStateToken(*stateTokenCounter);
            if (accumulatedOverrides) {
                accumulatedOverrides->clear();
                (*accumulatedOverrides)[token] = {};
            }
            return "{\"session\":\"ready\",\"state_token\":\"" + token + "\",\"engine_version\":2}";
        }

        if (verb == "close") {
            *sessionDone = true;
            return "{\"session\":\"closed\"}";
        }

        // Unknown session verb
        return BuildSessionError("unknown session verb: " + verb);
    }

    // Not a session control message — treat as sample request
    if (!*sessionOpen) {
        *sessionDone = true;
        return BuildSessionError("session not open");
    }

    // Parse and execute the probe request
    FractalProbeRequest request;
    std::string reqError;
    FractalProbeResponse response;

    if (!ParseFractalProbeRequestFromValue(parsed.value, &request, &reqError)) {
        std::string failedRequestId;
        auto idIt = obj.find("request_id");
        if (idIt != obj.end() && idIt->second.is_string()) {
            failedRequestId = idIt->second.as_string();
        }
        response = BuildProbeErrorResponse(failedRequestId, exePath, FractalProbeOperatorContext{}, reqError);
    } else {
        // V2-C: If state_token is present, merge accumulated overrides
        if (!request.state_token.empty() && accumulatedOverrides) {
            auto it = accumulatedOverrides->find(request.state_token);
            if (it == accumulatedOverrides->end()) {
                response = BuildProbeErrorResponse(request.request_id, exePath,
                    request.operator_context,
                    "unknown state_token: " + request.state_token);
                goto emit_response;
            }
            request.overrides = MergeOverrides(it->second, request.overrides);
        }

        std::string runError;
        if (!RunFractalProbeRequest(request, exePath, &response, &runError)) {
            response = BuildProbeErrorResponse(request.request_id, exePath, request.operator_context, runError);
        }
    }

emit_response:
    // Stamp response_version=2 and state_token
    response.response_version = 2;
    std::string token;
    if (response.ok) {
        (*stateTokenCounter)++;
        token = MakeStateToken(*stateTokenCounter);
    }
    std::string responseJson;
    if (request.output_mode == FractalProbeOutputMode::ndjson && response.ok) {
        responseJson = BuildNdjsonProbeResponse(request, response, token);
    } else {
        responseJson = CompactJson(SerializeFractalProbeResponseJson(response));
        size_t lastBrace = responseJson.rfind('}');
        if (!token.empty() && lastBrace != std::string::npos) {
            responseJson.insert(lastBrace, ",\"state_token\":\"" + token + "\"");
        }
    }

    // V2-C: Store accumulated overrides for this token
    if (accumulatedOverrides && response.ok) {
        (*accumulatedOverrides)[token] = request.overrides;
    }

    return responseJson;
}

int RunSessionMode(std::istream& in, std::ostream& out, const std::string& exePath) {
    bool sessionOpen = false;
    bool sessionDone = false;
    int stateTokenCounter = 0;
    SessionOverrideMap accumulatedOverrides;

    std::string line;
    while (std::getline(in, line)) {
        // Strip trailing \r from CRLF
        if (!line.empty() && line.back() == '\r') line.pop_back();

        std::string response = ProcessSessionLine(line, &sessionOpen, &sessionDone,
            &stateTokenCounter, &accumulatedOverrides, exePath);

        if (!response.empty()) {
            out << response << "\n";
            out.flush();
        }

        if (sessionDone) {
            // If the session was never opened, this is an error exit
            return sessionOpen ? 0 : 1;
        }
    }

    // EOF without close
    if (sessionOpen) {
        return 1;
    }
    return 1;
}

static bool IsAllowedSessionPipeNameChar(unsigned char ch) {
    return std::isalnum(ch) || ch == '_' || ch == '-' || ch == '.';
}

bool TryBuildSessionPipePath(const std::string& pipeName,
    std::string* outPipePath,
    std::string* outError) {
    if (pipeName.empty()) {
        if (outError) *outError = "sample session pipe name must not be empty";
        return false;
    }

    for (unsigned char ch : pipeName) {
        if (!IsAllowedSessionPipeNameChar(ch)) {
            if (outError) {
                *outError = "sample session pipe name must use only letters, numbers, dot, dash, or underscore";
            }
            return false;
        }
    }

    if (outPipePath) *outPipePath = "\\\\.\\pipe\\" + pipeName;
    return true;
}

static std::string FormatWin32ErrorMessage(DWORD errorCode) {
    LPSTR buffer = nullptr;
    const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS;
    const DWORD length = FormatMessageA(
        flags,
        nullptr,
        errorCode,
        0,
        reinterpret_cast<LPSTR>(&buffer),
        0,
        nullptr);
    std::string message;
    if (length != 0 && buffer) {
        message.assign(buffer, length);
        while (!message.empty() && (message.back() == '\r' || message.back() == '\n' || message.back() == ' ')) {
            message.pop_back();
        }
    } else {
        message = "Win32 error " + std::to_string(errorCode);
    }
    if (buffer) {
        LocalFree(buffer);
    }
    return message;
}

static bool WriteNamedPipeText(HANDLE pipe,
    const std::string& text,
    std::string* outError) {
    const char* next = text.data();
    size_t remaining = text.size();
    while (remaining > 0) {
        DWORD written = 0;
        const DWORD chunk = remaining > static_cast<size_t>(DWORD(-1))
            ? DWORD(-1)
            : static_cast<DWORD>(remaining);
        if (!WriteFile(pipe, next, chunk, &written, nullptr)) {
            if (outError) {
                *outError = "Failed to write named-pipe session response: "
                    + FormatWin32ErrorMessage(GetLastError());
            }
            return false;
        }
        next += written;
        remaining -= written;
    }

    if (!FlushFileBuffers(pipe)) {
        if (outError) {
            *outError = "Failed to flush named-pipe session response: "
                + FormatWin32ErrorMessage(GetLastError());
        }
        return false;
    }
    return true;
}

static bool ReadNamedPipeLine(HANDLE pipe,
    std::string* outLine,
    bool* outDisconnected,
    std::string* outError) {
    if (outLine) outLine->clear();
    if (outDisconnected) *outDisconnected = false;

    while (true) {
        char ch = 0;
        DWORD bytesRead = 0;
        if (!ReadFile(pipe, &ch, 1, &bytesRead, nullptr) || bytesRead == 0) {
            const DWORD errorCode = bytesRead == 0 ? ERROR_BROKEN_PIPE : GetLastError();
            if (errorCode == ERROR_BROKEN_PIPE || errorCode == ERROR_PIPE_NOT_CONNECTED || errorCode == ERROR_HANDLE_EOF) {
                if (outDisconnected) *outDisconnected = true;
                return false;
            }
            if (outError) {
                *outError = "Failed to read named-pipe session request: "
                    + FormatWin32ErrorMessage(errorCode);
            }
            return false;
        }

        if (ch == '\n') {
            return true;
        }
        if (ch != '\r' && outLine) {
            outLine->push_back(ch);
        }
    }
}

int RunNamedPipeSessionMode(const std::string& pipeName,
    const std::string& exePath) {
    std::string pipePath;
    std::string error;
    if (!TryBuildSessionPipePath(pipeName, &pipePath, &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    HANDLE pipe = CreateNamedPipeA(
        pipePath.c_str(),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1,
        65536,
        65536,
        0,
        nullptr);
    if (pipe == INVALID_HANDLE_VALUE) {
        std::fprintf(stderr, "Failed to create named-pipe session transport: %s\n",
            FormatWin32ErrorMessage(GetLastError()).c_str());
        return 1;
    }

    const auto cleanupPipe = [&]() {
        DisconnectNamedPipe(pipe);
        CloseHandle(pipe);
    };

    BOOL connected = ConnectNamedPipe(pipe, nullptr);
    if (!connected) {
        const DWORD errorCode = GetLastError();
        if (errorCode != ERROR_PIPE_CONNECTED) {
            std::fprintf(stderr, "Failed to connect named-pipe session transport: %s\n",
                FormatWin32ErrorMessage(errorCode).c_str());
            cleanupPipe();
            return 1;
        }
    }

    bool sessionOpen = false;
    bool sessionDone = false;
    int stateTokenCounter = 0;
    SessionOverrideMap accumulatedOverrides;

    while (true) {
        std::string line;
        bool disconnected = false;
        std::string readError;
        if (!ReadNamedPipeLine(pipe, &line, &disconnected, &readError)) {
            if (!readError.empty()) {
                std::fprintf(stderr, "%s\n", readError.c_str());
            }
            cleanupPipe();
            return 1;
        }

        std::string response = ProcessSessionLine(line,
            &sessionOpen,
            &sessionDone,
            &stateTokenCounter,
            &accumulatedOverrides,
            exePath);
        if (!response.empty()) {
            std::string writeError;
            if (!WriteNamedPipeText(pipe, response + "\n", &writeError)) {
                std::fprintf(stderr, "%s\n", writeError.c_str());
                cleanupPipe();
                return 1;
            }
        }

        if (sessionDone) {
            const int exitCode = sessionOpen ? 0 : 1;
            cleanupPipe();
            return exitCode;
        }

        if (disconnected) {
            cleanupPipe();
            return 1;
        }
    }
}
