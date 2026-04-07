#pragma once

#if defined(__CUDACC__)
#define POLY_EVAL_REAL_COEFFS_HD __host__ __device__
#else
#define POLY_EVAL_REAL_COEFFS_HD
#endif

template <typename Complex>
POLY_EVAL_REAL_COEFFS_HD inline Complex PolyEvalRealCoeffsAdd(Complex left, Complex right) {
    return {left.x + right.x, left.y + right.y};
}

template <typename Complex>
POLY_EVAL_REAL_COEFFS_HD inline Complex PolyEvalRealCoeffsMul(Complex left, Complex right) {
    return {left.x * right.x - left.y * right.y, left.x * right.y + left.y * right.x};
}

template <typename Complex, typename Scalar>
POLY_EVAL_REAL_COEFFS_HD inline Complex PolyEvalRealCoeffsScale(Complex value, Scalar scale) {
    return {value.x * scale, value.y * scale};
}

template <typename Complex>
POLY_EVAL_REAL_COEFFS_HD inline void PolyEvalRealCoeffsDeg4(const float coeffs[5], Complex z, Complex* outP, Complex* outDp) {
    using Scalar = decltype(z.x);

    const Complex z2 = PolyEvalRealCoeffsMul(z, z);
    const Complex z3 = PolyEvalRealCoeffsMul(z2, z);
    const Complex z4 = PolyEvalRealCoeffsMul(z2, z2);

    Complex p{static_cast<Scalar>(coeffs[0]), static_cast<Scalar>(0)};
    p = PolyEvalRealCoeffsAdd(p, PolyEvalRealCoeffsScale(z, static_cast<Scalar>(coeffs[1])));
    p = PolyEvalRealCoeffsAdd(p, PolyEvalRealCoeffsScale(z2, static_cast<Scalar>(coeffs[2])));
    p = PolyEvalRealCoeffsAdd(p, PolyEvalRealCoeffsScale(z3, static_cast<Scalar>(coeffs[3])));
    p = PolyEvalRealCoeffsAdd(p, PolyEvalRealCoeffsScale(z4, static_cast<Scalar>(coeffs[4])));

    Complex dp{static_cast<Scalar>(coeffs[1]), static_cast<Scalar>(0)};
    dp = PolyEvalRealCoeffsAdd(dp, PolyEvalRealCoeffsScale(z, static_cast<Scalar>(2) * static_cast<Scalar>(coeffs[2])));
    dp = PolyEvalRealCoeffsAdd(dp, PolyEvalRealCoeffsScale(z2, static_cast<Scalar>(3) * static_cast<Scalar>(coeffs[3])));
    dp = PolyEvalRealCoeffsAdd(dp, PolyEvalRealCoeffsScale(z3, static_cast<Scalar>(4) * static_cast<Scalar>(coeffs[4])));

    if (outP) *outP = p;
    if (outDp) *outDp = dp;
}

template <typename Complex>
POLY_EVAL_REAL_COEFFS_HD inline void PolyEvalRealCoeffsDeg4D2(
    const float coeffs[5],
    Complex z,
    Complex* outP,
    Complex* outDp,
    Complex* outD2p) {
    using Scalar = decltype(z.x);

    const Complex z2 = PolyEvalRealCoeffsMul(z, z);
    const Complex z3 = PolyEvalRealCoeffsMul(z2, z);
    const Complex z4 = PolyEvalRealCoeffsMul(z2, z2);

    Complex p{static_cast<Scalar>(coeffs[0]), static_cast<Scalar>(0)};
    p = PolyEvalRealCoeffsAdd(p, PolyEvalRealCoeffsScale(z, static_cast<Scalar>(coeffs[1])));
    p = PolyEvalRealCoeffsAdd(p, PolyEvalRealCoeffsScale(z2, static_cast<Scalar>(coeffs[2])));
    p = PolyEvalRealCoeffsAdd(p, PolyEvalRealCoeffsScale(z3, static_cast<Scalar>(coeffs[3])));
    p = PolyEvalRealCoeffsAdd(p, PolyEvalRealCoeffsScale(z4, static_cast<Scalar>(coeffs[4])));

    Complex dp{static_cast<Scalar>(coeffs[1]), static_cast<Scalar>(0)};
    dp = PolyEvalRealCoeffsAdd(dp, PolyEvalRealCoeffsScale(z, static_cast<Scalar>(2) * static_cast<Scalar>(coeffs[2])));
    dp = PolyEvalRealCoeffsAdd(dp, PolyEvalRealCoeffsScale(z2, static_cast<Scalar>(3) * static_cast<Scalar>(coeffs[3])));
    dp = PolyEvalRealCoeffsAdd(dp, PolyEvalRealCoeffsScale(z3, static_cast<Scalar>(4) * static_cast<Scalar>(coeffs[4])));

    Complex d2p{static_cast<Scalar>(2) * static_cast<Scalar>(coeffs[2]), static_cast<Scalar>(0)};
    d2p = PolyEvalRealCoeffsAdd(d2p, PolyEvalRealCoeffsScale(z, static_cast<Scalar>(6) * static_cast<Scalar>(coeffs[3])));
    d2p = PolyEvalRealCoeffsAdd(d2p, PolyEvalRealCoeffsScale(z2, static_cast<Scalar>(12) * static_cast<Scalar>(coeffs[4])));

    if (outP) *outP = p;
    if (outDp) *outDp = dp;
    if (outD2p) *outD2p = d2p;
}

#undef POLY_EVAL_REAL_COEFFS_HD