#pragma once

#include <cmath>
#include <cstdint>

#if defined(__CUDACC__)
#define EXPLAINO_SEED_CURVE_HD __host__ __device__
#else
#define EXPLAINO_SEED_CURVE_HD
#endif

EXPLAINO_SEED_CURVE_HD inline double ExplainoWedgeCumulativeRaw(double x) {
    const double p = 3.141592653589793;
    return (p - 1.0) * x * x * 0.5 - p * x * x * x / 3.0;
}

EXPLAINO_SEED_CURVE_HD inline double ExplainoWedgeTotalArea() {
    const double p = 3.141592653589793;
    const double x1 = 1.0 - 1.0 / p;
    return ExplainoWedgeCumulativeRaw(x1);
}

// H(t) = A(t*D) / A(D) for t in [0,1] -> smooth monotone S-curve tween.
// This is the wedge-area CDF: the normalized integral of (pi*x*(1-x) - x)
// over the pocket [0, D] where D = 1 - 1/pi.
EXPLAINO_SEED_CURVE_HD inline double ExplainoWedgeTween(double t) {
    if (t <= 0.0) return 0.0;
    if (t >= 1.0) return 1.0;
    const double p = 3.141592653589793;
    const double D = 1.0 - 1.0 / p;
    const double xw = t * D;
    return ExplainoWedgeCumulativeRaw(xw) / ExplainoWedgeTotalArea();
}

EXPLAINO_SEED_CURVE_HD inline double ExplainoAreaFractionToX(double u) {
    const double p = 3.141592653589793;
    const double x1 = 1.0 - 1.0 / p;
    if (u <= 0.0) return 0.0;
    if (u >= 1.0) return x1;

    const double target = u * ExplainoWedgeTotalArea();
    double lo = 0.0;
    double hi = x1;
    for (int i = 0; i < 60; ++i) {
        const double mid = 0.5 * (lo + hi);
        const double value = ExplainoWedgeCumulativeRaw(mid);
        if (value < target) lo = mid;
        else hi = mid;
    }
    return 0.5 * (lo + hi);
}

EXPLAINO_SEED_CURVE_HD inline std::uint64_t ExplainoSplitmix64(std::uint64_t z) {
    z += 0x9e3779b97f4a7c15ULL;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

EXPLAINO_SEED_CURVE_HD inline std::uint64_t ExplainoDoubleBits(double x) {
    union {
        double d;
        std::uint64_t u;
    } bits{};
    bits.d = x;
    return bits.u;
}

EXPLAINO_SEED_CURVE_HD inline std::uint64_t ExplainoHashLogisticOrbit(double x0, int iters) {
    std::uint64_t acc = 0x9e3779b97f4a7c15ULL;
    double x = x0;
    const double p = 3.141592653589793;
    for (int i = 0; i < iters; ++i) {
        x = p * x * (1.0 - x);
        acc = ExplainoSplitmix64(acc ^ ExplainoDoubleBits(x));
    }
    return acc;
}

EXPLAINO_SEED_CURVE_HD inline double LogisticAreaUToSeed(double s) {
    double u = s - floor(s);
    if (u < 0.0) u += 1.0;
    if (u >= 1.0) u = nextafter(1.0, 0.0);

    const double x = ExplainoAreaFractionToX(u);
    const std::uint64_t h = ExplainoHashLogisticOrbit(x, 12);
    const double denom = 18446744073709551615.0;
    return static_cast<double>(h) / denom;
}

EXPLAINO_SEED_CURVE_HD inline double ExplainoCombinedSeedToWarpSeed(double s) {
    const double fractionalSeed = LogisticAreaUToSeed(s);
    const double base = floor(s);
    const std::uint64_t h = ExplainoSplitmix64(
        ExplainoDoubleBits(fractionalSeed) ^
        ExplainoSplitmix64(ExplainoDoubleBits(base)));
    const double denom = 18446744073709551615.0;
    return static_cast<double>(h) / denom;
}

#undef EXPLAINO_SEED_CURVE_HD
