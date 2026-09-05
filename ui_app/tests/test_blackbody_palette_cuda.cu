#include "../src/escape_time_coloring.h"

#include <cuda_runtime.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <utility>

namespace {

struct BlackbodyPaletteSampleCase {
    float signal;
    ColorPipelinePaletteRuntimeParams params;
};

__global__ void SampleBlackbodyPaletteKernel(
    const BlackbodyPaletteSampleCase* cases,
    BlackbodyPaletteLutRgb* outputs,
    int count) {
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < count) {
        outputs[index] = SampleBlackbodyPaletteRuntime(cases[index].signal, cases[index].params);
    }
}

__global__ void BenchmarkPaletteKernel(
    float* output,
    int count,
    int mode,
    ColorPipelineShapeRuntimeParams shapeParams,
    ColorPipelinePaletteRuntimeParams paletteParams) {
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index >= count) return;
    const float signal = fmodf(static_cast<float>(index) * 0.61803398875f, 8.0f);
    EscapeTimeColorRgb rgb{};
    if (mode == 0) {
        rgb = SampleEscapeHeatmap(signal, 1.0f);
    } else {
        const float shaped = ApplyColorPipelineShapeRowValue(
            signal,
            ColorPipelineShape::mirror_repeat,
            shapeParams,
            1.0f);
        const BlackbodyPaletteLutRgb sampled = SampleBlackbodyPaletteRuntime(shaped, paletteParams);
        rgb = {sampled.r, sampled.g, sampled.b};
    }
    output[index] = rgb.r + rgb.g * 0.5f + rgb.b * 0.25f;
}

bool CheckCuda(cudaError_t status, const char* operation) {
    if (status == cudaSuccess) return true;
    std::cerr << operation << " failed: " << cudaGetErrorString(status) << "\n";
    return false;
}

float MaxChannelError(BlackbodyPaletteLutRgb left, BlackbodyPaletteLutRgb right) {
    return std::max({
        std::fabs(left.r - right.r),
        std::fabs(left.g - right.g),
        std::fabs(left.b - right.b),
    });
}

constexpr const char* kBlackbodyPaletteLutIncSha256 =
    "c143b7ff3b6df15282a4a50b53df1d2e8ce769d973c03a6754ef49619af08bab";

struct HostSampleStripCase {
    const char* id;
    float temperature0K;
    float temperature1K;
    BlackbodyTemperatureMapping mapping;
};

const char* MappingId(BlackbodyTemperatureMapping mapping) {
    return mapping == BlackbodyTemperatureMapping::reciprocal_temperature
        ? "reciprocal_temperature"
        : "linear_kelvin";
}

bool EmitHostSamples(const char* outputPath) {
    constexpr int kSampleCount = 512;
    const HostSampleStripCase strips[] = {
        {"forward_reciprocal", 1600.0f, 12000.0f, BlackbodyTemperatureMapping::reciprocal_temperature},
        {"forward_linear", 1600.0f, 12000.0f, BlackbodyTemperatureMapping::linear_kelvin},
        {"reverse_reciprocal", 12000.0f, 1600.0f, BlackbodyTemperatureMapping::reciprocal_temperature},
        {"constant_1600", 1600.0f, 1600.0f, BlackbodyTemperatureMapping::reciprocal_temperature},
    };
    std::ofstream output(outputPath, std::ios::binary | std::ios::trunc);
    if (!output) {
        std::cerr << "Could not open host sample output: " << outputPath << "\n";
        return false;
    }
    output << std::setprecision(9);
    output << "{\n"
           << "  \"schema_id\": \"viewer.blackbody_host_runtime_samples.v1\",\n"
           << "  \"sampler_id\": \"SampleBlackbodyPaletteRuntime\",\n"
           << "  \"lut_entry_count\": " << BLACKBODY_PALETTE_LUT_V1_ENTRY_COUNT << ",\n"
           << "  \"compiled_lut_inc_sha256\": \"" << kBlackbodyPaletteLutIncSha256 << "\",\n"
           << "  \"sample_count\": " << kSampleCount << ",\n"
           << "  \"strips\": [\n";
    for (std::size_t stripIndex = 0; stripIndex < std::size(strips); ++stripIndex) {
        const HostSampleStripCase& strip = strips[stripIndex];
        ColorPipelinePaletteRuntimeParams params{};
        params.blackbody_temperature_0_k = strip.temperature0K;
        params.blackbody_temperature_1_k = strip.temperature1K;
        params.blackbody_temperature_mapping = strip.mapping;
        RebuildBlackbodyPaletteRuntimeCache(&params);
        output << "    {\n"
               << "      \"id\": \"" << strip.id << "\",\n"
               << "      \"temperature_0_k\": " << strip.temperature0K << ",\n"
               << "      \"temperature_1_k\": " << strip.temperature1K << ",\n"
               << "      \"mapping\": \"" << MappingId(strip.mapping) << "\",\n"
               << "      \"lut_x0\": " << params.blackbody_lut_x0 << ",\n"
               << "      \"lut_x1\": " << params.blackbody_lut_x1 << ",\n"
               << "      \"lut_dx\": " << params.blackbody_lut_dx << ",\n"
               << "      \"samples\": [\n";
        for (int sampleIndex = 0; sampleIndex < kSampleCount; ++sampleIndex) {
            const float signal = static_cast<float>(sampleIndex) / static_cast<float>(kSampleCount - 1);
            const BlackbodyPaletteLutRgb rgb = SampleBlackbodyPaletteRuntime(signal, params);
            output << "        [" << rgb.r << ", " << rgb.g << ", " << rgb.b << "]";
            output << (sampleIndex + 1 == kSampleCount ? "\n" : ",\n");
        }
        output << "      ]\n"
               << "    }" << (stripIndex + 1 == std::size(strips) ? "\n" : ",\n");
    }
    output << "  ]\n}\n";
    return static_cast<bool>(output);
}

double Median(std::vector<double> values) {
    std::sort(values.begin(), values.end());
    const std::size_t middle = values.size() / 2;
    return values.size() % 2 == 0
        ? 0.5 * (values[middle - 1] + values[middle])
        : values[middle];
}

double MedianAbsoluteDeviation(const std::vector<double>& values) {
    const double median = Median(values);
    std::vector<double> deviations;
    deviations.reserve(values.size());
    for (double value : values) {
        deviations.push_back(std::fabs(value - median));
    }
    return Median(std::move(deviations));
}

bool RunTimedPalette(
    float* output,
    int pixelCount,
    int mode,
    const ColorPipelineShapeRuntimeParams& shapeParams,
    const ColorPipelinePaletteRuntimeParams& paletteParams,
    int repeatCount,
    cudaEvent_t start,
    cudaEvent_t stop,
    double* outMilliseconds) {
    const int threads = 256;
    const int blocks = (pixelCount + threads - 1) / threads;
    if (!CheckCuda(cudaEventRecord(start), "cudaEventRecord start")) return false;
    for (int repeat = 0; repeat < repeatCount; ++repeat) {
        BenchmarkPaletteKernel<<<blocks, threads>>>(
            output,
            pixelCount,
            mode,
            shapeParams,
            paletteParams);
    }
    if (!CheckCuda(cudaGetLastError(), "BenchmarkPaletteKernel launch") ||
        !CheckCuda(cudaEventRecord(stop), "cudaEventRecord stop") ||
        !CheckCuda(cudaEventSynchronize(stop), "cudaEventSynchronize stop")) {
        return false;
    }
    float elapsedMilliseconds = 0.0f;
    if (!CheckCuda(cudaEventElapsedTime(&elapsedMilliseconds, start, stop), "cudaEventElapsedTime")) {
        return false;
    }
    *outMilliseconds = static_cast<double>(elapsedMilliseconds) / static_cast<double>(repeatCount);
    return true;
}

struct PaletteBenchmarkResult {
    int width{0};
    int height{0};
    std::vector<double> heatmap;
    std::vector<double> blackbody;
    std::vector<double> pairedDelta;
    double heatmapMedian{0.0};
    double blackbodyMedian{0.0};
    double pairedMedian{0.0};
    double pairedMad{0.0};
    double heatmapDrift{0.0};
    double heatmapDriftLimit{0.0};
    double overheadBudget{0.0};
    double madLimit{0.0};
    bool proven{false};
};

bool RunPaletteBenchmarkSize(int width, int height, PaletteBenchmarkResult* result) {
    constexpr int kWarmupPairs = 5;
    constexpr int kMeasuredPairs = 20;
    constexpr int kKernelRepeats = 32;
    const int pixelCount = width * height;
    float* output = nullptr;
    cudaEvent_t start = nullptr;
    cudaEvent_t stop = nullptr;
    if (!CheckCuda(cudaMalloc(&output, static_cast<std::size_t>(pixelCount) * sizeof(float)), "cudaMalloc benchmark output") ||
        !CheckCuda(cudaEventCreate(&start), "cudaEventCreate start") ||
        !CheckCuda(cudaEventCreate(&stop), "cudaEventCreate stop")) {
        if (stop) cudaEventDestroy(stop);
        if (start) cudaEventDestroy(start);
        cudaFree(output);
        return false;
    }

    ColorPipelineShapeRuntimeParams shapeParams{};
    shapeParams.repeat_frequency = 4.0f;
    shapeParams.repeat_phase = 0.0f;
    ColorPipelinePaletteRuntimeParams paletteParams{};
    paletteParams.blackbody_temperature_0_k = 1600.0f;
    paletteParams.blackbody_temperature_1_k = 12000.0f;
    paletteParams.blackbody_temperature_mapping = BlackbodyTemperatureMapping::reciprocal_temperature;
    RebuildBlackbodyPaletteRuntimeCache(&paletteParams);

    auto runPair = [&](int pairIndex, bool record) {
        double timing[2]{};
        const int firstMode = pairIndex % 2 == 0 ? 0 : 1;
        const int secondMode = 1 - firstMode;
        if (!RunTimedPalette(output, pixelCount, firstMode, shapeParams, paletteParams, kKernelRepeats, start, stop, &timing[firstMode]) ||
            !RunTimedPalette(output, pixelCount, secondMode, shapeParams, paletteParams, kKernelRepeats, start, stop, &timing[secondMode])) {
            return false;
        }
        if (record) {
            result->heatmap.push_back(timing[0]);
            result->blackbody.push_back(timing[1]);
            result->pairedDelta.push_back(timing[1] - timing[0]);
        }
        return true;
    };

    bool ok = true;
    for (int pairIndex = 0; pairIndex < kWarmupPairs && ok; ++pairIndex) {
        ok = runPair(pairIndex, false);
    }
    for (int pairIndex = 0; pairIndex < kMeasuredPairs && ok; ++pairIndex) {
        ok = runPair(pairIndex, true);
    }
    cudaEventDestroy(stop);
    cudaEventDestroy(start);
    cudaFree(output);
    if (!ok) return false;

    result->width = width;
    result->height = height;
    result->heatmapMedian = Median(result->heatmap);
    result->blackbodyMedian = Median(result->blackbody);
    result->pairedMedian = Median(result->pairedDelta);
    result->pairedMad = MedianAbsoluteDeviation(result->pairedDelta);
    result->heatmapDrift = Median(std::vector<double>(result->heatmap.end() - 5, result->heatmap.end())) -
        Median(std::vector<double>(result->heatmap.begin(), result->heatmap.begin() + 5));
    result->heatmapDriftLimit = std::max(0.05 * Median(std::vector<double>(result->heatmap.begin(), result->heatmap.begin() + 5)), 0.25);
    result->overheadBudget = width == 1024 ? 1.5 : 6.0;
    result->madLimit = std::max(0.25, 0.20 * result->overheadBudget);
    result->proven =
        std::fabs(result->heatmapDrift) <= result->heatmapDriftLimit &&
        result->pairedMedian <= result->overheadBudget &&
        result->pairedMad <= result->madLimit;
    return true;
}

void WriteDoubleArray(std::ostream& output, const std::vector<double>& values) {
    output << "[";
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index > 0) output << ", ";
        output << values[index];
    }
    output << "]";
}

bool EmitCudaBenchmark(const char* outputPath) {
    int deviceCount = 0;
    if (!CheckCuda(cudaGetDeviceCount(&deviceCount), "cudaGetDeviceCount") || deviceCount < 1) return false;
    cudaDeviceProp device{};
    int driverVersion = 0;
    int runtimeVersion = 0;
    if (!CheckCuda(cudaGetDeviceProperties(&device, 0), "cudaGetDeviceProperties") ||
        !CheckCuda(cudaDriverGetVersion(&driverVersion), "cudaDriverGetVersion") ||
        !CheckCuda(cudaRuntimeGetVersion(&runtimeVersion), "cudaRuntimeGetVersion")) {
        return false;
    }
    PaletteBenchmarkResult results[2];
    if (!RunPaletteBenchmarkSize(1024, 768, &results[0]) ||
        !RunPaletteBenchmarkSize(2048, 1536, &results[1])) {
        return false;
    }
    const bool proven = results[0].proven && results[1].proven;
    std::ofstream output(outputPath, std::ios::binary | std::ios::trunc);
    if (!output) return false;
    output << std::setprecision(12);
    output << "{\n  \"schema_id\": \"viewer.blackbody_palette_interleaved_performance.v1\",\n"
           << "  \"classification\": \"" << (proven ? "proven" : "unproven") << "\",\n"
           << "  \"authority\": \"cuda_event_actual_device_palette_evaluator\",\n"
           << "  \"claim\": \"bounded incremental device color-path cost only; no viewer FPS or performance improvement claim\",\n"
           << "  \"device\": {\"name\": \"" << device.name << "\", \"driver_version\": " << driverVersion
           << ", \"runtime_version\": " << runtimeVersion << "},\n"
           << "  \"warmup_pairs\": 5,\n  \"measured_pairs\": 20,\n  \"kernel_repeats_per_sample\": 32,\n"
           << "  \"sizes\": [\n";
    for (int resultIndex = 0; resultIndex < 2; ++resultIndex) {
        const PaletteBenchmarkResult& item = results[resultIndex];
        output << "    {\n      \"resolution\": [" << item.width << ", " << item.height << "],\n"
               << "      \"classification\": \"" << (item.proven ? "proven" : "unproven") << "\",\n"
               << "      \"heatmap_median_ms\": " << item.heatmapMedian << ",\n"
               << "      \"blackbody_median_ms\": " << item.blackbodyMedian << ",\n"
               << "      \"paired_median_overhead_ms\": " << item.pairedMedian << ",\n"
               << "      \"paired_mad_ms\": " << item.pairedMad << ",\n"
               << "      \"heatmap_drift_ms\": " << item.heatmapDrift << ",\n"
               << "      \"heatmap_drift_limit_ms\": " << item.heatmapDriftLimit << ",\n"
               << "      \"overhead_budget_ms\": " << item.overheadBudget << ",\n"
               << "      \"mad_limit_ms\": " << item.madLimit << ",\n"
               << "      \"heatmap_samples_ms\": ";
        WriteDoubleArray(output, item.heatmap);
        output << ",\n      \"blackbody_samples_ms\": ";
        WriteDoubleArray(output, item.blackbody);
        output << ",\n      \"paired_delta_samples_ms\": ";
        WriteDoubleArray(output, item.pairedDelta);
        output << "\n    }" << (resultIndex == 1 ? "\n" : ",\n");
    }
    output << "  ]\n}\n";
    return static_cast<bool>(output) && proven;
}

} // namespace

int main(int argc, char** argv) {
    if (argc == 3 && std::string(argv[1]) == "--emit-host-samples") {
        return EmitHostSamples(argv[2]) ? 0 : 1;
    }
    if (argc == 3 && std::string(argv[1]) == "--benchmark-json") {
        return EmitCudaBenchmark(argv[2]) ? 0 : 1;
    }
    if (argc != 1) {
        std::cerr << "Usage: test_blackbody_palette_cuda.exe [--emit-host-samples OUTPUT_JSON | --benchmark-json OUTPUT_JSON]\n";
        return 2;
    }
    int deviceCount = 0;
    if (!CheckCuda(cudaGetDeviceCount(&deviceCount), "cudaGetDeviceCount") || deviceCount < 1) {
        std::cerr << "Black Body CUDA parity requires a CUDA device\n";
        return 1;
    }

    const float endpointPairs[][2] = {
        {1600.0f, 12000.0f},
        {12000.0f, 1600.0f},
        {6500.0f, 6500.0f},
        {800.0f, 40000.0f},
        {40000.0f, 800.0f},
        {2400.0f, 18000.0f},
    };
    const float signals[] = {
        -1.0f,
        0.0f,
        0.0001f,
        0.125f,
        0.33333334f,
        0.5f,
        0.731f,
        0.875f,
        0.9999f,
        1.0f,
        2.0f,
        std::numeric_limits<float>::quiet_NaN(),
    };

    std::vector<BlackbodyPaletteSampleCase> cases;
    for (const auto& endpoints : endpointPairs) {
        for (const BlackbodyTemperatureMapping mapping : {
                 BlackbodyTemperatureMapping::reciprocal_temperature,
                 BlackbodyTemperatureMapping::linear_kelvin}) {
            ColorPipelinePaletteRuntimeParams params{};
            params.blackbody_temperature_0_k = endpoints[0];
            params.blackbody_temperature_1_k = endpoints[1];
            params.blackbody_temperature_mapping = mapping;
            RebuildBlackbodyPaletteRuntimeCache(&params);
            for (float signal : signals) {
                cases.push_back({signal, params});
            }
        }
    }

    std::vector<BlackbodyPaletteLutRgb> expected(cases.size());
    for (std::size_t index = 0; index < cases.size(); ++index) {
        expected[index] = SampleBlackbodyPaletteRuntime(cases[index].signal, cases[index].params);
    }

    BlackbodyPaletteSampleCase* deviceCases = nullptr;
    BlackbodyPaletteLutRgb* deviceOutputs = nullptr;
    const std::size_t casesBytes = cases.size() * sizeof(BlackbodyPaletteSampleCase);
    const std::size_t outputsBytes = expected.size() * sizeof(BlackbodyPaletteLutRgb);
    if (!CheckCuda(cudaMalloc(&deviceCases, casesBytes), "cudaMalloc cases") ||
        !CheckCuda(cudaMalloc(&deviceOutputs, outputsBytes), "cudaMalloc outputs") ||
        !CheckCuda(cudaMemcpy(deviceCases, cases.data(), casesBytes, cudaMemcpyHostToDevice), "cudaMemcpy cases")) {
        cudaFree(deviceOutputs);
        cudaFree(deviceCases);
        return 1;
    }

    const int threads = 128;
    const int blocks = (static_cast<int>(cases.size()) + threads - 1) / threads;
    SampleBlackbodyPaletteKernel<<<blocks, threads>>>(deviceCases, deviceOutputs, static_cast<int>(cases.size()));
    if (!CheckCuda(cudaGetLastError(), "SampleBlackbodyPaletteKernel launch") ||
        !CheckCuda(cudaDeviceSynchronize(), "SampleBlackbodyPaletteKernel synchronize")) {
        cudaFree(deviceOutputs);
        cudaFree(deviceCases);
        return 1;
    }

    std::vector<BlackbodyPaletteLutRgb> actual(expected.size());
    if (!CheckCuda(cudaMemcpy(actual.data(), deviceOutputs, outputsBytes, cudaMemcpyDeviceToHost), "cudaMemcpy outputs")) {
        cudaFree(deviceOutputs);
        cudaFree(deviceCases);
        return 1;
    }
    cudaFree(deviceOutputs);
    cudaFree(deviceCases);

    float maxError = 0.0f;
    std::size_t maxErrorIndex = 0;
    for (std::size_t index = 0; index < actual.size(); ++index) {
        const float error = MaxChannelError(expected[index], actual[index]);
        if (error > maxError) {
            maxError = error;
            maxErrorIndex = index;
        }
    }
    if (!(maxError <= 2.0e-6f)) {
        std::cerr << "Black Body host/device max channel error " << maxError
                  << " exceeded 2e-6 at case " << maxErrorIndex << "\n";
        return 1;
    }

    std::cout << "test_blackbody_palette_cuda: cases=" << cases.size()
              << " max_channel_error=" << maxError << "\n";
    return 0;
}
