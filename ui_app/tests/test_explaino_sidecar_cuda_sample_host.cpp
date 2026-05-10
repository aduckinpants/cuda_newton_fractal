#include "../src/explaino_sidecar_cuda_sample_host.h"

#include <cstdio>
#include <string>
#include <vector>

namespace {

int g_passed = 0;
int g_failed = 0;
int g_sample_calls = 0;
bool g_stub_success = true;
const char* g_stub_error = nullptr;
std::vector<FractalSampleResult> g_stub_results;

void Check(bool condition, const char* name) {
    if (condition) {
        std::printf("  PASS: %s\n", name);
        ++g_passed;
    } else {
        std::printf("  FAIL: %s\n", name);
        ++g_failed;
    }
}

void ResetStub() {
    g_sample_calls = 0;
    g_stub_success = true;
    g_stub_error = nullptr;
    g_stub_results.clear();
}

} // namespace

bool SampleFractalPoints(
    const Double2* coords,
    int numPoints,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    FractalSampleResult* outResults,
    const char** outError) {
    (void)coords;
    (void)view;
    (void)params;
    (void)render;
    ++g_sample_calls;
    if (!g_stub_success) {
        if (outError) *outError = g_stub_error;
        return false;
    }
    for (int index = 0; index < numPoints; ++index) {
        outResults[index] = g_stub_results[static_cast<size_t>(index)];
    }
    if (outError) *outError = nullptr;
    return true;
}

namespace {

void TestRejectsNullResults() {
    ResetStub();
    CudaSidecarMeasurementHost host;
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    std::string error;

    Check(!host.Sample({}, view, params, render, nullptr, &error),
        "TestRejectsNullResults_Fails");
    Check(error == "CudaSidecarMeasurementHost requires outResults",
        "TestRejectsNullResults_Error");
    Check(g_sample_calls == 0,
        "TestRejectsNullResults_NoBackendCall");
}

void TestEmptyCoordsSkipsBackend() {
    ResetStub();
    CudaSidecarMeasurementHost host;
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    std::vector<FractalSampleResult> results(2);
    std::string error;

    Check(host.Sample({}, view, params, render, &results, &error),
        "TestEmptyCoordsSkipsBackend_Succeeds");
    Check(results.empty(),
        "TestEmptyCoordsSkipsBackend_ClearsResults");
    Check(g_sample_calls == 0,
        "TestEmptyCoordsSkipsBackend_NoBackendCall");
}

void TestBackendFailurePropagates() {
    ResetStub();
    g_stub_success = false;
    g_stub_error = "stub backend failure";

    CudaSidecarMeasurementHost host;
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    std::vector<Double2> coords = {{0.25, -0.5}};
    std::vector<FractalSampleResult> results;
    std::string error;

    Check(!host.Sample(coords, view, params, render, &results, &error),
        "TestBackendFailurePropagates_Fails");
    Check(error == "stub backend failure",
        "TestBackendFailurePropagates_Error");
    Check(g_sample_calls == 1,
        "TestBackendFailurePropagates_BackendCall");
}

void TestSuccessfulSampleForwardsResults() {
    ResetStub();
    g_stub_results = {
        {3, 1.0f, -1.0f, 0.5f, true, false, TerminationKind::root_converged, false, 0.0f},
        {8, -0.25f, 0.75f, 2.0f, false, true, TerminationKind::escaped_radius, true, 4.0f},
    };

    CudaSidecarMeasurementHost host;
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    std::vector<Double2> coords = {{0.0, 0.0}, {0.5, 0.5}};
    std::vector<FractalSampleResult> results;
    std::string error;

    Check(host.Sample(coords, view, params, render, &results, &error),
        "TestSuccessfulSampleForwardsResults_Succeeds");
    Check(results.size() == 2,
        "TestSuccessfulSampleForwardsResults_ResultCount");
    Check(results[0].iterations == 3 && results[0].termination_kind == TerminationKind::root_converged,
        "TestSuccessfulSampleForwardsResults_FirstResult");
    Check(results[1].escaped && results[1].has_far_field_delta && results[1].far_field_delta == 4.0f,
        "TestSuccessfulSampleForwardsResults_SecondResult");
    Check(g_sample_calls == 1,
        "TestSuccessfulSampleForwardsResults_BackendCall");
}

} // namespace

int main() {
    TestRejectsNullResults();
    TestEmptyCoordsSkipsBackend();
    TestBackendFailurePropagates();
    TestSuccessfulSampleForwardsResults();

    std::printf("test_explaino_sidecar_cuda_sample_host: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}