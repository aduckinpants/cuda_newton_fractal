#include "../src/fractal_probe_contract.h"
#include "../src/fractal_probe_runner.h"

#include "../src/enum_id_utils.h"
#include "../src/fractal_family_rules.h"
#include "../src/json_min.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

bool NearlyEqual(double left, double right, double eps = 1.0e-6) {
    const double delta = left - right;
    return delta < eps && delta > -eps;
}

bool WriteTextFile(const std::filesystem::path& path, const std::string& text) {
    std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file) return false;
    file.write(text.data(), static_cast<std::streamsize>(text.size()));
    return file.good();
}

bool CompareProbeSamples(const FractalProbeSample& expected,
    const FractalProbeSample& actual,
    const std::string& label) {
    if (expected.status != actual.status) {
        std::cerr << label << " status mismatch\n";
        return false;
    }
    if (std::abs(expected.iterations - actual.iterations) > 1) {
        std::cerr << label << " iteration mismatch\n";
        return false;
    }
    if (expected.has_root_index != actual.has_root_index) {
        std::cerr << label << " root-index presence mismatch\n";
        return false;
    }
    if (expected.has_root_index && expected.root_index != actual.root_index) {
        std::cerr << label << " root-index mismatch\n";
        return false;
    }
    if (!NearlyEqual(expected.final_z_x, actual.final_z_x, 1.0e-3) ||
        !NearlyEqual(expected.final_z_y, actual.final_z_y, 1.0e-3)) {
        std::cerr << label << " final_z mismatch\n";
        return false;
    }
    if (expected.has_residual != actual.has_residual) {
        std::cerr << label << " residual presence mismatch\n";
        return false;
    }
    if (expected.has_residual && !NearlyEqual(expected.residual, actual.residual, 1.0e-4)) {
        std::cerr << label << " residual mismatch\n";
        return false;
    }
    return true;
}

bool CompareProbeResponses(const FractalProbeResponse& expected,
    const FractalProbeResponse& actual,
    const std::string& label) {
    if (!expected.ok || !actual.ok) {
        std::cerr << label << " expected both responses to be ok\n";
        return false;
    }
    if (expected.summary.sample_count != actual.summary.sample_count ||
        expected.samples.size() != actual.samples.size()) {
        std::cerr << label << " sample-count mismatch\n";
        return false;
    }
    for (size_t index = 0; index < expected.samples.size(); ++index) {
        if (!CompareProbeSamples(expected.samples[index], actual.samples[index],
                label + " sample[" + std::to_string(index) + "]")) {
            return false;
        }
    }
    return true;
}

bool RunExplainoPointProbe(const std::string& requestId,
    const char* fractalType,
    double rippleAmplitude,
    double spliceOffset,
    double vortexStrength,
    double tensionStrength,
    FractalProbeResponse* outResponse,
    std::string* outError) {
    FractalProbeRequest request{};
    request.request_version = 1;
    request.request_id = requestId;
    request.function_id = "fractal.sample";
    request.mode = FractalProbeMode::point_set;
    request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String(fractalType)});
    request.overrides.push_back({"fractal.params.ripple_amplitude", FractalProbeScalar::Number(rippleAmplitude)});
    request.overrides.push_back({"fractal.params.splice_offset", FractalProbeScalar::Number(spliceOffset)});
    request.overrides.push_back({"fractal.params.vortex_strength", FractalProbeScalar::Number(vortexStrength)});
    request.overrides.push_back({"fractal.params.tension_strength", FractalProbeScalar::Number(tensionStrength)});
    request.points.push_back({0.125, 0.0});
    request.points.push_back({0.25, 0.0});
    request.points.push_back({-0.125, 0.125});
    return RunFractalProbeRequest(request, "probe.exe", outResponse, outError);
}

bool RunExplainoLegacyCouplingProbe(const std::string& requestId,
    const char* fractalType,
    double momentumBeta,
    double joyCoupling,
    double foldCoupling,
    double bellCoupling,
    FractalProbeResponse* outResponse,
    std::string* outError) {
    FractalProbeRequest request{};
    request.request_version = 1;
    request.request_id = requestId;
    request.function_id = "fractal.sample";
    request.mode = FractalProbeMode::point_set;
    request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String(fractalType)});
    request.overrides.push_back({"fractal.params.momentum_beta", FractalProbeScalar::Number(momentumBeta)});
    request.overrides.push_back({"fractal.params.joy_coupling", FractalProbeScalar::Number(joyCoupling)});
    request.overrides.push_back({"fractal.params.fold_coupling", FractalProbeScalar::Number(foldCoupling)});
    request.overrides.push_back({"fractal.params.bell_coupling", FractalProbeScalar::Number(bellCoupling)});
    request.points.push_back({0.125, 0.0});
    request.points.push_back({0.25, 0.0});
    request.points.push_back({-0.125, 0.125});
    return RunFractalProbeRequest(request, "probe.exe", outResponse, outError);
}

} // namespace

int main() {
    {
        const std::string requestJson = R"({
  "request_version": 1,
  "request_id": "probe-sequence-grid",
  "mode": "sequence_grid",
  "base_state": {
    "load_state_json": "D:/salt-fractal/cuda_newton_fractal_clone/runtime/diagnostics/last/state.json"
  },
  "overrides": [
    { "path": "fractal.view.fractal_type", "value": "explaino_lambda" },
    { "path": "fractal.params.lambda_real", "value": 2.9685855 },
    { "path": "fractal.params.lambda_imag", "value": -0.27446103 }
  ],
  "region": {
    "center_x": 0.5,
    "center_y": 0.0,
    "span_x": 0.25,
    "span_y": 0.25,
    "grid_width": 2,
    "grid_height": 2
  },
  "sequence": {
    "zip_paths": true,
    "vary": [
      {
        "path": "fractal.params.explaino_seed",
        "values": [3.0, 4.0]
      },
      {
        "path": "fractal.view.explaino_seed_drift",
        "values": [0.25, 0.5]
      }
    ]
  },
  "metrics": ["iterations", "status", "final_z", "final_abs2", "summary_mean_iterations"],
  "operator_context": {
    "source": "salticid",
    "operator": "parameter_probe_scout",
    "why": "check outside contract matching"
  }
})";

        FractalProbeRequest request{};
        std::string error;
        if (!ParseFractalProbeRequestJson(requestJson, &request, &error)) {
            std::cerr << "Expected sequence_grid request to parse: " << error << "\n";
            return 1;
        }
        if (request.request_version != 1 || request.request_id != "probe-sequence-grid") {
            std::cerr << "Parsed request header mismatch\n";
            return 1;
        }
        if (request.mode != FractalProbeMode::sequence_grid) {
            std::cerr << "Expected mode sequence_grid\n";
            return 1;
        }
        if (request.base_state_load_path.find("state.json") == std::string::npos) {
            std::cerr << "Expected base_state.load_state_json to parse\n";
            return 1;
        }
        if (!request.has_region || request.region.grid_width != 2 || request.region.grid_height != 2) {
            std::cerr << "Expected 2x2 region\n";
            return 1;
        }
        if (!request.has_sequence || !request.sequence.zip_paths || request.sequence.axes.size() != 2) {
            std::cerr << "Expected zipped two-axis sequence\n";
            return 1;
        }
        if (request.overrides.size() != 3 || request.metrics.size() != 5) {
            std::cerr << "Expected overrides and metrics to parse\n";
            return 1;
        }
        if (request.operator_context.source != "salticid" ||
            request.operator_context.operator_name != "parameter_probe_scout") {
            std::cerr << "Expected operator context to parse\n";
            return 1;
        }
    }

        {
                const std::string requestJson = R"({
    "request_version": 1,
    "request_id": "probe-variant-crossfade-parse",
    "mode": "sequence_grid",
    "region": {
        "center_x": 0.0,
        "center_y": 0.0,
        "span_x": 0.2,
        "span_y": 0.2,
        "grid_width": 2,
        "grid_height": 2
    },
    "sequence": {
        "mode": "variant_crossfade",
        "from_variant": "explaino_ripple",
        "to_variant": "explaino_splice",
        "steps": 5
    }
})";

                FractalProbeRequest request{};
                std::string error;
                if (!ParseFractalProbeRequestJson(requestJson, &request, &error)) {
                        std::cerr << "Expected variant_crossfade request to parse: " << error << "\n";
                        return 1;
                }
                if (!request.has_sequence || request.sequence.mode != FractalProbeSequenceMode::variant_crossfade) {
                        std::cerr << "Expected sequence.mode=variant_crossfade\n";
                        return 1;
                }
                if (request.sequence.variant_crossfade.from_variant_id != "explaino_ripple" ||
                        request.sequence.variant_crossfade.to_variant_id != "explaino_splice" ||
                        request.sequence.variant_crossfade.steps != 5) {
                        std::cerr << "Expected variant_crossfade fields to parse\n";
                        return 1;
                }
        }

        {
                const std::string badJson = R"({
    "request_version": 1,
    "request_id": "probe-variant-crossfade-bad-steps",
    "mode": "sequence_grid",
    "region": {
        "center_x": 0.0,
        "center_y": 0.0,
        "span_x": 0.2,
        "span_y": 0.2,
        "grid_width": 2,
        "grid_height": 2
    },
    "sequence": {
        "mode": "variant_crossfade",
        "from_variant": "explaino_ripple",
        "to_variant": "explaino_splice",
        "steps": 4
    }
})";

                FractalProbeRequest request{};
                std::string error;
                if (ParseFractalProbeRequestJson(badJson, &request, &error)) {
                        std::cerr << "variant_crossfade should reject even step counts\n";
                        return 1;
                }
                if (error.find("odd integer >= 3") == std::string::npos) {
                        std::cerr << "Expected variant_crossfade step error to mention odd integer >= 3\n";
                        return 1;
                }
        }

    {
        const std::string badJson = R"({
  "request_version": 1,
  "request_id": "bad-top-level",
  "mode": "point_set",
  "points": [{ "x": 0.0, "y": 0.0 }],
  "mystery_field": 123
})";
        FractalProbeRequest request{};
        std::string error;
        if (ParseFractalProbeRequestJson(badJson, &request, &error)) {
            std::cerr << "Unknown top-level request keys should fail fast\n";
            return 1;
        }
        if (error.find("mystery_field") == std::string::npos) {
            std::cerr << "Expected unknown-key error to mention mystery_field\n";
            return 1;
        }
    }

    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "point-set-lambda";
        request.function_id = "fractal.sample";
        request.mode = FractalProbeMode::point_set;
        request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("explaino_lambda")});
        request.overrides.push_back({"fractal.params.lambda_real", FractalProbeScalar::Number(2.9685855)});
        request.overrides.push_back({"fractal.params.lambda_imag", FractalProbeScalar::Number(-0.27446103)});
        request.overrides.push_back({"fractal.params.explaino_warp_strength", FractalProbeScalar::Number(0.2)});
        request.overrides.push_back({"fractal.params.explaino_seed", FractalProbeScalar::Number(3.0)});
        request.overrides.push_back({"fractal.view.explaino_seed_drift", FractalProbeScalar::Number(0.25)});
        request.points.push_back({0.48, -0.04});
        request.points.push_back({0.52, 0.04});

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe", &response, &error)) {
            std::cerr << "Expected explaino_lambda point_set probe to run: " << error << "\n";
            return 1;
        }
        if (!response.ok || response.runtime.fractal_type != "explaino_lambda") {
            std::cerr << "Expected successful explaino_lambda probe response\n";
            return 1;
        }
        if (response.summary.sample_count != 2 || response.samples.size() != 2) {
            std::cerr << "Expected two point samples in response\n";
            return 1;
        }
        if (response.cost.sample_count != 2 || response.cost.gpu_ms < 0.0) {
            std::cerr << "Expected cost metadata with non-negative gpu_ms and sample_count=2\n";
            return 1;
        }
        if (response.sequence_results.size() != 1 || response.sequence_results[0].sequence_index != 0) {
            std::cerr << "Point-set probe should emit one implicit sequence summary\n";
            return 1;
        }

        const std::string responseJson = SerializeFractalProbeResponseJson(response);
        json_min::ParseResult pr = json_min::Parse(responseJson);
        if (!pr.error.empty()) {
            std::cerr << "Serialized point-set response should be valid JSON: " << pr.error << "\n";
            return 1;
        }
        const json_min::Value* summary = pr.value.get("summary");
        const json_min::Value* cost = pr.value.get("cost");
        const json_min::Value* samples = pr.value.get("samples");
        if (!summary || !summary->is_object() || !cost || !cost->is_object() || !samples || !samples->is_array()) {
            std::cerr << "Serialized response missing summary/cost/samples\n";
            return 1;
        }
        const json_min::Value* sampleCount = summary->get("sample_count");
        if (!sampleCount || !sampleCount->is_number() || !NearlyEqual(sampleCount->as_number(), 2.0)) {
            std::cerr << "Serialized summary sample_count mismatch\n";
            return 1;
        }
        const json_min::Value* costSampleCount = cost->get("sample_count");
        const json_min::Value* gpuMs = cost->get("gpu_ms");
        if (!costSampleCount || !costSampleCount->is_number() || !NearlyEqual(costSampleCount->as_number(), 2.0)) {
            std::cerr << "Serialized cost sample_count mismatch\n";
            return 1;
        }
        if (!gpuMs || !gpuMs->is_number() || gpuMs->as_number() < 0.0) {
            std::cerr << "Serialized cost gpu_ms should be a non-negative number\n";
            return 1;
        }
    }

    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "sequence-grid-rational-escape";
        request.function_id = "fractal.sample";
        request.mode = FractalProbeMode::sequence_grid;
        request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("explaino_rational_escape")});
        request.overrides.push_back({"fractal.params.explaino_warp_strength", FractalProbeScalar::Number(0.35)});
        request.has_region = true;
        request.region = {0.0, 0.0, 0.2, 0.2, 2, 2};
        request.has_sequence = true;
        request.sequence.zip_paths = true;
        request.sequence.axes.push_back({"fractal.params.explaino_seed", {FractalProbeScalar::Number(4.0), FractalProbeScalar::Number(5.0)}});
        request.sequence.axes.push_back({"fractal.view.explaino_seed_drift", {FractalProbeScalar::Number(0.1), FractalProbeScalar::Number(0.2)}});

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe", &response, &error)) {
            std::cerr << "Expected explaino_rational_escape sequence_grid probe to run: " << error << "\n";
            return 1;
        }
        if (!response.ok || response.runtime.fractal_type != "explaino_rational_escape") {
            std::cerr << "Expected successful explaino_rational_escape probe response\n";
            return 1;
        }
        if (response.sequence_results.size() != 2) {
            std::cerr << "Expected two sequence summaries for zipped sequence_grid\n";
            return 1;
        }
        if (response.summary.sample_count != 8 || response.samples.size() != 8) {
            std::cerr << "Expected 2 sequences * 2x2 grid = 8 samples\n";
            return 1;
        }
        if (response.summary.best_sequence_index < 0 || response.summary.best_sequence_index > 1) {
            std::cerr << "Expected best_sequence_index to reference one of the sequence entries\n";
            return 1;
        }
    }

    {
        FractalProbeResponse response{};
        response.request_id = "termination-transport";
        response.function_id = "fractal.sample";
        response.ok = true;
        response.summary.sample_count = 1;
        response.metric_selection = BuildFractalProbeMetricSelection({});

        FractalProbeSample sample{};
        sample.sequence_index = 0;
        sample.grid_x = 0;
        sample.grid_y = 0;
        sample.coord_x = 0.125;
        sample.coord_y = -0.25;
        sample.iterations = 12;
        sample.status = FractalProbeSampleStatus::bounded;
        sample.termination_kind = TerminationKind::none;
        sample.final_z_x = 1.0;
        sample.final_z_y = 2.0;
        sample.final_abs2 = 5.0;
        sample.has_far_field_delta = false;
        response.samples.push_back(sample);

        const std::string responseJson = SerializeFractalProbeResponseJson(response);
        json_min::ParseResult pr = json_min::Parse(responseJson);
        if (!pr.error.empty()) {
            std::cerr << "Termination transport response should serialize as valid JSON: " << pr.error << "\n";
            return 1;
        }
        const json_min::Value* samples = pr.value.get("samples");
        if (!samples || !samples->is_array() || samples->as_array().empty() || !samples->as_array()[0].is_object()) {
            std::cerr << "Termination transport response missing sample payload\n";
            return 1;
        }
        const json_min::Value* terminationKind = samples->as_array()[0].get("termination_kind");
        const json_min::Value* farFieldDelta = samples->as_array()[0].get("far_field_delta");
        if (!terminationKind || !terminationKind->is_string() || terminationKind->as_string() != "none") {
            std::cerr << "Termination transport response should include termination_kind=none\n";
            return 1;
        }
        if (!farFieldDelta || !farFieldDelta->is_null()) {
            std::cerr << "Termination transport response should serialize far_field_delta as null when absent\n";
            return 1;
        }
    }

    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "sequence-grid-variant-crossfade";
        request.function_id = "fractal.sample";
        request.mode = FractalProbeMode::sequence_grid;
        request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("explaino")});
        request.overrides.push_back({"fractal.params.explaino_seed", FractalProbeScalar::Number(3.0)});
        request.overrides.push_back({"fractal.params.explaino_warp_strength", FractalProbeScalar::Number(0.25)});
        request.overrides.push_back({"fractal.view.explaino_seed_drift", FractalProbeScalar::Number(0.1)});
        request.has_region = true;
        request.region = {0.0, 0.0, 0.2, 0.2, 2, 2};
        request.has_sequence = true;
        request.sequence.mode = FractalProbeSequenceMode::variant_crossfade;
        request.sequence.variant_crossfade.from_variant_id = "explaino_ripple";
        request.sequence.variant_crossfade.to_variant_id = "explaino_splice";
        request.sequence.variant_crossfade.steps = 5;

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe", &response, &error)) {
            std::cerr << "Expected variant_crossfade sequence_grid probe to run: " << error << "\n";
            return 1;
        }
        if (!response.ok || response.runtime.fractal_type != "explaino_all") {
            std::cerr << "Expected successful variant_crossfade probe response to publish the canonical explaino_all identity on the final projected selector step\n";
            return 1;
        }
        if (response.sequence_results.size() != 5) {
            std::cerr << "Expected five sequence summaries for variant_crossfade\n";
            return 1;
        }
        if (response.summary.sample_count != 20 || response.samples.size() != 20) {
            std::cerr << "Expected 5 sequences * 2x2 grid = 20 samples\n";
            return 1;
        }

        const auto& firstApplied = response.sequence_results[0].applied;
        const auto& secondApplied = response.sequence_results[1].applied;
        const auto& middleApplied = response.sequence_results[2].applied;
        const auto& fourthApplied = response.sequence_results[3].applied;
        const auto& lastApplied = response.sequence_results[4].applied;
        if (firstApplied.size() != 2 ||
            firstApplied[0].first != "fractal.view.fractal_type" ||
            firstApplied[0].second.string_value != "explaino_ripple" ||
            firstApplied[1].first != "fractal.params.ripple_amplitude" ||
            !NearlyEqual(firstApplied[1].second.number_value, 0.15)) {
            std::cerr << "Expected first crossfade step to use explaino_ripple default strength\n";
            return 1;
        }
        if (secondApplied.size() != 2 ||
            secondApplied[0].second.string_value != "explaino_ripple" ||
            !NearlyEqual(secondApplied[1].second.number_value, 0.075)) {
            std::cerr << "Expected second crossfade step to halve ripple strength\n";
            return 1;
        }
        if (middleApplied.size() != 1 ||
            middleApplied[0].first != "fractal.view.fractal_type" ||
            middleApplied[0].second.string_value != "explaino") {
            std::cerr << "Expected middle crossfade step to land on plain explaino\n";
            return 1;
        }
        if (fourthApplied.size() != 2 ||
            fourthApplied[0].second.string_value != "explaino_splice" ||
            fourthApplied[1].first != "fractal.params.splice_offset" ||
            !NearlyEqual(fourthApplied[1].second.number_value, 0.25)) {
            std::cerr << "Expected fourth crossfade step to ramp splice strength from the midpoint\n";
            return 1;
        }
        if (lastApplied.size() != 2 ||
            lastApplied[0].second.string_value != "explaino_splice" ||
            !NearlyEqual(lastApplied[1].second.number_value, 0.5)) {
            std::cerr << "Expected final crossfade step to use explaino_splice default strength\n";
            return 1;
        }
    }

    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "sequence-grid-variant-crossfade-balance-void-reject";
        request.function_id = "fractal.sample";
        request.mode = FractalProbeMode::sequence_grid;
        request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("explaino")});
        request.overrides.push_back({"fractal.params.explaino_seed", FractalProbeScalar::Number(3.0)});
        request.overrides.push_back({"fractal.params.explaino_warp_strength", FractalProbeScalar::Number(0.25)});
        request.overrides.push_back({"fractal.view.explaino_seed_drift", FractalProbeScalar::Number(0.1)});
        request.has_region = true;
        request.region = {0.0, 0.0, 0.2, 0.2, 2, 2};
        request.has_sequence = true;
        request.sequence.mode = FractalProbeSequenceMode::variant_crossfade;
        request.sequence.variant_crossfade.from_variant_id = "explaino_balance_void";
        request.sequence.variant_crossfade.to_variant_id = "explaino_splice";
        request.sequence.variant_crossfade.steps = 5;

        FractalProbeResponse response{};
        std::string error;
        if (RunFractalProbeRequest(request, "D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe", &response, &error)) {
            std::cerr << "variant_crossfade should reject explaino_balance_void because it is not a single-axis Explaino projection selector\n";
            return 1;
        }
        if (error.find("single-axis Explaino projection selectors") == std::string::npos) {
            std::cerr << "Expected explaino_balance_void crossfade rejection to mention single-axis Explaino projection selectors\n";
            return 1;
        }
    }

    {
        const struct ComposedReductionCase {
            const char* label;
            const char* primary_type;
            const char* secondary_type;
            double ripple_amplitude;
            double splice_offset;
            double vortex_strength;
            double tension_strength;
        } cases[] = {
            {"probe composed ripple->vortex", "explaino_ripple", "explaino_vortex", 0.0, 0.0, 0.3, 0.0},
            {"probe composed vortex->splice", "explaino_vortex", "explaino_splice", 0.0, 0.5, 0.0, 0.0},
            {"probe composed splice->tension", "explaino_splice", "explaino_tension", 0.0, 0.0, 0.0, 0.02},
            {"probe composed tension->ripple", "explaino_tension", "explaino_ripple", 0.15, 0.0, 0.0, 0.0},
        };

        for (const auto& composedCase : cases) {
            FractalProbeResponse expectedResponse{};
            FractalProbeResponse actualResponse{};
            std::string error;
            if (!RunExplainoPointProbe(
                    std::string(composedCase.label) + "-expected",
                    composedCase.secondary_type,
                    composedCase.ripple_amplitude,
                    composedCase.splice_offset,
                    composedCase.vortex_strength,
                    composedCase.tension_strength,
                    &expectedResponse,
                    &error)) {
                std::cerr << composedCase.label << " expected probe failed: " << error << "\n";
                return 1;
            }
            if (!RunExplainoPointProbe(
                    std::string(composedCase.label) + "-actual",
                    composedCase.primary_type,
                    composedCase.ripple_amplitude,
                    composedCase.splice_offset,
                    composedCase.vortex_strength,
                    composedCase.tension_strength,
                    &actualResponse,
                    &error)) {
                std::cerr << composedCase.label << " actual probe failed: " << error << "\n";
                return 1;
            }
            if (!CompareProbeResponses(expectedResponse, actualResponse, composedCase.label)) {
                return 1;
            }
        }
    }

    {
        FractalProbeResponse expectedResponse{};
        FractalProbeResponse actualResponse{};
        std::string error;
        if (!RunExplainoPointProbe("probe-plain-explaino-expected", "explaino", 0.0, 0.0, 0.0, 0.0, &expectedResponse, &error)) {
            std::cerr << "Expected plain explaino probe to run: " << error << "\n";
            return 1;
        }
        if (!RunExplainoPointProbe("probe-plain-explaino-actual", "explaino", 0.15, 0.5, 0.3, 0.02, &actualResponse, &error)) {
            std::cerr << "Expected plain explaino probe with latent params to run: " << error << "\n";
            return 1;
        }
        if (!CompareProbeResponses(expectedResponse, actualResponse, "probe plain explaino latent params")) {
            return 1;
        }
    }

    {
        FractalProbeResponse expectedResponse{};
        FractalProbeResponse actualResponse{};
        std::string error;
        if (!RunExplainoLegacyCouplingProbe("probe-inertial-neutral-expected", "explaino", 0.0, 0.0, 0.0, 0.0, &expectedResponse, &error)) {
            std::cerr << "Expected neutral explaino probe to run for inertial collapse proof: " << error << "\n";
            return 1;
        }
        if (!RunExplainoLegacyCouplingProbe("probe-inertial-neutral-actual", "explaino_inertial", 0.0, 0.0, 0.0, 0.0, &actualResponse, &error)) {
            std::cerr << "Expected neutral explaino_inertial probe to run: " << error << "\n";
            return 1;
        }
        if (!CompareProbeResponses(expectedResponse, actualResponse, "probe explaino_inertial neutral collapse")) {
            return 1;
        }
    }

    {
        const struct ComposedLabelCase {
            const char* label;
            const char* left_type;
            const char* right_type;
            double ripple_amplitude;
            double splice_offset;
            double vortex_strength;
            double tension_strength;
        } cases[] = {
            {"probe composed label invariance ripple-vortex", "explaino_ripple", "explaino_vortex", 0.15, 0.0, 0.3, 0.0},
            {"probe composed label invariance splice-tension", "explaino_splice", "explaino_tension", 0.0, 0.5, 0.0, 0.02},
        };

        for (const auto& composedCase : cases) {
            FractalProbeResponse leftResponse{};
            FractalProbeResponse rightResponse{};
            std::string error;
            if (!RunExplainoPointProbe(
                    std::string(composedCase.label) + "-left",
                    composedCase.left_type,
                    composedCase.ripple_amplitude,
                    composedCase.splice_offset,
                    composedCase.vortex_strength,
                    composedCase.tension_strength,
                    &leftResponse,
                    &error)) {
                std::cerr << composedCase.label << " left probe failed: " << error << "\n";
                return 1;
            }
            if (!RunExplainoPointProbe(
                    std::string(composedCase.label) + "-right",
                    composedCase.right_type,
                    composedCase.ripple_amplitude,
                    composedCase.splice_offset,
                    composedCase.vortex_strength,
                    composedCase.tension_strength,
                    &rightResponse,
                    &error)) {
                std::cerr << composedCase.label << " right probe failed: " << error << "\n";
                return 1;
            }
            if (!CompareProbeResponses(leftResponse, rightResponse, composedCase.label)) {
                return 1;
            }
        }
    }

    {
        const std::filesystem::path statePath = std::filesystem::temp_directory_path() / "fractal_probe_base_state.json";
        const std::string stateJson = R"({
  "state_version": 3,
  "fractal_type": "explaino_lambda",
  "view": {
    "center_x": 0.5,
    "center_y": 0.0,
    "zoom": 4.5,
    "rotation_degrees": 0,
    "center_hp_x": 0.5,
    "center_hp_y": 0.0,
    "log2_zoom": 2.169925001,
    "explaino_phase": 0,
    "explaino_seed_drift": 0.25,
    "explaino_seed_tween": true,
    "auto_increment_seed": false,
    "explaino_seed_rate": 0.05
  },
  "params": {
    "max_iter": 1200,
    "epsilon": 0.000001,
    "exposure": 1.4,
    "poly_kind": 2,
    "coloring_mode": "smooth_escape",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 2.9685855,
    "lambda_imag": -0.27446103,
    "explaino_seed": 3.0,
    "explaino_seed_b": 1.0,
    "explaino_mix": 0.5,
    "explaino_warp_strength": 0.2,
    "explaino_root_count": 4,
    "explaino_cluster_radius": 0.0,
    "transcendental_func": "f_sin",
    "momentum_beta": 0.0,
    "mcmullen_preset": "z3_z3",
    "poly_coeffs": [-1, 0, 0, 1, 0],
    "color_saturation": 1.15,
    "color_contrast": 1.1,
    "color_tint_r": 1.0,
    "color_tint_g": 1.0,
    "color_tint_b": 1.0
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 0,
    "last_device_id": 0
  }
})";
        if (!WriteTextFile(statePath, stateJson)) {
            std::cerr << "Failed to write temporary base-state JSON\n";
            return 1;
        }

        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "load-base-state";
        request.function_id = "fractal.sample";
        request.mode = FractalProbeMode::point_set;
        request.base_state_load_path = statePath.string();
        request.points.push_back({0.5, 0.0});

        FractalProbeResponse response{};
        std::string error;
        const bool ok = RunFractalProbeRequest(request, "D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe", &response, &error);
        std::filesystem::remove(statePath);

        if (!ok) {
            std::cerr << "Expected base-state loading probe to run: " << error << "\n";
            return 1;
        }
        if (!response.ok || response.runtime.fractal_type != "explaino_lambda") {
            std::cerr << "Expected base-state probe to preserve explaino_lambda fractal type\n";
            return 1;
        }
    }

    {
        const struct ProbeCase {
            const char* fractal_type;
            std::vector<FractalProbeOverride> overrides;
        } cases[] = {
            {
                "halley",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("halley")},
                },
            },
            {
                "burning_ship",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("burning_ship")},
                },
            },
            {
                "multibrot",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("multibrot")},
                    {"fractal.params.multibrot_power_float", FractalProbeScalar::Number(3.0)},
                },
            },
            {
                "phoenix",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("phoenix")},
                },
            },
            {
                "multicorn",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("multicorn")},
                    {"fractal.params.multibrot_power", FractalProbeScalar::Number(3.0)},
                },
            },
            {
                "collatz",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("collatz")},
                },
            },
            {
                "mcmullen",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("mcmullen")},
                },
            },
            {
                "explaino_y",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("explaino_y")},
                },
            },
            {
                "explaino_fp",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("explaino_fp")},
                },
            },
            {
                "explaino_nova",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("explaino_nova")},
                },
            },
            {
                "explaino_halley",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("explaino_halley")},
                },
            },
            {
                "explaino_phoenix",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("explaino_phoenix")},
                },
            },
            {
                "explaino_transcendental",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("explaino_transcendental")},
                },
            },
            {
                "explaino_inertial",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("explaino_inertial")},
                },
            },
            {
                "explaino_collatz",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("explaino_collatz")},
                },
            },
            {
                "explaino_joy",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("explaino_joy")},
                },
            },
            {
                "explaino_fold",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("explaino_fold")},
                },
            },
            {
                "explaino_bell",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("explaino_bell")},
                },
            },
            {
                "explaino_ripple",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("explaino_ripple")},
                },
            },
            {
                "explaino_splice",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("explaino_splice")},
                },
            },
            {
                "explaino_vortex",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("explaino_vortex")},
                },
            },
            {
                "explaino_tension",
                {
                    {"fractal.view.fractal_type", FractalProbeScalar::String("explaino_tension")},
                },
            },
        };

        for (const ProbeCase& probeCase : cases) {
            FractalProbeRequest request{};
            request.request_version = 1;
            request.request_id = std::string("probe-") + probeCase.fractal_type;
            request.function_id = "fractal.sample";
            request.mode = FractalProbeMode::point_set;
            request.overrides = probeCase.overrides;
            request.points.push_back({0.125, 0.0});
            request.points.push_back({0.25, 0.0});

            FractalProbeResponse response{};
            std::string error;
            if (!RunFractalProbeRequest(request, "D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe", &response, &error)) {
                std::cerr << "Expected point_set probe to run for " << probeCase.fractal_type << ": " << error << "\n";
                return 1;
            }
            FractalType requestedFractalType = FractalType::newton;
            if (!TryParseFractalTypeId(probeCase.fractal_type, &requestedFractalType)) {
                std::cerr << "Expected probe case fractal_type to parse through the checked-in enum id registry: " << probeCase.fractal_type << "\n";
                return 1;
            }
            const char* expectedRuntimeFractalType = FractalTypeId(ResolveExplainoPublicFractalType(requestedFractalType));
            if (!response.ok || response.runtime.fractal_type != expectedRuntimeFractalType) {
                std::cerr << "Expected successful probe response for " << probeCase.fractal_type << " to publish runtime.fractal_type=" << expectedRuntimeFractalType << "\n";
                return 1;
            }
            if (response.summary.sample_count != 2 || response.samples.size() != 2) {
                std::cerr << "Expected two samples for " << probeCase.fractal_type << "\n";
                return 1;
            }
        }
    }

    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "probe-nova-zero-derivative";
        request.function_id = "fractal.sample";
        request.mode = FractalProbeMode::point_set;
        request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("nova")});
        request.points.push_back({0.125, 0.0});
        request.points.push_back({0.25, 0.0});

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe", &response, &error)) {
            std::cerr << "Expected Nova point_set probe to run: " << error << "\n";
            return 1;
        }
        if (!response.ok || response.runtime.fractal_type != "nova") {
            std::cerr << "Expected successful probe response for nova\n";
            return 1;
        }
        if (response.summary.sample_count != 2 || response.samples.size() != 2) {
            std::cerr << "Expected two samples for nova\n";
            return 1;
        }
        for (const FractalProbeSample& sample : response.samples) {
            if (sample.iterations <= 0) {
                std::cerr << "Nova probe should advance beyond iteration 0 when z starts at a zero-derivative point\n";
                return 1;
            }
        }
    }

    {
        const struct ExpectedRoot {
            double x;
            double y;
            int rootIndex;
        } expected[] = {
            {1.0, 0.0, 2},
            {-0.5, 0.8660254037844386, 0},
            {-0.5, -0.8660254037844386, 1},
        };

        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "probe-newton-known-roots";
        request.function_id = "fractal.sample";
        request.mode = FractalProbeMode::point_set;
        request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("newton")});
        for (const auto& item : expected) {
            request.points.push_back({item.x, item.y});
        }

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "probe.exe", &response, &error)) {
            std::cerr << "Expected Newton known-roots probe to run: " << error << "\n";
            return 1;
        }
        if (!response.ok || response.runtime.fractal_type != "newton") {
            std::cerr << "Expected successful probe response for newton known roots\n";
            return 1;
        }
        if (response.summary.sample_count != 3 || response.samples.size() != 3) {
            std::cerr << "Expected three Newton known-root samples\n";
            return 1;
        }

        for (size_t index = 0; index < response.samples.size(); ++index) {
            const FractalProbeSample& sample = response.samples[index];
            if (sample.status != FractalProbeSampleStatus::converged) {
                std::cerr << "Newton known-root sample should converge at index " << index << "\n";
                return 1;
            }
            if (!sample.has_root_index || sample.root_index != expected[index].rootIndex) {
                std::cerr << "Newton known-root sample should preserve root_index at index " << index << "\n";
                return 1;
            }
            if (!NearlyEqual(sample.final_z_x, expected[index].x) || !NearlyEqual(sample.final_z_y, expected[index].y)) {
                std::cerr << "Newton known-root sample should preserve final_z at index " << index << "\n";
                return 1;
            }
        }
    }

    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "probe-explaino-y-nonfinite-status";
        request.function_id = "fractal.sample";
        request.mode = FractalProbeMode::point_set;
        request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("explaino_y")});
        request.points.push_back({1.0e300, 0.0});

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "probe.exe", &response, &error)) {
            std::cerr << "Expected Explaino-Y nonfinite probe to run: " << error << "\n";
            return 1;
        }
        if (!response.ok || response.samples.size() != 1) {
            std::cerr << "Expected a single Explaino-Y nonfinite sample\n";
            return 1;
        }
        if (response.samples[0].status != FractalProbeSampleStatus::nonfinite) {
            std::cerr << "Explaino-Y nonfinite samples must not be rewritten to converged\n";
            return 1;
        }
        if (response.samples[0].has_root_index) {
            std::cerr << "Explaino-Y nonfinite samples must not report a root index\n";
            return 1;
        }
    }

    std::cout << "test_fractal_probe: all passed\n";
    return 0;
}