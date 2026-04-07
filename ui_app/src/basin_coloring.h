#pragma once

#include "fractal_types.h"

#include <cmath>

#if defined(__CUDACC__)
#define BASIN_COLORING_HD __host__ __device__
#else
#define BASIN_COLORING_HD
#endif

BASIN_COLORING_HD inline constexpr int ResolvePolynomialRootCount(PolyKind polyKind) {
    return polyKind == PolyKind::z3_minus_1 ? 3 : (polyKind == PolyKind::z4_minus_1 ? 4 : 0);
}

template <typename Complex>
BASIN_COLORING_HD inline int NearestRootIndexUnitRoots(Complex z, int n) {
    const double pi = 3.14159265358979323846;
    const double angle = atan2((double)z.y, (double)z.x);
    const double t = (angle + pi) / (2.0 * pi);
    int k = (int)floor(t * n + 0.5) % n;
    if (k < 0) k += n;
    return k;
}

template <typename Complex>
BASIN_COLORING_HD inline int NearestRootIndexList(Complex z, const Float2* roots, int n) {
    int best = 0;
    double bestD2 = 1.0e30;
    for (int i = 0; i < n; ++i) {
        const double dx = (double)z.x - (double)roots[i].x;
        const double dy = (double)z.y - (double)roots[i].y;
        const double d2 = dx * dx + dy * dy;
        if (d2 < bestD2) {
            bestD2 = d2;
            best = i;
        }
    }
    return best;
}

template <typename Color>
BASIN_COLORING_HD inline Color PaletteRoot(int idx) {
    const Color colors[8] = {
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

template <typename Color>
BASIN_COLORING_HD inline Color PaletteJoyRoot(int idx) {
    const Color warm[8] = {
        {255, 140, 80, 255},
        {255, 205, 70, 255},
        {255, 90, 90, 255},
        {255, 130, 200, 255},
        {255, 165, 90, 255},
        {255, 95, 190, 255},
        {255, 185, 95, 255},
        {255, 235, 170, 255},
    };
    return warm[idx % 8];
}

#undef BASIN_COLORING_HD