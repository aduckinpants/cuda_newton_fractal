#pragma once

#include "fractal_types.h"

#include <cmath>

#if defined(__CUDACC__)
#define ESCAPE_TIME_SPECIALIZED_HD __host__ __device__
#else
#define ESCAPE_TIME_SPECIALIZED_HD
#endif

enum class SpecializedEscapeStepResult : int {
    advanced = 0,
    pole = 1,
};

struct McMullenPresetConfig {
    int m;
    int n;
    float lambda;
};

ESCAPE_TIME_SPECIALIZED_HD inline float SpecializedEscapeCos(float value) { return cosf(value); }
ESCAPE_TIME_SPECIALIZED_HD inline double SpecializedEscapeCos(double value) { return cos(value); }
ESCAPE_TIME_SPECIALIZED_HD inline float SpecializedEscapeSin(float value) { return sinf(value); }
ESCAPE_TIME_SPECIALIZED_HD inline double SpecializedEscapeSin(double value) { return sin(value); }
ESCAPE_TIME_SPECIALIZED_HD inline float SpecializedEscapeCosh(float value) { return coshf(value); }
ESCAPE_TIME_SPECIALIZED_HD inline double SpecializedEscapeCosh(double value) { return cosh(value); }
ESCAPE_TIME_SPECIALIZED_HD inline float SpecializedEscapeSinh(float value) { return sinhf(value); }
ESCAPE_TIME_SPECIALIZED_HD inline double SpecializedEscapeSinh(double value) { return sinh(value); }

template <typename Complex>
ESCAPE_TIME_SPECIALIZED_HD inline Complex SpecializedEscapeAdd(Complex left, Complex right) {
    return {left.x + right.x, left.y + right.y};
}

template <typename Complex>
ESCAPE_TIME_SPECIALIZED_HD inline Complex SpecializedEscapeSub(Complex left, Complex right) {
    return {left.x - right.x, left.y - right.y};
}

template <typename Complex>
ESCAPE_TIME_SPECIALIZED_HD inline Complex SpecializedEscapeMul(Complex left, Complex right) {
    return {left.x * right.x - left.y * right.y, left.x * right.y + left.y * right.x};
}

template <typename Complex, typename Scalar>
ESCAPE_TIME_SPECIALIZED_HD inline Complex SpecializedEscapeScale(Complex value, Scalar scale) {
    return {value.x * scale, value.y * scale};
}

template <typename Complex>
ESCAPE_TIME_SPECIALIZED_HD inline auto SpecializedEscapeAbs2(Complex value) -> decltype(value.x * value.x + value.y * value.y) {
    return value.x * value.x + value.y * value.y;
}

template <typename Complex>
ESCAPE_TIME_SPECIALIZED_HD inline Complex SpecializedEscapePowInt(Complex value, int power) {
    using Scalar = decltype(value.x);
    Complex result{static_cast<Scalar>(1), static_cast<Scalar>(0)};
    while (power > 0) {
        if ((power & 1) != 0) result = SpecializedEscapeMul(result, value);
        value = SpecializedEscapeMul(value, value);
        power >>= 1;
    }
    return result;
}

template <typename Complex>
ESCAPE_TIME_SPECIALIZED_HD inline Complex SpecializedEscapeCosPi(Complex value) {
    using Scalar = decltype(value.x);
    const Scalar pi = static_cast<Scalar>(3.14159265358979323846);
    const Scalar px = pi * value.x;
    const Scalar py = pi * value.y;
    return {
        SpecializedEscapeCos(px) * SpecializedEscapeCosh(py),
        -SpecializedEscapeSin(px) * SpecializedEscapeSinh(py),
    };
}

ESCAPE_TIME_SPECIALIZED_HD inline constexpr bool UsesSpecializedEscapeTimeFormula(FractalType fractalType) {
    return fractalType == FractalType::mcmullen || fractalType == FractalType::collatz;
}

ESCAPE_TIME_SPECIALIZED_HD inline constexpr McMullenPresetConfig ResolveMcMullenPresetConfig(McMullenPreset preset) {
    switch (preset) {
    case McMullenPreset::z2_z2:
        return {2, 2, -0.10f};
    case McMullenPreset::z4_z2:
        return {4, 2, -0.05f};
    case McMullenPreset::z3_z2:
        return {3, 2, -0.10f};
    case McMullenPreset::z3_z3:
    default:
        return {3, 3, -0.125f};
    }
}

template <typename Complex>
ESCAPE_TIME_SPECIALIZED_HD inline SpecializedEscapeStepResult StepMcMullenEscapeState(
    const McMullenPresetConfig& config,
    Complex* ioZ) {
    if (!ioZ) return SpecializedEscapeStepResult::pole;

    using Scalar = decltype(ioZ->x);

    const Scalar zAbs2 = SpecializedEscapeAbs2(*ioZ);
    if (zAbs2 < static_cast<Scalar>(1.0e-20)) {
        return SpecializedEscapeStepResult::pole;
    }

    const Complex zPow = SpecializedEscapePowInt(*ioZ, config.m);
    const Complex zInv{ioZ->x / zAbs2, -ioZ->y / zAbs2};
    const Complex zInvPow = SpecializedEscapePowInt(zInv, config.n);
    *ioZ = SpecializedEscapeAdd(zPow, SpecializedEscapeScale(zInvPow, static_cast<Scalar>(config.lambda)));
    return SpecializedEscapeStepResult::advanced;
}

template <typename Complex>
ESCAPE_TIME_SPECIALIZED_HD inline void StepCollatzEscapeState(Complex* ioZ) {
    if (!ioZ) return;

    using Scalar = decltype(ioZ->x);

    const Complex cosPi = SpecializedEscapeCosPi(*ioZ);
    const Complex linear{static_cast<Scalar>(2) + static_cast<Scalar>(5) * ioZ->x, static_cast<Scalar>(5) * ioZ->y};
    const Complex affine{static_cast<Scalar>(2) + static_cast<Scalar>(7) * ioZ->x, static_cast<Scalar>(7) * ioZ->y};
    *ioZ = SpecializedEscapeScale(
        SpecializedEscapeSub(affine, SpecializedEscapeMul(linear, cosPi)),
        static_cast<Scalar>(0.25));
}

ESCAPE_TIME_SPECIALIZED_HD inline constexpr float SpecializedEscapeRadiusSquared() {
    return 10000.0f;
}

#undef ESCAPE_TIME_SPECIALIZED_HD