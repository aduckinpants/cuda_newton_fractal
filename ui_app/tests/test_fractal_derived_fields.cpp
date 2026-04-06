#include "../src/explaino_seed.h"
#include "../src/fractal_derived_fields.h"

#include <cmath>
#include <iostream>

static bool NearlyEqual(float a, float b, float eps = 1.0e-5f) {
    return std::fabs(a - b) <= eps;
}

int main() {
    {
        ViewState view{};
        if (view.auto_dive) {
            std::cerr << "ViewState should start with auto_dive disabled\n";
            return 1;
        }
    }

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
        view.fractal_type = FractalType::nova;
        ApplyFractalPresetDefaults(view, params, nullptr);
        if (params.coloring_mode != ColoringMode::smooth_escape) {
            std::cerr << "Nova should default to smooth_escape, not basin coloring\n";
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
        params.explaino_warp_strength = 0.99f;
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
        if (!NearlyEqual(params.explaino_warp_strength, 0.0f)) {
            std::cerr << "Explaino should start with warp disabled\n";
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
        view.fractal_type = FractalType::explaino_nova;
        params.explaino_seed = 7.0;
        params.explaino_warp_strength = 0.99f;
        params.coloring_mode = ColoringMode::joy_basins;
        params.nova_alpha = 1.5f;

        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        UpdateExplainoPolynomial(view, params, &dirty);

        if (!dirty) {
            std::cerr << "Explaino-Nova defaults should mark dirty\n";
            return 1;
        }
        if (params.max_iter != 300 || !NearlyEqual(params.epsilon, 1.0e-6f) || !NearlyEqual(params.nova_alpha, 0.50f)) {
            std::cerr << "Explaino-Nova should use Nova-tuned iteration defaults\n";
            return 1;
        }
        if (params.poly_kind != PolyKind::custom) {
            std::cerr << "Explaino-Nova should force custom polynomial\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::smooth_escape) {
            std::cerr << "Explaino-Nova should default to smooth_escape\n";
            return 1;
        }
        if (!NearlyEqual(params.explaino_warp_strength, 0.0f)) {
            std::cerr << "Explaino-Nova should start with warp disabled\n";
            return 1;
        }
        if (params.explaino_root_count != 4 || !NearlyEqual(params.poly_coeffs[4], 1.0f)) {
            std::cerr << "Explaino-Nova should still derive the Explaino quartic surface\n";
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

    // Normalized-seed regression: the host polynomial update should depend on
    // the split combined seed surface (base + drift), not on raw fractional
    // writes into params.explaino_seed.
    {
        ViewState viewA{};
        ViewState viewB{};
        KernelParams paramsA{};
        KernelParams paramsB{};
        viewA.fractal_type = FractalType::explaino;
        viewB.fractal_type = FractalType::explaino;
        viewA.explaino_phase = 0.0f;
        viewB.explaino_phase = 0.0f;
        viewA.explaino_phase_strength = 1.0f;
        viewB.explaino_phase_strength = 1.0f;

        paramsA.explaino_seed = 5.0;
        viewA.explaino_seed_drift = 0.4f;
        paramsA.explaino_root_spread = 0.5f;
        UpdateExplainoPolynomial(viewA, paramsA, nullptr);
        float rootA0x = paramsA.explaino_roots[0].x;
        float coeffA0 = paramsA.poly_coeffs[0];

        ExplainoSeedSetCombined(viewB, paramsB, 5.4);
        paramsB.explaino_root_spread = 0.5f;
        UpdateExplainoPolynomial(viewB, paramsB, nullptr);
        float rootB0x = paramsB.explaino_roots[0].x;
        float coeffB0 = paramsB.poly_coeffs[0];

        if (!NearlyEqual(rootA0x, rootB0x, 1e-6f)) {
            std::cerr << "Equivalent combined seed splits must produce identical roots\n";
            return 1;
        }
        if (!NearlyEqual(coeffA0, coeffB0, 1e-6f)) {
            std::cerr << "Equivalent combined seed splits must produce identical coefficients\n";
            return 1;
        }
        if (!NearlyEqual((float)paramsB.explaino_seed, 5.0f) || !NearlyEqual(viewB.explaino_seed_drift, 0.4f, 1e-6f)) {
            std::cerr << "ExplainoSeedSetCombined should normalize 5.4 into base=5 drift=0.4\n";
            return 1;
        }
    }

    // Explaino-Halley must get preset defaults (custom poly, basin coloring)
    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::explaino_halley;
        params.explaino_seed = 3.0;
        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        UpdateExplainoPolynomial(view, params, &dirty);

        if (!dirty) {
            std::cerr << "Explaino-Halley defaults should mark dirty\n";
            return 1;
        }
        if (params.poly_kind != PolyKind::custom) {
            std::cerr << "Explaino-Halley should force custom polynomial\n";
            return 1;
        }
        if (params.max_iter != 500) {
            std::cerr << "Explaino-Halley should use 500 max_iter (got " << params.max_iter << ")\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::joy_basins) {
            std::cerr << "Explaino-Halley should default to joy_basins\n";
            return 1;
        }
        if (params.explaino_root_count != 4 || !NearlyEqual(params.poly_coeffs[4], 1.0f)) {
            std::cerr << "Explaino-Halley should derive the Explaino quartic\n";
            return 1;
        }
    }

    return 0;
}