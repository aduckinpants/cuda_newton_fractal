#include "../src/explaino_seed.h"

#include <cmath>
#include <iostream>

static bool NearlyEqual(double a, double b, double eps = 1.0e-9) {
    return std::fabs(a - b) <= eps;
}

int main() {
    {
        ViewState view{};
        KernelParams params{};
        ExplainoSeedSetCombined(view, params, 3.75);
        if (!NearlyEqual(params.explaino_seed, 3.0) || !NearlyEqual(view.explaino_seed_drift, 0.75f)) {
            std::cerr << "ExplainoSeedSetCombined positive case failed\n";
            return 1;
        }
        if (!NearlyEqual(ExplainoSeedCombined(view, params), 3.75)) {
            std::cerr << "ExplainoSeedCombined positive case failed\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        ExplainoSeedSetCombined(view, params, -0.25);
        if (!NearlyEqual(params.explaino_seed, -1.0) || !NearlyEqual(view.explaino_seed_drift, 0.75f)) {
            std::cerr << "ExplainoSeedSetCombined negative normalization failed\n";
            return 1;
        }
        if (!NearlyEqual(ExplainoSeedCombined(view, params), -0.25)) {
            std::cerr << "ExplainoSeedCombined negative case failed\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        params.explaino_seed = 5.0;
        view.explaino_seed_drift = 1.75f;
        ExplainoSeedNormalize(view, params);
        if (!NearlyEqual(params.explaino_seed, 6.0) || !NearlyEqual(view.explaino_seed_drift, 0.75f)) {
            std::cerr << "ExplainoSeedNormalize carry failed\n";
            return 1;
        }
    }

    return 0;
}