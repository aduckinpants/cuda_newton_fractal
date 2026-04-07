#include "../src/explaino_seed.h"
#include "../src/explaino_seed_dynamics.h"

#include <cmath>
#include <iostream>
#include <limits>

static bool NearlyEqual(double a, double b, double eps = 1.0e-9) {
    return std::fabs(a - b) <= eps;
}

int main() {
    {
        ViewState view{};
        KernelParams params{};
        RenderStats stats{};

        view.fractal_type = FractalType::explaino;
        view.auto_increment_seed = true;
        view.explaino_seed_rate = 0.5f;
        params.explaino_seed = 2.0;
        view.explaino_seed_drift = 0.25f;

        if (!ApplyExplainoSeedDynamics(stats, 2.0, view, params)) {
            std::cerr << "Expected auto-increment seed dynamics to report a change\n";
            return 1;
        }
        // Combined-seed transport: rate=0.5, dt=2.0 => delta=1.0.
        // 2.25 -> 3.25, so base carries to 3 and drift stays at 0.25.
        if (!NearlyEqual(params.explaino_seed, 3.0)) {
            std::cerr << "Expected auto-increment to carry whole-seed overflow into the base\n";
            return 1;
        }
        if (!NearlyEqual(view.explaino_seed_drift, 0.25f)) {
            std::cerr << "Expected auto-increment to preserve the fractional carry after whole-seed normalization\n";
            return 1;
        }
        if (!NearlyEqual(ExplainoSeedCombined(view, params), 3.25)) {
            std::cerr << "Expected combined seed = base + drift\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        RenderStats stats{};

        view.fractal_type = FractalType::nova;
        view.auto_increment_seed = true;
        view.explaino_seed_rate = 1.0f;
        params.explaino_seed = 4.0;
        view.explaino_seed_drift = 0.5f;

        if (ApplyExplainoSeedDynamics(stats, 1.0, view, params)) {
            std::cerr << "Non-Explaino families must ignore Explaino seed dynamics\n";
            return 1;
        }
        if (!NearlyEqual(ExplainoSeedCombined(view, params), 4.5)) {
            std::cerr << "Non-Explaino families should leave the combined seed untouched\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        RenderStats stats{};

        view.fractal_type = FractalType::explaino;
        view.auto_increment_seed = true;
        view.explaino_seed_rate = 1.0f;
        const double nan = std::numeric_limits<double>::quiet_NaN();

        if (ApplyExplainoSeedDynamics(stats, nan, view, params)) {
            std::cerr << "Non-finite frame deltas must not mutate Explaino seed dynamics\n";
            return 1;
        }
        if (!NearlyEqual(ExplainoSeedCombined(view, params), 0.0)) {
            std::cerr << "Non-finite frame deltas should leave the combined seed unchanged\n";
            return 1;
        }
    }

    // Combined seed must advance through the normalized split/base seam.
    // This pins the regression where auto-increment wrote straight into
    // params.explaino_seed, bypassing ExplainoSeedSetCombined and causing
    // wild transport through the viewer.
    {
        ViewState view{};
        KernelParams params{};
        RenderStats stats{};

        view.fractal_type = FractalType::explaino_fp;
        view.auto_increment_seed = true;
        view.explaino_seed_rate = 2.0f;
        params.explaino_seed = 0.0;
        view.explaino_seed_drift = 0.777f;

        for (int i = 0; i < 100; ++i) {
            ApplyExplainoSeedDynamics(stats, 0.016, view, params);
        }
        if (!NearlyEqual(ExplainoSeedCombined(view, params), 3.977, 0.01)) {
            std::cerr << "Combined seed should advance smoothly through time (got " << ExplainoSeedCombined(view, params) << ")\n";
            return 1;
        }
        if (!NearlyEqual(params.explaino_seed, 3.0, 1e-6) || !NearlyEqual(view.explaino_seed_drift, 0.977f, 0.01)) {
            std::cerr << "Normalized seed split should be base=3 drift~=0.977 (got base=" << params.explaino_seed << ", drift=" << view.explaino_seed_drift << ")\n";
            return 1;
        }
    }

    // Default seed rate should be 0.001, lowered for fine-grained exploration.
    {
        ViewState view{};
        if (!NearlyEqual(view.explaino_seed_rate, 0.001f, 1e-6)) {
            std::cerr << "Default explaino_seed_rate should be 0.001 (got " << view.explaino_seed_rate << ")\n";
            return 1;
        }
    }

    std::cout << "test_explaino_seed_dynamics: all passed\n";
    return 0;
}