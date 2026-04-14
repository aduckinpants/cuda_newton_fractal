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
        if (response.cost.sample_count != 2 || response.cost.gpu_ms < 0.0) {
            std::cerr << "[1] expected cost metadata with non-negative gpu_ms and sample_count=2\n";
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

    // 7b. Nonfinite sample payloads stay parseable without poisoning summary metrics.
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "gf-json-nonfinite-sanitize";
        request.function_id = "generic.sample";
        request.mode = FractalProbeMode::point_set;
        request.has_function = true;
        request.generic_expression = "iterate(z - 1 + exp(-z), 60)";
        request.generic_epsilon = 1e-9;
        request.generic_escape_radius = 1000.0;
        request.metrics = {"iterations", "status", "value", "abs2", "derivative", "summary_mean_abs2"};
        request.points.push_back({-6.5, -9.5});

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "unused", &response, &error)) {
            std::cerr << "[7b] nonfinite sanitize request failed: " << error << "\n";
            return 1;
        }
        std::string json = SerializeFractalProbeResponseJson(response);
        auto parsed = json_min::Parse(json);
        if (!parsed.error.empty()) {
            std::cerr << "[7b] response JSON invalid: " << parsed.error << "\n";
            return 1;
        }
        const auto* samples = parsed.value.get("samples");
        if (!samples || !samples->is_array() || samples->as_array().size() != 1) {
            std::cerr << "[7b] expected one serialized sample\n";
            return 1;
        }
        const json_min::Value& sample = samples->as_array()[0];
        const auto* abs2 = sample.get("abs2");
        const auto* derivativeX = sample.get("derivative_x");
        const auto* derivativeY = sample.get("derivative_y");
        if (!abs2 || !abs2->is_null()) {
            std::cerr << "[7b] abs2 should serialize as null when nonfinite\n";
            return 1;
        }
        if (!derivativeX || !derivativeX->is_null() || !derivativeY || !derivativeY->is_null()) {
            std::cerr << "[7b] derivative components should serialize as null when nonfinite\n";
            return 1;
        }
        const auto* summary = parsed.value.get("summary");
        if (!summary || !summary->is_object()) {
            std::cerr << "[7b] expected serialized summary object\n";
            return 1;
        }
        const auto* meanAbs2 = summary->get("mean_abs2");
        if (!meanAbs2 || !meanAbs2->is_number() || !NearlyEqual(meanAbs2->as_number(), 0.0, 1e-12)) {
            std::cerr << "[7b] summary.mean_abs2 should stay finite and collapse to 0 when every sampled abs2 is nonfinite\n";
            return 1;
        }
        std::cout << "[7b] nonfinite JSON sanitize: passed\n";
        passed++;
    }

    // 8. Empty function_id must be rejected (no-implicit-fallback) ----------
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "gf-empty-fid-reject";
        request.function_id = ""; // must be rejected, not silently defaulted
        request.mode = FractalProbeMode::point_set;
        request.points.push_back({0.0, 0.0});

        FractalProbeResponse response{};
        std::string error;
        const bool ok = RunFractalProbeRequest(request, "nonexistent_exe", &response, &error);
        if (ok) {
            std::cerr << "[8] empty function_id should be rejected, not silently routed\n";
            return 1;
        }
        if (error.find("function_id") == std::string::npos) {
            std::cerr << "[8] error should mention function_id: " << error << "\n";
            return 1;
        }
        std::cout << "[8] empty function_id rejection: passed\n";
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

    // 10. summary_mean_abs2 and summary_diverged_fraction are computed --------
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "gf-summary-abs2";
        request.function_id = "generic.sample";
        request.mode = FractalProbeMode::point_set;
        request.has_function = true;
        request.generic_expression = "iterate(z^2, 20)";
        request.generic_epsilon = 1e-8;
        request.generic_escape_radius = 4.0;
        request.metrics = {"status", "summary_mean_abs2", "summary_diverged_fraction"};
        // z=0.5 should converge (0.5^2=0.25, 0.25^2=0.0625, ...)
        request.points.push_back({0.5, 0.0});
        // z=3 should escape immediately (9 > 4)
        request.points.push_back({3.0, 0.0});

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "unused", &response, &error)) {
            std::cerr << "[10] summary metrics failed: " << error << "\n";
            return 1;
        }
        // summary.mean_abs2 should be non-zero (we have one converged, one escaped).
        if (response.summary.mean_abs2 <= 0.0) {
            std::cerr << "[10] mean_abs2 should be positive, got " << response.summary.mean_abs2 << "\n";
            return 1;
        }
        // diverged_fraction: 1 out of 2 escaped.
        if (!NearlyEqual(response.summary.diverged_fraction, 0.5, 0.01)) {
            std::cerr << "[10] diverged_fraction=" << response.summary.diverged_fraction << " expected ~0.5\n";
            return 1;
        }

        // Verify JSON serialization includes these fields.
        std::string json = SerializeFractalProbeResponseJson(response);
        if (json.find("mean_abs2") == std::string::npos) {
            std::cerr << "[10] JSON missing mean_abs2\n";
            return 1;
        }
        if (json.find("diverged_fraction") == std::string::npos) {
            std::cerr << "[10] JSON missing diverged_fraction\n";
            return 1;
        }
        std::cout << "[10] summary_mean_abs2/diverged_fraction: passed (mean_abs2=" << response.summary.mean_abs2 << ")\n";
        passed++;
    }

    // 11. function block rejects unknown keys (no-implicit-fallback) ----------
    {
        std::string badJson = R"({
  "request_version": 1,
  "request_id": "gf-bad-func-key",
  "function_id": "generic.sample",
  "mode": "point_set",
  "function": {
    "expression": "z^2",
    "expresion_typo": "should fail"
  },
  "points": [{"x": 0.0, "y": 0.0}]
})";
        FractalProbeRequest request{};
        std::string error;
        bool ok = ParseFractalProbeRequestJson(badJson, &request, &error);
        if (ok) {
            std::cerr << "[11] function block with unknown key should fail parsing\n";
            return 1;
        }
        if (error.find("expresion_typo") == std::string::npos) {
            std::cerr << "[11] error should mention the bad key: " << error << "\n";
            return 1;
        }
        std::cout << "[11] function block rejects unknown keys: passed\n";
        passed++;
    }

    // 12. derivative is computed for iterate roots ----------------------------
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "gf-derivative";
        request.function_id = "generic.sample";
        request.mode = FractalProbeMode::point_set;
        request.has_function = true;
        request.generic_expression = "iterate(z - (z^3 - 1) / (3 * z^2), 200)";
        request.generic_epsilon = 1e-10;
        request.generic_escape_radius = 1000.0;
        request.metrics = {"derivative", "value"};
        request.points.push_back({1.1, 0.05});

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "unused", &response, &error)) {
            std::cerr << "[12] derivative request failed: " << error << "\n";
            return 1;
        }
        if (response.samples.size() != 1) {
            std::cerr << "[12] expected 1 sample\n";
            return 1;
        }
        const auto& s = response.samples[0];
        // At a converged root, the derivative of the Newton step f(z) = z - p(z)/p'(z)
        // should have |f'(root)| = 0 for a simple root (superlinear convergence).
        double dmag = std::sqrt(s.derivative_x * s.derivative_x + s.derivative_y * s.derivative_y);
        // The numerical derivative at a root should be close to 0 for Newton's method.
        // For z^3-1, f'(z) = 1 - (3z^2 * 3z^2 - (z^3-1)*6z) / (3z^2)^2
        // At z=1: f'(1) = 1 - (9 - 0)/9 = 0
        if (dmag > 0.1) {
            std::cerr << "[12] derivative magnitude at root should be near 0, got " << dmag << "\n";
            return 1;
        }

        // Verify JSON has derivative fields.
        std::string json = SerializeFractalProbeResponseJson(response);
        if (json.find("derivative_x") == std::string::npos || json.find("derivative_y") == std::string::npos) {
            std::cerr << "[12] JSON missing derivative_x/derivative_y\n";
            return 1;
        }
        std::cout << "[12] derivative at root: passed (|f'|=" << dmag << ")\n";
        passed++;
    }

    // 13. direct eval derivative (non-iterate) --------------------------------
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "gf-direct-deriv";
        request.function_id = "generic.sample";
        request.mode = FractalProbeMode::point_set;
        request.has_function = true;
        request.generic_expression = "z^2";
        request.metrics = {"value", "derivative"};
        // d/dz(z^2) = 2z. At z=3: deriv = (6, 0).
        request.points.push_back({3.0, 0.0});

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "unused", &response, &error)) {
            std::cerr << "[13] direct derivative failed: " << error << "\n";
            return 1;
        }
        const auto& s = response.samples[0];
        // value should be (9, 0)
        if (!NearlyEqual(s.final_z_x, 9.0, 1e-6)) {
            std::cerr << "[13] value_x=" << s.final_z_x << " expected 9\n";
            return 1;
        }
        // derivative should be close to (6, 0)
        if (!NearlyEqual(s.derivative_x, 6.0, 1e-4)) {
            std::cerr << "[13] derivative_x=" << s.derivative_x << " expected 6\n";
            return 1;
        }
        if (!NearlyEqual(s.derivative_y, 0.0, 1e-4)) {
            std::cerr << "[13] derivative_y=" << s.derivative_y << " expected 0\n";
            return 1;
        }
        std::cout << "[13] direct eval derivative: passed (f'(3)=" << s.derivative_x << ")\n";
        passed++;
    }

    // 14. z_conj works --------------------------------------------------------
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "gf-zconj";
        request.function_id = "generic.sample";
        request.mode = FractalProbeMode::point_set;
        request.has_function = true;
        request.generic_expression = "z_conj";
        request.metrics = {"value"};
        // z = (2, 3), conj = (2, -3)
        request.points.push_back({2.0, 3.0});

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "unused", &response, &error)) {
            std::cerr << "[14] z_conj failed: " << error << "\n";
            return 1;
        }
        if (!NearlyEqual(response.samples[0].final_z_x, 2.0, 1e-10) ||
            !NearlyEqual(response.samples[0].final_z_y, -3.0, 1e-10)) {
            std::cerr << "[14] z_conj(2+3i)=" << response.samples[0].final_z_x << "+" << response.samples[0].final_z_y << "i\n";
            return 1;
        }
        std::cout << "[14] z_conj: passed\n";
        passed++;
    }

    // 15. transcendental: exp(z) at known value -------------------------------
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "gf-exp";
        request.function_id = "generic.sample";
        request.mode = FractalProbeMode::point_set;
        request.has_function = true;
        request.generic_expression = "exp(z)";
        request.metrics = {"value"};
        // exp(1 + pi*i) = e * (cos(pi) + i*sin(pi)) = -e + 0i
        request.points.push_back({1.0, 3.14159265358979});

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "unused", &response, &error)) {
            std::cerr << "[15] exp(z) failed: " << error << "\n";
            return 1;
        }
        double e = std::exp(1.0);
        if (!NearlyEqual(response.samples[0].final_z_x, -e, 1e-6)) {
            std::cerr << "[15] exp(1+pi*i) real=" << response.samples[0].final_z_x << " expected " << -e << "\n";
            return 1;
        }
        if (!NearlyEqual(response.samples[0].final_z_y, 0.0, 1e-6)) {
            std::cerr << "[15] exp(1+pi*i) imag=" << response.samples[0].final_z_y << " expected 0\n";
            return 1;
        }
        std::cout << "[15] exp(z): passed\n";
        passed++;
    }

    // 16. compose through probe path ------------------------------------------
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "gf-compose-probe";
        request.function_id = "generic.sample";
        request.mode = FractalProbeMode::point_set;
        request.has_function = true;
        request.generic_expression = "compose(z^2, z + 1)";
        request.metrics = {"value"};
        // compose(z^2, z+1) at z=2 -> (2+1)^2 = 9
        request.points.push_back({2.0, 0.0});

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "unused", &response, &error)) {
            std::cerr << "[16] compose failed: " << error << "\n";
            return 1;
        }
        if (!NearlyEqual(response.samples[0].final_z_x, 9.0, 1e-6)) {
            std::cerr << "[16] compose(z^2,z+1)(2)=" << response.samples[0].final_z_x << " expected 9\n";
            return 1;
        }
        std::cout << "[16] compose probe: passed\n";
        passed++;
    }

    // 17. log / abs / z_conj built-ins ---------------------------------------
    {
        FractalProbeRequest logRequest{};
        logRequest.request_version = 1;
        logRequest.request_id = "gf-log-builtin";
        logRequest.function_id = "generic.sample";
        logRequest.mode = FractalProbeMode::point_set;
        logRequest.has_function = true;
        logRequest.generic_expression = "log(z)";
        logRequest.metrics = {"value", "abs2"};
        logRequest.points.push_back({std::exp(1.0), 0.0});

        FractalProbeResponse logResponse{};
        std::string error;
        if (!RunFractalProbeRequest(logRequest, "unused", &logResponse, &error)) {
            std::cerr << "[17] log(z) failed: " << error << "\n";
            return 1;
        }
        if (!NearlyEqual(logResponse.samples[0].final_z_x, 1.0, 1e-9) ||
            !NearlyEqual(logResponse.samples[0].final_z_y, 0.0, 1e-9)) {
            std::cerr << "[17] log(e)=" << logResponse.samples[0].final_z_x << "+" << logResponse.samples[0].final_z_y << "i expected 1\n";
            return 1;
        }

        FractalProbeRequest absRequest{};
        absRequest.request_version = 1;
        absRequest.request_id = "gf-abs-conj-builtin";
        absRequest.function_id = "generic.sample";
        absRequest.mode = FractalProbeMode::point_set;
        absRequest.has_function = true;
        absRequest.generic_expression = "abs(z_conj)";
        absRequest.metrics = {"value", "abs2"};
        absRequest.points.push_back({3.0, 4.0});

        FractalProbeResponse absResponse{};
        if (!RunFractalProbeRequest(absRequest, "unused", &absResponse, &error)) {
            std::cerr << "[17] abs(z_conj) failed: " << error << "\n";
            return 1;
        }
        if (!NearlyEqual(absResponse.samples[0].final_z_x, 5.0, 1e-9) ||
            !NearlyEqual(absResponse.samples[0].final_z_y, 0.0, 1e-9)) {
            std::cerr << "[17] abs(z_conj)(3+4i)=" << absResponse.samples[0].final_z_x << "+" << absResponse.samples[0].final_z_y << "i expected 5\n";
            return 1;
        }
        std::cout << "[17] log / abs / z_conj built-ins: passed\n";
        passed++;
    }

    // 18. sequence_grid over generic params ----------------------------------
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "gf-sequence-grid-scale-real";
        request.function_id = "generic.sample";
        request.mode = FractalProbeMode::sequence_grid;
        request.has_function = true;
        request.generic_expression = "compose(exp(z), scale * log(z + shift))";
        request.generic_params = {
            {"scale_real", 1.0},
            {"scale_imag", 0.0},
            {"shift_real", 0.35},
            {"shift_imag", 0.0},
        };
        request.metrics = {"status", "value", "abs2"};
        request.has_region = true;
        request.region.center_x = 0.0;
        request.region.center_y = 0.0;
        request.region.span_x = 4.0;
        request.region.span_y = 4.0;
        request.region.grid_width = 4;
        request.region.grid_height = 4;

        request.has_sequence = true;
        FractalProbeSequenceAxis axis;
        axis.path = "function.params.scale_real";
        axis.values.push_back(FractalProbeScalar::Number(0.5));
        axis.values.push_back(FractalProbeScalar::Number(1.0));
        axis.values.push_back(FractalProbeScalar::Number(1.5));
        request.sequence.axes.push_back(axis);
        request.sequence.zip_paths = false;

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "unused", &response, &error)) {
            std::cerr << "[18] sequence_grid params failed: " << error << "\n";
            return 1;
        }
        if (response.sequence_results.size() != 3) {
            std::cerr << "[18] expected 3 sequence results, got " << response.sequence_results.size() << "\n";
            return 1;
        }
        if (response.samples.size() != 48) {
            std::cerr << "[18] expected 48 samples, got " << response.samples.size() << "\n";
            return 1;
        }
        if (response.sequence_results[0].applied.size() != 1 ||
            response.sequence_results[0].applied[0].first != "function.params.scale_real" ||
            !NearlyEqual(response.sequence_results[0].applied[0].second.number_value, 0.5, 1e-12)) {
            std::cerr << "[18] first applied override missing expected scale_real=0.5\n";
            return 1;
        }
        if (response.sequence_results[2].applied.size() != 1 ||
            !NearlyEqual(response.sequence_results[2].applied[0].second.number_value, 1.5, 1e-12)) {
            std::cerr << "[18] third applied override missing expected scale_real=1.5\n";
            return 1;
        }
        std::cout << "[18] sequence_grid generic params: passed\n";
        passed++;
    }

    // 19. iterate() count must stay an integer literal -----------------------
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "gf-iterate-param-count-reject";
        request.function_id = "generic.sample";
        request.mode = FractalProbeMode::point_set;
        request.has_function = true;
        request.generic_expression = "iterate(z, steps)";
        request.generic_params = {{"steps", 8.0}};
        request.metrics = {"value"};
        request.points.push_back({0.5, 0.0});

        FractalProbeResponse response{};
        std::string error;
        if (RunFractalProbeRequest(request, "unused", &response, &error)) {
            std::cerr << "[19] iterate(z, steps) should be rejected in the current parser\n";
            return 1;
        }
        if (error.find("integer literal") == std::string::npos) {
            std::cerr << "[19] error should mention integer literal: " << error << "\n";
            return 1;
        }
        std::cout << "[19] iterate() literal-count restriction: passed\n";
        passed++;
    }

    // 20. summary-only metrics suppress sample payloads ----------------------
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "gf-summary-only";
        request.function_id = "generic.sample";
        request.mode = FractalProbeMode::point_set;
        request.has_function = true;
        request.generic_expression = "iterate(z^2, 20)";
        request.generic_epsilon = 1e-8;
        request.generic_escape_radius = 4.0;
        request.metrics = {"summary_mean_iterations", "summary_diverged_fraction"};
        request.points.push_back({0.5, 0.0});
        request.points.push_back({3.0, 0.0});

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "unused", &response, &error)) {
            std::cerr << "[20] summary-only request failed: " << error << "\n";
            return 1;
        }
        if (!response.samples.empty()) {
            std::cerr << "[20] expected summary-only request to suppress sample payloads\n";
            return 1;
        }
        if (response.summary.sample_count != 2) {
            std::cerr << "[20] expected summary.sample_count=2, got " << response.summary.sample_count << "\n";
            return 1;
        }
        if (!NearlyEqual(response.summary.diverged_fraction, 0.5, 0.01)) {
            std::cerr << "[20] diverged_fraction=" << response.summary.diverged_fraction << " expected ~0.5\n";
            return 1;
        }
        std::cout << "[20] summary-only payload suppression: passed\n";
        passed++;
    }

    const int total = 21;
    std::cout << "test_generic_probe: " << passed << "/" << total << " passed\n";
    return (passed == total) ? 0 : 1;
}
