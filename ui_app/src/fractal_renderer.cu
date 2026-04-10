#include "fractal_types.h"
#include "fractal_family_rules.h"
#include "fractal_sample_result.h"
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

struct Cx {
    float x;
    float y;
};

struct Cxd {
    double x;
    double y;
};

__device__ __forceinline__ Cx cx_rot(Cx z, float a) {
    float cs = cosf(a);
    float sn = sinf(a);
    return {z.x * cs - z.y * sn, z.x * sn + z.y * cs};
}

__device__ __forceinline__ float hash01_u32(unsigned int x) {
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return (float)(x & 0x00ffffffU) / (float)0x01000000U;
}

__device__ __forceinline__ Cx explaino_warp_start(Cx coord, double seed, float phase, float strength) {
    float s = fmaxf(0.0f, fminf(1.0f, strength));
    if (s <= 0.0f) return coord;
    unsigned long long bits = (unsigned long long)__double_as_longlong(seed);
    unsigned int u = (unsigned int)(bits ^ (bits >> 32));
    float a0 = hash01_u32(u ^ 0x1234567u);
    float a1 = hash01_u32(u ^ 0x89abcdefu);

    float rot = s * (a0 * 2.0f - 1.0f) * 3.1415926f;
    Cx z = cx_rot(coord, rot);

    float freq = 2.0f + 6.0f * a1;
    float k = 0.10f + 0.35f * a0;
    z.x += s * k * sinf(z.y * freq + phase);
    z.y += s * k * sinf(z.x * freq - phase);

    Cx z2{z.x * z.x - z.y * z.y, 2.0f * z.x * z.y};
    float push = s * (0.06f + 0.10f * a1);
    z = {z.x + z2.x * push, z.y + z2.y * push};
    return z;
}

__device__ __forceinline__ Cxd explaino_warp_start_d(Cxd coord, double seed, float phase, float strength) {
    double s = fmax(0.0, fmin(1.0, (double)strength));
    if (s <= 0.0) return coord;
    unsigned long long bits = (unsigned long long)__double_as_longlong(seed);
    unsigned int u = (unsigned int)(bits ^ (bits >> 32));
    double a0 = (double)hash01_u32(u ^ 0x1234567u);
    double a1 = (double)hash01_u32(u ^ 0x89abcdefu);

    double rot = s * (a0 * 2.0 - 1.0) * 3.141592653589793;
    double cs = cos(rot), sn = sin(rot);
    Cxd z = {coord.x * cs - coord.y * sn, coord.x * sn + coord.y * cs};

    double freq = 2.0 + 6.0 * a1;
    double k = 0.10 + 0.35 * a0;
    z.x += s * k * sin(z.y * freq + (double)phase);
    z.y += s * k * sin(z.x * freq - (double)phase);

    Cxd z2{z.x * z.x - z.y * z.y, 2.0 * z.x * z.y};
    double push = s * (0.06 + 0.10 * a1);
    z = {z.x + z2.x * push, z.y + z2.y * push};
    return z;
}

__device__ __forceinline__ Cx cx_add(Cx a, Cx b) { return {a.x + b.x, a.y + b.y}; }
__device__ __forceinline__ Cx cx_sub(Cx a, Cx b) { return {a.x - b.x, a.y - b.y}; }
__device__ __forceinline__ Cx cx_mul(Cx a, Cx b) { return {a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x}; }
__device__ __forceinline__ Cx cx_scale(Cx a, float s) { return {a.x * s, a.y * s}; }
__device__ __forceinline__ float cx_abs2(Cx a) { return a.x * a.x + a.y * a.y; }
__device__ __forceinline__ float cx_abs(Cx a) { return sqrtf(cx_abs2(a)); }
__device__ __forceinline__ Cx cx_div(Cx a, Cx b) {
    // a / b
    float denom = b.x * b.x + b.y * b.y;
    if (denom == 0.0f) return {0.0f, 0.0f};
    return {(a.x * b.x + a.y * b.y) / denom, (a.y * b.x - a.x * b.y) / denom};
}

__device__ __forceinline__ Cxd cxd_add(Cxd a, Cxd b) { return {a.x + b.x, a.y + b.y}; }
__device__ __forceinline__ Cxd cxd_sub(Cxd a, Cxd b) { return {a.x - b.x, a.y - b.y}; }
__device__ __forceinline__ Cxd cxd_mul(Cxd a, Cxd b) { return {a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x}; }
__device__ __forceinline__ Cxd cxd_scale(Cxd a, double s) { return {a.x * s, a.y * s}; }
__device__ __forceinline__ double cxd_abs2(Cxd a) { return a.x * a.x + a.y * a.y; }
__device__ __forceinline__ double cxd_abs(Cxd a) { return sqrt(cxd_abs2(a)); }
__device__ __forceinline__ Cxd cxd_div(Cxd a, Cxd b) {
    double denom = b.x * b.x + b.y * b.y;
    if (denom == 0.0) return {0.0, 0.0};
    return {(a.x * b.x + a.y * b.y) / denom, (a.y * b.x - a.x * b.y) / denom};
}

__device__ __forceinline__ void poly_eval_real_coeffs_deg4(const float coeffs[5], Cx z, Cx* outP, Cx* outDp) {
    PolyEvalRealCoeffsDeg4(coeffs, z, outP, outDp);
}

__device__ __forceinline__ void poly_eval_real_coeffs_deg4_d2(const float coeffs[5], Cx z, Cx* outP, Cx* outDp, Cx* outD2p) {
    PolyEvalRealCoeffsDeg4D2(coeffs, z, outP, outDp, outD2p);
}

__device__ __forceinline__ void poly_eval_real_coeffs_deg4_d(const float coeffs[5], Cxd z, Cxd* outP, Cxd* outDp) {
    PolyEvalRealCoeffsDeg4(coeffs, z, outP, outDp);
}

__device__ __forceinline__ void poly_eval_real_coeffs_deg4_d2_d(const float coeffs[5], Cxd z, Cxd* outP, Cxd* outDp, Cxd* outD2p) {
    PolyEvalRealCoeffsDeg4D2(coeffs, z, outP, outDp, outD2p);
}

__device__ __forceinline__ Cx unit_root_k(int k, int n) {
    float a = (2.0f * CUDART_PI_F) * ((float)k / (float)max(1, n));
    return {cosf(a), sinf(a)};
}

__device__ __forceinline__ Cxd cxd_from_double2(double2 v) { return {v.x, v.y}; }

// --- Extracted device function: computes one fractal sample at a given coordinate.
// No pixel mapping, no coloring, no framebuffer — pure iteration result.
__device__ FractalSampleResult fractal_sample_device(
    Cx coord, Cxd coordD, ViewState view, KernelParams params,
    RenderSettings render, const double2* refOrbit, int refLen, double2 refC0)
{
#include "fractal_sample_device.inl"
}

// K2: Thin launch wrapper — samples arbitrary complex-plane coordinates.
// No pixel mapping, no coloring, no framebuffer.
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
            if (!converged) {
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
                int nRoots = ResolvePolynomialRootCount(params.poly_kind);

                bool isExplainoFamily = IsExplainoFamily(ft);
                bool useCustomRoots = (nRoots == 0) && isExplainoFamily && (params.explaino_root_count > 0);

                if (nRoots > 0 || useCustomRoots) {
                    int idx = useCustomRoots ? NearestRootIndexList(z, params.explaino_roots, params.explaino_root_count)
                                             : NearestRootIndexUnitRoots(z, nRoots);
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
        } else {
            if (!converged) {
                color = {0, 0, 0, 255};
            } else if (mode == ColoringMode::root_basin) {
                int nRoots = ResolvePolynomialRootCount(params.poly_kind);

                bool isExplainoFamily = IsExplainoFamily(ft);
                bool useCustomRoots = (nRoots == 0) && isExplainoFamily && (params.explaino_root_count > 0);

                if (useCustomRoots) {
                    int idx = NearestRootIndexList(z, params.explaino_roots, params.explaino_root_count);
                    color = PaletteRoot<uchar4>(idx);
                } else if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(z, nRoots);
                    color = PaletteRoot<uchar4>(idx);
                } else {
                    // Invalid: root identity not defined.
                    color = errorColor;
                }
            }

            if (mode == ColoringMode::iteration_count) {
                float t = (float)it / (float)maxIter;
                unsigned char v = (unsigned char)(fminf(1.0f, t) * 255.0f);
                color = {v, (unsigned char)(255 - v), 128, 255};
            } else if (mode == ColoringMode::smooth_escape) {
                float s = -logf(fmaxf(pAbs, 1e-12f));
                float t = fminf(1.0f, s / 20.0f);
                unsigned char r = (unsigned char)(t * 255.0f);
                unsigned char g = (unsigned char)(sqrtf(t) * 255.0f);
                color = {r, g, (unsigned char)(255 - r), 255};
            } else if (mode == ColoringMode::phase) {
                float angle = atan2f(z.y, z.x);
                float h = (angle + 3.14159265f) / (2.0f * 3.14159265f);
                float v = converged ? 0.85f : 0.25f;
                color = HsvToRgb<uchar4>(h, 0.9f, v);
            } else if (mode == ColoringMode::iteration_bands) {
                color = IterationBandColor<uchar4>(it, maxIter);
            }
        }
    } else {
        // Escape-time coloring.
        color = MakeEscapeTimeBaseColor<uchar4>(ft, mode, escaped, it, maxIter, z, params);
    }

    color = ApplyFractalColorGrading(color, params);

    uint32_t rgba = (uint32_t)color.x | ((uint32_t)color.y << 8) | ((uint32_t)color.z << 16) | ((uint32_t)color.w << 24);
    outRGBA[py * width + px] = rgba;

    if (outMask) {
        outMask[py * width + px] = LensMaskInsideForFractal(ft, converged, escaped) ? 255 : 0;
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
    resolvedRender.resolved_eval = ResolveSampleEvalMode(
        view.fractal_type, render.sample_tier, view.log2_zoom);

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

    // Allocate device buffers for coordinates and results.
    // Double2 (plain C++) and double2 (CUDA) have identical layout.
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
        wantPerturb ? g_cached.d_refOrbit : nullptr,
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

void CleanupFractalCUDA() {
    if (g_cached.d_rgba) { cudaFree(g_cached.d_rgba); g_cached.d_rgba = nullptr; }
    if (g_cached.d_mask) { cudaFree(g_cached.d_mask); g_cached.d_mask = nullptr; }
    if (g_cached.d_itersSum) { cudaFree(g_cached.d_itersSum); g_cached.d_itersSum = nullptr; }
    if (g_cached.d_refOrbit) { cudaFree(g_cached.d_refOrbit); g_cached.d_refOrbit = nullptr; }
    g_cached.w = 0;
    g_cached.h = 0;
    cudaDeviceReset();
}
