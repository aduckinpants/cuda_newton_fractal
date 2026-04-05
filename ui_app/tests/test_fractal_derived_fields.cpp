#include "../src/fractal_derived_fields.h"

#include <cmath>
#include <iostream>

static bool NearlyEqual(float a, float b, float eps = 1.0e-5f) {
    return std::fabs(a - b) <= eps;
}

int main() {
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

    return 0;
}