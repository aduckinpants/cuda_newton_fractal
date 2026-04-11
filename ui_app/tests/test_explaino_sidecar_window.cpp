#include "../src/explaino_sidecar_window.h"

#include <iostream>
#include <string>

namespace {

bool NearlyEqual(double left, double right, double eps = 1.0e-9) {
    const double delta = left - right;
    return delta < eps && delta > -eps;
}

class FakeMeasurementHost : public SidecarMeasurementHost {
public:
    bool fail = false;

    bool Sample(const std::vector<Double2>& coords,
        const ViewState& view,
        const KernelParams& params,
        const RenderSettings& render,
        std::vector<FractalSampleResult>* outResults,
        std::string* outError) const override {
        if (fail) {
            if (outError) *outError = "fake measurement host failure";
            return false;
        }
        if (!outResults) {
            if (outError) *outError = "outResults is null";
            return false;
        }

        outResults->clear();
        outResults->reserve(coords.size());
        for (const Double2& coord : coords) {
            const double coordSpan = std::fabs(coord.x - view.center_hp_x) + std::fabs(coord.y - view.center_hp_y);
            FractalSampleResult sample{};
            sample.iterations = static_cast<int>(std::lround(20.0 + static_cast<double>(params.explaino_mix) * 100.0 + coordSpan * 20.0 + static_cast<double>(render.device_id)));
            sample.final_z_x = static_cast<float>(coord.x);
            sample.final_z_y = static_cast<float>(coord.y);
            sample.residual = static_cast<float>(1.0 + static_cast<double>(params.explaino_mix) + coordSpan);
            sample.converged = params.explaino_mix >= 0.5f;
            sample.escaped = !sample.converged && coordSpan > 0.05;
            outResults->push_back(sample);
        }
        return true;
    }
};


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

EngineFunctionCatalog BuildDuplicateMeasurementCatalog() {
    EngineFunctionCatalog catalog = BuildCatalog();
    FunctionParamDescriptor duplicate = MakeParam("fractal.params.explaino_mix", "float", "Mix Duplicate", 0.0, 1.0, 0.5);
    duplicate.has_applicable_when = true;
    duplicate.applicable_when.op = "eq";
    duplicate.applicable_when.path = "fractal.view.fractal_type";
    duplicate.applicable_when.value = "explaino";
    catalog.functions[0].parameters.push_back(duplicate);
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
    view.zoom = 10.0f;
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
        FakeMeasurementHost host;
        ExplainoSidecarWindowState state;
        std::string error;
        if (!BuildExplainoSidecarWindowState(BuildCatalog(), ctx, &host, &state, &error)) {
            std::cerr << "Expected sidecar window state with measurement host to build: " << error << "\n";
            return 1;
        }
        if (state.measurement.rows.size() != 2) {
            std::cerr << "Expected measurement rows for zoom and explaino_mix\n";
            return 1;
        }
        if (state.measurement.rows[0].path != "fractal.params.explaino_mix") {
            std::cerr << "Expected sidecar measurements to rank explaino_mix first\n";
            return 1;
        }
        if (state.measurement.total_information_gain_estimate <= 0.0) {
            std::cerr << "Expected sidecar window state to expose positive measurement information gain\n";
            return 1;
        }
        if (!NearlyEqual(state.orientation.slime_energy_delta, state.measurement.total_information_gain_estimate)) {
            std::cerr << "Expected sidecar orientation to ingest measurement information gain\n";
            return 1;
        }
        if (!NearlyEqual(state.orientation.busy_beaver_metrics, state.measurement.explored_fraction)) {
            std::cerr << "Expected sidecar orientation to ingest measurement explored fraction\n";
            return 1;
        }
        if (!state.measurement_error_message.empty()) {
            std::cerr << "Expected successful measurements to leave no measurement error message\n";
            return 1;
        }
        if (state.budget.batch_count != 1 || state.budget.cumulative_information_gain_total != state.measurement.total_information_gain_estimate) {
            std::cerr << "Expected first sidecar window measurement build to seed the persistent budget state\n";
            return 1;
        }
    }

    {
        FakeMeasurementHost host;
        ExplainoSidecarWindowState first;
        ExplainoSidecarWindowState second;
        std::string error;
        if (!BuildExplainoSidecarWindowState(BuildCatalog(), ctx, &host, &first, &error)) {
            std::cerr << "Expected initial sidecar window state to build before persistence test: " << error << "\n";
            return 1;
        }
        if (!BuildExplainoSidecarWindowState(BuildCatalog(), ctx, &host, &first.budget, &second, &error)) {
            std::cerr << "Expected repeated sidecar window state build to preserve budget state: " << error << "\n";
            return 1;
        }
        if (second.budget.batch_count != 2) {
            std::cerr << "Expected repeated sidecar window builds on the same surface to accumulate budget batches\n";
            return 1;
        }
        if (second.budget.cumulative_information_gain_total != first.budget.cumulative_information_gain_total + second.measurement.total_information_gain_estimate) {
            std::cerr << "Expected repeated sidecar window builds to accumulate cumulative information gain\n";
            return 1;
        }
        if (!(second.budget.mean_posterior_uncertainty < first.budget.mean_posterior_uncertainty)) {
            std::cerr << "Expected repeated sidecar window builds to reduce posterior uncertainty\n";
            return 1;
        }
    }

    {
        FakeMeasurementHost host;
        host.fail = true;
        ExplainoSidecarWindowState seeded;
        ExplainoSidecarWindowState state;
        std::string error;
        host.fail = false;
        if (!BuildExplainoSidecarWindowState(BuildCatalog(), ctx, &host, &seeded, &error)) {
            std::cerr << "Expected seeded sidecar window state to build before failure preservation test: " << error << "\n";
            return 1;
        }
        host.fail = true;
        if (BuildExplainoSidecarWindowState(BuildCatalog(), ctx, &host, &seeded.budget, &state, &error)) {
            std::cerr << "Expected sidecar window state build to fail when measurement host fails\n";
            return 1;
        }
        if (error.find("fake measurement host failure") == std::string::npos) {
            std::cerr << "Expected sidecar window measurement error to mention the host failure\n";
            return 1;
        }
        if (state.measurement_error_message.find("fake measurement host failure") == std::string::npos) {
            std::cerr << "Expected sidecar window state to retain the measurement error message\n";
            return 1;
        }
        if (state.rows.size() != 3) {
            std::cerr << "Expected sidecar window state to retain base rows when measurement fails\n";
            return 1;
        }
        if (state.budget.cumulative_information_gain_total != seeded.budget.cumulative_information_gain_total) {
            std::cerr << "Expected sidecar window state to retain the last known budget state when measurement fails\n";
            return 1;
        }
    }

    {
        FakeMeasurementHost host;
        ExplainoSidecarWindowState seeded;
        ExplainoSidecarWindowState state;
        std::string error;
        if (!BuildExplainoSidecarWindowState(BuildCatalog(), ctx, &host, &seeded, &error)) {
            std::cerr << "Expected seeded sidecar window state to build before budget-failure preservation test: " << error << "\n";
            return 1;
        }
        if (BuildExplainoSidecarWindowState(BuildDuplicateMeasurementCatalog(), ctx, &host, &seeded.budget, &state, &error)) {
            std::cerr << "Expected duplicate measurement rows to fail during budget update\n";
            return 1;
        }
        if (error.find("duplicate") == std::string::npos) {
            std::cerr << "Expected budget update failure to report duplicate measurement rows\n";
            return 1;
        }
        if (state.measurement_error_message.find("duplicate") == std::string::npos) {
            std::cerr << "Expected sidecar window state to retain the budget failure message\n";
            return 1;
        }
        if (state.budget.batch_count != seeded.budget.batch_count ||
            state.budget.cumulative_information_gain_total != seeded.budget.cumulative_information_gain_total ||
            state.budget.function_id != seeded.budget.function_id) {
            std::cerr << "Expected budget update failures to retain the last known budget state\n";
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