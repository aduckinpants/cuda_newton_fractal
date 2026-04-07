#include "../src/fractal_probe_contract.h"
#include "../src/fractal_probe_runner.h"

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
        const json_min::Value* samples = pr.value.get("samples");
        if (!summary || !summary->is_object() || !samples || !samples->is_array()) {
            std::cerr << "Serialized response missing summary/samples\n";
            return 1;
        }
        const json_min::Value* sampleCount = summary->get("sample_count");
        if (!sampleCount || !sampleCount->is_number() || !NearlyEqual(sampleCount->as_number(), 2.0)) {
            std::cerr << "Serialized summary sample_count mismatch\n";
            return 1;
        }
    }

    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "sequence-grid-rational-escape";
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
        };

        for (const ProbeCase& probeCase : cases) {
            FractalProbeRequest request{};
            request.request_version = 1;
            request.request_id = std::string("probe-") + probeCase.fractal_type;
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
            if (!response.ok || response.runtime.fractal_type != probeCase.fractal_type) {
                std::cerr << "Expected successful probe response for " << probeCase.fractal_type << "\n";
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

    std::cout << "test_fractal_probe: all passed\n";
    return 0;
}