#include "schema_binding.h"
#include "explaino_seed.h"
#include "imgui.h"

#include <cmath>
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
    if (params && path == "fractal.params.coloring_mode") {
        switch (params->coloring_mode) {
        case ColoringMode::root_basin: return "root_basin";
        case ColoringMode::iteration_count: return "iteration_count";
        case ColoringMode::smooth_escape: return "smooth_escape";
        case ColoringMode::joy_basins: return "joy_basins";
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
    if (params && path == "fractal.params.coloring_mode") {
        if (id == "root_basin") params->coloring_mode = ColoringMode::root_basin;
        else if (id == "iteration_count") params->coloring_mode = ColoringMode::iteration_count;
        else if (id == "smooth_escape") params->coloring_mode = ColoringMode::smooth_escape;
        else if (id == "joy_basins") params->coloring_mode = ColoringMode::joy_basins;
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
    if (!view || !params) return false;
    if (path == "fractal.view.center.x") { *outPtr = &view->center.x; return true; }
    if (path == "fractal.view.center.y") { *outPtr = &view->center.y; return true; }
    if (path == "fractal.view.zoom") { *outPtr = &view->zoom; return true; }
    if (path == "fractal.view.rotation") { *outPtr = &view->rotation_degrees; return true; }
    if (path == "fractal.view.dive_speed") { *outPtr = &view->dive_speed; return true; }
    if (path == "fractal.view.explaino_phase") { *outPtr = &view->explaino_phase; return true; }
    if (path == "fractal.view.explaino_seed_drift") { *outPtr = &view->explaino_seed_drift; return true; }
    if (path == "fractal.view.explaino_seed_rate") { *outPtr = &view->explaino_seed_rate; return true; }
    if (path == "fractal.view.explaino_phase_strength") { *outPtr = &view->explaino_phase_strength; return true; }
    if (path == "fractal.params.epsilon") { *outPtr = &params->epsilon; return true; }
    if (path == "fractal.params.nova_alpha") { *outPtr = &params->nova_alpha; return true; }
    if (path == "fractal.params.phoenix_p_real") { *outPtr = &params->phoenix_p_real; return true; }
    if (path == "fractal.params.phoenix_p_imag") { *outPtr = &params->phoenix_p_imag; return true; }
    if (path == "fractal.params.exposure") { *outPtr = &params->exposure; return true; }
    if (path == "fractal.params.color_saturation") { *outPtr = &params->color_saturation; return true; }
    if (path == "fractal.params.color_contrast") { *outPtr = &params->color_contrast; return true; }
    if (path == "fractal.params.color_tint_r") { *outPtr = &params->color_tint_r; return true; }
    if (path == "fractal.params.color_tint_g") { *outPtr = &params->color_tint_g; return true; }
    if (path == "fractal.params.color_tint_b") { *outPtr = &params->color_tint_b; return true; }
    if (path == "fractal.params.explaino_warp_strength") { *outPtr = &params->explaino_warp_strength; return true; }
    if (path == "fractal.params.explaino_root_spread") { *outPtr = &params->explaino_root_spread; return true; }
    if (path == "fractal.params.explaino_damping") { *outPtr = &params->explaino_damping; return true; }
    if (path == "fractal.params.poly_coeffs.0") { *outPtr = &params->poly_coeffs[0]; return true; }
    if (path == "fractal.params.poly_coeffs.1") { *outPtr = &params->poly_coeffs[1]; return true; }
    if (path == "fractal.params.poly_coeffs.2") { *outPtr = &params->poly_coeffs[2]; return true; }
    if (path == "fractal.params.poly_coeffs.3") { *outPtr = &params->poly_coeffs[3]; return true; }
    if (path == "fractal.params.poly_coeffs.4") { *outPtr = &params->poly_coeffs[4]; return true; }
    return false;
}

bool BindingContext::BindDouble(const std::string& path, double** outPtr) {
    if (!params) return false;
    if (path == "fractal.params.explaino_seed") { *outPtr = &params->explaino_seed; return true; }
    return false;
}

bool BindingContext::BindInt(const std::string& path, int** outPtr) {
    if (!params || !render) return false;
    if (path == "fractal.params.max_iter") { *outPtr = &params->max_iter; return true; }
    if (path == "fractal.params.multibrot_power") { *outPtr = &params->multibrot_power; return true; }
    if (path == "fractal.render.resolution.x") { *outPtr = &render->resolution.x; return true; }
    if (path == "fractal.render.resolution.y") { *outPtr = &render->resolution.y; return true; }
    if (path == "fractal.render.block_size") { *outPtr = &render->block_size; return true; }
    if (path == "fractal.render.device_id") { *outPtr = &render->device_id; return true; }
    if (lens && path == "fractal.lens.downsample") { *outPtr = &lens->downsample; return true; }
    return false;
}

bool BindingContext::BindBool(const std::string& path, bool** outPtr) {
    if (!view || !render) return false;
    if (path == "fractal.view.auto_refresh") { *outPtr = &view->auto_refresh; return true; }
    if (path == "fractal.view.auto_dive") { *outPtr = &view->auto_dive; return true; }
    if (path == "fractal.view.explaino_alive") { *outPtr = &view->explaino_alive; return true; }
    if (path == "fractal.view.explaino_seed_tween") { *outPtr = &view->explaino_seed_tween; return true; }
    if (path == "fractal.view.auto_increment_seed") { *outPtr = &view->auto_increment_seed; return true; }
    if (path == "fractal.render.benchmark") { *outPtr = &render->benchmark; return true; }
    if (lens && path == "fractal.lens.enabled") { *outPtr = &lens->enabled; return true; }
    return false;
}

bool ApplySchemaDefaultForControl(const UISchemaControl& c, BindingContext& ctx, bool* ioDirty) {
    if (!c.has_binding || c.binding.kind != "param" || !c.has_default) return false;

    const std::string& path = c.binding.path;

    if (c.value_type == "bool") {
        bool* ptr = nullptr;
        if (!ctx.BindBool(path, &ptr) || !ptr) return false;
        bool newV = *ptr;
        if (c.def.is_bool()) newV = c.def.as_bool();
        else if (c.def.is_number()) newV = (c.def.as_number() != 0.0);
        else if (c.def.is_string()) newV = (c.def.as_string() == "true" || c.def.as_string() == "1");
        else return false;
        if (*ptr != newV) {
            *ptr = newV;
            if (ioDirty) *ioDirty = true;
            return true;
        }
        return false;
    }

    if (c.value_type == "int") {
        int* ptr = nullptr;
        if (!ctx.BindInt(path, &ptr) || !ptr) return false;
        int newV = *ptr;
        if (c.def.is_number()) newV = (int)c.def.as_number();
        else if (c.def.is_string()) {
            try { newV = std::stoi(c.def.as_string()); } catch (...) { return false; }
        } else return false;
        if (*ptr != newV) {
            *ptr = newV;
            if (ioDirty) *ioDirty = true;
            return true;
        }
        return false;
    }

    if (c.value_type == "float") {
        float* ptr = nullptr;
        if (!ctx.BindFloat(path, &ptr) || !ptr) return false;
        float newV = *ptr;
        if (c.def.is_number()) newV = (float)c.def.as_number();
        else if (c.def.is_string()) {
            try { newV = (float)std::stod(c.def.as_string()); } catch (...) { return false; }
        } else return false;
        if (*ptr != newV) {
            *ptr = newV;
            if (ioDirty) *ioDirty = true;
            return true;
        }
        return false;
    }

    if (c.value_type == "double") {
        double* ptr = nullptr;
        if (!ctx.BindDouble(path, &ptr) || !ptr) return false;
        double newV = *ptr;
        if (c.def.is_number()) newV = c.def.as_number();
        else if (c.def.is_string()) {
            try { newV = std::stod(c.def.as_string()); } catch (...) { return false; }
        } else return false;
        if (path == "fractal.params.explaino_seed" && ctx.view && ctx.params) {
            if (ExplainoSeedCombined(*ctx.view, *ctx.params) != newV) {
                ExplainoSeedSetCombined(*ctx.view, *ctx.params, newV);
                if (ioDirty) *ioDirty = true;
                return true;
            }
            return false;
        }
        if (*ptr != newV) {
            *ptr = newV;
            if (ioDirty) *ioDirty = true;
            return true;
        }
        return false;
    }

    if (c.value_type == "enum") {
        if (!c.def.is_string()) return false;
        std::string cur = ctx.GetEnumId(path);
        const std::string& want = c.def.as_string();
        if (cur != want) {
            bool ok = ctx.SetEnumId(path, want);
            if (ok && ioDirty) *ioDirty = true;
            return ok;
        }
        return false;
    }

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
                if (!(b.path == "fractal.actions.render_once" || b.path == "fractal.actions.reset_view" || b.path == "fractal.actions.reset_all" || b.path == "fractal.actions.load_state" || b.path == "fractal.actions.capture_finding" || b.path == "fractal.actions.capture_diagnostic" || b.path == "fractal.actions.next_seed" || b.path == "fractal.actions.prev_seed")) {
                    if (outError) *outError = "Unknown action binding path: " + b.path + " (control: " + c.id + ")";
                    return false;
                }
                continue;
            }

            if (b.kind != "param") {
                if (outError) *outError = "Unknown binding kind: " + b.kind + " (control: " + c.id + ")";
                return false;
            }

            if (c.value_type == "bool") {
                bool* ptr = nullptr;
                if (!ctx.BindBool(b.path, &ptr) || !ptr) {
                    if (outError) *outError = "Bind failed for bool path: " + b.path + " (control: " + c.id + ")";
                    return false;
                }
            } else if (c.value_type == "int") {
                int* ptr = nullptr;
                if (!ctx.BindInt(b.path, &ptr) || !ptr) {
                    if (outError) *outError = "Bind failed for int path: " + b.path + " (control: " + c.id + ")";
                    return false;
                }
            } else if (c.value_type == "float") {
                float* ptr = nullptr;
                if (!ctx.BindFloat(b.path, &ptr) || !ptr) {
                    if (outError) *outError = "Bind failed for float path: " + b.path + " (control: " + c.id + ")";
                    return false;
                }
            } else if (c.value_type == "double") {
                double* ptr = nullptr;
                if (!ctx.BindDouble(b.path, &ptr) || !ptr) {
                    if (outError) *outError = "Bind failed for double path: " + b.path + " (control: " + c.id + ")";
                    return false;
                }
            } else if (c.value_type == "enum") {
                if (!(b.path == "fractal.view.fractal_type" || b.path == "fractal.view.camera_behavior" || b.path == "fractal.params.poly_kind" ||
                      b.path == "fractal.params.coloring_mode")) {
                    if (outError) *outError = "Unknown enum binding path: " + b.path + " (control: " + c.id + ")";
                    return false;
                }
            }
        }
    }
    return true;
}

bool RenderControlFromSchema(const UISchemaControl& c, BindingContext& ctx, bool* ioDirty, bool* ioRenderOnce) {
    if (c.has_visible_if) {
        if (!ctx.EvalVisibleIf(c.visible_if)) return false;
    }

    ImGui::PushID(c.id.c_str());

    if (c.has_help) {
        ImGui::TextDisabled("?");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(300.0f);
            ImGui::TextUnformatted(c.help.c_str());
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
        ImGui::SameLine();
    }

    if (!c.has_binding) {
        ImGui::TextDisabled("%s (UNBOUND)", c.label.c_str());
        ImGui::PopID();
        return false;
    }

    const auto& b = c.binding;
    if (c.type == "button") {
        if (b.kind != "action") {
            ImGui::TextDisabled("%s (bad action binding)", c.label.c_str());
            ImGui::PopID();
            return false;
        }
        if (ImGui::Button(c.label.c_str())) {
            if (b.path == "fractal.actions.render_once") {
                if (ioRenderOnce) *ioRenderOnce = true;
            }
        }
        ImGui::PopID();
        return true;
    }

    if (b.kind != "param") {
        ImGui::TextDisabled("%s (bad param binding)", c.label.c_str());
        ImGui::PopID();
        return false;
    }

    if (c.type == "checkbox") {
        bool* ptr = nullptr;
        if (!ctx.BindBool(b.path, &ptr) || !ptr) {
            ImGui::TextDisabled("%s (bind failed)", c.label.c_str());
            ImGui::PopID();
            return false;
        }
        bool changed = ImGui::Checkbox(c.label.c_str(), ptr);
        if (changed && ioDirty) *ioDirty = true;
        ImGui::PopID();
        return changed;
    }

    if (c.type == "slider_int" || c.type == "drag_int") {
        int* ptr = nullptr;
        if (!ctx.BindInt(b.path, &ptr) || !ptr) {
            ImGui::TextDisabled("%s (bind failed)", c.label.c_str());
            ImGui::PopID();
            return false;
        }

        int minV = c.has_min ? (int)c.min : 0;
        int maxV = c.has_max ? (int)c.max : 100;

        bool changed = false;
        if (c.type == "slider_int") {
            changed = ImGui::SliderInt(c.label.c_str(), ptr, minV, maxV);
        } else {
            float speed = c.has_step ? (float)c.step : 1.0f;
            changed = ImGui::DragInt(c.label.c_str(), ptr, speed, minV, maxV);
        }
        if (changed && ioDirty) *ioDirty = true;
        ImGui::PopID();
        return changed;
    }

    if (c.type == "slider_float" || c.type == "drag_float") {
        float* ptr = nullptr;
        if (!ctx.BindFloat(b.path, &ptr) || !ptr) {
            ImGui::TextDisabled("%s (bind failed)", c.label.c_str());
            ImGui::PopID();
            return false;
        }

        float minV = c.has_min ? (float)c.min : 0.0f;
        float maxV = c.has_max ? (float)c.max : 1.0f;
        float speed = c.has_step ? (float)c.step : 0.01f;

        bool changed = false;
        ImGuiSliderFlags flags = c.logarithmic ? ImGuiSliderFlags_Logarithmic : 0;
        if (c.type == "slider_float") {
            changed = ImGui::SliderFloat(c.label.c_str(), ptr, minV, maxV, "%.5f", flags);
        } else {
            changed = ImGui::DragFloat(c.label.c_str(), ptr, speed, minV, maxV, "%.3f", flags);
        }
        if (changed && ioDirty) *ioDirty = true;
        ImGui::PopID();
        return changed;
    }

    if (c.type == "slider_double" || c.type == "drag_double") {
        double* ptr = nullptr;
        if (!ctx.BindDouble(b.path, &ptr) || !ptr) {
            ImGui::TextDisabled("%s (bind failed)", c.label.c_str());
            ImGui::PopID();
            return false;
        }

        double minV = c.has_min ? c.min : 0.0;
        double maxV = c.has_max ? c.max : 1.0;
        double speedD = c.has_step ? c.step : 0.001;
        const char* valueFormat = "%.6f";

        if (b.path == "fractal.params.explaino_seed" && ctx.view && ctx.params) {
            double displayed = ExplainoSeedCombined(*ctx.view, *ctx.params);
            bool changed = false;
            if (c.type == "slider_double") {
                changed = ImGui::SliderScalar(c.label.c_str(), ImGuiDataType_Double, &displayed, &minV, &maxV, valueFormat);
            } else {
                changed = ImGui::DragScalar(c.label.c_str(), ImGuiDataType_Double, &displayed, static_cast<float>(speedD), &minV, &maxV, valueFormat);
            }
            ImGui::SameLine();
            bool typedChanged = ImGui::InputDouble("##val", &displayed, 0.0, 0.0, valueFormat);
            if (typedChanged) changed = true;
            if (changed) {
                ExplainoSeedSetCombined(*ctx.view, *ctx.params, displayed);
                if (ioDirty) *ioDirty = true;
            }
            ImGui::PopID();
            return changed;
        }

        bool changed = false;
        if (c.type == "slider_double") {
            changed = ImGui::SliderScalar(c.label.c_str(), ImGuiDataType_Double, ptr, &minV, &maxV, valueFormat);
        } else {
            changed = ImGui::DragScalar(c.label.c_str(), ImGuiDataType_Double, ptr, static_cast<float>(speedD), &minV, &maxV, valueFormat);
        }
        ImGui::SameLine();
        bool typedChanged = ImGui::InputDouble("##val", ptr, 0.0, 0.0, valueFormat);
        if (typedChanged) changed = true;
        if (changed && ioDirty) *ioDirty = true;
        ImGui::PopID();
        return changed;
    }

    if (c.type == "combo") {
        // Int-valued combos: option ids are integer strings
        if (c.value_type == "int") {
            int* ptr = nullptr;
            if (!ctx.BindInt(b.path, &ptr) || !ptr) {
                ImGui::TextDisabled("%s (bind failed)", c.label.c_str());
                ImGui::PopID();
                return false;
            }

            int curIndex = 0;
            for (int i = 0; i < (int)c.options.size(); i++) {
                try {
                    int optV = std::stoi(c.options[i].id);
                    if (optV == *ptr) { curIndex = i; break; }
                } catch (...) {}
            }

            std::vector<const char*> labels;
            labels.reserve(c.options.size());
            for (const auto& o : c.options) labels.push_back(o.label.c_str());

            bool changed = false;
            if (!labels.empty() && ImGui::Combo(c.label.c_str(), &curIndex, labels.data(), (int)labels.size())) {
                if (curIndex >= 0 && curIndex < (int)c.options.size()) {
                    try {
                        int newV = std::stoi(c.options[curIndex].id);
                        if (newV != *ptr) {
                            *ptr = newV;
                            changed = true;
                            if (ioDirty) *ioDirty = true;
                        }
                    } catch (...) {}
                }
            }
            ImGui::PopID();
            return changed;
        }

        // Enum combos
        std::string cur = ctx.GetEnumId(b.path);
        int curIndex = 0;
        for (int i = 0; i < (int)c.options.size(); i++) {
            if (c.options[i].id == cur) { curIndex = i; break; }
        }

        std::vector<const char*> labels;
        labels.reserve(c.options.size());
        for (const auto& o : c.options) labels.push_back(o.label.c_str());

        bool changed = false;
        if (!labels.empty() && ImGui::Combo(c.label.c_str(), &curIndex, labels.data(), (int)labels.size())) {
            if (curIndex >= 0 && curIndex < (int)c.options.size()) {
                changed = ctx.SetEnumId(b.path, c.options[curIndex].id);
                if (changed && ioDirty) *ioDirty = true;
            }
        }
        ImGui::PopID();
        return changed;
    }

    ImGui::TextDisabled("%s (unsupported control type: %s)", c.label.c_str(), c.type.c_str());
    ImGui::PopID();
    return false;
}
