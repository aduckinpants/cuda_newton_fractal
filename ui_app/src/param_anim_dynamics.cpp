#include "param_anim_dynamics.h"

#include "explaino_seed.h"
#include "fractal_family_rules.h"

#include <cmath>

static float* ResolveAnimTarget(ParamAnimTarget target, ViewState& view, KernelParams& params) {
    switch (target) {
    case ParamAnimTarget::damping:        return &params.explaino_damping;
    case ParamAnimTarget::warp_strength:  return &params.explaino_warp_strength;
    case ParamAnimTarget::root_spread:    return &params.explaino_root_spread;
    case ParamAnimTarget::mix:            return &params.explaino_mix;
    case ParamAnimTarget::nova_alpha:     return &params.nova_alpha;
    case ParamAnimTarget::phoenix_p_real: return &params.phoenix_p_real;
    case ParamAnimTarget::multibrot_power:return &params.multibrot_power_float;
    case ParamAnimTarget::lambda_real:    return &params.lambda_real;
    case ParamAnimTarget::momentum_beta:  return &params.momentum_beta;
    case ParamAnimTarget::explaino_phase: return &view.explaino_phase;
    default: return nullptr;
    }
}

bool ApplyParamAnimDynamics(double deltaSeconds, ViewState& view, KernelParams& params) {
    if (view.param_anim_target == ParamAnimTarget::none) return false;
    if (!std::isfinite(deltaSeconds) || deltaSeconds <= 0.0) return false;

    const double rate = std::fmax(0.0, static_cast<double>(view.param_anim_rate));
    if (rate == 0.0) return false;

    const double delta = rate * deltaSeconds;

    if (view.param_anim_target == ParamAnimTarget::seed) {
        if (!IsExplainoFamily(view.fractal_type)) return false;
        ExplainoSeedSetCombined(view, params, ExplainoSeedCombined(view, params) + delta);
        return true;
    }

    float* target = ResolveAnimTarget(view.param_anim_target, view, params);
    if (!target) return false;

    *target += static_cast<float>(delta);
    return true;
}
