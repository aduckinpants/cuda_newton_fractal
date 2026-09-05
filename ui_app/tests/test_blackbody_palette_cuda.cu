#include "../src/blackbody_palette_lut.h"

#include <cuda_runtime.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

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

} // namespace

int main() {
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
