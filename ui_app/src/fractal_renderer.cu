#include "fractal_types.h"
#include "fractal_family_rules.h"
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

    // Precision tier dispatch: resolved eval mode determines arithmetic width.
    bool useFP64 = (render.resolved_eval.backend == NumericBackend::float64);

    int maxIter = max(1, params.max_iter);
    float eps = fmaxf(1e-12f, params.epsilon);
    double epsD = fmax(1.0e-14, (double)params.epsilon);

    int it = 0;
    float pAbs = 0.0f;
    bool converged = false;
    bool escaped = false;

    Cx z{0.0f, 0.0f};
    Cx cConst{0.0f, 0.0f};

    FractalType ft = view.fractal_type;
    if (ft == FractalType::newton) {
        if (useFP64) {
            Cxd zd = coordD;
            double pAbsD = 0.0;
            for (; it < maxIter; ++it) {
                Cxd P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; k++) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d(coeffs, zd, &P, &dP);
                pAbsD = cxd_abs(P);
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dP);
                if (dAbs2 < 1e-30) break;
                Cxd step = cxd_div(P, dP);
                zd = cxd_sub(zd, step);
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = coord;
            for (; it < maxIter; ++it) {
                Cx P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; k++) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);
                pAbs = cx_abs(P);
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dP);
                if (dAbs2 < 1e-20f) break;
                Cx step = cx_div(P, dP);
                z = cx_sub(z, step);
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
    } else if (ft == FractalType::explaino || ft == FractalType::explaino_dual || ft == FractalType::explaino_mult) {
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            for (; it < maxIter; ++it) {
                Cxd P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d(coeffs, zd, &P, &dP);
                pAbsD = cxd_abs(P);
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dP);
                if (dAbs2 < 1e-30) break;
                Cxd step = cxd_div(P, dP);
                zd = cxd_sub(zd, cxd_scale(step, dampD));
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            for (; it < maxIter; ++it) {
                Cx P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);
                pAbs = cx_abs(P);
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dP);
                if (dAbs2 < 1e-20f) break;
                Cx step = cx_div(P, dP);
                z = cx_sub(z, cx_scale(step, userDamp));
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
    } else if (ft == FractalType::explaino_fp) {
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            for (; it < maxIter; ++it) {
                Cxd P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d(coeffs, zd, &P, &dP);
                pAbsD = cxd_abs(P);
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dP);
                Cxd step = (dAbs2 < 1e-30) ? P : cxd_div(P, dP);
                double stepMag = sqrt(fmax(0.0, cxd_abs2(step)));
                double damp = dampD / (1.0 + stepMag);
                zd = cxd_sub(zd, cxd_scale(step, damp));
                double r2 = cxd_abs2(zd);
                if (r2 > 16.0) {
                    double r = sqrt(r2);
                    double s = 4.0 / fmax(1e-24, r);
                    zd = cxd_scale(zd, s);
                }
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            for (; it < maxIter; ++it) {
                Cx P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);
                pAbs = cx_abs(P);
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dP);
                Cx step = (dAbs2 < 1e-20f) ? P : cx_div(P, dP);
                float stepMag = sqrtf(fmaxf(0.0f, cx_abs2(step)));
                float damp = userDamp / (1.0f + stepMag);
                z = cx_sub(z, cx_scale(step, damp));
                float r2 = cx_abs2(z);
                if (r2 > 16.0f) {
                    float r = sqrtf(r2);
                    float s = 4.0f / fmaxf(1e-12f, r);
                    z = cx_scale(z, s);
                }
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
        if (!converged) {
            int nRoots = ResolvePolynomialRootCount(params.poly_kind);
            if (useFP64) {
                Cxd zd = {(double)z.x, (double)z.y};
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(zd, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(zd, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            } else {
                if (nRoots > 0) {
                    int idx = NearestRootIndexUnitRoots(z, nRoots);
                    z = unit_root_k(idx, nRoots);
                } else if (params.explaino_root_count > 0) {
                    int idx = NearestRootIndexList(z, params.explaino_roots, params.explaino_root_count);
                    z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
                }
            }
            converged = true;
        }
    } else if (ft == FractalType::explaino_y) {
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        z = explaino_warp_start(coord, seed, phase, strength);
        Cx zPrev = z;

        float bestP = 1.0e30f;
        int bestIt = 0;
        Cx bestZ = z;

        for (; it < maxIter; ++it) {
            float localPhase = phase + 0.07f * (float)it;
            Cx zW = explaino_warp_start(z, seed, localPhase, strength * 0.30f);

            Cx P, dP;

            float coeffs[5];
            #pragma unroll
            for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];

            poly_eval_real_coeffs_deg4(coeffs, zW, &P, &dP);

            pAbs = cx_abs(P);
            if (pAbs < bestP) {
                bestP = pAbs;
                bestIt = it;
                bestZ = zW;
            }
            if (pAbs < eps) {
                z = zW;
                break;
            }

            float dAbs2 = cx_abs2(dP);
            Cx step = (dAbs2 < 1e-20f) ? P : cx_div(P, dP);

            float stepMag = sqrtf(fmaxf(0.0f, cx_abs2(step)));
            float damp = 0.90f * userDamp / (1.0f + stepMag);
            Cx newtonW = cx_sub(zW, cx_scale(step, damp));

            float mix = 0.78f;
            Cx zNext = cx_add(cx_scale(z, 1.0f - mix), cx_scale(newtonW, mix));

            Cx vel = cx_sub(z, zPrev);
            zNext = cx_add(zNext, cx_scale(vel, 0.10f));
            zNext = cx_add(zNext, cx_scale(coord, 0.045f));

            zPrev = z;
            z = zNext;

            float r2 = cx_abs2(z);
            float k = 1.0f / sqrtf(1.0f + r2 * 0.0625f);
            z = cx_scale(z, 4.0f * k);

            if (!isfinite(z.x) || !isfinite(z.y)) {
                z = {0.0f, 0.0f};
                break;
            }
        }

        converged = (pAbs < eps);
        if (!converged) {
            z = bestZ;
            it = bestIt;
            pAbs = bestP;
            converged = true;
        }
    } else if (ft == FractalType::nova || ft == FractalType::explaino_nova) {
        // Nova (V1): z_{n+1} = z_n - alpha * f(z_n)/f'(z_n) + c
        z = {0.0f, 0.0f};
        cConst = coord;

        float alpha = params.nova_alpha;
        if (!(alpha > 0.0f) || !(alpha <= 2.0f) || !isfinite(alpha)) {
            escaped = true;
        } else if (useFP64) {
            Cxd zd = {0.0, 0.0};
            Cxd cConstD = coordD;
            double alphaD = (double)alpha;
            double pAbsD = 0.0;
            for (; it < maxIter; ++it) {
                Cxd P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; k++) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d(coeffs, zd, &P, &dP);
                pAbsD = cxd_abs(P);
                if (pAbsD < epsD) { converged = true; break; }
                double dAbs2 = cxd_abs2(dP);
                // Nova: when derivative is zero, skip Newton step but still apply +c.
                if (dAbs2 >= 1e-30) {
                    Cxd step = cxd_div(P, dP);
                    zd = cxd_sub(zd, cxd_scale(step, alphaD));
                }
                zd = cxd_add(zd, cConstD);
                if (!isfinite(zd.x) || !isfinite(zd.y)) { escaped = true; break; }
                if (cxd_abs2(zd) > 4.0) { escaped = true; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
        } else {
            for (; it < maxIter; ++it) {
                Cx P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; k++) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);
                pAbs = cx_abs(P);
                if (pAbs < eps) { converged = true; break; }
                float dAbs2 = cx_abs2(dP);
                // Nova: when derivative is zero, skip Newton step but still apply +c.
                if (dAbs2 >= 1e-20f) {
                    Cx step = cx_div(P, dP);
                    z = cx_sub(z, cx_scale(step, alpha));
                }
                z = cx_add(z, cConst);
                if (!isfinite(z.x) || !isfinite(z.y)) { escaped = true; break; }
                if (cx_abs2(z) > 4.0f) { escaped = true; break; }
            }
        }
    } else if (ft == FractalType::explaino_halley) {
        // Halley's method: z_{n+1} = z - 2 f(z) f'(z) / (2 f'(z)^2 - f(z) f''(z))
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            for (; it < maxIter; ++it) {
                Cxd P, dP, d2P;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d2_d(coeffs, zd, &P, &dP, &d2P);
                pAbsD = cxd_abs(P);
                if (pAbsD < epsD) break;
                Cxd dp2 = cxd_mul(dP, dP);
                Cxd fd2 = cxd_mul(P, d2P);
                Cxd denom = cxd_sub(cxd_scale(dp2, 2.0), fd2);
                double denomAbs2 = cxd_abs2(denom);
                if (denomAbs2 < 1e-30) break;
                Cxd numer = cxd_scale(cxd_mul(P, dP), 2.0);
                Cxd step = cxd_div(numer, denom);
                zd = cxd_sub(zd, cxd_scale(step, dampD));
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            for (; it < maxIter; ++it) {
                Cx P, dP, d2P;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d2(coeffs, z, &P, &dP, &d2P);
                pAbs = cx_abs(P);
                if (pAbs < eps) break;
                Cx dp2 = cx_mul(dP, dP);
                Cx fd2 = cx_mul(P, d2P);
                Cx denom = cx_sub(cx_scale(dp2, 2.0f), fd2);
                float denomAbs2 = cx_abs2(denom);
                if (denomAbs2 < 1e-20f) break;
                Cx numer = cx_scale(cx_mul(P, dP), 2.0f);
                Cx step = cx_div(numer, denom);
                z = cx_sub(z, cx_scale(step, userDamp));
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
    } else if (ft == FractalType::explaino_phoenix) {
        // Explaino-Phoenix: seeded Newton with previous-z memory term.
        // z_{n+1} = z_n - damping * f(z_n)/f'(z_n) + p * z_{n-1}
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            Cxd zPrevD = zd;
            Cxd pConstD{(double)params.phoenix_p_real, (double)params.phoenix_p_imag};
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            for (; it < maxIter; ++it) {
                Cxd P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d(coeffs, zd, &P, &dP);
                pAbsD = cxd_abs(P);
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dP);
                if (dAbs2 < 1e-30) break;
                Cxd step = cxd_div(P, dP);
                Cxd zNext = cxd_add(cxd_sub(zd, cxd_scale(step, dampD)), cxd_mul(pConstD, zPrevD));
                zPrevD = zd;
                zd = zNext;
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            Cx zPrev = z;
            Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
            for (; it < maxIter; ++it) {
                Cx P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);
                pAbs = cx_abs(P);
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dP);
                if (dAbs2 < 1e-20f) break;
                Cx step = cx_div(P, dP);
                Cx zNext = cx_add(cx_sub(z, cx_scale(step, userDamp)), cx_mul(pConst, zPrev));
                zPrev = z;
                z = zNext;
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
    } else if (ft == FractalType::explaino_joy) {
        // Explaino-Joy: coupled root-critical Newton.
        // z_{n+1} = z_n - damp * [(1-gamma)*P/P' + gamma*P'/P''] + phoenix_p * z_{n-1}
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        float joyCoupling = params.joy_coupling;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            Cxd zPrevD = zd;
            Cxd pConstD{(double)params.phoenix_p_real, (double)params.phoenix_p_imag};
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            double gammaD = (double)joyCoupling;
            double oneMinusGammaD = 1.0 - gammaD;
            for (; it < maxIter; ++it) {
                Cxd P, dP, d2P;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d2_d(coeffs, zd, &P, &dP, &d2P);
                pAbsD = cxd_abs(P);
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dP);
                if (dAbs2 < 1e-30) break;
                Cxd newtonStep = cxd_div(P, dP);
                Cxd joyStep = {0.0, 0.0};
                double d2Abs2 = cxd_abs2(d2P);
                if (d2Abs2 > 1e-30) {
                    joyStep = cxd_div(dP, d2P);
                }
                Cxd combinedStep = cxd_add(
                    cxd_scale(newtonStep, oneMinusGammaD),
                    cxd_scale(joyStep, gammaD));
                Cxd zNext = cxd_add(
                    cxd_sub(zd, cxd_scale(combinedStep, dampD)),
                    cxd_mul(pConstD, zPrevD));
                zPrevD = zd;
                zd = zNext;
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            Cx zPrev = z;
            Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
            float oneMinusGamma = 1.0f - joyCoupling;
            for (; it < maxIter; ++it) {
                Cx P, dP, d2P;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d2(coeffs, z, &P, &dP, &d2P);
                pAbs = cx_abs(P);
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dP);
                if (dAbs2 < 1e-20f) break;
                Cx newtonStep = cx_div(P, dP);
                Cx joyStep = {0.0f, 0.0f};
                float d2Abs2 = cx_abs2(d2P);
                if (d2Abs2 > 1e-20f) {
                    joyStep = cx_div(dP, d2P);
                }
                Cx combinedStep = cx_add(
                    cx_scale(newtonStep, oneMinusGamma),
                    cx_scale(joyStep, joyCoupling));
                Cx zNext = cx_add(
                    cx_sub(z, cx_scale(combinedStep, userDamp)),
                    cx_mul(pConst, zPrev));
                zPrev = z;
                z = zNext;
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
    } else if (ft == FractalType::explaino_transcendental) {
        // Explaino-Transcendental: seeded Newton applied to transcendental functions.
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);

        TranscendentalFunc tf = params.transcendental_func;
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            for (; it < maxIter; ++it) {
                Cxd F, dF;
                if (tf == TranscendentalFunc::f_sin) {
                    double sx = sin(zd.x), cx_ = cos(zd.x);
                    double shy = sinh(zd.y), chy = cosh(zd.y);
                    F = {sx * chy, cx_ * shy};
                    dF = {cx_ * chy, -sx * shy};
                } else if (tf == TranscendentalFunc::f_exp_minus_1) {
                    double ex = exp(zd.x);
                    double cy_ = cos(zd.y), sy_ = sin(zd.y);
                    F = {ex * cy_ - 1.0, ex * sy_};
                    dF = {ex * cy_, ex * sy_};
                } else {
                    double chx = cosh(zd.x), shx = sinh(zd.x);
                    double cy_ = cos(zd.y), sy_ = sin(zd.y);
                    F = {chx * cy_, shx * sy_};
                    dF = {shx * cy_, chx * sy_};
                }
                pAbsD = cxd_abs(F);
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dF);
                if (dAbs2 < 1e-30) break;
                Cxd step = cxd_div(F, dF);
                zd = cxd_sub(zd, cxd_scale(step, dampD));
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            for (; it < maxIter; ++it) {
                Cx F, dF;
                if (tf == TranscendentalFunc::f_sin) {
                    float sx = sinf(z.x), cx_ = cosf(z.x);
                    float shy = sinhf(z.y), chy = coshf(z.y);
                    F = {sx * chy, cx_ * shy};
                    dF = {cx_ * chy, -sx * shy};
                } else if (tf == TranscendentalFunc::f_exp_minus_1) {
                    float ex = expf(z.x);
                    float cy_ = cosf(z.y), sy_ = sinf(z.y);
                    F = {ex * cy_ - 1.0f, ex * sy_};
                    dF = {ex * cy_, ex * sy_};
                } else {
                    float chx = coshf(z.x), shx = sinhf(z.x);
                    float cy_ = cosf(z.y), sy_ = sinf(z.y);
                    F = {chx * cy_, shx * sy_};
                    dF = {shx * cy_, chx * sy_};
                }
                pAbs = cx_abs(F);
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dF);
                if (dAbs2 < 1e-20f) break;
                Cx step = cx_div(F, dF);
                z = cx_sub(z, cx_scale(step, userDamp));
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
    } else if (ft == FractalType::explaino_inertial) {
        // Explaino-Inertial: seeded Newton with previous-step momentum.
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        float beta = params.momentum_beta;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            Cxd zPrevD = zd;
            Cxd zPrev2D = zd;
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            double betaD = (double)beta;
            for (; it < maxIter; ++it) {
                Cxd P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d(coeffs, zd, &P, &dP);
                pAbsD = cxd_abs(P);
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dP);
                if (dAbs2 < 1e-30) break;
                Cxd newtonStep = cxd_div(P, dP);
                Cxd momentum = cxd_sub(zPrevD, zPrev2D);
                Cxd zNext = cxd_add(cxd_sub(zd, cxd_scale(newtonStep, dampD)),
                                    cxd_scale(momentum, betaD));
                zPrev2D = zPrevD;
                zPrevD = zd;
                zd = zNext;
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            Cx zPrev = z;
            Cx zPrev2 = z;
            for (; it < maxIter; ++it) {
                Cx P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);
                pAbs = cx_abs(P);
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dP);
                if (dAbs2 < 1e-20f) break;
                Cx newtonStep = cx_div(P, dP);
                Cx momentum = cx_sub(zPrev, zPrev2);
                Cx zNext = cx_add(cx_sub(z, cx_scale(newtonStep, userDamp)),
                                  cx_scale(momentum, beta));
                zPrev2 = zPrev;
                zPrev = z;
                z = zNext;
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
    } else if (ft == FractalType::explaino_julia) {
        // Explaino-Julia bridge: escape-time z^2+c with warp start and seeded Julia constant.
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        z = explaino_warp_start(coord, seed, phase, strength);

        // Julia constant c from the first seeded root (falls back to classic if no roots).
        Cx cJ = (params.explaino_root_count > 0)
            ? Cx{params.explaino_roots[0].x, params.explaino_roots[0].y}
            : Cx{-0.7f, 0.27015f};

        for (; it < maxIter; ++it) {
            Cx z2 = cx_mul(z, z);
            z = cx_add(z2, cJ);

            if (cx_abs2(z) > 4.0f) {
                escaped = true;
                break;
            }
            if (!isfinite(z.x) || !isfinite(z.y)) {
                escaped = true;
                break;
            }
        }
    } else if (ft == FractalType::explaino_lambda) {
        // Explaino-Lambda bridge: escape-time logistic map with warp start.
        // z_{n+1} = lambda * z * (1 - z), seeded z from explaino polynomial surface.
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        z = explaino_warp_start(coord, seed, phase, strength);

        Cx lambdaC{params.lambda_real, params.lambda_imag};

        for (; it < maxIter; ++it) {
            Cx oneMinusZ{1.0f - z.x, -z.y};
            z = cx_mul(lambdaC, cx_mul(z, oneMinusZ));

            if (cx_abs2(z) > 4.0f) {
                escaped = true;
                break;
            }
            if (!isfinite(z.x) || !isfinite(z.y)) {
                escaped = true;
                break;
            }
        }
    } else if (ft == FractalType::explaino_rational_escape) {
        // Explaino-Rational-Escape: escape-time iteration of Laurent polynomial P(z)/z^3.
        // z_{n+1} = P(z) / z^3 where P is the deg-4 seed polynomial.
        // Decomposes to: c0/z^3 + c1/z^2 + c2/z + c3 + c4*z (mixed positive/negative powers).
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        z = explaino_warp_start(coord, seed, phase, strength);

        float coeffs[5];
        #pragma unroll
        for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];

        for (; it < maxIter; ++it) {
            float zAbs2 = cx_abs2(z);
            if (zAbs2 < 1e-20f) {
                // At pole: treat as escaped (division by zero)
                escaped = true;
                break;
            }

            // Evaluate P(z)
            Cx P, dP;
            poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);

            // Compute z^3 = z * z * z
            Cx z2 = cx_mul(z, z);
            Cx z3 = cx_mul(z2, z);

            // z_{n+1} = P(z) / z^3
            z = cx_div(P, z3);

            if (cx_abs2(z) > 10000.0f) {
                escaped = true;
                break;
            }
            if (!isfinite(z.x) || !isfinite(z.y)) {
                escaped = true;
                break;
            }
        }
    } else if (ft == FractalType::explaino_rational) {
        // Explaino-Rational: Newton on f(z) = P(z) + cluster_radius/z.
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        float ratAlpha = params.explaino_cluster_radius;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            double ratAlphaD = (double)ratAlpha;
            for (; it < maxIter; ++it) {
                double zAbs2 = cxd_abs2(zd);
                if (zAbs2 < 1e-30) break;
                Cxd P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d(coeffs, zd, &P, &dP);
                Cxd zInv = cxd_div({1.0, 0.0}, zd);
                Cxd F = cxd_add(P, cxd_scale(zInv, ratAlphaD));
                Cxd zInv2 = cxd_mul(zInv, zInv);
                Cxd dF = cxd_sub(dP, cxd_scale(zInv2, ratAlphaD));
                pAbsD = cxd_abs(F);
                if (pAbsD < epsD) break;
                double dAbs2 = cxd_abs2(dF);
                if (dAbs2 < 1e-30) break;
                Cxd step = cxd_div(F, dF);
                zd = cxd_sub(zd, cxd_scale(step, dampD));
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            for (; it < maxIter; ++it) {
                float zAbs2 = cx_abs2(z);
                if (zAbs2 < 1e-20f) break;
                Cx P, dP;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);
                Cx zInv = cx_div({1.0f, 0.0f}, z);
                Cx F = cx_add(P, cx_scale(zInv, ratAlpha));
                Cx zInv2 = cx_mul(zInv, zInv);
                Cx dF = cx_sub(dP, cx_scale(zInv2, ratAlpha));
                pAbs = cx_abs(F);
                if (pAbs < eps) break;
                float dAbs2 = cx_abs2(dF);
                if (dAbs2 < 1e-20f) break;
                Cx step = cx_div(F, dF);
                z = cx_sub(z, cx_scale(step, userDamp));
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
    } else if (ft == FractalType::multicorn) {
        EscapeTimeDirectState<Cx> state = InitEscapeTimeDirectState(ft, coord);
        const Cx lambdaConst{params.lambda_real, params.lambda_imag};
        const Cx phoenixP{params.phoenix_p_real, params.phoenix_p_imag};

        for (; it < maxIter; ++it) {
            StepEscapeTimeDirectState(ft, params.multibrot_power_float, params.multibrot_power, lambdaConst, phoenixP, &state);
            z = state.z;

            if (cx_abs2(state.z) > DirectEscapeTimeRadiusSquared<float>()) {
                escaped = true;
                break;
            }
            if (!isfinite(state.z.x) || !isfinite(state.z.y) || !isfinite(state.z_prev.x) || !isfinite(state.z_prev.y)) {
                escaped = true;
                break;
            }
        }
    } else if (ft == FractalType::halley) {
        // Standalone Halley's method on standard polynomials.
        if (useFP64) {
            Cxd zd = coordD;
            double pAbsD = 0.0;
            for (; it < maxIter; ++it) {
                Cxd P, dP, d2P;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d2_d(coeffs, zd, &P, &dP, &d2P);
                pAbsD = cxd_abs(P);
                if (pAbsD < epsD) break;
                Cxd dp2 = cxd_mul(dP, dP);
                Cxd fd2 = cxd_mul(P, d2P);
                Cxd denom = cxd_sub(cxd_scale(dp2, 2.0), fd2);
                double denomAbs2 = cxd_abs2(denom);
                if (denomAbs2 < 1e-30) break;
                Cxd numer = cxd_scale(cxd_mul(P, dP), 2.0);
                Cxd step = cxd_div(numer, denom);
                zd = cxd_sub(zd, step);
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = coord;
            for (; it < maxIter; ++it) {
                Cx P, dP, d2P;
                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
                poly_eval_real_coeffs_deg4_d2(coeffs, z, &P, &dP, &d2P);
                pAbs = cx_abs(P);
                if (pAbs < eps) break;
                Cx dp2 = cx_mul(dP, dP);
                Cx fd2 = cx_mul(P, d2P);
                Cx denom = cx_sub(cx_scale(dp2, 2.0f), fd2);
                float denomAbs2 = cx_abs2(denom);
                if (denomAbs2 < 1e-20f) break;
                Cx numer = cx_scale(cx_mul(P, dP), 2.0f);
                Cx step = cx_div(numer, denom);
                z = cx_sub(z, step);
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
    } else if (UsesSpecializedEscapeTimeFormula(ft)) {
        z = coord;
        const McMullenPresetConfig mcmullenConfig = ResolveMcMullenPresetConfig(params.mcmullen_preset);

        for (; it < maxIter; ++it) {
            if (ft == FractalType::mcmullen) {
                if (StepMcMullenEscapeState(mcmullenConfig, &z) == SpecializedEscapeStepResult::pole) {
                    escaped = true;
                    break;
                }
            } else {
                StepCollatzEscapeState(&z);
            }

            if (cx_abs2(z) > SpecializedEscapeRadiusSquared()) {
                escaped = true;
                break;
            }
            if (!isfinite(z.x) || !isfinite(z.y)) {
                escaped = true;
                break;
            }
        }
    } else if (ft == FractalType::explaino_collatz) {
        // Explaino-Collatz: Newton's method on fixed points of the Collatz map.
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);

        if (useFP64) {
            Cxd zd = explaino_warp_start_d(coordD, seed, phase, strength);
            double pAbsD = 0.0;
            double dampD = (double)userDamp;
            for (; it < maxIter; ++it) {
                const ExplainoCollatzStepResult stepResult = StepExplainoCollatzNewton(dampD, epsD, &zd, &pAbsD);
                if (stepResult == ExplainoCollatzStepResult::converged) break;
                if (stepResult == ExplainoCollatzStepResult::degenerate) break;
                if (!isfinite(zd.x) || !isfinite(zd.y)) { zd = {0.0, 0.0}; break; }
            }
            z = {(float)zd.x, (float)zd.y};
            pAbs = (float)pAbsD;
            converged = (pAbsD < epsD);
        } else {
            z = explaino_warp_start(coord, seed, phase, strength);
            for (; it < maxIter; ++it) {
                const ExplainoCollatzStepResult stepResult = StepExplainoCollatzNewton(userDamp, eps, &z, &pAbs);
                if (stepResult == ExplainoCollatzStepResult::converged) break;
                if (stepResult == ExplainoCollatzStepResult::degenerate) break;
                if (!isfinite(z.x) || !isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
            }
            converged = (pAbs < eps);
        }
    } else {
        // Escape-time family.
        bool canPerturb = (refOrbit != nullptr) && (refLen >= (maxIter + 1));
        bool doPerturb = canPerturb && (ft == FractalType::mandelbrot || ft == FractalType::julia);

        if (doPerturb) {
            // Perturbation deep zoom.
            // Mandelbrot: z0=0, c = refC0 + dc, delta0=0
            // Julia: z0 = refZ0 + dz0, c fixed, delta0=dz0
            Cxd dc{0.0, 0.0};
            Cxd delta{0.0, 0.0};

            if (ft == FractalType::mandelbrot) {
                dc = cxd_sub(coordD, cxd_from_double2(refC0));
                delta = {0.0, 0.0};
            } else {
                // Julia: ref orbit uses z0 reference encoded in refC0
                dc = {0.0, 0.0};
                delta = cxd_sub(coordD, cxd_from_double2(refC0));
            }

            for (; it < maxIter; ++it) {
                Cxd zref = cxd_from_double2(refOrbit[it]);
                Cxd zcur = cxd_add(zref, delta);
                if (cxd_abs2(zcur) > 4.0) {
                    escaped = true;
                    z = {(float)zcur.x, (float)zcur.y};
                    break;
                }

                // delta_{n+1} = 2*zref*delta + delta^2 + dc
                Cxd term1 = cxd_scale(cxd_mul(zref, delta), 2.0);
                Cxd term2 = cxd_mul(delta, delta);
                delta = cxd_add(cxd_add(term1, term2), dc);

                if (!isfinite(delta.x) || !isfinite(delta.y)) {
                    escaped = true;
                    break;
                }

                z = {(float)zcur.x, (float)zcur.y};
            }
        } else {
            // Standard direct escape-time iteration shared with the probe sampler.
            if (useFP64) {
                EscapeTimeDirectState<Cxd> state = InitEscapeTimeDirectState(ft, coordD);
                const double powerFloat = (double)params.multibrot_power_float;
                const Cxd lambdaConstD{(double)params.lambda_real, (double)params.lambda_imag};
                const Cxd phoenixPD{(double)params.phoenix_p_real, (double)params.phoenix_p_imag};

                for (; it < maxIter; ++it) {
                    StepEscapeTimeDirectState(ft, powerFloat, params.multibrot_power, lambdaConstD, phoenixPD, &state);
                    if (cxd_abs2(state.z) > DirectEscapeTimeRadiusSquared<double>()) {
                        escaped = true;
                        break;
                    }
                    if (!isfinite(state.z.x) || !isfinite(state.z.y) || !isfinite(state.z_prev.x) || !isfinite(state.z_prev.y)) {
                        escaped = true;
                        break;
                    }
                }
                z = {(float)state.z.x, (float)state.z.y};
            } else {
                EscapeTimeDirectState<Cx> state = InitEscapeTimeDirectState(ft, coord);
                const float powerFloat = params.multibrot_power_float;
                const Cx lambdaConst{params.lambda_real, params.lambda_imag};
                const Cx phoenixP{params.phoenix_p_real, params.phoenix_p_imag};

                for (; it < maxIter; ++it) {
                    StepEscapeTimeDirectState(ft, powerFloat, params.multibrot_power, lambdaConst, phoenixP, &state);
                    if (cx_abs2(state.z) > DirectEscapeTimeRadiusSquared<float>()) {
                        escaped = true;
                        break;
                    }
                    if (!isfinite(state.z.x) || !isfinite(state.z.y) || !isfinite(state.z_prev.x) || !isfinite(state.z_prev.y)) {
                        escaped = true;
                        break;
                    }
                }
                z = state.z;
            }
        }
    }

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

void CleanupFractalCUDA() {
    if (g_cached.d_rgba) { cudaFree(g_cached.d_rgba); g_cached.d_rgba = nullptr; }
    if (g_cached.d_mask) { cudaFree(g_cached.d_mask); g_cached.d_mask = nullptr; }
    if (g_cached.d_itersSum) { cudaFree(g_cached.d_itersSum); g_cached.d_itersSum = nullptr; }
    if (g_cached.d_refOrbit) { cudaFree(g_cached.d_refOrbit); g_cached.d_refOrbit = nullptr; }
    g_cached.w = 0;
    g_cached.h = 0;
    cudaDeviceReset();
}
