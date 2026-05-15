#include "../src/explaino_sidecar_measurement.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {

bool NearlyEqual(double left, double right, double eps = 1.0e-9) {
    const double delta = left - right;
    return delta < eps && delta > -eps;
}

double ComputeAggregateDeltaForTest(
    const SidecarMeasurementAggregate& baseline,
    const SidecarMeasurementAggregate& variant) {
    double score = 0.0;
    score += std::fabs(variant.mean_iterations - baseline.mean_iterations);
    score += std::log1p(std::fabs(variant.mean_residual - baseline.mean_residual));
    score += 25.0 * std::fabs(variant.converged_fraction - baseline.converged_fraction);
    score += 25.0 * std::fabs(variant.escaped_fraction - baseline.escaped_fraction);
    return score;
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

EngineFunctionCatalog BuildProjectionParityCatalog() {
    EngineFunctionCatalog catalog;

    FunctionDescriptor fractalSample;
    fractalSample.id = "fractal.sample";
    fractalSample.name = "Fractal Sample";

    FunctionParamDescriptor fractalType;
    fractalType.path = "fractal.view.fractal_type";
    fractalType.type = "enum";
    fractalType.label = "Fractal Type";
    fractalType.required = true;
    fractalType.options.push_back({"explaino_all", "Explaino-all"});
    fractalSample.parameters.push_back(fractalType);

    FunctionParamDescriptor vortex = MakeParam("fractal.params.vortex_strength", "float", "Vortex Strength", 0.0, 1.0, 0.05);
    vortex.has_applicable_when = true;
    vortex.applicable_when.op = "eq";
    vortex.applicable_when.path = "fractal.view.fractal_type";
    vortex.applicable_when.value = "explaino_all";
    fractalSample.parameters.push_back(vortex);

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

class ProjectionCarrierHost : public SidecarMeasurementHost {
public:
    bool Sample(const std::vector<Double2>& coords,
        const ViewState& view,
        const KernelParams& params,
        const RenderSettings&,
        std::vector<FractalSampleResult>* outResults,
        std::string* outError) const override {
        if (!outResults) {
            if (outError) *outError = "outResults is null";
            return false;
        }

        const double carrierGain = view.fractal_type == FractalType::explaino_all ? 1.0 : 1.75;
        outResults->clear();
        outResults->reserve(coords.size());
        for (const Double2& coord : coords) {
            const double coordSpan = std::fabs(coord.x - view.center_hp_x) + std::fabs(coord.y - view.center_hp_y);
            const double signal = static_cast<double>(params.vortex_strength) * carrierGain;

            FractalSampleResult sample{};
            sample.iterations = static_cast<int>(std::lround(20.0 + signal * 400.0 + coordSpan * 10.0));
            sample.final_z_x = static_cast<float>(coord.x);
            sample.final_z_y = static_cast<float>(coord.y);
            sample.residual = static_cast<float>(1.0 + signal + coordSpan * carrierGain);
            sample.converged = signal < 0.45;
            sample.escaped = !sample.converged && coordSpan > 0.05;
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
        const SidecarMeasurementRow& rankedRow = batch.rows[0];
        const double minusDelta = ComputeAggregateDeltaForTest(rankedRow.baseline, rankedRow.minus_variant);
        const double plusDelta = ComputeAggregateDeltaForTest(rankedRow.baseline, rankedRow.plus_variant);
        const double expectedGradient = (plusDelta - minusDelta) / (2.0 * rankedRow.step_value);
        const double expectedCurvature = (plusDelta + minusDelta) / (rankedRow.step_value * rankedRow.step_value);
        if (!NearlyEqual(rankedRow.information_gradient, expectedGradient)) {
            std::cerr << "Expected measurement row to expose the centered information gradient\n";
            return 1;
        }
        if (!NearlyEqual(rankedRow.information_curvature, expectedCurvature)) {
            std::cerr << "Expected measurement row to expose the centered information curvature\n";
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
        ViewState legacyView{};
        KernelParams legacyParams{};
        RenderSettings legacyRender{};
        LensSettings legacyLens{};
        legacyView.fractal_type = FractalType::explaino_vortex;
        legacyView.zoom = 10.0f;
        legacyParams.vortex_strength = 0.3f;
        BindingContext legacyCtx = MakeBindingContext(&legacyView, &legacyParams, &legacyRender, &legacyLens);

        ViewState canonicalView{};
        KernelParams canonicalParams{};
        RenderSettings canonicalRender{};
        LensSettings canonicalLens{};
        canonicalView.fractal_type = FractalType::explaino_all;
        canonicalView.zoom = 10.0f;
        canonicalParams.vortex_strength = 0.3f;
        BindingContext canonicalCtx = MakeBindingContext(&canonicalView, &canonicalParams, &canonicalRender, &canonicalLens);

        const EngineFunctionCatalog projectionCatalog = BuildProjectionParityCatalog();
        SidecarHypothesisSpace legacySpace{};
        SidecarHypothesisSpace canonicalSpace{};
        std::string error;
        if (!BuildSidecarHypothesisSpace(projectionCatalog, "fractal.sample", legacyCtx, &legacySpace, &error) ||
            !BuildSidecarHypothesisSpace(projectionCatalog, "fractal.sample", canonicalCtx, &canonicalSpace, &error)) {
            std::cerr << "Expected projection-parity hypothesis spaces to build: " << error << "\n";
            return 1;
        }

        ProjectionCarrierHost host;
        SidecarMeasurementBatch legacyBatch;
        SidecarMeasurementBatch canonicalBatch;
        if (!BuildSidecarMeasurementBatch(legacySpace, legacyCtx, host, &legacyBatch, &error) ||
            !BuildSidecarMeasurementBatch(canonicalSpace, canonicalCtx, host, &canonicalBatch, &error)) {
            std::cerr << "Expected projection-parity measurement batches to build: " << error << "\n";
            return 1;
        }

        if (legacyBatch.rows.size() != 1 || canonicalBatch.rows.size() != 1 ||
            legacyBatch.rows[0].path != "fractal.params.vortex_strength" ||
            canonicalBatch.rows[0].path != "fractal.params.vortex_strength") {
            std::cerr << "Expected projection-parity catalog to expose only the canonical vortex axis measurement row\n";
            return 1;
        }
        if (!NearlyEqual(legacyBatch.total_information_gain_estimate, canonicalBatch.total_information_gain_estimate) ||
            !NearlyEqual(legacyBatch.explored_fraction, canonicalBatch.explored_fraction) ||
            !NearlyEqual(legacyBatch.mean_decode_stability, canonicalBatch.mean_decode_stability) ||
            !NearlyEqual(legacyBatch.rows[0].information_gain_estimate, canonicalBatch.rows[0].information_gain_estimate) ||
            !NearlyEqual(legacyBatch.rows[0].minus_variant.mean_iterations, canonicalBatch.rows[0].minus_variant.mean_iterations) ||
            !NearlyEqual(legacyBatch.rows[0].plus_variant.mean_iterations, canonicalBatch.rows[0].plus_variant.mean_iterations)) {
            std::cerr << "Expected legacy explaino_vortex sidecar measurement sweeps to canonicalize to explaino_all instead of preserving a separate raw carrier authority\n";
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