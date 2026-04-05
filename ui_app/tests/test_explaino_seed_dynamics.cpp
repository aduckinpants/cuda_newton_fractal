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
        if (!NearlyEqual(ExplainoSeedCombined(view, params), 3.25)) {
            std::cerr << "Expected auto-increment to advance the combined seed smoothly\n";
            return 1;
        }
        if (!NearlyEqual(params.explaino_seed, 3.0) || !NearlyEqual(view.explaino_seed_drift, 0.25f)) {
            std::cerr << "Expected auto-increment to carry whole-seed overflow into the base seed\n";
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

    std::cout << "test_explaino_seed_dynamics: all passed\n";
    return 0;
}