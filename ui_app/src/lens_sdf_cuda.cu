#include "lens_sdf_cuda.h"

#include <cuda_runtime.h>

#include <cmath>
#include <vector>

namespace {

constexpr int kThreadsPerBlock = 256;
constexpr float kMissingSeedDistance = 1.0e18f;

__device__ int PixelIndex(int x, int y, int width) {
    return y * width + x;
}

__device__ int SquaredDistanceToSeed(int x, int y, int2 seed) {
    if (seed.x < 0 || seed.y < 0) {
        return 0x7fffffff;
    }
    const int dx = x - seed.x;
    const int dy = y - seed.y;
    return dx * dx + dy * dy;
}

__global__ void InitializeSeedsKernel(const unsigned char* mask, int width, int height, int2* insideSeeds, int2* outsideSeeds) {
    const int count = width * height;
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index >= count) {
        return;
    }
    const int x = index % width;
    const int y = index / width;
    const bool inside = mask[index] > 127;
    insideSeeds[index] = inside ? make_int2(x, y) : make_int2(-1, -1);
    outsideSeeds[index] = inside ? make_int2(-1, -1) : make_int2(x, y);
}

__global__ void JumpFloodKernel(const int2* input, int width, int height, int step, int2* output) {
    const int count = width * height;
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index >= count) {
        return;
    }
    const int x = index % width;
    const int y = index / width;

    int2 best = input[index];
    int bestDistance = SquaredDistanceToSeed(x, y, best);
    for (int oy = -1; oy <= 1; ++oy) {
        const int ny = y + oy * step;
        if (ny < 0 || ny >= height) {
            continue;
        }
        for (int ox = -1; ox <= 1; ++ox) {
            const int nx = x + ox * step;
            if (nx < 0 || nx >= width) {
                continue;
            }
            const int2 candidate = input[PixelIndex(nx, ny, width)];
            const int candidateDistance = SquaredDistanceToSeed(x, y, candidate);
            if (candidateDistance < bestDistance) {
                bestDistance = candidateDistance;
                best = candidate;
            }
        }
    }
    output[index] = best;
}

__global__ void BuildSignedDistanceKernel(const int2* insideSeeds, const int2* outsideSeeds, int width, int height, float* signedDistance) {
    const int count = width * height;
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index >= count) {
        return;
    }
    const int x = index % width;
    const int y = index / width;
    const int insideSq = SquaredDistanceToSeed(x, y, insideSeeds[index]);
    const int outsideSq = SquaredDistanceToSeed(x, y, outsideSeeds[index]);
    const float dInside = insideSq == 0x7fffffff ? kMissingSeedDistance : sqrtf(static_cast<float>(insideSq));
    const float dOutside = outsideSq == 0x7fffffff ? kMissingSeedDistance : sqrtf(static_cast<float>(outsideSq));
    signedDistance[index] = dInside - dOutside;
}

struct CudaJfaBuffers {
    unsigned char* deviceMask{nullptr};
    int2* insideA{nullptr};
    int2* insideB{nullptr};
    int2* outsideA{nullptr};
    int2* outsideB{nullptr};
    float* signedDistance{nullptr};

    bool Allocate(size_t maskBytes, size_t seedBytes, size_t distanceBytes) {
        return cudaMalloc(&deviceMask, maskBytes) == cudaSuccess &&
            cudaMalloc(&insideA, seedBytes) == cudaSuccess &&
            cudaMalloc(&insideB, seedBytes) == cudaSuccess &&
            cudaMalloc(&outsideA, seedBytes) == cudaSuccess &&
            cudaMalloc(&outsideB, seedBytes) == cudaSuccess &&
            cudaMalloc(&signedDistance, distanceBytes) == cudaSuccess;
    }

    void Free() {
        cudaFree(deviceMask);
        cudaFree(insideA);
        cudaFree(insideB);
        cudaFree(outsideA);
        cudaFree(outsideB);
        cudaFree(signedDistance);
        deviceMask = nullptr;
        insideA = nullptr;
        insideB = nullptr;
        outsideA = nullptr;
        outsideB = nullptr;
        signedDistance = nullptr;
    }

    ~CudaJfaBuffers() {
        Free();
    }
};

bool RunCudaJfa(const uint8_t* mask, int width, int height, std::vector<float>& outSignedDistance) {
    if (!mask || width <= 0 || height <= 0) {
        return false;
    }
    const size_t count = static_cast<size_t>(width) * static_cast<size_t>(height);
    if (count == 0 || count > static_cast<size_t>(0x7fffffff)) {
        return false;
    }

    const size_t maskBytes = count * sizeof(unsigned char);
    const size_t seedBytes = count * sizeof(int2);
    const size_t distanceBytes = count * sizeof(float);

    CudaJfaBuffers buffers;
    if (!buffers.Allocate(maskBytes, seedBytes, distanceBytes) ||
        cudaMemcpy(buffers.deviceMask, mask, maskBytes, cudaMemcpyHostToDevice) != cudaSuccess) {
        return false;
    }

    const int blocks = static_cast<int>((count + kThreadsPerBlock - 1) / kThreadsPerBlock);
    InitializeSeedsKernel<<<blocks, kThreadsPerBlock>>>(buffers.deviceMask, width, height, buffers.insideA, buffers.outsideA);
    if (cudaGetLastError() != cudaSuccess) {
        return false;
    }

    int2* insideRead = buffers.insideA;
    int2* insideWrite = buffers.insideB;
    int2* outsideRead = buffers.outsideA;
    int2* outsideWrite = buffers.outsideB;
    int step = 1;
    for (const int maxDim = width > height ? width : height; step < maxDim; step <<= 1) {}
    for (step >>= 1; step >= 1; step >>= 1) {
        JumpFloodKernel<<<blocks, kThreadsPerBlock>>>(insideRead, width, height, step, insideWrite);
        if (cudaGetLastError() != cudaSuccess) {
            return false;
        }
        JumpFloodKernel<<<blocks, kThreadsPerBlock>>>(outsideRead, width, height, step, outsideWrite);
        if (cudaGetLastError() != cudaSuccess) {
            return false;
        }
        int2* tmpInside = insideRead;
        insideRead = insideWrite;
        insideWrite = tmpInside;
        int2* tmpOutside = outsideRead;
        outsideRead = outsideWrite;
        outsideWrite = tmpOutside;
    }

    BuildSignedDistanceKernel<<<blocks, kThreadsPerBlock>>>(insideRead, outsideRead, width, height, buffers.signedDistance);
    if (cudaGetLastError() != cudaSuccess || cudaDeviceSynchronize() != cudaSuccess) {
        return false;
    }
    outSignedDistance.assign(count, 0.0f);
    return cudaMemcpy(outSignedDistance.data(), buffers.signedDistance, distanceBytes, cudaMemcpyDeviceToHost) == cudaSuccess;
}

void FillReport(LensSdfBackendReport* report, LensSdfBackend requested, LensSdfBackend used, bool fallbackUsed) {
    if (!report) {
        return;
    }
    report->requested = requested;
    report->used = used;
    report->fallback_used = fallbackUsed;
}

} // namespace

bool ComputeLensSdfFieldCudaJfa(
    const uint8_t* mask,
    int width,
    int height,
    SdfFieldResult& outField) {
    outField.Clear();
    std::vector<float> signedDistance;
    if (!RunCudaJfa(mask, width, height, signedDistance)) {
        return false;
    }

    outField.width = width;
    outField.height = height;
    outField.pixel_scale = 1.0f;
    outField.sign_convention = SdfSignConvention::negative_inside_positive_outside;
    outField.source_kind = SdfFieldSourceKind::mask_derived;
    outField.signed_distance_px = std::move(signedDistance);
    return true;
}

bool ComputeLensSdfFieldForMaskWithBackend(
    const uint8_t* mask,
    int width,
    int height,
    int downsample,
    LensSdfBackend backend,
    SdfFieldResult& outField,
    LensSdfBackendReport* outReport) {
    outField.Clear();
    FillReport(outReport, backend, LensSdfBackend::cpu_chamfer, false);

    if (backend == LensSdfBackend::cpu_chamfer) {
        const bool ok = ComputeLensSdfFieldForMask(mask, width, height, downsample, outField);
        FillReport(outReport, backend, LensSdfBackend::cpu_chamfer, false);
        return ok;
    }

    std::vector<uint8_t> lowMask;
    int outWidth = 0;
    int outHeight = 0;
    if (!DownsampleMaskPow2(mask, width, height, downsample, lowMask, outWidth, outHeight)) {
        return false;
    }

    if (ComputeLensSdfFieldCudaJfa(lowMask.data(), outWidth, outHeight, outField)) {
        outField.pixel_scale = static_cast<float>(NormalizeLensDownsamplePow2(downsample));
        FillReport(outReport, backend, LensSdfBackend::cuda_jfa, false);
        return true;
    }

    if (backend == LensSdfBackend::auto_backend) {
        const bool ok = ComputeLensSdfFieldForMask(mask, width, height, downsample, outField);
        FillReport(outReport, backend, LensSdfBackend::cpu_chamfer, ok);
        return ok;
    }

    outField.Clear();
    FillReport(outReport, backend, LensSdfBackend::cuda_jfa, false);
    return false;
}
