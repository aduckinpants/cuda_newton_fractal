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

BASIN_COLORING_HD inline double BasinDistanceSquared(double x0, double y0, double x1, double y1) {
    const double dx = x0 - x1;
    const double dy = y0 - y1;
    return dx * dx + dy * dy;
}

BASIN_COLORING_HD inline Double2 UnitRootCoord(int index, int count) {
    const double angle = (2.0 * 3.14159265358979323846 * static_cast<double>(index)) / static_cast<double>(count);
    return {std::cos(angle), std::sin(angle)};
}

template <typename Complex>
BASIN_COLORING_HD inline double BasinDistanceSquaredToRoot(Complex z, const Float2& root) {
    return BasinDistanceSquared(
        static_cast<double>(z.x),
        static_cast<double>(z.y),
        static_cast<double>(root.x),
        static_cast<double>(root.y));
}

template <typename Scalar>
BASIN_COLORING_HD inline int NearestRootIndexUnitRootsImpl(Scalar x, Scalar y, int n);

template <>
BASIN_COLORING_HD inline int NearestRootIndexUnitRootsImpl<float>(float x, float y, int n) {
    const float pi = 3.14159265358979323846f;
    const float twoPi = 2.0f * pi;
    float angle = atan2f(y, x);
    if (angle < 0.0f) angle += twoPi;
    const int naturalIndex = (int)floorf((angle / twoPi) * n + 0.5f) % n;
    int k = (naturalIndex + ((n + 1) / 2)) % n;
    if (k < 0) k += n;
    return k;
}

template <>
BASIN_COLORING_HD inline int NearestRootIndexUnitRootsImpl<double>(double x, double y, int n) {
    const double pi = 3.14159265358979323846;
    const double twoPi = 2.0 * pi;
    double angle = atan2(y, x);
    if (angle < 0.0) angle += twoPi;
    const int naturalIndex = (int)floor((angle / twoPi) * n + 0.5) % n;
    int k = (naturalIndex + ((n + 1) / 2)) % n;
    if (k < 0) k += n;
    return k;
}

template <typename Complex>
BASIN_COLORING_HD inline int NearestRootIndexUnitRoots(Complex z, int n) {
    return NearestRootIndexUnitRootsImpl<decltype(z.x)>(z.x, z.y, n);
}

template <typename Complex>
BASIN_COLORING_HD inline int NearestRootIndexList(Complex z, const Float2* roots, int n) {
    int best = 0;
    double bestD2 = 1.0e30;
    for (int i = 0; i < n; ++i) {
        const double d2 = BasinDistanceSquaredToRoot(z, roots[i]);
        if (d2 < bestD2) {
            bestD2 = d2;
            best = i;
        }
    }
    return best;
}

template <typename Scalar>
BASIN_COLORING_HD inline double NearestRootDistanceSquaredUnitRootsImpl(Scalar x, Scalar y, int n) {
    if (n <= 0) {
        return 1.0e30;
    }
    double bestD2 = 1.0e30;
    for (int i = 0; i < n; ++i) {
        const Double2 root = UnitRootCoord(i, n);
        const double d2 = BasinDistanceSquared(static_cast<double>(x), static_cast<double>(y), root.x, root.y);
        if (d2 < bestD2) {
            bestD2 = d2;
        }
    }
    return bestD2;
}

template <typename Complex>
BASIN_COLORING_HD inline double NearestRootDistanceSquaredUnitRoots(Complex z, int n) {
    return NearestRootDistanceSquaredUnitRootsImpl<decltype(z.x)>(z.x, z.y, n);
}

template <typename Complex>
BASIN_COLORING_HD inline double NearestRootDistanceSquaredList(Complex z, const Float2* roots, int n) {
    if (!roots || n <= 0) {
        return 1.0e30;
    }
    double bestD2 = 1.0e30;
    for (int i = 0; i < n; ++i) {
        const double d2 = BasinDistanceSquaredToRoot(z, roots[i]);
        if (d2 < bestD2) {
            bestD2 = d2;
        }
    }
    return bestD2;
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