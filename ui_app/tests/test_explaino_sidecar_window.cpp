#include "../src/explaino_sidecar_window.h"

#include <iostream>
#include <string>

namespace {

FunctionParamDescriptor MakeParam(
    const char* path,
    const char* type,
    const char* label,
    double minValue,
    double maxValue,
    double defaultValue) {
    FunctionParamDescriptor param;
    param.path = path;
    param.type = type;
    param.label = label;
    param.has_min = true;
    param.min_value = minValue;
    param.has_max = true;
    param.max_value = maxValue;
    param.has_default = true;
    param.default_value = json_min::Value{defaultValue};
    return param;
}

EngineFunctionCatalog BuildCatalog() {
    EngineFunctionCatalog catalog;

    FunctionDescriptor fractalSample;
    fractalSample.id = "fractal.sample";

    FunctionParamDescriptor fractalType;
    fractalType.path = "fractal.view.fractal_type";
    fractalType.type = "enum";
    fractalType.label = "Fractal Type";
    fractalType.required = true;
    fractalType.options.push_back({"mandelbrot", "Mandelbrot"});
    fractalType.options.push_back({"explaino", "Explaino"});
    fractalSample.parameters.push_back(fractalType);

    FunctionParamDescriptor zoom = MakeParam("fractal.view.zoom", "float", "Zoom", 1.0, 1000.0, 1.0);
    fractalSample.parameters.push_back(zoom);

    FunctionParamDescriptor explainoMix = MakeParam("fractal.params.explaino_mix", "float", "Mix", 0.0, 1.0, 0.5);
    explainoMix.has_applicable_when = true;
    explainoMix.applicable_when.op = "eq";
    explainoMix.applicable_when.path = "fractal.view.fractal_type";
    explainoMix.applicable_when.value = "explaino";
    fractalSample.parameters.push_back(explainoMix);

    catalog.functions.push_back(fractalSample);
    return catalog;
}

EngineFunctionCatalog BuildBrokenCatalog() {
    EngineFunctionCatalog catalog = BuildCatalog();
    FunctionParamDescriptor broken = MakeParam("fractal.params.nova_alpha", "float", "Broken", 0.0, 1.0, 0.5);
    broken.has_applicable_when = true;
    broken.applicable_when.op = "eq";
    broken.applicable_when.path = "fractal.view.not_real";
    broken.applicable_when.value = "explaino";
    catalog.functions[0].parameters.push_back(broken);
    return catalog;
}

EngineFunctionCatalog BuildUnsupportedCurrentTypeCatalog() {
    EngineFunctionCatalog catalog = BuildCatalog();
    catalog.functions[0].parameters[0].options.clear();
    catalog.functions[0].parameters[0].options.push_back({"mandelbrot", "Mandelbrot"});
    return catalog;
}

EngineFunctionCatalog BuildNoSupportedTypeCatalog() {
    EngineFunctionCatalog catalog = BuildCatalog();
    catalog.functions[0].parameters[0].options.clear();
    return catalog;
}

BindingContext MakeBindingContext(ViewState* view, KernelParams* params, RenderSettings* render, LensSettings* lens) {
    BindingContext ctx;
    ctx.view = view;
    ctx.params = params;
    ctx.render = render;
    ctx.lens = lens;
    return ctx;
}

} // namespace

int main() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
    view.fractal_type = FractalType::explaino;
    params.explaino_seed = 7.0;
    view.explaino_seed_drift = 0.125f;
    BindingContext ctx = MakeBindingContext(&view, &params, &render, &lens);

    {
        ExplainoSidecarWindowState state;
        std::string error;
        if (!BuildExplainoSidecarWindowState(BuildCatalog(), ctx, &state, &error)) {
            std::cerr << "Expected sidecar window state to build: " << error << "\n";
            return 1;
        }
        if (state.title != "Explaino Sidecar") {
            std::cerr << "Expected canonical sidecar window title\n";
            return 1;
        }
        if (state.function_id != "fractal.sample") {
            std::cerr << "Expected fractal.sample function id in sidecar state\n";
            return 1;
        }
        if (state.fractal_type_id != "explaino") {
            std::cerr << "Expected explaino fractal type id in sidecar state\n";
            return 1;
        }
        if (state.rows.size() != 3) {
            std::cerr << "Expected 3 sidecar window rows, got: " << state.rows.size() << "\n";
            return 1;
        }
        if (state.rows[0].label != "Zoom" || state.rows[0].range_text.empty()) {
            std::cerr << "Expected first row to expose Zoom with a rendered range\n";
            return 1;
        }
        if (state.rows[1].label != "Mix" || state.rows[1].default_text.empty()) {
            std::cerr << "Expected Explaino-only row to expose default text\n";
            return 1;
        }
        if (state.rows[2].label != "Fractal Type") {
            std::cerr << "Expected required enum row to sort after ranged params\n";
            return 1;
        }
        if (state.orientation.import_signature == 0 || state.orientation.pack_projection_hash == 0) {
            std::cerr << "Expected sidecar window state to carry orientation data\n";
            return 1;
        }
    }

    {
        ExplainoSidecarWindowState state;
        std::string error;
        if (BuildExplainoSidecarWindowState(BuildBrokenCatalog(), ctx, &state, &error)) {
            std::cerr << "Expected broken sidecar window state build to fail\n";
            return 1;
        }
        if (error.find("fractal.view.not_real") == std::string::npos) {
            std::cerr << "Expected sidecar window build error to mention the bad predicate path\n";
            return 1;
        }
        if (state.error_message.find("fractal.view.not_real") == std::string::npos) {
            std::cerr << "Expected sidecar window state to retain the model error message\n";
            return 1;
        }
    }

    {
        ExplainoSidecarWindowState state;
        std::string error;
        if (BuildExplainoSidecarWindowState(BuildUnsupportedCurrentTypeCatalog(), ctx, &state, &error)) {
            std::cerr << "Expected unsupported current fractal_type to fail sidecar window build\n";
            return 1;
        }
        if (error.find("fractal.view.fractal_type=explaino") == std::string::npos) {
            std::cerr << "Expected sidecar window error to mention the unsupported enum selection\n";
            return 1;
        }
        if (state.error_message.find("fractal.view.fractal_type=explaino") == std::string::npos) {
            std::cerr << "Expected sidecar window state to retain the unsupported-type error message\n";
            return 1;
        }
    }

    {
        ExplainoSidecarWindowState state;
        std::string error;
        if (BuildExplainoSidecarWindowState(BuildNoSupportedTypeCatalog(), ctx, &state, &error)) {
            std::cerr << "Expected empty required enum options to fail sidecar window build\n";
            return 1;
        }
        if (error.find("has no supported options") == std::string::npos) {
            std::cerr << "Expected sidecar window error to mention the invalid required enum surface\n";
            return 1;
        }
        if (state.error_message.find("has no supported options") == std::string::npos) {
            std::cerr << "Expected sidecar window state to retain the empty-options error message\n";
            return 1;
        }
    }

    std::cout << "test_explaino_sidecar_window: all passed\n";
    return 0;
}