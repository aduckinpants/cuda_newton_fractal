#include "fractal_types.h"
#include "fractal_family_rules.h"
#include "fractal_sample_result.h"
#include "fractal_device_math.cuh"
#include "basin_coloring.h"
#include "escape_time_direct_formulas.h"
#include "escape_time_coloring.h"
#include "explaino_collatz_formulas.h"
#include "escape_time_specialized_formulas.h"
#include "fractal_runtime_validation.h"
#include "explaino_seed_curve.h"
#include "perturbation_reference_orbit.h"
#include "polynomial_eval_real_coeffs.h"
#include "sample_tier_resolver.h"

#include <cuda_runtime.h>
#include <math_constants.h>

#include <algorithm>
#include <cmath>
#include <vector>

namespace {

// Device math helpers are now in fractal_device_math.cuh.

// --- Extracted device function: computes one fractal sample at a given coordinate.
// No pixel mapping, no coloring, no framebuffer — pure iteration result.
__device__ FractalSampleResult fractal_sample_device(
    Cx coord, Cxd coordD, ViewState view, KernelParams params,
    RenderSettings render, const double2* refOrbit, int refLen, double2 refC0)
{
#include "fractal_sample_device.inl"
}

__device__ __forceinline__ int ResolveBasinRenderRootCount(FractalType ft, const KernelParams& params, bool* outUseCustomRoots) {
    if (ft == FractalType::counterfactual_pair || ft == FractalType::explaino_counterfactual_pair) {
        if (outUseCustomRoots) *outUseCustomRoots = false;
        return 4;
    }
    if (ft == FractalType::projection_and_flow) {
        if (outUseCustomRoots) *outUseCustomRoots = false;
        const int projectionRootCount =
            params.projection_and_flow_root_family == ProjectionAndFlowRootFamily::quartic_unit_roots ? 4 : 3;
        return projectionRootCount * 4 + 1;
    }
    const int nRoots = ResolvePolynomialRootCount(params.poly_kind);
    const bool useCustomRoots = (nRoots == 0) && IsExplainoFamily(ft) && (params.explaino_root_count > 0);
    if (outUseCustomRoots) *outUseCustomRoots = useCustomRoots;
    return useCustomRoots ? params.explaino_root_count : nRoots;
}

__device__ __forceinline__ int ResolveBasinRenderRootIndex(FractalType ft, Cx z, const KernelParams& params) {
    bool useCustomRoots = false;
    const int rootCount = ResolveBasinRenderRootCount(ft, params, &useCustomRoots);
    if (rootCount <= 0) {
        return -1;
    }
    if (useCustomRoots) {
        return NearestRootIndexList(z, params.explaino_roots, params.explaino_root_count);
    }
    return NearestRootIndexUnitRoots(z, rootCount);
}

__global__ void kernel_render(
    uint32_t* outRGBA,
    uint8_t* outMask,
    int width,
    int height,
    ViewState view,
    KernelParams params,
    RenderSettings render,
    const double2* refOrbit,
    int refLen,
    double2 refC0,
    int* outItersSum)
{
    int px = (int)(blockIdx.x * blockDim.x + threadIdx.x);
    int py = (int)(blockIdx.y * blockDim.y + threadIdx.y);
    if (px >= width || py >= height) return;

    // Use double intermediates for view mapping to reduce precision loss at deep zoom.
    double aspect = (height > 0) ? (double)width / (double)height : 1.0;
    // Authoritative zoom is exp2(log2_zoom). This avoids the float UI surface collapsing at deep zoom.
    double zoom = fmax(1.0e-300, exp2(view.log2_zoom));
    double base = 2.0 / zoom;

    double nx = (((double)px + 0.5) / (double)width - 0.5) * 2.0;
    double ny = (((double)py + 0.5) / (double)height - 0.5) * 2.0;

    double x = (double)view.center_hp_x + nx * base * aspect;
    double y = (double)view.center_hp_y + ny * base;

    // Optional rotation
    if (view.rotation_degrees != 0.0f) {
        double a = (double)view.rotation_degrees * (double)(CUDART_PI_F / 180.0f);
        double cs = cos(a);
        double sn = sin(a);
        double cx = (double)view.center_hp_x;
        double cy = (double)view.center_hp_y;
        double rx = (x - cx) * cs - (y - cy) * sn;
        double ry = (x - cx) * sn + (y - cy) * cs;
        x = cx + rx;
        y = cy + ry;
    }

    Cxd coordD{x, y};
    Cx coord{(float)x, (float)y};

    // K1: Iteration logic extracted to fractal_sample_device().
    FractalSampleResult sample = fractal_sample_device(coord, coordD, view, params, render, refOrbit, refLen, refC0);
    int it = sample.iterations;
    Cx z{sample.final_z_x, sample.final_z_y};
    float pAbs = sample.residual;
    bool converged = sample.converged;
    bool escaped = sample.escaped;
    FractalType ft = view.fractal_type;
    const bool hasExplicitSyntheticClass =
        ft == FractalType::counterfactual_pair || ft == FractalType::explaino_counterfactual_pair ||
        ft == FractalType::projection_and_flow;
    int maxIter = max(1, params.max_iter);


    // Accumulate a rough measure for UI stats
    if (outItersSum) {
        atomicAdd(outItersSum, it);
    }

    ColoringMode mode = params.coloring_mode;
    uchar4 color{0, 0, 0, 255};
    const uchar4 errorColor{255, 0, 255, 255};

    if (SupportsBasinColoring(ft)) {
        // Basin-coloring family branch (Newton + Explaino family).
        if (mode == ColoringMode::joy_basins) {
            if (!converged && !hasExplicitSyntheticClass) {
                // "Submerged" undertow: preserve faint structure, but keep it dark.
                float t = (float)it / (float)maxIter;
                float a = atan2f(z.y, z.x);
                bool isExplainoFamily = IsExplainoFamily(ft);
                float phase = isExplainoFamily ? view.explaino_phase : 0.0f;
                float v = 0.5f + 0.5f * sinf(a * 3.0f + t * 6.2831853f + phase);
                float w = 0.5f + 0.5f * sinf(a * 11.0f + t * 12.5663706f - phase * 0.7f);
                unsigned char r = (unsigned char)(10.0f + 26.0f * v + 8.0f * w);
                unsigned char g = (unsigned char)(8.0f + 20.0f * v + 10.0f * w);
                unsigned char b = (unsigned char)(18.0f + 34.0f * v + 14.0f * w);
                color = {r, g, b, 255};
            } else {
                bool useCustomRoots = false;
                const int rootCount = ResolveBasinRenderRootCount(ft, params, &useCustomRoots);

                bool isExplainoFamily = IsExplainoFamily(ft);

                if (rootCount > 0) {
                    int idx = ResolveBasinRenderRootIndex(ft, z, params);
                    idx = ResolveShapedColorPipelineRootIndex(idx, rootCount, params);
                    uchar4 base = PaletteJoyRoot<uchar4>(idx);

                    // Brightness: faster convergence => brighter, but never gloomy.
                    float u = (float)it / (float)maxIter;
                    float bright = 1.0f - powf(fminf(1.0f, u), 0.65f);
                    bright = 0.35f + 0.90f * bright;

                    float edge = powf(fminf(1.0f, u), 0.85f);
                    float phase = isExplainoFamily ? view.explaino_phase : 0.0f;
                    float a = atan2f(z.y, z.x);
                    float stripe = 0.5f + 0.5f * sinf((float)it * 0.35f + a * 3.0f + phase);

                    uchar4 c0 = EscapeTimeColorMulRgb(base, bright);
                    float glow = 0.30f * edge + 0.12f * stripe;
                    int rr = (int)c0.x + (int)(glow * 255.0f);
                    int gg = (int)c0.y + (int)(glow * 210.0f);
                    int bb = (int)c0.z + (int)(glow * 160.0f);
                    rr = max(0, min(255, rr));
                    gg = max(0, min(255, gg));
                    bb = max(0, min(255, bb));
                    color = {(unsigned char)rr, (unsigned char)gg, (unsigned char)bb, 255};
                } else {
                    // Invalid: basin identity not defined for custom polynomial in this demo.
                    color = errorColor;
                }
            }
        } else if (mode == ColoringMode::root_basin) {
            if (!converged && !hasExplicitSyntheticClass) {
                color = {0, 0, 0, 255};
            } else {
                bool useCustomRoots = false;
                const int rootCount = ResolveBasinRenderRootCount(ft, params, &useCustomRoots);

                if (rootCount > 0) {
                    int idx = ResolveBasinRenderRootIndex(ft, z, params);
                    idx = ResolveShapedColorPipelineRootIndex(idx, rootCount, params);
                    color = PaletteRoot<uchar4>(idx);
                } else {
                    // Invalid: root identity not defined.
                    color = errorColor;
                }
            }
        } else {
            const bool programmableBasinColorable = converged || ft == FractalType::projection_and_flow;
            color = MakeProgrammableBasinColor<uchar4>(
                ft,
                programmableBasinColorable,
                converged,
                it,
                maxIter,
                z,
                pAbs,
                params);
        }
    } else {
        // Escape-time coloring.
        color = MakeEscapeTimeBaseColor<uchar4>(ft, mode, escaped, it, maxIter, z, params);
    }

    color = ApplyFractalColorGrading(color, params);

    uint32_t rgba = (uint32_t)color.x | ((uint32_t)color.y << 8) | ((uint32_t)color.z << 16) | ((uint32_t)color.w << 24);
    outRGBA[py * width + px] = rgba;

    if (outMask) {
        bool inside;
        if (SupportsBasinColoring(ft)) {
            // Basin types: mask boundary = basin boundaries (root-index parity).
            // "converged vs not" is useless here — nearly all pixels converge,
            // producing a uniform mask and a flat (black) SDF.
            const int rootIdx = ResolveBasinRenderRootIndex(ft, z, params);
            inside = (rootIdx >= 0) && ((rootIdx & 1) == 0);
        } else {
            inside = LensMaskInsideForFractal(ft, converged, escaped);
        }
        outMask[py * width + px] = inside ? 255 : 0;
    }
}

struct CachedBuffers {
    int w = 0;
    int h = 0;
    uint32_t* d_rgba = nullptr;
    uint8_t* d_mask = nullptr;
    int* d_itersSum = nullptr;
    double2* d_refOrbit = nullptr;
    int refOrbitLen = 0;

    // Host-side ref-orbit cache to avoid rebuilding/uploading every frame.
    std::vector<double2> hostRefOrbit;
    bool hostRefValid = false;
    PerturbationReferenceOrbitCacheKey hostRefKey{};
};

CachedBuffers g_cached;

bool ensure_buffers(int w, int h, const char** outError) {
    if (w <= 0 || h <= 0) {
        if (outError) *outError = "Invalid resolution";
        return false;
    }
    if (g_cached.w == w && g_cached.h == h && g_cached.d_rgba && g_cached.d_mask && g_cached.d_itersSum) return true;

    if (g_cached.d_rgba) cudaFree(g_cached.d_rgba);
    if (g_cached.d_mask) cudaFree(g_cached.d_mask);
    if (g_cached.d_itersSum) cudaFree(g_cached.d_itersSum);
    if (g_cached.d_refOrbit) cudaFree(g_cached.d_refOrbit);

    g_cached.w = w;
    g_cached.h = h;
    g_cached.refOrbitLen = 0;
    g_cached.hostRefOrbit.clear();
    g_cached.hostRefValid = false;
    g_cached.hostRefKey = {};

    size_t bytes = (size_t)w * (size_t)h * sizeof(uint32_t);
    if (cudaMalloc(&g_cached.d_rgba, bytes) != cudaSuccess) {
        if (outError) *outError = "cudaMalloc failed for output buffer";
        g_cached.d_rgba = nullptr;
        return false;
    }

    size_t maskBytes = (size_t)w * (size_t)h * sizeof(uint8_t);
    if (cudaMalloc(&g_cached.d_mask, maskBytes) != cudaSuccess) {
        if (outError) *outError = "cudaMalloc failed for mask buffer";
        g_cached.d_mask = nullptr;
        return false;
    }

    if (cudaMalloc(&g_cached.d_itersSum, sizeof(int)) != cudaSuccess) {
        if (outError) *outError = "cudaMalloc failed for itersSum";
        g_cached.d_itersSum = nullptr;
        return false;
    }

    return true;
}

bool ensure_ref_orbit(int len, const char** outError) {
    if (len <= 0) {
        if (outError) *outError = "Invalid ref orbit length";
        return false;
    }
    if (g_cached.d_refOrbit && g_cached.refOrbitLen == len) return true;

    if (g_cached.d_refOrbit) cudaFree(g_cached.d_refOrbit);
    g_cached.d_refOrbit = nullptr;
    g_cached.refOrbitLen = 0;

    if (cudaMalloc(&g_cached.d_refOrbit, (size_t)len * sizeof(double2)) != cudaSuccess) {
        if (outError) *outError = "cudaMalloc failed for ref orbit";
        return false;
    }
    g_cached.refOrbitLen = len;
    return true;
}

} // namespace

bool RenderFractalCUDA(
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    uint32_t* outRGBA,
    uint8_t* outMask,
    RenderStats* outStats,
    const char** outError)
{
    if (outError) *outError = nullptr;
    if (!outRGBA) {
        if (outError) *outError = "outRGBA is null";
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

    int w = render.resolution.x;
    int h = render.resolution.y;

    if (!ensure_buffers(w, h, outError)) return false;

    // Resolve the sample tier into a concrete (backend, strategy) pair.
    RenderSettings resolvedRender = render;
    resolvedRender.resolved_eval = ResolveSampleEvalModeForRender(
        view.fractal_type, params, render.sample_tier, view.log2_zoom);

    const PerturbationReferenceOrbitRequest perturbRequest = ResolvePerturbationReferenceOrbitRequest(view, params);
    const bool wantPerturb = perturbRequest.enabled;
    const double2 refC0 = make_double2(perturbRequest.refCx, perturbRequest.refCy);
    const int refLen = perturbRequest.refLen;
    if (wantPerturb) {
        if (!ensure_ref_orbit(refLen, outError)) return false;

        const bool sameKey = g_cached.hostRefValid && MatchesPerturbationReferenceOrbitCacheKey(g_cached.hostRefKey, perturbRequest);

        if (!sameKey) {
            BuildPerturbationReferenceOrbit(perturbRequest, &g_cached.hostRefOrbit);

            if (cudaMemcpy(g_cached.d_refOrbit, g_cached.hostRefOrbit.data(), (size_t)refLen * sizeof(double2), cudaMemcpyHostToDevice) != cudaSuccess) {
                if (outError) *outError = "cudaMemcpy failed for ref orbit";
                return false;
            }

            g_cached.hostRefValid = true;
            g_cached.hostRefKey = MakePerturbationReferenceOrbitCacheKey(perturbRequest);
        }
    }

    int zero = 0;
    cudaMemcpy(g_cached.d_itersSum, &zero, sizeof(int), cudaMemcpyHostToDevice);

    dim3 block;
    int bs = render.block_size;
    if (bs < 64) bs = 64;
    if (bs > 1024) bs = 1024;

    // Choose a 2D block whose area is close to bs, favoring 16x16.
    int bx = 16;
    int by = bs / bx;
    if (by < 1) by = 1;
    if (by > 32) by = 32;
    block = dim3((unsigned)bx, (unsigned)by, 1);

    dim3 grid((w + block.x - 1) / block.x, (h + block.y - 1) / block.y, 1);

    cudaEvent_t start = nullptr, stop = nullptr;
    if (render.benchmark) {
        cudaEventCreate(&start);
        cudaEventCreate(&stop);
        cudaEventRecord(start);
    }

    kernel_render<<<grid, block>>>(
        g_cached.d_rgba,
        outMask ? g_cached.d_mask : nullptr,
        w,
        h,
        view,
        params,
        resolvedRender,
        wantPerturb ? g_cached.d_refOrbit : nullptr,
        refLen,
        refC0,
        g_cached.d_itersSum);

    cudaError_t launchErr = cudaGetLastError();
    if (launchErr != cudaSuccess) {
        if (outError) *outError = "CUDA kernel launch failed";
        return false;
    }

    cudaDeviceSynchronize();

    if (render.benchmark && start && stop) {
        cudaEventRecord(stop);
        cudaEventSynchronize(stop);
        float ms = 0.0f;
        cudaEventElapsedTime(&ms, start, stop);
        if (outStats) outStats->last_render_ms = ms;
        cudaEventDestroy(start);
        cudaEventDestroy(stop);
    }

    size_t bytes = (size_t)w * (size_t)h * sizeof(uint32_t);
    cudaMemcpy(outRGBA, g_cached.d_rgba, bytes, cudaMemcpyDeviceToHost);

    if (outMask) {
        size_t maskBytes = (size_t)w * (size_t)h * sizeof(uint8_t);
        cudaMemcpy(outMask, g_cached.d_mask, maskBytes, cudaMemcpyDeviceToHost);
    }

    int itersSum = 0;
    cudaMemcpy(&itersSum, g_cached.d_itersSum, sizeof(int), cudaMemcpyDeviceToHost);
    if (outStats) {
        outStats->last_device_id = dev;
        outStats->last_iters_avg = (w > 0 && h > 0) ? (itersSum / (w * h)) : 0;
        if (!render.benchmark) outStats->last_render_ms = 0.0f;
        outStats->resolved_eval = resolvedRender.resolved_eval;
    }

    return true;
}

// SampleFractalPoints() has been moved to fractal_sample_core.cu (K5).

void CleanupFractalCUDA() {
    if (g_cached.d_rgba) { cudaFree(g_cached.d_rgba); g_cached.d_rgba = nullptr; }
    if (g_cached.d_mask) { cudaFree(g_cached.d_mask); g_cached.d_mask = nullptr; }
    if (g_cached.d_itersSum) { cudaFree(g_cached.d_itersSum); g_cached.d_itersSum = nullptr; }
    if (g_cached.d_refOrbit) { cudaFree(g_cached.d_refOrbit); g_cached.d_refOrbit = nullptr; }
    g_cached.w = 0;
    g_cached.h = 0;
    CleanupFractalSampleCore();
    cudaDeviceReset();
}
