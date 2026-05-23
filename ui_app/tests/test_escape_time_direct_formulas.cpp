#include "../src/escape_time_direct_formulas.h"

#include <iostream>

namespace {

struct Cx {
    float x;
    float y;
};

bool NearlyEqual(float left, float right, float eps = 1.0e-6f) {
    const float delta = left - right;
    return delta < eps && delta > -eps;
}

bool NearlyEqualCx(Cx left, Cx right, float eps = 1.0e-6f) {
    return NearlyEqual(left.x, right.x, eps) && NearlyEqual(left.y, right.y, eps);
}

} // namespace

int main() {
    if (!UsesSharedEscapeTimeDirectFormula(FractalType::mandelbrot) ||
        !UsesSharedEscapeTimeDirectFormula(FractalType::phoenix) ||
        !UsesSharedEscapeTimeDirectFormula(FractalType::multicorn) ||
        !UsesSharedEscapeTimeDirectFormula(FractalType::magnet)) {
        std::cerr << "Expected direct escape-time helper to cover Mandelbrot, Phoenix, Multicorn, and Magnet\n";
        return 1;
    }
    if (UsesSharedEscapeTimeDirectFormula(FractalType::mcmullen) ||
        UsesSharedEscapeTimeDirectFormula(FractalType::collatz) ||
        UsesSharedEscapeTimeDirectFormula(FractalType::newton)) {
        std::cerr << "Direct escape-time helper should not absorb McMullen, Collatz, or root-finding families in phase one\n";
        return 1;
    }

    {
        const Cx coord{-0.5f, 0.25f};
        const EscapeTimeDirectState<Cx> state = InitEscapeTimeDirectState(FractalType::mandelbrot, coord);
        if (!NearlyEqualCx(state.z, {0.0f, 0.0f}) || !NearlyEqualCx(state.c, coord) || !NearlyEqualCx(state.z_prev, {0.0f, 0.0f})) {
            std::cerr << "Mandelbrot direct-state init should start at z0=0 with c=coord\n";
            return 1;
        }
    }

    {
        const Cx coord{-0.5f, 0.25f};
        const EscapeTimeDirectState<Cx> state = InitEscapeTimeDirectState(FractalType::julia, coord);
        if (!NearlyEqualCx(state.z, coord) || !NearlyEqualCx(state.c, {-0.7f, 0.27015f})) {
            std::cerr << "Julia direct-state init should start at z0=coord with the fixed Julia constant\n";
            return 1;
        }
    }

    {
        const Cx coord{-0.5f, 0.25f};
        const Cx juliaConst{0.285f, 0.01f};
        const EscapeTimeDirectState<Cx> state = InitEscapeTimeDirectState(FractalType::julia, coord, {0.0f, 0.0f}, juliaConst);
        if (!NearlyEqualCx(state.z, coord) || !NearlyEqualCx(state.c, juliaConst)) {
            std::cerr << "Julia direct-state init should accept the user-owned Julia constant\n";
            return 1;
        }
    }

    {
        const Cx coord{0.5f, 0.0f};
        const EscapeTimeDirectState<Cx> state = InitEscapeTimeDirectState(FractalType::lambda_map, coord);
        if (!NearlyEqualCx(state.z, coord) || !NearlyEqualCx(state.c, {0.0f, 0.0f})) {
            std::cerr << "Lambda direct-state init should start at z0=coord without a c-plane constant\n";
            return 1;
        }
    }

    {
        EscapeTimeDirectState<Cx> state{{1.0f, 2.0f}, {3.0f, 4.0f}, {0.0f, 0.0f}};
        StepEscapeTimeDirectState(FractalType::celtic_mandelbrot, 3.0f, 3, {0.0f, 0.0f}, {0.0f, 0.0f}, &state);
        if (!NearlyEqualCx(state.z, {6.0f, 8.0f})) {
            std::cerr << "Celtic Mandelbrot step should absolute the quadratic real term before adding c\n";
            return 1;
        }
    }

    {
        EscapeTimeDirectState<Cx> state{{1.0f, 2.0f}, {3.0f, 4.0f}, {0.0f, 0.0f}};
        StepEscapeTimeDirectState(
            FractalType::celtic_mandelbrot,
            3.0f,
            0.0f,
            3,
            {0.0f, 0.0f},
            {0.0f, 0.0f},
            1.0f,
            0.5f,
            1.0f,
            0.0f,
            1.0f,
            &state);
        if (!NearlyEqualCx(state.z, {0.0f, 8.0f})) {
            std::cerr << "Celtic abs mix 0 should expose the unfolded quadratic real term\n";
            return 1;
        }
    }

    {
        EscapeTimeDirectState<Cx> state{{-1.0f, 2.0f}, {3.0f, 4.0f}, {0.0f, 0.0f}};
        StepEscapeTimeDirectState(FractalType::perpendicular_burning_ship, 3.0f, 3, {0.0f, 0.0f}, {0.0f, 0.0f}, &state);
        if (!NearlyEqualCx(state.z, {0.0f, 8.0f})) {
            std::cerr << "Perpendicular Burning Ship step should absolute only the real factor in the imaginary term\n";
            return 1;
        }
    }

    {
        EscapeTimeDirectState<Cx> state{{-1.0f, 2.0f}, {3.0f, 4.0f}, {0.0f, 0.0f}};
        StepEscapeTimeDirectState(
            FractalType::perpendicular_burning_ship,
            3.0f,
            0.0f,
            3,
            {0.0f, 0.0f},
            {0.0f, 0.0f},
            1.0f,
            0.5f,
            1.0f,
            1.0f,
            0.0f,
            &state);
        if (!NearlyEqualCx(state.z, {0.0f, 0.0f})) {
            std::cerr << "Perpendicular fold mix 0 should expose the signed real factor in the imaginary term\n";
            return 1;
        }
    }

    {
        EscapeTimeDirectState<Cx> state{{1.0f, -2.0f}, {3.0f, 4.0f}, {0.0f, 0.0f}};
        StepEscapeTimeDirectState(FractalType::burning_ship, 3.0f, 3, {0.0f, 0.0f}, {0.0f, 0.0f}, &state);
        if (!NearlyEqualCx(state.z, {0.0f, 8.0f})) {
            std::cerr << "Burning Ship step should square the abs-components form before adding c\n";
            return 1;
        }
    }

    {
        EscapeTimeDirectState<Cx> state{{1.0f, -2.0f}, {3.0f, 4.0f}, {0.0f, 0.0f}};
        StepEscapeTimeDirectState(
            FractalType::burning_ship,
            3.0f,
            0.0f,
            3,
            {0.0f, 0.0f},
            {0.0f, 0.0f},
            1.0f,
            0.5f,
            0.0f,
            1.0f,
            1.0f,
            &state);
        if (!NearlyEqualCx(state.z, {0.0f, 0.0f})) {
            std::cerr << "Burning Ship fold mix 0 should expose the unfolded quadratic step\n";
            return 1;
        }
    }

    {
        EscapeTimeDirectState<Cx> state{{1.0f, 2.0f}, {3.0f, 4.0f}, {0.0f, 0.0f}};
        StepEscapeTimeDirectState(FractalType::multicorn, 3.0f, 2, {0.0f, 0.0f}, {0.0f, 0.0f}, &state);
        if (!NearlyEqualCx(state.z, {0.0f, 0.0f})) {
            std::cerr << "Multicorn step should apply the integer power to the conjugate orbit before adding c\n";
            return 1;
        }
    }

    {
        const Cx z{1.0f, 2.0f};
        const Cx realPath = EscapeTimeDirectPowRealPrincipal(z, 1.5f);
        const Cx complexZeroImagPath = EscapeTimeDirectPowComplexPrincipal(z, 1.5f, 0.0f);
        const Cx complexImagPath = EscapeTimeDirectPowComplexPrincipal(z, 1.5f, 0.75f);
        if (!NearlyEqualCx(realPath, complexZeroImagPath, 1.0e-5f)) {
            std::cerr << "Multibrot complex exponent helper should preserve the existing real-power path when imaginary power is zero\n";
            return 1;
        }
        if (NearlyEqualCx(realPath, complexImagPath, 1.0e-4f)) {
            std::cerr << "Multibrot complex exponent helper should change when imaginary power is nonzero\n";
            return 1;
        }
    }

    {
        EscapeTimeDirectState<Cx> realState{{1.0f, 2.0f}, {3.0f, 4.0f}, {0.0f, 0.0f}};
        EscapeTimeDirectState<Cx> complexState = realState;
        StepEscapeTimeDirectState(FractalType::multibrot, 1.5f, 0.0f, 3, {0.0f, 0.0f}, {0.0f, 0.0f}, &realState);
        StepEscapeTimeDirectState(FractalType::multibrot, 1.5f, 0.75f, 3, {0.0f, 0.0f}, {0.0f, 0.0f}, &complexState);
        if (NearlyEqualCx(realState.z, complexState.z, 1.0e-4f)) {
            std::cerr << "Multibrot step should use the imaginary exponent instead of silently falling back to the old real-only power\n";
            return 1;
        }
    }

    {
        EscapeTimeDirectState<Cx> state{{1.0f, 2.0f}, {3.0f, 4.0f}, {5.0f, 6.0f}};
        StepEscapeTimeDirectState(FractalType::phoenix, 3.0f, 3, {0.0f, 0.0f}, {0.0f, 0.0f}, &state);
        if (!NearlyEqualCx(state.z, {0.0f, 8.0f}) || !NearlyEqualCx(state.z_prev, {1.0f, 2.0f})) {
            std::cerr << "Phoenix step should update both the live orbit and z_prev memory register\n";
            return 1;
        }
    }

    {
        EscapeTimeDirectState<Cx> state{{0.5f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}};
        StepEscapeTimeDirectState(FractalType::lambda_map, 3.0f, 3, {2.0f, 0.0f}, {0.0f, 0.0f}, &state);
        if (!NearlyEqualCx(state.z, {0.5f, 0.0f})) {
            std::cerr << "Lambda step should follow z <- lambda * z * (1-z)\n";
            return 1;
        }
    }

    {
        EscapeTimeDirectState<Cx> state{{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f}};
        StepEscapeTimeDirectState(FractalType::spider, 3.0f, 3, {0.0f, 0.0f}, {0.0f, 0.0f}, &state);
        if (!NearlyEqualCx(state.z, {1.0f, 1.0f}) || !NearlyEqualCx(state.c, {1.5f, 1.5f})) {
            std::cerr << "Spider step should update both z and the running c state\n";
            return 1;
        }
    }

    {
        const Cx coord{0.0f, 0.0f};
        const Cx magnetSeed{0.0f, 0.0f};
        EscapeTimeDirectState<Cx> state = InitEscapeTimeDirectState(FractalType::magnet, coord, magnetSeed);
        if (!NearlyEqualCx(state.z, magnetSeed) || !NearlyEqualCx(state.c, coord)) {
            std::cerr << "Magnet direct-state init should start from the explicit seed and keep c=coord\n";
            return 1;
        }
        StepEscapeTimeDirectState(FractalType::magnet, 3.0f, 3, {0.0f, 0.0f}, {0.0f, 0.0f}, 0.5f, &state);
        if (!NearlyEqualCx(state.z, {0.125f, 0.0f}) ||
            !NearlyEqual(EscapeTimeDirectMagnetResidualSquared(state.z), 0.765625f)) {
            std::cerr << "Magnet step should apply the bounded rational Type I map and report residual to the unit attractor\n";
            return 1;
        }
    }

    if (!NearlyEqual(DirectEscapeTimeRadiusSquared<float>(12.0f), 144.0f)) {
        std::cerr << "Direct escape-time helper should square Magnet's explicit bailout radius\n";
        return 1;
    }

    if (!NearlyEqual(DirectEscapeTimeRadiusSquared<float>(), 4.0f)) {
        std::cerr << "Direct escape-time helper should use the shared radius-squared contract of 4\n";
        return 1;
    }

    std::cout << "test_escape_time_direct_formulas: all passed\n";
    return 0;
}
