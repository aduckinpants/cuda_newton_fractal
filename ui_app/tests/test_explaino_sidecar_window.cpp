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
    explainoMix.has_cost_hint = true;
    explainoMix.cost_hint = 0.75;
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

EngineFunctionCatalog BuildInvalidCostHintCatalog() {
    EngineFunctionCatalog catalog = BuildCatalog();
    catalog.functions[0].parameters[2].cost_hint = -0.25;
    return catalog;
}

SidecarBudgetState BuildCompleteBudget(const SidecarBudgetState& budget) {
    SidecarBudgetState next = budget;
    next.mean_posterior_uncertainty = 0.0;
    for (auto& row : next.rows) {
        row.posterior_uncertainty = 0.0;
        row.observation_count = 2;
    }
    return next;
}

SidecarExplorationCompleteness BuildCompleteCompleteness(
    const SidecarExplorationCompleteness& completeness) {
    SidecarExplorationCompleteness next = completeness;
    next.demonstrated_count = static_cast<int>(next.rows.size());
    next.uncertain_count = 0;
    next.demonstrated_fraction = 1.0;
    next.mean_coverage_score = 1.0;
    for (auto& row : next.rows) {
        row.posterior_uncertainty = 0.0;
        row.observation_count = 2;
        row.coverage_score = 1.0;
        row.demonstrated = true;
        row.coverage_bucket = "demonstrated";
    }
    return next;
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
        if (!state.has_orientation) {
            std::cerr << "Expected successful sidecar window builds to expose a reusable orientation\n";
            return 1;
        }
        if (state.measurement.rows.size() != 2) {
            std::cerr << "Expected measurement rows for zoom and explaino_mix\n";
            return 1;
        }
        if (state.divergence.status != SidecarStateDivergenceStatus::unavailable ||
            !state.divergence_error_message.empty()) {
            std::cerr << "Expected first sidecar measurement build to expose an explicit unavailable divergence state\n";
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
        if (state.lens.rows.size() != state.budget.rows.size()) {
            std::cerr << "Expected sidecar window state to expose lens rows alongside the ranked budget rows\n";
            return 1;
        }
        if (state.completeness.rows.size() != state.budget.rows.size()) {
            std::cerr << "Expected sidecar window state to expose completeness rows alongside the tracked budget rows\n";
            return 1;
        }
        if (state.completeness.demonstrated_count != 0 || state.completeness.uncertain_count != static_cast<int>(state.completeness.rows.size())) {
            std::cerr << "Expected first measurement batch to leave the sidecar completeness surface fully uncertain\n";
            return 1;
        }
        if (state.completeness.rows[0].coverage_bucket.empty()) {
            std::cerr << "Expected sidecar completeness rows to carry a rendered bucket label\n";
            return 1;
        }
        if (state.lens.rows[0].path != state.budget.rows[0].path || state.lens.rows[0].guidance.empty()) {
            std::cerr << "Expected top-ranked lens guidance to align with the top-ranked budget row\n";
            return 1;
        }
        if (!state.has_action_recommendation) {
            std::cerr << "Expected sidecar window state to expose a passive action recommendation when cost hints are available: "
                      << state.action_error_message << "\n";
            return 1;
        }
        if (state.controller_decision.status != SidecarAutoDemoControllerStatus::disabled ||
            state.controller_decision.should_mutate) {
            std::cerr << "Expected sidecar window controller state to stay disabled by default\n";
            return 1;
        }
        if (state.action_recommendation.path != "fractal.params.explaino_mix") {
            std::cerr << "Expected Explaino Mix to be the recommended next action in the fake measurement fixture, got: "
                      << state.action_recommendation.path << "\n";
            return 1;
        }
        if (state.action_recommendation.guidance.empty() || !state.action_error_message.empty()) {
            std::cerr << "Expected successful action recommendation builds to carry guidance and no action error\n";
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
        if (!first.has_orientation) {
            std::cerr << "Expected initial sidecar window state to expose a reusable orientation\n";
            return 1;
        }
        if (!BuildExplainoSidecarWindowState(BuildCatalog(), ctx, &host, &first.budget, &first.completeness, &first.orientation, nullptr, &second, &error)) {
            std::cerr << "Expected repeated sidecar window state build to preserve budget state: " << error << "\n";
            return 1;
        }
        if (second.divergence.status != SidecarStateDivergenceStatus::stable ||
            !NearlyEqual(second.divergence.scalar_divergence, 0.0) ||
            !second.divergence_error_message.empty()) {
            std::cerr << "Expected repeated builds without fractal_type or seed change to keep divergence stable\n";
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
        if (!(second.completeness.demonstrated_count > first.completeness.demonstrated_count)) {
            std::cerr << "Expected repeated sidecar window builds to move params from uncertain toward demonstrated coverage\n";
            return 1;
        }
        if (!(second.completeness.demonstrated_fraction > first.completeness.demonstrated_fraction)) {
            std::cerr << "Expected repeated sidecar window builds to improve demonstrated coverage fraction\n";
            return 1;
        }

        params.explaino_seed = 8.0;
        ExplainoSidecarWindowState third;
        if (!BuildExplainoSidecarWindowState(BuildCatalog(), ctx, &host, &second.budget, &second.completeness, &second.orientation, nullptr, &third, &error)) {
            std::cerr << "Expected seed-change sidecar window state build to succeed: " << error << "\n";
            return 1;
        }
        if (third.divergence.status != SidecarStateDivergenceStatus::diverged ||
            !(third.divergence.scalar_divergence > 0.0) ||
            !third.divergence.import_changed ||
            !third.divergence_error_message.empty()) {
            std::cerr << "Expected explaino seed changes to produce a positive divergence indicator\n";
            return 1;
        }
        params.explaino_seed = 7.0;
    }

    {
        FakeMeasurementHost host;
        host.fail = true;
        ExplainoSidecarWindowState state;
        SidecarAutoDemoControllerPolicy policy;
        policy.enabled = true;
        policy.allow_runtime_mutation = true;
        std::string error;
        if (BuildExplainoSidecarWindowState(BuildCatalog(), ctx, &host, nullptr, nullptr, &policy, &state, &error)) {
            std::cerr << "Expected initial sidecar window state build to fail when the measurement host fails\n";
            return 1;
        }
        if (error.find("fake measurement host failure") == std::string::npos) {
            std::cerr << "Expected initial measurement failure to mention the host failure\n";
            return 1;
        }
        if (state.controller_error_message.find("completeness function_id") == std::string::npos) {
            std::cerr << "Expected initial measurement failures to expose an explicit controller-unavailable error instead of a blank controller state\n";
            return 1;
        }
    }

    {
        FakeMeasurementHost host;
        host.fail = true;
        ExplainoSidecarWindowState seeded;
        ExplainoSidecarWindowState state;
        SidecarAutoDemoControllerPolicy policy;
        policy.enabled = true;
        policy.allow_runtime_mutation = true;
        std::string error;
        host.fail = false;
        if (!BuildExplainoSidecarWindowState(BuildCatalog(), ctx, &host, &seeded, &error)) {
            std::cerr << "Expected seeded sidecar window state to build before failure preservation test: " << error << "\n";
            return 1;
        }
        host.fail = true;
        if (BuildExplainoSidecarWindowState(BuildCatalog(), ctx, &host, &seeded.budget, &seeded.completeness, &policy, &state, &error)) {
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
        if (state.completeness.rows.size() != seeded.completeness.rows.size() ||
            state.completeness.demonstrated_count != seeded.completeness.demonstrated_count ||
            state.completeness.uncertain_count != seeded.completeness.uncertain_count) {
            std::cerr << "Expected sidecar window measurement failures to preserve the last known completeness surface\n";
            return 1;
        }
        if (state.controller_decision.status != SidecarAutoDemoControllerStatus::blocked_no_action ||
            state.controller_decision.should_mutate ||
            !state.controller_error_message.empty()) {
            std::cerr << "Expected measurement failures with preserved incomplete completeness to expose an explicit blocked controller state\n";
            return 1;
        }
        if (!state.lens.rows.empty()) {
            std::cerr << "Expected failed measurement updates to leave no derived lens projection rows\n";
            return 1;
        }
    }

    {
        FakeMeasurementHost host;
        ExplainoSidecarWindowState seeded;
        ExplainoSidecarWindowState state;
        SidecarAutoDemoControllerPolicy policy;
        policy.enabled = true;
        policy.allow_runtime_mutation = true;
        std::string error;
        if (!BuildExplainoSidecarWindowState(BuildCatalog(), ctx, &host, &seeded, &error)) {
            std::cerr << "Expected seeded sidecar window state to build before controller stop preservation test: " << error << "\n";
            return 1;
        }

        SidecarBudgetState completeBudget = BuildCompleteBudget(seeded.budget);
        SidecarExplorationCompleteness completeCompleteness = BuildCompleteCompleteness(seeded.completeness);
        host.fail = true;
        if (BuildExplainoSidecarWindowState(BuildCatalog(), ctx, &host, &completeBudget, &completeCompleteness, &policy, &state, &error)) {
            std::cerr << "Expected sidecar window state build to fail when measurement host fails during controller stop preservation test\n";
            return 1;
        }
        if (state.controller_decision.status != SidecarAutoDemoControllerStatus::stopped_complete ||
            state.controller_decision.should_mutate ||
            !state.controller_error_message.empty()) {
            std::cerr << "Expected preserved complete completeness to keep the controller in explicit stopped state during measurement failures\n";
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
        if (BuildExplainoSidecarWindowState(BuildDuplicateMeasurementCatalog(), ctx, &host, &seeded.budget, &seeded.completeness, &state, &error)) {
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
        if (state.completeness.rows.size() != seeded.completeness.rows.size() ||
            state.completeness.demonstrated_count != seeded.completeness.demonstrated_count ||
            state.completeness.uncertain_count != seeded.completeness.uncertain_count) {
            std::cerr << "Expected budget update failures to preserve the last known completeness surface\n";
            return 1;
        }
        if (!state.lens.rows.empty()) {
            std::cerr << "Expected failed budget updates to leave no derived lens projection rows\n";
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
        if (state.has_orientation) {
            std::cerr << "Expected model-error sidecar state to reject orientation reuse\n";
            return 1;
        }
    }

    {
        FakeMeasurementHost host;
        ExplainoSidecarWindowState broken;
        std::string error;
        if (BuildExplainoSidecarWindowState(BuildBrokenCatalog(), ctx, &broken, &error)) {
            std::cerr << "Expected broken sidecar window state build to fail before recovery test\n";
            return 1;
        }

        ExplainoSidecarWindowState recovered;
        error.clear();
        if (!BuildExplainoSidecarWindowState(
                BuildCatalog(),
                ctx,
                &host,
                nullptr,
                nullptr,
                broken.has_orientation ? &broken.orientation : nullptr,
                nullptr,
                &recovered,
                &error)) {
            std::cerr << "Expected sidecar window state to recover after a prior model failure: " << error << "\n";
            return 1;
        }
        if (!recovered.has_orientation) {
            std::cerr << "Expected recovered sidecar window state to restore a reusable orientation\n";
            return 1;
        }
        if (recovered.divergence.status != SidecarStateDivergenceStatus::unavailable ||
            !recovered.divergence_error_message.empty()) {
            std::cerr << "Expected recovery after a prior model-error state to treat divergence as unavailable, not compare against zero orientation\n";
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

    {
        ExplainoSidecarWindowState state;
        std::string error;
        if (BuildExplainoSidecarWindowState(BuildInvalidCostHintCatalog(), ctx, &state, &error)) {
            std::cerr << "Expected invalid sidecar cost_hint metadata to fail window-state build\n";
            return 1;
        }
        if (error.find("cost_hint") == std::string::npos || error.find("fractal.params.explaino_mix") == std::string::npos) {
            std::cerr << "Expected sidecar window error to mention the invalid cost_hint path\n";
            return 1;
        }
        if (state.error_message.find("cost_hint") == std::string::npos) {
            std::cerr << "Expected sidecar window state to retain the invalid cost_hint error message\n";
            return 1;
        }
    }

    std::cout << "test_explaino_sidecar_window: all passed\n";
    return 0;
}