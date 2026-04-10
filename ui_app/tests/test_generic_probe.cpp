// test_generic_probe.cpp — Integration tests for generic.sample probe path.
// Validates that RunFractalProbeRequest dispatches generic.sample correctly,
// that the expression parser + CPU evaluator produce valid samples, and that
// error cases fail fast with descriptive messages.

#include "../src/fractal_probe_contract.h"
#include "../src/fractal_probe_runner.h"
#include "../src/json_min.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {

bool NearlyEqual(double left, double right, double eps = 1.0e-6) {
    const double delta = left - right;
    return delta < eps && delta > -eps;
}

} // namespace

int main() {
    int passed = 0;

    // 1. Point-set: z^3 - 1 iterate (Newton roots) ---------------------------
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "gf-newton-z3m1";
        request.function_id = "generic.sample";
        request.mode = FractalProbeMode::point_set;
        request.has_function = true;
        request.generic_expression = "iterate(z - (z^3 - 1) / (3 * z^2), 200)";
        request.generic_epsilon = 1.0e-10;
        request.generic_escape_radius = 1000.0;
        request.metrics = {"iterations", "status", "value", "abs2", "derivative"};
        // Seed near the real root z=1: should converge there.
        request.points.push_back({1.1, 0.05});
        // Seed near the complex root at (-0.5, sqrt(3)/2).
        request.points.push_back({-0.4, 0.9});

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "unused", &response, &error)) {
            std::cerr << "[1] generic.sample z^3-1 failed: " << error << "\n";
            return 1;
        }
        if (!response.ok || response.function_id != "generic.sample") {
            std::cerr << "[1] response.ok=" << response.ok << " function_id=" << response.function_id << "\n";
            return 1;
        }
        if (response.samples.size() != 2) {
            std::cerr << "[1] expected 2 samples, got " << response.samples.size() << "\n";
            return 1;
        }
        // First sample should converge near (1, 0).
        const auto& s0 = response.samples[0];
        if (s0.status != FractalProbeSampleStatus::converged) {
            std::cerr << "[1] sample 0 should converge, status=" << static_cast<int>(s0.status) << "\n";
            return 1;
        }
        if (!NearlyEqual(s0.final_z_x, 1.0, 1e-4) || !NearlyEqual(s0.final_z_y, 0.0, 1e-4)) {
            std::cerr << "[1] sample 0 root: (" << s0.final_z_x << ", " << s0.final_z_y << ") expected ~(1,0)\n";
            return 1;
        }
        // Second sample should converge near (-0.5, 0.866).
        const auto& s1 = response.samples[1];
        if (s1.status != FractalProbeSampleStatus::converged) {
            std::cerr << "[1] sample 1 should converge, status=" << static_cast<int>(s1.status) << "\n";
            return 1;
        }
        if (!NearlyEqual(s1.final_z_x, -0.5, 1e-4) || !NearlyEqual(s1.final_z_y, 0.866025, 1e-3)) {
            std::cerr << "[1] sample 1 root: (" << s1.final_z_x << ", " << s1.final_z_y << ") expected ~(-0.5, 0.866)\n";
            return 1;
        }
        std::cout << "[1] z^3-1 point-set: passed (" << s0.iterations << " / " << s1.iterations << " iters)\n";
        passed++;
    }

    // 2. Grid mode: z^2 iterate (Mandelbrot-like with param c) ----------------
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "gf-grid-mandel";
        request.function_id = "generic.sample";
        request.mode = FractalProbeMode::grid;
        request.has_function = true;
        request.generic_expression = "iterate(z^2 + c, 50)";
        request.generic_params = {{"c_real", -0.75}, {"c_imag", 0.0}};
        request.generic_epsilon = 1e-8;
        request.generic_escape_radius = 4.0;
        request.metrics = {"iterations", "status", "summary_mean_iterations"};
        request.has_region = true;
        request.region.center_x = 0.0;
        request.region.center_y = 0.0;
        request.region.span_x = 2.0;
        request.region.span_y = 2.0;
        request.region.grid_width = 4;
        request.region.grid_height = 4;

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "unused", &response, &error)) {
            std::cerr << "[2] grid z^2+c failed: " << error << "\n";
            return 1;
        }
        if (response.samples.size() != 16) {
            std::cerr << "[2] expected 16 samples, got " << response.samples.size() << "\n";
            return 1;
        }
        if (response.summary.sample_count != 16) {
            std::cerr << "[2] summary.sample_count=" << response.summary.sample_count << "\n";
            return 1;
        }
        // The center point (0,0) with c=(-0.75,0) should converge (period-2 cycle in Mandelbrot).
        // Some edge points should escape.
        int escaped = 0, converged = 0;
        for (const auto& s : response.samples) {
            if (s.status == FractalProbeSampleStatus::escaped) escaped++;
            if (s.status == FractalProbeSampleStatus::converged) converged++;
        }
        if (escaped == 0) {
            std::cerr << "[2] expected some escaped samples\n";
            return 1;
        }
        std::cout << "[2] grid z^2+c: passed (escaped=" << escaped << " converged=" << converged << ")\n";
        passed++;
    }

    // 3. Direct evaluation (non-iterate expression) ---------------------------
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "gf-direct-eval";
        request.function_id = "generic.sample";
        request.mode = FractalProbeMode::point_set;
        request.has_function = true;
        request.generic_expression = "z^2 + z + 1";
        request.metrics = {"value", "abs2"};
        // Evaluate at z=(1,0): should give 1+1+1 = (3, 0).
        request.points.push_back({1.0, 0.0});
        // Evaluate at z=(0,1) = i: i^2+i+1 = -1+i+1 = (0, 1).
        request.points.push_back({0.0, 1.0});

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "unused", &response, &error)) {
            std::cerr << "[3] direct eval failed: " << error << "\n";
            return 1;
        }
        if (response.samples.size() != 2) {
            std::cerr << "[3] expected 2 samples\n";
            return 1;
        }
        // z=1: f(1) = 1+1+1 = 3
        if (!NearlyEqual(response.samples[0].final_z_x, 3.0, 1e-10) ||
            !NearlyEqual(response.samples[0].final_z_y, 0.0, 1e-10)) {
            std::cerr << "[3] f(1)=" << response.samples[0].final_z_x << "+" << response.samples[0].final_z_y << "i, expected 3\n";
            return 1;
        }
        // z=i: f(i) = i^2+i+1 = -1+i+1 = 0+i
        if (!NearlyEqual(response.samples[1].final_z_x, 0.0, 1e-10) ||
            !NearlyEqual(response.samples[1].final_z_y, 1.0, 1e-10)) {
            std::cerr << "[3] f(i)=" << response.samples[1].final_z_x << "+" << response.samples[1].final_z_y << "i, expected i\n";
            return 1;
        }
        std::cout << "[3] direct eval: passed\n";
        passed++;
    }

    // 4. Malformed expression → fail fast with parser error -------------------
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "gf-bad-expr";
        request.function_id = "generic.sample";
        request.mode = FractalProbeMode::point_set;
        request.has_function = true;
        request.generic_expression = "z^2 +* 1";
        request.points.push_back({0.0, 0.0});

        FractalProbeResponse response{};
        std::string error;
        if (RunFractalProbeRequest(request, "unused", &response, &error)) {
            std::cerr << "[4] malformed expression should fail\n";
            return 1;
        }
        if (error.find("parse error") == std::string::npos &&
            error.find("Expression parse error") == std::string::npos) {
            std::cerr << "[4] error should mention parser: " << error << "\n";
            return 1;
        }
        std::cout << "[4] malformed expression: passed (error: " << error << ")\n";
        passed++;
    }

    // 5. Missing function block → fail fast -----------------------------------
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "gf-no-function";
        request.function_id = "generic.sample";
        request.mode = FractalProbeMode::point_set;
        // has_function = false, generic_expression = empty
        request.points.push_back({0.0, 0.0});

        FractalProbeResponse response{};
        std::string error;
        if (RunFractalProbeRequest(request, "unused", &response, &error)) {
            std::cerr << "[5] missing function block should fail\n";
            return 1;
        }
        if (error.find("function") == std::string::npos) {
            std::cerr << "[5] error should mention function: " << error << "\n";
            return 1;
        }
        std::cout << "[5] missing function block: passed\n";
        passed++;
    }

    // 6. Unknown function_id → fail fast with valid list ----------------------
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "gf-bad-id";
        request.function_id = "bogus.thing";
        request.mode = FractalProbeMode::point_set;
        request.points.push_back({0.0, 0.0});

        FractalProbeResponse response{};
        std::string error;
        if (RunFractalProbeRequest(request, "unused", &response, &error)) {
            std::cerr << "[6] unknown function_id should fail\n";
            return 1;
        }
        if (error.find("fractal.sample") == std::string::npos || error.find("generic.sample") == std::string::npos) {
            std::cerr << "[6] error should list valid ids: " << error << "\n";
            return 1;
        }
        std::cout << "[6] unknown function_id: passed\n";
        passed++;
    }

    // 7. JSON round-trip: serialize and reparse response -----------------------
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "gf-json-round-trip";
        request.function_id = "generic.sample";
        request.mode = FractalProbeMode::point_set;
        request.has_function = true;
        request.generic_expression = "iterate(z^2 - 1, 100)";
        request.generic_epsilon = 1e-8;
        request.metrics = {"iterations", "status", "value", "abs2", "derivative"};
        request.points.push_back({0.5, 0.3});

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "unused", &response, &error)) {
            std::cerr << "[7] round-trip request failed: " << error << "\n";
            return 1;
        }
        std::string json = SerializeFractalProbeResponseJson(response);
        auto parsed = json_min::Parse(json);
        if (!parsed.error.empty()) {
            std::cerr << "[7] response JSON invalid: " << parsed.error << "\n";
            return 1;
        }
        const auto* fid = parsed.value.get("function_id");
        if (!fid || fid->as_string() != "generic.sample") {
            std::cerr << "[7] function_id missing or wrong in JSON\n";
            return 1;
        }
        std::cout << "[7] JSON round-trip: passed\n";
        passed++;
    }

    // 8. Backward compat: fractal.sample still routes (empty function_id) -----
    {
        // This just validates that RunFractalProbeRequest still accepts the
        // fractal.sample path. We don't need a GPU so we expect it to fail
        // at the rendering stage, not at dispatch. The key check is that it
        // does NOT produce "Unknown function_id".
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "gf-compat-fractal";
        request.function_id = ""; // should default to fractal.sample
        request.mode = FractalProbeMode::point_set;
        request.points.push_back({0.0, 0.0});

        FractalProbeResponse response{};
        std::string error;
        // This will fail because we don't have the runtime, but the error
        // should NOT be "Unknown function_id".
        RunFractalProbeRequest(request, "nonexistent_exe", &response, &error);
        if (error.find("Unknown function_id") != std::string::npos) {
            std::cerr << "[8] empty function_id should default to fractal.sample\n";
            return 1;
        }
        std::cout << "[8] backward compat: passed\n";
        passed++;
    }

    // 9. Sequence variation with param overrides ------------------------------
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "gf-sequence-params";
        request.function_id = "generic.sample";
        request.mode = FractalProbeMode::sequence_point_set;
        request.has_function = true;
        request.generic_expression = "iterate(z^2 + c, 50)";
        request.generic_params = {{"c_real", 0.0}, {"c_imag", 0.0}};
        request.generic_epsilon = 1e-8;
        request.generic_escape_radius = 4.0;
        request.metrics = {"iterations", "status", "summary_mean_iterations"};
        request.points.push_back({0.5, 0.5});

        request.has_sequence = true;
        FractalProbeSequenceAxis axis;
        axis.path = "function.params.c_real";
        axis.values.push_back(FractalProbeScalar::Number(0.0));
        axis.values.push_back(FractalProbeScalar::Number(-2.0));
        request.sequence.axes.push_back(axis);
        request.sequence.zip_paths = false;

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "unused", &response, &error)) {
            std::cerr << "[9] sequence params failed: " << error << "\n";
            return 1;
        }
        if (response.sequence_results.size() != 2) {
            std::cerr << "[9] expected 2 sequence results, got " << response.sequence_results.size() << "\n";
            return 1;
        }
        std::cout << "[9] sequence param override: passed\n";
        passed++;
    }

    std::cout << "test_generic_probe: " << passed << "/9 passed\n";
    return (passed == 9) ? 0 : 1;
}
