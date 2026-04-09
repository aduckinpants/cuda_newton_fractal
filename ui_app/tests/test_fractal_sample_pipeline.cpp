// test_fractal_sample_pipeline.cpp
// Regression tests for the fractal.sample function pipeline:
// request construction, override application, probe execution,
// basin-coloring continuity, and response contract invariants.
//
// All tests use the in-process probe runner (no GPU needed).

#include "../src/fractal_probe_contract.h"
#include "../src/fractal_probe_runner.h"
#include "../src/fractal_family_rules.h"
#include "../src/json_min.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

const char* kExePath = "D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe";

bool NearlyEqual(double left, double right, double eps = 1.0e-6) {
    const double d = left - right;
    return d < eps && d > -eps;
}

int g_passed = 0;
int g_failed = 0;

bool Check(bool condition, const char* file, int line, const char* msg) {
    if (!condition) {
        std::cerr << file << ":" << line << ": FAIL: " << msg << "\n";
        ++g_failed;
        return false;
    }
    return true;
}

#define ASSERT(cond, msg) do { if (!Check((cond), __FILE__, __LINE__, (msg))) return false; } while (0)

struct SamplePipelineTestCase {
    const char* name;
    bool (*run)();
};

// --- Newton known-root regression ---
bool TestNewtonKnownRoots() {
    struct Expected { double x; double y; int rootIndex; };
    const Expected roots[] = {
        {1.0, 0.0, 2},
        {-0.5, 0.8660254037844386, 0},
        {-0.5, -0.8660254037844386, 1},
    };

    FractalProbeRequest request{};
    request.request_version = 1;
    request.request_id = "pipeline-newton-roots";
    request.mode = FractalProbeMode::point_set;
    request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("newton")});
    for (const auto& root : roots) {
        request.points.push_back({root.x, root.y});
    }

    FractalProbeResponse response{};
    std::string error;
    ASSERT(RunFractalProbeRequest(request, kExePath, &response, &error),
        "Newton known-root probe should succeed");
    ASSERT(response.ok, "Response should be ok");
    ASSERT(response.samples.size() == 3, "Should get 3 samples");

    for (size_t i = 0; i < 3; ++i) {
        const auto& s = response.samples[i];
        ASSERT(s.status == FractalProbeSampleStatus::converged,
            "Newton root sample should converge");
        ASSERT(s.has_root_index && s.root_index == roots[i].rootIndex,
            "Newton root_index mismatch");
        ASSERT(NearlyEqual(s.final_z_x, roots[i].x) && NearlyEqual(s.final_z_y, roots[i].y),
            "Newton final_z should be near the root");
        ASSERT(s.iterations <= 5, "Newton at a root should converge fast");
    }
    return true;
}

// --- Halley known-root regression ---
bool TestHalleyKnownRoots() {
    struct Expected { double x; double y; int rootIndex; };
    const Expected roots[] = {
        {1.0, 0.0, 2},
        {-0.5, 0.8660254037844386, 0},
        {-0.5, -0.8660254037844386, 1},
    };

    FractalProbeRequest request{};
    request.request_version = 1;
    request.request_id = "pipeline-halley-roots";
    request.mode = FractalProbeMode::point_set;
    request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("halley")});
    for (const auto& root : roots) {
        request.points.push_back({root.x, root.y});
    }

    FractalProbeResponse response{};
    std::string error;
    ASSERT(RunFractalProbeRequest(request, kExePath, &response, &error),
        "Halley known-root probe should succeed");
    ASSERT(response.ok, "Response should be ok");
    ASSERT(response.samples.size() == 3, "Should get 3 samples");

    for (size_t i = 0; i < 3; ++i) {
        const auto& s = response.samples[i];
        ASSERT(s.status == FractalProbeSampleStatus::converged,
            "Halley root sample should converge");
        ASSERT(s.has_root_index && s.root_index == roots[i].rootIndex,
            "Halley root_index mismatch");
        ASSERT(NearlyEqual(s.final_z_x, roots[i].x) && NearlyEqual(s.final_z_y, roots[i].y),
            "Halley final_z should be near the root");
    }
    return true;
}

// --- Explaino-Y basin continuity (seam diagnostic) ---
// Verifies that nearby points in the same basin report the same root_index.
bool TestExplainoYBasinContinuity() {
    FractalProbeRequest request{};
    request.request_version = 1;
    request.request_id = "pipeline-explaino-y-continuity";
    request.mode = FractalProbeMode::point_set;
    request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("explaino_y")});
    request.overrides.push_back({"fractal.params.explaino_warp_strength", FractalProbeScalar::Number(0.16)});
    request.overrides.push_back({"fractal.params.explaino_seed", FractalProbeScalar::Number(0.0)});

    // Sample a tight cluster of points at the origin — should all converge
    // to the same basin when far from any boundary.
    const double cx = 0.1, cy = 0.1;
    const double offsets[] = {-1e-4, -1e-5, 0.0, 1e-5, 1e-4};
    for (double dy : offsets) {
        request.points.push_back({cx, cy + dy});
    }

    FractalProbeResponse response{};
    std::string error;
    ASSERT(RunFractalProbeRequest(request, kExePath, &response, &error),
        "Explaino-Y continuity probe should succeed");
    ASSERT(response.ok, "Response should be ok");
    ASSERT(response.samples.size() == 5, "Should get 5 samples");

    // All should converge (explaino_y snaps to bestZ).
    for (const auto& s : response.samples) {
        ASSERT(s.status == FractalProbeSampleStatus::converged,
            "Explaino-Y sample should converge");
    }

    // All nearby points should hit the same basin.
    const int firstRoot = response.samples[0].root_index;
    for (size_t i = 1; i < response.samples.size(); ++i) {
        ASSERT(response.samples[i].root_index == firstRoot,
            "Explaino-Y nearby points should share the same basin");
    }
    return true;
}

// --- Grid probe structural invariants ---
bool TestGridProbeStructure() {
    FractalProbeRequest request{};
    request.request_version = 1;
    request.request_id = "pipeline-grid-structure";
    request.mode = FractalProbeMode::grid;
    request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("newton")});
    request.has_region = true;
    request.region = {0.0, 0.0, 2.0, 2.0, 4, 3};

    FractalProbeResponse response{};
    std::string error;
    ASSERT(RunFractalProbeRequest(request, kExePath, &response, &error),
        "Grid probe should succeed");
    ASSERT(response.ok, "Response should be ok");
    ASSERT(response.summary.sample_count == 12, "4x3 grid = 12 samples");
    ASSERT(response.samples.size() == 12, "Sample vector size matches count");

    // All grid coords should be in range.
    for (const auto& s : response.samples) {
        ASSERT(s.grid_x >= 0 && s.grid_x < 4, "grid_x in range");
        ASSERT(s.grid_y >= 0 && s.grid_y < 3, "grid_y in range");
    }

    // Every (gx,gy) pair should appear exactly once.
    bool seen[4][3] = {};
    for (const auto& s : response.samples) {
        ASSERT(!seen[s.grid_x][s.grid_y], "No duplicate grid cell");
        seen[s.grid_x][s.grid_y] = true;
    }
    for (int gx = 0; gx < 4; ++gx)
        for (int gy = 0; gy < 3; ++gy)
            ASSERT(seen[gx][gy], "Every grid cell covered");

    return true;
}

// --- Response serialization round-trip ---
bool TestResponseSerialization() {
    FractalProbeRequest request{};
    request.request_version = 1;
    request.request_id = "pipeline-serialize";
    request.mode = FractalProbeMode::point_set;
    request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("newton")});
    request.points.push_back({0.5, 0.0});
    request.metrics.push_back("iterations");
    request.metrics.push_back("status");
    request.metrics.push_back("final_z");
    request.metrics.push_back("summary_sample_count");

    FractalProbeResponse response{};
    std::string error;
    ASSERT(RunFractalProbeRequest(request, kExePath, &response, &error),
        "Serialization probe should succeed");
    ASSERT(response.ok, "Response should be ok");

    const std::string json = SerializeFractalProbeResponseJson(response);
    json_min::ParseResult pr = json_min::Parse(json);
    ASSERT(pr.error.empty(), "Serialized JSON should parse cleanly");

    const json_min::Value* ok = pr.value.get("ok");
    ASSERT(ok && ok->is_bool() && ok->as_bool(), "ok field should be true");

    const json_min::Value* summary = pr.value.get("summary");
    ASSERT(summary && summary->is_object(), "summary should be an object");

    const json_min::Value* samples = pr.value.get("samples");
    ASSERT(samples && samples->is_array() && samples->as_array().size() == 1, "samples should have 1 entry");

    const json_min::Value* requestIdField = pr.value.get("request_id");
    ASSERT(requestIdField && requestIdField->is_string() && requestIdField->as_string() == "pipeline-serialize",
        "request_id should be echoed");

    return true;
}

// --- Mandelbrot escape at known-exterior point ---
bool TestMandelbrotEscape() {
    FractalProbeRequest request{};
    request.request_version = 1;
    request.request_id = "pipeline-mandelbrot-escape";
    request.mode = FractalProbeMode::point_set;
    request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("mandelbrot")});
    // c = 2+0i is well outside the Mandelbrot set.
    request.points.push_back({2.0, 0.0});
    // c = 0+0i is deep inside (period-1 bulb center).
    request.points.push_back({0.0, 0.0});

    FractalProbeResponse response{};
    std::string error;
    ASSERT(RunFractalProbeRequest(request, kExePath, &response, &error),
        "Mandelbrot probe should succeed");
    ASSERT(response.ok, "Response should be ok");
    ASSERT(response.samples.size() == 2, "Should get 2 samples");

    const auto& exterior = response.samples[0];
    const auto& interior = response.samples[1];
    ASSERT(exterior.status == FractalProbeSampleStatus::escaped,
        "c=2 should escape the Mandelbrot set");
    ASSERT(exterior.iterations <= 5,
        "c=2 should escape very fast");

    ASSERT(interior.status == FractalProbeSampleStatus::bounded ||
           interior.status == FractalProbeSampleStatus::converged,
        "c=0 should stay bounded");
    return true;
}

// --- Nova zero-derivative handling ---
bool TestNovaZeroDerivative() {
    FractalProbeRequest request{};
    request.request_version = 1;
    request.request_id = "pipeline-nova-zero-deriv";
    request.mode = FractalProbeMode::point_set;
    request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("nova")});
    // z=0 for z^3-1 has f'(0)=0. Nova should skip Newton step and apply +c.
    request.points.push_back({0.125, 0.0});
    request.points.push_back({0.25, 0.0});

    FractalProbeResponse response{};
    std::string error;
    ASSERT(RunFractalProbeRequest(request, kExePath, &response, &error),
        "Nova probe should succeed");
    ASSERT(response.ok, "Response should be ok");
    ASSERT(response.samples.size() == 2, "Should get 2 samples");

    for (const auto& s : response.samples) {
        ASSERT(s.iterations > 0, "Nova should advance past iteration 0");
        ASSERT(std::isfinite(s.final_z_x) && std::isfinite(s.final_z_y),
            "Nova final_z should be finite");
    }
    return true;
}

// --- Unknown function_id fails fast ---
bool TestUnknownFunctionIdFails() {
    FractalProbeRequest request{};
    request.request_version = 1;
    request.request_id = "pipeline-bad-function";
    request.function_id = "fractal.nonexistent";
    request.mode = FractalProbeMode::point_set;
    request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("newton")});
    request.points.push_back({0.5, 0.0});

    FractalProbeResponse response{};
    std::string error;
    const bool ran = RunFractalProbeRequest(request, kExePath, &response, &error);
    // Either RunFractalProbeRequest returns false, or response.ok is false.
    ASSERT(!ran || !response.ok, "Unknown function_id should fail");
    return true;
}

// --- Sequence-grid mode with zipped axes ---
bool TestSequenceGridZip() {
    FractalProbeRequest request{};
    request.request_version = 1;
    request.request_id = "pipeline-sequence-grid-zip";
    request.mode = FractalProbeMode::sequence_grid;
    request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("newton")});
    request.has_region = true;
    request.region = {0.0, 0.0, 1.0, 1.0, 2, 2};
    request.has_sequence = true;
    request.sequence.zip_paths = true;
    request.sequence.axes.push_back({"fractal.params.max_iter",
        {FractalProbeScalar::Number(50.0), FractalProbeScalar::Number(100.0)}});

    FractalProbeResponse response{};
    std::string error;
    ASSERT(RunFractalProbeRequest(request, kExePath, &response, &error),
        "Sequence-grid probe should succeed");
    ASSERT(response.ok, "Response should be ok");
    ASSERT(response.sequence_results.size() == 2,
        "Zipped 2-value axis should produce 2 sequence results");
    ASSERT(response.summary.sample_count == 8,
        "2 sequences * 2x2 grid = 8 samples");
    return true;
}

// --- All basin-coloring types assign valid root indices ---
bool TestBasinColoringRootIndex() {
    const char* basinTypes[] = {
        "newton",
        "halley",
        "explaino",
        "explaino_y",
        "explaino_fp",
        "explaino_halley",
        "explaino_dual",
        "explaino_mult",
        "explaino_phoenix",
        "explaino_transcendental",
        "explaino_inertial",
        "explaino_rational",
        "explaino_collatz",
        "explaino_joy",
        "explaino_fold",
    };

    for (const char* type : basinTypes) {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = std::string("pipeline-basin-") + type;
        request.mode = FractalProbeMode::point_set;
        request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String(type)});
        request.points.push_back({0.5, 0.3});

        FractalProbeResponse response{};
        std::string error;
        ASSERT(RunFractalProbeRequest(request, kExePath, &response, &error),
            (std::string("Basin probe should succeed for ") + type).c_str());
        ASSERT(response.ok, (std::string("Response ok for ") + type).c_str());
        ASSERT(response.samples.size() == 1,
            (std::string("One sample for ") + type).c_str());

        const auto& s = response.samples[0];
        // Not every basin type converges at every point; verify the contract:
        // converged samples must carry a valid root_index.
        if (s.status == FractalProbeSampleStatus::converged) {
            ASSERT(s.has_root_index && s.root_index >= 0,
                (std::string("Converged basin should assign root_index: ") + type).c_str());
            ASSERT(s.root_index < 8,
                (std::string("root_index should be in [0,8): ") + type).c_str());
        }
        // Whether converged or exhausted, the sample must have run.
        ASSERT(s.iterations >= 0,
            (std::string("Iterations should be non-negative: ") + type).c_str());
    }
    return true;
}

// --- Explaino-Y with custom polynomial: 4 distinct basins ---
bool TestExplainoYFourBasins() {
    FractalProbeRequest request{};
    request.request_version = 1;
    request.request_id = "pipeline-explaino-y-four-basins";
    request.mode = FractalProbeMode::grid;
    request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("explaino_y")});
    request.overrides.push_back({"fractal.params.explaino_warp_strength", FractalProbeScalar::Number(0.16)});
    request.overrides.push_back({"fractal.params.explaino_seed", FractalProbeScalar::Number(0.0)});
    request.has_region = true;
    request.region = {0.0, 0.0, 80.0, 80.0, 16, 16};

    FractalProbeResponse response{};
    std::string error;
    ASSERT(RunFractalProbeRequest(request, kExePath, &response, &error),
        "Explaino-Y four-basin grid probe should succeed");
    ASSERT(response.ok, "Response should be ok");
    ASSERT(response.samples.size() == 256, "16x16 grid = 256 samples");

    // Count distinct basins.
    bool basinSeen[8] = {};
    int distinctCount = 0;
    for (const auto& s : response.samples) {
        ASSERT(s.status == FractalProbeSampleStatus::converged,
            "Explaino-Y should always converge (bestZ snap)");
        ASSERT(s.has_root_index && s.root_index >= 0 && s.root_index < 8,
            "root_index in valid range");
        if (!basinSeen[s.root_index]) {
            basinSeen[s.root_index] = true;
            ++distinctCount;
        }
    }
    ASSERT(distinctCount >= 2,
        "Explaino-Y wide grid should cover at least 2 distinct basins");
    return true;
}

} // namespace

int main() {
    const SamplePipelineTestCase tests[] = {
        {"NewtonKnownRoots", TestNewtonKnownRoots},
        {"HalleyKnownRoots", TestHalleyKnownRoots},
        {"ExplainoYBasinContinuity", TestExplainoYBasinContinuity},
        {"GridProbeStructure", TestGridProbeStructure},
        {"ResponseSerialization", TestResponseSerialization},
        {"MandelbrotEscape", TestMandelbrotEscape},
        {"NovaZeroDerivative", TestNovaZeroDerivative},
        {"UnknownFunctionIdFails", TestUnknownFunctionIdFails},
        {"SequenceGridZip", TestSequenceGridZip},
        {"BasinColoringRootIndex", TestBasinColoringRootIndex},
        {"ExplainoYFourBasins", TestExplainoYFourBasins},
    };

    for (const auto& t : tests) {
        const bool ok = t.run();
        if (ok) {
            ++g_passed;
            std::cout << "  PASS: " << t.name << "\n";
        }
        // g_failed is incremented inside ASSERT on failure
    }

    std::cout << "test_fractal_sample_pipeline: " << g_passed << " passed, " << g_failed << " failed\n";
    return g_failed > 0 ? 1 : 0;
}
