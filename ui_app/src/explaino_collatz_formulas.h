#pragma once

#include <cmath>

#if defined(__CUDACC__)
#define EXPLAINO_COLLATZ_HD __host__ __device__
#else
#define EXPLAINO_COLLATZ_HD
#endif

enum class ExplainoCollatzStepResult : int {
    advanced = 0,
    converged = 1,
    degenerate = 2,
};

EXPLAINO_COLLATZ_HD inline float ExplainoCollatzCos(float value) { return cosf(value); }
EXPLAINO_COLLATZ_HD inline double ExplainoCollatzCos(double value) { return cos(value); }
EXPLAINO_COLLATZ_HD inline float ExplainoCollatzSin(float value) { return sinf(value); }
EXPLAINO_COLLATZ_HD inline double ExplainoCollatzSin(double value) { return sin(value); }
EXPLAINO_COLLATZ_HD inline float ExplainoCollatzCosh(float value) { return coshf(value); }
EXPLAINO_COLLATZ_HD inline double ExplainoCollatzCosh(double value) { return cosh(value); }
EXPLAINO_COLLATZ_HD inline float ExplainoCollatzSinh(float value) { return sinhf(value); }
EXPLAINO_COLLATZ_HD inline double ExplainoCollatzSinh(double value) { return sinh(value); }
EXPLAINO_COLLATZ_HD inline float ExplainoCollatzSqrt(float value) { return sqrtf(value); }
EXPLAINO_COLLATZ_HD inline double ExplainoCollatzSqrt(double value) { return sqrt(value); }
EXPLAINO_COLLATZ_HD inline float ExplainoCollatzDerivativeThreshold(float) { return 1.0e-20f; }
EXPLAINO_COLLATZ_HD inline double ExplainoCollatzDerivativeThreshold(double) { return 1.0e-30; }

template <typename Complex>
EXPLAINO_COLLATZ_HD inline Complex ExplainoCollatzAdd(Complex left, Complex right) {
    return {left.x + right.x, left.y + right.y};
}

template <typename Complex>
EXPLAINO_COLLATZ_HD inline Complex ExplainoCollatzSub(Complex left, Complex right) {
    return {left.x - right.x, left.y - right.y};
}

template <typename Complex>
EXPLAINO_COLLATZ_HD inline Complex ExplainoCollatzMul(Complex left, Complex right) {
    return {left.x * right.x - left.y * right.y, left.x * right.y + left.y * right.x};
}

template <typename Complex, typename Scalar>
EXPLAINO_COLLATZ_HD inline Complex ExplainoCollatzScale(Complex value, Scalar scale) {
    return {value.x * scale, value.y * scale};
}

template <typename Complex>
EXPLAINO_COLLATZ_HD inline auto ExplainoCollatzAbs2(Complex value) -> decltype(value.x * value.x + value.y * value.y) {
    return value.x * value.x + value.y * value.y;
}

template <typename Complex>
EXPLAINO_COLLATZ_HD inline auto ExplainoCollatzAbs(Complex value) -> decltype(value.x + value.y) {
    return ExplainoCollatzSqrt(ExplainoCollatzAbs2(value));
}

template <typename Complex>
EXPLAINO_COLLATZ_HD inline Complex ExplainoCollatzDiv(Complex left, Complex right) {
    using Scalar = decltype(left.x);
    const Scalar denom = right.x * right.x + right.y * right.y;
    if (denom == static_cast<Scalar>(0)) {
        return {static_cast<Scalar>(0), static_cast<Scalar>(0)};
    }
    return {
        (left.x * right.x + left.y * right.y) / denom,
        (left.y * right.x - left.x * right.y) / denom,
    };
}

template <typename Complex>
EXPLAINO_COLLATZ_HD inline Complex ExplainoCollatzCosPi(Complex value) {
    using Scalar = decltype(value.x);
    const Scalar pi = static_cast<Scalar>(3.14159265358979323846);
    const Scalar px = pi * value.x;
    const Scalar py = pi * value.y;
    return {
        ExplainoCollatzCos(px) * ExplainoCollatzCosh(py),
        -ExplainoCollatzSin(px) * ExplainoCollatzSinh(py),
    };
}

template <typename Complex>
EXPLAINO_COLLATZ_HD inline Complex ExplainoCollatzSinPi(Complex value) {
    using Scalar = decltype(value.x);
    const Scalar pi = static_cast<Scalar>(3.14159265358979323846);
    const Scalar px = pi * value.x;
    const Scalar py = pi * value.y;
    return {
        ExplainoCollatzSin(px) * ExplainoCollatzCosh(py),
        ExplainoCollatzCos(px) * ExplainoCollatzSinh(py),
    };
}

template <typename Complex>
EXPLAINO_COLLATZ_HD inline void ComputeExplainoCollatzResidualAndDerivative(
    Complex z,
    Complex* outResidual,
    Complex* outDerivative) {
    using Scalar = decltype(z.x);
    const Scalar pi = static_cast<Scalar>(3.14159265358979323846);
    const Complex cosPi = ExplainoCollatzCosPi(z);
    const Complex sinPi = ExplainoCollatzSinPi(z);
    const Complex linear{static_cast<Scalar>(2) + static_cast<Scalar>(5) * z.x, static_cast<Scalar>(5) * z.y};
    const Complex affine{static_cast<Scalar>(2) + static_cast<Scalar>(3) * z.x, static_cast<Scalar>(3) * z.y};
    const Complex ac = ExplainoCollatzMul(linear, cosPi);
    const Complex as = ExplainoCollatzMul(linear, sinPi);

    if (outResidual) {
        *outResidual = ExplainoCollatzScale(ExplainoCollatzSub(affine, ac), static_cast<Scalar>(0.25));
    }
    if (outDerivative) {
        *outDerivative = ExplainoCollatzScale(
            Complex{
                static_cast<Scalar>(3) - static_cast<Scalar>(5) * cosPi.x + pi * as.x,
                -static_cast<Scalar>(5) * cosPi.y + pi * as.y,
            },
            static_cast<Scalar>(0.25));
    }
}

template <typename Complex, typename Scalar>
EXPLAINO_COLLATZ_HD inline ExplainoCollatzStepResult StepExplainoCollatzNewton(
    Scalar damping,
    Scalar epsilon,
    Complex* ioZ,
    Scalar* outResidualAbs) {
    if (!ioZ) return ExplainoCollatzStepResult::degenerate;

    Complex residual{};
    Complex derivative{};
    ComputeExplainoCollatzResidualAndDerivative(*ioZ, &residual, &derivative);

    const Scalar residualAbs = ExplainoCollatzAbs(residual);
    if (outResidualAbs) *outResidualAbs = residualAbs;
    if (residualAbs < epsilon) {
        return ExplainoCollatzStepResult::converged;
    }

    if (ExplainoCollatzAbs2(derivative) < ExplainoCollatzDerivativeThreshold(Scalar{})) {
        return ExplainoCollatzStepResult::degenerate;
    }

    *ioZ = ExplainoCollatzSub(*ioZ, ExplainoCollatzScale(ExplainoCollatzDiv(residual, derivative), damping));
    return ExplainoCollatzStepResult::advanced;
}

#undef EXPLAINO_COLLATZ_HD