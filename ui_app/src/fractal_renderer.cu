#include "fractal_types.h"
#include "fractal_family_rules.h"
#include "explaino_seed_curve.h"

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

__device__ __forceinline__ void poly_eval_real_coeffs_deg4(const float coeffs[5], Cx z, Cx* outP, Cx* outDp) {
    // P(z) = c0 + c1 z + c2 z^2 + c3 z^3 + c4 z^4
    // P'(z) = c1 + 2 c2 z + 3 c3 z^2 + 4 c4 z^3
    Cx z2 = cx_mul(z, z);
    Cx z3 = cx_mul(z2, z);
    Cx z4 = cx_mul(z2, z2);

    Cx P{coeffs[0], 0.0f};
    P = cx_add(P, cx_scale(z, coeffs[1]));
    P = cx_add(P, cx_scale(z2, coeffs[2]));
    P = cx_add(P, cx_scale(z3, coeffs[3]));
    P = cx_add(P, cx_scale(z4, coeffs[4]));

    Cx dP{coeffs[1], 0.0f};
    dP = cx_add(dP, cx_scale(z, 2.0f * coeffs[2]));
    dP = cx_add(dP, cx_scale(z2, 3.0f * coeffs[3]));
    dP = cx_add(dP, cx_scale(z3, 4.0f * coeffs[4]));

    *outP = P;
    *outDp = dP;
}

__device__ __forceinline__ void poly_eval_real_coeffs_deg4_d2(const float coeffs[5], Cx z, Cx* outP, Cx* outDp, Cx* outD2p) {
    // P(z) = c0 + c1 z + c2 z^2 + c3 z^3 + c4 z^4
    // P'(z) = c1 + 2 c2 z + 3 c3 z^2 + 4 c4 z^3
    // P''(z) = 2 c2 + 6 c3 z + 12 c4 z^2
    Cx z2 = cx_mul(z, z);
    Cx z3 = cx_mul(z2, z);
    Cx z4 = cx_mul(z2, z2);

    Cx P{coeffs[0], 0.0f};
    P = cx_add(P, cx_scale(z, coeffs[1]));
    P = cx_add(P, cx_scale(z2, coeffs[2]));
    P = cx_add(P, cx_scale(z3, coeffs[3]));
    P = cx_add(P, cx_scale(z4, coeffs[4]));

    Cx dP{coeffs[1], 0.0f};
    dP = cx_add(dP, cx_scale(z, 2.0f * coeffs[2]));
    dP = cx_add(dP, cx_scale(z2, 3.0f * coeffs[3]));
    dP = cx_add(dP, cx_scale(z3, 4.0f * coeffs[4]));

    Cx d2P{2.0f * coeffs[2], 0.0f};
    d2P = cx_add(d2P, cx_scale(z, 6.0f * coeffs[3]));
    d2P = cx_add(d2P, cx_scale(z2, 12.0f * coeffs[4]));

    *outP = P;
    *outDp = dP;
    *outD2p = d2P;
}

__device__ __forceinline__ int nearest_root_index_unit_roots(Cx z, int n) {
    // For z^n - 1, roots are exp(i * 2pi k / n).
    float angle = atan2f(z.y, z.x);
    float t = (angle + CUDART_PI_F) / (2.0f * CUDART_PI_F); // [0,1)
    int k = (int)floorf(t * n + 0.5f) % n;
    if (k < 0) k += n;
    return k;
}

__device__ __forceinline__ Cx unit_root_k(int k, int n) {
    float a = (2.0f * CUDART_PI_F) * ((float)k / (float)max(1, n));
    return {cosf(a), sinf(a)};
}

__device__ __forceinline__ int nearest_root_index_list(Cx z, const Float2* roots, int n) {
    int best = 0;
    float bestD2 = 1.0e30f;
    for (int i = 0; i < n; ++i) {
        float dx = z.x - roots[i].x;
        float dy = z.y - roots[i].y;
        float d2 = dx * dx + dy * dy;
        if (d2 < bestD2) {
            bestD2 = d2;
            best = i;
        }
    }
    return best;
}

__device__ __forceinline__ uchar4 palette_root(int idx, int n) {
    // Simple distinct palette (wrap)
    const uchar4 colors[8] = {
        {255, 64, 64, 255},
        {64, 255, 64, 255},
        {64, 128, 255, 255},
        {255, 255, 64, 255},
        {255, 64, 255, 255},
        {64, 255, 255, 255},
        {255, 128, 64, 255},
        {192, 192, 192, 255},
    };
    return colors[idx % 8];
}

__device__ __forceinline__ uchar4 palette_joy_root(int idx, int n) {
    // Warm, happy palette tuned for "many success pits".
    // Keep it simple and distinct per root.
    const uchar4 warm[8] = {
        {255, 140, 80, 255},
        {255, 205, 70, 255},
        {255, 90, 90, 255},
        {255, 130, 200, 255},
        {255, 165, 90, 255},
        {255, 95, 190, 255},
        {255, 185, 95, 255},
        {255, 235, 170, 255},
    };
    (void)n;
    return warm[idx % 8];
}

__device__ __forceinline__ uchar4 mul_rgb(uchar4 c, float s) {
    s = fmaxf(0.0f, fminf(1.5f, s));
    int r = (int)roundf((float)c.x * s);
    int g = (int)roundf((float)c.y * s);
    int b = (int)roundf((float)c.z * s);
    r = max(0, min(255, r));
    g = max(0, min(255, g));
    b = max(0, min(255, b));
    return {(unsigned char)r, (unsigned char)g, (unsigned char)b, 255};
}

__device__ __forceinline__ unsigned char tone_map(unsigned char v, float exposure) {
    float x = (v / 255.0f) * exposure;
    x = 1.0f - expf(-x);
    return (unsigned char)(fminf(1.0f, x) * 255.0f);
}

__device__ __forceinline__ uchar4 apply_exposure(uchar4 c, float exposure) {
    float e = fmaxf(0.0f, exposure);
    return {tone_map(c.x, e), tone_map(c.y, e), tone_map(c.z, e), 255};
}

__device__ __forceinline__ uchar4 tint_rgb(uchar4 c, float tr, float tg, float tb) {
    return {(unsigned char)fminf(255.0f, fmaxf(0.0f, (float)c.x * tr)),
            (unsigned char)fminf(255.0f, fmaxf(0.0f, (float)c.y * tg)),
            (unsigned char)fminf(255.0f, fmaxf(0.0f, (float)c.z * tb)),
            c.w};
}

__device__ __forceinline__ uchar4 saturate_rgb(uchar4 c, float sat) {
    float lum = 0.299f * (float)c.x + 0.587f * (float)c.y + 0.114f * (float)c.z;
    return {(unsigned char)fminf(255.0f, fmaxf(0.0f, lum + sat * ((float)c.x - lum))),
            (unsigned char)fminf(255.0f, fmaxf(0.0f, lum + sat * ((float)c.y - lum))),
            (unsigned char)fminf(255.0f, fmaxf(0.0f, lum + sat * ((float)c.z - lum))),
            c.w};
}

__device__ __forceinline__ uchar4 contrast_rgb(uchar4 c, float con) {
    return {(unsigned char)fminf(255.0f, fmaxf(0.0f, 128.0f + con * ((float)c.x - 128.0f))),
            (unsigned char)fminf(255.0f, fmaxf(0.0f, 128.0f + con * ((float)c.y - 128.0f))),
            (unsigned char)fminf(255.0f, fmaxf(0.0f, 128.0f + con * ((float)c.z - 128.0f))),
            c.w};
}

__device__ __forceinline__ Cx cx_abs_components(Cx a) {
    return {fabsf(a.x), fabsf(a.y)};
}

__device__ __forceinline__ Cx cx_pow_int(Cx z, int p) {
    // Integer power >= 2.
    Cx r = z;
    for (int i = 1; i < p; ++i) r = cx_mul(r, z);
    return r;
}

__device__ __forceinline__ Cxd cxd_add(Cxd a, Cxd b) { return {a.x + b.x, a.y + b.y}; }
__device__ __forceinline__ Cxd cxd_sub(Cxd a, Cxd b) { return {a.x - b.x, a.y - b.y}; }
__device__ __forceinline__ Cxd cxd_mul(Cxd a, Cxd b) { return {a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x}; }
__device__ __forceinline__ Cxd cxd_scale(Cxd a, double s) { return {a.x * s, a.y * s}; }
__device__ __forceinline__ double cxd_abs2(Cxd a) { return a.x * a.x + a.y * a.y; }
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

    int maxIter = max(1, params.max_iter);
    float eps = fmaxf(1e-12f, params.epsilon);

    int it = 0;
    float pAbs = 0.0f;
    bool converged = false;
    bool escaped = false;

    Cx z{0.0f, 0.0f};
    Cx cConst{0.0f, 0.0f};

    FractalType ft = view.fractal_type;
    if (ft == FractalType::newton) {
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

            if (!isfinite(z.x) || !isfinite(z.y)) {
                z = {0.0f, 0.0f};
                break;
            }
        }

        converged = (pAbs < eps);
    } else if (ft == FractalType::explaino || ft == FractalType::explaino_dual || ft == FractalType::explaino_mult) {
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
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

            if (!isfinite(z.x) || !isfinite(z.y)) {
                z = {0.0f, 0.0f};
                break;
            }
        }

        converged = (pAbs < eps);
    } else if (ft == FractalType::explaino_fp) {
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
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

            if (!isfinite(z.x) || !isfinite(z.y)) {
                z = {0.0f, 0.0f};
                break;
            }
        }

        converged = (pAbs < eps);
        if (!converged) {
            int nRoots = 0;
            if (params.poly_kind == PolyKind::z3_minus_1) nRoots = 3;
            if (params.poly_kind == PolyKind::z4_minus_1) nRoots = 4;

            if (nRoots > 0) {
                int idx = nearest_root_index_unit_roots(z, nRoots);
                z = unit_root_k(idx, nRoots);
            } else if (params.explaino_root_count > 0) {
                int idx = nearest_root_index_list(z, params.explaino_roots, params.explaino_root_count);
                z = {params.explaino_roots[idx].x, params.explaino_roots[idx].y};
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
        // Treat as escape-time family for coloring; points that do not escape are interior.
        // Parameterization choice (best judgment): z0=0, c=coord (Mandelbrot-like c-plane). This yields stable, rich structure.
        z = {0.0f, 0.0f};
        cConst = coord;

        float alpha = params.nova_alpha;
        // Device-side safety: avoid NaN propagation if host validation is bypassed.
        if (!(alpha > 0.0f) || !(alpha <= 2.0f) || !isfinite(alpha)) {
            escaped = true;
        } else {
            for (; it < maxIter; ++it) {
                Cx P, dP;

                float coeffs[5];
                #pragma unroll
                for (int k = 0; k < 5; k++) coeffs[k] = params.poly_coeffs[k];

                poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);

                pAbs = cx_abs(P);
                if (pAbs < eps) {
                    converged = true;
                    break;
                }

                float dAbs2 = cx_abs2(dP);
                if (dAbs2 < 1e-20f) {
                    // Derivative too small: treat as non-convergent and stop.
                    break;
                }

                Cx step = cx_div(P, dP);
                // z = z - alpha*step + c
                z = cx_add(cx_sub(z, cx_scale(step, alpha)), cConst);

                if (!isfinite(z.x) || !isfinite(z.y)) {
                    escaped = true;
                    break;
                }

                if (cx_abs2(z) > 4.0f) {
                    escaped = true;
                    break;
                }
            }
        }
    } else if (ft == FractalType::explaino_halley) {
        // Halley's method: z_{n+1} = z - 2 f(z) f'(z) / (2 f'(z)^2 - f(z) f''(z))
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        z = explaino_warp_start(coord, seed, phase, strength);

        for (; it < maxIter; ++it) {
            Cx P, dP, d2P;

            float coeffs[5];
            #pragma unroll
            for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];

            poly_eval_real_coeffs_deg4_d2(coeffs, z, &P, &dP, &d2P);

            pAbs = cx_abs(P);
            if (pAbs < eps) break;

            // Halley denominator: 2 f'(z)^2 - f(z) f''(z)
            Cx dp2 = cx_mul(dP, dP);
            Cx fd2 = cx_mul(P, d2P);
            Cx denom = cx_sub(cx_scale(dp2, 2.0f), fd2);

            float denomAbs2 = cx_abs2(denom);
            if (denomAbs2 < 1e-20f) break;

            // Halley numerator: 2 f(z) f'(z)
            Cx numer = cx_scale(cx_mul(P, dP), 2.0f);
            Cx step = cx_div(numer, denom);
            z = cx_sub(z, cx_scale(step, userDamp));

            if (!isfinite(z.x) || !isfinite(z.y)) {
                z = {0.0f, 0.0f};
                break;
            }
        }

        converged = (pAbs < eps);
    } else if (ft == FractalType::explaino_phoenix) {
        // Explaino-Phoenix: seeded Newton with previous-z memory term.
        // z_{n+1} = z_n - damping * f(z_n)/f'(z_n) + p * z_{n-1}
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
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

            if (!isfinite(z.x) || !isfinite(z.y)) {
                z = {0.0f, 0.0f};
                break;
            }
        }

        converged = (pAbs < eps);
    } else if (ft == FractalType::explaino_transcendental) {
        // Explaino-Transcendental: seeded Newton applied to transcendental functions.
        // sin(z): f=sin(z), f'=cos(z)
        // exp(z)-1: f=exp(z)-1, f'=exp(z)
        // cosh(z): f=cosh(z), f'=sinh(z)
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        z = explaino_warp_start(coord, seed, phase, strength);

        TranscendentalFunc tf = params.transcendental_func;

        for (; it < maxIter; ++it) {
            Cx F, dF;

            if (tf == TranscendentalFunc::f_sin) {
                // sin(z) = sin(x)cosh(y) + i cos(x)sinh(y)
                float sx = sinf(z.x), cx_ = cosf(z.x);
                float shy = sinhf(z.y), chy = coshf(z.y);
                F = {sx * chy, cx_ * shy};
                dF = {cx_ * chy, -sx * shy};
            } else if (tf == TranscendentalFunc::f_exp_minus_1) {
                // exp(z)-1 = exp(x)(cos(y) + i sin(y)) - 1
                float ex = expf(z.x);
                float cy_ = cosf(z.y), sy_ = sinf(z.y);
                F = {ex * cy_ - 1.0f, ex * sy_};
                dF = {ex * cy_, ex * sy_};
            } else {
                // cosh(z) = cosh(x)cos(y) + i sinh(x)sin(y)
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

            if (!isfinite(z.x) || !isfinite(z.y)) {
                z = {0.0f, 0.0f};
                break;
            }
        }

        converged = (pAbs < eps);
    } else if (ft == FractalType::explaino_inertial) {
        // Explaino-Inertial: seeded Newton with previous-step momentum.
        // z_{n+1} = z_n - damping * f(z_n)/f'(z_n) + beta * (z_{n-1} - z_{n-2})
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        float beta = params.momentum_beta;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
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

            if (!isfinite(z.x) || !isfinite(z.y)) {
                z = {0.0f, 0.0f};
                break;
            }
        }

        converged = (pAbs < eps);
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
    } else if (ft == FractalType::explaino_rational) {
        // Explaino-Rational: Newton on f(z) = P(z) + cluster_radius/z.
        // The rational perturbation adds a pole at the origin, producing dust/web geometry.
        float phase = view.explaino_phase;
        float strength = params.explaino_warp_strength;
        float userDamp = params.explaino_damping;
        float ratAlpha = params.explaino_cluster_radius;
        double combinedSeed = params.explaino_seed + (double)view.explaino_seed_drift;
        double seed = LogisticAreaUToSeed(combinedSeed);
        z = explaino_warp_start(coord, seed, phase, strength);

        for (; it < maxIter; ++it) {
            float zAbs2 = cx_abs2(z);
            if (zAbs2 < 1e-20f) break; // at pole, bail

            Cx P, dP;
            float coeffs[5];
            #pragma unroll
            for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
            poly_eval_real_coeffs_deg4(coeffs, z, &P, &dP);

            // f(z) = P(z) + alpha/z
            Cx zInv = cx_div({1.0f, 0.0f}, z);
            Cx F = cx_add(P, cx_scale(zInv, ratAlpha));
            // f'(z) = P'(z) - alpha/z^2
            Cx zInv2 = cx_mul(zInv, zInv);
            Cx dF = cx_sub(dP, cx_scale(zInv2, ratAlpha));

            pAbs = cx_abs(F);
            if (pAbs < eps) break;

            float dAbs2 = cx_abs2(dF);
            if (dAbs2 < 1e-20f) break;

            Cx step = cx_div(F, dF);
            z = cx_sub(z, cx_scale(step, userDamp));

            if (!isfinite(z.x) || !isfinite(z.y)) {
                z = {0.0f, 0.0f};
                break;
            }
        }

        converged = (pAbs < eps);
    } else if (ft == FractalType::multicorn) {
        // Multicorn/Tricorn: z_{n+1} = conj(z)^p + c, escape-time family.
        z = {0.0f, 0.0f};
        cConst = coord;
        int p = params.multibrot_power;

        for (; it < maxIter; ++it) {
            // Conjugate z: negate imaginary part.
            Cx zBar = {z.x, -z.y};
            Cx zp = cx_pow_int(zBar, p);
            z = cx_add(zp, cConst);

            if (cx_abs2(z) > 4.0f) {
                escaped = true;
                break;
            }
            if (!isfinite(z.x) || !isfinite(z.y)) {
                escaped = true;
                break;
            }
        }
    } else if (ft == FractalType::halley) {
        // Standalone Halley's method on standard polynomials (poly_kind/poly_coeffs).
        z = coord;

        for (; it < maxIter; ++it) {
            Cx P, dP, d2P;
            float coeffs[5];
            #pragma unroll
            for (int k = 0; k < 5; ++k) coeffs[k] = params.poly_coeffs[k];
            poly_eval_real_coeffs_deg4_d2(coeffs, z, &P, &dP, &d2P);

            pAbs = cx_abs(P);
            if (pAbs < eps) break;

            // Halley denominator: 2 f'(z)^2 - f(z) f''(z)
            Cx dp2 = cx_mul(dP, dP);
            Cx fd2 = cx_mul(P, d2P);
            Cx denom = cx_sub(cx_scale(dp2, 2.0f), fd2);

            float denomAbs2 = cx_abs2(denom);
            if (denomAbs2 < 1e-20f) break;

            // Halley numerator: 2 f(z) f'(z)
            Cx numer = cx_scale(cx_mul(P, dP), 2.0f);
            Cx step = cx_div(numer, denom);
            z = cx_sub(z, step);

            if (!isfinite(z.x) || !isfinite(z.y)) {
                z = {0.0f, 0.0f};
                break;
            }
        }

        converged = (pAbs < eps);
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
            // Standard escape-time iteration.
            // Mandelbrot/BurningShip/Multibrot use z0=0, c=coord.
            // Julia uses z0=coord, c=fixed.
            if (ft == FractalType::phoenix) {
                // Phoenix (V1): z_{n+1} = z_n^2 + c + p * z_{n-1}
                // Parameterization choice: c = coord (c-plane), z0 = 0, z_{-1} = 0.
                Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
                Cx zPrev{0.0f, 0.0f};
                z = {0.0f, 0.0f};
                cConst = coord;

                float r2 = 0.0f;
                for (; it < maxIter; ++it) {
                    Cx z2 = cx_mul(z, z);
                    Cx mem = cx_mul(pConst, zPrev);
                    Cx zNext = cx_add(cx_add(z2, cConst), mem);
                    zPrev = z;
                    z = zNext;

                    r2 = cx_abs2(z);
                    if (r2 > 4.0f) {
                        escaped = true;
                        break;
                    }
                    if (!isfinite(z.x) || !isfinite(z.y) || !isfinite(zPrev.x) || !isfinite(zPrev.y)) {
                        escaped = true;
                        break;
                    }
                }
            } else if (ft == FractalType::julia) {
                z = coord;
                cConst = {-0.7f, 0.27015f};
            } else {
                z = {0.0f, 0.0f};
                cConst = coord;
            }

            int p = params.multibrot_power;

            if (ft != FractalType::phoenix) {
                float r2 = 0.0f;
                for (; it < maxIter; ++it) {
                    if (ft == FractalType::burning_ship) {
                        Cx a = cx_abs_components(z);
                        Cx z2 = cx_mul(a, a);
                        z = cx_add(z2, cConst);
                    } else if (ft == FractalType::multibrot) {
                        Cx zp = cx_pow_int(z, p);
                        z = cx_add(zp, cConst);
                    } else {
                        // Mandelbrot / Julia default to z^2 + c
                        Cx z2 = cx_mul(z, z);
                        z = cx_add(z2, cConst);
                    }

                    r2 = cx_abs2(z);
                    if (r2 > 4.0f) {
                        escaped = true;
                        break;
                    }

                    if (!isfinite(z.x) || !isfinite(z.y)) {
                        escaped = true;
                        break;
                    }
                }
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
                int nRoots = 0;
                if (params.poly_kind == PolyKind::z3_minus_1) nRoots = 3;
                if (params.poly_kind == PolyKind::z4_minus_1) nRoots = 4;

                bool isExplainoFamily = IsExplainoFamily(ft);
                bool useCustomRoots = (nRoots == 0) && isExplainoFamily && (params.explaino_root_count > 0);

                if (nRoots > 0 || useCustomRoots) {
                    int idx = useCustomRoots ? nearest_root_index_list(z, params.explaino_roots, params.explaino_root_count)
                                             : nearest_root_index_unit_roots(z, nRoots);
                    uchar4 base = palette_joy_root(idx, nRoots);

                    // Brightness: faster convergence => brighter, but never gloomy.
                    float u = (float)it / (float)maxIter;
                    float bright = 1.0f - powf(fminf(1.0f, u), 0.65f);
                    bright = 0.35f + 0.90f * bright;

                    float edge = powf(fminf(1.0f, u), 0.85f);
                    float phase = isExplainoFamily ? view.explaino_phase : 0.0f;
                    float a = atan2f(z.y, z.x);
                    float stripe = 0.5f + 0.5f * sinf((float)it * 0.35f + a * 3.0f + phase);

                    uchar4 c0 = mul_rgb(base, bright);
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
                int nRoots = 0;
                if (params.poly_kind == PolyKind::z3_minus_1) nRoots = 3;
                if (params.poly_kind == PolyKind::z4_minus_1) nRoots = 4;

                bool isExplainoFamily = IsExplainoFamily(ft);
                bool useCustomRoots = (nRoots == 0) && isExplainoFamily && (params.explaino_root_count > 0);

                if (useCustomRoots) {
                    int idx = nearest_root_index_list(z, params.explaino_roots, params.explaino_root_count);
                    color = palette_root(idx, params.explaino_root_count);
                } else if (nRoots > 0) {
                    int idx = nearest_root_index_unit_roots(z, nRoots);
                    color = palette_root(idx, nRoots);
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
            }
        }
    } else {
        // Escape-time coloring.
        if (mode == ColoringMode::root_basin || mode == ColoringMode::joy_basins) {
            // Invalid: basin coloring is not valid for escape-time modes.
            // Fail-fast: show error color even for interior points.
            color = errorColor;
        } else if (!escaped) {
            color = {0, 0, 0, 255};
        } else if (mode == ColoringMode::iteration_count) {
            float t = (float)it / (float)maxIter;
            unsigned char v = (unsigned char)(fminf(1.0f, t) * 255.0f);
            color = {64, v, (unsigned char)(255 - v), 255};
        } else if (mode == ColoringMode::smooth_escape) {
            // Cyclic multi-stop palette for escape-time fractals.
            float mag = cx_abs(z);
            float log_zn = logf(fmaxf(mag, 1e-12f));

            float denom = logf(2.0f);
            int p = params.multibrot_power;
            if (ft == FractalType::multibrot && p > 1) denom = logf((float)p);

            float nu = (float)it + 1.0f - logf(fmaxf(log_zn / denom, 1e-12f)) / denom;

            // Map to a cycling band index rather than a linear 0-1 ramp.
            float band = nu * 0.025f; // period ~40 iterations
            float frac = band - floorf(band);

            // 5-stop palette: deep blue -> cyan -> gold -> orange -> deep blue
            // Each stop is a (r,g,b) triple in [0,1].
            const float stops[6][3] = {
                {0.00f, 0.03f, 0.20f},  // deep navy
                {0.05f, 0.35f, 0.65f},  // ocean blue
                {0.10f, 0.75f, 0.85f},  // cyan
                {0.95f, 0.85f, 0.25f},  // gold
                {0.90f, 0.45f, 0.10f},  // burnt orange
                {0.00f, 0.03f, 0.20f},  // wrap to deep navy
            };
            float u5 = frac * 5.0f;
            int seg = (int)u5;
            if (seg > 4) seg = 4;
            float segT = u5 - (float)seg;
            float rf = stops[seg][0] + (stops[seg+1][0] - stops[seg][0]) * segT;
            float gf = stops[seg][1] + (stops[seg+1][1] - stops[seg][1]) * segT;
            float bf = stops[seg][2] + (stops[seg+1][2] - stops[seg][2]) * segT;

            color = {(unsigned char)(fminf(1.0f, rf) * 255.0f),
                     (unsigned char)(fminf(1.0f, gf) * 255.0f),
                     (unsigned char)(fminf(1.0f, bf) * 255.0f), 255};
        }
    }

    color = apply_exposure(color, params.exposure);
    color = tint_rgb(color, params.color_tint_r, params.color_tint_g, params.color_tint_b);
    color = saturate_rgb(color, params.color_saturation);
    color = contrast_rgb(color, params.color_contrast);

    uint32_t rgba = (uint32_t)color.x | ((uint32_t)color.y << 8) | ((uint32_t)color.z << 16) | ((uint32_t)color.w << 24);
    outRGBA[py * width + px] = rgba;

    if (outMask) {
        outMask[py * width + px] = converged ? 255 : 0;
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
    FractalType hostRefType = FractalType::newton;
    double2 hostRefC0 = make_double2(0.0, 0.0);
    int hostRefLen = 0;
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
    g_cached.hostRefType = FractalType::newton;
    g_cached.hostRefC0 = make_double2(0.0, 0.0);
    g_cached.hostRefLen = 0;

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
    if (view.fractal_type == FractalType::nova) {
        if (!(params.nova_alpha > 0.0f) || !(params.nova_alpha <= 5.0f) || !std::isfinite(params.nova_alpha)) {
            if (outError) *outError = "nova_alpha must be in (0,5] and finite";
            return false;
        }
    }
    if (!IsColoringModeAllowedForFractal(view.fractal_type, params.coloring_mode)) {
        if (outError) *outError = "selected coloring_mode is not valid for fractal_type";
        return false;
    }
    if (view.fractal_type == FractalType::phoenix) {
        if (!std::isfinite(params.phoenix_p_real) || !std::isfinite(params.phoenix_p_imag)) {
            if (outError) *outError = "phoenix_p must be finite";
            return false;
        }
        // Explicit domain (V1): keep memory term bounded for stability.
        if (fabs(params.phoenix_p_real) > 1.0f || fabs(params.phoenix_p_imag) > 1.0f) {
            if (outError) *outError = "phoenix_p_real/imag must be in [-1,1]";
            return false;
        }
    }
    if (view.fractal_type == FractalType::multibrot || view.fractal_type == FractalType::multicorn) {
        if (params.multibrot_power < 2 || params.multibrot_power > 12) {
            if (outError) *outError = "multibrot_power must be in [2,12]";
            return false;
        }
    }
    if (IsExplainoFamily(view.fractal_type)) {
        if (!std::isfinite(params.explaino_seed)) {
            if (outError) *outError = "explaino_seed must be finite";
            return false;
        }
        if (!std::isfinite(params.explaino_seed_b)) {
            if (outError) *outError = "explaino_seed_b must be finite";
            return false;
        }
        if (!std::isfinite(params.explaino_mix) || params.explaino_mix < 0.0f || params.explaino_mix > 1.0f) {
            if (outError) *outError = "explaino_mix must be finite and in [0,1]";
            return false;
        }
        if (!std::isfinite(params.explaino_warp_strength) || params.explaino_warp_strength < 0.0f || params.explaino_warp_strength > 5.0f) {
            if (outError) *outError = "explaino_warp_strength must be finite and in [0,5]";
            return false;
        }
    }

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

    // Perturbation deep zoom (Mandelbrot/Julia): build and upload a double-precision reference orbit.
    // This is enabled only at high zoom to extend effective precision.
    const double kPerturbZoomThreshold = 1.0e10;
    const double kPerturbLog2Threshold = log(kPerturbZoomThreshold) / log(2.0);
    const bool wantPerturb = (view.fractal_type == FractalType::mandelbrot || view.fractal_type == FractalType::julia) && (view.log2_zoom >= kPerturbLog2Threshold);
    double2 refC0 = make_double2((double)view.center_hp_x, (double)view.center_hp_y);
    int refLen = 0;
    if (wantPerturb) {
        refLen = max(1, params.max_iter) + 1;
        if (!ensure_ref_orbit(refLen, outError)) return false;

        const bool sameKey =
            g_cached.hostRefValid &&
            g_cached.hostRefType == view.fractal_type &&
            g_cached.hostRefLen == refLen &&
            g_cached.hostRefC0.x == refC0.x &&
            g_cached.hostRefC0.y == refC0.y;

        if (!sameKey) {
            g_cached.hostRefOrbit.resize((size_t)refLen);

            double zx = 0.0;
            double zy = 0.0;
            double cx = refC0.x;
            double cy = refC0.y;

            if (view.fractal_type == FractalType::julia) {
                zx = refC0.x;
                zy = refC0.y;
                cx = -0.7;
                cy = 0.27015;
            }

            g_cached.hostRefOrbit[0] = make_double2(zx, zy);
            for (int i = 0; i < refLen - 1; ++i) {
                double nx = zx * zx - zy * zy + cx;
                double ny = 2.0 * zx * zy + cy;
                zx = nx;
                zy = ny;
                g_cached.hostRefOrbit[i + 1] = make_double2(zx, zy);
            }

            if (cudaMemcpy(g_cached.d_refOrbit, g_cached.hostRefOrbit.data(), (size_t)refLen * sizeof(double2), cudaMemcpyHostToDevice) != cudaSuccess) {
                if (outError) *outError = "cudaMemcpy failed for ref orbit";
                return false;
            }

            g_cached.hostRefValid = true;
            g_cached.hostRefType = view.fractal_type;
            g_cached.hostRefC0 = refC0;
            g_cached.hostRefLen = refLen;
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
        render,
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
