#include "../src/polynomial_eval_real_coeffs.h"

#include <iostream>

namespace {

struct Cx {
    float x;
    float y;
};

struct Cxd {
    double x;
    double y;
};

template <typename Scalar>
bool NearlyEqual(Scalar left, Scalar right, Scalar eps) {
    const Scalar delta = left - right;
    return delta < eps && delta > -eps;
}

template <typename Complex, typename Scalar>
bool NearlyEqualComplex(Complex left, Complex right, Scalar eps) {
    return NearlyEqual(left.x, right.x, eps) && NearlyEqual(left.y, right.y, eps);
}

} // namespace

int main() {
    const float coeffs[5]{1.0f, 2.0f, 3.0f, 4.0f, 5.0f};

    {
        Cx p{};
        Cx dp{};
        Cx d2p{};
        PolyEvalRealCoeffsDeg4(coeffs, Cx{1.0f, 1.0f}, &p, &dp);
        PolyEvalRealCoeffsDeg4D2(coeffs, Cx{1.0f, 1.0f}, &p, &dp, &d2p);

        if (!NearlyEqualComplex(p, Cx{-25.0f, 16.0f}, 1.0e-6f) ||
            !NearlyEqualComplex(dp, Cx{-32.0f, 70.0f}, 1.0e-6f) ||
            !NearlyEqualComplex(d2p, Cx{30.0f, 144.0f}, 1.0e-6f)) {
            std::cerr << "Float polynomial evaluation helper should preserve the current deg-4 results\n";
            return 1;
        }
    }

    {
        Cxd p{};
        Cxd dp{};
        Cxd d2p{};
        PolyEvalRealCoeffsDeg4(coeffs, Cxd{1.0, 1.0}, &p, &dp);
        PolyEvalRealCoeffsDeg4D2(coeffs, Cxd{1.0, 1.0}, &p, &dp, &d2p);

        if (!NearlyEqualComplex(p, Cxd{-25.0, 16.0}, 1.0e-12) ||
            !NearlyEqualComplex(dp, Cxd{-32.0, 70.0}, 1.0e-12) ||
            !NearlyEqualComplex(d2p, Cxd{30.0, 144.0}, 1.0e-12)) {
            std::cerr << "Double polynomial evaluation helper should preserve the current deg-4 results\n";
            return 1;
        }
    }

    std::cout << "test_polynomial_eval_real_coeffs: all passed\n";
    return 0;
}