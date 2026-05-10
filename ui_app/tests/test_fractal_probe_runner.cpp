#include "../src/fractal_probe_contract.h"
#include "../src/fractal_probe_runner.h"
#include "../src/json_min.h"

#include <cstdio>
#include <string>

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

std::string RuntimeBackendUsed(const FractalProbeResponse& response) {
    const json_min::ParseResult parsed = json_min::Parse(SerializeFractalProbeResponseJson(response));
    if (!parsed.error.empty()) return std::string();
    const json_min::Value* runtime = parsed.value.get("runtime");
    if (!runtime || !runtime->is_object()) return std::string();
    const json_min::Value* backendUsed = runtime->get("backend_used");
    if (!backendUsed || !backendUsed->is_string()) return std::string();
    return backendUsed->as_string();
}

void TestProbeSamplingImplementationFlag() {
    Check(IsProbeSamplingImplementedForFractalTypeId("newton"),
        "TestProbeSamplingImplementationFlag_KnownType");
    Check(IsProbeSamplingImplementedForFractalTypeId("lambda"),
        "TestProbeSamplingImplementationFlag_KnownEscapeType");
    Check(!IsProbeSamplingImplementedForFractalTypeId("not_a_real_fractal_type"),
        "TestProbeSamplingImplementationFlag_UnknownType");
}

void TestRunFractalProbeRequestRejectsNullResponse() {
    FractalProbeRequest request{};
    request.request_version = 1;
    request.request_id = "probe-runner-null-response";
    request.function_id = "fractal.sample";

    std::string error;
    Check(!RunFractalProbeRequest(request, "unused", nullptr, &error),
        "TestRunFractalProbeRequestRejectsNullResponse");
}

void TestRunFractalProbeRequestDispatchesGenericSample() {
    FractalProbeRequest request{};
    request.request_version = 1;
    request.request_id = "probe-runner-generic-sample";
    request.function_id = "generic.sample";
    request.mode = FractalProbeMode::point_set;
    request.has_function = true;
    request.generic_expression = "z^2 + z + 1";
    request.metrics = {"value", "abs2"};
    request.points.push_back({1.0, 0.0});
    request.points.push_back({0.0, 1.0});

    FractalProbeResponse response{};
    std::string error;
    const bool ok = RunFractalProbeRequest(request, "unused", &response, &error);

    Check(ok, "TestRunFractalProbeRequestDispatchesGenericSample_Runs");
    Check(response.ok, "TestRunFractalProbeRequestDispatchesGenericSample_ResponseOk");
    Check(response.function_id == "generic.sample",
        "TestRunFractalProbeRequestDispatchesGenericSample_FunctionId");
    Check(response.samples.size() == 2,
        "TestRunFractalProbeRequestDispatchesGenericSample_SampleCount");
    Check(RuntimeBackendUsed(response) == "cpu",
        "TestRunFractalProbeRequestDispatchesGenericSample_BackendUsed");
}

void TestRunFractalProbeRequestRejectsUnknownFunctionId() {
    FractalProbeRequest request{};
    request.request_version = 1;
    request.request_id = "probe-runner-unknown-function";
    request.function_id = "evil.compute";
    request.mode = FractalProbeMode::point_set;
    request.points.push_back({0.0, 0.0});

    FractalProbeResponse response{};
    std::string error;
    const bool ok = RunFractalProbeRequest(request, "unused", &response, &error);

    Check(!ok, "TestRunFractalProbeRequestRejectsUnknownFunctionId_Fails");
    Check(error.find("fractal.sample") != std::string::npos &&
            error.find("generic.sample") != std::string::npos,
        "TestRunFractalProbeRequestRejectsUnknownFunctionId_ListsValidIds");
}

} // namespace

int main() {
    TestProbeSamplingImplementationFlag();
    TestRunFractalProbeRequestRejectsNullResponse();
    TestRunFractalProbeRequestDispatchesGenericSample();
    TestRunFractalProbeRequestRejectsUnknownFunctionId();

    std::printf("test_fractal_probe_runner: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}