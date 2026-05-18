#include "../src/fractal_sample_result.h"
#include "../src/fractal_types.h"

#include <cstdio>
#include <type_traits>

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

void TestWidenedEvidenceDefaults() {
    FractalSampleEvidence evidence{};

    Check(evidence.sample_coord.x == 0.0, "TestWidenedEvidenceDefaults_CoordX");
    Check(evidence.sample_coord.y == 0.0, "TestWidenedEvidenceDefaults_CoordY");
    Check(evidence.legacy_result.iterations == 0, "TestWidenedEvidenceDefaults_LegacyIterations");
    Check(evidence.legacy_result.final_z_x == 0.0f, "TestWidenedEvidenceDefaults_LegacyFinalZX");
    Check(evidence.legacy_result.final_z_y == 0.0f, "TestWidenedEvidenceDefaults_LegacyFinalZY");
    Check(evidence.legacy_result.residual == 0.0f, "TestWidenedEvidenceDefaults_LegacyResidual");
    Check(evidence.legacy_result.termination_kind == TerminationKind::none,
        "TestWidenedEvidenceDefaults_LegacyTerminationKind");
    Check(!evidence.legacy_result.has_far_field_delta,
        "TestWidenedEvidenceDefaults_LegacyHasFarFieldDelta");
    Check(evidence.legacy_result.far_field_delta == 0.0f,
        "TestWidenedEvidenceDefaults_LegacyFarFieldDelta");
    Check(sizeof(FractalSampleEvidence) > sizeof(FractalSampleResult),
        "TestWidenedEvidenceDefaults_SizeExceedsLegacy");
}

void TestLegacyProjectionHelperRoundTrip() {
    FractalSampleEvidence evidence{};
    evidence.sample_coord = {0.25, -0.5};
    evidence.legacy_result.iterations = 7;
    evidence.legacy_result.final_z_x = -1.25f;
    evidence.legacy_result.final_z_y = 0.75f;
    evidence.legacy_result.residual = 0.0625f;
    evidence.legacy_result.converged = true;
    evidence.legacy_result.escaped = false;
    evidence.legacy_result.termination_kind = TerminationKind::root_converged;
    evidence.legacy_result.has_far_field_delta = true;
    evidence.legacy_result.far_field_delta = 2.5f;

    const FractalSampleResult projected = BuildLegacySampleResult(evidence);
    Check(projected.iterations == 7, "TestLegacyProjectionHelperRoundTrip_Iterations");
    Check(projected.final_z_x == -1.25f, "TestLegacyProjectionHelperRoundTrip_FinalZX");
    Check(projected.final_z_y == 0.75f, "TestLegacyProjectionHelperRoundTrip_FinalZY");
    Check(projected.residual == 0.0625f, "TestLegacyProjectionHelperRoundTrip_Residual");
    Check(projected.converged, "TestLegacyProjectionHelperRoundTrip_Converged");
    Check(!projected.escaped, "TestLegacyProjectionHelperRoundTrip_Escaped");
    Check(projected.termination_kind == TerminationKind::root_converged,
        "TestLegacyProjectionHelperRoundTrip_TerminationKind");
    Check(projected.has_far_field_delta, "TestLegacyProjectionHelperRoundTrip_HasFarFieldDelta");
    Check(projected.far_field_delta == 2.5f, "TestLegacyProjectionHelperRoundTrip_FarFieldDelta");
}

void TestSampleFractalEvidencePointsSurfaceExists() {
    using EvidenceFn = bool (*)(
        const Double2*,
        int,
        const ViewState&,
        const KernelParams&,
        const RenderSettings&,
        FractalSampleEvidence*,
        const char**);
    Check(std::is_same_v<decltype(&SampleFractalEvidencePoints), EvidenceFn>,
        "TestSampleFractalEvidencePointsSurfaceExists_Signature");
}

void TestSampleFractalPointsStaysLegacyProjectionSurface() {
    using SampleFn = bool (*)(
        const Double2*,
        int,
        const ViewState&,
        const KernelParams&,
        const RenderSettings&,
        FractalSampleResult*,
        const char**);
    Check(std::is_same_v<decltype(&SampleFractalPoints), SampleFn>,
        "TestSampleFractalPointsStaysLegacyProjectionSurface_Signature");
}

void TestCompactLayoutExpectation() {
    Check(sizeof(FractalSampleResult) <= 24,
        "TestCompactLayoutExpectation_SizeBound");
}

} // namespace

int main() {
    TestDefaultInitialization();
    TestFieldRoundTrip();
    TestWidenedEvidenceDefaults();
    TestLegacyProjectionHelperRoundTrip();
    TestSampleFractalEvidencePointsSurfaceExists();
    TestSampleFractalPointsStaysLegacyProjectionSurface();
    TestCompactLayoutExpectation();

    std::printf("test_fractal_sample_result: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}