#include "../src/explaino_collatz_formulas.h"

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
    {
        Cx residual{};
        Cx derivative{};
        ComputeExplainoCollatzResidualAndDerivative(Cx{0.0f, 0.0f}, &residual, &derivative);
        if (!NearlyEqualComplex(residual, Cx{0.0f, 0.0f}, 1.0e-6f) ||
            !NearlyEqualComplex(derivative, Cx{-0.5f, 0.0f}, 1.0e-6f)) {
            std::cerr << "Explaino-Collatz helper should preserve the fixed point and derivative at z=0\n";
            return 1;
        }
    }

    {
        Cx z{1.0f, 0.0f};
        float residualAbs = 0.0f;
        const ExplainoCollatzStepResult result = StepExplainoCollatzNewton(1.0f, 1.0e-6f, &z, &residualAbs);
        if (result != ExplainoCollatzStepResult::advanced ||
            !NearlyEqual(residualAbs, 3.0f, 1.0e-6f) ||
            !NearlyEqualComplex(z, Cx{-0.5f, 0.0f}, 1.0e-6f)) {
            std::cerr << "Explaino-Collatz Newton step should preserve the current z=1 update\n";
            return 1;
        }
    }

    {
        Cxd residual{};
        Cxd derivative{};
        ComputeExplainoCollatzResidualAndDerivative(Cxd{0.0, 0.0}, &residual, &derivative);
        if (!NearlyEqualComplex(residual, Cxd{0.0, 0.0}, 1.0e-12) ||
            !NearlyEqualComplex(derivative, Cxd{-0.5, 0.0}, 1.0e-12)) {
            std::cerr << "Explaino-Collatz helper should preserve the double-precision fixed point and derivative at z=0\n";
            return 1;
        }
    }

    std::cout << "test_explaino_collatz_formulas: all passed\n";
    return 0;
}