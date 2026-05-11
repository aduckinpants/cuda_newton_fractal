#include "../src/fractal_probe_contract.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

int g_passed = 0;
int g_failed = 0;

void Check(bool condition, const char* message) {
    if (condition) {
        ++g_passed;
    } else {
        ++g_failed;
        std::cerr << "FAIL: " << message << "\n";
    }
}

bool Contains(const std::string& text, const char* needle) {
    return text.find(needle) != std::string::npos;
}

void CheckParsesJson(const std::string& text, const char* message) {
    const json_min::ParseResult parsed = json_min::Parse(text);
    Check(parsed.error.empty(), message);
}

void TestScalarFactoriesAndDefaults() {
    const FractalProbeScalar boolean = FractalProbeScalar::Boolean(true);
    Check(boolean.kind == FractalProbeScalar::Kind::boolean && boolean.bool_value,
        "Boolean scalar factory preserves kind and value");

    const FractalProbeScalar number = FractalProbeScalar::Number(2.5);
    Check(number.kind == FractalProbeScalar::Kind::number && number.number_value == 2.5,
        "Number scalar factory preserves kind and value");

    const FractalProbeScalar string = FractalProbeScalar::String("explaino");
    Check(string.kind == FractalProbeScalar::Kind::string && string.string_value == "explaino",
        "String scalar factory preserves kind and value");

    const FractalProbeRequest request{};
    Check(request.mode == FractalProbeMode::point_set, "Request defaults to point_set mode");
    Check(request.output_mode == FractalProbeOutputMode::json, "Request defaults to JSON output");
    Check(request.execution.backend_preference == FractalProbeExecutionBackendPreference::default_backend,
        "Request defaults to automatic backend selection");
    Check(request.generic_epsilon == 1e-6 && request.generic_escape_radius == 1000.0,
        "Generic sample defaults preserve numeric contracts");

    const FractalProbeSample sample{};
    Check(sample.status == FractalProbeSampleStatus::bounded, "Sample defaults to bounded status");
    Check(sample.termination_kind == TerminationKind::none, "Sample defaults to no termination kind");
}

void TestParseRichSequenceGridRequest() {
    const std::string requestJson = R"({
  "request_version": 1,
  "request_id": "contract-sequence-grid",
  "function_id": "generic.sample",
  "function": {
    "expression": "z*z+c",
    "params": { "c_re": -0.5, "c_im": 0.25 },
    "epsilon": 0.0001,
    "escape_radius": 64.0
  },
  "state_token": "session-42",
  "mode": "sequence_grid",
  "output_mode": "ndjson",
  "execution": { "backend_preference": "cpu" },
  "base_state": { "load_state_json": "state.json" },
  "overrides": [
    { "path": "fractal.view.auto_max_iter", "value": true },
    { "path": "fractal.params.explaino_seed", "value": 3.5 },
    { "path": "fractal.view.fractal_type", "value": "explaino" }
  ],
  "region": {
    "center_x": 0.0,
    "center_y": 0.0,
    "span_x": 1.0,
    "span_y": 1.0,
    "grid_width": 2,
    "grid_height": 2
  },
  "sequence": {
    "mode": "axes",
    "zip_paths": true,
    "vary": [
      { "path": "fractal.params.explaino_seed", "values": [1.0, 2.0] },
      { "path": "fractal.view.explaino_seed_drift", "values": [0.1, 0.2] }
    ]
  },
  "metrics": ["iterations", "status", "summary_mean_iterations", "derivative"],
  "operator_context": {
    "source": "salticid",
    "operator": "probe_contract_test",
    "why": "prove request parsing"
  }
})";

    FractalProbeRequest request{};
    std::string error;
    const bool ok = ParseFractalProbeRequestJson(requestJson, &request, &error);
    Check(ok, "Rich sequence_grid request parses");
    Check(error.empty(), "Successful parse clears error");
    Check(request.request_id == "contract-sequence-grid" && request.function_id == "generic.sample",
        "Request header fields parse");
    Check(request.has_function && request.generic_expression == "z*z+c" && request.generic_params.size() == 2,
        "Generic function block parses");
    Check(request.generic_epsilon == 0.0001 && request.generic_escape_radius == 64.0,
        "Generic numeric tolerances parse");
    Check(request.state_token == "session-42" && request.base_state_load_path == "state.json",
        "State token and base state parse");
    Check(request.mode == FractalProbeMode::sequence_grid && request.output_mode == FractalProbeOutputMode::ndjson,
        "Mode and output mode parse");
    Check(request.execution.backend_preference == FractalProbeExecutionBackendPreference::cpu,
        "Execution backend preference parses");
    Check(request.overrides.size() == 3 && request.overrides[0].value.kind == FractalProbeScalar::Kind::boolean,
        "Override scalar variants parse");
    Check(request.has_region && request.region.grid_width == 2 && request.region.grid_height == 2,
        "Grid region parses");
    Check(request.has_sequence && request.sequence.zip_paths && request.sequence.axes.size() == 2,
        "Zipped sequence axes parse");
    Check(request.metrics.size() == 4 && request.operator_context.operator_name == "probe_contract_test",
        "Metrics and operator context parse");
}

void TestRejectsInvalidRequests() {
    FractalProbeRequest request{};
    std::string error;

    const bool unknownOk = ParseFractalProbeRequestJson(R"({
  "request_version": 1,
  "request_id": "bad",
  "mode": "point_set",
  "points": [{ "x": 0.0, "y": 0.0 }],
  "surprise": true
})", &request, &error);
    Check(!unknownOk && Contains(error, "surprise"), "Unknown top-level keys fail closed");

    const bool badMetricOk = ParseFractalProbeRequestJson(R"({
  "request_version": 1,
  "request_id": "bad-metric",
  "mode": "point_set",
  "points": [{ "x": 0.0, "y": 0.0 }],
  "metrics": ["iterations", "mystery_metric"]
})", &request, &error);
    Check(!badMetricOk && Contains(error, "mystery_metric"), "Unsupported metrics fail closed");

    const bool badRegionOk = ParseFractalProbeRequestJson(R"({
  "request_version": 1,
  "request_id": "bad-region",
  "mode": "grid",
  "region": {
    "center_x": 0.0,
    "center_y": 0.0,
    "span_x": -1.0,
    "span_y": 1.0,
    "grid_width": 2,
    "grid_height": 2
  }
})", &request, &error);
    Check(!badRegionOk && Contains(error, "non-negative"), "Negative region spans fail closed");

    const bool zipOk = ParseFractalProbeRequestJson(R"({
  "request_version": 1,
  "request_id": "bad-zip",
  "mode": "sequence_point_set",
  "points": [{ "x": 0.0, "y": 0.0 }],
  "sequence": {
    "zip_paths": true,
    "vary": [
      { "path": "a", "values": [1.0, 2.0] },
      { "path": "b", "values": [3.0] }
    ]
  }
})", &request, &error);
    Check(!zipOk && Contains(error, "matching lengths"), "Mismatched zipped sequence axes fail closed");
}

void TestMetricSelectionContract() {
    const FractalProbeMetricSelection defaultSelection = BuildFractalProbeMetricSelection({});
    Check(defaultSelection.include_iterations && defaultSelection.include_status && defaultSelection.include_derivative,
        "Empty metrics request includes the full sample set");
    Check(FractalProbeSelectionIncludesAnySampleMetrics(defaultSelection),
        "Default metric selection includes sample metrics");

    const FractalProbeMetricSelection summaryOnly = BuildFractalProbeMetricSelection({"summary_mean_abs2"});
    Check(!summaryOnly.include_iterations && !summaryOnly.include_status && summaryOnly.include_summary_mean_abs2,
        "Explicit summary-only metrics suppress sample fields");
    Check(!FractalProbeSelectionIncludesAnySampleMetrics(summaryOnly),
        "Summary-only selection reports no sample metrics");

    const FractalProbeMetricSelection statusOnly = BuildFractalProbeMetricSelection({"status"});
    Check(statusOnly.include_status && !statusOnly.include_iterations,
        "Explicit status metric includes only requested sample field");
    Check(FractalProbeSelectionIncludesAnySampleMetrics(statusOnly),
        "Status-only selection reports sample metrics");
}

void TestSerializationAndIds() {
    Check(std::string(FractalProbeModeId(FractalProbeMode::sequence_grid)) == "sequence_grid",
        "Mode id maps sequence_grid");
    Check(std::string(FractalProbeOutputModeId(FractalProbeOutputMode::ndjson)) == "ndjson",
        "Output mode id maps ndjson");
    Check(std::string(FractalProbeSampleStatusId(FractalProbeSampleStatus::invalid_param)) == "invalid_param",
        "Sample status id maps invalid_param");
    Check(std::string(TerminationKindId(TerminationKind::far_field_settled)) == "far_field_settled",
        "Termination kind id maps far_field_settled");

    FractalProbeResponse response{};
    response.request_id = "serialize-contract";
    response.function_id = "generic.sample";
    response.ok = true;
    response.runtime.fractal_type = "generic";
    response.runtime.backend_used = "cpu";
    response.summary.sample_count = 1;
    response.summary.mean_abs2 = 9.0;
    response.metric_selection = BuildFractalProbeMetricSelection({"status", "value", "derivative", "summary_mean_abs2"});

    FractalProbeSample sample{};
    sample.coord_x = 0.25;
    sample.coord_y = -0.5;
    sample.status = FractalProbeSampleStatus::bounded;
    sample.termination_kind = TerminationKind::max_iterations;
    sample.final_z_x = 1.5;
    sample.final_z_y = -2.0;
    sample.final_abs2 = 6.25;
    sample.derivative_x = 0.75;
    sample.derivative_y = -0.25;
    response.samples.push_back(sample);

    const std::string json = SerializeFractalProbeResponseJson(response);
    CheckParsesJson(json, "Response JSON parses");
    Check(Contains(json, "\"function_id\": \"generic.sample\""), "Response JSON includes function_id");
    Check(Contains(json, "\"mean_abs2\": 9"), "Response JSON includes selected summary metric");
    Check(Contains(json, "\"status\": \"bounded\""), "Response JSON includes selected sample status");
    Check(Contains(json, "\"value_x\": 1.5"), "Response JSON includes selected value field");
    Check(Contains(json, "\"derivative_x\": 0.75"), "Response JSON includes selected derivative field");
    Check(!Contains(json, "\"iterations\""), "Response JSON suppresses unselected iterations field");

    const std::string batch = SerializeFractalProbeNdjsonSampleBatchJson(
        "serialize-contract",
        "generic.sample",
        2,
        7,
        response.samples,
        response.metric_selection);
    CheckParsesJson(batch, "NDJSON batch JSON parses");
    Check(Contains(batch, "\"type\":\"sample_batch\""), "NDJSON batch includes type");
    Check(Contains(batch, "\"row_index\":7"), "NDJSON batch includes row index");
    Check(Contains(batch, "\"function_id\":\"generic.sample\""), "NDJSON batch includes function_id");
    Check(!Contains(batch, "\"iterations\""), "NDJSON batch suppresses unselected iterations field");

    const std::string summary = SerializeFractalProbeNdjsonSummaryJson(response, "state-99");
    CheckParsesJson(summary, "NDJSON summary JSON parses");
    Check(Contains(summary, "\"type\":\"summary\""), "NDJSON summary includes type");
    Check(Contains(summary, "\"state_token\":\"state-99\""), "NDJSON summary includes state token");
}

} // namespace

int main() {
    TestScalarFactoriesAndDefaults();
    TestParseRichSequenceGridRequest();
    TestRejectsInvalidRequests();
    TestMetricSelectionContract();
    TestSerializationAndIds();

    std::cout << "test_fractal_probe_contract: passed=" << g_passed << " failed=" << g_failed << "\n";
    return g_failed == 0 ? 0 : 1;
}
