// test_fractal_sample_core.cu
// Dedicated coverage for SampleFractalPoints host-side guard behavior.

#include "../src/fractal_types.h"
#include "../src/fractal_family_rules.h"
#include "../src/fractal_sample_result.h"

#include <cstring>
#include <iostream>

namespace {

int g_passed = 0;
int g_failed = 0;

void Check(const char* name, bool condition) {
    if (condition) {
        ++g_passed;
    } else {
        ++g_failed;
        std::cerr << "FAIL: " << name << "\n";
    }
}

void MakeDefaults(ViewState* view, KernelParams* params, RenderSettings* render) {
    *view = ViewState{};
    view->fractal_type = FractalType::newton;
    view->center_hp_x = 0.0;
    view->center_hp_y = 0.0;
    view->log2_zoom = 0.0;

    *params = KernelParams{};
    params->max_iter = 64;
    params->epsilon = 1.0e-6f;
    params->coloring_mode = DefaultColoringModeForFractal(view->fractal_type);

    *render = RenderSettings{};
    render->resolution = {1, 1};
    render->device_id = 0;
    render->sample_tier = SampleTier::fast;
}

void TestZeroPointNoop() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    MakeDefaults(&view, &params, &render);
    params.max_iter = 0;

    const char* error = "stale";
    const bool ok = SampleFractalPoints(nullptr, 0, view, params, render, nullptr, &error);
    Check("zero points succeed without coords or output", ok);
    Check("zero points clear stale error", error == nullptr);
}

void TestNullInputGuards() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    MakeDefaults(&view, &params, &render);

    Double2 coord{0.0, 0.0};
    FractalSampleResult result{};
    const char* error = nullptr;

    bool ok = SampleFractalPoints(nullptr, 1, view, params, render, &result, &error);
    Check("null coords fail", !ok);
    Check("null coords report owner error", error && std::strcmp(error, "coords is null") == 0);

    error = nullptr;
    ok = SampleFractalPoints(&coord, 1, view, params, render, nullptr, &error);
    Check("null output fails", !ok);
    Check("null output reports owner error", error && std::strcmp(error, "outResults is null") == 0);
}

void TestRuntimeValidationBeforeCudaWork() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    MakeDefaults(&view, &params, &render);
    params.max_iter = 0;

    Double2 coord{0.0, 0.0};
    FractalSampleResult result{};
    const char* error = nullptr;
    const bool ok = SampleFractalPoints(&coord, 1, view, params, render, &result, &error);

    Check("invalid runtime state fails before sampling", !ok);
    Check("invalid runtime state returns validation error", error && std::strcmp(error, "max_iter must be > 0") == 0);
}

void TestCleanupIsIdempotent() {
    CleanupFractalSampleCore();
    CleanupFractalSampleCore();
    Check("cleanup can be called repeatedly", true);
}

} // namespace

int main() {
    TestZeroPointNoop();
    TestNullInputGuards();
    TestRuntimeValidationBeforeCudaWork();
    TestCleanupIsIdempotent();

    std::cout << "test_fractal_sample_core: passed=" << g_passed << " failed=" << g_failed << "\n";
    return g_failed == 0 ? 0 : 1;
}
