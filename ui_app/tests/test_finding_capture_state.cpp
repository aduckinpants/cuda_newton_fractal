#include "../src/finding_capture_state.h"

#include <iostream>

namespace {

bool NearlyEqual(float a, float b, float eps = 1.0e-5f) {
    return (a - b) < eps && (a - b) > -eps;
}

} // namespace

int main() {
    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::mandelbrot;
        view.auto_max_iter = false;
        params.max_iter = 321;

        if (PrepareFindingCaptureRuntimeState(view, params)) {
            std::cerr << "Capture prep should not invalidate caches when no derived runtime state changes are needed\n";
            return 1;
        }
        if (params.max_iter != 321) {
            std::cerr << "Capture prep should preserve max_iter when auto_max_iter is disabled\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::mandelbrot;
        view.auto_max_iter = true;
        view.log2_zoom = 15.0;
        params.max_iter = 12;

        if (!PrepareFindingCaptureRuntimeState(view, params)) {
            std::cerr << "Capture prep should invalidate caches when auto_max_iter rewrites max_iter\n";
            return 1;
        }
        if (params.max_iter == 12) {
            std::cerr << "Capture prep should recompute max_iter when auto_max_iter is enabled\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::explaino;
        params.poly_kind = PolyKind::z3_minus_1;
        params.explaino_root_count = 0;

        if (!PrepareFindingCaptureRuntimeState(view, params)) {
            std::cerr << "Capture prep should invalidate caches for Explaino-derived polynomial refresh\n";
            return 1;
        }
        if (params.poly_kind != PolyKind::custom || params.explaino_root_count != 4) {
            std::cerr << "Capture prep should refresh Explaino-derived polynomial state\n";
            return 1;
        }
        if (!NearlyEqual(params.poly_coeffs[4], 1.0f)) {
            std::cerr << "Capture prep should populate Explaino polynomial coefficients\n";
            return 1;
        }
    }

    std::cout << "test_finding_capture_state: all passed\n";
    return 0;
}