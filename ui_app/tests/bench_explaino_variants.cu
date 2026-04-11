#include "../src/explaino_variant_benchmark.h"
#include "../src/fractal_sample_result.h"
#include "../src/fractal_types.h"

#include <cuda_runtime.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

namespace {

constexpr int kWidth = 1024;
constexpr int kHeight = 1024;
constexpr int kWarmupRuns = 1;
constexpr int kTimedRuns = 3;

struct AggregateMetrics {
    double total_iterations = 0.0;
    double total_converged_iterations = 0.0;
    std::size_t total_points = 0;
    std::size_t converged_points = 0;
};

const char* NumericBackendIdLocal(NumericBackend backend) {
    switch (backend) {
    case NumericBackend::float32: return "float32";
    case NumericBackend::float64: return "float64";
    }
    return "unknown";
}

const char* IterationStrategyIdLocal(IterationStrategy strategy) {
    switch (strategy) {
    case IterationStrategy::direct: return "direct";
    }
    return "unknown";
}

void BuildGridCoordinates(const ViewState& view, int width, int height, std::vector<Double2>* outCoords) {
    if (!outCoords) return;
    outCoords->resize(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));

    const double aspect = static_cast<double>(width) / static_cast<double>(height);
    const double zoom = std::fmax(1.0e-300, std::exp2(view.log2_zoom));
    const double base = 2.0 / zoom;

    for (int py = 0; py < height; ++py) {
        for (int px = 0; px < width; ++px) {
            const double nx = (((static_cast<double>(px) + 0.5) / static_cast<double>(width)) - 0.5) * 2.0;
            const double ny = (((static_cast<double>(py) + 0.5) / static_cast<double>(height)) - 0.5) * 2.0;
            const double x = view.center_hp_x + nx * base * aspect;
            const double y = view.center_hp_y + ny * base;
            (*outCoords)[static_cast<std::size_t>(py) * static_cast<std::size_t>(width) + static_cast<std::size_t>(px)] = {x, y};
        }
    }
}

void AccumulateMetrics(const std::vector<FractalSampleResult>& results, AggregateMetrics* ioMetrics) {
    if (!ioMetrics) return;
    for (const FractalSampleResult& result : results) {
        ioMetrics->total_iterations += static_cast<double>(result.iterations);
        ++ioMetrics->total_points;
        if (result.converged) {
            ioMetrics->total_converged_iterations += static_cast<double>(result.iterations);
            ++ioMetrics->converged_points;
        }
    }
}

bool SampleOnce(const std::vector<Double2>& coords,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    std::vector<FractalSampleResult>* outResults,
    const char** outError) {
    if (!outResults) {
        if (outError) *outError = "benchmark output results vector is null";
        return false;
    }
    outResults->assign(coords.size(), {});
    return SampleFractalPoints(coords.data(),
        static_cast<int>(coords.size()),
        view,
        params,
        render,
        outResults->data(),
        outError);
}

bool MeasureSampleMs(const std::vector<Double2>& coords,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    std::vector<FractalSampleResult>* outResults,
    double* outMs,
    const char** outError) {
    if (!outMs) {
        if (outError) *outError = "benchmark elapsed-ms pointer is null";
        return false;
    }

    cudaSetDevice(render.device_id);

    cudaEvent_t start = nullptr;
    cudaEvent_t stop = nullptr;
    if (cudaEventCreate(&start) != cudaSuccess || cudaEventCreate(&stop) != cudaSuccess) {
        if (start) cudaEventDestroy(start);
        if (stop) cudaEventDestroy(stop);
        if (outError) *outError = "cudaEventCreate failed for explaino benchmark";
        return false;
    }

    if (cudaEventRecord(start) != cudaSuccess) {
        cudaEventDestroy(start);
        cudaEventDestroy(stop);
        if (outError) *outError = "cudaEventRecord(start) failed for explaino benchmark";
        return false;
    }

    if (!SampleOnce(coords, view, params, render, outResults, outError)) {
        cudaEventDestroy(start);
        cudaEventDestroy(stop);
        return false;
    }

    if (cudaEventRecord(stop) != cudaSuccess || cudaEventSynchronize(stop) != cudaSuccess) {
        cudaEventDestroy(start);
        cudaEventDestroy(stop);
        if (outError) *outError = "cudaEvent stop/sync failed for explaino benchmark";
        return false;
    }

    float elapsedMs = 0.0f;
    if (cudaEventElapsedTime(&elapsedMs, start, stop) != cudaSuccess) {
        cudaEventDestroy(start);
        cudaEventDestroy(stop);
        if (outError) *outError = "cudaEventElapsedTime failed for explaino benchmark";
        return false;
    }

    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    *outMs = static_cast<double>(elapsedMs);
    return true;
}

bool RunBenchmarkCase(const ExplainoVariantBenchmarkCase& benchmarkCase,
    ExplainoVariantBenchmarkResult* outResult,
    const char** outError) {
    if (!outResult) {
        if (outError) *outError = "benchmark result pointer is null";
        return false;
    }

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    if (!BuildExplainoVariantBenchmarkState(benchmarkCase, kWidth, kHeight, &view, &params, &render, outError)) {
        return false;
    }

    std::vector<Double2> coords;
    BuildGridCoordinates(view, render.resolution.x, render.resolution.y, &coords);
    std::vector<FractalSampleResult> results;

    for (int warmup = 0; warmup < kWarmupRuns; ++warmup) {
        if (!SampleOnce(coords, view, params, render, &results, outError)) {
            return false;
        }
    }

    AggregateMetrics metrics{};
    double sumMs = 0.0;
    double minMs = std::numeric_limits<double>::max();
    double maxMs = 0.0;
    for (int run = 0; run < kTimedRuns; ++run) {
        double elapsedMs = 0.0;
        if (!MeasureSampleMs(coords, view, params, render, &results, &elapsedMs, outError)) {
            return false;
        }
        sumMs += elapsedMs;
        minMs = std::min(minMs, elapsedMs);
        maxMs = std::max(maxMs, elapsedMs);
        AccumulateMetrics(results, &metrics);
    }

    outResult->case_id = benchmarkCase.case_id;
    outResult->fractal_type = benchmarkCase.fractal_type;
    outResult->param_name = benchmarkCase.param_name;
    outResult->param_value = benchmarkCase.param_value;
    outResult->zero_axis = benchmarkCase.zero_axis;
    outResult->sample_tier = render.sample_tier;
    outResult->resolved_eval = render.resolved_eval;
    outResult->width = render.resolution.x;
    outResult->height = render.resolution.y;
    outResult->repeats = kTimedRuns;
    outResult->avg_ms = sumMs / static_cast<double>(kTimedRuns);
    outResult->min_ms = minMs;
    outResult->max_ms = maxMs;
    outResult->gpu_ms_per_1m = outResult->avg_ms * (1000000.0 / static_cast<double>(coords.size()));
    outResult->avg_iters = metrics.total_points > 0 ? metrics.total_iterations / static_cast<double>(metrics.total_points) : 0.0;
    outResult->avg_converged_iters = metrics.converged_points > 0 ? metrics.total_converged_iterations / static_cast<double>(metrics.converged_points) : 0.0;
    outResult->converged_fraction = metrics.total_points > 0 ? static_cast<double>(metrics.converged_points) / static_cast<double>(metrics.total_points) : 0.0;
    return true;
}

} // namespace

int main(int argc, char** argv) {
    const char* csvPath = (argc >= 2 && argv[1]) ? argv[1] : nullptr;

    std::size_t caseCount = 0;
    const ExplainoVariantBenchmarkCase* cases = GetExplainoVariantBenchmarkCases(&caseCount);
    if (!cases || caseCount == 0) {
        std::cerr << "No explaino variant benchmark cases available.\n";
        return 1;
    }

    std::vector<ExplainoVariantBenchmarkResult> results(caseCount);
    const char* error = nullptr;
    for (std::size_t index = 0; index < caseCount; ++index) {
        if (!RunBenchmarkCase(cases[index], &results[index], &error)) {
            std::cerr << "Benchmark failed for " << (cases[index].case_id ? cases[index].case_id : "unknown")
                      << ": " << (error ? error : "unknown") << "\n";
            CleanupFractalSampleCore();
            return 1;
        }
        std::cout << std::fixed << std::setprecision(3)
                  << results[index].case_id
                  << " param=" << results[index].param_value
                  << " avg_ms=" << results[index].avg_ms
                  << " gpu_ms_per_1m=" << results[index].gpu_ms_per_1m
                  << " avg_iters=" << results[index].avg_iters
                  << " conv_frac=" << results[index].converged_fraction
                  << " resolved=" << NumericBackendIdLocal(results[index].resolved_eval.backend)
                  << "/" << IterationStrategyIdLocal(results[index].resolved_eval.strategy)
                  << "\n";
    }

    WriteExplainoVariantBenchmarkCsv(csvPath, results.data(), results.size());
    if (csvPath && csvPath[0] != '\0') {
        std::cout << "CSV: " << csvPath << "\n";
    }

    CleanupFractalSampleCore();
    return 0;
}