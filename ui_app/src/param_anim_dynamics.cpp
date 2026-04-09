#include "param_anim_dynamics.h"

#include "explaino_seed.h"
#include "fractal_family_rules.h"
#include "schema_binding.h"

#include <cmath>
#include <cstring>

// Resolve animation target by name using the BindFloat surface.
// Tries "fractal.params.<name>" then "fractal.view.<name>".
// Returns nullptr if name is unknown or "none".
//
// DO NOT replace this with an enum or switch/case dispatch.
// BindFloat IS the property registry. Adding a new animatable param
// requires only a BindFloat entry + a schema dropdown option.
// test_param_anim_generic.cpp enforces this contract.
static float* ResolveAnimTarget(const char* name, ViewState& view, KernelParams& params) {
    if (!name[0] || std::strcmp(name, "none") == 0) return nullptr;

    BindingContext ctx;
    ctx.view = &view;
    ctx.params = &params;
    float* ptr = nullptr;

    std::string paramPath = std::string("fractal.params.") + name;
    if (ctx.BindFloat(paramPath, &ptr)) return ptr;

    std::string viewPath = std::string("fractal.view.") + name;
    if (ctx.BindFloat(viewPath, &ptr)) return ptr;

    return nullptr;
}

bool ApplyParamAnimDynamics(double deltaSeconds, ViewState& view, KernelParams& params) {
    if (view.param_anim_target[0] == '\0' || std::strcmp(view.param_anim_target, "none") == 0)
        return false;
    if (!std::isfinite(deltaSeconds) || deltaSeconds <= 0.0) return false;

    const double rate = std::fmax(0.0, static_cast<double>(view.param_anim_rate));
    if (rate == 0.0) return false;

    const double delta = rate * deltaSeconds;

    if (std::strcmp(view.param_anim_target, "seed") == 0) {
        if (!IsExplainoFamily(view.fractal_type)) return false;
        ExplainoSeedSetCombined(view, params, ExplainoSeedCombined(view, params) + delta);
        return true;
    }

    float* target = ResolveAnimTarget(view.param_anim_target, view, params);
    if (!target) return false;

    *target += static_cast<float>(delta);
    return true;
}
