#include "../src/fractal_types.h"
#include "../src/sample_tier_resolver.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

struct BenchCase {
    const char* name;
    FractalType fractal_type;
    double center_x;
    double center_y;
    double zoom;
    int max_iter;
};

struct BenchResult {
    std::string case_name;
    FractalType fractal_type{FractalType::mandelbrot};
    SampleTier tier{SampleTier::tier_auto};
    ResolvedEvalMode resolved{};
    double zoom{1.0};
    double log2_zoom{0.0};
    int width{0};
    int height{0};
    int repeats{0};
    double avg_ms{0.0};
    double min_ms{0.0};
    double max_ms{0.0};
    double avg_iters{0.0};
};

const char* FractalTypeId(FractalType ft) {
    switch (ft) {
    case FractalType::mandelbrot: return "mandelbrot";
    case FractalType::julia: return "julia";
    default: return "other";
    }
}

const char* SampleTierId(SampleTier tier) {
    switch (tier) {
    case SampleTier::tier_auto: return "auto";
    case SampleTier::fast: return "fast";
    case SampleTier::standard: return "standard";
    }
    return "unknown";
}

const char* NumericBackendId(NumericBackend backend) {
    switch (backend) {
    case NumericBackend::float32: return "float32";
    case NumericBackend::float64: return "float64";
    }
    return "unknown";
}

const char* IterationStrategyId(IterationStrategy strategy) {
    switch (strategy) {
    case IterationStrategy::direct: return "direct";
    }
    return "unknown";
}

double Log2D(double value) {
    return std::log(value) / std::log(2.0);
}

bool RunBenchCase(const BenchCase& benchCase,
    SampleTier tier,
    int width,
    int height,
    int warmupRuns,
    int timedRuns,
    BenchResult* outResult)
{
    if (!outResult) return false;

    ViewState view{};
    view.fractal_type = benchCase.fractal_type;
    view.center_hp_x = benchCase.center_x;
    view.center_hp_y = benchCase.center_y;
    view.log2_zoom = Log2D(benchCase.zoom);
    view.center.x = static_cast<float>(benchCase.center_x);
    view.center.y = static_cast<float>(benchCase.center_y);
    view.zoom = static_cast<float>(benchCase.zoom);

    KernelParams params{};
    params.max_iter = benchCase.max_iter;
    params.coloring_mode = ColoringMode::smooth_escape;
    params.exposure = 1.0f;

    RenderSettings render{};
    render.resolution = {width, height};
    render.block_size = 256;
    render.device_id = 0;
    render.benchmark = true;
    render.sample_tier = tier;

    std::vector<uint32_t> pixels(static_cast<size_t>(width) * static_cast<size_t>(height), 0u);
    const char* error = nullptr;
    RenderStats stats{};

    for (int i = 0; i < warmupRuns; ++i) {
        if (!RenderFractalCUDA(view, params, render, pixels.data(), nullptr, &stats, &error)) {
            std::cerr << "Warmup render failed for " << benchCase.name << ": " << (error ? error : "unknown") << "\n";
            CleanupFractalCUDA();
            return false;
        }
    }

    double sumMs = 0.0;
    double sumIters = 0.0;
    double minMs = std::numeric_limits<double>::max();
    double maxMs = 0.0;
    for (int i = 0; i < timedRuns; ++i) {
        if (!RenderFractalCUDA(view, params, render, pixels.data(), nullptr, &stats, &error)) {
            std::cerr << "Timed render failed for " << benchCase.name << ": " << (error ? error : "unknown") << "\n";
            CleanupFractalCUDA();
            return false;
        }
        sumMs += stats.last_render_ms;
        sumIters += static_cast<double>(stats.last_iters_avg);
        minMs = std::min(minMs, static_cast<double>(stats.last_render_ms));
        maxMs = std::max(maxMs, static_cast<double>(stats.last_render_ms));
    }

    outResult->case_name = benchCase.name;
    outResult->fractal_type = benchCase.fractal_type;
    outResult->tier = tier;
    outResult->resolved = ResolveSampleEvalMode(benchCase.fractal_type, tier, view.log2_zoom);
    outResult->zoom = benchCase.zoom;
    outResult->log2_zoom = view.log2_zoom;
    outResult->width = width;
    outResult->height = height;
    outResult->repeats = timedRuns;
    outResult->avg_ms = sumMs / static_cast<double>(timedRuns);
    outResult->min_ms = minMs;
    outResult->max_ms = maxMs;
    outResult->avg_iters = sumIters / static_cast<double>(timedRuns);
    return true;
}

void WriteCsv(const std::string& path, const std::vector<BenchResult>& results) {
    if (path.empty()) return;
    std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file) {
        std::cerr << "Failed to open benchmark CSV output: " << path << "\n";
        return;
    }
    file << "case_name,fractal_type,tier,resolved_backend,resolved_strategy,zoom,log2_zoom,width,height,repeats,avg_ms,min_ms,max_ms,avg_iters\n";
    file << std::fixed << std::setprecision(6);
    for (const BenchResult& result : results) {
        file << result.case_name << ','
             << FractalTypeId(result.fractal_type) << ','
             << SampleTierId(result.tier) << ','
             << NumericBackendId(result.resolved.backend) << ','
             << IterationStrategyId(result.resolved.strategy) << ','
             << result.zoom << ','
             << result.log2_zoom << ','
             << result.width << ','
             << result.height << ','
             << result.repeats << ','
             << result.avg_ms << ','
             << result.min_ms << ','
             << result.max_ms << ','
             << result.avg_iters << '\n';
    }
}

} // namespace

int main(int argc, char** argv) {
    const std::string csvPath = (argc >= 2 && argv[1]) ? argv[1] : std::string();

    const int width = 1024;
    const int height = 1024;
    const int warmupRuns = 1;
    const int timedRuns = 3;

    const BenchCase cases[] = {
        {"mandelbrot_zoom_379264", FractalType::mandelbrot, -0.743643887037151, 0.131825904205330, 379264.375, 1200},
        {"mandelbrot_zoom_4194304", FractalType::mandelbrot, -0.743643887037151, 0.131825904205330, 4194304.0, 1200},
        {"julia_zoom_126421", FractalType::julia, 0.0, 0.0, 126421.461, 1200},
        {"julia_zoom_4194304", FractalType::julia, 0.0, 0.0, 4194304.0, 1200},
    };
    const SampleTier tiers[] = {SampleTier::fast, SampleTier::standard, SampleTier::tier_auto};

    std::vector<BenchResult> results;
    for (const BenchCase& benchCase : cases) {
        for (SampleTier tier : tiers) {
            BenchResult result{};
            if (!RunBenchCase(benchCase, tier, width, height, warmupRuns, timedRuns, &result)) {
                CleanupFractalCUDA();
                return 1;
            }
            results.push_back(result);
            std::cout << std::fixed << std::setprecision(3)
                      << result.case_name
                      << " tier=" << SampleTierId(result.tier)
                      << " resolved=" << NumericBackendId(result.resolved.backend)
                      << '/' << IterationStrategyId(result.resolved.strategy)
                      << " avg_ms=" << result.avg_ms
                      << " min_ms=" << result.min_ms
                      << " max_ms=" << result.max_ms
                      << " avg_iters=" << result.avg_iters
                      << " log2_zoom=" << result.log2_zoom
                      << "\n";
        }
    }

    WriteCsv(csvPath, results);
    if (!csvPath.empty()) {
        std::cout << "CSV: " << csvPath << "\n";
    }

    CleanupFractalCUDA();
    return 0;
}