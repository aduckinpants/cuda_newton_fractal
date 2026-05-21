#pragma once

#include "fractal_types.h"

#include <cmath>

#if defined(__CUDACC__)
#define ESCAPE_TIME_DIRECT_HD __host__ __device__
#else
#define ESCAPE_TIME_DIRECT_HD
#endif

template <typename Complex>
struct EscapeTimeDirectState {
    Complex z{};
    Complex c{};
    Complex z_prev{};
};

ESCAPE_TIME_DIRECT_HD inline float EscapeTimeDirectSqrt(float value) { return sqrtf(value); }
ESCAPE_TIME_DIRECT_HD inline double EscapeTimeDirectSqrt(double value) { return sqrt(value); }
ESCAPE_TIME_DIRECT_HD inline float EscapeTimeDirectAtan2(float y, float x) { return atan2f(y, x); }
ESCAPE_TIME_DIRECT_HD inline double EscapeTimeDirectAtan2(double y, double x) { return atan2(y, x); }
ESCAPE_TIME_DIRECT_HD inline float EscapeTimeDirectPow(float base, float power) { return powf(base, power); }
ESCAPE_TIME_DIRECT_HD inline double EscapeTimeDirectPow(double base, double power) { return pow(base, power); }
ESCAPE_TIME_DIRECT_HD inline float EscapeTimeDirectExp(float value) { return expf(value); }
ESCAPE_TIME_DIRECT_HD inline double EscapeTimeDirectExp(double value) { return exp(value); }
ESCAPE_TIME_DIRECT_HD inline float EscapeTimeDirectLog(float value) { return logf(value); }
ESCAPE_TIME_DIRECT_HD inline double EscapeTimeDirectLog(double value) { return log(value); }
ESCAPE_TIME_DIRECT_HD inline float EscapeTimeDirectCos(float angle) { return cosf(angle); }
ESCAPE_TIME_DIRECT_HD inline double EscapeTimeDirectCos(double angle) { return cos(angle); }
ESCAPE_TIME_DIRECT_HD inline float EscapeTimeDirectSin(float angle) { return sinf(angle); }
ESCAPE_TIME_DIRECT_HD inline double EscapeTimeDirectSin(double angle) { return sin(angle); }

template <typename Scalar>
ESCAPE_TIME_DIRECT_HD inline Scalar EscapeTimeDirectAbs(Scalar value) {
    return value < static_cast<Scalar>(0) ? -value : value;
}

template <typename Complex>
ESCAPE_TIME_DIRECT_HD inline Complex EscapeTimeDirectAdd(Complex left, Complex right) {
    return {left.x + right.x, left.y + right.y};
}

template <typename Complex>
ESCAPE_TIME_DIRECT_HD inline Complex EscapeTimeDirectSub(Complex left, Complex right) {
    return {left.x - right.x, left.y - right.y};
}

template <typename Complex>
ESCAPE_TIME_DIRECT_HD inline Complex EscapeTimeDirectMul(Complex left, Complex right) {
    return {left.x * right.x - left.y * right.y, left.x * right.y + left.y * right.x};
}

template <typename Complex, typename Scalar>
ESCAPE_TIME_DIRECT_HD inline Complex EscapeTimeDirectScale(Complex value, Scalar scale) {
    return {value.x * scale, value.y * scale};
}

template <typename Complex>
ESCAPE_TIME_DIRECT_HD inline Complex EscapeTimeDirectDiv(Complex left, Complex right) {
    using Scalar = decltype(left.x);
    const Scalar denom = right.x * right.x + right.y * right.y;
    if (denom <= static_cast<Scalar>(1.0e-30)) {
        return {static_cast<Scalar>(1.0e20), static_cast<Scalar>(1.0e20)};
    }
    return {(left.x * right.x + left.y * right.y) / denom, (left.y * right.x - left.x * right.y) / denom};
}

template <typename Scalar>
ESCAPE_TIME_DIRECT_HD inline Scalar EscapeTimeDirectClamp(Scalar value, Scalar lo, Scalar hi) {
    return value < lo ? lo : (value > hi ? hi : value);
}

template <typename Complex>
ESCAPE_TIME_DIRECT_HD inline auto EscapeTimeDirectAbs2(Complex value) -> decltype(value.x * value.x + value.y * value.y) {
    return value.x * value.x + value.y * value.y;
}

template <typename Complex>
ESCAPE_TIME_DIRECT_HD inline Complex EscapeTimeDirectAbsComponents(Complex value) {
    return {EscapeTimeDirectAbs(value.x), EscapeTimeDirectAbs(value.y)};
}

template <typename Complex>
ESCAPE_TIME_DIRECT_HD inline Complex EscapeTimeDirectPowInt(Complex value, int power) {
    using Scalar = decltype(value.x);
    Complex result{static_cast<Scalar>(1), static_cast<Scalar>(0)};
    while (power > 0) {
        if ((power & 1) != 0) result = EscapeTimeDirectMul(result, value);
        value = EscapeTimeDirectMul(value, value);
        power >>= 1;
    }
    return result;
}

template <typename Complex, typename Scalar>
ESCAPE_TIME_DIRECT_HD inline Complex EscapeTimeDirectPowRealPrincipal(Complex value, Scalar power) {
    const Scalar radius2 = EscapeTimeDirectAbs2(value);
    if (radius2 <= static_cast<Scalar>(1.0e-30)) {
        return {static_cast<Scalar>(0), static_cast<Scalar>(0)};
    }

    const Scalar radius = EscapeTimeDirectSqrt(radius2);
    const Scalar theta = EscapeTimeDirectAtan2(value.y, value.x);
    const Scalar radiusPower = EscapeTimeDirectPow(radius, power);
    const Scalar angle = power * theta;
    return {radiusPower * EscapeTimeDirectCos(angle), radiusPower * EscapeTimeDirectSin(angle)};
}

template <typename Complex, typename Scalar>
ESCAPE_TIME_DIRECT_HD inline Complex EscapeTimeDirectPowComplexPrincipal(Complex value, Scalar realPower, Scalar imagPower) {
    const Scalar radius2 = EscapeTimeDirectAbs2(value);
    if (radius2 <= static_cast<Scalar>(1.0e-30)) {
        return {static_cast<Scalar>(0), static_cast<Scalar>(0)};
    }
    if (EscapeTimeDirectAbs(imagPower) <= static_cast<Scalar>(1.0e-12)) {
        return EscapeTimeDirectPowRealPrincipal(value, realPower);
    }

    const Scalar logRadius = static_cast<Scalar>(0.5) * EscapeTimeDirectLog(radius2);
    const Scalar theta = EscapeTimeDirectAtan2(value.y, value.x);
    const Scalar magnitude = EscapeTimeDirectExp(realPower * logRadius - imagPower * theta);
    const Scalar angle = imagPower * logRadius + realPower * theta;
    return {magnitude * EscapeTimeDirectCos(angle), magnitude * EscapeTimeDirectSin(angle)};
}

ESCAPE_TIME_DIRECT_HD inline constexpr bool UsesSharedEscapeTimeDirectFormula(FractalType fractalType) {
    return fractalType == FractalType::mandelbrot ||
        fractalType == FractalType::julia ||
        fractalType == FractalType::burning_ship ||
        fractalType == FractalType::multibrot ||
        fractalType == FractalType::phoenix ||
        fractalType == FractalType::multicorn ||
        fractalType == FractalType::lambda_map ||
        fractalType == FractalType::magnet ||
        fractalType == FractalType::spider ||
        fractalType == FractalType::celtic_mandelbrot ||
        fractalType == FractalType::perpendicular_burning_ship;
}

template <typename Complex>
ESCAPE_TIME_DIRECT_HD inline EscapeTimeDirectState<Complex> InitEscapeTimeDirectState(
    FractalType fractalType,
    Complex coord,
    Complex magnetSeed,
    Complex juliaConst) {
    using Scalar = decltype(coord.x);

    EscapeTimeDirectState<Complex> state{};
    if (fractalType == FractalType::julia) {
        state.z = coord;
        state.c = juliaConst;
    } else if (fractalType == FractalType::lambda_map) {
        state.z = coord;
        state.c = {static_cast<Scalar>(0), static_cast<Scalar>(0)};
    } else if (fractalType == FractalType::magnet) {
        state.z = magnetSeed;
        state.c = coord;
    } else {
        state.z = {static_cast<Scalar>(0), static_cast<Scalar>(0)};
        state.c = coord;
    }
    state.z_prev = {static_cast<Scalar>(0), static_cast<Scalar>(0)};
    return state;
}

template <typename Complex>
ESCAPE_TIME_DIRECT_HD inline EscapeTimeDirectState<Complex> InitEscapeTimeDirectState(
    FractalType fractalType,
    Complex coord,
    Complex magnetSeed) {
    using Scalar = decltype(coord.x);
    return InitEscapeTimeDirectState(
        fractalType,
        coord,
        magnetSeed,
        {static_cast<Scalar>(-0.7), static_cast<Scalar>(0.27015)});
}

template <typename Complex>
ESCAPE_TIME_DIRECT_HD inline EscapeTimeDirectState<Complex> InitEscapeTimeDirectState(FractalType fractalType, Complex coord) {
    using Scalar = decltype(coord.x);
    return InitEscapeTimeDirectState(fractalType, coord, {static_cast<Scalar>(0), static_cast<Scalar>(0)});
}

template <typename Complex, typename Scalar>
ESCAPE_TIME_DIRECT_HD inline void StepEscapeTimeDirectState(
    FractalType fractalType,
    Scalar multibrotPowerFloat,
    Scalar multibrotPowerImag,
    int multibrotPowerInt,
    Complex lambdaConst,
    Complex phoenixP,
    Scalar magnetRelaxation,
    EscapeTimeDirectState<Complex>* ioState) {
    if (!ioState) return;

    using Real = decltype(ioState->z.x);

    Complex z = ioState->z;
    Complex c = ioState->c;
    Complex zPrev = ioState->z_prev;

    if (fractalType == FractalType::spider) {
        const Complex z2 = EscapeTimeDirectMul(z, z);
        z = EscapeTimeDirectAdd(z2, c);
        c = EscapeTimeDirectAdd(EscapeTimeDirectScale(c, static_cast<Real>(0.5)), z);
    } else if (fractalType == FractalType::celtic_mandelbrot) {
        const Complex z2{EscapeTimeDirectAbs(z.x * z.x - z.y * z.y), static_cast<Real>(2) * z.x * z.y};
        z = EscapeTimeDirectAdd(z2, c);
    } else if (fractalType == FractalType::perpendicular_burning_ship) {
        const Complex z2{z.x * z.x - z.y * z.y, static_cast<Real>(2) * EscapeTimeDirectAbs(z.x) * z.y};
        z = EscapeTimeDirectAdd(z2, c);
    } else if (fractalType == FractalType::burning_ship) {
        const Complex absZ = EscapeTimeDirectAbsComponents(z);
        z = EscapeTimeDirectAdd(EscapeTimeDirectMul(absZ, absZ), c);
    } else if (fractalType == FractalType::multibrot) {
        z = EscapeTimeDirectAdd(EscapeTimeDirectPowComplexPrincipal(z, static_cast<Real>(multibrotPowerFloat), static_cast<Real>(multibrotPowerImag)), c);
    } else if (fractalType == FractalType::multicorn) {
        const Complex conjugateZ{z.x, -z.y};
        z = EscapeTimeDirectAdd(EscapeTimeDirectPowInt(conjugateZ, multibrotPowerInt), c);
    } else if (fractalType == FractalType::lambda_map) {
        const Complex oneMinusZ{static_cast<Real>(1) - z.x, -z.y};
        z = EscapeTimeDirectMul(lambdaConst, EscapeTimeDirectMul(z, oneMinusZ));
    } else if (fractalType == FractalType::phoenix) {
        const Complex zNext = EscapeTimeDirectAdd(EscapeTimeDirectAdd(EscapeTimeDirectMul(z, z), c), EscapeTimeDirectMul(phoenixP, zPrev));
        zPrev = z;
        z = zNext;
    } else if (fractalType == FractalType::magnet) {
        const Complex z2 = EscapeTimeDirectMul(z, z);
        const Complex numerator = EscapeTimeDirectAdd(EscapeTimeDirectSub(z2, {static_cast<Real>(1), static_cast<Real>(0)}), c);
        const Complex denominator = EscapeTimeDirectAdd(EscapeTimeDirectSub(EscapeTimeDirectScale(z, static_cast<Real>(2)), {static_cast<Real>(2), static_cast<Real>(0)}), c);
        const Complex quotient = EscapeTimeDirectDiv(numerator, denominator);
        const Complex raw = EscapeTimeDirectMul(quotient, quotient);
        const Real relax = EscapeTimeDirectClamp(static_cast<Real>(magnetRelaxation), static_cast<Real>(0.05), static_cast<Real>(1.5));
        z = EscapeTimeDirectAdd(z, EscapeTimeDirectScale(EscapeTimeDirectSub(raw, z), relax));
    } else {
        z = EscapeTimeDirectAdd(EscapeTimeDirectMul(z, z), c);
    }

    ioState->z = z;
    ioState->c = c;
    ioState->z_prev = zPrev;
}

template <typename Complex, typename Scalar>
ESCAPE_TIME_DIRECT_HD inline void StepEscapeTimeDirectState(
    FractalType fractalType,
    Scalar multibrotPowerFloat,
    Scalar multibrotPowerImag,
    int multibrotPowerInt,
    Complex lambdaConst,
    Complex phoenixP,
    EscapeTimeDirectState<Complex>* ioState) {
    StepEscapeTimeDirectState(fractalType, multibrotPowerFloat, multibrotPowerImag, multibrotPowerInt, lambdaConst, phoenixP, static_cast<Scalar>(1), ioState);
}

template <typename Complex, typename Scalar>
ESCAPE_TIME_DIRECT_HD inline void StepEscapeTimeDirectState(
    FractalType fractalType,
    Scalar multibrotPowerFloat,
    int multibrotPowerInt,
    Complex lambdaConst,
    Complex phoenixP,
    EscapeTimeDirectState<Complex>* ioState) {
    StepEscapeTimeDirectState(fractalType, multibrotPowerFloat, static_cast<Scalar>(0), multibrotPowerInt, lambdaConst, phoenixP, static_cast<Scalar>(1), ioState);
}

template <typename Complex, typename Scalar>
ESCAPE_TIME_DIRECT_HD inline void StepEscapeTimeDirectState(
    FractalType fractalType,
    Scalar multibrotPowerFloat,
    int multibrotPowerInt,
    Complex lambdaConst,
    Complex phoenixP,
    Scalar magnetRelaxation,
    EscapeTimeDirectState<Complex>* ioState) {
    StepEscapeTimeDirectState(fractalType, multibrotPowerFloat, static_cast<Scalar>(0), multibrotPowerInt, lambdaConst, phoenixP, magnetRelaxation, ioState);
}

template <typename Complex>
ESCAPE_TIME_DIRECT_HD inline auto EscapeTimeDirectMagnetResidualSquared(Complex z) -> decltype(z.x * z.x + z.y * z.y) {
    using Scalar = decltype(z.x);
    const Scalar dx = z.x - static_cast<Scalar>(1);
    return dx * dx + z.y * z.y;
}

template <typename Scalar>
ESCAPE_TIME_DIRECT_HD inline constexpr Scalar DirectEscapeTimeRadiusSquared() {
    return static_cast<Scalar>(4.0);
}

template <typename Scalar>
ESCAPE_TIME_DIRECT_HD inline Scalar DirectEscapeTimeRadiusSquared(Scalar radius) {
    return radius * radius;
}

#undef ESCAPE_TIME_DIRECT_HD
