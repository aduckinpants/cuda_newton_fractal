#include "../src/schema_binding.h"

#include "../src/color_pipeline_core.h"
#include "../src/color_pipeline_window.h"
#include "../src/imgui_stack_editor.h"

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

bool NearlyEqual(double left, double right, double eps = 1.0e-6) {
    const double delta = left - right;
    return delta < eps && delta > -eps;
}

constexpr double kTestMaxLog2Zoom = 1020.0;

double TestLog2(double value) {
    return std::log(value) / std::log(2.0);
}

double TestExp2(double value) {
    return std::exp(value * std::log(2.0));
}

double TestSafeZoomFromLog2(double log2Zoom) {
    double clamped = log2Zoom;
    const double minLog2 = TestLog2(1.0e-30);
    if (clamped < minLog2) {
        clamped = minLog2;
    }
    if (clamped > kTestMaxLog2Zoom) {
        clamped = kTestMaxLog2Zoom;
    }
    return TestExp2(clamped);
}

void SyncTestViewUiFromHp(ViewState& view) {
    double zoom = TestSafeZoomFromLog2(view.log2_zoom);
    if (zoom < 1.0e-30) {
        zoom = 1.0e-30;
    }
    if (zoom > 1.0e30) {
        zoom = 1.0e30;
    }
    view.zoom = static_cast<float>(zoom);
    view.center.x = static_cast<float>(view.center_hp_x);
    view.center.y = static_cast<float>(view.center_hp_y);
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
        if (ctx.GetEnumId("fractal.params.color_signal") != "smooth_escape" ||
            ctx.GetEnumId("fractal.params.color_palette") != "cyclic_escape" ||
            ctx.GetEnumId("fractal.params.color_grading") != "escape_default") {
            std::cerr << "Expected legacy coloring mode edits to keep the split color pipeline in sync\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.params.color_signal", "phase_angle") ||
            ctx.GetEnumId("fractal.params.color_signal") != "phase_angle" ||
            ctx.GetEnumId("fractal.params.color_palette") != "phase_wheel" ||
            ctx.GetEnumId("fractal.params.color_grading") != "phase_default" ||
            ctx.GetEnumId("fractal.params.coloring_mode") != "phase") {
            std::cerr << "Expected split color signal edits to coerce the rest of the color pipeline onto a valid exact runtime combination\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.params.color_signal", "escape_magnitude") ||
            ctx.GetEnumId("fractal.params.color_signal") != "escape_magnitude" ||
            ctx.GetEnumId("fractal.params.color_palette") != "cyclic_escape" ||
            ctx.GetEnumId("fractal.params.color_grading") != "escape_default" ||
            ctx.GetEnumId("fractal.params.coloring_mode") != "smooth_escape") {
            std::cerr << "Expected widened split color signal edits to accept escape_magnitude through the runtime-supported programmable mirror\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.params.color_signal", "orbit_stripe") ||
            ctx.GetEnumId("fractal.params.color_signal") != "orbit_stripe" ||
            ctx.GetEnumId("fractal.params.color_palette") != "phase_wheel" ||
            ctx.GetEnumId("fractal.params.color_grading") != "phase_default" ||
            ctx.GetEnumId("fractal.params.coloring_mode") != "phase") {
            std::cerr << "Expected widened split color signal edits to accept orbit_stripe through the runtime-supported programmable mirror\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.params.coloring_mode", "smooth_escape") ||
            !ctx.SetEnumId("fractal.params.color_signal", "root_proximity") ||
            ctx.GetEnumId("fractal.params.color_signal") != "root_proximity" ||
            ctx.GetEnumId("fractal.params.color_palette") != "cyclic_escape" ||
            ctx.GetEnumId("fractal.params.color_grading") != "escape_default" ||
            ctx.GetEnumId("fractal.params.coloring_mode") != "smooth_escape") {
            std::cerr << "Expected basin-capable split color signal edits to accept family-gated root_proximity through the current source-only programmable mirror\n";
            return 1;
        }
        view.fractal_type = FractalType::mandelbrot;
        if (ctx.SetEnumId("fractal.params.color_signal", "root_proximity")) {
            std::cerr << "Expected escape-time families to reject family-gated root_proximity instead of silently coercing it\n";
            return 1;
        }
        view.fractal_type = FractalType::explaino;
        if (!ctx.SetEnumId("fractal.params.coloring_mode", "smooth_escape") ||
            !ctx.SetEnumId("fractal.params.color_grading", "phase_default") ||
            ctx.GetEnumId("fractal.params.coloring_mode") != "phase" ||
            ctx.GetEnumId("fractal.params.color_signal") != "phase_angle" ||
            ctx.GetEnumId("fractal.params.color_palette") != "phase_wheel" ||
            ctx.GetEnumId("fractal.params.color_grading") != "phase_default") {
            std::cerr << "Expected public coloring mode plus grading edits to stay on a coherent runtime-supported color pipeline\n";
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
        if (ctx.SetEnumId("fractal.params.color_palette", "not_real")) {
            std::cerr << "Invalid split-color enum ids should fail instead of silently falling back\n";
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

        UISchemaControl colorSignal = MakeBoundControl("color_signal", "combo", "Color Signal", "enum", "param", "fractal.params.color_signal");
        colorSignal.options = {
            {"root_index", "Root Index", ""},
            {"iteration_count", "Iteration Count", ""},
            {"smooth_escape", "Smooth Escape", ""},
            {"phase_angle", "Phase Angle", ""},
            {"iteration_bands", "Iteration Bands", ""},
            {"escape_magnitude", "Escape Magnitude", ""},
            {"orbit_stripe", "Orbit Stripe", ""},
            {"root_proximity", "Root Proximity", ""},
        };

        view.fractal_type = FractalType::explaino;
        const std::vector<const UISchemaOption*> explainoSignalOptions = ResolveVisibleEnumOptions(colorSignal, ctx);
        if (explainoSignalOptions.size() != 8) {
            std::cerr << "Expected basin-capable fractals to expose the widened split color signal set including root_proximity\n";
            return 1;
        }
        bool foundEscapeMagnitude = false;
        bool foundOrbitStripe = false;
        bool foundRootProximity = false;
        for (const UISchemaOption* option : explainoSignalOptions) {
            if (!option) {
                std::cerr << "Expected widened split color signal options to remain addressable\n";
                return 1;
            }
            if (option->id == "escape_magnitude") foundEscapeMagnitude = true;
            if (option->id == "orbit_stripe") foundOrbitStripe = true;
            if (option->id == "root_proximity") foundRootProximity = true;
        }
        if (!foundEscapeMagnitude || !foundOrbitStripe || !foundRootProximity) {
            std::cerr << "Expected basin-capable fractals to expose all widened split color source ids\n";
            return 1;
        }

        view.fractal_type = FractalType::mandelbrot;
        const std::vector<const UISchemaOption*> mandelbrotSignalOptions = ResolveVisibleEnumOptions(colorSignal, ctx);
        if (mandelbrotSignalOptions.size() != 6) {
            std::cerr << "Expected escape-time fractals to expose the widened reusable split color signals while still hiding basin-only ones\n";
            return 1;
        }
        for (const UISchemaOption* option : mandelbrotSignalOptions) {
            if (!option) {
                std::cerr << "Expected visible split color signal options to remain addressable\n";
                return 1;
            }
            if (option->id == "root_index" || option->id == "root_proximity") {
                std::cerr << "Escape-time fractals should not expose basin-only split color signals\n";
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
        NumericDragWidgetBounds centerDragBounds = ResolveNumericDragWidgetBounds(centerX);
        if (centerDragBounds.has_bounds) {
            std::cerr << "Expected ui-only camera center drags to avoid in-widget clamp bounds\n";
            return 1;
        }

        UISchemaControl zoom = MakeBoundControl("zoom", "drag_float", "Zoom", "float", "param", "fractal.view.zoom");
        zoom.has_min = true;
        zoom.min = 1.0e-12;

        NumericControlRange zoomRange = ResolveNumericControlRange(zoom);
        if (zoomRange.has_widget_max ||
            !zoomRange.has_hard_min || zoomRange.hard_min != 1.0e-12 || zoomRange.has_hard_max) {
            std::cerr << "Expected zoom drags to keep their true hard minimum without a UI maximum cap\n";
            return 1;
        }
        NumericDragWidgetBounds zoomDragBounds = ResolveNumericDragWidgetBounds(zoom);
        if (zoomDragBounds.has_bounds) {
            std::cerr << "Expected zoom drags to avoid in-widget clamp bounds when only a one-sided hard limit exists\n";
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

        UISchemaControl lambdaReal = MakeBoundControl("lambda_real", "drag_float", "Lambda Real", "float", "param", "fractal.params.lambda_real");
        lambdaReal.has_min = true;
        lambdaReal.min = -4.0;
        lambdaReal.has_max = true;
        lambdaReal.max = 4.0;
        NumericDragWidgetBounds lambdaRealDragBounds = ResolveNumericDragWidgetBounds(lambdaReal);
        if (!lambdaRealDragBounds.has_bounds || lambdaRealDragBounds.min != -4.0 || lambdaRealDragBounds.max != 4.0) {
            std::cerr << "Expected truly clamped drags to keep their bilateral hard bounds in the widget\n";
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

        FunctionParamDescriptor phaseOffsetParam = MakeColorPipelineFloatParam(
            "signal.phase_offset",
            "Phase Offset",
            "Rotate the sampled phase before downstream palette work.",
            -3.141592653589793,
            3.141592653589793,
            0.01,
            0.0);
        NumericControlRange phaseOffsetRange = ResolveColorPipelineNumericControlRange(phaseOffsetParam);
        if (!phaseOffsetRange.has_widget_min || !phaseOffsetRange.has_widget_max ||
            !phaseOffsetRange.has_hard_min || !phaseOffsetRange.has_hard_max ||
            phaseOffsetRange.widget_min != -3.141592653589793 ||
            phaseOffsetRange.widget_max != 3.141592653589793) {
            std::cerr << "Expected color-pipeline float params to preserve their widget and hard bounds through the shared numeric range path\n";
            return 1;
        }
        float clampedPhaseOffset = 9.0f;
        ClampColorPipelineNumericValue(&clampedPhaseOffset, phaseOffsetRange);
        if (!NearlyEqual(clampedPhaseOffset, static_cast<float>(phaseOffsetRange.hard_max))) {
            std::cerr << "Expected color-pipeline float params to clamp edits to their declared hard maximum\n";
            return 1;
        }

        FunctionParamDescriptor bandCountParam = MakeColorPipelineIntParam(
            "signal.band_count",
            "Band Count",
            "Choose how many bands to carve out of the escape signal.",
            2,
            24,
            1,
            8);
        NumericControlRange bandCountRange = ResolveColorPipelineNumericControlRange(bandCountParam);
        int clampedBandCount = 64;
        ClampColorPipelineNumericValue(&clampedBandCount, bandCountRange);
        if (clampedBandCount != 24) {
            std::cerr << "Expected color-pipeline int params to clamp edits to their declared hard maximum\n";
            return 1;
        }

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        view.center_hp_x = 1.25;
        view.center_hp_y = -0.75;
        view.log2_zoom = TestLog2(32.0);
        SyncTestViewUiFromHp(view);

        BindingContext cameraCtx = MakeBindingContext(&view, &params, &render, &lens);

        UISchemaBinding centerXBinding;
        centerXBinding.kind = "param";
        centerXBinding.path = "fractal.view.center.x";

        double displayedValue = 0.0;
        if (!TryGetFloatControlDisplayValue(centerXBinding, cameraCtx, &displayedValue) ||
            !NearlyEqual(displayedValue, 1.25, 1.0e-12)) {
            std::cerr << "Expected camera center controls to read back from the HP center authority\n";
            return 1;
        }

        view.center.x = 999.0f;
        if (!TryGetFloatControlDisplayValue(centerXBinding, cameraCtx, &displayedValue) ||
            !NearlyEqual(displayedValue, 1.25, 1.0e-12)) {
            std::cerr << "Expected camera center controls to ignore stale UI mirror values\n";
            return 1;
        }

        NumericControlRange cameraCenterRange{};
        if (!ApplyFloatControlEdit(centerXBinding, cameraCtx, cameraCenterRange, -2.5) ||
            !NearlyEqual(view.center_hp_x, -2.5, 1.0e-12) ||
            !NearlyEqual(view.center.x, -2.5f, 1.0e-6) ||
            !NearlyEqual(view.center_hp_y, -0.75, 1.0e-12)) {
            std::cerr << "Expected camera center edits to update HP authority and resync the UI mirror\n";
            return 1;
        }

        UISchemaBinding zoomBinding;
        zoomBinding.kind = "param";
        zoomBinding.path = "fractal.view.zoom";

        view.log2_zoom = TestLog2(32.0);
        SyncTestViewUiFromHp(view);
        view.zoom = 0.0f;
        if (!TryGetFloatControlDisplayValue(zoomBinding, cameraCtx, &displayedValue) ||
            !NearlyEqual(displayedValue, 32.0, 1.0e-12)) {
            std::cerr << "Expected camera zoom controls to read back from log2_zoom instead of a stale float mirror\n";
            return 1;
        }

        NumericControlRange cameraZoomRange{};
        cameraZoomRange.has_hard_min = true;
        cameraZoomRange.hard_min = 1.0e-12;
        if (!ApplyFloatControlEdit(zoomBinding, cameraCtx, cameraZoomRange, 1.0e9) ||
            !NearlyEqual(TestSafeZoomFromLog2(view.log2_zoom), 1.0e9, 1.0) ||
            view.zoom <= 0.0f) {
            std::cerr << "Expected camera zoom edits to update log2_zoom and preserve a positive synced zoom value\n";
            return 1;
        }

        if (!ApplyFloatControlEdit(zoomBinding, cameraCtx, cameraZoomRange, 0.0) ||
            TestSafeZoomFromLog2(view.log2_zoom) < 1.0e-12 ||
            view.zoom < 1.0e-12f) {
            std::cerr << "Expected camera zoom edits to respect only the hard zoom floor when clamped\n";
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
        render.resolution.x = 640;
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
        view.center_hp_x = 0.0;
        view.center_hp_y = 0.0;
        view.log2_zoom = 0.0;
        SyncTestViewUiFromHp(view);
        UISchemaControl centerDefault = MakeBoundControl("center_x", "drag_float", "Center X", "float", "param", "fractal.view.center.x");
        centerDefault.has_default = true;
        centerDefault.def = json_min::Value{1.5};
        if (!ApplySchemaDefaultForControl(centerDefault, ctx, &dirty) || !dirty ||
            !NearlyEqual(view.center_hp_x, 1.5, 1.0e-12) || !NearlyEqual(view.center.x, 1.5f, 1.0e-6)) {
            std::cerr << "Expected camera center defaults to update HP authority and the synced UI mirror\n";
            return 1;
        }

        dirty = false;
        UISchemaControl zoomDefault = MakeBoundControl("zoom", "drag_float", "Zoom", "float", "param", "fractal.view.zoom");
        zoomDefault.has_default = true;
        zoomDefault.has_min = true;
        zoomDefault.min = 1.0e-12;
        zoomDefault.def = json_min::Value{1000000.0};
        if (!ApplySchemaDefaultForControl(zoomDefault, ctx, &dirty) || !dirty ||
            !NearlyEqual(TestSafeZoomFromLog2(view.log2_zoom), 1000000.0, 1.0) || view.zoom <= 0.0f) {
            std::cerr << "Expected camera zoom defaults to update log2_zoom and keep the UI mirror synced\n";
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
        ColorPipelineRenderInteractionState interactionState{};
        NoteColorPipelineInteractionSnapshot(false, false, true, false, &interactionState);
        NoteColorPipelineInteractionSnapshot(false, false, false, false, &interactionState);

        if (!interactionState.has_active_item) {
            std::cerr << "Expected a slider-side active drag to survive the combined slider-plus-input seam so end-of-frame apply stays suppressed while dragging\n";
            return 1;
        }
    }

    {
        const std::vector<ColorPipelineLaneCatalog>& coreCatalogs = color_pipeline_core::GetColorPipelineLaneCatalogs();
        if (coreCatalogs.size() != 3 ||
            coreCatalogs[0].lane_id != std::string("source") ||
            coreCatalogs[1].lane_id != std::string("shape") ||
            coreCatalogs[2].lane_id != std::string("palette")) {
            std::cerr << "Expected the extracted advanced color core to own the shipped Source / Shape / Palette lane catalog\n";
            return 1;
        }
        const ColorPipelineLaneCatalog* coreSourceCatalog = color_pipeline_core::FindColorPipelineLaneCatalog("source");
        if (!coreSourceCatalog ||
            coreSourceCatalog->default_function_id != std::string("smooth_escape_ramp") ||
            coreSourceCatalog->functions.size() != 6 ||
            coreSourceCatalog->functions[0].id != "smooth_escape_ramp" ||
            coreSourceCatalog->functions[1].id != "phase_orbit" ||
            coreSourceCatalog->functions[2].id != "banded_signal" ||
            coreSourceCatalog->functions[3].id != "escape_magnitude" ||
            coreSourceCatalog->functions[4].id != "orbit_stripe" ||
            coreSourceCatalog->functions[5].id != "root_proximity") {
            std::cerr << "Expected the extracted advanced color core to widen the shipped Source catalog through runtime-real source rows\n";
            return 1;
        }
        const FunctionDescriptor* coreEscapeMagnitudeDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreSourceCatalog, "escape_magnitude");
        if (!coreEscapeMagnitudeDescriptor ||
            coreEscapeMagnitudeDescriptor->parameters.size() != 2 ||
            coreEscapeMagnitudeDescriptor->parameters[0].path != "signal.magnitude_scale" ||
            coreEscapeMagnitudeDescriptor->parameters[1].path != "signal.magnitude_bias") {
            std::cerr << "Expected escape_magnitude to carry stable magnitude-scale and magnitude-bias source parameters\n";
            return 1;
        }
        const FunctionDescriptor* coreOrbitStripeDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreSourceCatalog, "orbit_stripe");
        if (!coreOrbitStripeDescriptor ||
            coreOrbitStripeDescriptor->parameters.size() != 2 ||
            coreOrbitStripeDescriptor->parameters[0].path != "signal.stripe_frequency" ||
            coreOrbitStripeDescriptor->parameters[1].path != "signal.phase_offset") {
            std::cerr << "Expected orbit_stripe to carry stable stripe-frequency and phase-offset source parameters\n";
            return 1;
        }
        const FunctionDescriptor* coreRootProximityDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreSourceCatalog, "root_proximity");
        if (!coreRootProximityDescriptor ||
            coreRootProximityDescriptor->parameters.size() != 2 ||
            coreRootProximityDescriptor->parameters[0].path != "signal.proximity_scale" ||
            coreRootProximityDescriptor->parameters[1].path != "signal.proximity_bias") {
            std::cerr << "Expected root_proximity to carry stable proximity-scale and proximity-bias source parameters\n";
            return 1;
        }
        const ColorPipelineLaneCatalog* corePaletteCatalog = color_pipeline_core::FindColorPipelineLaneCatalog("palette");
        if (!corePaletteCatalog ||
            corePaletteCatalog->default_function_id != std::string("heatmap") ||
            corePaletteCatalog->functions.size() != 4 ||
            corePaletteCatalog->functions[0].id != "heatmap" ||
            corePaletteCatalog->functions[1].id != "phase_wheel_palette" ||
            corePaletteCatalog->functions[2].id != "banded_heatmap" ||
            corePaletteCatalog->functions[3].id != "explaino_cmap") {
            std::cerr << "Expected the extracted advanced color core to widen the shipped Palette catalog with explaino_cmap as the next runtime-real row\n";
            return 1;
        }
        const FunctionDescriptor* coreExplainoCmapDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*corePaletteCatalog, "explaino_cmap");
        if (!coreExplainoCmapDescriptor ||
            coreExplainoCmapDescriptor->parameters.size() != 3 ||
            coreExplainoCmapDescriptor->parameters[0].path != "palette.seed_scale" ||
            coreExplainoCmapDescriptor->parameters[1].path != "palette.seed_phase" ||
            coreExplainoCmapDescriptor->parameters[2].path != "palette.colorfulness") {
            std::cerr << "Expected explaino_cmap to expose stable seed-scale, seed-phase, and colorfulness parameter paths\n";
            return 1;
        }
        const ColorPipelineLaneCatalog* coreShapeCatalog = color_pipeline_core::FindColorPipelineLaneCatalog("shape");
        if (!coreShapeCatalog ||
            coreShapeCatalog->default_function_id != std::string("identity") ||
            coreShapeCatalog->functions.size() != 7 ||
            coreShapeCatalog->functions[0].id != "identity" ||
            coreShapeCatalog->functions[1].id != "offset_scale" ||
            coreShapeCatalog->functions[2].id != "repeat" ||
            coreShapeCatalog->functions[3].id != "posterize" ||
            coreShapeCatalog->functions[4].id != "mirror_repeat" ||
            coreShapeCatalog->functions[5].id != "bias_gain_curve" ||
            coreShapeCatalog->functions[6].id != "smooth_window") {
            std::cerr << "Expected the extracted advanced color core to widen the shipped Shape catalog with smooth_window as the final runtime-real row\n";
            return 1;
        }
        const FunctionDescriptor* coreRepeatDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreShapeCatalog, "repeat");
        if (!coreRepeatDescriptor ||
            coreRepeatDescriptor->parameters.size() != 2 ||
            coreRepeatDescriptor->parameters[0].path != "shape.frequency" ||
            coreRepeatDescriptor->parameters[1].path != "shape.phase") {
            std::cerr << "Expected the extracted advanced color core to preserve repeat parameter ordering and meaning\n";
            return 1;
        }
        const FunctionDescriptor* corePosterizeDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreShapeCatalog, "posterize");
        if (!corePosterizeDescriptor ||
            corePosterizeDescriptor->parameters.size() != 2 ||
            corePosterizeDescriptor->parameters[0].path != "shape.steps" ||
            corePosterizeDescriptor->parameters[1].path != "shape.mix") {
            std::cerr << "Expected posterize to expose stable steps and mix parameter paths\n";
            return 1;
        }
        const FunctionDescriptor* coreMirrorRepeatDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreShapeCatalog, "mirror_repeat");
        if (!coreMirrorRepeatDescriptor ||
            coreMirrorRepeatDescriptor->parameters.size() != 2 ||
            coreMirrorRepeatDescriptor->parameters[0].path != "shape.frequency" ||
            coreMirrorRepeatDescriptor->parameters[1].path != "shape.phase") {
            std::cerr << "Expected mirror_repeat to reuse the stable repeat frequency and phase parameter paths\n";
            return 1;
        }
        const FunctionDescriptor* coreBiasGainDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreShapeCatalog, "bias_gain_curve");
        if (!coreBiasGainDescriptor ||
            coreBiasGainDescriptor->parameters.size() != 2 ||
            coreBiasGainDescriptor->parameters[0].path != "shape.bias" ||
            coreBiasGainDescriptor->parameters[1].path != "shape.gain") {
            std::cerr << "Expected bias_gain_curve to expose stable bias and gain parameter paths\n";
            return 1;
        }
        const FunctionDescriptor* coreSmoothWindowDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreShapeCatalog, "smooth_window");
        if (!coreSmoothWindowDescriptor ||
            coreSmoothWindowDescriptor->parameters.size() != 3 ||
            coreSmoothWindowDescriptor->parameters[0].path != "shape.center" ||
            coreSmoothWindowDescriptor->parameters[1].path != "shape.width" ||
            coreSmoothWindowDescriptor->parameters[2].path != "shape.softness") {
            std::cerr << "Expected smooth_window to expose stable center, width, and softness parameter paths\n";
            return 1;
        }
        const char* bridgeSourceFunctionId = nullptr;
        const char* bridgePaletteFunctionId = nullptr;
        const ColorPipelineSelection bandsPipeline = {
            ColorSignal::iteration_bands,
            ColorPalette::banded_escape,
            ColorGradingPreset::bands_default,
        };
        if (!color_pipeline_core::TryBuildColorPipelineScheduleBridgeIds(
                bandsPipeline,
                &bridgeSourceFunctionId,
                &bridgePaletteFunctionId) ||
            std::string(bridgeSourceFunctionId ? bridgeSourceFunctionId : "") != "banded_signal" ||
            std::string(bridgePaletteFunctionId ? bridgePaletteFunctionId : "") != "banded_heatmap") {
            std::cerr << "Expected the extracted advanced color core to preserve the shipped schedule bridge ids for runtime-backed tuples\n";
            return 1;
        }
        const ColorPipelineSelection explainoPipeline = {
            ColorSignal::smooth_escape,
            ColorPalette::explaino_cmap,
            ColorGradingPreset::escape_default,
        };
        if (!color_pipeline_core::TryBuildColorPipelineScheduleBridgeIds(
                explainoPipeline,
                &bridgeSourceFunctionId,
                &bridgePaletteFunctionId) ||
            std::string(bridgeSourceFunctionId ? bridgeSourceFunctionId : "") != "smooth_escape_ramp" ||
            std::string(bridgePaletteFunctionId ? bridgePaletteFunctionId : "") != "explaino_cmap") {
            std::cerr << "Expected the extracted advanced color core to bridge explaino_cmap through the smooth-escape runtime tuple\n";
            return 1;
        }

        ViewState view{};
        KernelParams params{};
        ColorPipelineWindowState windowState{};
        if (windowState.open) {
            std::cerr << "Expected advanced color pipeline windows to start closed\n";
            return 1;
        }
        OpenColorPipelineWindow(&windowState);
        if (!windowState.open) {
            std::cerr << "Expected the advanced color pipeline helper to open the window state\n";
            return 1;
        }
        if (!EnsureColorPipelineWindowInitialized(&windowState)) {
            std::cerr << "Expected the advanced color pipeline window to initialize a fixed draft editor state\n";
            return 1;
        }
        if (!windowState.initialized || windowState.lanes.size() != 3 || windowState.next_row_id != 4) {
            std::cerr << "Expected the advanced color pipeline window to initialize exactly three schedule lanes with stable row ids\n";
            return 1;
        }
        if (windowState.lanes[0].label != "Source" ||
            windowState.lanes[0].rows.size() != 1 ||
            windowState.lanes[0].rows[0].ui_row_id != 1 ||
            windowState.lanes[1].label != "Shape" ||
            windowState.lanes[1].rows.size() != 1 ||
            windowState.lanes[1].rows[0].ui_row_id != 2 ||
            windowState.lanes[2].label != "Palette" ||
            windowState.lanes[2].rows.size() != 1 ||
            windowState.lanes[2].rows[0].ui_row_id != 3) {
            std::cerr << "Expected the advanced color pipeline draft lanes to keep deterministic Source / Shape / Palette starter rows\n";
            return 1;
        }
        if (windowState.lanes[0].rows[0].function_id != "smooth_escape_ramp" ||
            windowState.lanes[1].rows[0].function_id != "identity" ||
            windowState.lanes[2].rows[0].function_id != "heatmap") {
            std::cerr << "Expected the advanced color pipeline draft editor to start from Source / Shape / Palette starter rows instead of a fixed legacy trio\n";
            return 1;
        }

        std::vector<std::size_t> visibleParamIndexes;
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[0].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 2 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1) {
            std::cerr << "Expected the shipped smooth_escape_ramp signal to expose its runtime-backed parameter controls\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[1].rows[0], &visibleParamIndexes) ||
            !visibleParamIndexes.empty()) {
            std::cerr << "Expected the starter Identity shape row to stay out of the current live-runtime-backed control surface\n";
            return 1;
        }
        const ColorPipelineLaneCatalog* shapeCatalog = FindColorPipelineLaneCatalog("shape");
        if (!shapeCatalog ||
            shapeCatalog->functions.size() != 7 ||
            shapeCatalog->functions[0].id != "identity" ||
            shapeCatalog->functions[1].id != "offset_scale" ||
            shapeCatalog->functions[2].id != "repeat" ||
            shapeCatalog->functions[3].id != "posterize" ||
            shapeCatalog->functions[4].id != "mirror_repeat" ||
            shapeCatalog->functions[5].id != "bias_gain_curve" ||
            shapeCatalog->functions[6].id != "smooth_window") {
            std::cerr << "Expected the shipped Shape catalog to expose Identity plus the real offset_scale, repeat, posterize, mirror_repeat, bias_gain_curve, and smooth_window rows\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 1, "offset_scale") ||
            windowState.lanes[1].rows[0].parameter_values.size() != 2 ||
            windowState.lanes[1].rows[0].parameter_values[0].path != "shape.offset" ||
            !CollectRenderableColorPipelineParamIndexes(windowState.lanes[1].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 2 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1) {
            std::cerr << "Expected offset_scale to expose its runtime-backed Shape parameter controls\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 1, "identity")) {
            std::cerr << "Expected the Shape lane to switch back to Identity after offset_scale coverage\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 1, "repeat")) {
            std::cerr << "Expected the shipped Shape lane to allow selecting repeat once it is runtime-backed\n";
            return 1;
        }
        if (windowState.lanes[1].rows[0].parameter_values.size() != 2) {
            std::cerr << "Expected repeat to initialize two Shape parameters\n";
            return 1;
        }
        if (windowState.lanes[1].rows[0].parameter_values[0].path != "shape.frequency" ||
            windowState.lanes[1].rows[0].parameter_values[1].path != "shape.phase") {
            std::cerr << "Expected repeat to expose frequency and phase parameter paths in order\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[1].rows[0], &visibleParamIndexes)) {
            std::cerr << "Expected repeat renderability collection to succeed\n";
            return 1;
        }
        if (visibleParamIndexes.size() != 2 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1) {
            std::cerr << "Expected repeat to mark both Shape parameters as live-renderable\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 1, "identity")) {
            std::cerr << "Expected the Shape lane to switch back to Identity after repeat coverage\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[2].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 2 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1) {
            std::cerr << "Expected the shipped heatmap palette to expose its runtime-backed parameter controls\n";
            return 1;
        }
        const ColorPipelineLaneCatalog* paletteCatalog = FindColorPipelineLaneCatalog("palette");
        if (!paletteCatalog ||
            paletteCatalog->functions.size() != 4 ||
            paletteCatalog->functions[0].id != "heatmap" ||
            paletteCatalog->functions[1].id != "phase_wheel_palette" ||
            paletteCatalog->functions[2].id != "banded_heatmap" ||
            paletteCatalog->functions[3].id != "explaino_cmap") {
            std::cerr << "Expected the shipped Palette catalog to expose heatmap, phase_wheel_palette, banded_heatmap, and explaino_cmap\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 2, "explaino_cmap") ||
            windowState.lanes[2].rows[0].parameter_values.size() != 3 ||
            windowState.lanes[2].rows[0].parameter_values[0].path != "palette.seed_scale" ||
            windowState.lanes[2].rows[0].parameter_values[1].path != "palette.seed_phase" ||
            windowState.lanes[2].rows[0].parameter_values[2].path != "palette.colorfulness") {
            std::cerr << "Expected the shipped Palette lane to accept explaino_cmap once its runtime backend exists\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[2].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 3 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1 ||
            visibleParamIndexes[2] != 2) {
            std::cerr << "Expected explaino_cmap to expose its three live Palette controls\n";
            return 1;
        }
        if (!AddColorPipelineLaneRow(&windowState, 2, "explaino_cmap") ||
            windowState.lanes[2].rows.size() != 2 ||
            windowState.lanes[2].rows[1].function_id != "explaino_cmap") {
            std::cerr << "Expected the schedule-style Palette lane to support appending explaino_cmap rows once runtime-backed\n";
            return 1;
        }
        if (!RemoveColorPipelineLaneRow(&windowState, 2, 1) ||
            windowState.lanes[2].rows.size() != 1 ||
            windowState.lanes[2].rows[0].function_id != "explaino_cmap") {
            std::cerr << "Expected explaino_cmap rows to participate in the same schedule-style Palette row editing surface\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 2, "heatmap")) {
            std::cerr << "Expected the Palette lane to switch back to heatmap after explaino_cmap coverage\n";
            return 1;
        }

        if (SelectColorPipelineLaneFunction(&windowState, 0, "phase_angle")) {
            std::cerr << "Expected the raw legacy enum ids to stop working as advanced color function ids\n";
            return 1;
        }
        if (SelectColorPipelineLaneFunction(&windowState, 0, "basin_index") ||
            SelectColorPipelineLaneFunction(&windowState, 1, "classic_basin_palette") ||
            SelectColorPipelineLaneFunction(&windowState, 2, "phase_finish") ||
            SelectColorPipelineLaneFunction(&windowState, 2, "basin_balance")) {
            std::cerr << "Expected the advanced color pipeline catalog to stop shipping the legacy simple-panel tuple pieces\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 0, "banded_signal")) {
            std::cerr << "Expected advanced color pipeline lane function selection to accept known lane-local functions\n";
            return 1;
        }
        if (windowState.lanes[0].rows[0].function_id != "banded_signal" ||
            windowState.lanes[0].rows[0].parameter_values.size() != 2 ||
            windowState.lanes[0].rows[0].parameter_values[0].path != "signal.band_count") {
            std::cerr << "Expected advanced color pipeline lane edits to swap the live-backed descriptor parameter surface\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[0].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 2 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1) {
            std::cerr << "Expected runtime-backed advanced color functions to expose only their real renderable parameter controls\n";
            return 1;
        }
        if (!AddColorPipelineLaneRow(&windowState, 1, "offset_scale") ||
            windowState.lanes[1].rows.size() != 2 ||
            windowState.lanes[1].rows[1].function_id != "offset_scale" ||
            windowState.lanes[1].rows[1].ui_row_id != 5) {
            std::cerr << "Expected the schedule-style Shape lane to support appending a second shipped row with a stable row id\n";
            return 1;
        }
        if (!MoveColorPipelineLaneRow(&windowState, 1, 1, -1) ||
            windowState.lanes[1].rows[0].function_id != "offset_scale" ||
            windowState.lanes[1].rows[1].function_id != "identity") {
            std::cerr << "Expected schedule-style lane rows to support reorder operations\n";
            return 1;
        }
        if (!RemoveColorPipelineLaneRow(&windowState, 1, 1) ||
            windowState.lanes[1].rows.size() != 1 ||
            windowState.lanes[1].rows[0].function_id != "offset_scale") {
            std::cerr << "Expected schedule-style lane rows to support removing non-last rows\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 1, "repeat") ||
            !AddColorPipelineLaneRow(&windowState, 1, "repeat")) {
            std::cerr << "Expected the shipped Shape lane to accept repeat once the runtime backend supports it\n";
            return 1;
        }
        if (!RemoveColorPipelineLaneRow(&windowState, 1, 1) ||
            windowState.lanes[1].rows.size() != 1 ||
            windowState.lanes[1].rows[0].function_id != "repeat") {
            std::cerr << "Expected repeat rows to participate in the same schedule-style Shape row editing surface\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 1, "posterize") ||
            windowState.lanes[1].rows[0].parameter_values.size() != 2 ||
            windowState.lanes[1].rows[0].parameter_values[0].path != "shape.steps" ||
            windowState.lanes[1].rows[0].parameter_values[1].path != "shape.mix") {
            std::cerr << "Expected the shipped Shape lane to accept posterize once its runtime backend exists\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[1].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 2 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1) {
            std::cerr << "Expected posterize to expose both live Shape controls\n";
            return 1;
        }
        if (!AddColorPipelineLaneRow(&windowState, 1, "posterize") ||
            windowState.lanes[1].rows.size() != 2 ||
            windowState.lanes[1].rows[1].function_id != "posterize") {
            std::cerr << "Expected the schedule-style Shape lane to support appending posterize rows once runtime-backed\n";
            return 1;
        }
        if (!RemoveColorPipelineLaneRow(&windowState, 1, 1) ||
            windowState.lanes[1].rows.size() != 1 ||
            windowState.lanes[1].rows[0].function_id != "posterize") {
            std::cerr << "Expected posterize rows to participate in the same schedule-style Shape row editing surface\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 1, "mirror_repeat") ||
            windowState.lanes[1].rows[0].parameter_values.size() != 2 ||
            windowState.lanes[1].rows[0].parameter_values[0].path != "shape.frequency" ||
            windowState.lanes[1].rows[0].parameter_values[1].path != "shape.phase") {
            std::cerr << "Expected the shipped Shape lane to accept mirror_repeat once its runtime backend exists\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[1].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 2 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1) {
            std::cerr << "Expected mirror_repeat to expose the reused repeat Shape controls\n";
            return 1;
        }
        if (!AddColorPipelineLaneRow(&windowState, 1, "mirror_repeat") ||
            windowState.lanes[1].rows.size() != 2 ||
            windowState.lanes[1].rows[1].function_id != "mirror_repeat") {
            std::cerr << "Expected the schedule-style Shape lane to support appending mirror_repeat rows once runtime-backed\n";
            return 1;
        }
        if (!RemoveColorPipelineLaneRow(&windowState, 1, 1) ||
            windowState.lanes[1].rows.size() != 1 ||
            windowState.lanes[1].rows[0].function_id != "mirror_repeat") {
            std::cerr << "Expected mirror_repeat rows to participate in the same schedule-style Shape row editing surface\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 1, "bias_gain_curve") ||
            windowState.lanes[1].rows[0].parameter_values.size() != 2 ||
            windowState.lanes[1].rows[0].parameter_values[0].path != "shape.bias" ||
            windowState.lanes[1].rows[0].parameter_values[1].path != "shape.gain") {
            std::cerr << "Expected the shipped Shape lane to accept bias_gain_curve once its runtime backend exists\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[1].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 2 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1) {
            std::cerr << "Expected bias_gain_curve to expose both live Shape controls\n";
            return 1;
        }
        if (!AddColorPipelineLaneRow(&windowState, 1, "bias_gain_curve") ||
            windowState.lanes[1].rows.size() != 2 ||
            windowState.lanes[1].rows[1].function_id != "bias_gain_curve") {
            std::cerr << "Expected the schedule-style Shape lane to support appending bias_gain_curve rows once runtime-backed\n";
            return 1;
        }
        if (!RemoveColorPipelineLaneRow(&windowState, 1, 1) ||
            windowState.lanes[1].rows.size() != 1 ||
            windowState.lanes[1].rows[0].function_id != "bias_gain_curve") {
            std::cerr << "Expected bias_gain_curve rows to participate in the same schedule-style Shape row editing surface\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 1, "smooth_window") ||
            windowState.lanes[1].rows[0].parameter_values.size() != 3 ||
            windowState.lanes[1].rows[0].parameter_values[0].path != "shape.center" ||
            windowState.lanes[1].rows[0].parameter_values[1].path != "shape.width" ||
            windowState.lanes[1].rows[0].parameter_values[2].path != "shape.softness") {
            std::cerr << "Expected the shipped Shape lane to accept smooth_window once its runtime backend exists\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[1].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 3 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1 ||
            visibleParamIndexes[2] != 2) {
            std::cerr << "Expected smooth_window to expose its three live Shape controls\n";
            return 1;
        }
        if (!AddColorPipelineLaneRow(&windowState, 1, "smooth_window") ||
            windowState.lanes[1].rows.size() != 2 ||
            windowState.lanes[1].rows[1].function_id != "smooth_window") {
            std::cerr << "Expected the schedule-style Shape lane to support appending smooth_window rows once runtime-backed\n";
            return 1;
        }
        if (!RemoveColorPipelineLaneRow(&windowState, 1, 1) ||
            windowState.lanes[1].rows.size() != 1 ||
            windowState.lanes[1].rows[0].function_id != "smooth_window") {
            std::cerr << "Expected smooth_window rows to participate in the same schedule-style Shape row editing surface\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 1, "identity") ||
            windowState.lanes[1].rows[0].function_id != "identity") {
            std::cerr << "Expected the Shape lane to keep accepting lane-local function changes after reorder/remove coverage\n";
            return 1;
        }
        if (SelectColorPipelineLaneFunction(&windowState, 0, "not_real")) {
            std::cerr << "Unknown advanced color lane functions should fail instead of silently falling back\n";
            return 1;
        }
        if (windowState.lanes[0].rows[0].function_id != "banded_signal") {
            std::cerr << "Failed advanced color lane selections should preserve the current function choice\n";
            return 1;
        }

        params.coloring_mode = ColoringMode::root_basin;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::root_basin);
        if (!SyncColorPipelineWindowFromLiveState(&windowState, view.fractal_type, &params)) {
            std::cerr << "Expected unsupported live tuples to keep the schedule editor on its current draft instead of failing sync\n";
            return 1;
        }
        if (!windowState.live_snapshot.valid || windowState.live_snapshot.draft_import_supported) {
            std::cerr << "Expected root-basin live tuples to stay outside the current Source / Shape / Palette bridge\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 0, "phase_orbit") ||
            !SelectColorPipelineLaneFunction(&windowState, 2, "phase_wheel_palette") ||
            !HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected the draft editor to diverge while the live runtime is outside the current bridge\n";
            return 1;
        }

        params.coloring_mode = ColoringMode::iteration_bands;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::iteration_bands);
        if (!SyncColorPipelineWindowFromLiveState(&windowState, view.fractal_type, &params)) {
            std::cerr << "Expected supported live tuples to resync the snapshot without clobbering an already-diverged draft\n";
            return 1;
        }
        if (!windowState.live_snapshot.valid ||
            !windowState.live_snapshot.draft_import_supported ||
            windowState.live_snapshot.lanes.size() != 3 ||
            windowState.live_snapshot.lanes[0].rows[0].function_id != "banded_signal" ||
            windowState.live_snapshot.lanes[1].rows[0].function_id != "identity" ||
            windowState.live_snapshot.lanes[2].rows[0].function_id != "banded_heatmap") {
            std::cerr << "Expected the live snapshot to refresh to the newly supported runtime tuple\n";
            return 1;
        }
        if (windowState.lanes[0].rows[0].function_id != "phase_orbit" ||
            windowState.lanes[2].rows[0].function_id != "phase_wheel_palette" ||
            !HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected a diverged draft to survive when the live tuple becomes bridge-supported later\n";
            return 1;
        }

        ColorPipelineWindowState stableSyncWindowState{};
        params.coloring_mode = ColoringMode::phase;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::phase);
        params.color_shape = ColorPipelineShape::offset_scale;
        params.color_shape_offset = 0.25f;
        params.color_shape_scale = 1.5f;
        if (!SyncColorPipelineWindowFromLiveState(&stableSyncWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected the advanced color pipeline draft to import a supported live tuple before row-id stability coverage\n";
            return 1;
        }
        const std::uint64_t stableSourceRowId = stableSyncWindowState.lanes[0].rows[0].ui_row_id;
        const std::uint64_t stableShapeRowId = stableSyncWindowState.lanes[1].rows[0].ui_row_id;
        const std::uint64_t stablePaletteRowId = stableSyncWindowState.lanes[2].rows[0].ui_row_id;
        if (!SyncColorPipelineWindowFromLiveState(&stableSyncWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected unchanged supported live tuples to keep syncing successfully during row-id stability coverage\n";
            return 1;
        }
        if (stableSyncWindowState.lanes[0].rows[0].ui_row_id != stableSourceRowId ||
            stableSyncWindowState.lanes[1].rows[0].ui_row_id != stableShapeRowId ||
            stableSyncWindowState.lanes[2].rows[0].ui_row_id != stablePaletteRowId) {
            std::cerr << "Expected unchanged supported live sync to preserve row ids instead of rebuilding the programmable draft between frames\n";
            return 1;
        }

        ColorPipelineWindowState explainoSyncWindowState{};
        params.coloring_mode = ColoringMode::smooth_escape;
        params.color_pipeline = {ColorSignal::smooth_escape, ColorPalette::explaino_cmap, ColorGradingPreset::escape_default};
        params.color_shape = ColorPipelineShape::identity;
        params.color_shape_offset = 0.0f;
        params.color_shape_scale = 1.0f;
        params.color_explaino_palette_seed_scale = 1.5f;
        params.color_explaino_palette_seed_phase = 0.25f;
        params.color_explaino_palette_colorfulness = 0.8f;
        if (!SyncColorPipelineWindowFromLiveState(&explainoSyncWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected the advanced color pipeline draft to import the explaino_cmap runtime tuple once it is supported\n";
            return 1;
        }
        if (!explainoSyncWindowState.live_snapshot.valid ||
            !explainoSyncWindowState.live_snapshot.draft_import_supported ||
            explainoSyncWindowState.live_snapshot.lanes[2].rows[0].function_id != "explaino_cmap" ||
            explainoSyncWindowState.live_snapshot.lanes[2].rows[0].parameter_values.size() != 3 ||
            explainoSyncWindowState.live_snapshot.lanes[2].rows[0].parameter_values[0].path != "palette.seed_scale") {
            std::cerr << "Expected explaino_cmap live sync to import the dedicated Palette owner fields\n";
            return 1;
        }

        ColorPipelineWindowState invalidLiveWindowState{};
        params.coloring_mode = ColoringMode::phase;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
        params.color_shape = ColorPipelineShape::identity;
        params.color_shape_offset = 0.0f;
        params.color_shape_scale = 1.0f;
        if (SyncColorPipelineWindowFromLiveState(&invalidLiveWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected out-of-sync live color state to fail snapshot import instead of pretending it was valid\n";
            return 1;
        }
        if (invalidLiveWindowState.live_snapshot.valid) {
            std::cerr << "Expected invalid live color state to leave the live snapshot unavailable for import\n";
            return 1;
        }
        const ColorPipelineDraftApplyState invalidLiveApplyState = DescribeColorPipelineDraftApplyState(
            invalidLiveWindowState,
            view.fractal_type,
            &params);
        if (invalidLiveApplyState.status != ColorPipelineDraftApplyStatus::can_apply) {
            std::cerr << "Expected the default supported draft to remain applicable so the advanced window can repair an invalid live color state\n";
            return 1;
        }
        ColorPipelineRenderInteractionState invalidLiveInteractionState{};
        if (ShouldAutoApplySupportedColorPipelineDraft(
                invalidLiveWindowState,
                invalidLiveApplyState,
                invalidLiveInteractionState,
                &params)) {
            std::cerr << "Expected supported auto-apply to stay dormant until the user actually interacts with the advanced color window\n";
            return 1;
        }
        invalidLiveInteractionState.interacted = true;
        if (!ShouldAutoApplySupportedColorPipelineDraft(
                invalidLiveWindowState,
                invalidLiveApplyState,
                invalidLiveInteractionState,
                &params)) {
            std::cerr << "Expected supported auto-apply to become eligible once the user has interacted with the advanced color window\n";
            return 1;
        }
        if (!ApplyColorPipelineDraftToLiveState(&invalidLiveWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected the supported draft to repair an invalid live color state\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::smooth_escape ||
            params.color_pipeline.signal != ColorSignal::smooth_escape ||
            params.color_pipeline.palette != ColorPalette::cyclic_escape ||
            params.color_pipeline.grading != ColorGradingPreset::escape_default ||
            !invalidLiveWindowState.live_snapshot.valid ||
            !invalidLiveWindowState.live_snapshot.draft_import_supported) {
            std::cerr << "Expected invalid live recovery to repair the runtime tuple and restore an importable live snapshot\n";
            return 1;
        }
    }

    {
        ImGuiTestContext imgui;
        ViewState view{};
        KernelParams params{};
        ColorPipelineWindowState windowState{};
        params.coloring_mode = ColoringMode::phase;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::phase);
        params.color_shape = ColorPipelineShape::offset_scale;
        params.color_shape_offset = 0.25f;
        params.color_shape_scale = 1.5f;
        if (!EnsureColorPipelineWindowInitialized(&windowState)) {
            std::cerr << "Expected the advanced color pipeline draft editor to initialize before param mutation coverage\n";
            return 1;
        }
        auto setParam = [](ColorPipelineRowState& row, const char* path, double value) {
            for (ColorPipelineParamState& param : row.parameter_values) {
                if (param.path == path) {
                    param.number_value = value;
                    return true;
                }
            }
            return false;
        };
        std::vector<std::size_t> visibleParamIndexes;
        if (!SelectColorPipelineLaneFunction(&windowState, 0, "phase_orbit") ||
            !SelectColorPipelineLaneFunction(&windowState, 1, "repeat") ||
            !SelectColorPipelineLaneFunction(&windowState, 2, "phase_wheel_palette")) {
            std::cerr << "Expected the live programmable editor RED to construct a legal phase tuple with the repeat Shape row\n";
            return 1;
        }
        if (!setParam(windowState.lanes[0].rows[0], "signal.phase_offset", 1.25) ||
            !setParam(windowState.lanes[0].rows[0], "signal.wrap_cycles", 2.5) ||
            !setParam(windowState.lanes[1].rows[0], "shape.frequency", 6.0) ||
            !setParam(windowState.lanes[1].rows[0], "shape.phase", 0.2) ||
            !setParam(windowState.lanes[2].rows[0], "palette.phase_offset", -0.75)) {
            std::cerr << "Expected the live programmable editor RED to expose the current phase and Shape parameter controls\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[1].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 2 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1) {
            std::cerr << "Expected offset_scale to expose only its real runtime-backed Shape controls\n";
            return 1;
        }
        if (!ApplyColorPipelineDraftToLiveState(&windowState, view.fractal_type, &params)) {
            std::cerr << "Expected the live programmable editor RED to apply the current phase tuple\n";
            return 1;
        }
        if (!NearlyEqual(params.color_phase_signal_offset, 1.25) ||
            !NearlyEqual(params.color_phase_wrap_cycles, 2.5) ||
            params.color_shape != ColorPipelineShape::repeat ||
            !NearlyEqual(params.color_shape_repeat_frequency, 6.0) ||
            !NearlyEqual(params.color_shape_repeat_phase, 0.2) ||
            !NearlyEqual(params.color_shape_offset, 0.0) ||
            !NearlyEqual(params.color_shape_scale, 1.0) ||
            !NearlyEqual(params.color_phase_palette_offset, -0.75) ||
            !windowState.live_snapshot.valid ||
            windowState.live_snapshot.lanes[1].rows[0].function_id != "repeat" ||
            HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected live programmable apply to write the phase plus Shape owner fields and resync the live snapshot\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 1, "posterize") ||
            !setParam(windowState.lanes[1].rows[0], "shape.steps", 5.0) ||
            !setParam(windowState.lanes[1].rows[0], "shape.mix", 0.65)) {
            std::cerr << "Expected the live programmable editor to expose the posterize Shape controls once runtime-backed\n";
            return 1;
        }
        if (!ApplyColorPipelineDraftToLiveState(&windowState, view.fractal_type, &params)) {
            std::cerr << "Expected the live programmable editor to apply the posterize Shape tuple\n";
            return 1;
        }
        if (params.color_shape != ColorPipelineShape::posterize ||
            params.color_shape_posterize_steps != 5 ||
            !NearlyEqual(params.color_shape_posterize_mix, 0.65) ||
            !NearlyEqual(params.color_shape_offset, 0.0) ||
            !NearlyEqual(params.color_shape_scale, 1.0) ||
            !NearlyEqual(params.color_shape_repeat_frequency, 8.0) ||
            !NearlyEqual(params.color_shape_repeat_phase, 0.0) ||
            !windowState.live_snapshot.valid ||
            windowState.live_snapshot.lanes[1].rows[0].function_id != "posterize" ||
            HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected live programmable apply to write posterize owner fields, reset other Shape owners, and resync the live snapshot\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 1, "mirror_repeat") ||
            !setParam(windowState.lanes[1].rows[0], "shape.frequency", 3.0) ||
            !setParam(windowState.lanes[1].rows[0], "shape.phase", 0.15)) {
            std::cerr << "Expected the live programmable editor to expose the mirror_repeat Shape controls once runtime-backed\n";
            return 1;
        }
        if (!ApplyColorPipelineDraftToLiveState(&windowState, view.fractal_type, &params)) {
            std::cerr << "Expected the live programmable editor to apply the mirror_repeat Shape tuple\n";
            return 1;
        }
        if (params.color_shape != ColorPipelineShape::mirror_repeat ||
            !NearlyEqual(params.color_shape_repeat_frequency, 3.0) ||
            !NearlyEqual(params.color_shape_repeat_phase, 0.15) ||
            !NearlyEqual(params.color_shape_offset, 0.0) ||
            !NearlyEqual(params.color_shape_scale, 1.0) ||
            !windowState.live_snapshot.valid ||
            windowState.live_snapshot.lanes[1].rows[0].function_id != "mirror_repeat" ||
            HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected live programmable apply to write the mirror_repeat Shape choice through the reused repeat owner fields and resync the live snapshot\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 1, "bias_gain_curve") ||
            !setParam(windowState.lanes[1].rows[0], "shape.bias", 0.25) ||
            !setParam(windowState.lanes[1].rows[0], "shape.gain", 0.75)) {
            std::cerr << "Expected the live programmable editor to expose the bias_gain_curve Shape controls once runtime-backed\n";
            return 1;
        }
        if (!ApplyColorPipelineDraftToLiveState(&windowState, view.fractal_type, &params)) {
            std::cerr << "Expected the live programmable editor to apply the bias_gain_curve Shape tuple\n";
            return 1;
        }
        if (params.color_shape != ColorPipelineShape::bias_gain_curve ||
            !NearlyEqual(params.color_shape_bias, 0.25) ||
            !NearlyEqual(params.color_shape_gain, 0.75) ||
            !NearlyEqual(params.color_shape_offset, 0.0) ||
            !NearlyEqual(params.color_shape_scale, 1.0) ||
            !NearlyEqual(params.color_shape_repeat_frequency, 8.0) ||
            !NearlyEqual(params.color_shape_repeat_phase, 0.0) ||
            params.color_shape_posterize_steps != 6 ||
            !NearlyEqual(params.color_shape_posterize_mix, 1.0) ||
            !windowState.live_snapshot.valid ||
            windowState.live_snapshot.lanes[1].rows[0].function_id != "bias_gain_curve" ||
            HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected live programmable apply to write the bias_gain_curve owner fields, reset other Shape owners, and resync the live snapshot\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 1, "smooth_window") ||
            !setParam(windowState.lanes[1].rows[0], "shape.center", 0.35) ||
            !setParam(windowState.lanes[1].rows[0], "shape.width", 0.4) ||
            !setParam(windowState.lanes[1].rows[0], "shape.softness", 0.05)) {
            std::cerr << "Expected the live programmable editor to expose the smooth_window Shape controls once runtime-backed\n";
            return 1;
        }
        if (!ApplyColorPipelineDraftToLiveState(&windowState, view.fractal_type, &params)) {
            std::cerr << "Expected the live programmable editor to apply the smooth_window Shape tuple\n";
            return 1;
        }
        if (params.color_shape != ColorPipelineShape::smooth_window ||
            !NearlyEqual(params.color_shape_window_center, 0.35) ||
            !NearlyEqual(params.color_shape_window_width, 0.4) ||
            !NearlyEqual(params.color_shape_window_softness, 0.05) ||
            !NearlyEqual(params.color_shape_offset, 0.0) ||
            !NearlyEqual(params.color_shape_scale, 1.0) ||
            !NearlyEqual(params.color_shape_repeat_frequency, 8.0) ||
            !NearlyEqual(params.color_shape_repeat_phase, 0.0) ||
            params.color_shape_posterize_steps != 6 ||
            !NearlyEqual(params.color_shape_posterize_mix, 1.0) ||
            !NearlyEqual(params.color_shape_bias, 0.5) ||
            !NearlyEqual(params.color_shape_gain, 0.5) ||
            !windowState.live_snapshot.valid ||
            windowState.live_snapshot.lanes[1].rows[0].function_id != "smooth_window" ||
            HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected live programmable apply to write the smooth_window owner fields, reset other Shape owners, and resync the live snapshot\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 0, "smooth_escape_ramp") ||
            !SelectColorPipelineLaneFunction(&windowState, 1, "identity") ||
            !SelectColorPipelineLaneFunction(&windowState, 2, "explaino_cmap") ||
            !setParam(windowState.lanes[2].rows[0], "palette.seed_scale", 1.5) ||
            !setParam(windowState.lanes[2].rows[0], "palette.seed_phase", 0.25) ||
            !setParam(windowState.lanes[2].rows[0], "palette.colorfulness", 0.8)) {
            std::cerr << "Expected the live programmable editor to expose the explaino_cmap Palette controls once runtime-backed\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[2].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 3 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1 ||
            visibleParamIndexes[2] != 2) {
            std::cerr << "Expected explaino_cmap to expose only its dedicated live Palette controls\n";
            return 1;
        }
        if (!ApplyColorPipelineDraftToLiveState(&windowState, view.fractal_type, &params)) {
            std::cerr << "Expected the live programmable editor to apply the explaino_cmap Palette tuple\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::smooth_escape ||
            params.color_pipeline.signal != ColorSignal::smooth_escape ||
            params.color_pipeline.palette != ColorPalette::explaino_cmap ||
            params.color_pipeline.grading != ColorGradingPreset::escape_default ||
            !NearlyEqual(params.color_explaino_palette_seed_scale, 1.5) ||
            !NearlyEqual(params.color_explaino_palette_seed_phase, 0.25) ||
            !NearlyEqual(params.color_explaino_palette_colorfulness, 0.8) ||
            !NearlyEqual(params.color_heatmap_cycle_scale, 1.0) ||
            !NearlyEqual(params.color_heatmap_saturation, 1.0) ||
            !NearlyEqual(params.color_phase_palette_offset, 0.0) ||
            !NearlyEqual(params.color_iteration_band_emphasis, 1.0) ||
            !NearlyEqual(params.color_iteration_band_palette_offset, 0.0) ||
            !windowState.live_snapshot.valid ||
            windowState.live_snapshot.lanes[2].rows[0].function_id != "explaino_cmap" ||
            HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected live programmable apply to write the explaino_cmap owner fields, reset other Palette owners, and resync the live snapshot\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 2, "phase_wheel_palette")) {
            std::cerr << "Expected the rebuilt programmable editor to construct an unsupported shipped lane mix for preview-state coverage\n";
            return 1;
        }
        const ColorPipelineDraftApplyState invalidApplyState = DescribeColorPipelineDraftApplyState(windowState, view.fractal_type, &params);
        if (invalidApplyState.status != ColorPipelineDraftApplyStatus::unsupported_tuple) {
            std::cerr << "Expected unsupported shipped lane mixes to classify as preview-only before apply\n";
            return 1;
        }
        if (!AddColorPipelineLaneRow(&windowState, 1, "identity")) {
            std::cerr << "Expected the schedule editor RED to support stacking a second enabled Shape row for validation coverage\n";
            return 1;
        }
        const ColorPipelineDraftApplyState stackedShapeState = DescribeColorPipelineDraftApplyState(windowState, view.fractal_type, &params);
        if (stackedShapeState.status != ColorPipelineDraftApplyStatus::unsupported_tuple ||
            stackedShapeState.message.find("one enabled row in the Shape lane") == std::string::npos) {
            std::cerr << "Expected invalid schedule stacks to surface the specific live-bridge reason instead of a generic preview-only message\n";
            return 1;
        }
        if (!RemoveColorPipelineLaneRow(&windowState, 1, 1)) {
            std::cerr << "Expected the schedule editor RED to restore the single-row Shape bridge after validation coverage\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 0, "phase_orbit") ||
            !SelectColorPipelineLaneFunction(&windowState, 1, "offset_scale") ||
            !SelectColorPipelineLaneFunction(&windowState, 2, "phase_wheel_palette")) {
            std::cerr << "Expected the live programmable editor to reconstruct a legal phase tuple after the preview-only check\n";
            return 1;
        }
        if (!setParam(windowState.lanes[0].rows[0], "signal.phase_offset", 0.5) ||
            !setParam(windowState.lanes[1].rows[0], "shape.scale", 2.0)) {
            std::cerr << "Expected the live programmable editor to expose phase and Shape controls for apply-state coverage\n";
            return 1;
        }
        const ColorPipelineDraftApplyState validApplyState = DescribeColorPipelineDraftApplyState(windowState, view.fractal_type, &params);
        if (validApplyState.status != ColorPipelineDraftApplyStatus::can_apply) {
            std::cerr << "Expected valid phase drafts to classify as live-applicable\n";
            return 1;
        }

        ColorPipelineRenderInteractionState interactionState{};
        if (ShouldAutoApplySupportedColorPipelineDraft(windowState, validApplyState, interactionState, &params)) {
            std::cerr << "Expected supported end-of-frame apply to stay dormant until the user interacts with the programmable controls\n";
            return 1;
        }
        interactionState.interacted = true;
        if (!ShouldAutoApplySupportedColorPipelineDraft(windowState, validApplyState, interactionState, &params)) {
            std::cerr << "Expected supported end-of-frame apply to become eligible after the user interacts with the programmable controls\n";
            return 1;
        }
        interactionState.has_active_item = true;
        if (ShouldAutoApplySupportedColorPipelineDraft(
                windowState,
                validApplyState,
                interactionState,
                &params)) {
            std::cerr << "Expected active programmable-control drags to bypass the end-of-frame apply helper so supported slider edits can use a direct live path instead\n";
            return 1;
        }
        interactionState.has_active_item = false;
    }

    {
        ImGuiTestContext imgui;
        ViewState view{};
        KernelParams params{};
        ColorPipelineWindowState windowState{};
        params.coloring_mode = ColoringMode::phase;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::phase);
        params.color_shape = ColorPipelineShape::offset_scale;
        params.color_shape_offset = 0.25f;
        params.color_shape_scale = 1.5f;

        BeginFrame();
        ColorPipelineWindowState closedWindowState{};
        if (RenderColorPipelineWindow(&closedWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected the advanced color pipeline window helper to stay hidden while closed\n";
            return 1;
        }
        EndFrame();

        BeginFrame();
        ColorPipelineWindowState openWindowState{};
        openWindowState.open = true;
        if (!RenderColorPipelineWindow(&openWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected the advanced color pipeline window helper to render once opened\n";
            return 1;
        }
        if (!openWindowState.open) {
            std::cerr << "Expected the advanced color pipeline window helper to preserve the open state without user dismissal\n";
            return 1;
        }
        if (!openWindowState.initialized || !openWindowState.live_snapshot.valid || openWindowState.lanes.size() != 3 ||
            openWindowState.lanes[0].rows.size() != 1 ||
            openWindowState.lanes[0].rows[0].function_id != "phase_orbit" ||
            openWindowState.lanes[1].rows.size() != 1 ||
            openWindowState.lanes[1].rows[0].function_id != "offset_scale" ||
            openWindowState.lanes[2].rows.size() != 1 ||
            openWindowState.lanes[2].rows[0].function_id != "phase_wheel_palette") {
            std::cerr << "Expected the advanced color pipeline window to import the live programmable tuple during render\n";
            return 1;
        }
        if (openWindowState.live_snapshot.lanes.size() != 3 ||
            openWindowState.live_snapshot.lanes[0].rows[0].function_id != "phase_orbit" ||
            openWindowState.live_snapshot.lanes[1].rows[0].function_id != "offset_scale" ||
            openWindowState.live_snapshot.lanes[2].rows[0].function_id != "phase_wheel_palette") {
            std::cerr << "Expected the advanced color pipeline window to keep the live snapshot aligned with the visible programmable tuple\n";
            return 1;
        }
        if (params.color_pipeline.signal != ColorSignal::phase_angle ||
            params.color_pipeline.palette != ColorPalette::phase_wheel ||
            params.color_pipeline.grading != ColorGradingPreset::phase_default) {
            std::cerr << "Expected rendering the live programmable window to preserve the current runtime tuple\n";
            return 1;
        }
        if (HasColorPipelineDraftEdits(openWindowState)) {
            std::cerr << "Expected the advanced color pipeline window to stay synchronized with the live programmable tuple until the user edits it\n";
            return 1;
        }
        EndFrame();

        BeginFrame();
        ColorPipelineWindowState explainoRenderWindowState{};
        explainoRenderWindowState.open = true;
        params.coloring_mode = ColoringMode::smooth_escape;
        params.color_pipeline = {ColorSignal::smooth_escape, ColorPalette::explaino_cmap, ColorGradingPreset::escape_default};
        params.color_shape = ColorPipelineShape::identity;
        params.color_shape_offset = 0.0f;
        params.color_shape_scale = 1.0f;
        params.color_explaino_palette_seed_scale = 1.5f;
        params.color_explaino_palette_seed_phase = 0.25f;
        params.color_explaino_palette_colorfulness = 0.8f;
        if (!RenderColorPipelineWindow(&explainoRenderWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected the advanced color pipeline window to render a live explaino_cmap tuple\n";
            return 1;
        }
        EndFrame();
        if (!explainoRenderWindowState.live_snapshot.valid ||
            !explainoRenderWindowState.live_snapshot.draft_import_supported ||
            explainoRenderWindowState.lanes.size() != 3 ||
            explainoRenderWindowState.lanes[2].rows.size() != 1 ||
            explainoRenderWindowState.lanes[2].rows[0].function_id != "explaino_cmap" ||
            explainoRenderWindowState.lanes[2].rows[0].parameter_values.size() != 3 ||
            !NearlyEqual(explainoRenderWindowState.lanes[2].rows[0].parameter_values[0].number_value, 1.5) ||
            !NearlyEqual(explainoRenderWindowState.lanes[2].rows[0].parameter_values[1].number_value, 0.25) ||
            !NearlyEqual(explainoRenderWindowState.lanes[2].rows[0].parameter_values[2].number_value, 0.8) ||
            params.color_pipeline.palette != ColorPalette::explaino_cmap ||
            !NearlyEqual(params.color_explaino_palette_seed_scale, 1.5) ||
            !NearlyEqual(params.color_explaino_palette_seed_phase, 0.25) ||
            !NearlyEqual(params.color_explaino_palette_colorfulness, 0.8)) {
            std::cerr << "Expected opening the advanced color pipeline window on a live explaino_cmap tuple to preserve the runtime and visible explaino controls\n";
            return 1;
        }

        BeginFrame();
        ColorPipelineWindowState unsupportedStartupWindowState{};
        unsupportedStartupWindowState.open = true;
        params = KernelParams{};
        view.fractal_type = FractalType::explaino;
        if (!RenderColorPipelineWindow(&unsupportedStartupWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected the advanced color pipeline window to render even when startup begins on an unsupported live tuple\n";
            return 1;
        }
        EndFrame();
        if (params.coloring_mode != ColoringMode::root_basin ||
            params.color_pipeline.signal != ColorSignal::root_index ||
            params.color_pipeline.palette != ColorPalette::root_classic ||
            params.color_pipeline.grading != ColorGradingPreset::basin_default ||
            !unsupportedStartupWindowState.live_snapshot.valid ||
            unsupportedStartupWindowState.live_snapshot.draft_import_supported ||
            unsupportedStartupWindowState.lanes.size() != 3 ||
            unsupportedStartupWindowState.lanes[0].rows.size() != 1 ||
            unsupportedStartupWindowState.lanes[0].rows[0].function_id != "smooth_escape_ramp" ||
            unsupportedStartupWindowState.lanes[2].rows.size() != 1 ||
            unsupportedStartupWindowState.lanes[2].rows[0].function_id != "heatmap") {
            std::cerr << "Expected opening the advanced color pipeline window from the default unsupported startup tuple to preserve the runtime until the user edits the draft\n";
            return 1;
        }

        BeginFrame();
        ColorPipelineWindowState invalidLiveRenderState{};
        invalidLiveRenderState.open = true;
        params.coloring_mode = ColoringMode::phase;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
        params.color_shape = ColorPipelineShape::identity;
        params.color_shape_offset = 0.0f;
        params.color_shape_scale = 1.0f;
        if (!RenderColorPipelineWindow(&invalidLiveRenderState, view.fractal_type, &params)) {
            std::cerr << "Expected the advanced color pipeline window to keep rendering while surfacing an invalid live color state\n";
            return 1;
        }
        EndFrame();
        if (params.coloring_mode != ColoringMode::phase ||
            params.color_pipeline.signal != ColorSignal::smooth_escape ||
            params.color_pipeline.palette != ColorPalette::cyclic_escape ||
            params.color_pipeline.grading != ColorGradingPreset::escape_default ||
            invalidLiveRenderState.live_snapshot.valid ||
            invalidLiveRenderState.lanes.size() != 3 ||
            invalidLiveRenderState.lanes[0].rows.size() != 1 ||
            invalidLiveRenderState.lanes[0].rows[0].function_id != "smooth_escape_ramp") {
            std::cerr << "Expected opening the advanced color pipeline window on an invalid live state to preserve the runtime until the user edits the draft\n";
            return 1;
        }

        auto setParam = [](ColorPipelineRowState& row, const char* path, double value) {
            for (ColorPipelineParamState& param : row.parameter_values) {
                if (param.path == path) {
                    param.number_value = value;
                    return true;
                }
            }
            return false;
        };
        if (!setParam(openWindowState.lanes[0].rows[0], "signal.phase_offset", 0.625) ||
            !setParam(openWindowState.lanes[1].rows[0], "shape.scale", 2.0)) {
            std::cerr << "Expected the advanced color pipeline window render test to find the supported live-backed controls before auto-apply coverage\n";
            return 1;
        }
        bool directControlDirty = false;
        ColorPipelineRenderInteractionState directControlInteraction{};
        if (!TryApplySupportedColorPipelineDraftFromControl(
                &openWindowState,
                view.fractal_type,
                &params,
                &directControlDirty,
                &directControlInteraction)) {
            std::cerr << "Expected the advanced color pipeline control helper to keep supported live-backed edits synced without a separate apply toggle\n";
            return 1;
        }
        if (!NearlyEqual(params.color_phase_signal_offset, 0.625) ||
            !NearlyEqual(params.color_shape_scale, 2.0) ||
            !directControlDirty ||
            !directControlInteraction.interacted ||
            HasColorPipelineDraftEdits(openWindowState)) {
            std::cerr << "Expected supported edits to keep live-backed params synced without a separate apply toggle\n";
            return 1;
        }

        BeginFrame();
        if (!SelectColorPipelineLaneFunction(&openWindowState, 2, "banded_heatmap")) {
            std::cerr << "Expected advanced color pipeline lane changes to work after the window has rendered\n";
            return 1;
        }
        if (!RenderColorPipelineWindow(&openWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected the advanced color pipeline editor to keep rendering after a lane function change\n";
            return 1;
        }
        if (!HasColorPipelineDraftEdits(openWindowState) ||
            openWindowState.lanes[2].rows[0].function_id != "banded_heatmap" ||
            openWindowState.lanes[2].rows[0].parameter_values.size() != 2 ||
            openWindowState.lanes[2].rows[0].parameter_values[0].path != "palette.band_emphasis" ||
            params.color_pipeline.palette != ColorPalette::phase_wheel) {
            std::cerr << "Expected advanced color pipeline renders to honor switched live-backed descriptors without mutating the current runtime palette\n";
            return 1;
        }
        EndFrame();

        openWindowState.lanes[2].rows[0].function_id = "not_real";
        ClearColorPipelineValidationMessages(&openWindowState);
        BeginFrame();
        if (!RenderColorPipelineWindow(&openWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected the advanced color pipeline window to keep rendering even when a lane function id is invalid\n";
            return 1;
        }
        EndFrame();
        bool foundUnknownFunctionMessage = false;
        for (const std::string& message : openWindowState.validation_messages) {
            if (message.find("Unknown advanced color function 'not_real'") != std::string::npos) {
                foundUnknownFunctionMessage = true;
                break;
            }
        }
        if (!foundUnknownFunctionMessage) {
            std::cerr << "Expected invalid advanced color lane function ids to surface an explicit validation message\n";
            return 1;
        }
        openWindowState.lanes[2].rows[0].function_id = "phase_wheel_palette";
        ClearColorPipelineValidationMessages(&openWindowState);
    }

    {
        std::uint64_t nextRowId = 1;
        std::uint64_t missingRowId = 0;
        if (!EnsureImGuiStackEditorRowId(&missingRowId, &nextRowId)) {
            std::cerr << "Expected missing row ids to be assigned through the shared stack-editor helper\n";
            return 1;
        }
        if (missingRowId != 1 || nextRowId != 2) {
            std::cerr << "Expected the first assigned row id to consume the next shared stack-editor id\n";
            return 1;
        }
        if (!EnsureImGuiStackEditorRowId(&missingRowId, &nextRowId)) {
            std::cerr << "Expected already-assigned row ids to remain valid when rechecked\n";
            return 1;
        }
        if (missingRowId != 1 || nextRowId != 2) {
            std::cerr << "Expected existing row ids to stay stable when the helper rechecks them\n";
            return 1;
        }

        std::uint64_t existingRowId = 7;
        if (!EnsureImGuiStackEditorRowId(&existingRowId, &nextRowId)) {
            std::cerr << "Expected the shared helper to accept pre-existing stable row ids\n";
            return 1;
        }
        if (existingRowId != 7 || nextRowId != 8) {
            std::cerr << "Expected the shared helper to advance the next id past any reused stable row id\n";
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
        std::vector<std::string> emptyValidationMessages;
        if (RenderImGuiStackEditorValidationBox("Validation", emptyValidationMessages)) {
            std::cerr << "Expected the shared stack-editor validation box to stay hidden when no messages exist\n";
            return 1;
        }
        EndFrame();

        BeginFrame();
        std::vector<std::string> validationMessages{
            "Source lane needs at least one enabled row.",
            "Palette lane has a duplicate function."
        };
        if (!RenderImGuiStackEditorValidationBox("Validation", validationMessages)) {
            std::cerr << "Expected the shared stack-editor validation box to render when messages exist\n";
            return 1;
        }
        EndFrame();

        BeginFrame();
        bool rowEnabled = true;
        ImGuiStackEditorRowChromeSpec rowSpec;
        rowSpec.tree_node_id = "binding";
        rowSpec.header_label = "mean -> fractal.params.exposure";
        rowSpec.stable_row_id = 42;
        rowSpec.enabled = &rowEnabled;
        ImGuiStackEditorRowChromeResult rowChrome = RenderImGuiStackEditorRowChrome(rowSpec);
        if (!rowChrome.open) {
            std::cerr << "Expected shared stack-editor rows to open by default when first rendered\n";
            return 1;
        }
        if (rowChrome.changed || rowChrome.remove_requested || rowChrome.move_up_requested || rowChrome.move_down_requested) {
            std::cerr << "Expected idle shared stack-editor row chrome to report no actions without clicks\n";
            return 1;
        }
        if (!rowEnabled) {
            std::cerr << "Expected idle shared stack-editor row chrome to preserve the enabled flag\n";
            return 1;
        }
        EndFrame();

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