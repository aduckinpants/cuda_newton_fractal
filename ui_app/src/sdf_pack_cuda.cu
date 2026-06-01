#include "sdf_pack_cuda.h"

#include "sdf_pack_runtime_eval.cuh"

#include <cuda_runtime.h>

namespace {

constexpr int kThreadsPerBlock = 256;

struct SdfPackGridBuffers {
    float* distances{nullptr};
    int* error_code{nullptr};
    size_t distance_capacity{0};

    bool Allocate(size_t distanceBytes) {
        const bool ok = cudaMalloc(&distances, distanceBytes) == cudaSuccess &&
            cudaMalloc(&error_code, sizeof(int)) == cudaSuccess;
        if (!ok) {
            Free();
            return false;
        }
        distance_capacity = distanceBytes;
        return true;
    }

    bool EnsureCapacity(size_t distanceBytes) {
        if (distances && error_code && distance_capacity >= distanceBytes) {
            return true;
        }
        Free();
        return Allocate(distanceBytes);
    }

    void Free() {
        cudaFree(distances);
        cudaFree(error_code);
        distances = nullptr;
        error_code = nullptr;
        distance_capacity = 0;
    }

    ~SdfPackGridBuffers() {
        Free();
    }
};

__device__ void SetFirstSdfPackGridError(int* errorCode, int code) {
    if (code != SDF_PACK_EVAL_OK) {
        atomicCAS(errorCode, SDF_PACK_EVAL_OK, code);
    }
}

__global__ void sdf_pack_grid_field_kernel(
    int width,
    int height,
    double centerX,
    double centerY,
    double halfWidth,
    double halfHeight,
    double pixelScale,
    SdfPackRuntimeDesc desc,
    float* outSignedDistancePx,
    int* outErrorCode) {
    const int idx = static_cast<int>(blockIdx.x * blockDim.x + threadIdx.x);
    const int count = width * height;
    if (idx >= count) return;

    const int x = idx % width;
    const int y = idx / width;
    const double u = ((static_cast<double>(x) + 0.5) / static_cast<double>(width)) * 2.0 - 1.0;
    const double v = ((static_cast<double>(y) + 0.5) / static_cast<double>(height) - 0.5) * 2.0;
    const double worldX = centerX + u * halfWidth;
    const double worldY = centerY + v * halfHeight;

    const SdfPackGpuSample sample = EvaluateSdfPackRuntimeDesc(desc, worldX, worldY);
    if (!sample.ok || sample.error_code != SDF_PACK_EVAL_OK) {
        SetFirstSdfPackGridError(outErrorCode, sample.error_code == SDF_PACK_EVAL_OK
            ? SDF_PACK_EVAL_INVALID_GEOMETRY
            : sample.error_code);
        outSignedDistancePx[idx] = 0.0f;
        return;
    }
    const double distancePx = sample.distance / pixelScale;
    if (!isfinite(distancePx)) {
        SetFirstSdfPackGridError(outErrorCode, SDF_PACK_EVAL_INVALID_GEOMETRY);
        outSignedDistancePx[idx] = 0.0f;
        return;
    }
    outSignedDistancePx[idx] = static_cast<float>(distancePx);
}

} // namespace

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

    const int blocks = (pointCount + kThreadsPerBlock - 1) / kThreadsPerBlock;
    sdf_pack_sample_kernel<<<blocks, kThreadsPerBlock>>>(dPoints, pointCount, desc, dSamples);

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

bool SampleSdfPackGridCuda(
    int width,
    int height,
    double centerX,
    double centerY,
    double halfWidth,
    double halfHeight,
    double pixelScale,
    const SdfPackRuntimeDesc& desc,
    float* outSignedDistancePx,
    const char** outError) {
    if (outError) *outError = nullptr;
    if (width <= 0 || height <= 0) {
        if (outError) *outError = "SDF pack grid dimensions must be positive";
        return false;
    }
    const size_t count = static_cast<size_t>(width) * static_cast<size_t>(height);
    if (count == 0 || count > static_cast<size_t>(0x7fffffff)) {
        if (outError) *outError = "SDF pack grid dimensions are too large";
        return false;
    }
    if (!isfinite(centerX) || !isfinite(centerY) ||
        !isfinite(halfWidth) || !isfinite(halfHeight) ||
        !isfinite(pixelScale) || halfWidth <= 0.0 || halfHeight <= 0.0 || pixelScale <= 0.0) {
        if (outError) *outError = "SDF pack grid geometry is invalid";
        return false;
    }
    if (!sdf_pack_desc_is_valid(desc)) {
        if (outError) *outError = "invalid SDF pack runtime descriptor";
        return false;
    }
    if (!outSignedDistancePx) {
        if (outError) *outError = "outSignedDistancePx is null";
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

    const size_t distanceBytes = count * sizeof(float);
    static SdfPackGridBuffers buffers;
    if (!buffers.EnsureCapacity(distanceBytes)) {
        if (outError) *outError = "cudaMalloc failed for SDF pack grid field";
        return false;
    }
    if (cudaMemset(buffers.error_code, 0, sizeof(int)) != cudaSuccess) {
        if (outError) *outError = "cudaMemset failed for SDF pack grid error code";
        return false;
    }

    const int blocks = static_cast<int>((count + kThreadsPerBlock - 1) / kThreadsPerBlock);
    sdf_pack_grid_field_kernel<<<blocks, kThreadsPerBlock>>>(
        width,
        height,
        centerX,
        centerY,
        halfWidth,
        halfHeight,
        pixelScale,
        desc,
        buffers.distances,
        buffers.error_code);
    cudaError_t launchErr = cudaGetLastError();
    if (launchErr != cudaSuccess) {
        if (outError) *outError = "CUDA SDF pack grid kernel launch failed";
        return false;
    }
    cudaError_t syncErr = cudaDeviceSynchronize();
    if (syncErr != cudaSuccess) {
        if (outError) *outError = "CUDA SDF pack grid kernel execution failed";
        return false;
    }

    int errorCode = SDF_PACK_EVAL_OK;
    if (cudaMemcpy(&errorCode, buffers.error_code, sizeof(int), cudaMemcpyDeviceToHost) != cudaSuccess) {
        if (outError) *outError = "cudaMemcpy failed for SDF pack grid error code";
        return false;
    }
    if (errorCode != SDF_PACK_EVAL_OK) {
        if (outError) *outError = "SDF pack grid CUDA sample returned a per-point error";
        return false;
    }
    if (cudaMemcpy(outSignedDistancePx, buffers.distances, distanceBytes, cudaMemcpyDeviceToHost) != cudaSuccess) {
        if (outError) *outError = "cudaMemcpy failed for SDF pack grid distances";
        return false;
    }
    return true;
}
