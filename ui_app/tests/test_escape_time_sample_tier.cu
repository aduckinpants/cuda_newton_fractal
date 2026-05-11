#include "../src/fractal_types.h"

#include <cstdint>
#include <iostream>
#include <vector>

namespace {

const char* BackendName(NumericBackend backend) {
    switch (backend) {
    case NumericBackend::float32: return "float32";
    case NumericBackend::float64: return "float64";
    }
    return "unknown";
}

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


bool RenderExplainoSmoothEscape(SampleTier tier, std::vector<uint32_t>* outPixels, RenderStats* outStats) {
    if (!outPixels) return false;

    ViewState view{};
    view.fractal_type = FractalType::explaino;
    view.center_hp_x = 1.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 11.0;
    view.center.x = static_cast<float>(view.center_hp_x);
    view.center.y = static_cast<float>(view.center_hp_y);
    view.zoom = static_cast<float>(2048.0);

    KernelParams params{};
    params.max_iter = 64;
    params.epsilon = 1.0e-6f;
    params.coloring_mode = ColoringMode::smooth_escape;
    params.color_pipeline = {ColorSignal::smooth_escape, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
    params.color_smooth_escape_scale = 1.0f;
    params.color_smooth_escape_bias = 0.0f;
    params.color_heatmap_cycle_scale = 0.25f;
    params.color_heatmap_saturation = 1.0f;
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
        std::cerr << "RenderFractalCUDA failed for ExplainO smooth_escape tier test: " << (error ? error : "unknown") << "\n";
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

    std::vector<uint32_t> explainoFastPixels;
    std::vector<uint32_t> explainoStandardPixels;
    std::vector<uint32_t> explainoAutoPixels;
    RenderStats explainoFastStats{};
    RenderStats explainoStandardStats{};
    RenderStats explainoAutoStats{};

    if (!RenderExplainoSmoothEscape(SampleTier::fast, &explainoFastPixels, &explainoFastStats)) {
        CleanupFractalCUDA();
        return 1;
    }
    if (!RenderExplainoSmoothEscape(SampleTier::standard, &explainoStandardPixels, &explainoStandardStats)) {
        CleanupFractalCUDA();
        return 1;
    }
    if (!RenderExplainoSmoothEscape(SampleTier::tier_auto, &explainoAutoPixels, &explainoAutoStats)) {
        CleanupFractalCUDA();
        return 1;
    }

    const int explainoPixelDiffs = CountPixelDiffs(explainoFastPixels, explainoStandardPixels);
    constexpr int kMaterialSmoothEscapeDiffPixels = 64;
    if (explainoPixelDiffs < kMaterialSmoothEscapeDiffPixels) {
        std::cerr << "Smooth_escape classification witness is inconclusive: fast vs standard differ by "
                  << explainoPixelDiffs << " pixels, below material threshold "
                  << kMaterialSmoothEscapeDiffPixels << "\n";
        CleanupFractalCUDA();
        return 1;
    }

    if (explainoFastStats.resolved_eval.backend != NumericBackend::float32 ||
        explainoStandardStats.resolved_eval.backend != NumericBackend::float64) {
        std::cerr << "Smooth_escape classification witness expected explicit fast=float32 and standard=float64, got fast="
                  << BackendName(explainoFastStats.resolved_eval.backend)
                  << ", standard=" << BackendName(explainoStandardStats.resolved_eval.backend) << "\n";
        CleanupFractalCUDA();
        return 1;
    }

    if (explainoAutoStats.resolved_eval.backend != NumericBackend::float64) {
        const int autoFastDiffs = CountPixelDiffs(explainoAutoPixels, explainoFastPixels);
        const int autoStandardDiffs = CountPixelDiffs(explainoAutoPixels, explainoStandardPixels);
        std::cerr << "Expected tier_auto to classify the material ExplainO smooth_escape fast/standard delta as requiring float64; "
                  << "auto backend=" << BackendName(explainoAutoStats.resolved_eval.backend)
                  << ", fast_standard_diffs=" << explainoPixelDiffs
                  << ", auto_fast_diffs=" << autoFastDiffs
                  << ", auto_standard_diffs=" << autoStandardDiffs << "\n";
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