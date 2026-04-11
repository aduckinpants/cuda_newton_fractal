#include "fractal_probe_contract.h"

#include <cmath>
#include <cstdio>
#include <iomanip>
#include <set>
#include <sstream>

namespace {

bool RejectUnknownKeys(const json_min::Object& object,
    const std::set<std::string>& allowed,
    const std::string& where,
    std::string* outError) {
    for (const auto& entry : object) {
        if (allowed.find(entry.first) == allowed.end()) {
            if (outError) *outError = "Unknown " + where + " key: " + entry.first;
            return false;
        }
    }
    return true;
}

bool GetRequiredString(const json_min::Object& object,
    const char* key,
    std::string* outValue,
    std::string* outError) {
    auto it = object.find(key);
    if (it == object.end() || !it->second.is_string()) {
        if (outError) *outError = std::string("Missing string field: ") + key;
        return false;
    }
    if (outValue) *outValue = it->second.as_string();
    return true;
}

bool GetRequiredNumber(const json_min::Object& object,
    const char* key,
    double* outValue,
    std::string* outError) {
    auto it = object.find(key);
    if (it == object.end() || !it->second.is_number()) {
        if (outError) *outError = std::string("Missing number field: ") + key;
        return false;
    }
    if (outValue) *outValue = it->second.as_number();
    return true;
}

bool GetRequiredInt(const json_min::Object& object,
    const char* key,
    int* outValue,
    std::string* outError) {
    double raw = 0.0;
    if (!GetRequiredNumber(object, key, &raw, outError)) return false;
    if (!std::isfinite(raw) || std::floor(raw) != raw) {
        if (outError) *outError = std::string("Expected integer field: ") + key;
        return false;
    }
    if (outValue) *outValue = static_cast<int>(raw);
    return true;
}

bool GetOptionalBool(const json_min::Object& object,
    const char* key,
    bool* outValue,
    std::string* outError) {
    auto it = object.find(key);
    if (it == object.end()) return true;
    if (!it->second.is_bool()) {
        if (outError) *outError = std::string("Expected bool field: ") + key;
        return false;
    }
    if (outValue) *outValue = it->second.as_bool();
    return true;
}

bool ParseScalarValue(const json_min::Value& value,
    FractalProbeScalar* outScalar,
    std::string* outError) {
    if (value.is_bool()) {
        if (outScalar) *outScalar = FractalProbeScalar::Boolean(value.as_bool());
        return true;
    }
    if (value.is_number()) {
        if (outScalar) *outScalar = FractalProbeScalar::Number(value.as_number());
        return true;
    }
    if (value.is_string()) {
        if (outScalar) *outScalar = FractalProbeScalar::String(value.as_string());
        return true;
    }
    if (outError) *outError = "Probe scalar values must be bool, number, or string";
    return false;
}

bool IsSupportedMetric(const std::string& metric) {
    static const std::set<std::string> supported = {
        "iterations",
        "status",
        "final_z",
        "final_abs2",
        "residual",
        "root_index",
        "summary_mean_iterations",
        "summary_escape_fraction",
        "summary_converged_fraction",
        "summary_nonfinite_fraction",
        "summary_pole_fraction",
        "summary_best_sequence_index",
        "value",
        "abs2",
        "derivative",
        "summary_mean_abs2",
        "summary_diverged_fraction",
    };
    return supported.find(metric) != supported.end();
}

bool ParseModeId(const std::string& text, FractalProbeMode* outMode) {
    if (text == "point_set") { if (outMode) *outMode = FractalProbeMode::point_set; return true; }
    if (text == "grid") { if (outMode) *outMode = FractalProbeMode::grid; return true; }
    if (text == "sequence_point_set") { if (outMode) *outMode = FractalProbeMode::sequence_point_set; return true; }
    if (text == "sequence_grid") { if (outMode) *outMode = FractalProbeMode::sequence_grid; return true; }
    return false;
}

bool ParseOutputModeId(const std::string& text, FractalProbeOutputMode* outMode) {
    if (text == "json") { if (outMode) *outMode = FractalProbeOutputMode::json; return true; }
    if (text == "ndjson") { if (outMode) *outMode = FractalProbeOutputMode::ndjson; return true; }
    return false;
}

bool ParseOverride(const json_min::Value& value,
    FractalProbeOverride* outOverride,
    std::string* outError) {
    if (!value.is_object()) {
        if (outError) *outError = "Override entries must be objects";
        return false;
    }
    const json_min::Object& object = value.as_object();
    if (!RejectUnknownKeys(object, {"path", "value"}, "override", outError)) return false;

    FractalProbeOverride result;
    if (!GetRequiredString(object, "path", &result.path, outError)) return false;

    auto it = object.find("value");
    if (it == object.end()) {
        if (outError) *outError = "Missing override value field";
        return false;
    }
    if (!ParseScalarValue(it->second, &result.value, outError)) return false;

    if (outOverride) *outOverride = std::move(result);
    return true;
}

bool ParsePoint(const json_min::Value& value,
    FractalProbePoint* outPoint,
    std::string* outError) {
    if (!value.is_object()) {
        if (outError) *outError = "Point entries must be objects";
        return false;
    }
    const json_min::Object& object = value.as_object();
    if (!RejectUnknownKeys(object, {"x", "y"}, "point", outError)) return false;

    FractalProbePoint point;
    if (!GetRequiredNumber(object, "x", &point.x, outError)) return false;
    if (!GetRequiredNumber(object, "y", &point.y, outError)) return false;
    if (!std::isfinite(point.x) || !std::isfinite(point.y)) {
        if (outError) *outError = "Point coordinates must be finite";
        return false;
    }

    if (outPoint) *outPoint = point;
    return true;
}

bool ParseRegion(const json_min::Value& value,
    FractalProbeRegion* outRegion,
    std::string* outError) {
    if (!value.is_object()) {
        if (outError) *outError = "Region must be an object";
        return false;
    }
    const json_min::Object& object = value.as_object();
    if (!RejectUnknownKeys(object,
            {"center_x", "center_y", "span_x", "span_y", "grid_width", "grid_height"},
            "region",
            outError)) return false;

    FractalProbeRegion region;
    if (!GetRequiredNumber(object, "center_x", &region.center_x, outError)) return false;
    if (!GetRequiredNumber(object, "center_y", &region.center_y, outError)) return false;
    if (!GetRequiredNumber(object, "span_x", &region.span_x, outError)) return false;
    if (!GetRequiredNumber(object, "span_y", &region.span_y, outError)) return false;
    if (!GetRequiredInt(object, "grid_width", &region.grid_width, outError)) return false;
    if (!GetRequiredInt(object, "grid_height", &region.grid_height, outError)) return false;

    if (!std::isfinite(region.center_x) || !std::isfinite(region.center_y) ||
        !std::isfinite(region.span_x) || !std::isfinite(region.span_y)) {
        if (outError) *outError = "Region values must be finite";
        return false;
    }
    if (region.span_x < 0.0 || region.span_y < 0.0) {
        if (outError) *outError = "Region spans must be non-negative";
        return false;
    }
    if (region.grid_width <= 0 || region.grid_height <= 0) {
        if (outError) *outError = "Region grid_width/grid_height must be > 0";
        return false;
    }

    if (outRegion) *outRegion = region;
    return true;
}

bool ParseSequenceAxis(const json_min::Value& value,
    FractalProbeSequenceAxis* outAxis,
    std::string* outError) {
    if (!value.is_object()) {
        if (outError) *outError = "Sequence axis entries must be objects";
        return false;
    }
    const json_min::Object& object = value.as_object();
    if (!RejectUnknownKeys(object, {"path", "values"}, "sequence axis", outError)) return false;

    FractalProbeSequenceAxis axis;
    if (!GetRequiredString(object, "path", &axis.path, outError)) return false;

    auto it = object.find("values");
    if (it == object.end() || !it->second.is_array()) {
        if (outError) *outError = "Sequence axis requires an array field: values";
        return false;
    }

    for (const auto& item : it->second.as_array()) {
        FractalProbeScalar scalar;
        if (!ParseScalarValue(item, &scalar, outError)) return false;
        axis.values.push_back(std::move(scalar));
    }
    if (axis.values.empty()) {
        if (outError) *outError = "Sequence axis values must not be empty";
        return false;
    }

    if (outAxis) *outAxis = std::move(axis);
    return true;
}

bool ParseSequence(const json_min::Value& value,
    FractalProbeSequence* outSequence,
    std::string* outError) {
    if (!value.is_object()) {
        if (outError) *outError = "Sequence must be an object";
        return false;
    }
    const json_min::Object& object = value.as_object();
    if (!RejectUnknownKeys(object, {"zip_paths", "vary"}, "sequence", outError)) return false;

    FractalProbeSequence sequence;
    if (!GetOptionalBool(object, "zip_paths", &sequence.zip_paths, outError)) return false;

    auto it = object.find("vary");
    if (it == object.end() || !it->second.is_array()) {
        if (outError) *outError = "Sequence requires an array field: vary";
        return false;
    }
    for (const auto& item : it->second.as_array()) {
        FractalProbeSequenceAxis axis;
        if (!ParseSequenceAxis(item, &axis, outError)) return false;
        sequence.axes.push_back(std::move(axis));
    }
    if (sequence.axes.empty()) {
        if (outError) *outError = "Sequence vary must not be empty";
        return false;
    }
    if (sequence.zip_paths) {
        const size_t count = sequence.axes.front().values.size();
        for (const auto& axis : sequence.axes) {
            if (axis.values.size() != count) {
                if (outError) *outError = "zip_paths requires all sequence value arrays to have matching lengths";
                return false;
            }
        }
    }

    if (outSequence) *outSequence = std::move(sequence);
    return true;
}

bool ParseOperatorContext(const json_min::Value& value,
    FractalProbeOperatorContext* outContext,
    std::string* outError) {
    if (!value.is_object()) {
        if (outError) *outError = "operator_context must be an object";
        return false;
    }
    const json_min::Object& object = value.as_object();
    if (!RejectUnknownKeys(object, {"source", "operator", "why"}, "operator_context", outError)) return false;

    FractalProbeOperatorContext context;
    if (!GetRequiredString(object, "source", &context.source, outError)) return false;
    if (!GetRequiredString(object, "operator", &context.operator_name, outError)) return false;
    if (!GetRequiredString(object, "why", &context.why, outError)) return false;
    if (outContext) *outContext = std::move(context);
    return true;
}

std::string EscapeJsonString(const std::string& text) {
    std::string result;
    result.reserve(text.size() + 8);
    for (char ch : text) {
        switch (ch) {
        case '\\': result += "\\\\"; break;
        case '"': result += "\\\""; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default:
            if (static_cast<unsigned char>(ch) < 0x20) {
                char buf[8];
                std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned int>(static_cast<unsigned char>(ch)));
                result += buf;
            } else {
                result.push_back(ch);
            }
            break;
        }
    }
    return result;
}

std::string ScalarToJson(const FractalProbeScalar& scalar) {
    std::ostringstream ss;
    ss << std::setprecision(17);
    switch (scalar.kind) {
    case FractalProbeScalar::Kind::boolean:
        return scalar.bool_value ? "true" : "false";
    case FractalProbeScalar::Kind::number:
        ss << scalar.number_value;
        return ss.str();
    case FractalProbeScalar::Kind::string:
        return "\"" + EscapeJsonString(scalar.string_value) + "\"";
    }
    return "null";
}

std::string DoubleToJson(double value) {
    std::ostringstream ss;
    ss << std::setprecision(17) << value;
    return ss.str();
}

std::string JsonStringLiteral(const std::string& text) {
    return "\"" + EscapeJsonString(text) + "\"";
}

void AppendJsonObject(std::ostringstream& ss,
    const std::vector<std::pair<std::string, std::string>>& entries,
    int indent) {
    const std::string pad(static_cast<size_t>(indent), ' ');
    ss << "{\n";
    for (size_t index = 0; index < entries.size(); ++index) {
        ss << pad << "  \"" << entries[index].first << "\": " << entries[index].second;
        if (index + 1 < entries.size()) ss << ",";
        ss << "\n";
    }
    ss << pad << "}";
}

void AppendJsonObjectCompact(std::ostringstream& ss,
    const std::vector<std::pair<std::string, std::string>>& entries) {
    ss << "{";
    for (size_t index = 0; index < entries.size(); ++index) {
        if (index > 0) ss << ",";
        ss << "\"" << entries[index].first << "\":" << entries[index].second;
    }
    ss << "}";
}

std::vector<std::pair<std::string, std::string>> BuildSummaryEntries(
    const FractalProbeSummary& summary,
    const FractalProbeMetricSelection& selection) {
    std::vector<std::pair<std::string, std::string>> entries;
    entries.push_back({"sample_count", std::to_string(summary.sample_count)});
    if (selection.include_summary_mean_iterations) {
        entries.push_back({"mean_iterations", DoubleToJson(summary.mean_iterations)});
    }
    if (selection.include_summary_escape_fraction) {
        entries.push_back({"escape_fraction", DoubleToJson(summary.escape_fraction)});
    }
    if (selection.include_summary_converged_fraction) {
        entries.push_back({"converged_fraction", DoubleToJson(summary.converged_fraction)});
    }
    if (selection.include_summary_nonfinite_fraction) {
        entries.push_back({"nonfinite_fraction", DoubleToJson(summary.nonfinite_fraction)});
    }
    if (selection.include_summary_pole_fraction) {
        entries.push_back({"pole_fraction", DoubleToJson(summary.pole_fraction)});
    }
    if (selection.include_summary_best_sequence_index) {
        entries.push_back({"best_sequence_index", std::to_string(summary.best_sequence_index)});
    }
    if (selection.include_summary_mean_abs2) {
        entries.push_back({"mean_abs2", DoubleToJson(summary.mean_abs2)});
    }
    if (selection.include_summary_diverged_fraction) {
        entries.push_back({"diverged_fraction", DoubleToJson(summary.diverged_fraction)});
    }
    return entries;
}

std::vector<std::pair<std::string, std::string>> BuildCostEntries(
    const FractalProbeCost& cost) {
    std::vector<std::pair<std::string, std::string>> entries;
    entries.push_back({"gpu_ms", DoubleToJson(cost.gpu_ms)});
    entries.push_back({"sample_count", std::to_string(cost.sample_count)});
    return entries;
}

std::vector<std::pair<std::string, std::string>> BuildSequenceSummaryEntries(
    const FractalProbeSequenceResult& result,
    const FractalProbeMetricSelection& selection) {
    std::vector<std::pair<std::string, std::string>> entries;
    if (selection.include_summary_mean_iterations) {
        entries.push_back({"mean_iterations", DoubleToJson(result.mean_iterations)});
    }
    if (selection.include_summary_escape_fraction) {
        entries.push_back({"escape_fraction", DoubleToJson(result.escape_fraction)});
    }
    if (selection.include_summary_converged_fraction) {
        entries.push_back({"converged_fraction", DoubleToJson(result.converged_fraction)});
    }
    if (selection.include_summary_nonfinite_fraction) {
        entries.push_back({"nonfinite_fraction", DoubleToJson(result.nonfinite_fraction)});
    }
    if (selection.include_summary_pole_fraction) {
        entries.push_back({"pole_fraction", DoubleToJson(result.pole_fraction)});
    }
    return entries;
}

std::vector<std::pair<std::string, std::string>> BuildSampleEntries(
    const FractalProbeSample& sample,
    const FractalProbeMetricSelection& selection) {
    std::vector<std::pair<std::string, std::string>> entries;
    entries.push_back({"sequence_index", std::to_string(sample.sequence_index)});
    entries.push_back({"grid_x", std::to_string(sample.grid_x)});
    entries.push_back({"grid_y", std::to_string(sample.grid_y)});
    entries.push_back({"coord_x", DoubleToJson(sample.coord_x)});
    entries.push_back({"coord_y", DoubleToJson(sample.coord_y)});
    if (selection.include_iterations) {
        entries.push_back({"iterations", std::to_string(sample.iterations)});
    }
    if (selection.include_status) {
        entries.push_back({"status", JsonStringLiteral(FractalProbeSampleStatusId(sample.status))});
    }
    if (selection.include_final_z) {
        entries.push_back({"final_z_x", DoubleToJson(sample.final_z_x)});
        entries.push_back({"final_z_y", DoubleToJson(sample.final_z_y)});
    }
    if (selection.include_final_abs2) {
        entries.push_back({"final_abs2", DoubleToJson(sample.final_abs2)});
    }
    if (selection.include_residual) {
        entries.push_back({"residual", sample.has_residual ? DoubleToJson(sample.residual) : std::string("null")});
    }
    if (selection.include_root_index) {
        entries.push_back({"root_index", sample.has_root_index ? std::to_string(sample.root_index) : std::string("null")});
    }
    if (selection.include_value) {
        entries.push_back({"value_x", DoubleToJson(sample.final_z_x)});
        entries.push_back({"value_y", DoubleToJson(sample.final_z_y)});
    }
    if (selection.include_abs2) {
        entries.push_back({"abs2", DoubleToJson(sample.final_abs2)});
    }
    if (selection.include_derivative) {
        entries.push_back({"derivative_x", DoubleToJson(sample.derivative_x)});
        entries.push_back({"derivative_y", DoubleToJson(sample.derivative_y)});
    }
    return entries;
}

void AppendSummaryJson(std::ostringstream& ss,
    const FractalProbeSummary& summary,
    const FractalProbeMetricSelection& selection,
    int indent) {
    const std::vector<std::pair<std::string, std::string>> entries = BuildSummaryEntries(summary, selection);
    const std::string pad(static_cast<size_t>(indent), ' ');
    ss << pad << "\"summary\": ";
    AppendJsonObject(ss, entries, indent);
}

void AppendCostJson(std::ostringstream& ss,
    const FractalProbeCost& cost,
    int indent) {
    const std::vector<std::pair<std::string, std::string>> entries = BuildCostEntries(cost);
    const std::string pad(static_cast<size_t>(indent), ' ');
    ss << pad << "\"cost\": ";
    AppendJsonObject(ss, entries, indent);
}

void AppendSequenceSummaryJson(std::ostringstream& ss,
    const FractalProbeSequenceResult& result,
    const FractalProbeMetricSelection& selection,
    int indent) {
    const std::vector<std::pair<std::string, std::string>> entries = BuildSequenceSummaryEntries(result, selection);
    const std::string pad(static_cast<size_t>(indent), ' ');
    ss << pad << "\"summary\": ";
    AppendJsonObject(ss, entries, indent);
}

void AppendSampleJson(std::ostringstream& ss,
    const FractalProbeSample& sample,
    const FractalProbeMetricSelection& selection,
    int indent) {
    const std::vector<std::pair<std::string, std::string>> entries = BuildSampleEntries(sample, selection);
    const std::string pad(static_cast<size_t>(indent), ' ');
    ss << pad;
    AppendJsonObject(ss, entries, indent);
}

} // namespace

FractalProbeMetricSelection BuildFractalProbeMetricSelection(const std::vector<std::string>& metrics) {
    FractalProbeMetricSelection selection;
    if (metrics.empty()) return selection;

    selection.include_iterations = false;
    selection.include_status = false;
    selection.include_final_z = false;
    selection.include_final_abs2 = false;
    selection.include_residual = false;
    selection.include_root_index = false;
    selection.include_summary_mean_iterations = false;
    selection.include_summary_escape_fraction = false;
    selection.include_summary_converged_fraction = false;
    selection.include_summary_nonfinite_fraction = false;
    selection.include_summary_pole_fraction = false;
    selection.include_summary_best_sequence_index = false;
    selection.include_derivative = false;
    selection.include_value = false;
    selection.include_abs2 = false;
    selection.include_summary_mean_abs2 = false;
    selection.include_summary_diverged_fraction = false;
    for (const std::string& metric : metrics) {
        if (metric == "iterations") selection.include_iterations = true;
        else if (metric == "status") selection.include_status = true;
        else if (metric == "final_z") selection.include_final_z = true;
        else if (metric == "final_abs2") selection.include_final_abs2 = true;
        else if (metric == "residual") selection.include_residual = true;
        else if (metric == "root_index") selection.include_root_index = true;
        else if (metric == "summary_mean_iterations") selection.include_summary_mean_iterations = true;
        else if (metric == "summary_escape_fraction") selection.include_summary_escape_fraction = true;
        else if (metric == "summary_converged_fraction") selection.include_summary_converged_fraction = true;
        else if (metric == "summary_nonfinite_fraction") selection.include_summary_nonfinite_fraction = true;
        else if (metric == "summary_pole_fraction") selection.include_summary_pole_fraction = true;
        else if (metric == "summary_best_sequence_index") selection.include_summary_best_sequence_index = true;
        else if (metric == "value") selection.include_value = true;
        else if (metric == "abs2") selection.include_abs2 = true;
        else if (metric == "derivative") selection.include_derivative = true;
        else if (metric == "summary_mean_abs2") selection.include_summary_mean_abs2 = true;
        else if (metric == "summary_diverged_fraction") selection.include_summary_diverged_fraction = true;
    }
    return selection;
}

bool FractalProbeSelectionIncludesAnySampleMetrics(const FractalProbeMetricSelection& selection) {
    return selection.include_iterations ||
        selection.include_status ||
        selection.include_final_z ||
        selection.include_final_abs2 ||
        selection.include_residual ||
        selection.include_root_index ||
        selection.include_value ||
        selection.include_abs2 ||
        selection.include_derivative;
}

const char* FractalProbeModeId(FractalProbeMode mode) {
    switch (mode) {
    case FractalProbeMode::point_set: return "point_set";
    case FractalProbeMode::grid: return "grid";
    case FractalProbeMode::sequence_point_set: return "sequence_point_set";
    case FractalProbeMode::sequence_grid: return "sequence_grid";
    }
    return "point_set";
}

const char* FractalProbeOutputModeId(FractalProbeOutputMode mode) {
    switch (mode) {
    case FractalProbeOutputMode::json: return "json";
    case FractalProbeOutputMode::ndjson: return "ndjson";
    }
    return "json";
}

const char* FractalProbeSampleStatusId(FractalProbeSampleStatus status) {
    switch (status) {
    case FractalProbeSampleStatus::escaped: return "escaped";
    case FractalProbeSampleStatus::converged: return "converged";
    case FractalProbeSampleStatus::bounded: return "bounded";
    case FractalProbeSampleStatus::pole: return "pole";
    case FractalProbeSampleStatus::nonfinite: return "nonfinite";
    case FractalProbeSampleStatus::invalid_param: return "invalid_param";
    }
    return "bounded";
}

bool ParseFractalProbeRequestFromValue(const json_min::Value& value,
    FractalProbeRequest* outRequest,
    std::string* outError) {
    if (outError) outError->clear();
    if (!outRequest) {
        if (outError) *outError = "ParseFractalProbeRequestFromValue requires outRequest";
        return false;
    }
    if (!value.is_object()) {
        if (outError) *outError = "Probe request root must be an object";
        return false;
    }

    const json_min::Object& root = value.as_object();
    if (!RejectUnknownKeys(root,
            {"request_version", "request_id", "function_id", "function", "mode", "output_mode", "base_state", "overrides", "region", "points", "sequence", "metrics", "operator_context", "state_token"},
            "request",
            outError)) return false;

    FractalProbeRequest request;
    if (!GetRequiredInt(root, "request_version", &request.request_version, outError)) return false;
    if (request.request_version != 1) {
        if (outError) *outError = "Unsupported request_version: " + std::to_string(request.request_version);
        return false;
    }
    if (!GetRequiredString(root, "request_id", &request.request_id, outError)) return false;

    // V2-C: state_token is optional — references accumulated session state.
    {
        auto stIt = root.find("state_token");
        if (stIt != root.end()) {
            if (!stIt->second.is_string()) {
                if (outError) *outError = "state_token must be a string";
                return false;
            }
            request.state_token = stIt->second.as_string();
        }
    }

    // function_id is optional; defaults to "fractal.sample" for backward compat.
    {
        auto fidIt = root.find("function_id");
        if (fidIt != root.end()) {
            if (!fidIt->second.is_string()) {
                if (outError) *outError = "function_id must be a string";
                return false;
            }
            request.function_id = fidIt->second.as_string();
        } else {
            request.function_id = "fractal.sample";
        }
    }

    // Parse "function" block for generic.sample requests.
    {
        auto funcIt = root.find("function");
        if (funcIt != root.end()) {
            if (!funcIt->second.is_object()) {
                if (outError) *outError = "function must be an object";
                return false;
            }
            const json_min::Object& funcObj = funcIt->second.as_object();
            if (!RejectUnknownKeys(funcObj,
                    {"expression", "params", "epsilon", "escape_radius"},
                    "function",
                    outError)) return false;
            request.has_function = true;

            auto exprIt = funcObj.find("expression");
            if (exprIt == funcObj.end() || !exprIt->second.is_string()) {
                if (outError) *outError = "function.expression must be a string";
                return false;
            }
            request.generic_expression = exprIt->second.as_string();

            auto paramsIt = funcObj.find("params");
            if (paramsIt != funcObj.end()) {
                if (!paramsIt->second.is_object()) {
                    if (outError) *outError = "function.params must be an object";
                    return false;
                }
                for (const auto& kv : paramsIt->second.as_object()) {
                    if (!kv.second.is_number()) {
                        if (outError) *outError = "function.params values must be numbers";
                        return false;
                    }
                    request.generic_params[kv.first] = kv.second.as_number();
                }
            }

            auto epsIt = funcObj.find("epsilon");
            if (epsIt != funcObj.end()) {
                if (!epsIt->second.is_number()) {
                    if (outError) *outError = "function.epsilon must be a number";
                    return false;
                }
                request.generic_epsilon = epsIt->second.as_number();
            }

            auto escIt = funcObj.find("escape_radius");
            if (escIt != funcObj.end()) {
                if (!escIt->second.is_number()) {
                    if (outError) *outError = "function.escape_radius must be a number";
                    return false;
                }
                request.generic_escape_radius = escIt->second.as_number();
            }
        }
    }

    std::string modeText;
    if (!GetRequiredString(root, "mode", &modeText, outError)) return false;
    if (!ParseModeId(modeText, &request.mode)) {
        if (outError) *outError = "Unsupported probe mode: " + modeText;
        return false;
    }

    {
        auto outputIt = root.find("output_mode");
        if (outputIt != root.end()) {
            if (!outputIt->second.is_string()) {
                if (outError) *outError = "output_mode must be a string";
                return false;
            }
            if (!ParseOutputModeId(outputIt->second.as_string(), &request.output_mode)) {
                if (outError) *outError = "Unsupported output_mode: " + outputIt->second.as_string();
                return false;
            }
        }
    }

    auto it = root.find("base_state");
    if (it != root.end()) {
        if (!it->second.is_object()) {
            if (outError) *outError = "base_state must be an object";
            return false;
        }
        const json_min::Object& base = it->second.as_object();
        if (!RejectUnknownKeys(base, {"load_state_json"}, "base_state", outError)) return false;
        if (!GetRequiredString(base, "load_state_json", &request.base_state_load_path, outError)) return false;
    }

    it = root.find("overrides");
    if (it != root.end()) {
        if (!it->second.is_array()) {
            if (outError) *outError = "overrides must be an array";
            return false;
        }
        for (const auto& item : it->second.as_array()) {
            FractalProbeOverride overrideValue;
            if (!ParseOverride(item, &overrideValue, outError)) return false;
            request.overrides.push_back(std::move(overrideValue));
        }
    }

    it = root.find("region");
    if (it != root.end()) {
        request.has_region = true;
        if (!ParseRegion(it->second, &request.region, outError)) return false;
    }

    it = root.find("points");
    if (it != root.end()) {
        if (!it->second.is_array()) {
            if (outError) *outError = "points must be an array";
            return false;
        }
        for (const auto& item : it->second.as_array()) {
            FractalProbePoint point;
            if (!ParsePoint(item, &point, outError)) return false;
            request.points.push_back(point);
        }
    }

    it = root.find("sequence");
    if (it != root.end()) {
        request.has_sequence = true;
        if (!ParseSequence(it->second, &request.sequence, outError)) return false;
    }

    it = root.find("metrics");
    if (it != root.end()) {
        if (!it->second.is_array()) {
            if (outError) *outError = "metrics must be an array";
            return false;
        }
        for (const auto& item : it->second.as_array()) {
            if (!item.is_string()) {
                if (outError) *outError = "metrics entries must be strings";
                return false;
            }
            const std::string metric = item.as_string();
            if (!IsSupportedMetric(metric)) {
                if (outError) *outError = "Unsupported metric name: " + metric;
                return false;
            }
            request.metrics.push_back(metric);
        }
    }

    it = root.find("operator_context");
    if (it != root.end()) {
        if (!ParseOperatorContext(it->second, &request.operator_context, outError)) return false;
    }

    const bool usesPointSet = request.mode == FractalProbeMode::point_set || request.mode == FractalProbeMode::sequence_point_set;
    const bool usesGrid = request.mode == FractalProbeMode::grid || request.mode == FractalProbeMode::sequence_grid;
    const bool usesSequence = request.mode == FractalProbeMode::sequence_point_set || request.mode == FractalProbeMode::sequence_grid;

    if (usesPointSet && request.points.empty()) {
        if (outError) *outError = "point_set modes require a non-empty points array";
        return false;
    }
    if (!usesPointSet && !request.points.empty()) {
        if (outError) *outError = "points is only valid for point_set modes";
        return false;
    }
    if (usesGrid && !request.has_region) {
        if (outError) *outError = "grid modes require a region object";
        return false;
    }
    if (!usesGrid && request.has_region) {
        if (outError) *outError = "region is only valid for grid modes";
        return false;
    }
    if (usesSequence && !request.has_sequence) {
        if (outError) *outError = "sequence modes require a sequence object";
        return false;
    }
    if (!usesSequence && request.has_sequence) {
        if (outError) *outError = "sequence is only valid for sequence_* modes";
        return false;
    }

    *outRequest = std::move(request);
    return true;
}

bool ParseFractalProbeRequestJson(const std::string& text,
    FractalProbeRequest* outRequest,
    std::string* outError) {
    if (outError) outError->clear();
    if (!outRequest) {
        if (outError) *outError = "ParseFractalProbeRequestJson requires outRequest";
        return false;
    }
    json_min::ParseResult parsed = json_min::Parse(text);
    if (!parsed.error.empty()) {
        if (outError) *outError = parsed.error;
        return false;
    }
    return ParseFractalProbeRequestFromValue(parsed.value, outRequest, outError);
}

static void AppendSequenceResultsJson(std::ostringstream& ss,
    const std::vector<FractalProbeSequenceResult>& results,
    const FractalProbeMetricSelection& selection) {
    ss << "  \"sequence_results\": [\n";
    for (size_t index = 0; index < results.size(); ++index) {
        const FractalProbeSequenceResult& result = results[index];
        ss << "    {\n";
        ss << "      \"sequence_index\": " << result.sequence_index << ",\n";
        ss << "      \"applied\": {";
        for (size_t itemIndex = 0; itemIndex < result.applied.size(); ++itemIndex) {
            const auto& item = result.applied[itemIndex];
            if (itemIndex > 0) ss << ", ";
            ss << "\"" << EscapeJsonString(item.first) << "\": " << ScalarToJson(item.second);
        }
        ss << "},\n";
        AppendSequenceSummaryJson(ss, result, selection, 6);
        ss << "\n    }";
        if (index + 1 < results.size()) ss << ",";
        ss << "\n";
    }
    ss << "  ],\n";
}

std::string SerializeFractalProbeResponseJson(const FractalProbeResponse& response) {
    std::ostringstream ss;
    ss << std::setprecision(17);
    ss << "{\n";
    ss << "  \"response_version\": " << response.response_version << ",\n";
    ss << "  \"request_id\": \"" << EscapeJsonString(response.request_id) << "\",\n";
    if (!response.function_id.empty()) {
        ss << "  \"function_id\": \"" << EscapeJsonString(response.function_id) << "\",\n";
    }
    ss << "  \"ok\": " << (response.ok ? "true" : "false") << ",\n";
    ss << "  \"runtime\": {\n";
    ss << "    \"exe_path\": \"" << EscapeJsonString(response.runtime.exe_path) << "\",\n";
    ss << "    \"fractal_type\": \"" << EscapeJsonString(response.runtime.fractal_type) << "\",\n";
    ss << "    \"device_id\": " << response.runtime.device_id << "\n";
    ss << "  },\n";
    AppendSummaryJson(ss, response.summary, response.metric_selection, 2);
    ss << ",\n";
    AppendCostJson(ss, response.cost, 2);
    ss << ",\n";
    AppendSequenceResultsJson(ss, response.sequence_results, response.metric_selection);
    ss << "  \"samples\": [\n";
    for (size_t index = 0; index < response.samples.size(); ++index) {
        const FractalProbeSample& sample = response.samples[index];
        AppendSampleJson(ss, sample, response.metric_selection, 4);
        if (index + 1 < response.samples.size()) ss << ",";
        ss << "\n";
    }
    ss << "  ],\n";
    ss << "  \"operator_context\": {\n";
    ss << "    \"source\": \"" << EscapeJsonString(response.operator_context.source) << "\",\n";
    ss << "    \"operator\": \"" << EscapeJsonString(response.operator_context.operator_name) << "\",\n";
    ss << "    \"why\": \"" << EscapeJsonString(response.operator_context.why) << "\"\n";
    ss << "  },\n";
    ss << "  \"error\": " << (response.error.empty() ? std::string("null") : std::string("\"") + EscapeJsonString(response.error) + "\"") << "\n";
    ss << "}\n";
    return ss.str();
}

std::string SerializeFractalProbeNdjsonSampleBatchJson(
    const std::string& requestId,
    const std::string& functionId,
    int sequenceIndex,
    int rowIndex,
    const std::vector<FractalProbeSample>& samples,
    const FractalProbeMetricSelection& selection) {
    std::ostringstream sampleArray;
    sampleArray << "[";
    for (size_t index = 0; index < samples.size(); ++index) {
        if (index > 0) sampleArray << ",";
        AppendJsonObjectCompact(sampleArray, BuildSampleEntries(samples[index], selection));
    }
    sampleArray << "]";

    std::vector<std::pair<std::string, std::string>> entries;
    entries.push_back({"type", JsonStringLiteral("sample_batch")});
    entries.push_back({"request_id", JsonStringLiteral(requestId)});
    if (!functionId.empty()) {
        entries.push_back({"function_id", JsonStringLiteral(functionId)});
    }
    entries.push_back({"sequence_index", std::to_string(sequenceIndex)});
    if (rowIndex >= 0) {
        entries.push_back({"row_index", std::to_string(rowIndex)});
    }
    entries.push_back({"samples", sampleArray.str()});

    std::ostringstream ss;
    AppendJsonObjectCompact(ss, entries);
    return ss.str();
}

std::string SerializeFractalProbeNdjsonSummaryJson(
    const FractalProbeResponse& response,
    const std::string& stateToken) {
    std::ostringstream summaryJson;
    AppendJsonObjectCompact(summaryJson, BuildSummaryEntries(response.summary, response.metric_selection));

    std::ostringstream costJson;
    AppendJsonObjectCompact(costJson, BuildCostEntries(response.cost));

    std::vector<std::pair<std::string, std::string>> entries;
    entries.push_back({"type", JsonStringLiteral("summary")});
    entries.push_back({"request_id", JsonStringLiteral(response.request_id)});
    if (!response.function_id.empty()) {
        entries.push_back({"function_id", JsonStringLiteral(response.function_id)});
    }
    entries.push_back({"ok", response.ok ? "true" : "false"});
    entries.push_back({"summary", summaryJson.str()});
    entries.push_back({"cost", costJson.str()});
    if (!stateToken.empty()) {
        entries.push_back({"state_token", JsonStringLiteral(stateToken)});
    }
    if (!response.error.empty()) {
        entries.push_back({"error", JsonStringLiteral(response.error)});
    }

    std::ostringstream ss;
    AppendJsonObjectCompact(ss, entries);
    return ss.str();
}