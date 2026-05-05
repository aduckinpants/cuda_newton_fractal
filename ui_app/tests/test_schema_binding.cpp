#include "../src/schema_binding.h"

#include "../src/explaino_seed.h"
#include "../third_party/imgui/imgui.h"

#include <iostream>

namespace {

struct ImGuiTestContext {
    ImGuiContext* context = nullptr;

    ImGuiTestContext() {
        context = ImGui::CreateContext();
        ImGui::SetCurrentContext(context);
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(640.0f, 480.0f);
        io.DeltaTime = 1.0f / 60.0f;
        unsigned char* pixels = nullptr;
        int width = 0;
        int height = 0;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    }

    ~ImGuiTestContext() {
        if (context) {
            ImGui::SetCurrentContext(context);
            ImGui::DestroyContext(context);
        }
    }
};

void BeginFrame() {
    ImGui::NewFrame();
    ImGui::Begin("SchemaBindingTest");
}

void EndFrame() {
    ImGui::End();
    ImGui::Render();
}

BindingContext MakeBindingContext(ViewState* view, KernelParams* params, RenderSettings* render, LensSettings* lens) {
    BindingContext ctx;
    ctx.view = view;
    ctx.params = params;
    ctx.render = render;
    ctx.lens = lens;
    return ctx;
}

UISchemaControl MakeBoundControl(const char* id, const char* type, const char* label, const char* valueType, const char* kind, const char* path) {
    UISchemaControl control;
    control.id = id;
    control.type = type;
    control.label = label;
    control.value_type = valueType;
    control.has_binding = true;
    control.binding.kind = kind;
    control.binding.path = path;
    return control;
}

} // namespace

int main() {
    {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        BindingContext ctx = MakeBindingContext(&view, &params, &render, &lens);

        if (ctx.GetEnumId("fractal.params.poly_kind") != "z3_minus_1") {
            std::cerr << "Expected poly kind enum round-trip to start at z3_minus_1\n";
            return 1;
        }
        if (ctx.GetEnumId("fractal.view.fractal_type") != "explaino") {
            std::cerr << "Expected fractal type enum round-trip to start at explaino\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.params.coloring_mode", "smooth_escape") ||
            ctx.GetEnumId("fractal.params.coloring_mode") != "smooth_escape") {
            std::cerr << "Expected coloring mode enum round-trip to accept smooth_escape\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.view.fractal_type", "lambda") ||
            ctx.GetEnumId("fractal.view.fractal_type") != "lambda") {
            std::cerr << "Expected fractal type enum round-trip to accept lambda\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.view.camera_behavior", "orbit") ||
            ctx.GetEnumId("fractal.view.camera_behavior") != "orbit") {
            std::cerr << "Expected camera behavior enum round-trip to accept orbit\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.render.sample_tier", "standard") ||
            ctx.GetEnumId("fractal.render.sample_tier") != "standard") {
            std::cerr << "Expected sample tier enum round-trip to accept standard\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.params.mcmullen_preset", "z4_z2") ||
            ctx.GetEnumId("fractal.params.mcmullen_preset") != "z4_z2") {
            std::cerr << "Expected McMullen preset enum round-trip to accept z4_z2\n";
            return 1;
        }
        if (ctx.SetEnumId("fractal.view.fractal_type", "not_real")) {
            std::cerr << "Invalid enum ids should fail instead of silently falling back\n";
            return 1;
        }
        if (!ctx.GetEnumId("fractal.unknown.path").empty()) {
            std::cerr << "Unknown enum paths should return an empty id\n";
            return 1;
        }

        UISchemaControl coloringMode = MakeBoundControl("coloring_mode", "combo", "Coloring Mode", "enum", "param", "fractal.params.coloring_mode");
        coloringMode.options = {
            {"root_basin", "Root Basins (Root-Finding)", ""},
            {"joy_basins", "Joy Basins (All paths succeed)", ""},
            {"iteration_count", "Iteration Count", ""},
            {"smooth_escape", "Smooth Escape", ""},
            {"phase", "Phase (Hue Wheel)", ""},
            {"iteration_bands", "Iteration Bands", ""},
        };

        view.fractal_type = FractalType::explaino;
        const std::vector<const UISchemaOption*> explainoOptions = ResolveVisibleEnumOptions(coloringMode, ctx);
        if (explainoOptions.size() != 6) {
            std::cerr << "Expected basin-capable fractals to expose all coloring options\n";
            return 1;
        }

        view.fractal_type = FractalType::mandelbrot;
        const std::vector<const UISchemaOption*> mandelbrotOptions = ResolveVisibleEnumOptions(coloringMode, ctx);
        if (mandelbrotOptions.size() != 4) {
            std::cerr << "Expected escape-time fractals to hide basin-only coloring options\n";
            return 1;
        }
        for (const UISchemaOption* option : mandelbrotOptions) {
            if (!option) {
                std::cerr << "Expected visible coloring options to remain addressable\n";
                return 1;
            }
            if (option->id == "root_basin" || option->id == "joy_basins") {
                std::cerr << "Escape-time fractals should not expose basin-only coloring options\n";
                return 1;
            }
        }
    }

    {
        UISchemaControl centerX = MakeBoundControl("center_x", "drag_float", "Center X", "float", "param", "fractal.view.center.x");
        centerX.has_ui_min = true;
        centerX.ui_min = -2.0;
        centerX.has_ui_max = true;
        centerX.ui_max = 2.0;

        NumericControlRange centerRange = ResolveNumericControlRange(centerX);
        if (!centerRange.has_widget_min || !centerRange.has_widget_max ||
            centerRange.widget_min != -2.0 || centerRange.widget_max != 2.0) {
            std::cerr << "Expected drag controls to expose explicit widget ranges from ui_min/ui_max\n";
            return 1;
        }
        if (centerRange.has_hard_min || centerRange.has_hard_max) {
            std::cerr << "Expected ui_min/ui_max-only drag controls to remain unclamped\n";
            return 1;
        }

        UISchemaControl epsilon = MakeBoundControl("epsilon", "slider_float", "Epsilon", "float", "param", "fractal.params.epsilon");
        epsilon.has_min = true;
        epsilon.min = 0.0001;
        epsilon.has_max = true;
        epsilon.max = 1.0;

        NumericControlRange epsilonRange = ResolveNumericControlRange(epsilon);
        if (!epsilonRange.has_widget_min || !epsilonRange.has_widget_max ||
            !epsilonRange.has_hard_min || !epsilonRange.has_hard_max) {
            std::cerr << "Expected slider controls to keep hard min/max as the widget range when no ui_min/ui_max override exists\n";
            return 1;
        }
        if (epsilonRange.widget_min != 0.0001 || epsilonRange.widget_max != 1.0 ||
            epsilonRange.hard_min != 0.0001 || epsilonRange.hard_max != 1.0) {
            std::cerr << "Expected hard min/max to round-trip through numeric range resolution\n";
            return 1;
        }

        UISchemaControl maxIter = MakeBoundControl("max_iter", "slider_int", "Max Iterations", "int", "param", "fractal.params.max_iter");
        maxIter.has_min = true;
        maxIter.min = 1.0;
        maxIter.has_ui_min = true;
        maxIter.ui_min = 1.0;
        maxIter.has_ui_max = true;
        maxIter.ui_max = 5000.0;

        NumericControlRange maxIterRange = ResolveNumericControlRange(maxIter);
        if (!maxIterRange.has_widget_min || !maxIterRange.has_widget_max ||
            !maxIterRange.has_hard_min || maxIterRange.has_hard_max) {
            std::cerr << "Expected mixed int ranges to keep a hard minimum with a UI-only maximum\n";
            return 1;
        }
        if (maxIterRange.widget_min != 1.0 || maxIterRange.widget_max != 5000.0 ||
            maxIterRange.hard_min != 1.0) {
            std::cerr << "Expected max_iter range resolution to preserve the runtime minimum and UI-only slider cap\n";
            return 1;
        }

        UISchemaControl explainoSeed = MakeBoundControl("explaino_seed", "slider_double", "Explaino Seed", "double", "param", "fractal.params.explaino_seed");
        explainoSeed.has_ui_min = true;
        explainoSeed.ui_min = -10.0;
        explainoSeed.has_ui_max = true;
        explainoSeed.ui_max = 10.0;

        NumericControlRange explainoSeedRange = ResolveNumericControlRange(explainoSeed);
        if (!explainoSeedRange.has_widget_min || !explainoSeedRange.has_widget_max ||
            explainoSeedRange.has_hard_min || explainoSeedRange.has_hard_max) {
            std::cerr << "Expected explaino seed sliders to keep UI-only bounds with no hard clamp\n";
            return 1;
        }

        UISchemaControl explainoDamping = MakeBoundControl("explaino_damping", "slider_float", "Newton Damping", "float", "param", "fractal.params.explaino_damping");
        explainoDamping.has_ui_min = true;
        explainoDamping.ui_min = 0.01;
        explainoDamping.has_ui_max = true;
        explainoDamping.ui_max = 10.0;

        NumericControlRange explainoDampingRange = ResolveNumericControlRange(explainoDamping);
        if (!explainoDampingRange.has_widget_min || !explainoDampingRange.has_widget_max ||
            explainoDampingRange.widget_min != 0.01 || explainoDampingRange.widget_max != 10.0 ||
            explainoDampingRange.has_hard_min || explainoDampingRange.has_hard_max) {
            std::cerr << "Expected Newton damping sliders to keep their shipped UI span without a hard clamp\n";
            return 1;
        }

        UISchemaControl momentumBeta = MakeBoundControl("momentum_beta", "slider_float", "Momentum Beta", "float", "param", "fractal.params.momentum_beta");
        momentumBeta.has_ui_min = true;
        momentumBeta.ui_min = -1.0;
        momentumBeta.has_ui_max = true;
        momentumBeta.ui_max = 1.0;

        NumericControlRange momentumBetaRange = ResolveNumericControlRange(momentumBeta);
        if (!momentumBetaRange.has_widget_min || !momentumBetaRange.has_widget_max ||
            momentumBetaRange.widget_min != -1.0 || momentumBetaRange.widget_max != 1.0 ||
            momentumBetaRange.has_hard_min || momentumBetaRange.has_hard_max) {
            std::cerr << "Expected momentum beta to use a signed UI-only slider range with no hard clamp\n";
            return 1;
        }
    }

    {
        ViewState view{};
        bool* boolValue = nullptr;
        BindingContext viewOnly = MakeBindingContext(&view, nullptr, nullptr, nullptr);
        if (!viewOnly.BindBool("fractal.view.auto_refresh", &boolValue) || boolValue != &view.auto_refresh) {
            std::cerr << "Expected view-only bool bindings to succeed without a render context\n";
            return 1;
        }

        RenderSettings render{};
        boolValue = nullptr;
        BindingContext renderOnly = MakeBindingContext(nullptr, nullptr, &render, nullptr);
        if (!renderOnly.BindBool("fractal.render.benchmark", &boolValue) || boolValue != &render.benchmark) {
            std::cerr << "Expected render-only bool bindings to succeed without a view context\n";
            return 1;
        }

        LensSettings lens{};
        boolValue = nullptr;
        BindingContext lensOnly = MakeBindingContext(nullptr, nullptr, nullptr, &lens);
        if (!lensOnly.BindBool("fractal.lens.enabled", &boolValue) || boolValue != &lens.enabled) {
            std::cerr << "Expected lens-only bool bindings to succeed without view/render contexts\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        BindingContext ctx = MakeBindingContext(&view, &params, &render, &lens);

        UISchemaPredicate invalidPath{};
        invalidPath.op = "eq";
        invalidPath.path = "fractal.view.not_real";
        invalidPath.value = "explaino";
        if (ctx.EvalVisibleIf(invalidPath)) {
            std::cerr << "Invalid visible_if paths should fail closed\n";
            return 1;
        }

        UISchemaPredicate invalidOp{};
        invalidOp.op = "mystery_op";
        invalidOp.path = "fractal.view.fractal_type";
        invalidOp.value = "explaino";
        if (ctx.EvalVisibleIf(invalidOp)) {
            std::cerr << "Invalid visible_if operators should fail closed\n";
            return 1;
        }

        UISchemaPredicate invalidNumeric{};
        invalidNumeric.op = "gt";
        invalidNumeric.path = "fractal.view.zoom";
        invalidNumeric.value = "not_a_number";
        if (ctx.EvalVisibleIf(invalidNumeric)) {
            std::cerr << "Invalid visible_if numeric values should fail closed\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        BindingContext ctx = MakeBindingContext(&view, &params, &render, &lens);
        bool dirty = false;

        UISchemaControl boolControl = MakeBoundControl("auto_refresh", "checkbox", "Continuous Render", "bool", "param", "fractal.view.auto_refresh");
        boolControl.has_default = true;
        boolControl.def = json_min::Value{true};
        if (!ApplySchemaDefaultForControl(boolControl, ctx, &dirty) || !view.auto_refresh || !dirty) {
            std::cerr << "Expected bool defaults to apply through binding context\n";
            return 1;
        }

        dirty = false;
        UISchemaControl intControl = MakeBoundControl("width", "slider_int", "Width", "int", "param", "fractal.render.resolution.x");
        intControl.has_default = true;
        intControl.def = json_min::Value{2048.0};
        if (!ApplySchemaDefaultForControl(intControl, ctx, &dirty) || render.resolution.x != 2048 || !dirty) {
            std::cerr << "Expected int defaults to apply through binding context\n";
            return 1;
        }

        dirty = false;
        UISchemaControl seedControl = MakeBoundControl("seed", "drag_double", "Seed", "double", "param", "fractal.params.explaino_seed");
        seedControl.has_default = true;
        seedControl.def = json_min::Value{7.25};
        if (!ApplySchemaDefaultForControl(seedControl, ctx, &dirty) || !dirty) {
            std::cerr << "Expected explaino seed defaults to apply through combined-seed binding\n";
            return 1;
        }
        if (ExplainoSeedCombined(view, params) != 7.25) {
            std::cerr << "Expected explaino combined seed to track schema default application\n";
            return 1;
        }

        dirty = false;
        UISchemaControl enumControl = MakeBoundControl("fractal_type", "combo", "Fractal Type", "enum", "param", "fractal.view.fractal_type");
        enumControl.has_default = true;
        enumControl.def = json_min::Value{std::string("nova")};
        if (!ApplySchemaDefaultForControl(enumControl, ctx, &dirty) || view.fractal_type != FractalType::nova || !dirty) {
            std::cerr << "Expected enum defaults to apply through binding context\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        BindingContext ctx = MakeBindingContext(&view, &params, &render, &lens);

        UISchema schema;
        UISchemaPanel panel;
        panel.id = "panel";
        panel.label = "Panel";

        UISchemaControl actionControl = MakeBoundControl("bad_action", "button", "Bad Action", "", "action", "fractal.actions.not_real");
        panel.controls.push_back(actionControl);
        schema.panels.push_back(panel);

        std::string error;
        if (ValidateSchemaBindings(schema, ctx, &error)) {
            std::cerr << "Unknown action bindings should fail validation\n";
            return 1;
        }
        if (error.find("Unknown action binding path") == std::string::npos) {
            std::cerr << "Expected unknown action binding validation error details\n";
            return 1;
        }

        schema.panels.clear();
        panel.controls.clear();
        UISchemaControl enumControl = MakeBoundControl("sample_tier", "combo", "Sample Tier", "enum", "param", "fractal.render.sample_tier");
        enumControl.options = {
            {"tier_auto", "Auto", ""},
            {"fast", "Fast", ""},
            {"standard", "Standard", ""},
        };
        panel.controls.push_back(enumControl);
        schema.panels.push_back(panel);
        error.clear();
        if (!ValidateSchemaBindings(schema, ctx, &error)) {
            std::cerr << "Supported enum binding paths should validate cleanly: " << error << "\n";
            return 1;
        }

        schema.panels.clear();
        panel.controls.clear();
        UISchemaControl invalidVisibleIf = MakeBoundControl("bad_visible_if", "slider_float", "Bad VisibleIf", "float", "param", "fractal.params.exposure");
        invalidVisibleIf.has_visible_if = true;
        invalidVisibleIf.visible_if.op = "eq";
        invalidVisibleIf.visible_if.path = "fractal.view.not_real";
        invalidVisibleIf.visible_if.value = "explaino";
        panel.controls.push_back(invalidVisibleIf);
        schema.panels.push_back(panel);
        error.clear();
        if (ValidateSchemaBindings(schema, ctx, &error)) {
            std::cerr << "Invalid visible_if predicates should fail schema validation\n";
            return 1;
        }
        if (error.find("visible_if") == std::string::npos) {
            std::cerr << "Expected invalid visible_if validation error details\n";
            return 1;
        }

        schema.panels.clear();
        panel.controls.clear();
        UISchemaControl invalidIntCombo = MakeBoundControl("bad_int_combo", "combo", "Bad Int Combo", "int", "param", "fractal.render.device_id");
        invalidIntCombo.options = {
            {"0", "Zero", ""},
            {"not_an_int", "Broken", ""},
        };
        panel.controls.push_back(invalidIntCombo);
        schema.panels.push_back(panel);
        error.clear();
        if (ValidateSchemaBindings(schema, ctx, &error)) {
            std::cerr << "Invalid int combo option ids should fail schema validation\n";
            return 1;
        }
        if (error.find("not_an_int") == std::string::npos) {
            std::cerr << "Expected invalid int combo validation error details\n";
            return 1;
        }
    }

    {
        ImGuiTestContext imgui;
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        BindingContext ctx = MakeBindingContext(&view, &params, &render, &lens);

        BeginFrame();
        UISchemaControl unbound;
        unbound.id = "unbound";
        unbound.type = "checkbox";
        unbound.label = "Unbound";
        if (RenderControlFromSchema(unbound, ctx, nullptr, nullptr, nullptr)) {
            std::cerr << "Unbound controls should render as diagnostics and report no change\n";
            return 1;
        }
        EndFrame();

        BeginFrame();
        UISchemaControl badAction = MakeBoundControl("bad_action", "button", "Bad Action", "", "param", "fractal.view.auto_refresh");
        if (RenderControlFromSchema(badAction, ctx, nullptr, nullptr, nullptr)) {
            std::cerr << "Buttons with non-action bindings should fail closed\n";
            return 1;
        }
        EndFrame();

        BeginFrame();
        bool renderOnce = false;
        bool interacted = false;
        UISchemaControl renderOnceButton = MakeBoundControl("render_once", "button", "Render Once", "", "action", "fractal.actions.render_once");
        if (!RenderControlFromSchema(renderOnceButton, ctx, nullptr, &renderOnce, &interacted)) {
            std::cerr << "Valid action buttons should render successfully even without a click\n";
            return 1;
        }
        if (renderOnce || interacted) {
            std::cerr << "Idle action buttons should not mark render-once or interaction flags\n";
            return 1;
        }
        EndFrame();

        BeginFrame();
        UISchemaControl badCheckbox = MakeBoundControl("bad_checkbox", "checkbox", "Bad Checkbox", "bool", "param", "fractal.params.not_a_real_bool");
        if (RenderControlFromSchema(badCheckbox, ctx, nullptr, nullptr, nullptr)) {
            std::cerr << "Checkboxes with invalid binding paths should fail closed\n";
            return 1;
        }
        EndFrame();

        BeginFrame();
        UISchemaControl unsupported = MakeBoundControl("unsupported", "mystery_control", "Unsupported", "float", "param", "fractal.params.exposure");
        if (RenderControlFromSchema(unsupported, ctx, nullptr, nullptr, nullptr)) {
            std::cerr << "Unsupported control types should fail closed\n";
            return 1;
        }
        EndFrame();
    }

    std::cout << "test_schema_binding: all passed\n";
    return 0;
}