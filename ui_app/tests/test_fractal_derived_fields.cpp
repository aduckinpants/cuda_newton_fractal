#include "../src/fractal_derived_fields.h"

#include <cmath>
#include <iostream>

static bool NearlyEqual(float a, float b, float eps = 1.0e-5f) {
    return std::fabs(a - b) <= eps;
}

int main() {
    {
        ViewState view{};
        view.fractal_type = FractalType::mandelbrot;
        bool dirty = false;
        ApplyFractalViewPresetDefaults(view, &dirty);
        if (!dirty) {
            std::cerr << "Mandelbrot view preset should mark dirty\n";
            return 1;
        }
        if (!NearlyEqual(view.center.x, -0.745f) || !NearlyEqual(view.center.y, 0.186f) || !NearlyEqual(view.zoom, 38.0f)) {
            std::cerr << "Mandelbrot view preset should land on Seahorse Valley\n";
            return 1;
        }
    }

    {
        ViewState view{};
        view.fractal_type = FractalType::burning_ship;
        ApplyFractalViewPresetDefaults(view, nullptr);
        if (!NearlyEqual(view.center.x, -1.762f) || !NearlyEqual(view.center.y, -0.028f) || !NearlyEqual(view.zoom, 25.0f)) {
            std::cerr << "Burning Ship view preset should land on the antenna spiral\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::mandelbrot;
        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        if (params.max_iter != 1200 || !NearlyEqual(params.exposure, 1.5f)) {
            std::cerr << "Mandelbrot param preset should have tuned max_iter and exposure\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::phoenix;
        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        if (params.max_iter != 1200 || !NearlyEqual(params.phoenix_p_real, 0.5667f) || !NearlyEqual(params.exposure, 1.6f)) {
            std::cerr << "Phoenix param preset should have tuned p_real, max_iter, and exposure\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::explaino;
        params.explaino_seed = 7.0;
        params.explaino_warp_strength = 0.35f;
        view.explaino_phase = 0.0f;
        view.explaino_seed_drift = 0.0f;

        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        UpdateExplainoPolynomial(view, params, &dirty);

        if (!dirty) {
            std::cerr << "Explaino defaults should mark dirty\n";
            return 1;
        }
        if (params.poly_kind != PolyKind::custom) {
            std::cerr << "Explaino should force custom polynomial\n";
            return 1;
        }
        if (params.explaino_root_count != 4) {
            std::cerr << "Explaino should derive four roots\n";
            return 1;
        }
        if (!NearlyEqual(params.poly_coeffs[4], 1.0f)) {
            std::cerr << "Explaino quartic should be monic\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::julia;
        params.explaino_root_count = 99;
        UpdateExplainoPolynomial(view, params, nullptr);
        if (params.explaino_root_count != 0) {
            std::cerr << "Non-Explaino modes must clear Explaino roots\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::explaino;
        bool dirty = false;
        ApplyFractalDerivedFieldsAndSyncHp(view, params, &dirty, true, 0.75);
        if (!NearlyEqual((float)params.explaino_seed, 0.0f) || !NearlyEqual(view.explaino_seed_drift, 0.75f)) {
            std::cerr << "Explaino combined seed override must split into base + drift\n";
            return 1;
        }
    }

    return 0;
}