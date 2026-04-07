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