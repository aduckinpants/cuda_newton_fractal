#include "fractal_parameter_surface_descriptor.h"

#include "enum_id_utils.h"
#include "fractal_family_rules.h"
#include "schema_binding.h"

#include <cstddef>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace {

bool StartsWith(const std::string& value, const char* prefix) {
    return value.rfind(prefix, 0) == 0;
}

bool IsGlobalFixedFormulaBindingPath(const std::string& path) {
    return path == "fractal.params.max_iter" ||
        path == "fractal.params.coloring_mode" ||
        path == "fractal.params.color_grading" ||
        path == "fractal.params.exposure" ||
        StartsWith(path, "fractal.params.color_") ||
        StartsWith(path, "fractal.view.") ||
        StartsWith(path, "fractal.render.") ||
        StartsWith(path, "fractal.lens.");
}

bool IsFamilySurfaceBindingPath(const std::string& path) {
    if (path == "fractal.view.fractal_type") {
        return false;
    }
    return !IsGlobalFixedFormulaBindingPath(path);
}

BindingContext MakeDescriptorBindingContext(
    ViewState& view,
    KernelParams& params,
    RenderSettings& render,
    LensSettings& lens) {
    BindingContext ctx;
    ctx.view = &view;
    ctx.params = &params;
    ctx.render = &render;
    ctx.lens = &lens;
    return ctx;
}

bool IsPolynomialEditorFractalType(FractalType fractalType) {
    return fractalType == FractalType::newton ||
        fractalType == FractalType::nova ||
        fractalType == FractalType::halley;
}

bool IsDescriptorExplainoRootEditorFractalType(FractalType fractalType) {
    switch (fractalType) {
    case FractalType::explaino_julia:
    case FractalType::explaino_lambda:
    case FractalType::explaino_rational_escape:
    case FractalType::explaino_collatz_direct:
    case FractalType::explaino_counterfactual_pair:
    case FractalType::explaino_projection_and_flow:
        return false;
    default:
        return IsExplainoFamily(fractalType);
    }
}

enum class DescriptorSurfaceKind {
    default_state,
    poly_custom,
    explaino_roots_custom,
    explaino_julia_custom,
};

struct DescriptorSurfaceSpec {
    const char* id;
    DescriptorSurfaceKind kind;
    bool default_visible;
};

constexpr DescriptorSurfaceSpec kDescriptorSurfaceSpecs[] = {
    {"default", DescriptorSurfaceKind::default_state, true},
    {"poly_custom", DescriptorSurfaceKind::poly_custom, false},
    {"explaino_roots_custom", DescriptorSurfaceKind::explaino_roots_custom, false},
    {"explaino_julia_custom", DescriptorSurfaceKind::explaino_julia_custom, false},
};

bool DescriptorSurfaceApplies(FractalType fractalType, DescriptorSurfaceKind kind) {
    switch (kind) {
    case DescriptorSurfaceKind::default_state:
        return true;
    case DescriptorSurfaceKind::poly_custom:
        return IsPolynomialEditorFractalType(fractalType);
    case DescriptorSurfaceKind::explaino_roots_custom:
        return IsDescriptorExplainoRootEditorFractalType(fractalType);
    case DescriptorSurfaceKind::explaino_julia_custom:
        return fractalType == FractalType::explaino_julia;
    }
    return false;
}

BindingContext MakeDescriptorBindingContextForSurface(
    FractalType fractalType,
    DescriptorSurfaceKind surfaceKind,
    ViewState& view,
    KernelParams& params,
    RenderSettings& render,
    LensSettings& lens) {
    view = ViewState{};
    params = KernelParams{};
    render = RenderSettings{};
    lens = LensSettings{};
    view.fractal_type = fractalType;

    if (surfaceKind == DescriptorSurfaceKind::poly_custom) {
        params.poly_kind = PolyKind::custom;
    } else if (surfaceKind == DescriptorSurfaceKind::explaino_roots_custom) {
        params.explaino_root_authority = ExplainoRootAuthority::custom;
        params.explaino_root_count = 4;
        params.poly_kind = PolyKind::custom;
    } else if (surfaceKind == DescriptorSurfaceKind::explaino_julia_custom) {
        params.explaino_julia_constant_mode = ExplainoJuliaConstantMode::custom;
    }

    return MakeDescriptorBindingContext(view, params, render, lens);
}

bool ControlVisibleForContext(const UISchemaControl& control, const BindingContext& ctx) {
    if (!control.has_visible_if) {
        return true;
    }
    return ctx.EvalVisibleIf(control.visible_if);
}

bool OptionVisibleForContext(const UISchemaOption& option, const BindingContext& ctx) {
    if (!option.has_visible_if) {
        return true;
    }
    return ctx.EvalVisibleIf(option.visible_if);
}

bool BindingResolvesAsKind(
    const UISchemaControl& control,
    BindingContext& ctx,
    std::string* outKind) {
    if (outKind) {
        outKind->clear();
    }
    if (!control.has_binding || control.binding.kind != "param") {
        return false;
    }

    const std::string& path = control.binding.path;
    if (control.value_type == "float") {
        float* value = nullptr;
        const bool resolves = ctx.BindFloat(path, &value) && value;
        if (resolves && outKind) *outKind = "float";
        return resolves;
    }
    if (control.value_type == "double") {
        double* value = nullptr;
        const bool resolves = ctx.BindDouble(path, &value) && value;
        if (resolves && outKind) *outKind = "double";
        return resolves;
    }
    if (control.value_type == "int") {
        int value = 0;
        const bool resolves = ctx.GetIntValue(path, value);
        if (resolves && outKind) *outKind = "int";
        return resolves;
    }
    if (control.value_type == "bool") {
        bool* value = nullptr;
        const bool resolves = ctx.BindBool(path, &value) && value;
        if (resolves && outKind) *outKind = "bool";
        return resolves;
    }
    if (control.value_type == "enum") {
        const bool resolves = !ctx.GetEnumId(path).empty();
        if (resolves && outKind) *outKind = "enum";
        return resolves;
    }
    return false;
}

const UISchemaControl* FindControlById(const UISchema& schema, const std::string& id) {
    for (const UISchemaPanel& panel : schema.panels) {
        for (const UISchemaControl& control : panel.controls) {
            if (control.id == id) {
                return &control;
            }
        }
    }
    return nullptr;
}

bool FloatBindingResolves(BindingContext& ctx, const std::string& path) {
    float* value = nullptr;
    return ctx.BindFloat(path, &value) && value;
}

std::string AnimationOptionIdForControl(const std::string& controlId) {
    if (controlId == "explaino_seed") return "seed";
    if (controlId == "explaino_damping") return "damping";
    if (controlId == "explaino_warp_strength") return "warp_strength";
    if (controlId == "explaino_root_spread") return "root_spread";
    if (controlId == "explaino_mix") return "mix";
    return controlId;
}

bool ControlIsAnimatableForContext(
    const UISchemaControl& control,
    const UISchemaControl* animTarget,
    BindingContext& ctx) {
    if (!animTarget || control.id.empty()) {
        return false;
    }
    const std::string optionId = AnimationOptionIdForControl(control.id);
    for (const UISchemaOption& option : animTarget->options) {
        if (option.id != optionId || !OptionVisibleForContext(option, ctx)) {
            continue;
        }
        if (option.id == "seed") {
            return true;
        }
        return FloatBindingResolves(ctx, "fractal.params." + control.id) ||
            FloatBindingResolves(ctx, "fractal.view." + control.id) ||
            FloatBindingResolves(ctx, "fractal.params." + option.id) ||
            FloatBindingResolves(ctx, "fractal.view." + option.id);
    }
    return false;
}

std::string StateIoKeyForBindingPath(const std::string& path) {
    constexpr const char* prefix = "fractal.params.";
    if (!StartsWith(path, prefix)) {
        return {};
    }
    return path.substr(std::char_traits<char>::length(prefix));
}

bool HasValidationRange(const UISchemaControl& control) {
    return control.has_min || control.has_max || control.has_ui_min || control.has_ui_max;
}

void WriteJsonString(std::ostream& out, const std::string& value) {
    out << '"';
    for (unsigned char ch : value) {
        switch (ch) {
        case '"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
        case '\b': out << "\\b"; break;
        case '\f': out << "\\f"; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default:
            if (ch < 0x20) {
                out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(ch)
                    << std::dec << std::setfill(' ');
            } else {
                out << static_cast<char>(ch);
            }
            break;
        }
    }
    out << '"';
}

std::string JsonScalarAsDescriptorValue(const json_min::Value* value) {
    if (!value) {
        return "null";
    }
    if (value->is_string()) {
        return value->as_string();
    }
    if (value->is_number()) {
        std::ostringstream ss;
        ss << std::setprecision(17) << value->as_number();
        return ss.str();
    }
    if (value->is_bool()) {
        return value->as_bool() ? "true" : "false";
    }
    return "null";
}

double NumericDefaultValue(const UISchemaControl& control) {
    if (!control.has_default) {
        return 0.0;
    }
    if (control.def.is_number()) {
        return control.def.as_number();
    }
    if (control.def.is_string()) {
        try {
            return std::stod(control.def.as_string());
        } catch (...) {
            return 0.0;
        }
    }
    if (control.def.is_bool()) {
        return control.def.as_bool() ? 1.0 : 0.0;
    }
    return 0.0;
}

std::string CandidateValueForControl(const UISchemaControl& control) {
    if (control.value_type == "bool") {
        const bool current = control.has_default && control.def.is_bool() ? control.def.as_bool() : false;
        return current ? "false" : "true";
    }
    if (control.value_type == "enum") {
        const std::string defaultId = JsonScalarAsDescriptorValue(control.has_default ? &control.def : nullptr);
        for (const UISchemaOption& option : control.options) {
            if (option.id != defaultId) {
                return option.id;
            }
        }
        return defaultId;
    }

    const double defaultNumber = NumericDefaultValue(control);
    const double step = control.has_step ? control.step : 1.0;
    double candidate = defaultNumber + step;
    if (control.has_max && candidate > control.max) {
        candidate = defaultNumber - step;
    } else if (!control.has_max && control.has_ui_max && candidate > control.ui_max) {
        candidate = defaultNumber - step;
    }
    if (control.has_min && candidate < control.min) {
        candidate = defaultNumber;
    }
    std::ostringstream ss;
    ss << std::setprecision(17) << candidate;
    return ss.str();
}

bool ControlAlreadyDescribed(const std::vector<std::string>& describedControlIds, const std::string& controlId) {
    for (const std::string& described : describedControlIds) {
        if (described == controlId) {
            return true;
        }
    }
    return false;
}

void WriteControlDescriptor(
    std::ostream& out,
    const char* laneId,
    const UISchemaControl& control,
    const std::string& runtimeBindingKind,
    bool bindingResolves,
    bool animatable,
    const char* visibilitySurfaceId,
    bool defaultVisible) {
    out << "{";
    out << "\"control_id\": ";
    WriteJsonString(out, control.id);
    out << ", \"owner_lane\": ";
    WriteJsonString(out, laneId ? laneId : "");
    out << ", \"binding_path\": ";
    WriteJsonString(out, control.binding.path);
    out << ", \"control_type\": ";
    WriteJsonString(out, control.type);
    out << ", \"value_type\": ";
    WriteJsonString(out, control.value_type);
    out << ", \"default_value\": ";
    WriteJsonString(out, JsonScalarAsDescriptorValue(control.has_default ? &control.def : nullptr));
    out << ", \"candidate_value\": ";
    WriteJsonString(out, CandidateValueForControl(control));
    out << ", \"runtime_binding_kind\": ";
    WriteJsonString(out, runtimeBindingKind);
    out << ", \"binding_resolves\": " << (bindingResolves ? "true" : "false");
    out << ", \"state_io_key\": ";
    WriteJsonString(out, StateIoKeyForBindingPath(control.binding.path));
    out << ", \"has_validation_range\": " << (HasValidationRange(control) ? "true" : "false");
    out << ", \"animatable\": " << (animatable ? "true" : "false");
    out << ", \"visibility_surface_id\": ";
    WriteJsonString(out, visibilitySurfaceId ? visibilitySurfaceId : "");
    out << ", \"default_visible\": " << (defaultVisible ? "true" : "false");
    out << "}";
}

} // namespace

std::string SerializeFractalParameterSurfaceDescriptorJson(const UISchema& schema) {
    const UISchemaControl* animTarget = FindControlById(schema, "param_anim_target");

    std::ostringstream out;
    int fractalCount = 0;
    int surfaceCount = 0;
    int visibleFamilyControlCells = 0;
    int defaultVisibleFamilyControlCells = 0;

    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"lanes\": [\n";

    bool firstLane = true;
    for (const auto& lane : enum_id_utils::kFractalTypeIds) {
        if (!firstLane) {
            out << ",\n";
        }
        firstLane = false;
        ++fractalCount;

        out << "    { \"fractal_id\": ";
        WriteJsonString(out, lane.id);
        out << ", \"controls\": [";

        bool firstControl = true;
        std::vector<std::string> describedControlIds;
        for (const DescriptorSurfaceSpec& surface : kDescriptorSurfaceSpecs) {
            if (!DescriptorSurfaceApplies(lane.value, surface.kind)) {
                continue;
            }
            ++surfaceCount;

            ViewState view{};
            KernelParams params{};
            RenderSettings render{};
            LensSettings lens{};
            BindingContext ctx = MakeDescriptorBindingContextForSurface(
                lane.value,
                surface.kind,
                view,
                params,
                render,
                lens);

            for (const UISchemaPanel& panel : schema.panels) {
                for (const UISchemaControl& control : panel.controls) {
                    if (!control.has_binding ||
                        control.binding.kind != "param" ||
                        !IsFamilySurfaceBindingPath(control.binding.path) ||
                        ControlAlreadyDescribed(describedControlIds, control.id) ||
                        !ControlVisibleForContext(control, ctx)) {
                        continue;
                    }

                    std::string runtimeBindingKind;
                    const bool bindingResolves = BindingResolvesAsKind(control, ctx, &runtimeBindingKind);
                    const bool animatable = ControlIsAnimatableForContext(control, animTarget, ctx);
                    describedControlIds.push_back(control.id);
                    ++visibleFamilyControlCells;
                    if (surface.default_visible) {
                        ++defaultVisibleFamilyControlCells;
                    }

                    if (!firstControl) {
                        out << ",";
                    }
                    firstControl = false;
                    out << "\n        ";
                    WriteControlDescriptor(
                        out,
                        lane.id,
                        control,
                        runtimeBindingKind,
                        bindingResolves,
                        animatable,
                        surface.id,
                        surface.default_visible);
                }
            }
        }

        if (!firstControl) {
            out << "\n      ";
        }
        out << "] }";
    }

    out << "\n  ],\n";
    out << "  \"fractal_count\": " << fractalCount << ",\n";
    out << "  \"surface_count\": " << surfaceCount << ",\n";
    out << "  \"default_visible_family_control_cells\": " << defaultVisibleFamilyControlCells << ",\n";
    out << "  \"visible_family_control_cells\": " << visibleFamilyControlCells << "\n";
    out << "}\n";
    return out.str();
}
