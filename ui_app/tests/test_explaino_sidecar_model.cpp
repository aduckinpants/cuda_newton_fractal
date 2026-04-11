#include "../src/explaino_sidecar_model.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {

bool NearlyEqual(double left, double right, double eps = 1.0e-9) {
    const double delta = left - right;
    return delta < eps && delta > -eps;
}

FunctionParamDescriptor MakeParam(
    const char* path,
    const char* type,
    const char* label,
    double minValue,
    double maxValue) {
    FunctionParamDescriptor param;
    param.path = path;
    param.type = type;
    param.label = label;
    param.has_min = true;
    param.min_value = minValue;
    param.has_max = true;
    param.max_value = maxValue;
    return param;
}

EngineFunctionCatalog BuildCatalog() {
    EngineFunctionCatalog catalog;

    FunctionDescriptor fractalSample;
    fractalSample.id = "fractal.sample";
    fractalSample.name = "Fractal Sample";

    FunctionParamDescriptor fractalType;
    fractalType.path = "fractal.view.fractal_type";
    fractalType.type = "enum";
    fractalType.label = "Fractal Type";
    fractalType.required = true;
    fractalType.options.push_back({"mandelbrot", "Mandelbrot"});
    fractalType.options.push_back({"explaino", "Explaino"});
    fractalSample.parameters.push_back(fractalType);

    FunctionParamDescriptor zoom = MakeParam("fractal.view.zoom", "float", "Zoom", 1.0, 1000.0);
    fractalSample.parameters.push_back(zoom);

    FunctionParamDescriptor epsilon = MakeParam("fractal.params.epsilon", "float", "Epsilon", 1.0e-12, 1.0e-2);
    epsilon.has_applicable_when = true;
    epsilon.applicable_when.op = "eq";
    epsilon.applicable_when.path = "fractal.view.fractal_type";
    epsilon.applicable_when.value = "explaino";
    fractalSample.parameters.push_back(epsilon);

    FunctionParamDescriptor explainoMix = MakeParam("fractal.params.explaino_mix", "float", "Mix", 0.0, 1.0);
    explainoMix.has_applicable_when = true;
    explainoMix.applicable_when.op = "eq";
    explainoMix.applicable_when.path = "fractal.view.fractal_type";
    explainoMix.applicable_when.value = "explaino";
    fractalSample.parameters.push_back(explainoMix);

    catalog.functions.push_back(fractalSample);
    return catalog;
}

EngineFunctionCatalog BuildUnknownPredicatePathCatalog() {
    EngineFunctionCatalog catalog = BuildCatalog();
    FunctionParamDescriptor broken = MakeParam("fractal.params.nova_alpha", "float", "Broken Path", 0.0, 1.0);
    broken.has_applicable_when = true;
    broken.applicable_when.op = "eq";
    broken.applicable_when.path = "fractal.view.not_real";
    broken.applicable_when.value = "explaino";
    catalog.functions[0].parameters.push_back(broken);
    return catalog;
}

EngineFunctionCatalog BuildInvalidNumericPredicateCatalog() {
    EngineFunctionCatalog catalog = BuildCatalog();
    FunctionParamDescriptor broken = MakeParam("fractal.params.nova_alpha", "float", "Broken Numeric", 0.0, 1.0);
    broken.has_applicable_when = true;
    broken.applicable_when.op = "gte";
    broken.applicable_when.path = "fractal.view.zoom";
    broken.applicable_when.value = "not-a-number";
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

bool HasPath(const SidecarHypothesisSpace& space, const char* path) {
    for (const auto& param : space.applicable_parameters) {
        if (param.path == path) return true;
    }
    return false;
}

} // namespace

int main() {
    EngineFunctionCatalog catalog = BuildCatalog();
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
    view.fractal_type = FractalType::explaino;
    params.explaino_seed = 3.0;
    view.explaino_seed_drift = 0.25f;
    BindingContext ctx = MakeBindingContext(&view, &params, &render, &lens);

    {
        SidecarHypothesisSpace missing{};
        std::string error;
        if (BuildSidecarHypothesisSpace(catalog, "missing.function", ctx, &missing, &error)) {
            std::cerr << "Expected missing function id to fail\n";
            return 1;
        }
        if (error.find("missing.function") == std::string::npos) {
            std::cerr << "Expected missing-function error to mention the requested id\n";
            return 1;
        }
    }

    {
        SidecarHypothesisSpace broken{};
        std::string error;
        if (BuildSidecarHypothesisSpace(BuildUnknownPredicatePathCatalog(), "fractal.sample", ctx, &broken, &error)) {
            std::cerr << "Expected unknown applicable_when path to fail fast\n";
            return 1;
        }
        if (error.find("fractal.view.not_real") == std::string::npos) {
            std::cerr << "Expected unknown applicable_when path error to mention the bad path\n";
            return 1;
        }
    }

    {
        SidecarHypothesisSpace broken{};
        std::string error;
        if (BuildSidecarHypothesisSpace(BuildInvalidNumericPredicateCatalog(), "fractal.sample", ctx, &broken, &error)) {
            std::cerr << "Expected invalid numeric applicable_when value to fail fast\n";
            return 1;
        }
        if (error.find("not-a-number") == std::string::npos) {
            std::cerr << "Expected invalid numeric applicable_when error to mention the bad value\n";
            return 1;
        }
    }

    {
        SidecarHypothesisSpace broken{};
        std::string error;
        if (BuildSidecarHypothesisSpace(BuildUnsupportedCurrentTypeCatalog(), "fractal.sample", ctx, &broken, &error)) {
            std::cerr << "Expected unsupported current fractal_type to fail fast\n";
            return 1;
        }
        if (error.find("fractal.view.fractal_type=explaino") == std::string::npos) {
            std::cerr << "Expected unsupported enum error to mention the filtered current selection\n";
            return 1;
        }
    }

    {
        SidecarHypothesisSpace broken{};
        std::string error;
        if (BuildSidecarHypothesisSpace(BuildNoSupportedTypeCatalog(), "fractal.sample", ctx, &broken, &error)) {
            std::cerr << "Expected empty required enum options to fail fast\n";
            return 1;
        }
        if (error.find("has no supported options") == std::string::npos) {
            std::cerr << "Expected empty-required-enum error to mention the invalid catalog surface\n";
            return 1;
        }
    }

    SidecarHypothesisSpace explainoSpace{};
    std::string error;
    if (!BuildSidecarHypothesisSpace(catalog, "fractal.sample", ctx, &explainoSpace, &error)) {
        std::cerr << "Expected explaino sidecar hypothesis space to build: " << error << "\n";
        return 1;
    }
    if (explainoSpace.applicable_parameters.size() != 4) {
        std::cerr << "Expected 4 applicable explaino params, got: " << explainoSpace.applicable_parameters.size() << "\n";
        return 1;
    }
    if (explainoSpace.applicable_parameters[0].path != "fractal.view.zoom") {
        std::cerr << "Expected widest declared span to sort first\n";
        return 1;
    }
    if (explainoSpace.applicable_parameters[1].path != "fractal.params.explaino_mix") {
        std::cerr << "Expected explaino_mix to sort ahead of epsilon by declared span\n";
        return 1;
    }
    if (explainoSpace.applicable_parameters[2].path != "fractal.params.epsilon") {
        std::cerr << "Expected epsilon to remain applicable for explaino\n";
        return 1;
    }
    if (explainoSpace.applicable_parameters[3].path != "fractal.view.fractal_type") {
        std::cerr << "Expected span-less fractal_type to sort last\n";
        return 1;
    }

    SidecarOrientationVector orientationA = ComputeSidecarOrientationVector(ctx, explainoSpace);
    if (orientationA.import_signature == 0) {
        std::cerr << "Expected import signature to hash current state\n";
        return 1;
    }
    if (orientationA.pack_projection_hash == 0) {
        std::cerr << "Expected pack projection hash to reflect applicable params\n";
        return 1;
    }
    if (!NearlyEqual(orientationA.field_embedding_stats, 4.0)) {
        std::cerr << "Expected field embedding stats to reflect applicable param count\n";
        return 1;
    }

    params.explaino_seed = 4.0;
    SidecarOrientationVector orientationB = ComputeSidecarOrientationVector(ctx, explainoSpace);
    if (orientationA.import_signature == orientationB.import_signature) {
        std::cerr << "Expected import signature to change when the Explaino seed changes\n";
        return 1;
    }
    if (orientationA.pack_projection_hash != orientationB.pack_projection_hash) {
        std::cerr << "Expected pack projection hash to remain stable for the same applicable surface\n";
        return 1;
    }
    if (HashSidecarOrientationVector(orientationA) == HashSidecarOrientationVector(orientationB)) {
        std::cerr << "Expected orientation hash to change when import signature changes\n";
        return 1;
    }

    view.fractal_type = FractalType::mandelbrot;
    SidecarHypothesisSpace mandelbrotSpace{};
    if (!BuildSidecarHypothesisSpace(catalog, "fractal.sample", ctx, &mandelbrotSpace, &error)) {
        std::cerr << "Expected mandelbrot sidecar hypothesis space to build: " << error << "\n";
        return 1;
    }
    if (mandelbrotSpace.applicable_parameters.size() != 2) {
        std::cerr << "Expected only 2 applicable mandelbrot params, got: " << mandelbrotSpace.applicable_parameters.size() << "\n";
        return 1;
    }
    if (HasPath(mandelbrotSpace, "fractal.params.epsilon") || HasPath(mandelbrotSpace, "fractal.params.explaino_mix")) {
        std::cerr << "Expected Explaino-only params to be filtered for mandelbrot\n";
        return 1;
    }
    SidecarOrientationVector orientationC = ComputeSidecarOrientationVector(ctx, mandelbrotSpace);
    if (orientationC.pack_projection_hash == orientationA.pack_projection_hash) {
        std::cerr << "Expected pack projection hash to change when applicable params change\n";
        return 1;
    }
    if (!NearlyEqual(orientationC.field_embedding_stats, 2.0)) {
        std::cerr << "Expected mandelbrot field embedding stats to reflect its narrower surface\n";
        return 1;
    }

    {
        SidecarHypothesisSpace collisionA{};
        collisionA.function_id = "fractal.sample";
        collisionA.applicable_parameters.push_back({"ab", "A", "c", "", false, {}, false, 0.0, false, 0.0});

        SidecarHypothesisSpace collisionB{};
        collisionB.function_id = "fractal.sample";
        collisionB.applicable_parameters.push_back({"a", "B", "bc", "", false, {}, false, 0.0, false, 0.0});

        SidecarOrientationVector collisionOrientationA = ComputeSidecarOrientationVector(ctx, collisionA);
        SidecarOrientationVector collisionOrientationB = ComputeSidecarOrientationVector(ctx, collisionB);
        if (collisionOrientationA.pack_projection_hash == collisionOrientationB.pack_projection_hash) {
            std::cerr << "Expected pack projection hash to distinguish adjacent-field boundary changes\n";
            return 1;
        }
    }

    std::cout << "test_explaino_sidecar_model: all passed\n";
    return 0;
}