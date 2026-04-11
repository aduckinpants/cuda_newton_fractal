#include "explaino_sidecar_window.h"

#include "imgui.h"

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

} // namespace

bool BuildExplainoSidecarWindowState(
    const EngineFunctionCatalog& catalog,
    const BindingContext& ctx,
    const SidecarMeasurementHost* measurementHost,
    const SidecarBudgetState* previousBudget,
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
            *outState = std::move(next);
            if (outError) *outError = budgetError;
            return false;
        }

        next.orientation = ComputeSidecarOrientationVector(ctx, space, BuildSidecarOrientationInputs(next.measurement));
        if (!next.budget.rows.empty()) {
            next.orientation.field_embedding_stats = next.budget.estimated_information_gain_total;
        }

        std::string lensError;
        if (!BuildSidecarLensProjection(space, next.measurement, next.budget, &next.lens, &lensError)) {
            next.measurement_error_message = lensError;
            if (previousBudget) {
                next.budget = *previousBudget;
            }
            *outState = std::move(next);
            if (outError) *outError = lensError;
            return false;
        }

        std::string completenessError;
        if (!BuildSidecarExplorationCompleteness(space, next.budget, &next.completeness, &completenessError)) {
            next.completeness_error_message = completenessError;
            *outState = std::move(next);
            if (outError) *outError = completenessError;
            return false;
        }

        std::string actionError;
        if (BuildSidecarActionRecommendation(space, next.budget, next.lens, &next.action_recommendation, &actionError)) {
            next.has_action_recommendation = true;
        } else {
            next.action_error_message = actionError;
        }
    }

    *outState = std::move(next);
    return true;
}

bool BuildExplainoSidecarWindowState(
    const EngineFunctionCatalog& catalog,
    const BindingContext& ctx,
    const SidecarMeasurementHost* measurementHost,
    ExplainoSidecarWindowState* outState,
    std::string* outError) {
    return BuildExplainoSidecarWindowState(catalog, ctx, measurementHost, nullptr, outState, outError);
}

bool BuildExplainoSidecarWindowState(
    const EngineFunctionCatalog& catalog,
    const BindingContext& ctx,
    ExplainoSidecarWindowState* outState,
    std::string* outError) {
    return BuildExplainoSidecarWindowState(catalog, ctx, nullptr, nullptr, outState, outError);
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
    ImGui::Separator();
    ImGui::Text("Orientation");
    ImGui::BulletText("import_signature: %llu", static_cast<unsigned long long>(state.orientation.import_signature));
    ImGui::BulletText("pack_projection_hash: %llu", static_cast<unsigned long long>(state.orientation.pack_projection_hash));
    ImGui::BulletText("field_embedding_stats: %.3f", state.orientation.field_embedding_stats);
    ImGui::BulletText("slime_energy_delta: %.3f", state.orientation.slime_energy_delta);
    ImGui::BulletText("busy_beaver_metrics: %.3f", state.orientation.busy_beaver_metrics);
    ImGui::BulletText("decode_stability: %.3f", state.orientation.decode_stability);
    ImGui::BulletText("diff_magnitude: %.3f", state.orientation.diff_magnitude);
    ImGui::Separator();
    ImGui::Text("Information Budget");
    if (state.budget.rows.empty()) {
        ImGui::TextDisabled("Measurement unavailable.");
    } else {
        ImGui::BulletText("estimated_information_gain_total: %.3f", state.budget.estimated_information_gain_total);
        ImGui::BulletText("cumulative_information_gain_total: %.3f", state.budget.cumulative_information_gain_total);
        ImGui::BulletText("mean_posterior_uncertainty: %.3f", state.budget.mean_posterior_uncertainty);
        ImGui::BulletText("mean_decode_stability: %.3f", state.budget.mean_decode_stability);
        ImGui::BulletText("budget_batches: %d", state.budget.batch_count);

        if (ImGui::BeginTable("sidecar_budget", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
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
    }

    ImGui::Separator();
    ImGui::Text("Exploration Completeness");
    if (!state.completeness_error_message.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "Completeness error");
        ImGui::TextWrapped("%s", state.completeness_error_message.c_str());
    } else if (state.completeness.rows.empty()) {
        ImGui::TextDisabled("Completeness unavailable.");
    } else {
        ImGui::BulletText("demonstrated: %d/%d (%.3f)",
            state.completeness.demonstrated_count,
            static_cast<int>(state.completeness.rows.size()),
            state.completeness.demonstrated_fraction);
        ImGui::BulletText("uncertain: %d", state.completeness.uncertain_count);
        ImGui::BulletText("mean_coverage_score: %.3f", state.completeness.mean_coverage_score);

        if (ImGui::BeginTable("sidecar_completeness", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
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
    }

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
    } else if (!state.action_error_message.empty()) {
        ImGui::TextDisabled("Recommendation unavailable.");
        ImGui::TextWrapped("%s", state.action_error_message.c_str());
    } else {
        ImGui::TextDisabled("Recommendation unavailable.");
    }

    ImGui::Separator();
    ImGui::Text("Applicable Parameters");

    if (state.rows.empty()) {
        ImGui::TextDisabled("No applicable parameters.");
        ImGui::End();
        return;
    }

    if (ImGui::BeginTable("sidecar_params", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
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

    ImGui::End();
}