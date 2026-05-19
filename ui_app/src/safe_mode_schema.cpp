#include "safe_mode_schema.h"

#include "fractal_types.h"

namespace {

struct SafeModeFractalTypeOptionDef {
    const char* id;
    const char* label;
    const char* group;
};

constexpr SafeModeFractalTypeOptionDef kSafeModeFractalTypeOptionDefs[] = {
    {"explaino", "Explaino", "Common"},
    {"nova", "Nova", "Common"},
    {"mandelbrot", "Mandelbrot", "Common"},
    {"julia", "Julia", "Common"},
    {"burning_ship", "Burning Ship", "Common"},
    {"phoenix", "Phoenix", "Common"},
    {"newton", "Newton", "Root-Finding"},
    {"halley", "Halley", "Root-Finding"},
    {"projection_and_flow", "Projection and Flow", "Root-Finding"},
    {"counterfactual_pair", "Counterfactual Pair", "Root-Finding"},
    {"multibrot", "Multibrot", "Escape-Time"},
    {"spider", "Spider", "Escape-Time"},
    {"celtic_mandelbrot", "Celtic Mandelbrot", "Escape-Time"},
    {"perpendicular_burning_ship", "Perpendicular Burning Ship", "Escape-Time"},
    {"multicorn", "Multicorn", "Escape-Time"},
    {"collatz", "Collatz", "Escape-Time"},
    {"mcmullen", "McMullen", "Escape-Time"},
    {"lambda", "Lambda", "Escape-Time"},
    {"magnet", "Magnet Type I", "Escape-Time"},
    {"explaino_all", "Explaino-all", "Explaino"},
    {"explaino_y", "Explaino Y", "Explaino"},
    {"explaino_fp", "Explaino FP", "Explaino"},
    {"explaino_nova", "Explaino Nova", "Explaino"},
    {"explaino_halley", "Explaino Halley", "Explaino"},
    {"explaino_dual", "Explaino DualSeed", "Explaino"},
    {"explaino_mult", "Explaino Multiplicity", "Explaino"},
    {"explaino_phoenix", "Explaino Phoenix", "Explaino"},
    {"explaino_transcendental", "Explaino Transcendental", "Explaino"},
    {"explaino_inertial", "Explaino Inertial", "Explaino"},
    {"explaino_julia", "Explaino Julia", "Explaino"},
    {"explaino_rational", "Explaino Rational", "Explaino"},
    {"explaino_collatz", "Explaino Collatz", "Explaino"},
    {"explaino_lambda", "Explaino Lambda", "Explaino"},
    {"explaino_rational_escape", "Explaino Rational Escape", "Explaino"},
    {"explaino_joy", "Explaino Joy", "Explaino"},
    {"explaino_fold", "Explaino Fold", "Explaino"},
    {"explaino_bell", "Explaino Bell", "Explaino"},
    {"explaino_ripple", "Explaino Ripple", "Explaino"},
    {"explaino_splice", "Explaino Splice", "Explaino"},
    {"explaino_vortex", "Explaino Vortex", "Explaino"},
    {"explaino_tension", "Explaino Tension", "Explaino"},
    {"explaino_balance_void", "Explaino BalanceVoid", "Explaino"},
    {"explaino_counterfactual_pair", "Explaino Counterfactual Pair", "Explaino"},
    {"explaino_projection_and_flow", "Explaino Projection and Flow", "Explaino"},
};

UISchemaBinding MakeBinding(const char* kind, const char* path) {
    UISchemaBinding binding;
    binding.kind = kind;
    binding.path = path;
    return binding;
}

UISchemaPredicate MakeEqVisibleIf(const char* path, const char* value) {
    UISchemaPredicate predicate;
    predicate.op = "eq";
    predicate.path = path;
    predicate.value = value;
    return predicate;
}

UISchemaPredicate MakeInVisibleIf(const char* path, const char* value) {
    UISchemaPredicate predicate;
    predicate.op = "in";
    predicate.path = path;
    predicate.value = value;
    return predicate;
}

UISchemaControl MakeParamControl(
    const char* id,
    const char* type,
    const char* label,
    const char* value_type,
    const char* path,
    const json_min::Value& default_value) {
    UISchemaControl control;
    control.id = id;
    control.type = type;
    control.label = label;
    control.value_type = value_type;
    control.has_binding = true;
    control.binding = MakeBinding("param", path);
    control.has_default = true;
    control.def = default_value;
    return control;
}

UISchemaControl MakeRangedParamControl(
    const char* id,
    const char* type,
    const char* label,
    const char* value_type,
    double min,
    double max,
    double step,
    const char* path,
    const json_min::Value& default_value) {
    UISchemaControl control = MakeParamControl(id, type, label, value_type, path, default_value);
    control.min = min;
    control.max = max;
    control.step = step;
    control.has_min = true;
    control.has_max = true;
    control.has_step = true;
    return control;
}

UISchemaControl MakeUiRangedParamControl(
    const char* id,
    const char* type,
    const char* label,
    const char* value_type,
    double uiMin,
    double uiMax,
    double step,
    const char* path,
    const json_min::Value& default_value) {
    UISchemaControl control = MakeParamControl(id, type, label, value_type, path, default_value);
    control.ui_min = uiMin;
    control.ui_max = uiMax;
    control.step = step;
    control.has_ui_min = true;
    control.has_ui_max = true;
    control.has_step = true;
    return control;
}

UISchemaControl MakeSoftMinParamControl(
    const char* id,
    const char* type,
    const char* label,
    const char* value_type,
    double hardMin,
    double uiMin,
    double uiMax,
    double step,
    const char* path,
    const json_min::Value& default_value,
    bool logarithmic = false) {
    UISchemaControl control = MakeUiRangedParamControl(id, type, label, value_type, uiMin, uiMax, step, path, default_value);
    control.min = hardMin;
    control.has_min = true;
    control.logarithmic = logarithmic;
    return control;
}

UISchemaControl MakeMinOnlyParamControl(
    const char* id,
    const char* type,
    const char* label,
    const char* value_type,
    double hardMin,
    double step,
    const char* path,
    const json_min::Value& default_value,
    bool logarithmic = false) {
    UISchemaControl control = MakeParamControl(id, type, label, value_type, path, default_value);
    control.min = hardMin;
    control.step = step;
    control.has_min = true;
    control.has_step = true;
    control.logarithmic = logarithmic;
    return control;
}

UISchemaControl MakeActionControl(const char* id, const char* label, const char* path) {
    UISchemaControl control;
    control.id = id;
    control.type = "button";
    control.label = label;
    control.has_binding = true;
    control.binding = MakeBinding("action", path);
    return control;
}

void SetVisibleForFractalType(UISchemaControl* control, const char* fractalTypeId) {
    if (!control) {
        return;
    }
    control->has_visible_if = true;
    control->visible_if = MakeEqVisibleIf("fractal.view.fractal_type", fractalTypeId);
}

void SetVisibleForFractalTypes(UISchemaControl* control, const char* fractalTypeIds) {
    if (!control) {
        return;
    }
    control->has_visible_if = true;
    control->visible_if = MakeInVisibleIf("fractal.view.fractal_type", fractalTypeIds);
}

UISchemaControl BuildCounterfactualPairRootFamilyControl() {
    UISchemaControl control = MakeParamControl(
        "counterfactual_pair_root_family",
        "combo",
        "Pair Root Family",
        "enum",
        "fractal.params.counterfactual_pair_root_family",
        json_min::Value{std::string("cubic_unit_roots")});
    control.options = {
        {"cubic_unit_roots", "Cubic Unit Roots", ""},
        {"quartic_unit_roots", "Quartic Unit Roots", ""},
    };
    control.help = "Choose the paired Newton family used by both orbits.";
    control.has_help = true;
    SetVisibleForFractalType(&control, "counterfactual_pair");
    return control;
}

UISchemaControl BuildCounterfactualPairFrameControl() {
    UISchemaControl control = MakeParamControl(
        "counterfactual_pair_frame",
        "combo",
        "Perturbation Frame",
        "enum",
        "fractal.params.counterfactual_pair_frame",
        json_min::Value{std::string("world_absolute")});
    control.options = {
        {"world_absolute", "World Absolute", ""},
        {"view_relative", "View Relative", ""},
    };
    control.help = "World Absolute keeps the partner gap fixed in the complex plane. View Relative scales the gap with the current zoom.";
    control.has_help = true;
    SetVisibleForFractalTypes(&control, "counterfactual_pair,explaino_counterfactual_pair");
    return control;
}

UISchemaControl BuildCounterfactualPairOffsetControl(
    const char* id,
    const char* label,
    const char* path,
    double defaultValue) {
    UISchemaControl control = MakeUiRangedParamControl(
        id,
        "drag_float",
        label,
        "float",
        -1.0,
        1.0,
        0.001,
        path,
        json_min::Value{defaultValue});
    SetVisibleForFractalTypes(&control, "counterfactual_pair,explaino_counterfactual_pair");
    return control;
}

UISchemaControl BuildCounterfactualPairReconvergenceControl() {
    UISchemaControl control = MakeSoftMinParamControl(
        "counterfactual_pair_reconvergence_ratio",
        "drag_float",
        "Reconvergence Ratio",
        "float",
        0.0,
        0.0,
        4.0,
        0.01,
        "fractal.params.counterfactual_pair_reconvergence_ratio",
        json_min::Value{0.60});
    control.help =
        "Class 0 = same-root reconvergence when mean pair gap stays under ratio * initial gap. "
        "Class 1 = same-basin drift above that ratio. Class 2 = basin swap. Class 3 = unstable.";
    control.has_help = true;
    SetVisibleForFractalTypes(&control, "counterfactual_pair,explaino_counterfactual_pair");
    return control;
}

UISchemaControl BuildProjectionAndFlowRootFamilyControl() {
    UISchemaControl control = MakeParamControl(
        "projection_and_flow_root_family",
        "combo",
        "Flow Root Family",
        "enum",
        "fractal.params.projection_and_flow_root_family",
        json_min::Value{std::string("cubic_unit_roots")});
    control.options = {
        {"cubic_unit_roots", "Cubic Unit Roots", ""},
        {"quartic_unit_roots", "Quartic Unit Roots", ""},
    };
    control.help = "Choose whether the free Newton step chases the cubic or quartic unit-root family before every radial projection.";
    control.has_help = true;
    SetVisibleForFractalTypes(&control, "projection_and_flow,explaino_projection_and_flow");
    return control;
}

UISchemaControl BuildProjectionAndFlowTargetRadiusControl() {
    UISchemaControl control = MakeSoftMinParamControl(
        "projection_and_flow_target_radius",
        "drag_float",
        "Projection Radius",
        "float",
        1.0e-4,
        0.25,
        4.0,
        0.01,
        "fractal.params.projection_and_flow_target_radius",
        json_min::Value{1.0});
    control.help = "After each free Newton step, project radially onto the circle |z| = radius. This is the explicit admissible manifold for Projection-and-Flow.";
    control.has_help = true;
    SetVisibleForFractalTypes(&control, "projection_and_flow,explaino_projection_and_flow");
    return control;
}

UISchemaControl BuildMagnetFloatControl(
    const char* id,
    const char* label,
    const char* path,
    double uiMin,
    double uiMax,
    double step,
    double defaultValue) {
    UISchemaControl control = MakeUiRangedParamControl(
        id,
        "drag_float",
        label,
        "float",
        uiMin,
        uiMax,
        step,
        path,
        json_min::Value{defaultValue});
    SetVisibleForFractalType(&control, "magnet");
    return control;
}

UISchemaControl BuildMagnetBailoutControl() {
    UISchemaControl control = MakeRangedParamControl(
        "magnet_bailout",
        "slider_float",
        "Magnet Bailout",
        "float",
        2.0,
        64.0,
        0.1,
        "fractal.params.magnet_bailout",
        json_min::Value{12.0});
    SetVisibleForFractalType(&control, "magnet");
    return control;
}

UISchemaControl BuildProjectionAndFlowPressureThresholdControl() {
    UISchemaControl control = MakeSoftMinParamControl(
        "projection_and_flow_pressure_threshold",
        "drag_float",
        "Pressure Threshold",
        "float",
        0.0,
        0.0,
        8.0,
        0.01,
        "fractal.params.projection_and_flow_pressure_threshold",
        json_min::Value{1.0});
    control.help =
        "Classes 0..4N-1 = root sector x transient-pressure band, using the projected orbit's peak radial correction minus its settled correction. Band 0 stays below 25% of the threshold, Band 1 stays in [25%, 50%), Band 2 stays in [50%, 100%), Band 3 saturates at or above the threshold. Class 4N = unstable.";
    control.has_help = true;
    SetVisibleForFractalTypes(&control, "projection_and_flow,explaino_projection_and_flow");
    return control;
}

std::vector<UISchemaOption> BuildSafeModeFractalTypeOptions() {
    std::vector<UISchemaOption> options;
    options.reserve(sizeof(kSafeModeFractalTypeOptionDefs) / sizeof(kSafeModeFractalTypeOptionDefs[0]));
    for (const auto& option : kSafeModeFractalTypeOptionDefs) {
        options.push_back({option.id, option.label, option.group});
    }
    return options;
}

UISchemaControl BuildSafeModeFractalTypeControl() {
    UISchemaControl control = MakeParamControl(
        "fractal_type",
        "combo",
        "Fractal Type",
        "enum",
        "fractal.view.fractal_type",
        json_min::Value{std::string("explaino_all")});
    control.options = BuildSafeModeFractalTypeOptions();
    return control;
}

UISchemaPanel BuildSafeModeViewPanel() {
    UISchemaPanel panel;
    panel.id = "view";
    panel.label = "View (Safe Mode)";
    panel.order = 10;
    panel.has_order = true;

    panel.controls = {
        BuildSafeModeFractalTypeControl(),
        MakeUiRangedParamControl("center_x", "drag_float", "Center X", "float", -2.0, 2.0, 0.001, "fractal.view.center.x", json_min::Value{0.0}),
        MakeUiRangedParamControl("center_y", "drag_float", "Center Y", "float", -2.0, 2.0, 0.001, "fractal.view.center.y", json_min::Value{0.0}),
        MakeMinOnlyParamControl("zoom", "drag_float", "Zoom", "float", 1.0e-12, 0.01, "fractal.view.zoom", json_min::Value{1.0}, true),
        MakeUiRangedParamControl("rotation_deg", "drag_float", "Rotation (deg)", "float", -180.0, 180.0, 0.1, "fractal.view.rotation", json_min::Value{0.0}),
        MakeParamControl("auto_refresh", "checkbox", "Continuous Render", "bool", "fractal.view.auto_refresh", json_min::Value{false}),
        MakeActionControl("render_once", "Render Once", "fractal.actions.render_once"),
        MakeActionControl("reset_view", "Reset View", "fractal.actions.reset_view"),
        MakeActionControl("reset_all", "Reset All", "fractal.actions.reset_all"),
        MakeActionControl("load_state", "Load Finding State", "fractal.actions.load_state"),
        MakeActionControl("capture_finding", "Capture Finding", "fractal.actions.capture_finding"),
    };
    return panel;
}

UISchemaPanel BuildSafeModeFractalPanel() {
    UISchemaPanel panel;
    panel.id = "fractal";
    panel.label = "Fractal (Safe Mode)";
    panel.order = 20;
    panel.has_order = true;

    panel.controls = {
        MakeSoftMinParamControl("max_iter", "slider_int", "Max Iterations", "int", 1.0, 1.0, 5000.0, 1.0, "fractal.params.max_iter", json_min::Value{500.0}),
        MakeRangedParamControl("exposure", "slider_float", "Exposure", "float", 0.1, 5.0, 0.01, "fractal.params.exposure", json_min::Value{1.0}),
        BuildCounterfactualPairRootFamilyControl(),
        BuildCounterfactualPairFrameControl(),
        BuildCounterfactualPairOffsetControl("counterfactual_pair_offset_x", "Partner Offset X", "fractal.params.counterfactual_pair_offset_x", 0.16),
        BuildCounterfactualPairOffsetControl("counterfactual_pair_offset_y", "Partner Offset Y", "fractal.params.counterfactual_pair_offset_y", 0.08),
        BuildCounterfactualPairReconvergenceControl(),
        BuildProjectionAndFlowRootFamilyControl(),
        BuildProjectionAndFlowTargetRadiusControl(),
        BuildProjectionAndFlowPressureThresholdControl(),
        BuildMagnetFloatControl("magnet_seed_real", "Magnet Seed Real", "fractal.params.magnet_seed_real", -2.0, 2.0, 0.01, 0.0),
        BuildMagnetFloatControl("magnet_seed_imag", "Magnet Seed Imag", "fractal.params.magnet_seed_imag", -2.0, 2.0, 0.01, 0.0),
        BuildMagnetFloatControl("magnet_relaxation", "Magnet Relaxation", "fractal.params.magnet_relaxation", 0.05, 1.5, 0.01, 1.0),
        BuildMagnetBailoutControl(),
    };
    return panel;
}

UISchemaPanel BuildSafeModeRenderPanel() {
    UISchemaPanel panel;
    panel.id = "render";
    panel.label = "Render (Safe Mode)";
    panel.order = 30;
    panel.has_order = true;
    panel.controls = {
        MakeRangedParamControl("width", "slider_int", "Width", "int", 64.0, 4096.0, 1.0, "fractal.render.resolution.x", json_min::Value{static_cast<double>(RenderSettings::kDefaultWidth)}),
        MakeRangedParamControl("height", "slider_int", "Height", "int", 64.0, 4096.0, 1.0, "fractal.render.resolution.y", json_min::Value{static_cast<double>(RenderSettings::kDefaultHeight)}),
        MakeRangedParamControl("interaction_debounce_ms", "slider_int", "Interaction Debounce (ms)", "int", 0.0, 1000.0, 10.0, "fractal.render.interaction_debounce_ms", json_min::Value{static_cast<double>(RenderSettings::kDefaultInteractionDebounceMs)}),
        MakeRangedParamControl("preview_target_fps", "slider_float", "Preview Target FPS", "float", 5.0, 120.0, 1.0, "fractal.render.preview_target_fps", json_min::Value{static_cast<double>(RenderSettings::kDefaultPreviewTargetFps)}),
        MakeRangedParamControl("preview_min_scale", "slider_float", "Preview Min Scale", "float", 0.25, 1.0, 0.05, "fractal.render.preview_min_scale", json_min::Value{static_cast<double>(RenderSettings::kDefaultPreviewMinScale)}),
    };
    return panel;
}

} // namespace

UISchema BuildSafeModeSchema() {
    UISchema schema;
    schema.schema_version = "1";
    schema.name_space = "fractal";
    schema.panels = {
        BuildSafeModeViewPanel(),
        BuildSafeModeFractalPanel(),
        BuildSafeModeRenderPanel(),
    };
    return schema;
}
