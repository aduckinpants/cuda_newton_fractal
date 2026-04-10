// fractal_sample_core.cu — Standalone CUDA compilation unit for the sample path.
// Contains: fractal_sample_device(), fractal_sample_kernel<<<>>>(), SampleFractalPoints().
// Linkable independently from fractal_renderer.cu (no DX11 / ImGui / framebuffer deps).

#include "fractal_types.h"
#include "fractal_family_rules.h"
#include "fractal_sample_result.h"
#include "fractal_device_math.cuh"
#include "basin_coloring.h"
#include "escape_time_direct_formulas.h"
#include "explaino_collatz_formulas.h"
#include "escape_time_specialized_formulas.h"
#include "fractal_runtime_validation.h"
#include "explaino_seed_curve.h"
#include "perturbation_reference_orbit.h"
#include "sample_tier_resolver.h"

#include <algorithm>
#include <cmath>
#include <vector>

// --- Device function: computes one fractal sample at a given coordinate.
__device__ FractalSampleResult fractal_sample_device(
    Cx coord, Cxd coordD, ViewState view, KernelParams params,
    RenderSettings render, const double2* refOrbit, int refLen, double2 refC0)
{
#include "fractal_sample_device.inl"
}

// K2: Thin launch wrapper — samples arbitrary complex-plane coordinates.
__global__ void fractal_sample_kernel(
    const double2* coords,
    int numPoints,
    FractalSampleResult* results,
    ViewState view,
    KernelParams params,
    RenderSettings render,
    const double2* refOrbit,
    int refLen,
    double2 refC0)
{
    int idx = (int)(blockIdx.x * blockDim.x + threadIdx.x);
    if (idx >= numPoints) return;

    double2 c = coords[idx];
    Cx coord{(float)c.x, (float)c.y};
    Cxd coordD{c.x, c.y};

    results[idx] = fractal_sample_device(coord, coordD, view, params, render, refOrbit, refLen, refC0);
}

// Self-contained reference orbit cache for the sample path.
// Separate from the renderer's g_cached to allow independent linking.
namespace {

struct SampleCoreCache {
    double2* d_refOrbit = nullptr;
    int refOrbitLen = 0;
    std::vector<double2> hostRefOrbit;
    bool hostRefValid = false;
    PerturbationReferenceOrbitCacheKey hostRefKey{};
};

SampleCoreCache g_sampleCache;

bool ensure_sample_ref_orbit(int len, const char** outError) {
    if (len <= 0) {
        if (outError) *outError = "Invalid ref orbit length";
        return false;
    }
    if (g_sampleCache.d_refOrbit && g_sampleCache.refOrbitLen == len) return true;

    if (g_sampleCache.d_refOrbit) cudaFree(g_sampleCache.d_refOrbit);
    g_sampleCache.d_refOrbit = nullptr;
    g_sampleCache.refOrbitLen = 0;

    if (cudaMalloc(&g_sampleCache.d_refOrbit, (size_t)len * sizeof(double2)) != cudaSuccess) {
        if (outError) *outError = "cudaMalloc failed for sample ref orbit";
        return false;
    }
    g_sampleCache.refOrbitLen = len;
    return true;
}

} // namespace

bool SampleFractalPoints(
    const Double2* coords,
    int numPoints,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    FractalSampleResult* outResults,
    const char** outError)
{
    if (outError) *outError = nullptr;

    // Zero points is a valid no-op.
    if (numPoints <= 0) return true;

    if (!coords) {
        if (outError) *outError = "coords is null";
        return false;
    }
    if (!outResults) {
        if (outError) *outError = "outResults is null";
        return false;
    }

    // Fail-fast validation (no implicit fallback/repair).
    if (!ValidateFractalRuntimeState(view, params, outError)) return false;

    int deviceCount = 0;
    cudaGetDeviceCount(&deviceCount);
    int dev = render.device_id;
    if (deviceCount > 0) {
        if (dev < 0) dev = 0;
        if (dev >= deviceCount) dev = deviceCount - 1;
        cudaSetDevice(dev);
    }

    // Resolve the sample tier into a concrete (backend, strategy) pair.
    RenderSettings resolvedRender = render;
    resolvedRender.resolved_eval = ResolveSampleEvalMode(
        view.fractal_type, render.sample_tier, view.log2_zoom);

    // Handle perturbation reference orbit if needed.
    const PerturbationReferenceOrbitRequest perturbRequest = ResolvePerturbationReferenceOrbitRequest(view, params);
    const bool wantPerturb = perturbRequest.enabled;
    const double2 refC0 = make_double2(perturbRequest.refCx, perturbRequest.refCy);
    const int refLen = perturbRequest.refLen;
    if (wantPerturb) {
        if (!ensure_sample_ref_orbit(refLen, outError)) return false;
        const bool sameKey = g_sampleCache.hostRefValid && MatchesPerturbationReferenceOrbitCacheKey(g_sampleCache.hostRefKey, perturbRequest);
        if (!sameKey) {
            BuildPerturbationReferenceOrbit(perturbRequest, &g_sampleCache.hostRefOrbit);
            if (cudaMemcpy(g_sampleCache.d_refOrbit, g_sampleCache.hostRefOrbit.data(), (size_t)refLen * sizeof(double2), cudaMemcpyHostToDevice) != cudaSuccess) {
                if (outError) *outError = "cudaMemcpy failed for ref orbit";
                return false;
            }
            g_sampleCache.hostRefValid = true;
            g_sampleCache.hostRefKey = MakePerturbationReferenceOrbitCacheKey(perturbRequest);
        }
    }

    // Allocate device buffers for coordinates and results.
    static_assert(sizeof(Double2) == sizeof(double2), "Double2/double2 layout mismatch");
    double2* d_coords = nullptr;
    FractalSampleResult* d_results = nullptr;
    size_t coordBytes = (size_t)numPoints * sizeof(double2);
    size_t resultBytes = (size_t)numPoints * sizeof(FractalSampleResult);

    if (cudaMalloc(&d_coords, coordBytes) != cudaSuccess) {
        if (outError) *outError = "cudaMalloc failed for sample coords";
        return false;
    }
    if (cudaMalloc(&d_results, resultBytes) != cudaSuccess) {
        cudaFree(d_coords);
        if (outError) *outError = "cudaMalloc failed for sample results";
        return false;
    }

    // Upload coordinates.
    if (cudaMemcpy(d_coords, coords, coordBytes, cudaMemcpyHostToDevice) != cudaSuccess) {
        cudaFree(d_coords);
        cudaFree(d_results);
        if (outError) *outError = "cudaMemcpy failed for sample coords";
        return false;
    }

    // Launch kernel.
    int threadsPerBlock = 256;
    int blocks = (numPoints + threadsPerBlock - 1) / threadsPerBlock;

    fractal_sample_kernel<<<blocks, threadsPerBlock>>>(
        d_coords,
        numPoints,
        d_results,
        view,
        params,
        resolvedRender,
        wantPerturb ? g_sampleCache.d_refOrbit : nullptr,
        refLen,
        refC0);

    cudaError_t launchErr = cudaGetLastError();
    if (launchErr != cudaSuccess) {
        cudaFree(d_coords);
        cudaFree(d_results);
        if (outError) *outError = "CUDA sample kernel launch failed";
        return false;
    }

    cudaDeviceSynchronize();

    // Copy results back.
    if (cudaMemcpy(outResults, d_results, resultBytes, cudaMemcpyDeviceToHost) != cudaSuccess) {
        cudaFree(d_coords);
        cudaFree(d_results);
        if (outError) *outError = "cudaMemcpy failed for sample results";
        return false;
    }

    cudaFree(d_coords);
    cudaFree(d_results);
    return true;
}

void CleanupFractalSampleCore() {
    if (g_sampleCache.d_refOrbit) { cudaFree(g_sampleCache.d_refOrbit); g_sampleCache.d_refOrbit = nullptr; }
    g_sampleCache.refOrbitLen = 0;
    g_sampleCache.hostRefOrbit.clear();
    g_sampleCache.hostRefValid = false;
    g_sampleCache.hostRefKey = {};
}
