#include "../src/explaino_sidecar_measurement.h"

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
    double maxValue,
    double stepValue) {
    FunctionParamDescriptor param;
    param.path = path;
    param.type = type;
    param.label = label;
    param.has_min = true;
    param.min_value = minValue;
    param.has_max = true;
    param.max_value = maxValue;
    param.has_step = true;
    param.step_value = stepValue;
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

    fractalSample.parameters.push_back(MakeParam("fractal.view.zoom", "float", "Zoom", 1.0, 100.0, 1.0));

    FunctionParamDescriptor explainoMix = MakeParam("fractal.params.explaino_mix", "float", "Mix", 0.0, 1.0, 0.1);
    explainoMix.has_applicable_when = true;
    explainoMix.applicable_when.op = "eq";
    explainoMix.applicable_when.path = "fractal.view.fractal_type";
    explainoMix.applicable_when.value = "explaino";
    fractalSample.parameters.push_back(explainoMix);

    FunctionParamDescriptor epsilon = MakeParam("fractal.params.epsilon", "float", "Epsilon", 1.0e-6, 1.0e-3, 1.0e-4);
    epsilon.has_applicable_when = true;
    epsilon.applicable_when.op = "eq";
    epsilon.applicable_when.path = "fractal.view.fractal_type";
    epsilon.applicable_when.value = "explaino";
    fractalSample.parameters.push_back(epsilon);

    catalog.functions.push_back(fractalSample);
    return catalog;
}

EngineFunctionCatalog BuildBrokenBindingCatalog() {
    EngineFunctionCatalog catalog = BuildCatalog();
    FunctionParamDescriptor broken = MakeParam("fractal.params.not_real", "float", "Broken", 0.0, 1.0, 0.1);
    broken.has_applicable_when = true;
    broken.applicable_when.op = "eq";
    broken.applicable_when.path = "fractal.view.fractal_type";
    broken.applicable_when.value = "explaino";
    catalog.functions[0].parameters.push_back(broken);
    return catalog;
}

EngineFunctionCatalog BuildDerivedStateCatalog() {
    EngineFunctionCatalog catalog;

    FunctionDescriptor fractalSample;
    fractalSample.id = "fractal.sample";
    fractalSample.name = "Fractal Sample";

    FunctionParamDescriptor fractalType;
    fractalType.path = "fractal.view.fractal_type";
    fractalType.type = "enum";
    fractalType.label = "Fractal Type";
    fractalType.required = true;
    fractalType.options.push_back({"explaino", "Explaino"});
    fractalSample.parameters.push_back(fractalType);

    FunctionParamDescriptor explainoSeed = MakeParam("fractal.params.explaino_seed", "double", "Seed", 0.0, 10.0, 0.25);
    explainoSeed.has_applicable_when = true;
    explainoSeed.applicable_when.op = "eq";
    explainoSeed.applicable_when.path = "fractal.view.fractal_type";
    explainoSeed.applicable_when.value = "explaino";
    fractalSample.parameters.push_back(explainoSeed);

    catalog.functions.push_back(fractalSample);
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

class FakeMeasurementHost : public SidecarMeasurementHost {
public:
    bool fail = false;
    bool wrong_count = false;

    bool Sample(const std::vector<Double2>& coords,
        const ViewState& view,
        const KernelParams& params,
        const RenderSettings& render,
        std::vector<FractalSampleResult>* outResults,
        std::string* outError) const override {
        if (fail) {
            if (outError) *outError = "fake host failure";
            return false;
        }
        if (!outResults) {
            if (outError) *outError = "outResults is null";
            return false;
        }

        const size_t resultCount = wrong_count ? (coords.empty() ? 0u : coords.size() - 1u) : coords.size();
        outResults->clear();
        outResults->reserve(resultCount);

        for (size_t index = 0; index < resultCount; ++index) {
            const Double2 coord = coords[index];
            const double coordSpan = std::fabs(coord.x - view.center_hp_x) + std::fabs(coord.y - view.center_hp_y);
            const double mixSignal = static_cast<double>(params.explaino_mix) * 100.0;
            const double epsilonSignal = static_cast<double>(params.epsilon) * 10000.0;
            const double zoomSignal = coordSpan * 20.0;
            const double renderSignal = static_cast<double>(render.device_id);

            FractalSampleResult sample{};
            sample.iterations = static_cast<int>(std::lround(10.0 + mixSignal + epsilonSignal + zoomSignal + renderSignal));
            sample.final_z_x = static_cast<float>(coord.x);
            sample.final_z_y = static_cast<float>(coord.y);
            sample.residual = static_cast<float>(1.0 + static_cast<double>(params.explaino_mix) + coordSpan + epsilonSignal * 0.1);
            sample.converged = params.explaino_mix >= 0.5f;
            sample.escaped = !sample.converged && coordSpan > 0.05;
            outResults->push_back(sample);
        }
        return true;
    }
};

class DerivedStateHost : public SidecarMeasurementHost {
public:
    bool Sample(const std::vector<Double2>& coords,
        const ViewState& view,
        const KernelParams& params,
        const RenderSettings& render,
        std::vector<FractalSampleResult>* outResults,
        std::string* outError) const override {
        if (!outResults) {
            if (outError) *outError = "outResults is null";
            return false;
        }

        outResults->clear();
        outResults->reserve(coords.size());
        for (const Double2& coord : coords) {
            FractalSampleResult sample{};
            sample.iterations = static_cast<int>(std::lround(100.0 +
                static_cast<double>(params.poly_coeffs[0]) * 50.0 +
                static_cast<double>(params.explaino_roots[0].x) * 10.0 +
                static_cast<double>(render.device_id) +
                std::fabs(coord.x - view.center_hp_x)));
            sample.final_z_x = static_cast<float>(coord.x);
            sample.final_z_y = static_cast<float>(coord.y);
            sample.residual = static_cast<float>(std::fabs(static_cast<double>(params.poly_coeffs[1])) + std::fabs(static_cast<double>(params.explaino_roots[0].y)));
            sample.converged = params.explaino_root_count > 0;
            sample.escaped = false;
            outResults->push_back(sample);
        }
        return true;
    }
};

} // namespace

int main() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
    view.fractal_type = FractalType::explaino;
    view.zoom = 10.0f;
    params.explaino_mix = 0.5f;
    params.epsilon = 1.0e-4f;
    render.device_id = 0;
    BindingContext ctx = MakeBindingContext(&view, &params, &render, &lens);

    {
        SidecarHypothesisSpace space{};
        std::string error;
        if (!BuildSidecarHypothesisSpace(BuildCatalog(), "fractal.sample", ctx, &space, &error)) {
            std::cerr << "Expected sidecar hypothesis space to build: " << error << "\n";
            return 1;
        }

        FakeMeasurementHost host;
        SidecarMeasurementBatch batch;
        if (!BuildSidecarMeasurementBatch(space, ctx, host, &batch, &error)) {
            std::cerr << "Expected sidecar measurement batch to build: " << error << "\n";
            return 1;
        }
        if (batch.coordinate_count != 5) {
            std::cerr << "Expected 5 sample coordinates per measurement batch\n";
            return 1;
        }
        if (batch.rows.size() != 3) {
            std::cerr << "Expected zoom, explaino_mix, and epsilon measurement rows\n";
            return 1;
        }
        if (batch.rows[0].path != "fractal.params.explaino_mix") {
            std::cerr << "Expected measurement rows to rank highest-information param first\n";
            return 1;
        }
        if (batch.rows[0].information_gain_estimate <= batch.rows[1].information_gain_estimate) {
            std::cerr << "Expected ranked measurement rows to sort by descending information gain\n";
            return 1;
        }
        bool sawZoom = false;
        for (const auto& row : batch.rows) {
            if (row.path == "fractal.view.zoom") {
                sawZoom = true;
                if (row.information_gain_estimate <= 0.0) {
                    std::cerr << "Expected zoom measurement to respond to view-derived sample coordinates\n";
                    return 1;
                }
            }
        }
        if (!sawZoom) {
            std::cerr << "Expected zoom row in measurement batch\n";
            return 1;
        }
        if (batch.total_information_gain_estimate <= 0.0 || batch.mean_information_gain_estimate <= 0.0) {
            std::cerr << "Expected positive information-gain aggregates\n";
            return 1;
        }
        if (batch.mean_decode_stability < 0.0 || batch.mean_decode_stability > 1.0) {
            std::cerr << "Expected decode stability aggregate in [0,1]\n";
            return 1;
        }
        SidecarOrientationInputs inputs = BuildSidecarOrientationInputs(batch);
        if (!NearlyEqual(inputs.slime_energy_delta, batch.total_information_gain_estimate)) {
            std::cerr << "Expected orientation inputs to carry total information gain\n";
            return 1;
        }
        if (!NearlyEqual(inputs.busy_beaver_metrics, batch.explored_fraction)) {
            std::cerr << "Expected orientation inputs to carry explored fraction\n";
            return 1;
        }
        if (!NearlyEqual(inputs.decode_stability, batch.mean_decode_stability)) {
            std::cerr << "Expected orientation inputs to carry decode stability\n";
            return 1;
        }
        if (!NearlyEqual(inputs.diff_magnitude, batch.total_diff_magnitude)) {
            std::cerr << "Expected orientation inputs to carry total diff magnitude\n";
            return 1;
        }
    }

    {
        SidecarHypothesisSpace space{};
        std::string error;
        if (!BuildSidecarHypothesisSpace(BuildBrokenBindingCatalog(), "fractal.sample", ctx, &space, &error)) {
            std::cerr << "Expected broken-binding hypothesis space to build so measurement catches the bad path\n";
            return 1;
        }

        FakeMeasurementHost host;
        SidecarMeasurementBatch batch;
        if (BuildSidecarMeasurementBatch(space, ctx, host, &batch, &error)) {
            std::cerr << "Expected measurement batch build to fail on unknown numeric binding path\n";
            return 1;
        }
        if (error.find("fractal.params.not_real") == std::string::npos) {
            std::cerr << "Expected measurement binding error to mention the bad path\n";
            return 1;
        }
    }

    {
        SidecarHypothesisSpace space{};
        std::string error;
        if (!BuildSidecarHypothesisSpace(BuildDerivedStateCatalog(), "fractal.sample", ctx, &space, &error)) {
            std::cerr << "Expected derived-state hypothesis space to build: " << error << "\n";
            return 1;
        }

        DerivedStateHost host;
        SidecarMeasurementBatch batch;
        if (!BuildSidecarMeasurementBatch(space, ctx, host, &batch, &error)) {
            std::cerr << "Expected derived-state measurement batch to build: " << error << "\n";
            return 1;
        }
        if (batch.rows.size() != 1 || batch.rows[0].path != "fractal.params.explaino_seed") {
            std::cerr << "Expected derived-state catalog to expose only explaino_seed as a sweep target\n";
            return 1;
        }
        if (batch.rows[0].information_gain_estimate <= 0.0) {
            std::cerr << "Expected explaino_seed measurement to refresh derived polynomial state\n";
            return 1;
        }
    }

    {
        SidecarHypothesisSpace space{};
        std::string error;
        if (!BuildSidecarHypothesisSpace(BuildCatalog(), "fractal.sample", ctx, &space, &error)) {
            std::cerr << "Expected sidecar hypothesis space to build for failure-path tests\n";
            return 1;
        }

        FakeMeasurementHost host;
        host.fail = true;
        SidecarMeasurementBatch batch;
        if (BuildSidecarMeasurementBatch(space, ctx, host, &batch, &error)) {
            std::cerr << "Expected measurement batch build to fail when the host reports an error\n";
            return 1;
        }
        if (error.find("fake host failure") == std::string::npos) {
            std::cerr << "Expected host error to propagate into the measurement batch failure\n";
            return 1;
        }
    }

    {
        SidecarHypothesisSpace space{};
        std::string error;
        if (!BuildSidecarHypothesisSpace(BuildCatalog(), "fractal.sample", ctx, &space, &error)) {
            std::cerr << "Expected sidecar hypothesis space to build for wrong-count test\n";
            return 1;
        }

        FakeMeasurementHost host;
        host.wrong_count = true;
        SidecarMeasurementBatch batch;
        if (BuildSidecarMeasurementBatch(space, ctx, host, &batch, &error)) {
            std::cerr << "Expected measurement batch build to fail when the host returns the wrong sample count\n";
            return 1;
        }
        if (error.find("returned 4 results for 5 coords") == std::string::npos) {
            std::cerr << "Expected wrong-count error to mention the returned and expected sizes\n";
            return 1;
        }
    }

    std::cout << "test_explaino_sidecar_measurement: all passed\n";
    return 0;
}