#include "explaino_sidecar_window.h"

#include "imgui.h"

#include <cmath>
#include <iomanip>
#include <sstream>

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

    if (!ImGui::BeginTable("sidecar_energy", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
        return;
    }

    ImGui::TableSetupColumn("Label");
    ImGui::TableSetupColumn("Path");
    ImGui::TableSetupColumn("Energy");
    ImGui::TableSetupColumn("Eff IG");
    ImGui::TableSetupColumn("Cost");
    ImGui::TableSetupColumn("Zone");
    ImGui::TableSetupColumn("Status");
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

void RenderControllerSection(const ExplainoSidecarWindowState& state) {
    ImGui::Separator();
    ImGui::Text("Auto Demonstration");
    if (!state.controller_error_message.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "Controller error");
        ImGui::TextWrapped("%s", state.controller_error_message.c_str());
        return;
    }

    ImGui::BulletText("status: %s", state.controller_decision.summary.c_str());
    ImGui::BulletText("reason: %s", state.controller_decision.reason.c_str());
    ImGui::BulletText("enabled: %s", state.controller_policy.enabled ? "true" : "false");
    ImGui::BulletText("mutation_opt_in: %s", state.controller_policy.allow_runtime_mutation ? "true" : "false");
    if (state.controller_decision.has_target_value) {
        ImGui::BulletText("path: %s", state.controller_decision.path.c_str());
        ImGui::BulletText("target_value: %.6f", state.controller_decision.target_value);
        ImGui::BulletText("guidance: %s", state.controller_decision.guidance.c_str());
    }
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
        PopulateDivergenceState(previousOrientation, &next);
        if (!next.controller_error_message.empty()) {
            *outState = std::move(next);
            if (outError) *outError = outState->controller_error_message;
            return false;
        }
        if (!next.divergence_error_message.empty()) {
            *outState = std::move(next);
            if (outError) *outError = outState->divergence_error_message;
            return false;
        }
    }

    if (!measurementHost) {
        PopulateDivergenceState(previousOrientation, &next);
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

void RenderExplainoSidecarWindow(const ExplainoSidecarWindowState& state) {
    ImGui::Begin(state.title.c_str());

    if (!state.error_message.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "Sidecar model error");
        ImGui::TextWrapped("%s", state.error_message.c_str());
        ImGui::End();
        return;
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
    RenderCompletenessSection(state);
    RenderDivergenceSection(state);
    RenderControllerSection(state);
    RenderRecommendationSection(state);
    RenderApplicableParamsSection(state);

    ImGui::End();
}