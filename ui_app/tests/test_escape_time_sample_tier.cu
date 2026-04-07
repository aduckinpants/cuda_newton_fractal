#include "../src/fractal_types.h"

#include <cstdint>
#include <iostream>
#include <vector>

namespace {

bool RenderDeepMandelbrot(SampleTier tier, std::vector<uint32_t>* outPixels, RenderStats* outStats) {
    if (!outPixels) return false;

    ViewState view{};
    view.fractal_type = FractalType::mandelbrot;
    view.center_hp_x = -0.743643887037151;
    view.center_hp_y = 0.131825904205330;
    view.log2_zoom = 30.0;
    view.center.x = static_cast<float>(view.center_hp_x);
    view.center.y = static_cast<float>(view.center_hp_y);
    view.zoom = static_cast<float>(1073741824.0);

    KernelParams params{};
    params.max_iter = 1200;
    params.coloring_mode = ColoringMode::smooth_escape;
    params.exposure = 1.0f;

    RenderSettings render{};
    render.resolution = {64, 64};
    render.block_size = 256;
    render.device_id = 0;
    render.sample_tier = tier;

    outPixels->assign(static_cast<size_t>(render.resolution.x) * static_cast<size_t>(render.resolution.y), 0u);
    const char* error = nullptr;
    RenderStats stats{};
    if (!RenderFractalCUDA(view, params, render, outPixels->data(), nullptr, &stats, &error)) {
        std::cerr << "RenderFractalCUDA failed for Mandelbrot tier test: " << (error ? error : "unknown") << "\n";
        return false;
    }
    if (outStats) *outStats = stats;
    return true;
}

int CountPixelDiffs(const std::vector<uint32_t>& left, const std::vector<uint32_t>& right) {
    if (left.size() != right.size()) return -1;
    int diffs = 0;
    for (size_t i = 0; i < left.size(); ++i) {
        if (left[i] != right[i]) ++diffs;
    }
    return diffs;
}

} // namespace

int main() {
    std::vector<uint32_t> fastPixels;
    std::vector<uint32_t> standardPixels;
    RenderStats fastStats{};
    RenderStats standardStats{};

    if (!RenderDeepMandelbrot(SampleTier::fast, &fastPixels, &fastStats)) {
        CleanupFractalCUDA();
        return 1;
    }
    if (!RenderDeepMandelbrot(SampleTier::standard, &standardPixels, &standardStats)) {
        CleanupFractalCUDA();
        return 1;
    }

    const int pixelDiffs = CountPixelDiffs(fastPixels, standardPixels);
    if (pixelDiffs <= 0) {
        std::cerr << "Expected deep Mandelbrot fast vs standard tiers to differ, but images were identical"
                  << " (fast avg iters=" << fastStats.last_iters_avg
                  << ", standard avg iters=" << standardStats.last_iters_avg << ")\n";
        CleanupFractalCUDA();
        return 1;
    }

    std::cout << "escape_time_sample_tier: " << pixelDiffs
              << " differing pixels, fast avg iters " << fastStats.last_iters_avg
              << ", standard avg iters " << standardStats.last_iters_avg << "\n";

    CleanupFractalCUDA();
    return 0;
}