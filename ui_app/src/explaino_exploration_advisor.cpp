#include "explaino_exploration_advisor.h"

#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

#include "explaino_sidecar_measurement.h"
#include "explaino_sidecar_window.h"
#include "fractal_family_rules.h"
#include "schema_binding.h"

namespace {

bool IsExplainoFractalTypeId(std::string_view fractalTypeId) {
    return fractalTypeId == "explaino" || fractalTypeId.substr(0, 9) == "explaino_";
}

void WriteEscapedJsonString(std::ostringstream& out, const std::string& text) {
    out << '"';
    for (char ch : text) {
        switch (ch) {
        case '"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
        case '\b': out << "\\b"; break;
        case '\f': out << "\\f"; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default:
            if (static_cast<unsigned char>(ch) < 0x20) {
                const char* hex = "0123456789abcdef";
                out << "\\u00"
                    << hex[(static_cast<unsigned char>(ch) >> 4) & 0x0f]
                    << hex[static_cast<unsigned char>(ch) & 0x0f];
            } else {
                out << ch;
            }
            break;
        }
    }
    out << '"';
}

ExplainoExplorationAdvisorObservation BuildObservation(
    const SidecarEnergyLandscapeRow& row,
    int rank) {
    ExplainoExplorationAdvisorObservation observation;
    observation.rank = rank;
    observation.label = row.label;
    observation.path = row.path;
    observation.type = row.type;
    observation.guidance = row.guidance;
    observation.summary = row.summary;
    observation.reason = row.reason;
    observation.current_value = row.current_value;
    observation.estimated_information_gain = row.estimated_information_gain;
    observation.effective_information_gain = row.effective_information_gain;
    observation.cumulative_information_gain = row.cumulative_information_gain;
    observation.posterior_uncertainty = row.posterior_uncertainty;
    observation.cost_hint = row.cost_hint;
    observation.utility = row.energy;
    observation.active_min = row.active_min;
    observation.active_max = row.active_max;
    observation.active_fraction = row.active_fraction;
    observation.observation_count = row.observation_count;
    return observation;
}

void WriteObservationJson(
    std::ostringstream& out,
    const ExplainoExplorationAdvisorObservation& observation,
    int indentSpaces) {
    const std::string indent(static_cast<size_t>(indentSpaces), ' ');
    const std::string childIndent(static_cast<size_t>(indentSpaces + 2), ' ');
    out << indent << "{\n";
    out << childIndent << "\"rank\": " << observation.rank << ",\n";
    out << childIndent << "\"label\": "; WriteEscapedJsonString(out, observation.label); out << ",\n";
    out << childIndent << "\"path\": "; WriteEscapedJsonString(out, observation.path); out << ",\n";
    out << childIndent << "\"type\": "; WriteEscapedJsonString(out, observation.type); out << ",\n";
    out << childIndent << "\"guidance\": "; WriteEscapedJsonString(out, observation.guidance); out << ",\n";
    out << childIndent << "\"summary\": "; WriteEscapedJsonString(out, observation.summary); out << ",\n";
    out << childIndent << "\"reason\": "; WriteEscapedJsonString(out, observation.reason); out << ",\n";
    out << childIndent << "\"current_value\": " << observation.current_value << ",\n";
    out << childIndent << "\"estimated_information_gain\": " << observation.estimated_information_gain << ",\n";
    out << childIndent << "\"effective_information_gain\": " << observation.effective_information_gain << ",\n";
    out << childIndent << "\"cumulative_information_gain\": " << observation.cumulative_information_gain << ",\n";
    out << childIndent << "\"posterior_uncertainty\": " << observation.posterior_uncertainty << ",\n";
    out << childIndent << "\"cost_hint\": " << observation.cost_hint << ",\n";
    out << childIndent << "\"utility\": " << observation.utility << ",\n";
    out << childIndent << "\"active_min\": " << observation.active_min << ",\n";
    out << childIndent << "\"active_max\": " << observation.active_max << ",\n";
    out << childIndent << "\"active_fraction\": " << observation.active_fraction << ",\n";
    out << childIndent << "\"observation_count\": " << observation.observation_count << "\n";
    out << indent << '}';
}

} // namespace

bool BuildExplainoExplorationAdvisorReport(
    const ExplainoSidecarWindowState& state,
    ExplainoExplorationAdvisorReport* outReport,
    std::string* outError) {
    if (!outReport) {
        if (outError) *outError = "BuildExplainoExplorationAdvisorReport requires outReport";
        return false;
    }
    if (!IsExplainoFractalTypeId(state.fractal_type_id)) {
        *outReport = {};
        if (outError) *outError = "Explaino exploration advisor only supports Explaino-family states";
        return false;
    }

    ExplainoExplorationAdvisorReport next;
    next.function_id = state.function_id;
    next.fractal_type = state.fractal_type_id;
    next.fractal_type_id = state.fractal_type_id;
    next.coordinate_count = state.measurement.coordinate_count;
    next.available_row_count = state.energy_landscape.available_row_count;
    next.recommendation_eligible_count = state.energy_landscape.recommendation_eligible_count;
    next.demonstrated_count = state.completeness.demonstrated_count;
    next.uncertain_count = state.completeness.uncertain_count;
    next.demonstrated_fraction = state.completeness.demonstrated_fraction;
    next.estimated_information_gain_total = state.measurement.total_information_gain_estimate;
    next.cumulative_information_gain_total = state.budget.cumulative_information_gain_total;
    next.mean_posterior_uncertainty = state.budget.mean_posterior_uncertainty;

    int rank = 1;
    for (const SidecarEnergyLandscapeRow& row : state.energy_landscape.rows) {
        if (!row.recommendation_eligible) {
            continue;
        }
        next.recommendations.push_back(BuildObservation(row, rank));
        ++rank;
    }

    if (state.has_action_recommendation) {
        for (const ExplainoExplorationAdvisorObservation& observation : next.recommendations) {
            if (observation.path == state.action_recommendation.path) {
                next.recommended_observation = observation;
                next.has_recommended_observation = true;
                break;
            }
        }
    }
    if (!next.has_recommended_observation && !next.recommendations.empty()) {
        next.recommended_observation = next.recommendations.front();
        next.has_recommended_observation = true;
    }

    *outReport = std::move(next);
    return true;
}

bool BuildExplainoExplorationAdvisorReport(
    const EngineFunctionCatalog& catalog,
    const BindingContext& ctx,
    const SidecarMeasurementHost& measurementHost,
    ExplainoExplorationAdvisorReport* outReport,
    std::string* outError) {
    if (!ctx.view) {
        if (outError) *outError = "Explaino exploration advisor requires ctx.view";
        return false;
    }
    if (!IsExplainoFamily(ctx.view->fractal_type)) {
        if (outReport) *outReport = {};
        if (outError) *outError = "Explaino exploration advisor only supports Explaino-family runtime states";
        return false;
    }

    ExplainoSidecarWindowState sidecarState;
    std::string sidecarError;
    if (!BuildExplainoSidecarWindowState(catalog, ctx, &measurementHost, &sidecarState, &sidecarError)) {
        if (outReport) *outReport = {};
        if (outError) *outError = sidecarError;
        return false;
    }

    return BuildExplainoExplorationAdvisorReport(sidecarState, outReport, outError);
}

std::string SerializeExplainoExplorationAdvisorReportJson(
    const ExplainoExplorationAdvisorReport& report) {
    std::ostringstream out;
    out << std::setprecision(17);
    out << "{\n";
    out << "  \"report_version\": " << report.report_version << ",\n";
    out << "  \"function_id\": "; WriteEscapedJsonString(out, report.function_id); out << ",\n";
    out << "  \"fractal_type\": "; WriteEscapedJsonString(out, report.fractal_type); out << ",\n";
    out << "  \"fractal_type_id\": "; WriteEscapedJsonString(out, report.fractal_type_id); out << ",\n";
    out << "  \"coordinate_count\": " << report.coordinate_count << ",\n";
    out << "  \"available_row_count\": " << report.available_row_count << ",\n";
    out << "  \"recommendation_eligible_count\": " << report.recommendation_eligible_count << ",\n";
    out << "  \"demonstrated_count\": " << report.demonstrated_count << ",\n";
    out << "  \"uncertain_count\": " << report.uncertain_count << ",\n";
    out << "  \"demonstrated_fraction\": " << report.demonstrated_fraction << ",\n";
    out << "  \"estimated_information_gain_total\": " << report.estimated_information_gain_total << ",\n";
    out << "  \"cumulative_information_gain_total\": " << report.cumulative_information_gain_total << ",\n";
    out << "  \"mean_posterior_uncertainty\": " << report.mean_posterior_uncertainty << ",\n";
    out << "  \"recommended_observation\": ";
    if (report.has_recommended_observation) {
        out << "\n";
        WriteObservationJson(out, report.recommended_observation, 2);
        out << ",\n";
    } else {
        out << "null,\n";
    }
    out << "  \"recommendations\": [";
    if (!report.recommendations.empty()) {
        out << "\n";
        for (size_t index = 0; index < report.recommendations.size(); ++index) {
            WriteObservationJson(out, report.recommendations[index], 4);
            if (index + 1 < report.recommendations.size()) {
                out << ",";
            }
            out << "\n";
        }
        out << "  ";
    }
    out << "]\n";
    out << "}\n";
    return out.str();
}