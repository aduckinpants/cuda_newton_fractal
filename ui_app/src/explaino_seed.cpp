#include "explaino_seed.h"

#include <algorithm>

static bool IsFiniteD(double x) {
    return std::isfinite(x);
}

static bool IsFiniteF(float x) {
    return std::isfinite(x);
}

double ExplainoSeedCombined(const ViewState& view, const KernelParams& params) {
    const double base = IsFiniteD(params.explaino_seed) ? params.explaino_seed : 0.0;
    const double drift = IsFiniteF(view.explaino_seed_drift) ? static_cast<double>(view.explaino_seed_drift) : 0.0;
    return base + drift;
}

void ExplainoSeedSetCombined(ViewState& view, KernelParams& params, double combined) {
    if (!IsFiniteD(combined)) {
        params.explaino_seed = 0.0;
        view.explaino_seed_drift = 0.0f;
        return;
    }

    double base = std::floor(combined);
    double frac = combined - base;

    if (!IsFiniteD(frac)) frac = 0.0;

    if (frac < 0.0) {
        double adjust = std::ceil(-frac);
        base -= adjust;
        frac += adjust;
    }

    if (frac >= 1.0) {
        double carry = std::floor(frac);
        base += carry;
        frac -= carry;
    }
    frac = std::clamp(frac, 0.0, 0.999999999999);

    params.explaino_seed = base;
    view.explaino_seed_drift = static_cast<float>(frac);
}

void ExplainoSeedNormalize(ViewState& view, KernelParams& params) {
    ExplainoSeedSetCombined(view, params, ExplainoSeedCombined(view, params));
}