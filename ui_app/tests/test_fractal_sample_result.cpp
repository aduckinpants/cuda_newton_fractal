#include "../src/fractal_sample_result.h"

#include <cstdio>

namespace {

int g_passed = 0;
int g_failed = 0;

void Check(bool condition, const char* name) {
    if (condition) {
        std::printf("  PASS: %s\n", name);
        ++g_passed;
    } else {
        std::printf("  FAIL: %s\n", name);
        ++g_failed;
    }
}

void TestDefaultInitialization() {
    FractalSampleResult result{};

    Check(result.iterations == 0, "TestDefaultInitialization_Iterations");
    Check(result.final_z_x == 0.0f, "TestDefaultInitialization_FinalZX");
    Check(result.final_z_y == 0.0f, "TestDefaultInitialization_FinalZY");
    Check(result.residual == 0.0f, "TestDefaultInitialization_Residual");
    Check(!result.converged, "TestDefaultInitialization_Converged");
    Check(!result.escaped, "TestDefaultInitialization_Escaped");
    Check(result.termination_kind == TerminationKind::none,
        "TestDefaultInitialization_TerminationKind");
    Check(!result.has_far_field_delta, "TestDefaultInitialization_HasFarFieldDelta");
    Check(result.far_field_delta == 0.0f, "TestDefaultInitialization_FarFieldDelta");
}

void TestFieldRoundTrip() {
    FractalSampleResult result{};
    result.iterations = 42;
    result.final_z_x = 1.25f;
    result.final_z_y = -0.75f;
    result.residual = 0.125f;
    result.converged = true;
    result.escaped = false;
    result.termination_kind = TerminationKind::max_iterations;
    result.has_far_field_delta = true;
    result.far_field_delta = 3.5f;

    Check(result.iterations == 42, "TestFieldRoundTrip_Iterations");
    Check(result.final_z_x == 1.25f, "TestFieldRoundTrip_FinalZX");
    Check(result.final_z_y == -0.75f, "TestFieldRoundTrip_FinalZY");
    Check(result.residual == 0.125f, "TestFieldRoundTrip_Residual");
    Check(result.converged, "TestFieldRoundTrip_Converged");
    Check(!result.escaped, "TestFieldRoundTrip_Escaped");
    Check(result.termination_kind == TerminationKind::max_iterations,
        "TestFieldRoundTrip_TerminationKind");
    Check(result.has_far_field_delta, "TestFieldRoundTrip_HasFarFieldDelta");
    Check(result.far_field_delta == 3.5f, "TestFieldRoundTrip_FarFieldDelta");
}

void TestCompactLayoutExpectation() {
    Check(sizeof(FractalSampleResult) <= 24,
        "TestCompactLayoutExpectation_SizeBound");
}

} // namespace

int main() {
    TestDefaultInitialization();
    TestFieldRoundTrip();
    TestCompactLayoutExpectation();

    std::printf("test_fractal_sample_result: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}