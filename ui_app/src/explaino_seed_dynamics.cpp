#include "explaino_seed_dynamics.h"

#include "explaino_seed.h"
#include "fractal_family_rules.h"

#include <cmath>

namespace {

} // namespace

bool ApplyExplainoSeedDynamics(const RenderStats& stats,
    double deltaSeconds,
    ViewState& view,
    KernelParams& params) {
    (void)stats;
    if (!IsExplainoFamily(view.fractal_type)) return false;
    if (!std::isfinite(deltaSeconds) || deltaSeconds <= 0.0) return false;

    if (view.auto_increment_seed) {
        const double ratePerSecond = std::fmax(0.0, static_cast<double>(view.explaino_seed_rate));
        const double delta = ratePerSecond * deltaSeconds;
        if (delta != 0.0) {
            ExplainoSeedSetCombined(view, params, ExplainoSeedCombined(view, params) + delta);
            return true;
        }
    }

    return false;
}