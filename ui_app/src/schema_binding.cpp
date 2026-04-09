#include "schema_binding.h"
#include "explaino_seed.h"
#include "imgui.h"
#include "ui_schema_grouping.h"

#include <cmath>
#include <cstring>
#include <string>
#include <vector>

std::string BindingContext::GetEnumId(const std::string& path) const {
    if (params && path == "fractal.params.poly_kind") {
        switch (params->poly_kind) {
        case PolyKind::z3_minus_1: return "z3_minus_1";
        case PolyKind::z4_minus_1: return "z4_minus_1";
        case PolyKind::custom: return "custom";
        }
    }
    if (params && path == "fractal.params.transcendental_func") {
        switch (params->transcendental_func) {
        case TranscendentalFunc::f_sin: return "f_sin";
        case TranscendentalFunc::f_exp_minus_1: return "f_exp_minus_1";
        case TranscendentalFunc::f_cosh: return "f_cosh";
        }
    }
    if (params && path == "fractal.params.mcmullen_preset") {
        switch (params->mcmullen_preset) {
        case McMullenPreset::z3_z3: return "z3_z3";
        case McMullenPreset::z2_z2: return "z2_z2";
        case McMullenPreset::z4_z2: return "z4_z2";
        case McMullenPreset::z3_z2: return "z3_z2";
        }
    }
    if (params && path == "fractal.params.coloring_mode") {
        switch (params->coloring_mode) {
        case ColoringMode::root_basin: return "root_basin";
        case ColoringMode::iteration_count: return "iteration_count";
        case ColoringMode::smooth_escape: return "smooth_escape";
        case ColoringMode::joy_basins: return "joy_basins";
        case ColoringMode::phase: return "phase";
        case ColoringMode::iteration_bands: return "iteration_bands";
        }
    }
    if (view && path == "fractal.view.fractal_type") {
        switch (view->fractal_type) {
        case FractalType::newton: return "newton";
        case FractalType::nova: return "nova";
        case FractalType::mandelbrot: return "mandelbrot";
        case FractalType::julia: return "julia";
        case FractalType::burning_ship: return "burning_ship";
        case FractalType::multibrot: return "multibrot";
        case FractalType::phoenix: return "phoenix";
        case FractalType::explaino: return "explaino";
        case FractalType::explaino_y: return "explaino_y";
        case FractalType::explaino_fp: return "explaino_fp";
        case FractalType::explaino_nova: return "explaino_nova";
        case FractalType::explaino_halley: return "explaino_halley";
        case FractalType::explaino_dual: return "explaino_dual";
        case FractalType::explaino_mult: return "explaino_mult";
        case FractalType::explaino_phoenix: return "explaino_phoenix";
        case FractalType::explaino_transcendental: return "explaino_transcendental";
        case FractalType::explaino_inertial: return "explaino_inertial";
        case FractalType::explaino_julia: return "explaino_julia";
        case FractalType::explaino_rational: return "explaino_rational";
        case FractalType::explaino_joy: return "explaino_joy";
        case FractalType::explaino_fold: return "explaino_fold";
        case FractalType::explaino_bell: return "explaino_bell";
        case FractalType::explaino_ripple: return "explaino_ripple";
        case FractalType::explaino_splice: return "explaino_splice";
        case FractalType::explaino_vortex: return "explaino_vortex";
        case FractalType::multicorn: return "multicorn";
        case FractalType::halley: return "halley";
        case FractalType::collatz: return "collatz";
        case FractalType::explaino_collatz: return "explaino_collatz";
        case FractalType::mcmullen: return "mcmullen";
        case FractalType::lambda_map: return "lambda";
        case FractalType::explaino_lambda: return "explaino_lambda";
        case FractalType::explaino_rational_escape: return "explaino_rational_escape";
        case FractalType::spider: return "spider";
        case FractalType::celtic_mandelbrot: return "celtic_mandelbrot";
        case FractalType::perpendicular_burning_ship: return "perpendicular_burning_ship";
        }
    }
    if (view && path == "fractal.view.camera_behavior") {
        switch (view->camera_behavior) {
        case CameraBehavior::manual: return "manual";
        case CameraBehavior::complexity: return "complexity";
        case CameraBehavior::orbit: return "orbit";
        case CameraBehavior::entropy: return "entropy";
        case CameraBehavior::off: return "off";
        }
    }
    if (render && path == "fractal.render.sample_tier") {
        switch (render->sample_tier) {
        case SampleTier::tier_auto: return "tier_auto";
        case SampleTier::fast: return "fast";
        case SampleTier::standard: return "standard";
        }
    }
    if (view && path == "fractal.view.param_anim_target") {
        return std::string(view->param_anim_target);
    }
    return {};
}

bool BindingContext::SetEnumId(const std::string& path, const std::string& id) {
    if (params && path == "fractal.params.poly_kind") {
        if (id == "z3_minus_1") params->poly_kind = PolyKind::z3_minus_1;
        else if (id == "z4_minus_1") params->poly_kind = PolyKind::z4_minus_1;
        else if (id == "custom") params->poly_kind = PolyKind::custom;
        else return false;
        return true;
    }
    if (params && path == "fractal.params.transcendental_func") {
        if (id == "f_sin") params->transcendental_func = TranscendentalFunc::f_sin;
        else if (id == "f_exp_minus_1") params->transcendental_func = TranscendentalFunc::f_exp_minus_1;
        else if (id == "f_cosh") params->transcendental_func = TranscendentalFunc::f_cosh;
        else return false;
        return true;
    }
    if (params && path == "fractal.params.mcmullen_preset") {
        if (id == "z3_z3") params->mcmullen_preset = McMullenPreset::z3_z3;
        else if (id == "z2_z2") params->mcmullen_preset = McMullenPreset::z2_z2;
        else if (id == "z4_z2") params->mcmullen_preset = McMullenPreset::z4_z2;
        else if (id == "z3_z2") params->mcmullen_preset = McMullenPreset::z3_z2;
        else return false;
        return true;
    }
    if (params && path == "fractal.params.coloring_mode") {
        if (id == "root_basin") params->coloring_mode = ColoringMode::root_basin;
        else if (id == "iteration_count") params->coloring_mode = ColoringMode::iteration_count;
        else if (id == "smooth_escape") params->coloring_mode = ColoringMode::smooth_escape;
        else if (id == "joy_basins") params->coloring_mode = ColoringMode::joy_basins;
        else if (id == "phase") params->coloring_mode = ColoringMode::phase;
        else if (id == "iteration_bands") params->coloring_mode = ColoringMode::iteration_bands;
        else return false;
        return true;
    }
    if (view && path == "fractal.view.fractal_type") {
        if (id == "newton") view->fractal_type = FractalType::newton;
        else if (id == "nova") view->fractal_type = FractalType::nova;
        else if (id == "mandelbrot") view->fractal_type = FractalType::mandelbrot;
        else if (id == "julia") view->fractal_type = FractalType::julia;
        else if (id == "burning_ship") view->fractal_type = FractalType::burning_ship;
        else if (id == "multibrot") view->fractal_type = FractalType::multibrot;
        else if (id == "phoenix") view->fractal_type = FractalType::phoenix;
        else if (id == "explaino") view->fractal_type = FractalType::explaino;
        else if (id == "explaino_y") view->fractal_type = FractalType::explaino_y;
        else if (id == "explaino_fp") view->fractal_type = FractalType::explaino_fp;
        else if (id == "explaino_nova") view->fractal_type = FractalType::explaino_nova;
        else if (id == "explaino_halley") view->fractal_type = FractalType::explaino_halley;
        else if (id == "explaino_dual") view->fractal_type = FractalType::explaino_dual;
        else if (id == "explaino_mult") view->fractal_type = FractalType::explaino_mult;
        else if (id == "explaino_phoenix") view->fractal_type = FractalType::explaino_phoenix;
        else if (id == "explaino_transcendental") view->fractal_type = FractalType::explaino_transcendental;
        else if (id == "explaino_inertial") view->fractal_type = FractalType::explaino_inertial;
        else if (id == "explaino_julia") view->fractal_type = FractalType::explaino_julia;
        else if (id == "explaino_rational") view->fractal_type = FractalType::explaino_rational;
        else if (id == "explaino_joy") view->fractal_type = FractalType::explaino_joy;
        else if (id == "explaino_fold") view->fractal_type = FractalType::explaino_fold;
        else if (id == "explaino_bell") view->fractal_type = FractalType::explaino_bell;
        else if (id == "explaino_ripple") view->fractal_type = FractalType::explaino_ripple;
        else if (id == "explaino_splice") view->fractal_type = FractalType::explaino_splice;
        else if (id == "explaino_vortex") view->fractal_type = FractalType::explaino_vortex;
        else if (id == "multicorn") view->fractal_type = FractalType::multicorn;
        else if (id == "halley") view->fractal_type = FractalType::halley;
        else if (id == "collatz") view->fractal_type = FractalType::collatz;
        else if (id == "explaino_collatz") view->fractal_type = FractalType::explaino_collatz;
        else if (id == "mcmullen") view->fractal_type = FractalType::mcmullen;
        else if (id == "lambda") view->fractal_type = FractalType::lambda_map;
        else if (id == "explaino_lambda") view->fractal_type = FractalType::explaino_lambda;
        else if (id == "explaino_rational_escape") view->fractal_type = FractalType::explaino_rational_escape;
        else if (id == "spider") view->fractal_type = FractalType::spider;
        else if (id == "celtic_mandelbrot") view->fractal_type = FractalType::celtic_mandelbrot;
        else if (id == "perpendicular_burning_ship") view->fractal_type = FractalType::perpendicular_burning_ship;
        else return false;
        return true;
    }
    if (view && path == "fractal.view.camera_behavior") {
        if (id == "manual") view->camera_behavior = CameraBehavior::manual;
        else if (id == "complexity") view->camera_behavior = CameraBehavior::complexity;
        else if (id == "orbit") view->camera_behavior = CameraBehavior::orbit;
        else if (id == "entropy") view->camera_behavior = CameraBehavior::entropy;
        else if (id == "off") view->camera_behavior = CameraBehavior::off;
        else return false;
        return true;
    }
    if (render && path == "fractal.render.sample_tier") {
        if (id == "tier_auto") render->sample_tier = SampleTier::tier_auto;
        else if (id == "fast") render->sample_tier = SampleTier::fast;
        else if (id == "standard") render->sample_tier = SampleTier::standard;
        else return false;
        return true;
    }
    if (view && path == "fractal.view.param_anim_target") {
        if (id.size() >= sizeof(view->param_anim_target)) return false;
        std::memcpy(view->param_anim_target, id.c_str(), id.size() + 1);
        return true;
    }
    return false;
}

bool BindingContext::GetBoolValue(const std::string& path, bool& out) const {
    bool* ptr = nullptr;
    BindingContext* self = const_cast<BindingContext*>(this);
    if (!self->BindBool(path, &ptr) || !ptr) return false;
    out = *ptr;
    return true;
}

bool BindingContext::GetIntValue(const std::string& path, int& out) const {
    int* ptr = nullptr;
    BindingContext* self = const_cast<BindingContext*>(this);
    if (!self->BindInt(path, &ptr) || !ptr) return false;
    out = *ptr;
    return true;
}

bool BindingContext::GetFloatValue(const std::string& path, float& out) const {
    float* ptr = nullptr;
    BindingContext* self = const_cast<BindingContext*>(this);
    if (!self->BindFloat(path, &ptr) || !ptr) return false;
    out = *ptr;
    return true;
}

bool BindingContext::GetDoubleValue(const std::string& path, double& out) const {
    double* ptr = nullptr;
    BindingContext* self = const_cast<BindingContext*>(this);
    if (!self->BindDouble(path, &ptr) || !ptr) return false;
    out = *ptr;
    return true;
}

bool BindingContext::EvalVisibleIf(const UISchemaPredicate& pred) const {
    // Fail-open: if we cannot evaluate a predicate, keep the control visible.
    if (pred.op.empty() || pred.path.empty()) return true;

    // Enum predicates
    std::string curEnum = GetEnumId(pred.path);
    if (!curEnum.empty()) {
        if (pred.op == "eq") return curEnum == pred.value;
        if (pred.op == "neq") return curEnum != pred.value;
        if (pred.op == "in") {
            // Comma-separated list: "a,b,c" (whitespace tolerated).
            std::string v = pred.value;
            size_t i = 0;
            while (i < v.size()) {
                while (i < v.size() && (v[i] == ' ' || v[i] == '\t' || v[i] == '\n' || v[i] == '\r' || v[i] == ',')) ++i;
                size_t j = i;
                while (j < v.size() && v[j] != ',') ++j;
                std::string tok = v.substr(i, j - i);
                // Trim trailing whitespace
                while (!tok.empty() && (tok.back() == ' ' || tok.back() == '\t' || tok.back() == '\n' || tok.back() == '\r')) tok.pop_back();
                if (!tok.empty() && tok == curEnum) return true;
                i = j + 1;
            }
            return false;
        }
        return true;
    }

    // Bool predicates
    bool curB = false;
    if (GetBoolValue(pred.path, curB)) {
        bool rhs = (pred.value == "true" || pred.value == "1");
        if (pred.op == "eq") return curB == rhs;
        if (pred.op == "neq") return curB != rhs;
        return true;
    }

    // Numeric predicates (int/float)
    double curN = 0.0;
    {
        int curI = 0;
        float curF = 0.0f;
        double curD = 0.0;
        if (GetIntValue(pred.path, curI)) curN = (double)curI;
        else if (GetFloatValue(pred.path, curF)) curN = (double)curF;
        else if (GetDoubleValue(pred.path, curD)) curN = curD;
        else return true;
    }

    double rhsN = 0.0;
    try {
        rhsN = std::stod(pred.value);
    } catch (...) {
        return true;
    }

    if (pred.op == "eq") return curN == rhsN;
    if (pred.op == "neq") return curN != rhsN;
    if (pred.op == "lt") return curN < rhsN;
    if (pred.op == "lte") return curN <= rhsN;
    if (pred.op == "gt") return curN > rhsN;
    if (pred.op == "gte") return curN >= rhsN;
    return true;
}

bool BindingContext::BindFloat(const std::string& path, float** outPtr) {
    if (view) {
        if (path == "fractal.view.center.x") { *outPtr = &view->center.x; return true; }
        if (path == "fractal.view.center.y") { *outPtr = &view->center.y; return true; }
        if (path == "fractal.view.zoom") { *outPtr = &view->zoom; return true; }
        if (path == "fractal.view.rotation") { *outPtr = &view->rotation_degrees; return true; }
        if (path == "fractal.view.dive_speed") { *outPtr = &view->dive_speed; return true; }
        if (path == "fractal.view.explaino_phase") { *outPtr = &view->explaino_phase; return true; }
        if (path == "fractal.view.explaino_seed_drift") { *outPtr = &view->explaino_seed_drift; return true; }
        if (path == "fractal.view.explaino_seed_rate") { *outPtr = &view->explaino_seed_rate; return true; }
        if (path == "fractal.view.explaino_phase_strength") { *outPtr = &view->explaino_phase_strength; return true; }
        if (path == "fractal.view.param_anim_rate") { *outPtr = &view->param_anim_rate; return true; }
    }
    if (params) {
        if (path == "fractal.params.epsilon") { *outPtr = &params->epsilon; return true; }
        if (path == "fractal.params.nova_alpha") { *outPtr = &params->nova_alpha; return true; }
        if (path == "fractal.params.phoenix_p_real") { *outPtr = &params->phoenix_p_real; return true; }
        if (path == "fractal.params.phoenix_p_imag") { *outPtr = &params->phoenix_p_imag; return true; }
        if (path == "fractal.params.lambda_real") { *outPtr = &params->lambda_real; return true; }
        if (path == "fractal.params.lambda_imag") { *outPtr = &params->lambda_imag; return true; }
        if (path == "fractal.params.exposure") { *outPtr = &params->exposure; return true; }
        if (path == "fractal.params.multibrot_power_float") { *outPtr = &params->multibrot_power_float; return true; }
        if (path == "fractal.params.multibrot_power") { *outPtr = &params->multibrot_power_float; return true; }
        if (path == "fractal.params.color_saturation") { *outPtr = &params->color_saturation; return true; }
        if (path == "fractal.params.color_contrast") { *outPtr = &params->color_contrast; return true; }
        if (path == "fractal.params.color_tint_r") { *outPtr = &params->color_tint_r; return true; }
        if (path == "fractal.params.color_tint_g") { *outPtr = &params->color_tint_g; return true; }
        if (path == "fractal.params.color_tint_b") { *outPtr = &params->color_tint_b; return true; }
        if (path == "fractal.params.explaino_warp_strength") { *outPtr = &params->explaino_warp_strength; return true; }
        if (path == "fractal.params.explaino_mix") { *outPtr = &params->explaino_mix; return true; }
        if (path == "fractal.params.explaino_root_spread") { *outPtr = &params->explaino_root_spread; return true; }
        if (path == "fractal.params.explaino_damping") { *outPtr = &params->explaino_damping; return true; }
        if (path == "fractal.params.explaino_cluster_radius") { *outPtr = &params->explaino_cluster_radius; return true; }
        if (path == "fractal.params.momentum_beta") { *outPtr = &params->momentum_beta; return true; }
        if (path == "fractal.params.joy_coupling") { *outPtr = &params->joy_coupling; return true; }
        if (path == "fractal.params.fold_coupling") { *outPtr = &params->fold_coupling; return true; }
        if (path == "fractal.params.bell_coupling") { *outPtr = &params->bell_coupling; return true; }
        if (path == "fractal.params.ripple_amplitude") { *outPtr = &params->ripple_amplitude; return true; }
        if (path == "fractal.params.splice_offset") { *outPtr = &params->splice_offset; return true; }
        if (path == "fractal.params.vortex_strength") { *outPtr = &params->vortex_strength; return true; }
        // Short-name aliases for param_anim_target resolution.
        // These let the animation system resolve "damping" -> explaino_damping, etc.
        if (path == "fractal.params.damping") { *outPtr = &params->explaino_damping; return true; }
        if (path == "fractal.params.warp_strength") { *outPtr = &params->explaino_warp_strength; return true; }
        if (path == "fractal.params.root_spread") { *outPtr = &params->explaino_root_spread; return true; }
        if (path == "fractal.params.mix") { *outPtr = &params->explaino_mix; return true; }
        if (path == "fractal.params.poly_coeffs.0") { *outPtr = &params->poly_coeffs[0]; return true; }
        if (path == "fractal.params.poly_coeffs.1") { *outPtr = &params->poly_coeffs[1]; return true; }
        if (path == "fractal.params.poly_coeffs.2") { *outPtr = &params->poly_coeffs[2]; return true; }
        if (path == "fractal.params.poly_coeffs.3") { *outPtr = &params->poly_coeffs[3]; return true; }
        if (path == "fractal.params.poly_coeffs.4") { *outPtr = &params->poly_coeffs[4]; return true; }
    }
    if (render) {
        if (path == "fractal.render.preview_target_fps") { *outPtr = &render->preview_target_fps; return true; }
        if (path == "fractal.render.preview_min_scale") { *outPtr = &render->preview_min_scale; return true; }
    }
    return false;
}

bool BindingContext::BindDouble(const std::string& path, double** outPtr) {
    if (!params) return false;
    if (path == "fractal.params.explaino_seed") { *outPtr = &params->explaino_seed; return true; }
    if (path == "fractal.params.explaino_seed_b") { *outPtr = &params->explaino_seed_b; return true; }
    return false;
}

bool BindingContext::BindInt(const std::string& path, int** outPtr) {
    if (params) {
        if (path == "fractal.params.max_iter") { *outPtr = &params->max_iter; return true; }
        if (path == "fractal.params.multibrot_power") { *outPtr = &params->multibrot_power; return true; }
    }
    if (render) {
        if (path == "fractal.render.resolution.x") { *outPtr = &render->resolution.x; return true; }
        if (path == "fractal.render.resolution.y") { *outPtr = &render->resolution.y; return true; }
        if (path == "fractal.render.block_size") { *outPtr = &render->block_size; return true; }
        if (path == "fractal.render.device_id") { *outPtr = &render->device_id; return true; }
        if (path == "fractal.render.interaction_debounce_ms") { *outPtr = &render->interaction_debounce_ms; return true; }
    }
    if (lens && path == "fractal.lens.downsample") { *outPtr = &lens->downsample; return true; }
    return false;
}

bool BindingContext::BindBool(const std::string& path, bool** outPtr) {
    if (!view || !render) return false;
    if (path == "fractal.view.auto_refresh") { *outPtr = &view->auto_refresh; return true; }
    if (path == "fractal.view.auto_dive") { *outPtr = &view->auto_dive; return true; }
    if (path == "fractal.view.auto_max_iter") { *outPtr = &view->auto_max_iter; return true; }
    if (path == "fractal.view.explaino_alive") { *outPtr = &view->explaino_alive; return true; }
    if (path == "fractal.view.explaino_seed_tween") { *outPtr = &view->explaino_seed_tween; return true; }
    if (path == "fractal.view.auto_increment_seed") { *outPtr = &view->auto_increment_seed; return true; }
    if (path == "fractal.render.benchmark") { *outPtr = &render->benchmark; return true; }
    if (lens && path == "fractal.lens.enabled") { *outPtr = &lens->enabled; return true; }
    return false;
}

namespace {

bool ApplyBoolSchemaDefault(const UISchemaControl& control, BindingContext& ctx, bool* ioDirty) {
    bool* value = nullptr;
    if (!ctx.BindBool(control.binding.path, &value) || !value) return false;

    bool newValue = *value;
    if (control.def.is_bool()) newValue = control.def.as_bool();
    else if (control.def.is_number()) newValue = (control.def.as_number() != 0.0);
    else if (control.def.is_string()) newValue = (control.def.as_string() == "true" || control.def.as_string() == "1");
    else return false;

    if (*value == newValue) return false;
    *value = newValue;
    if (ioDirty) *ioDirty = true;
    return true;
}

bool ApplyIntSchemaDefault(const UISchemaControl& control, BindingContext& ctx, bool* ioDirty) {
    int* value = nullptr;
    if (!ctx.BindInt(control.binding.path, &value) || !value) return false;

    int newValue = *value;
    if (control.def.is_number()) newValue = static_cast<int>(control.def.as_number());
    else if (control.def.is_string()) {
        try {
            newValue = std::stoi(control.def.as_string());
        } catch (...) {
            return false;
        }
    } else {
        return false;
    }

    if (*value == newValue) return false;
    *value = newValue;
    if (ioDirty) *ioDirty = true;
    return true;
}

bool ApplyFloatSchemaDefault(const UISchemaControl& control, BindingContext& ctx, bool* ioDirty) {
    float* value = nullptr;
    if (!ctx.BindFloat(control.binding.path, &value) || !value) return false;

    float newValue = *value;
    if (control.def.is_number()) newValue = static_cast<float>(control.def.as_number());
    else if (control.def.is_string()) {
        try {
            newValue = static_cast<float>(std::stod(control.def.as_string()));
        } catch (...) {
            return false;
        }
    } else {
        return false;
    }

    if (*value == newValue) return false;
    *value = newValue;
    if (ioDirty) *ioDirty = true;
    return true;
}

bool ApplyDoubleSchemaDefault(const UISchemaControl& control, BindingContext& ctx, bool* ioDirty) {
    double* value = nullptr;
    if (!ctx.BindDouble(control.binding.path, &value) || !value) return false;

    double newValue = *value;
    if (control.def.is_number()) newValue = control.def.as_number();
    else if (control.def.is_string()) {
        try {
            newValue = std::stod(control.def.as_string());
        } catch (...) {
            return false;
        }
    } else {
        return false;
    }

    if (control.binding.path == "fractal.params.explaino_seed" && ctx.view && ctx.params) {
        if (ExplainoSeedCombined(*ctx.view, *ctx.params) == newValue) return false;
        ExplainoSeedSetCombined(*ctx.view, *ctx.params, newValue);
        if (ioDirty) *ioDirty = true;
        return true;
    }

    if (*value == newValue) return false;
    *value = newValue;
    if (ioDirty) *ioDirty = true;
    return true;
}

bool ApplyEnumSchemaDefault(const UISchemaControl& control, BindingContext& ctx, bool* ioDirty) {
    if (!control.def.is_string()) return false;

    const std::string currentValue = ctx.GetEnumId(control.binding.path);
    const std::string& wantedValue = control.def.as_string();
    if (currentValue == wantedValue) return false;

    const bool applied = ctx.SetEnumId(control.binding.path, wantedValue);
    if (applied && ioDirty) *ioDirty = true;
    return applied;
}

bool IsKnownActionBindingPath(const std::string& path) {
    return path == "fractal.actions.render_once" ||
        path == "fractal.actions.reset_view" ||
        path == "fractal.actions.reset_all" ||
        path == "fractal.actions.load_state" ||
        path == "fractal.actions.capture_finding" ||
        path == "fractal.actions.capture_diagnostic" ||
        path == "fractal.actions.next_seed" ||
        path == "fractal.actions.prev_seed";
}

bool ValidateEnumBindingPath(const UISchemaControl& control, BindingContext& ctx, std::string* outError) {
    if (!ctx.GetEnumId(control.binding.path).empty()) {
        return true;
    }

    ViewState viewCopy{};
    KernelParams paramsCopy{};
    RenderSettings renderCopy{};
    LensSettings lensCopy{};
    BindingContext probe = ctx;
    if (ctx.view) {
        viewCopy = *ctx.view;
        probe.view = &viewCopy;
    }
    if (ctx.params) {
        paramsCopy = *ctx.params;
        probe.params = &paramsCopy;
    }
    if (ctx.render) {
        renderCopy = *ctx.render;
        probe.render = &renderCopy;
    }
    if (ctx.lens) {
        lensCopy = *ctx.lens;
        probe.lens = &lensCopy;
    }

    for (const auto& option : control.options) {
        if (probe.SetEnumId(control.binding.path, option.id)) {
            return true;
        }
    }

    if (outError) {
        *outError = "Unknown enum binding path: " + control.binding.path + " (control: " + control.id + ")";
    }
    return false;
}

bool ValidateParamBinding(const UISchemaControl& control, BindingContext& ctx, std::string* outError) {
    const std::string& path = control.binding.path;
    if (control.value_type == "bool") {
        bool* value = nullptr;
        if (!ctx.BindBool(path, &value) || !value) {
            if (outError) *outError = "Bind failed for bool path: " + path + " (control: " + control.id + ")";
            return false;
        }
        return true;
    }
    if (control.value_type == "int") {
        int* value = nullptr;
        if (!ctx.BindInt(path, &value) || !value) {
            if (outError) *outError = "Bind failed for int path: " + path + " (control: " + control.id + ")";
            return false;
        }
        return true;
    }
    if (control.value_type == "float") {
        float* value = nullptr;
        if (!ctx.BindFloat(path, &value) || !value) {
            if (outError) *outError = "Bind failed for float path: " + path + " (control: " + control.id + ")";
            return false;
        }
        return true;
    }
    if (control.value_type == "double") {
        double* value = nullptr;
        if (!ctx.BindDouble(path, &value) || !value) {
            if (outError) *outError = "Bind failed for double path: " + path + " (control: " + control.id + ")";
            return false;
        }
        return true;
    }
    if (control.value_type == "enum") {
        return ValidateEnumBindingPath(control, ctx, outError);
    }
    return true;
}

} // namespace

bool ApplySchemaDefaultForControl(const UISchemaControl& c, BindingContext& ctx, bool* ioDirty) {
    if (!c.has_binding || c.binding.kind != "param" || !c.has_default) return false;

    if (c.value_type == "bool") return ApplyBoolSchemaDefault(c, ctx, ioDirty);
    if (c.value_type == "int") return ApplyIntSchemaDefault(c, ctx, ioDirty);
    if (c.value_type == "float") return ApplyFloatSchemaDefault(c, ctx, ioDirty);
    if (c.value_type == "double") return ApplyDoubleSchemaDefault(c, ctx, ioDirty);
    if (c.value_type == "enum") return ApplyEnumSchemaDefault(c, ctx, ioDirty);
    return false;
}

void ApplySchemaDefaults(const UISchema& schema, BindingContext& ctx, bool* ioDirty) {
    for (const auto& panel : schema.panels) {
        for (const auto& ctrl : panel.controls) {
            ApplySchemaDefaultForControl(ctrl, ctx, ioDirty);
        }
    }
}

bool ValidateSchemaBindings(const UISchema& schema, BindingContext& ctx, std::string* outError) {
    for (const auto& panel : schema.panels) {
        for (const auto& c : panel.controls) {
            if (!c.has_binding) continue;

            const auto& b = c.binding;
            if (b.path.empty() || b.kind.empty()) {
                if (outError) *outError = "Schema binding missing kind/path for control: " + c.id;
                return false;
            }

            if (b.kind == "action") {
                if (!IsKnownActionBindingPath(b.path)) {
                    if (outError) *outError = "Unknown action binding path: " + b.path + " (control: " + c.id + ")";
                    return false;
                }
                continue;
            }

            if (b.kind != "param") {
                if (outError) *outError = "Unknown binding kind: " + b.kind + " (control: " + c.id + ")";
                return false;
            }

            if (!ValidateParamBinding(c, ctx, outError)) {
                return false;
            }
        }
    }
    return true;
}

namespace {

void MarkDirtyIfChanged(bool changed, bool* ioDirty) {
    if (changed && ioDirty) *ioDirty = true;
}

void MarkCurrentItemInteraction(bool changed, bool* ioInteracted) {
    if ((changed || ImGui::IsItemActivated() || ImGui::IsItemActive() || ImGui::IsItemDeactivatedAfterEdit()) && ioInteracted) {
        *ioInteracted = true;
    }
}

void RenderControlHelp(const UISchemaControl& control) {
    if (!control.has_help) {
        return;
    }

    ImGui::TextDisabled("?");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(300.0f);
        ImGui::TextUnformatted(control.help.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
    ImGui::SameLine();
}

bool RenderDiagnosticLabel(const UISchemaControl& control, const char* detail) {
    ImGui::TextDisabled("%s (%s)", control.label.c_str(), detail);
    return false;
}

bool RenderActionControl(
    const UISchemaControl& control,
    const UISchemaBinding& binding,
    bool* ioRenderOnce,
    bool* ioInteracted) {
    if (binding.kind != "action") {
        return RenderDiagnosticLabel(control, "bad action binding");
    }

    if (ImGui::Button(control.label.c_str())) {
        if (ioInteracted) *ioInteracted = true;
        if (binding.path == "fractal.actions.render_once" && ioRenderOnce) {
            *ioRenderOnce = true;
        }
    }
    return true;
}

bool RenderCheckboxControl(
    const UISchemaControl& control,
    BindingContext& ctx,
    const UISchemaBinding& binding,
    bool* ioDirty,
    bool* ioInteracted) {
    bool* value = nullptr;
    if (!ctx.BindBool(binding.path, &value) || !value) {
        return RenderDiagnosticLabel(control, "bind failed");
    }

    const bool changed = ImGui::Checkbox(control.label.c_str(), value);
    MarkDirtyIfChanged(changed, ioDirty);
    MarkCurrentItemInteraction(changed, ioInteracted);
    return changed;
}

bool RenderIntControl(
    const UISchemaControl& control,
    BindingContext& ctx,
    const UISchemaBinding& binding,
    bool* ioDirty,
    bool* ioInteracted) {
    int* value = nullptr;
    if (!ctx.BindInt(binding.path, &value) || !value) {
        return RenderDiagnosticLabel(control, "bind failed");
    }

    const int minValue = control.has_min ? static_cast<int>(control.min) : 0;
    const int maxValue = control.has_max ? static_cast<int>(control.max) : 100;

    bool changed = false;
    if (control.type == "slider_int") {
        changed = ImGui::SliderInt(control.label.c_str(), value, minValue, maxValue);
    } else {
        const float speed = control.has_step ? static_cast<float>(control.step) : 1.0f;
        changed = ImGui::DragInt(control.label.c_str(), value, speed, minValue, maxValue);
    }
    MarkDirtyIfChanged(changed, ioDirty);
    MarkCurrentItemInteraction(changed, ioInteracted);
    return changed;
}

bool RenderFloatControl(
    const UISchemaControl& control,
    BindingContext& ctx,
    const UISchemaBinding& binding,
    bool* ioDirty,
    bool* ioInteracted) {
    float* value = nullptr;
    if (!ctx.BindFloat(binding.path, &value) || !value) {
        return RenderDiagnosticLabel(control, "bind failed");
    }

    const float minValue = control.has_min ? static_cast<float>(control.min) : 0.0f;
    const float maxValue = control.has_max ? static_cast<float>(control.max) : 1.0f;
    const float speed = control.has_step ? static_cast<float>(control.step) : 0.01f;
    const ImGuiSliderFlags flags = control.logarithmic ? ImGuiSliderFlags_Logarithmic : 0;

    bool changed = false;
    if (control.type == "slider_float") {
        changed = ImGui::SliderFloat(control.label.c_str(), value, minValue, maxValue, "%.5f", flags);
    } else {
        changed = ImGui::DragFloat(control.label.c_str(), value, speed, minValue, maxValue, "%.3f", flags);
    }
    MarkDirtyIfChanged(changed, ioDirty);
    MarkCurrentItemInteraction(changed, ioInteracted);
    return changed;
}

bool RenderExplainoSeedDoubleControl(
    const UISchemaControl& control,
    BindingContext& ctx,
    double minValue,
    double maxValue,
    double speed,
    const char* valueFormat,
    bool* ioDirty,
    bool* ioInteracted) {
    double displayed = ExplainoSeedCombined(*ctx.view, *ctx.params);
    bool changed = false;
    if (control.type == "slider_double") {
        changed = ImGui::SliderScalar(control.label.c_str(), ImGuiDataType_Double, &displayed, &minValue, &maxValue, valueFormat);
    } else {
        changed = ImGui::DragScalar(control.label.c_str(), ImGuiDataType_Double, &displayed, static_cast<float>(speed), &minValue, &maxValue, valueFormat);
    }
    ImGui::SameLine();
    const bool typedChanged = ImGui::InputDouble("##val", &displayed, 0.0, 0.0, valueFormat);
    if (typedChanged) changed = true;
    if (changed) {
        ExplainoSeedSetCombined(*ctx.view, *ctx.params, displayed);
    }
    MarkDirtyIfChanged(changed, ioDirty);
    MarkCurrentItemInteraction(changed, ioInteracted);
    return changed;
}

bool RenderDoubleControl(
    const UISchemaControl& control,
    BindingContext& ctx,
    const UISchemaBinding& binding,
    bool* ioDirty,
    bool* ioInteracted) {
    double* value = nullptr;
    if (!ctx.BindDouble(binding.path, &value) || !value) {
        return RenderDiagnosticLabel(control, "bind failed");
    }

    const double minValue = control.has_min ? control.min : 0.0;
    const double maxValue = control.has_max ? control.max : 1.0;
    const double speed = control.has_step ? control.step : 0.001;
    const char* valueFormat = "%.6f";

    if (binding.path == "fractal.params.explaino_seed" && ctx.view && ctx.params) {
        return RenderExplainoSeedDoubleControl(control, ctx, minValue, maxValue, speed, valueFormat, ioDirty, ioInteracted);
    }

    bool changed = false;
    if (control.type == "slider_double") {
        changed = ImGui::SliderScalar(control.label.c_str(), ImGuiDataType_Double, value, &minValue, &maxValue, valueFormat);
    } else {
        changed = ImGui::DragScalar(control.label.c_str(), ImGuiDataType_Double, value, static_cast<float>(speed), &minValue, &maxValue, valueFormat);
    }
    ImGui::SameLine();
    const bool typedChanged = ImGui::InputDouble("##val", value, 0.0, 0.0, valueFormat);
    if (typedChanged) changed = true;
    MarkDirtyIfChanged(changed, ioDirty);
    MarkCurrentItemInteraction(changed, ioInteracted);
    return changed;
}

bool RenderIntComboControl(
    const UISchemaControl& control,
    BindingContext& ctx,
    const UISchemaBinding& binding,
    bool* ioDirty,
    bool* ioInteracted) {
    int* value = nullptr;
    if (!ctx.BindInt(binding.path, &value) || !value) {
        return RenderDiagnosticLabel(control, "bind failed");
    }

    int currentIndex = 0;
    for (int index = 0; index < static_cast<int>(control.options.size()); ++index) {
        try {
            const int optionValue = std::stoi(control.options[index].id);
            if (optionValue == *value) {
                currentIndex = index;
                break;
            }
        } catch (...) {
        }
    }

    std::vector<const char*> labels;
    labels.reserve(control.options.size());
    for (const auto& option : control.options) {
        labels.push_back(option.label.c_str());
    }

    bool changed = false;
    if (!labels.empty() && ImGui::Combo(control.label.c_str(), &currentIndex, labels.data(), static_cast<int>(labels.size()))) {
        if (currentIndex >= 0 && currentIndex < static_cast<int>(control.options.size())) {
            try {
                const int newValue = std::stoi(control.options[currentIndex].id);
                if (newValue != *value) {
                    *value = newValue;
                    changed = true;
                }
            } catch (...) {
            }
        }
    }
    MarkDirtyIfChanged(changed, ioDirty);
    MarkCurrentItemInteraction(changed, ioInteracted);
    return changed;
}

bool RenderGroupedEnumComboControl(
    const UISchemaControl& control,
    BindingContext& ctx,
    const UISchemaBinding& binding,
    const std::string& currentId,
    bool* ioDirty,
    bool* ioInteracted) {
    const std::vector<std::string> groups = CollectOptionGroups(control);
    if (groups.size() <= 1) {
        return false;
    }

    std::string currentGroup = OptionGroupForId(control, currentId);
    if (currentGroup.empty()) {
        currentGroup = groups.front();
    }

    int groupIndex = 0;
    for (int index = 0; index < static_cast<int>(groups.size()); ++index) {
        if (groups[index] == currentGroup) {
            groupIndex = index;
            break;
        }
    }

    std::vector<const char*> groupLabels;
    groupLabels.reserve(groups.size());
    for (const auto& group : groups) {
        groupLabels.push_back(group.c_str());
    }

    std::string selectedId = currentId;
    bool groupChanged = false;
    bool groupInteracted = false;
    if (!groupLabels.empty() && ImGui::Combo("Category", &groupIndex, groupLabels.data(), static_cast<int>(groupLabels.size()))) {
        groupInteracted = true;
        const std::vector<const UISchemaOption*> groupedOptions = OptionsForGroup(control, groups[groupIndex]);
        if (!groupedOptions.empty()) {
            groupChanged = ctx.SetEnumId(binding.path, groupedOptions.front()->id);
            MarkDirtyIfChanged(groupChanged, ioDirty);
            selectedId = groupedOptions.front()->id;
        }
    }
    groupInteracted = groupInteracted || ImGui::IsItemActivated() || ImGui::IsItemActive() || ImGui::IsItemDeactivatedAfterEdit();

    const std::vector<const UISchemaOption*> groupedOptions = OptionsForGroup(control, groups[groupIndex]);
    int currentIndex = 0;
    for (int index = 0; index < static_cast<int>(groupedOptions.size()); ++index) {
        if (groupedOptions[index]->id == selectedId) {
            currentIndex = index;
            break;
        }
    }

    std::vector<const char*> labels;
    labels.reserve(groupedOptions.size());
    for (const UISchemaOption* option : groupedOptions) {
        labels.push_back(option->label.c_str());
    }

    bool valueChanged = false;
    if (!labels.empty() && ImGui::Combo(control.label.c_str(), &currentIndex, labels.data(), static_cast<int>(labels.size()))) {
        if (currentIndex >= 0 && currentIndex < static_cast<int>(groupedOptions.size())) {
            valueChanged = ctx.SetEnumId(binding.path, groupedOptions[currentIndex]->id);
            MarkDirtyIfChanged(valueChanged, ioDirty);
        }
    }
    const bool valueInteracted = valueChanged || ImGui::IsItemActivated() || ImGui::IsItemActive() || ImGui::IsItemDeactivatedAfterEdit();
    if ((groupChanged || valueChanged || groupInteracted || valueInteracted) && ioInteracted) {
        *ioInteracted = true;
    }
    return groupChanged || valueChanged;
}

bool RenderEnumComboControl(
    const UISchemaControl& control,
    BindingContext& ctx,
    const UISchemaBinding& binding,
    bool* ioDirty,
    bool* ioInteracted) {
    const std::string currentId = ctx.GetEnumId(binding.path);
    if (HasGroupedOptions(control)) {
        RenderGroupedEnumComboControl(control, ctx, binding, currentId, ioDirty, ioInteracted);
        return true;
    }

    int currentIndex = 0;
    for (int index = 0; index < static_cast<int>(control.options.size()); ++index) {
        if (control.options[index].id == currentId) {
            currentIndex = index;
            break;
        }
    }

    std::vector<const char*> labels;
    labels.reserve(control.options.size());
    for (const auto& option : control.options) {
        labels.push_back(option.label.c_str());
    }

    bool changed = false;
    if (!labels.empty() && ImGui::Combo(control.label.c_str(), &currentIndex, labels.data(), static_cast<int>(labels.size()))) {
        if (currentIndex >= 0 && currentIndex < static_cast<int>(control.options.size())) {
            changed = ctx.SetEnumId(binding.path, control.options[currentIndex].id);
        }
    }
    MarkDirtyIfChanged(changed, ioDirty);
    MarkCurrentItemInteraction(changed, ioInteracted);
    return changed;
}

bool RenderComboControl(
    const UISchemaControl& control,
    BindingContext& ctx,
    const UISchemaBinding& binding,
    bool* ioDirty,
    bool* ioInteracted) {
    if (control.value_type == "int") {
        return RenderIntComboControl(control, ctx, binding, ioDirty, ioInteracted);
    }
    return RenderEnumComboControl(control, ctx, binding, ioDirty, ioInteracted);
}

} // namespace

bool RenderControlFromSchema(const UISchemaControl& c, BindingContext& ctx, bool* ioDirty, bool* ioRenderOnce, bool* ioInteracted) {
    if (c.has_visible_if) {
        if (!ctx.EvalVisibleIf(c.visible_if)) return false;
    }

    ImGui::PushID(c.id.c_str());

    RenderControlHelp(c);

    if (!c.has_binding) {
        RenderDiagnosticLabel(c, "UNBOUND");
        ImGui::PopID();
        return false;
    }

    const auto& b = c.binding;

    bool result = false;
    if (c.type == "button") {
        result = RenderActionControl(c, b, ioRenderOnce, ioInteracted);
    } else if (b.kind != "param") {
        result = RenderDiagnosticLabel(c, "bad param binding");
    } else if (c.type == "checkbox") {
        result = RenderCheckboxControl(c, ctx, b, ioDirty, ioInteracted);
    } else if (c.type == "slider_int" || c.type == "drag_int") {
        result = RenderIntControl(c, ctx, b, ioDirty, ioInteracted);
    } else if (c.type == "slider_float" || c.type == "drag_float") {
        result = RenderFloatControl(c, ctx, b, ioDirty, ioInteracted);
    } else if (c.type == "slider_double" || c.type == "drag_double") {
        result = RenderDoubleControl(c, ctx, b, ioDirty, ioInteracted);
    } else if (c.type == "combo") {
        result = RenderComboControl(c, ctx, b, ioDirty, ioInteracted);
    } else {
        ImGui::TextDisabled("%s (unsupported control type: %s)", c.label.c_str(), c.type.c_str());
        result = false;
    }

    ImGui::PopID();
    return result;
}
