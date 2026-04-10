#pragma once
// Shared CUDA device math helpers for fractal iteration.
// Include from any .cu file that needs complex-number arithmetic.

#include "polynomial_eval_real_coeffs.h"

#include <cuda_runtime.h>
#include <math_constants.h>

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

__device__ __forceinline__ Cxd cxd_from_double2(double2 v) { return {v.x, v.y}; }

__device__ __forceinline__ Cx unit_root_k(int k, int n) {
    float a = (2.0f * CUDART_PI_F) * ((float)k / (float)max(1, n));
    return {cosf(a), sinf(a)};
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
