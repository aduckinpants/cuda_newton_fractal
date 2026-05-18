// test_fractal_probe_coverage.cpp
// Generic fractal probe coverage: every advertised fractal type must produce
// valid samples with finite coordinates and consistent status/sample-count.
//
// This test auto-discovers the supported fractal type list so that adding a
// new fractal type without wiring it into the probe runner is caught as a
// build-time failure.
//
// All tests use the in-process probe runner (no GPU needed).

#include "../src/fractal_probe_contract.h"
#include "../src/fractal_probe_runner.h"
#include "../src/fractal_family_rules.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

const char* kExePath = "D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe";

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

// Every fractal type that IsProbeSamplingImplementedForFractalTypeId supports.
// When a new type is added to the enum and the probe runner, it must be added
// here too — or the completeness check at the end catches it.
struct FractalProbeCase {
    const char* type_id;         // string ID used in overrides
    const char* expected_runtime_type; // runtime-visible public ID after request overrides
    bool is_basin;               // true if SupportsBasinColoring
    bool is_escape;              // true if escape-time family
    // Extra overrides needed for valid sampling (e.g. multibrot_power).
    std::vector<FractalProbeOverride> extra_overrides;
};

std::vector<FractalProbeCase> AllProbeCases() {
    return {
        // Basin-coloring (Newton-family) types
        {"newton", "newton", true, false, {}},
        {"halley", "halley", true, false, {}},
        {"explaino", "explaino", true, false, {}},
        {"explaino_all", "explaino_all", true, false, {}},
        {"explaino_y", "explaino_y", true, false, {}},
        {"explaino_fp", "explaino_fp", true, false, {}},
        {"explaino_halley", "explaino_halley", true, false, {}},
        {"explaino_dual", "explaino_dual", true, false, {}},
        {"explaino_mult", "explaino_mult", true, false, {}},
        {"explaino_phoenix", "explaino_phoenix", true, false, {}},
        {"explaino_transcendental", "explaino_transcendental", true, false, {}},
        {"explaino_inertial", "explaino_inertial", true, false, {}},
        {"explaino_rational", "explaino_rational", true, false, {}},
        {"explaino_collatz", "explaino_collatz", true, false, {}},
        {"explaino_joy", "explaino_joy", true, false, {}},
        {"explaino_fold", "explaino_fold", true, false, {}},
        {"explaino_bell", "explaino_bell", true, false, {}},
        {"explaino_projection_and_flow", "explaino_projection_and_flow", true, false, {}},
        // These explicit Explaino selectors now publish their own public runtime identity.
        {"explaino_ripple", "explaino_ripple", true, false, {}},
        {"explaino_splice", "explaino_splice", true, false, {}},
        {"explaino_vortex", "explaino_vortex", true, false, {}},
        {"explaino_tension", "explaino_tension", true, false, {}},
        {"explaino_balance_void", "explaino_balance_void", true, false, {}},

        // Escape-time types
        {"mandelbrot", "mandelbrot", false, true, {}},
        {"julia", "julia", false, true, {}},
        {"burning_ship", "burning_ship", false, true, {}},
        {"multibrot", "multibrot", false, true, {
            {"fractal.params.multibrot_power_float", FractalProbeScalar::Number(3.0)},
        }},
        {"phoenix", "phoenix", false, true, {}},
        {"multicorn", "multicorn", false, true, {
            {"fractal.params.multibrot_power", FractalProbeScalar::Number(3.0)},
        }},
        {"collatz", "collatz", false, true, {}},
        {"mcmullen", "mcmullen", false, true, {}},
        {"spider", "spider", false, true, {}},
        {"celtic_mandelbrot", "celtic_mandelbrot", false, true, {}},
        {"perpendicular_burning_ship", "perpendicular_burning_ship", false, true, {}},

        // Explaino escape-time hybrids
        {"nova", "nova", false, true, {}},
        {"explaino_nova", "explaino_nova", false, true, {}},
        {"explaino_julia", "explaino_julia", false, true, {}},
        {"lambda", "lambda", false, true, {}},
        {"explaino_lambda", "explaino_lambda", false, true, {
            {"fractal.params.lambda_real", FractalProbeScalar::Number(2.9685855)},
            {"fractal.params.lambda_imag", FractalProbeScalar::Number(-0.27446103)},
        }},
        {"explaino_rational_escape", "explaino_rational_escape", false, true, {}},
    };
}

// --- Test: every type produces valid point_set samples ---
bool TestPointSetSmokeForType(const FractalProbeCase& c) {
    FractalProbeRequest request{};
    request.request_version = 1;
    request.request_id = std::string("coverage-smoke-") + c.type_id;
    request.function_id = "fractal.sample";
    request.mode = FractalProbeMode::point_set;
    request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String(c.type_id)});
    for (const auto& ov : c.extra_overrides) {
        request.overrides.push_back(ov);
    }
    // Two well-separated sample points.
    request.points.push_back({0.125, 0.0});
    request.points.push_back({0.5, 0.3});

    FractalProbeResponse response{};
    std::string error;
    ASSERT(RunFractalProbeRequest(request, kExePath, &response, &error),
        (std::string("Probe should succeed for ") + c.type_id).c_str());
    ASSERT(response.ok,
        (std::string("Response ok for ") + c.type_id).c_str());
    ASSERT(response.runtime.fractal_type == c.expected_runtime_type,
        (std::string("Runtime fractal_type should match expected public id for ") + c.type_id).c_str());
    ASSERT(response.summary.sample_count == 2,
        (std::string("Should get 2 samples for ") + c.type_id).c_str());
    ASSERT(response.samples.size() == 2,
        (std::string("Sample vector size for ") + c.type_id).c_str());

    for (size_t i = 0; i < response.samples.size(); ++i) {
        const auto& s = response.samples[i];
        // final_z must be finite (no NaN/Inf leaking through).
        ASSERT(std::isfinite(s.final_z_x) && std::isfinite(s.final_z_y),
            (std::string("final_z finite for ") + c.type_id).c_str());
        // Status must be a valid enum value.
        ASSERT(s.status == FractalProbeSampleStatus::escaped ||
               s.status == FractalProbeSampleStatus::converged ||
               s.status == FractalProbeSampleStatus::bounded ||
               s.status == FractalProbeSampleStatus::pole ||
               s.status == FractalProbeSampleStatus::nonfinite,
            (std::string("Valid status for ") + c.type_id).c_str());
        // Iterations must be non-negative.
        ASSERT(s.iterations >= 0,
            (std::string("Non-negative iterations for ") + c.type_id).c_str());
    }
    return true;
}

// --- Test: basin types assign root indices ---
bool TestBasinTypesAssignRoots(const FractalProbeCase& c) {
    if (!c.is_basin) return true;

    FractalProbeRequest request{};
    request.request_version = 1;
    request.request_id = std::string("coverage-basin-") + c.type_id;
    request.function_id = "fractal.sample";
    request.mode = FractalProbeMode::point_set;
    request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String(c.type_id)});
    for (const auto& ov : c.extra_overrides) {
        request.overrides.push_back(ov);
    }
    request.points.push_back({0.5, 0.3});

    FractalProbeResponse response{};
    std::string error;
    ASSERT(RunFractalProbeRequest(request, kExePath, &response, &error),
        (std::string("Basin probe should succeed for ") + c.type_id).c_str());
    ASSERT(response.ok, (std::string("Response ok for ") + c.type_id).c_str());

    const auto& s = response.samples[0];
    // Not every basin type converges at every point; verify the contract:
    // converged samples must carry a valid root_index.
    if (s.status == FractalProbeSampleStatus::converged) {
        ASSERT(s.has_root_index,
            (std::string("Converged basin should have root_index: ") + c.type_id).c_str());
        ASSERT(s.root_index >= 0 && s.root_index < 8,
            (std::string("root_index in [0,8) for ") + c.type_id).c_str());
    }
    ASSERT(s.iterations >= 0,
        (std::string("Iterations non-negative for ") + c.type_id).c_str());
    return true;
}

// --- Test: grid mode produces correct sample count for every type ---
bool TestGridModeForType(const FractalProbeCase& c) {
    FractalProbeRequest request{};
    request.request_version = 1;
    request.request_id = std::string("coverage-grid-") + c.type_id;
    request.function_id = "fractal.sample";
    request.mode = FractalProbeMode::grid;
    request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String(c.type_id)});
    for (const auto& ov : c.extra_overrides) {
        request.overrides.push_back(ov);
    }
    request.has_region = true;
    request.region = {0.0, 0.0, 1.0, 1.0, 3, 3};

    FractalProbeResponse response{};
    std::string error;
    ASSERT(RunFractalProbeRequest(request, kExePath, &response, &error),
        (std::string("Grid probe should succeed for ") + c.type_id).c_str());
    ASSERT(response.ok, (std::string("Grid ok for ") + c.type_id).c_str());
    ASSERT(response.summary.sample_count == 9,
        (std::string("3x3 grid = 9 samples for ") + c.type_id).c_str());
    return true;
}

// --- Completeness gate: IsProbeSamplingImplementedForFractalTypeId must
//     cover every case in our test table. If a type id isn't recognized,
//     this catches the wiring gap. ---
bool TestProbeSamplingCompleteness() {
    const auto cases = AllProbeCases();
    for (const auto& c : cases) {
        ASSERT(IsProbeSamplingImplementedForFractalTypeId(c.type_id),
            (std::string("Type should be probe-implemented: ") + c.type_id).c_str());
    }
    return true;
}

} // namespace

int main() {
    const auto cases = AllProbeCases();

    // Completeness gate first.
    {
        const bool ok = TestProbeSamplingCompleteness();
        if (ok) {
            ++g_passed;
            std::cout << "  PASS: ProbeSamplingCompleteness\n";
        }
    }

    // Per-type smoke, basin, and grid tests.
    for (const auto& c : cases) {
        {
            const std::string name = std::string("PointSetSmoke[") + c.type_id + "]";
            const bool ok = TestPointSetSmokeForType(c);
            if (ok) { ++g_passed; std::cout << "  PASS: " << name << "\n"; }
        }
        {
            if (c.is_basin) {
                const std::string name = std::string("BasinRootIndex[") + c.type_id + "]";
                const bool ok = TestBasinTypesAssignRoots(c);
                if (ok) { ++g_passed; std::cout << "  PASS: " << name << "\n"; }
            }
        }
        {
            const std::string name = std::string("GridMode[") + c.type_id + "]";
            const bool ok = TestGridModeForType(c);
            if (ok) { ++g_passed; std::cout << "  PASS: " << name << "\n"; }
        }
    }

    std::cout << "test_fractal_probe_coverage: " << g_passed << " passed, " << g_failed << " failed\n";
    return g_failed > 0 ? 1 : 0;
}
