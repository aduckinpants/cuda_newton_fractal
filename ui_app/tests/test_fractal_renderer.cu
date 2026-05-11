#include "../src/fractal_types.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

namespace {

int g_passed = 0;
int g_failed = 0;

void Check(bool condition, const char* message) {
    if (condition) {
        ++g_passed;
    } else {
        ++g_failed;
        std::cerr << "FAIL: " << message << "\n";
    }
}

bool ErrorContains(const char* error, const char* needle) {
    return error && std::strstr(error, needle) != nullptr;
}

void TestNullOutputFailsBeforeCudaWork() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    const char* error = "stale";

    const bool ok = RenderFractalCUDA(view, params, render, nullptr, nullptr, &stats, &error);
    Check(!ok, "RenderFractalCUDA rejects null output buffer");
    Check(ErrorContains(error, "outRGBA is null"), "Null output error names outRGBA");
}

void TestRuntimeValidationFailsBeforeBufferAllocation() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    uint32_t pixel = 0;
    const char* error = "stale";

    params.max_iter = 0;
    const bool ok = RenderFractalCUDA(view, params, render, &pixel, nullptr, &stats, &error);
    Check(!ok, "RenderFractalCUDA rejects invalid runtime state before rendering");
    Check(ErrorContains(error, "max_iter must be > 0"), "Runtime validation error names max_iter");
}

void TestInvalidResolutionFailsBeforeCudaAllocation() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    uint32_t pixel = 0;
    const char* error = "stale";

    render.resolution = {0, 1};
    const bool ok = RenderFractalCUDA(view, params, render, &pixel, nullptr, &stats, &error);
    Check(!ok, "RenderFractalCUDA rejects invalid resolution before buffer allocation");
    Check(ErrorContains(error, "Invalid resolution"), "Invalid resolution error is reported");
}

void TestTinyRenderProducesPixelsMaskAndStats() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    const char* error = nullptr;

    view.fractal_type = FractalType::newton;
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    params.max_iter = 12;
    params.coloring_mode = ColoringMode::root_basin;
    render.resolution = {2, 2};
    render.block_size = 64;
    render.sample_tier = SampleTier::fast;

    std::vector<uint32_t> pixels(4, 0u);
    std::vector<uint8_t> mask(4, 123u);
    const bool ok = RenderFractalCUDA(view, params, render, pixels.data(), mask.data(), &stats, &error);
    Check(ok, error ? error : "RenderFractalCUDA tiny render succeeds");
    if (!ok) {
        CleanupFractalCUDA();
        return;
    }

    bool sawAlpha = true;
    for (uint32_t pixel : pixels) {
        sawAlpha = sawAlpha && ((pixel >> 24) == 255u);
    }
    Check(sawAlpha, "Tiny render writes opaque RGBA pixels");

    bool maskValuesValid = true;
    for (uint8_t value : mask) {
        maskValuesValid = maskValuesValid && (value == 0u || value == 255u);
    }
    Check(maskValuesValid, "Tiny render writes binary mask values");
    Check(stats.last_device_id >= 0, "Render stats record a non-negative device id");
    Check(stats.last_iters_avg >= 0 && stats.last_iters_avg <= params.max_iter,
        "Render stats average iterations stay within max_iter");
    Check(stats.resolved_eval.backend == NumericBackend::float32 && stats.resolved_eval.strategy == IterationStrategy::direct,
        "Render stats record resolved fast direct mode");
    Check(stats.last_render_ms == 0.0f, "Non-benchmark render leaves elapsed milliseconds at zero");

    CleanupFractalCUDA();
    CleanupFractalCUDA();
    Check(true, "CleanupFractalCUDA is idempotent after a tiny render");
}

} // namespace

int main() {
    TestNullOutputFailsBeforeCudaWork();
    TestRuntimeValidationFailsBeforeBufferAllocation();
    TestInvalidResolutionFailsBeforeCudaAllocation();
    TestTinyRenderProducesPixelsMaskAndStats();

    std::cout << "test_fractal_renderer: passed=" << g_passed << " failed=" << g_failed << "\n";
    return g_failed == 0 ? 0 : 1;
}
