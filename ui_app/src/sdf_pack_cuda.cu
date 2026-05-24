#include "sdf_pack_cuda.h"

#include "sdf_pack_runtime_eval.cuh"

#include <cuda_runtime.h>

__global__ void sdf_pack_sample_kernel(
    const SdfPackGpuPoint* points,
    int pointCount,
    SdfPackRuntimeDesc desc,
    SdfPackGpuSample* outSamples) {
    const int idx = static_cast<int>(blockIdx.x * blockDim.x + threadIdx.x);
    if (idx >= pointCount) return;
    const SdfPackGpuPoint p = points[idx];
    outSamples[idx] = EvaluateSdfPackRuntimeDesc(desc, p.x, p.y);
}

bool SampleSdfPackCuda(
    const SdfPackGpuPoint* points,
    int pointCount,
    const SdfPackRuntimeDesc& desc,
    SdfPackGpuSample* outSamples,
    const char** outError) {
    if (outError) *outError = nullptr;
    if (pointCount < 0) {
        if (outError) *outError = "pointCount is negative";
        return false;
    }
    if (!sdf_pack_desc_is_valid(desc)) {
        if (outError) *outError = "invalid SDF pack runtime descriptor";
        return false;
    }
    if (pointCount == 0) return true;
    if (!points) {
        if (outError) *outError = "points is null";
        return false;
    }
    if (!outSamples) {
        if (outError) *outError = "outSamples is null";
        return false;
    }

    int deviceCount = 0;
    cudaError_t deviceCountErr = cudaGetDeviceCount(&deviceCount);
    if (deviceCountErr != cudaSuccess || deviceCount <= 0) {
        if (outError) *outError = "CUDA backend unavailable: no CUDA device";
        return false;
    }
    if (cudaSetDevice(0) != cudaSuccess) {
        if (outError) *outError = "CUDA backend unavailable: failed to select device 0";
        return false;
    }

    SdfPackGpuPoint* dPoints = nullptr;
    SdfPackGpuSample* dSamples = nullptr;
    const size_t pointBytes = static_cast<size_t>(pointCount) * sizeof(SdfPackGpuPoint);
    const size_t sampleBytes = static_cast<size_t>(pointCount) * sizeof(SdfPackGpuSample);

    if (cudaMalloc(&dPoints, pointBytes) != cudaSuccess) {
        if (outError) *outError = "cudaMalloc failed for SDF points";
        return false;
    }
    if (cudaMalloc(&dSamples, sampleBytes) != cudaSuccess) {
        cudaFree(dPoints);
        if (outError) *outError = "cudaMalloc failed for SDF samples";
        return false;
    }
    if (cudaMemcpy(dPoints, points, pointBytes, cudaMemcpyHostToDevice) != cudaSuccess) {
        cudaFree(dPoints);
        cudaFree(dSamples);
        if (outError) *outError = "cudaMemcpy failed for SDF points";
        return false;
    }

    const int threadsPerBlock = 256;
    const int blocks = (pointCount + threadsPerBlock - 1) / threadsPerBlock;
    sdf_pack_sample_kernel<<<blocks, threadsPerBlock>>>(dPoints, pointCount, desc, dSamples);

    cudaError_t launchErr = cudaGetLastError();
    if (launchErr != cudaSuccess) {
        cudaFree(dPoints);
        cudaFree(dSamples);
        if (outError) *outError = "CUDA SDF pack kernel launch failed";
        return false;
    }
    cudaError_t syncErr = cudaDeviceSynchronize();
    if (syncErr != cudaSuccess) {
        cudaFree(dPoints);
        cudaFree(dSamples);
        if (outError) *outError = "CUDA SDF pack kernel execution failed";
        return false;
    }
    if (cudaMemcpy(outSamples, dSamples, sampleBytes, cudaMemcpyDeviceToHost) != cudaSuccess) {
        cudaFree(dPoints);
        cudaFree(dSamples);
        if (outError) *outError = "cudaMemcpy failed for SDF samples";
        return false;
    }

    cudaFree(dPoints);
    cudaFree(dSamples);
    return true;
}
