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

} // namespace

bool BuildExplainoSidecarWindowState(
    const EngineFunctionCatalog& catalog,
    const BindingContext& ctx,
    const SidecarMeasurementHost* measurementHost,
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
            *outState = std::move(next);
            if (outError) *outError = measurementError;
            return false;
        }

        next.orientation = ComputeSidecarOrientationVector(ctx, space, BuildSidecarOrientationInputs(next.measurement));
        if (!next.measurement.rows.empty()) {
            next.orientation.field_embedding_stats = next.measurement.mean_information_gain_estimate;
        }
    }

    *outState = std::move(next);
    return true;
}

bool BuildExplainoSidecarWindowState(
    const EngineFunctionCatalog& catalog,
    const BindingContext& ctx,
    ExplainoSidecarWindowState* outState,
    std::string* outError) {
    return BuildExplainoSidecarWindowState(catalog, ctx, nullptr, outState, outError);
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
    if (state.measurement.rows.empty()) {
        ImGui::TextDisabled("Measurement unavailable.");
    } else {
        ImGui::BulletText("total_information_gain: %.3f", state.measurement.total_information_gain_estimate);
        ImGui::BulletText("mean_information_gain: %.3f", state.measurement.mean_information_gain_estimate);
        ImGui::BulletText("explored_fraction: %.3f", state.measurement.explored_fraction);
        ImGui::BulletText("mean_decode_stability: %.3f", state.measurement.mean_decode_stability);
        ImGui::BulletText("total_diff_magnitude: %.3f", state.measurement.total_diff_magnitude);

        if (ImGui::BeginTable("sidecar_measurements", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn("Label");
            ImGui::TableSetupColumn("Path");
            ImGui::TableSetupColumn("Current");
            ImGui::TableSetupColumn("Step");
            ImGui::TableSetupColumn("IG");
            ImGui::TableSetupColumn("Stability");
            ImGui::TableHeadersRow();

            for (const auto& row : state.measurement.rows) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(row.label.c_str());
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(row.path.c_str());
                ImGui::TableNextColumn();
                const std::string currentText = FormatNumber(row.current_value);
                ImGui::TextUnformatted(currentText.c_str());
                ImGui::TableNextColumn();
                const std::string stepText = FormatNumber(row.step_value);
                ImGui::TextUnformatted(stepText.c_str());
                ImGui::TableNextColumn();
                const std::string informationGainText = FormatNumber(row.information_gain_estimate);
                ImGui::TextUnformatted(informationGainText.c_str());
                ImGui::TableNextColumn();
                const std::string stabilityText = FormatNumber(row.decode_stability);
                ImGui::TextUnformatted(stabilityText.c_str());
            }

            ImGui::EndTable();
        }
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