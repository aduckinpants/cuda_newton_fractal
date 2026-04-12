#include "explaino_sidecar_window.h"

#include "imgui.h"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <unordered_map>

namespace {

std::string FormatNumber(double value) {
    std::ostringstream out;
    out << std::setprecision(6) << value;
    return out.str();
}

std::string FormatDefaultValue(const json_min::Value& value) {
    if (value.is_string()) return value.as_string();
    if (value.is_number()) return FormatNumber(value.as_number());
    if (value.is_bool()) return value.as_bool() ? "true" : "false";
    return "null";
}

std::string FormatRange(const SidecarParamSurfaceEntry& entry) {
    if (entry.has_min && entry.has_max) {
        return "[" + FormatNumber(entry.min_value) + ", " + FormatNumber(entry.max_value) + "]";
    }
    if (entry.has_min) {
        return ">= " + FormatNumber(entry.min_value);
    }
    if (entry.has_max) {
        return "<= " + FormatNumber(entry.max_value);
    }
    return std::string();
}

std::string FormatActiveZone(const SidecarLensProjectionRow& row) {
    return "[" + FormatNumber(row.active_min) + ", " + FormatNumber(row.active_max) + "]";
}

std::string FormatOptionalNumber(double value) {
    if (!std::isfinite(value)) {
        return "--";
    }
    return FormatNumber(value);
}

std::string EnergyStatusText(SidecarEnergyLandscapeRowStatus status) {
    switch (status) {
    case SidecarEnergyLandscapeRowStatus::available:
        return "available";
    case SidecarEnergyLandscapeRowStatus::inactive:
        return "inactive";
    case SidecarEnergyLandscapeRowStatus::missing_cost_hint:
        return "missing_cost_hint";
    case SidecarEnergyLandscapeRowStatus::unsupported_type:
        return "unsupported_type";
    }
    return "unknown";
}

std::string FormatTraceOverlay(
    const std::string& path,
    const SidecarSlimeTrace& trace) {
    for (const auto& visit : trace.visit_counts) {
        if (visit.path != path) {
            continue;
        }
        std::ostringstream out;
        out << "x" << visit.visit_count << " @#" << visit.latest_step_index;
        if (visit.latest_should_mutate) {
            out << " apply";
        } else {
            out << " proposal";
        }
        return out.str();
    }
    return "--";
}

void RenderOrientationSection(const ExplainoSidecarWindowState& state) {
    ImGui::Separator();
    ImGui::Text("Orientation");
    ImGui::BulletText("import_signature: %llu", static_cast<unsigned long long>(state.orientation.import_signature));
    ImGui::BulletText("pack_projection_hash: %llu", static_cast<unsigned long long>(state.orientation.pack_projection_hash));
    ImGui::BulletText("field_embedding_stats: %.3f", state.orientation.field_embedding_stats);
    ImGui::BulletText("slime_energy_delta: %.3f", state.orientation.slime_energy_delta);
    ImGui::BulletText("busy_beaver_metrics: %.3f", state.orientation.busy_beaver_metrics);
    ImGui::BulletText("decode_stability: %.3f", state.orientation.decode_stability);
    ImGui::BulletText("diff_magnitude: %.3f", state.orientation.diff_magnitude);
}

void RenderBudgetSection(const ExplainoSidecarWindowState& state) {
    ImGui::Separator();
    ImGui::Text("Information Budget");
    if (state.budget.rows.empty()) {
        ImGui::TextDisabled("Measurement unavailable.");
        return;
    }

    ImGui::BulletText("estimated_information_gain_total: %.3f", state.budget.estimated_information_gain_total);
    ImGui::BulletText("cumulative_information_gain_total: %.3f", state.budget.cumulative_information_gain_total);
    ImGui::BulletText("mean_posterior_uncertainty: %.3f", state.budget.mean_posterior_uncertainty);
    ImGui::BulletText("mean_decode_stability: %.3f", state.budget.mean_decode_stability);
    ImGui::BulletText("budget_batches: %d", state.budget.batch_count);

    if (!ImGui::BeginTable("sidecar_budget", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
        return;
    }

    ImGui::TableSetupColumn("Label");
    ImGui::TableSetupColumn("Path");
    ImGui::TableSetupColumn("EIG");
    ImGui::TableSetupColumn("Cumulative");
    ImGui::TableSetupColumn("Uncertainty");
    ImGui::TableSetupColumn("Obs");
    ImGui::TableSetupColumn("Zone");
    ImGui::TableSetupColumn("Guidance");
    ImGui::TableHeadersRow();

    for (size_t index = 0; index < state.budget.rows.size(); ++index) {
        const auto& row = state.budget.rows[index];
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(row.label.c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(row.path.c_str());
        ImGui::TableNextColumn();
        const std::string eigText = FormatNumber(row.estimated_information_gain);
        ImGui::TextUnformatted(eigText.c_str());
        ImGui::TableNextColumn();
        const std::string cumulativeText = FormatNumber(row.cumulative_information_gain);
        ImGui::TextUnformatted(cumulativeText.c_str());
        ImGui::TableNextColumn();
        const std::string uncertaintyText = FormatNumber(row.posterior_uncertainty);
        ImGui::TextUnformatted(uncertaintyText.c_str());
        ImGui::TableNextColumn();
        ImGui::Text("%d", row.observation_count);
        ImGui::TableNextColumn();
        if (index < state.lens.rows.size() && state.lens.rows[index].path == row.path) {
            const std::string zoneText = FormatActiveZone(state.lens.rows[index]);
            ImGui::TextUnformatted(zoneText.c_str());
        } else {
            ImGui::TextDisabled("--");
        }
        ImGui::TableNextColumn();
        if (index < state.lens.rows.size() && state.lens.rows[index].path == row.path) {
            ImGui::TextUnformatted(state.lens.rows[index].guidance.c_str());
        } else {
            ImGui::TextDisabled("--");
        }
    }

    ImGui::EndTable();
}

void RenderCompletenessSection(const ExplainoSidecarWindowState& state) {
    ImGui::Separator();
    ImGui::Text("Exploration Completeness");
    if (!state.completeness_error_message.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "Completeness error");
        ImGui::TextWrapped("%s", state.completeness_error_message.c_str());
        return;
    }
    if (state.completeness.rows.empty()) {
        ImGui::TextDisabled("Completeness unavailable.");
        return;
    }

    ImGui::BulletText("demonstrated: %d/%d (%.3f)",
        state.completeness.demonstrated_count,
        static_cast<int>(state.completeness.rows.size()),
        state.completeness.demonstrated_fraction);
    ImGui::BulletText("uncertain: %d", state.completeness.uncertain_count);
    ImGui::BulletText("mean_coverage_score: %.3f", state.completeness.mean_coverage_score);

    if (!ImGui::BeginTable("sidecar_completeness", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
        return;
    }

    ImGui::TableSetupColumn("Label");
    ImGui::TableSetupColumn("Path");
    ImGui::TableSetupColumn("Coverage");
    ImGui::TableSetupColumn("Uncertainty");
    ImGui::TableSetupColumn("Obs");
    ImGui::TableSetupColumn("Status");
    ImGui::TableHeadersRow();

    for (const auto& row : state.completeness.rows) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(row.label.c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(row.path.c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(FormatNumber(row.coverage_score).c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(FormatNumber(row.posterior_uncertainty).c_str());
        ImGui::TableNextColumn();
        ImGui::Text("%d", row.observation_count);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(row.coverage_bucket.c_str());
    }

    ImGui::EndTable();
}

void RenderEnergySection(const ExplainoSidecarWindowState& state) {
    ImGui::Separator();
    ImGui::Text("Energy Landscape");
    if (!state.energy_error_message.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "Energy error");
        ImGui::TextWrapped("%s", state.energy_error_message.c_str());
        return;
    }
    if (state.energy_landscape.rows.empty()) {
        ImGui::TextDisabled("Energy unavailable.");
        return;
    }

    ImGui::BulletText("available_rows: %d", state.energy_landscape.available_row_count);
    ImGui::BulletText("recommendation_eligible_rows: %d", state.energy_landscape.recommendation_eligible_count);
    ImGui::BulletText("peak_energy: %.3f", state.energy_landscape.peak_energy);
    ImGui::BulletText("mean_energy: %.3f", state.energy_landscape.mean_energy);
    if (!state.energy_landscape.peak_path.empty()) {
        ImGui::BulletText("peak_path: %s", state.energy_landscape.peak_path.c_str());
    }

    if (!ImGui::BeginTable("sidecar_energy", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
        return;
    }

    ImGui::TableSetupColumn("Label");
    ImGui::TableSetupColumn("Path");
    ImGui::TableSetupColumn("Energy");
    ImGui::TableSetupColumn("Eff IG");
    ImGui::TableSetupColumn("Cost");
    ImGui::TableSetupColumn("Zone");
    ImGui::TableSetupColumn("Status");
    ImGui::TableSetupColumn("Trace");
    ImGui::TableHeadersRow();

    for (const auto& row : state.energy_landscape.rows) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(row.label.c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(row.path.c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(FormatOptionalNumber(row.energy).c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(FormatOptionalNumber(row.effective_information_gain).c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(FormatOptionalNumber(row.cost_hint).c_str());
        ImGui::TableNextColumn();
        const SidecarLensProjectionRow zoneRow{row.label, row.path, row.type, 0.0, row.active_min, row.active_max, row.active_fraction, 0.0, 0.0, false, row.guidance};
        ImGui::TextUnformatted(FormatActiveZone(zoneRow).c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(row.summary.c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(FormatTraceOverlay(row.path, state.trace).c_str());
    }

    ImGui::EndTable();
}

void RenderTraceSection(const ExplainoSidecarWindowState& state) {
    ImGui::Separator();
    ImGui::Text("Slime Trace");
    if (!state.trace_error_message.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "Trace error");
        ImGui::TextWrapped("%s", state.trace_error_message.c_str());
        return;
    }
    if (state.trace.steps.empty()) {
        ImGui::TextDisabled("Trace unavailable.");
        return;
    }

    ImGui::BulletText("proposal_steps: %d", state.trace.proposal_step_count);
    ImGui::BulletText("apply_steps: %d", state.trace.apply_step_count);
    ImGui::BulletText("latest_path: %s", state.trace.latest_path.c_str());

    if (!ImGui::BeginTable("sidecar_trace", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
        return;
    }

    ImGui::TableSetupColumn("Step");
    ImGui::TableSetupColumn("Path");
    ImGui::TableSetupColumn("Mode");
    ImGui::TableSetupColumn("Target");
    ImGui::TableSetupColumn("Energy");
    ImGui::TableHeadersRow();

    for (const auto& step : state.trace.steps) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("#%d", step.step_index);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(step.path.c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(step.summary.c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(FormatOptionalNumber(step.target_value).c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(FormatOptionalNumber(step.energy).c_str());
    }

    ImGui::EndTable();
}

void RenderDivergenceSection(const ExplainoSidecarWindowState& state) {
    ImGui::Separator();
    ImGui::Text("Divergence Indicator");
    if (!state.divergence_error_message.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "Divergence error");
        ImGui::TextWrapped("%s", state.divergence_error_message.c_str());
        return;
    }

    ImGui::BulletText("status: %s", state.divergence.summary.c_str());
    ImGui::BulletText("reason: %s", state.divergence.reason.c_str());
    ImGui::BulletText("scalar_divergence: %.3f", state.divergence.scalar_divergence);
    ImGui::BulletText("import_changed: %s", state.divergence.import_changed ? "true" : "false");
    ImGui::BulletText("projection_changed: %s", state.divergence.projection_changed ? "true" : "false");
}

bool RenderControllerSection(
    const ExplainoSidecarWindowState& state,
    SidecarAutoDemoControllerPolicy* ioPolicy,
    bool* outApplyArmedDecision) {
    if (outApplyArmedDecision) *outApplyArmedDecision = false;
    bool interacted = false;
    SidecarAutoDemoControllerPolicy displayPolicy = state.controller_policy;
    if (ioPolicy) {
        displayPolicy = *ioPolicy;
    }

    ImGui::Separator();
    ImGui::Text("Auto Demonstration");
    if (ioPolicy) {
        bool enabled = displayPolicy.enabled;
        if (ImGui::Checkbox("Enable Auto Demonstration", &enabled)) {
            ioPolicy->enabled = enabled;
            displayPolicy.enabled = enabled;
            interacted = true;
        }

        bool allowRuntimeMutation = displayPolicy.allow_runtime_mutation;
        if (ImGui::Checkbox("Allow Runtime Mutation", &allowRuntimeMutation)) {
            ioPolicy->allow_runtime_mutation = allowRuntimeMutation;
            displayPolicy.allow_runtime_mutation = allowRuntimeMutation;
            interacted = true;
        }

        float stopDemonstratedFraction = static_cast<float>(displayPolicy.stop_demonstrated_fraction);
        if (ImGui::SliderFloat("Stop Demonstrated Fraction", &stopDemonstratedFraction, 0.0f, 1.0f, "%.2f")) {
            ioPolicy->stop_demonstrated_fraction = stopDemonstratedFraction;
            displayPolicy.stop_demonstrated_fraction = stopDemonstratedFraction;
            interacted = true;
        }

        int stopUncertainCount = displayPolicy.stop_uncertain_count;
        if (ImGui::InputInt("Stop Uncertain Count", &stopUncertainCount)) {
            if (stopUncertainCount < 0) stopUncertainCount = 0;
            ioPolicy->stop_uncertain_count = stopUncertainCount;
            displayPolicy.stop_uncertain_count = stopUncertainCount;
            interacted = true;
        }
    }

    if (!state.controller_error_message.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "Controller error");
        ImGui::TextWrapped("%s", state.controller_error_message.c_str());
        return interacted;
    }

    ImGui::BulletText("status: %s", state.controller_decision.summary.c_str());
    ImGui::BulletText("reason: %s", state.controller_decision.reason.c_str());
    ImGui::BulletText("enabled: %s", displayPolicy.enabled ? "true" : "false");
    ImGui::BulletText("mutation_opt_in: %s", displayPolicy.allow_runtime_mutation ? "true" : "false");
    ImGui::BulletText("stop_demonstrated_fraction: %.2f", displayPolicy.stop_demonstrated_fraction);
    ImGui::BulletText("stop_uncertain_count: %d", displayPolicy.stop_uncertain_count);
    if (state.controller_decision.has_target_value) {
        ImGui::BulletText("path: %s", state.controller_decision.path.c_str());
        ImGui::BulletText("target_value: %.6f", state.controller_decision.target_value);
        ImGui::BulletText("guidance: %s", state.controller_decision.guidance.c_str());
        if (ioPolicy) {
            if (state.controller_decision.should_mutate) {
                if (ImGui::Button("Apply Armed Step")) {
                    if (outApplyArmedDecision) *outApplyArmedDecision = true;
                    interacted = true;
                }
            } else {
                ImGui::TextDisabled("Enable runtime mutation to arm apply.");
            }
        }
    }

    return interacted;
}

void RenderRecommendationSection(const ExplainoSidecarWindowState& state) {
    ImGui::Separator();
    ImGui::Text("Recommended Action");
    if (state.has_action_recommendation) {
        const SidecarActionRecommendation& action = state.action_recommendation;
        ImGui::BulletText("label: %s", action.label.c_str());
        ImGui::BulletText("path: %s", action.path.c_str());
        ImGui::BulletText("utility: %.3f", action.utility);
        ImGui::BulletText("effective_information_gain: %.3f", action.effective_information_gain);
        ImGui::BulletText("cost_hint: %.3f", action.cost_hint);
        ImGui::BulletText("zone: %s", FormatActiveZone({action.label, action.path, action.type, 0.0, action.active_min, action.active_max, action.active_fraction, 0.0, 0.0, false, action.guidance}).c_str());
        ImGui::BulletText("guidance: %s", action.guidance.c_str());
        return;
    }

    ImGui::TextDisabled("Recommendation unavailable.");
    if (!state.action_error_message.empty()) {
        ImGui::TextWrapped("%s", state.action_error_message.c_str());
    }
}

void RenderApplicableParamsSection(const ExplainoSidecarWindowState& state) {
    ImGui::Separator();
    ImGui::Text("Applicable Parameters");

    if (state.rows.empty()) {
        ImGui::TextDisabled("No applicable parameters.");
        return;
    }

    if (!ImGui::BeginTable("sidecar_params", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
        return;
    }

    ImGui::TableSetupColumn("Label");
    ImGui::TableSetupColumn("Path");
    ImGui::TableSetupColumn("Type");
    ImGui::TableSetupColumn("Range");
    ImGui::TableSetupColumn("Default");
    ImGui::TableHeadersRow();

    for (const auto& row : state.rows) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(row.label.c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(row.path.c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(row.type.c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(row.range_text.c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(row.default_text.c_str());
    }

    ImGui::EndTable();
}

void PopulateCompletenessFromBudget(
    const SidecarHypothesisSpace& space,
    const SidecarExplorationCompleteness* previousCompleteness,
    ExplainoSidecarWindowState* ioState) {
    if (!ioState) {
        return;
    }
    if (ioState->budget.function_id.empty()) {
        return;
    }

    std::string completenessError;
    if (BuildSidecarExplorationCompleteness(space, ioState->budget, &ioState->completeness, &completenessError)) {
        ioState->completeness_error_message.clear();
    } else if (previousCompleteness &&
        previousCompleteness->function_id == ioState->budget.function_id &&
        previousCompleteness->fractal_type_id == ioState->budget.fractal_type_id) {
        ioState->completeness = *previousCompleteness;
        ioState->completeness_error_message.clear();
    } else {
        ioState->completeness = {};
        ioState->completeness_error_message = completenessError;
    }
}

void PopulateControllerDecision(ExplainoSidecarWindowState* ioState) {
    if (!ioState) {
        return;
    }

    ioState->controller_error_message.clear();
    ioState->controller_decision = {};

    std::string controllerError;
    if (!BuildSidecarAutoDemoControllerDecision(
            ioState->has_action_recommendation ? &ioState->action_recommendation : nullptr,
            ioState->completeness,
            ioState->controller_policy,
            &ioState->controller_decision,
            &controllerError)) {
        ioState->controller_decision = {};
        ioState->controller_error_message = controllerError;
    }
}

void PopulateEnergyLandscape(
    const SidecarHypothesisSpace& space,
    ExplainoSidecarWindowState* ioState) {
    if (!ioState) {
        return;
    }

    ioState->energy_error_message.clear();
    ioState->energy_landscape = {};
    if (ioState->budget.function_id.empty() || ioState->lens.function_id.empty()) {
        return;
    }

    std::string energyError;
    if (!BuildSidecarEnergyLandscape(
            space,
            ioState->budget,
            ioState->lens,
            &ioState->energy_landscape,
            &energyError)) {
        ioState->energy_landscape = {};
        ioState->energy_error_message = energyError;
    }
}

bool PreviousTraceCompatibleWithWindowState(
    const SidecarSlimeTrace& trace,
    const ExplainoSidecarWindowState& state) {
    if (trace.function_id.empty() || trace.fractal_type_id.empty()) {
        return false;
    }
    if (trace.function_id != state.function_id || trace.fractal_type_id != state.fractal_type_id) {
        return false;
    }

    std::unordered_map<std::string, bool> rowPathSet;
    rowPathSet.reserve(state.rows.size());
    for (const auto& row : state.rows) {
        rowPathSet.emplace(row.path, true);
    }
    for (const auto& step : trace.steps) {
        if (rowPathSet.find(step.path) == rowPathSet.end()) {
            return false;
        }
    }
    return true;
}

void PopulateSlimeTrace(
    const SidecarSlimeTrace* previousTrace,
    ExplainoSidecarWindowState* ioState) {
    if (!ioState) {
        return;
    }

    ioState->trace_error_message.clear();
    ioState->trace = {};
    if (!ioState->controller_error_message.empty() || !ioState->energy_error_message.empty()) {
        if (previousTrace && PreviousTraceCompatibleWithWindowState(*previousTrace, *ioState)) {
            ioState->trace = *previousTrace;
        }
        return;
    }
    if (ioState->energy_landscape.function_id.empty()) {
        if (previousTrace && PreviousTraceCompatibleWithWindowState(*previousTrace, *ioState)) {
            ioState->trace = *previousTrace;
        }
        return;
    }

    std::string traceError;
    if (!BuildSidecarSlimeTrace(
            ioState->energy_landscape,
            ioState->controller_decision,
            previousTrace,
            &ioState->trace,
            &traceError)) {
        ioState->trace = {};
        ioState->trace_error_message = traceError;
    }
}

void PopulateDivergenceState(
    const SidecarOrientationVector* previousOrientation,
    ExplainoSidecarWindowState* ioState) {
    if (!ioState) {
        return;
    }

    ioState->divergence_error_message.clear();
    ioState->divergence = {};

    std::string divergenceError;
    if (!BuildSidecarStateDivergence(
            previousOrientation,
            ioState->orientation,
            &ioState->divergence,
            &divergenceError)) {
        ioState->divergence = {};
        ioState->divergence_error_message = divergenceError;
    }
}

} // namespace

bool BuildExplainoSidecarWindowState(
    const EngineFunctionCatalog& catalog,
    const BindingContext& ctx,
    const SidecarMeasurementHost* measurementHost,
    const SidecarBudgetState* previousBudget,
    const SidecarExplorationCompleteness* previousCompleteness,
    const SidecarAutoDemoControllerPolicy* controllerPolicy,
    ExplainoSidecarWindowState* outState,
    std::string* outError) {
    return BuildExplainoSidecarWindowState(
        catalog,
        ctx,
        measurementHost,
        previousBudget,
        previousCompleteness,
        nullptr,
        nullptr,
        controllerPolicy,
        outState,
        outError);
}

bool BuildExplainoSidecarWindowState(
    const EngineFunctionCatalog& catalog,
    const BindingContext& ctx,
    const SidecarMeasurementHost* measurementHost,
    const SidecarBudgetState* previousBudget,
    const SidecarExplorationCompleteness* previousCompleteness,
    const SidecarOrientationVector* previousOrientation,
    const SidecarAutoDemoControllerPolicy* controllerPolicy,
    ExplainoSidecarWindowState* outState,
    std::string* outError) {
    return BuildExplainoSidecarWindowState(
        catalog,
        ctx,
        measurementHost,
        previousBudget,
        previousCompleteness,
        previousOrientation,
        nullptr,
        controllerPolicy,
        outState,
        outError);
}

bool BuildExplainoSidecarWindowState(
    const EngineFunctionCatalog& catalog,
    const BindingContext& ctx,
    const SidecarMeasurementHost* measurementHost,
    const SidecarBudgetState* previousBudget,
    const SidecarExplorationCompleteness* previousCompleteness,
    const SidecarOrientationVector* previousOrientation,
    const SidecarSlimeTrace* previousTrace,
    const SidecarAutoDemoControllerPolicy* controllerPolicy,
    ExplainoSidecarWindowState* outState,
    std::string* outError) {
    if (!outState) {
        if (outError) *outError = "BuildExplainoSidecarWindowState requires outState";
        return false;
    }

    SidecarHypothesisSpace space;
    std::string modelError;
    if (!BuildSidecarHypothesisSpace(catalog, "fractal.sample", ctx, &space, &modelError)) {
        *outState = {};
        outState->title = "Explaino Sidecar";
        outState->error_message = modelError;
        if (outError) *outError = modelError;
        return false;
    }

    ExplainoSidecarWindowState next;
    next.title = "Explaino Sidecar";
    next.function_id = space.function_id;
    next.fractal_type_id = ctx.GetEnumId("fractal.view.fractal_type");
    next.orientation = ComputeSidecarOrientationVector(ctx, space);
    next.has_orientation = SidecarOrientationHasFiniteValues(next.orientation);
    if (controllerPolicy) {
        next.controller_policy = *controllerPolicy;
    }

    for (const auto& param : space.applicable_parameters) {
        ExplainoSidecarWindowRow row;
        row.label = param.label;
        row.path = param.path;
        row.type = param.type;
        row.range_text = FormatRange(param);
        if (param.has_default) {
            row.default_text = FormatDefaultValue(param.default_value);
        }
        next.rows.push_back(std::move(row));
    }

    if (measurementHost) {
        std::string measurementError;
        if (!BuildSidecarMeasurementBatch(space, ctx, *measurementHost, &next.measurement, &measurementError)) {
            next.measurement_error_message = measurementError;
            if (previousBudget) {
                next.budget = *previousBudget;
            }
            PopulateCompletenessFromBudget(space, previousCompleteness, &next);
            PopulateControllerDecision(&next);
            PopulateSlimeTrace(previousTrace, &next);
            PopulateDivergenceState(previousOrientation, &next);
            *outState = std::move(next);
            if (outError) *outError = measurementError;
            return false;
        }

        std::string budgetError;
        if (!UpdateSidecarBudgetState(space, next.fractal_type_id, next.measurement, previousBudget, &next.budget, &budgetError)) {
            next.measurement_error_message = budgetError;
            if (previousBudget) {
                next.budget = *previousBudget;
            }
            PopulateCompletenessFromBudget(space, previousCompleteness, &next);
            PopulateControllerDecision(&next);
            PopulateSlimeTrace(previousTrace, &next);
            PopulateDivergenceState(previousOrientation, &next);
            *outState = std::move(next);
            if (outError) *outError = budgetError;
            return false;
        }

        next.orientation = ComputeSidecarOrientationVector(ctx, space, BuildSidecarOrientationInputs(next.measurement));
        if (!next.budget.rows.empty()) {
            next.orientation.field_embedding_stats = next.budget.estimated_information_gain_total;
        }
        next.has_orientation = SidecarOrientationHasFiniteValues(next.orientation);

        std::string lensError;
        if (!BuildSidecarLensProjection(space, next.measurement, next.budget, &next.lens, &lensError)) {
            next.measurement_error_message = lensError;
            if (previousBudget) {
                next.budget = *previousBudget;
            }
            PopulateCompletenessFromBudget(space, previousCompleteness, &next);
            PopulateControllerDecision(&next);
            PopulateSlimeTrace(previousTrace, &next);
            PopulateDivergenceState(previousOrientation, &next);
            *outState = std::move(next);
            if (outError) *outError = lensError;
            return false;
        }

        PopulateEnergyLandscape(space, &next);
        if (!next.energy_error_message.empty()) {
            *outState = std::move(next);
            if (outError) *outError = outState->energy_error_message;
            return false;
        }

        std::string completenessError;
        if (!BuildSidecarExplorationCompleteness(space, next.budget, &next.completeness, &completenessError)) {
            next.completeness_error_message = completenessError;
            PopulateDivergenceState(previousOrientation, &next);
            *outState = std::move(next);
            if (outError) *outError = completenessError;
            return false;
        }

        std::string actionError;
        if (BuildSidecarActionRecommendation(next.energy_landscape, &next.action_recommendation, &actionError)) {
            next.has_action_recommendation = true;
        } else {
            next.action_error_message = actionError;
        }

        PopulateControllerDecision(&next);
        PopulateSlimeTrace(previousTrace, &next);
        PopulateDivergenceState(previousOrientation, &next);
        if (!next.controller_error_message.empty()) {
            *outState = std::move(next);
            if (outError) *outError = outState->controller_error_message;
            return false;
        }
        if (!next.trace_error_message.empty()) {
            *outState = std::move(next);
            if (outError) *outError = outState->trace_error_message;
            return false;
        }
        if (!next.divergence_error_message.empty()) {
            *outState = std::move(next);
            if (outError) *outError = outState->divergence_error_message;
            return false;
        }
    }

    if (!measurementHost) {
        PopulateSlimeTrace(previousTrace, &next);
        PopulateDivergenceState(previousOrientation, &next);
        if (!next.trace_error_message.empty()) {
            *outState = std::move(next);
            if (outError) *outError = outState->trace_error_message;
            return false;
        }
        if (!next.divergence_error_message.empty()) {
            *outState = std::move(next);
            if (outError) *outError = outState->divergence_error_message;
            return false;
        }
    }

    *outState = std::move(next);
    return true;
}

bool BuildExplainoSidecarWindowState(
    const EngineFunctionCatalog& catalog,
    const BindingContext& ctx,
    const SidecarMeasurementHost* measurementHost,
    const SidecarBudgetState* previousBudget,
    const SidecarExplorationCompleteness* previousCompleteness,
    ExplainoSidecarWindowState* outState,
    std::string* outError) {
    return BuildExplainoSidecarWindowState(
        catalog,
        ctx,
        measurementHost,
        previousBudget,
        previousCompleteness,
        nullptr,
        nullptr,
        nullptr,
        outState,
        outError);
}

bool BuildExplainoSidecarWindowState(
    const EngineFunctionCatalog& catalog,
    const BindingContext& ctx,
    const SidecarMeasurementHost* measurementHost,
    ExplainoSidecarWindowState* outState,
    std::string* outError) {
    return BuildExplainoSidecarWindowState(catalog, ctx, measurementHost, nullptr, nullptr, nullptr, outState, outError);
}

bool BuildExplainoSidecarWindowState(
    const EngineFunctionCatalog& catalog,
    const BindingContext& ctx,
    ExplainoSidecarWindowState* outState,
    std::string* outError) {
    return BuildExplainoSidecarWindowState(catalog, ctx, nullptr, nullptr, nullptr, nullptr, outState, outError);
}

bool RenderExplainoSidecarWindow(
    const ExplainoSidecarWindowState& state,
    SidecarAutoDemoControllerPolicy* ioPolicy,
    bool* outApplyArmedDecision) {
    if (outApplyArmedDecision) *outApplyArmedDecision = false;
    bool interacted = false;
    ImGui::Begin(state.title.c_str());

    if (!state.error_message.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "Sidecar model error");
        ImGui::TextWrapped("%s", state.error_message.c_str());
        ImGui::End();
        return interacted;
    }

    ImGui::Text("Function: %s", state.function_id.c_str());
    ImGui::Text("Fractal Type: %s", state.fractal_type_id.c_str());
    if (!state.measurement_error_message.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.2f, 1.0f), "Measurement error");
        ImGui::TextWrapped("%s", state.measurement_error_message.c_str());
    }
    RenderOrientationSection(state);
    RenderBudgetSection(state);
    RenderEnergySection(state);
    RenderTraceSection(state);
    RenderCompletenessSection(state);
    RenderDivergenceSection(state);
    if (RenderControllerSection(state, ioPolicy, outApplyArmedDecision)) {
        interacted = true;
    }
    RenderRecommendationSection(state);
    RenderApplicableParamsSection(state);

    ImGui::End();
    return interacted;
}

void RenderExplainoSidecarWindow(const ExplainoSidecarWindowState& state) {
    bool ignoredApply = false;
    RenderExplainoSidecarWindow(state, nullptr, &ignoredApply);
}