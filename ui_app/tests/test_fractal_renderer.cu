#include "../src/fractal_types.h"
#include "../src/fractal_family_rules.h"

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

int CountDistinctPixels(const std::vector<uint32_t>& pixels) {
    std::vector<uint32_t> distinct;
    distinct.reserve(pixels.size());
    for (uint32_t pixel : pixels) {
        bool seen = false;
        for (uint32_t existing : distinct) {
            if (existing == pixel) {
                seen = true;
                break;
            }
        }
        if (!seen) {
            distinct.push_back(pixel);
        }
    }
    return static_cast<int>(distinct.size());
}

int CountNonBlackPixels(const std::vector<uint32_t>& pixels) {
    int count = 0;
    for (uint32_t pixel : pixels) {
        if ((pixel & 0x00ffffffu) != 0u) ++count;
    }
    return count;
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

void TestCounterfactualPairRenderProducesMultipleClassColors() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    const char* error = nullptr;

    view.fractal_type = FractalType::counterfactual_pair;
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    params.max_iter = 96;
    params.epsilon = 1e-6f;
    params.coloring_mode = ColoringMode::root_basin;
    render.resolution = {16, 16};
    render.block_size = 64;
    render.sample_tier = SampleTier::fast;

    std::vector<uint32_t> pixels(16 * 16, 0u);
    const bool ok = RenderFractalCUDA(view, params, render, pixels.data(), nullptr, &stats, &error);
    Check(ok, error ? error : "Counterfactual Pair render succeeds");
    if (!ok) {
        CleanupFractalCUDA();
        return;
    }

    bool sawDifferentPixel = false;
    const uint32_t firstPixel = pixels.front();
    for (uint32_t pixel : pixels) {
        if (pixel != firstPixel) {
            sawDifferentPixel = true;
            break;
        }
    }
    Check(sawDifferentPixel, "Counterfactual Pair render emits more than one explicit class color");

    CleanupFractalCUDA();
}

void TestProjectionAndFlowRenderProducesMultipleClassColors() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    const char* error = nullptr;

    view.fractal_type = FractalType::projection_and_flow;
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    params.max_iter = 96;
    params.epsilon = 1e-6f;
    params.coloring_mode = ColoringMode::root_basin;
    render.resolution = {16, 16};
    render.block_size = 64;
    render.sample_tier = SampleTier::fast;

    std::vector<uint32_t> pixels(16 * 16, 0u);
    const bool ok = RenderFractalCUDA(view, params, render, pixels.data(), nullptr, &stats, &error);
    Check(ok, error ? error : "Projection-and-Flow render succeeds");
    if (!ok) {
        CleanupFractalCUDA();
        return;
    }

    Check(CountDistinctPixels(pixels) >= 6, "Projection-and-Flow unit-radius render emits several explicit class colors");

    CleanupFractalCUDA();
}

void TestProjectionAndFlowNonUnitRadiusRenderDoesNotCollapseToThreeColors() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    const char* error = nullptr;

    view.fractal_type = FractalType::projection_and_flow;
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    params.max_iter = 96;
    params.epsilon = 1e-6f;
    params.coloring_mode = ColoringMode::root_basin;
    params.projection_and_flow_target_radius = 1.75f;
    params.projection_and_flow_pressure_threshold = 1.0f;
    render.resolution = {64, 48};
    render.block_size = 64;
    render.sample_tier = SampleTier::fast;

    std::vector<uint32_t> pixels(64 * 48, 0u);
    const bool ok = RenderFractalCUDA(view, params, render, pixels.data(), nullptr, &stats, &error);
    Check(ok, error ? error : "Projection-and-Flow non-unit-radius render succeeds");
    if (!ok) {
        CleanupFractalCUDA();
        return;
    }

    Check(CountDistinctPixels(pixels) >= 6, "Projection-and-Flow non-unit-radius render does not collapse to only three public class colors");

    CleanupFractalCUDA();
}

void TestProjectionAndFlowSmoothEscapeRenderKeepsStableClassesVisible() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    const char* error = nullptr;

    view.fractal_type = FractalType::projection_and_flow;
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    params.max_iter = 96;
    params.epsilon = 1e-6f;
    params.coloring_mode = ColoringMode::smooth_escape;
    params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
    params.projection_and_flow_target_radius = 1.75f;
    params.projection_and_flow_pressure_threshold = 1.0f;
    params.color_smooth_escape_scale = 1.0f;
    params.color_smooth_escape_bias = 0.0f;
    render.resolution = {64, 48};
    render.block_size = 64;
    render.sample_tier = SampleTier::fast;

    std::vector<uint32_t> pixels(64 * 48, 0u);
    const bool ok = RenderFractalCUDA(view, params, render, pixels.data(), nullptr, &stats, &error);
    Check(ok, error ? error : "Projection-and-Flow smooth_escape render succeeds");
    if (!ok) {
        CleanupFractalCUDA();
        return;
    }

    Check(CountDistinctPixels(pixels) > 1, "Projection-and-Flow smooth_escape render should not collapse to one flat color");
    Check(CountNonBlackPixels(pixels) > 0, "Projection-and-Flow smooth_escape render should keep stable classes visible instead of rendering all black");
    CleanupFractalCUDA();
}

} // namespace

int main() {
    TestNullOutputFailsBeforeCudaWork();
    TestRuntimeValidationFailsBeforeBufferAllocation();
    TestInvalidResolutionFailsBeforeCudaAllocation();
    TestTinyRenderProducesPixelsMaskAndStats();
    TestCounterfactualPairRenderProducesMultipleClassColors();
    TestProjectionAndFlowRenderProducesMultipleClassColors();
    TestProjectionAndFlowNonUnitRadiusRenderDoesNotCollapseToThreeColors();
    TestProjectionAndFlowSmoothEscapeRenderKeepsStableClassesVisible();

    std::cout << "test_fractal_renderer: passed=" << g_passed << " failed=" << g_failed << "\n";
    return g_failed == 0 ? 0 : 1;
}
