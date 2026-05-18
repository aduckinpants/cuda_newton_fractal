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
    fractalType.options.push_back({"explaino_vortex", "Explaino Vortex"});
    fractalType.options.push_back({"explaino_all", "Explaino-all"});
    fractalSample.parameters.push_back(fractalType);

    FunctionParamDescriptor vortex = MakeParam("fractal.params.vortex_strength", "float", "Vortex Strength", 0.0, 1.0, 0.05);
    vortex.has_applicable_when = true;
    vortex.applicable_when.op = "in";
    vortex.applicable_when.path = "fractal.view.fractal_type";
    vortex.applicable_when.value = "explaino_all,explaino_vortex";
    fractalSample.parameters.push_back(vortex);

    catalog.functions.push_back(fractalSample);
    return catalog;
}

EngineFunctionCatalog BuildBalanceVoidParityCatalog() {
    EngineFunctionCatalog catalog;

    FunctionDescriptor fractalSample;
    fractalSample.id = "fractal.sample";
    fractalSample.name = "Fractal Sample";

    FunctionParamDescriptor fractalType;
    fractalType.path = "fractal.view.fractal_type";
    fractalType.type = "enum";
    fractalType.label = "Fractal Type";
    fractalType.required = true;
    fractalType.options.push_back({"explaino_balance_void", "Explaino BalanceVoid"});
    fractalType.options.push_back({"explaino_all", "Explaino-all"});
    fractalSample.parameters.push_back(fractalType);

    FunctionParamDescriptor balanceVoid = MakeParam("fractal.params.balance_void", "float", "Balance Void", -1.0, 1.0, 0.05);
    balanceVoid.has_applicable_when = true;
    balanceVoid.applicable_when.op = "in";
    balanceVoid.applicable_when.path = "fractal.view.fractal_type";
    balanceVoid.applicable_when.value = "explaino_all,explaino_balance_void";
    fractalSample.parameters.push_back(balanceVoid);

    FunctionParamDescriptor symmetryTension = MakeParam("fractal.params.symmetry_tension", "float", "Symmetry Tension", -1.0, 1.0, 0.05);
    symmetryTension.has_applicable_when = true;
    symmetryTension.applicable_when.op = "in";
    symmetryTension.applicable_when.path = "fractal.view.fractal_type";
    symmetryTension.applicable_when.value = "explaino_all,explaino_balance_void";
    fractalSample.parameters.push_back(symmetryTension);

    FunctionParamDescriptor fieldCurvature = MakeParam("fractal.params.field_curvature", "float", "Field Curvature", -1.0, 1.0, 0.05);
    fieldCurvature.has_applicable_when = true;
    fieldCurvature.applicable_when.op = "in";
    fieldCurvature.applicable_when.path = "fractal.view.fractal_type";
    fieldCurvature.applicable_when.value = "explaino_all,explaino_balance_void";
    fractalSample.parameters.push_back(fieldCurvature);

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

class EvidenceMeasurementHost : public SidecarMeasurementHost {
public:
    mutable int legacy_calls = 0;
    bool mismatch_sample_coord = false;

    bool Sample(const std::vector<Double2>&,
        const ViewState&,
        const KernelParams&,
        const RenderSettings&,
        std::vector<FractalSampleResult>*,
        std::string* outError) const override {
        ++legacy_calls;
        if (outError) *outError = "legacy sample should not be used when widened evidence is supported";
        return false;
    }

    bool SupportsWidenedEvidence() const override {
        return true;
    }

    bool SampleEvidence(const std::vector<Double2>& coords,
        const ViewState& view,
        const KernelParams& params,
        const RenderSettings& render,
        std::vector<FractalSampleEvidence>* outEvidence,
        std::string* outError) const override {
        if (!outEvidence) {
            if (outError) *outError = "outEvidence is null";
            return false;
        }

        outEvidence->clear();
        outEvidence->reserve(coords.size());
        for (std::size_t index = 0; index < coords.size(); ++index) {
            const Double2 coord = coords[index];
            const double coordSpan = std::fabs(coord.x - view.center_hp_x) + std::fabs(coord.y - view.center_hp_y);
            const double mixSignal = static_cast<double>(params.explaino_mix) * 100.0;
            const double epsilonSignal = static_cast<double>(params.epsilon) * 10000.0;
            const double zoomSignal = coordSpan * 20.0;
            const double renderSignal = static_cast<double>(render.device_id);

            FractalSampleEvidence evidence{};
            evidence.sample_coord = mismatch_sample_coord && index == 0u
                ? Double2{coord.x + 0.25, coord.y}
                : coord;
            evidence.legacy_result.iterations = static_cast<int>(std::lround(10.0 + mixSignal + epsilonSignal + zoomSignal + renderSignal));
            evidence.legacy_result.final_z_x = static_cast<float>(coord.x);
            evidence.legacy_result.final_z_y = static_cast<float>(coord.y);
            evidence.legacy_result.residual = static_cast<float>(1.0 + static_cast<double>(params.explaino_mix) + coordSpan + epsilonSignal * 0.1);
            evidence.legacy_result.converged = params.explaino_mix >= 0.5f;
            evidence.legacy_result.escaped = !evidence.legacy_result.converged && coordSpan > 0.05;
            outEvidence->push_back(evidence);
        }
        return true;
    }
};

class BalanceVoidEvidenceHost : public SidecarMeasurementHost {
public:
    mutable int legacy_calls = 0;

    bool Sample(const std::vector<Double2>&,
        const ViewState&,
        const KernelParams&,
        const RenderSettings&,
        std::vector<FractalSampleResult>*,
        std::string* outError) const override {
        ++legacy_calls;
        if (outError) *outError = "legacy sample should not be used for BalanceVoid widened-evidence proof";
        return false;
    }

    bool SupportsWidenedEvidence() const override {
        return true;
    }

    bool SampleEvidence(const std::vector<Double2>& coords,
        const ViewState& view,
        const KernelParams& params,
        const RenderSettings& render,
        std::vector<FractalSampleEvidence>* outEvidence,
        std::string* outError) const override {
        if (!outEvidence) {
            if (outError) *outError = "outEvidence is null";
            return false;
        }

        outEvidence->clear();
        outEvidence->reserve(coords.size());
        for (const Double2& coord : coords) {
            const double coordSpan = std::fabs(coord.x - view.center_hp_x) + std::fabs(coord.y - view.center_hp_y);
            const double balanceSignal = static_cast<double>(params.balance_void) * 60.0;
            const double symmetrySignal = static_cast<double>(params.symmetry_tension) * 45.0;
            const double curvatureSignal = static_cast<double>(params.field_curvature) * 30.0;
            const double renderSignal = static_cast<double>(render.device_id);
            const double totalSignal = balanceSignal + symmetrySignal + curvatureSignal + renderSignal;

            FractalSampleEvidence evidence{};
            evidence.sample_coord = coord;
            evidence.legacy_result.iterations = static_cast<int>(std::lround(18.0 + totalSignal + coordSpan * 8.0));
            evidence.legacy_result.final_z_x = static_cast<float>(coord.x);
            evidence.legacy_result.final_z_y = static_cast<float>(coord.y);
            evidence.legacy_result.residual = static_cast<float>(1.0 +
                0.20 * std::fabs(static_cast<double>(params.balance_void)) +
                0.15 * std::fabs(static_cast<double>(params.symmetry_tension)) +
                0.10 * std::fabs(static_cast<double>(params.field_curvature)) +
                coordSpan);
            evidence.legacy_result.converged =
                (static_cast<double>(params.balance_void) + static_cast<double>(params.field_curvature)) >=
                std::fabs(static_cast<double>(params.symmetry_tension));
            evidence.legacy_result.escaped = !evidence.legacy_result.converged && coordSpan > 0.05;
            outEvidence->push_back(evidence);
        }
        return true;
    }
};

const SidecarMeasurementRow* FindMeasurementRowByPath(const SidecarMeasurementBatch& batch, const char* path) {
    for (const SidecarMeasurementRow& row : batch.rows) {
        if (row.path == path) {
            return &row;
        }
    }
    return nullptr;
}

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

        if (legacySpace.applicable_parameters.size() != 2 || canonicalSpace.applicable_parameters.size() != 2) {
            std::cerr << "Expected projection-parity hypothesis spaces to expose the canonical fractal selector plus the shared vortex axis on both explaino_all and Explaino-Vortex\n";
            return 1;
        }
        if (legacyBatch.rows.size() != 1 || canonicalBatch.rows.size() != 1 ||
            legacyBatch.rows[0].path != "fractal.params.vortex_strength" ||
            canonicalBatch.rows[0].path != "fractal.params.vortex_strength") {
            std::cerr << "Expected projection-parity catalog to expose the shared vortex axis row on both explaino_all and Explaino-Vortex\n";
            return 1;
        }
        if (!(legacyBatch.rows[0].information_gain_estimate > 0.0) ||
            !(canonicalBatch.rows[0].information_gain_estimate > 0.0) ||
            !NearlyEqual(legacyBatch.explored_fraction, 1.0) ||
            !NearlyEqual(canonicalBatch.explored_fraction, 1.0)) {
            std::cerr << "Expected Explaino-Vortex sidecar measurement sweeps to stay real on both the explicit owner lane and the canonical explaino_all shared surface\n";
            return 1;
        }
    }

    {
        SidecarHypothesisSpace space{};
        std::string error;
        if (!BuildSidecarHypothesisSpace(BuildCatalog(), "fractal.sample", ctx, &space, &error)) {
            std::cerr << "Expected sidecar hypothesis space to build for widened-evidence measurement tests: " << error << "\n";
            return 1;
        }

        EvidenceMeasurementHost host;
        SidecarMeasurementBatch batch;
        if (!BuildSidecarMeasurementBatch(space, ctx, host, &batch, &error)) {
            std::cerr << "Expected widened-evidence measurement batch to build: " << error << "\n";
            return 1;
        }
        if (host.legacy_calls != 0) {
            std::cerr << "Expected widened sidecar measurement path to use widened evidence instead of falling back to legacy sampling\n";
            return 1;
        }
        const SidecarMeasurementRow* mixRow = nullptr;
        const SidecarMeasurementRow* zoomRow = nullptr;
        for (const SidecarMeasurementRow& row : batch.rows) {
            if (row.path == "fractal.params.explaino_mix") mixRow = &row;
            if (row.path == "fractal.view.zoom") zoomRow = &row;
        }
        if (!mixRow || !zoomRow) {
            std::cerr << "Expected widened measurement batch to keep both the explaino_mix and zoom rows\n";
            return 1;
        }
        if (mixRow->minus_counterfactual.coordinate_count != batch.coordinate_count ||
            mixRow->plus_counterfactual.coordinate_count != batch.coordinate_count) {
            std::cerr << "Expected widened measurement rows to expose paired counterfactual witnesses for every sampled coordinate\n";
            return 1;
        }
        if (!(mixRow->minus_counterfactual.mean_abs_residual_delta > 0.0) ||
            !(mixRow->plus_counterfactual.mean_abs_iteration_delta > 0.0)) {
            std::cerr << "Expected paired counterfactual witness metrics to react to widened evidence deltas\n";
            return 1;
        }
        if (!(zoomRow->minus_counterfactual.mean_sample_coord_distance > 0.0) ||
            !(zoomRow->plus_counterfactual.mean_sample_coord_distance > 0.0)) {
            std::cerr << "Expected widened counterfactual witness to read sample_coord displacement for view-driven sweeps\n";
            return 1;
        }
    }

    {
        SidecarHypothesisSpace space{};
        std::string error;
        if (!BuildSidecarHypothesisSpace(BuildCatalog(), "fractal.sample", ctx, &space, &error)) {
            std::cerr << "Expected sidecar hypothesis space to build for coordinate-mismatch test: " << error << "\n";
            return 1;
        }

        EvidenceMeasurementHost host;
        host.mismatch_sample_coord = true;
        SidecarMeasurementBatch batch;
        if (BuildSidecarMeasurementBatch(space, ctx, host, &batch, &error)) {
            std::cerr << "Expected widened measurement batch to fail when widened sample evidence does not preserve the requested sample_coord pairing\n";
            return 1;
        }
        if (error.find("sample_coord") == std::string::npos) {
            std::cerr << "Expected widened measurement mismatch error to mention sample_coord pairing\n";
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

    {
        ViewState legacyView{};
        KernelParams legacyParams{};
        RenderSettings legacyRender{};
        LensSettings legacyLens{};
        legacyView.fractal_type = FractalType::explaino_balance_void;
        legacyView.zoom = 10.0f;
        legacyParams.balance_void = 0.4f;
        legacyParams.symmetry_tension = -0.3f;
        legacyParams.field_curvature = 0.25f;
        BindingContext legacyCtx = MakeBindingContext(&legacyView, &legacyParams, &legacyRender, &legacyLens);

        ViewState canonicalView{};
        KernelParams canonicalParams{};
        RenderSettings canonicalRender{};
        LensSettings canonicalLens{};
        canonicalView.fractal_type = FractalType::explaino_all;
        canonicalView.zoom = 10.0f;
        canonicalParams.balance_void = 0.4f;
        canonicalParams.symmetry_tension = -0.3f;
        canonicalParams.field_curvature = 0.25f;
        BindingContext canonicalCtx = MakeBindingContext(&canonicalView, &canonicalParams, &canonicalRender, &canonicalLens);

        const EngineFunctionCatalog balanceVoidCatalog = BuildBalanceVoidParityCatalog();
        SidecarHypothesisSpace legacySpace{};
        SidecarHypothesisSpace canonicalSpace{};
        std::string error;
        if (!BuildSidecarHypothesisSpace(balanceVoidCatalog, "fractal.sample", legacyCtx, &legacySpace, &error) ||
            !BuildSidecarHypothesisSpace(balanceVoidCatalog, "fractal.sample", canonicalCtx, &canonicalSpace, &error)) {
            std::cerr << "Expected BalanceVoid parity hypothesis spaces to build: " << error << "\n";
            return 1;
        }

        BalanceVoidEvidenceHost host;
        SidecarMeasurementBatch legacyBatch;
        SidecarMeasurementBatch canonicalBatch;
        if (!BuildSidecarMeasurementBatch(legacySpace, legacyCtx, host, &legacyBatch, &error) ||
            !BuildSidecarMeasurementBatch(canonicalSpace, canonicalCtx, host, &canonicalBatch, &error)) {
            std::cerr << "Expected BalanceVoid parity measurement batches to build on existing widened evidence: " << error << "\n";
            return 1;
        }
        if (host.legacy_calls != 0) {
            std::cerr << "Expected BalanceVoid parity proof to stay on widened evidence instead of the legacy sample host\n";
            return 1;
        }
        if (legacySpace.applicable_parameters.size() != 4 || canonicalSpace.applicable_parameters.size() != 4) {
            std::cerr << "Expected BalanceVoid parity hypothesis spaces to expose the canonical fractal selector plus the three shared BalanceVoid axes on both explaino_all and Explaino-BalanceVoid\n";
            return 1;
        }
        if (legacyBatch.rows.size() != 3 || canonicalBatch.rows.size() != 3) {
            std::cerr << "Expected BalanceVoid parity measurement batches to expose the three shared BalanceVoid rows on both explaino_all and Explaino-BalanceVoid\n";
            return 1;
        }

        const char* expectedPaths[] = {
            "fractal.params.balance_void",
            "fractal.params.symmetry_tension",
            "fractal.params.field_curvature",
        };
        for (const char* path : expectedPaths) {
            const SidecarMeasurementRow* legacyRow = FindMeasurementRowByPath(legacyBatch, path);
            const SidecarMeasurementRow* canonicalRow = FindMeasurementRowByPath(canonicalBatch, path);
            if (!legacyRow || !canonicalRow) {
                std::cerr << "Expected BalanceVoid parity measurement rows to keep path " << path << " on both the explicit owner lane and explaino_all\n";
                return 1;
            }
            if (legacyRow->minus_counterfactual.coordinate_count != legacyBatch.coordinate_count ||
                legacyRow->plus_counterfactual.coordinate_count != legacyBatch.coordinate_count ||
                canonicalRow->minus_counterfactual.coordinate_count != canonicalBatch.coordinate_count ||
                canonicalRow->plus_counterfactual.coordinate_count != canonicalBatch.coordinate_count) {
                std::cerr << "Expected BalanceVoid widened witnesses to cover every sampled coordinate for path " << path << "\n";
                return 1;
            }
            if (!NearlyEqual(legacyRow->minus_counterfactual.mean_sample_coord_distance, 0.0) ||
                !NearlyEqual(legacyRow->plus_counterfactual.mean_sample_coord_distance, 0.0) ||
                !NearlyEqual(canonicalRow->minus_counterfactual.mean_sample_coord_distance, 0.0) ||
                !NearlyEqual(canonicalRow->plus_counterfactual.mean_sample_coord_distance, 0.0)) {
                std::cerr << "Expected BalanceVoid param sweeps to preserve sample_coord pairing and react through legacy-result deltas instead of coordinate motion for path " << path << "\n";
                return 1;
            }
            if (!(legacyRow->minus_counterfactual.mean_abs_iteration_delta > 0.0) ||
                !(legacyRow->plus_counterfactual.mean_abs_residual_delta > 0.0) ||
                !(canonicalRow->minus_counterfactual.mean_abs_iteration_delta > 0.0) ||
                !(canonicalRow->plus_counterfactual.mean_abs_residual_delta > 0.0) ||
                !(legacyRow->information_gain_estimate > 0.0) ||
                !(canonicalRow->information_gain_estimate > 0.0)) {
                std::cerr << "Expected existing widened payload to stay sufficient for BalanceVoid axis witnesses on both the owner lane and explaino_all for path " << path << "\n";
                return 1;
            }
        }
        if (!(canonicalBatch.total_information_gain_estimate > 0.0) ||
            !NearlyEqual(canonicalBatch.explored_fraction, 1.0)) {
            std::cerr << "Expected explaino_all widened parity to keep real BalanceVoid measurement rows on the restored shared surface\n";
            return 1;
        }
    }

    std::cout << "test_explaino_sidecar_measurement: all passed\n";
    return 0;
}
