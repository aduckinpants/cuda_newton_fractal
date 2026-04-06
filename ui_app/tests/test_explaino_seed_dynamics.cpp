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
        // Auto-increment now advances base seed only, leaving drift untouched.
        // rate=0.5, dt=2.0 => delta=1.0 added to base (2.0 + 1.0 = 3.0)
        // drift stays at 0.25
        if (!NearlyEqual(params.explaino_seed, 3.0)) {
            std::cerr << "Expected auto-increment to advance the base seed by rate*dt\n";
            return 1;
        }
        if (!NearlyEqual(view.explaino_seed_drift, 0.25f)) {
            std::cerr << "Expected auto-increment to leave drift untouched\n";
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

    // Drift stability: auto-increment must NOT modify explaino_seed_drift
    // even across many frames (regression for seed-bleed-into-fraction bug).
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
        if (!NearlyEqual(view.explaino_seed_drift, 0.777f, 1e-6)) {
            std::cerr << "Drift slider must remain stable during auto-increment (got " << view.explaino_seed_drift << ")\n";
            return 1;
        }
        // Base seed should have advanced: 100 frames * 0.016s * 2.0/s = 3.2
        if (!NearlyEqual(params.explaino_seed, 3.2, 0.01)) {
            std::cerr << "Base seed should advance by rate*total_time (got " << params.explaino_seed << ")\n";
            return 1;
        }
    }

    // Default seed rate should be 0.05, not 0.35 (too fast for visual use).
    {
        ViewState view{};
        if (!NearlyEqual(view.explaino_seed_rate, 0.05f, 1e-6)) {
            std::cerr << "Default explaino_seed_rate should be 0.05 (got " << view.explaino_seed_rate << ")\n";
            return 1;
        }
    }

    std::cout << "test_explaino_seed_dynamics: all passed\n";
    return 0;
}