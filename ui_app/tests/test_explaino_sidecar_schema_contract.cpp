#include "../src/explaino_sidecar_window.h"
#include "../src/function_descriptor.h"
#include "../src/headless_modes.h"
#include "../src/viewer_schema_load.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::string ReadTextFile(const char* path) {
    std::ifstream input(path, std::ios::in | std::ios::binary);
    if (!input) {
        return {};
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

bool TryReadTextFileExact(const std::string&, std::string*, std::string*) {
    return false;
}

bool ReadStdinText(std::string*, std::string*) {
    return false;
}

bool WriteTextFileExact(const std::string&, const std::string&, std::string*) {
    return false;
}

namespace {

class ContractMeasurementHost : public SidecarMeasurementHost {
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
            const double coordSpan = std::fabs(coord.x - view.center_hp_x) + std::fabs(coord.y - view.center_hp_y);
            const double explainoSignal =
                std::fabs(params.explaino_seed) * 250.0 +
                std::fabs(params.explaino_seed_b) * 250.0 +
                std::fabs(static_cast<double>(view.explaino_seed_drift)) * 220.0 +
                std::fabs(static_cast<double>(view.explaino_phase)) * 40.0 +
                std::fabs(static_cast<double>(view.explaino_phase_strength) - 1.0) * 15.0 +
                std::fabs(static_cast<double>(params.explaino_damping) - 1.0f) * 25.0 +
                std::fabs(static_cast<double>(params.explaino_warp_strength)) * 20.0 +
                std::fabs(static_cast<double>(params.explaino_root_spread) - 0.5f) * 20.0 +
                std::fabs(static_cast<double>(params.explaino_mix) - 0.5) * 140.0 +
                std::fabs(static_cast<double>(params.explaino_cluster_radius)) * 120.0 +
                std::fabs(static_cast<double>(params.momentum_beta)) * 140.0 +
                std::fabs(static_cast<double>(params.joy_coupling)) * 160.0 +
                std::fabs(static_cast<double>(params.fold_coupling)) * 150.0 +
                std::fabs(static_cast<double>(params.bell_coupling)) * 150.0 +
                std::fabs(static_cast<double>(params.ripple_amplitude)) * 120.0 +
                std::fabs(static_cast<double>(params.splice_offset)) * 80.0 +
                std::fabs(static_cast<double>(params.vortex_strength)) * 120.0 +
                std::fabs(static_cast<double>(params.tension_strength)) * 600.0;

            FractalSampleResult sample{};
            sample.iterations = static_cast<int>(std::lround(40.0 + explainoSignal * 18.0 + coordSpan * 12.0 + static_cast<double>(render.device_id)));
            sample.final_z_x = static_cast<float>(coord.x + explainoSignal * 0.01);
            sample.final_z_y = static_cast<float>(coord.y - explainoSignal * 0.01);
            sample.residual = static_cast<float>(1.0 + explainoSignal + coordSpan * 0.5);
            sample.converged = explainoSignal < 3.0;
            sample.escaped = !sample.converged && coordSpan > 0.02;
            outResults->push_back(sample);
        }
        return true;
    }
};

const FunctionDescriptor* FindFunction(const EngineFunctionCatalog& catalog, const std::string& functionId) {
    for (const auto& function : catalog.functions) {
        if (function.id == functionId) {
            return &function;
        }
    }
    return nullptr;
}

const FunctionParamDescriptor* FindParam(const FunctionDescriptor& function, const std::string& path) {
    for (const auto& param : function.parameters) {
        if (param.path == path) {
            return &param;
        }
    }
    return nullptr;
}

bool HasOptionId(const FunctionParamDescriptor& param, const char* optionId) {
    if (!optionId) {
        return false;
    }
    for (const auto& option : param.options) {
        if (option.id == optionId) {
            return true;
        }
    }
    return false;
}

bool LoadRealCatalog(BindingContext& bind, EngineFunctionCatalog* outCatalog, std::string* outError) {
    if (!outCatalog) {
        if (outError) *outError = "LoadRealCatalog requires outCatalog";
        return false;
    }

    const std::vector<std::string> candidates = {
        "..\\ui\\fractal_binding_surface_v1.ui_schema.json",
    };

    SchemaLoadResult schemaResult = LoadAndValidateViewerSchema(candidates, bind, false);
    if (schemaResult.fatal_error) {
        if (outError) *outError = "Failed to validate real viewer schema";
        return false;
    }
    if (!schemaResult.from_file) {
        if (outError) *outError = "Real viewer schema did not load from file";
        return false;
    }
    if (!schemaResult.warning.empty()) {
        if (outError) *outError = "Real viewer schema unexpectedly entered Safe Mode: " + schemaResult.warning;
        return false;
    }

    *outCatalog = BuildEngineCatalog(schemaResult.schema);
    return true;
}

bool IsBaselineExplainoCommonPath(const std::string& path) {
    return path == "fractal.params.explaino_seed" ||
        path == "fractal.view.explaino_seed_drift" ||
        path == "fractal.view.explaino_phase" ||
        path == "fractal.view.explaino_phase_strength" ||
        path == "fractal.params.explaino_damping" ||
        path == "fractal.params.explaino_warp_strength" ||
        path == "fractal.params.explaino_root_spread";
}

BindingContext BuildBindingContext(
    ViewState* view,
    KernelParams* params,
    RenderSettings* render,
    LensSettings* lens) {
    BindingContext bind;
    bind.view = view;
    bind.params = params;
    bind.render = render;
    bind.lens = lens;
    return bind;
}

double ReadBoundNumericValue(const BindingContext& bind, const std::string& path, const std::string& type) {
    if (type == "double") {
        double value = 0.0;
        if (bind.GetDoubleValue(path, value)) {
            return value;
        }
    }
    if (type == "float") {
        float value = 0.0f;
        if (bind.GetFloatValue(path, value)) {
            return static_cast<double>(value);
        }
    }
    if (type == "int") {
        int value = 0;
        if (bind.GetIntValue(path, value)) {
            return static_cast<double>(value);
        }
    }
    return 0.0;
}

double CaptureResidual(
    const ContractMeasurementHost& host,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render) {
    std::vector<Double2> coords = {{view.center_hp_x, view.center_hp_y}};
    std::vector<FractalSampleResult> results;
    std::string error;
    if (!host.Sample(coords, view, params, render, &results, &error) || results.size() != 1) {
        return -1.0;
    }
    return static_cast<double>(results[0].residual);
}

} // namespace

int main() {
    ViewState loadView{};
    KernelParams loadParams{};
    RenderSettings loadRender{};
    LensSettings loadLens{};
    loadView.fractal_type = FractalType::explaino;
    BindingContext bind = BuildBindingContext(&loadView, &loadParams, &loadRender, &loadLens);

    EngineFunctionCatalog catalog;
    std::string error;
    if (!LoadRealCatalog(bind, &catalog, &error)) {
        std::cerr << "Expected real viewer schema catalog to load: " << error << "\n";
        return 1;
    }

    const FunctionDescriptor* fractalSample = FindFunction(catalog, "fractal.sample");
    if (!fractalSample) {
        std::cerr << "Expected real viewer catalog to describe fractal.sample\n";
        return 1;
    }
    {
        const FunctionParamDescriptor* fractalType = FindParam(*fractalSample, "fractal.view.fractal_type");
        if (!fractalType) {
            std::cerr << "Expected real viewer catalog to expose fractal.view.fractal_type for the sidecar path\n";
            return 1;
        }
        if (!HasOptionId(*fractalType, "explaino_all")) {
            std::cerr << "Expected the shipped schema-derived sidecar catalog to advertise explaino_all as a supported fractal.sample enum option\n";
            return 1;
        }
    }

    {
        const FunctionParamDescriptor* explainoSeed = FindParam(*fractalSample, "fractal.params.explaino_seed");
        const FunctionParamDescriptor* rippleAmplitude = FindParam(*fractalSample, "fractal.params.ripple_amplitude");
        const FunctionParamDescriptor* explainoMix = FindParam(*fractalSample, "fractal.params.explaino_mix");
        const FunctionParamDescriptor* momentumBeta = FindParam(*fractalSample, "fractal.params.momentum_beta");
        const FunctionParamDescriptor* joyCoupling = FindParam(*fractalSample, "fractal.params.joy_coupling");
        const FunctionParamDescriptor* foldCoupling = FindParam(*fractalSample, "fractal.params.fold_coupling");
        const FunctionParamDescriptor* bellCoupling = FindParam(*fractalSample, "fractal.params.bell_coupling");
        if (!explainoSeed || !rippleAmplitude || !explainoMix || !momentumBeta || !joyCoupling || !foldCoupling || !bellCoupling) {
            std::cerr << "Expected real viewer catalog to expose baseline, dual, inertial, and explaino-family variant params\n";
            return 1;
        }
        if (!explainoSeed->has_cost_hint) {
            std::cerr << "Expected baseline Explaino seed to carry sidecar cost metadata in the shipped schema-derived catalog\n";
            return 1;
        }
        if (!rippleAmplitude->has_cost_hint) {
            std::cerr << "Expected Explaino Ripple strength to retain shipped cost metadata in the schema-derived catalog\n";
            return 1;
        }
        if (!explainoMix->has_cost_hint || !momentumBeta->has_cost_hint || !joyCoupling->has_cost_hint ||
            !foldCoupling->has_cost_hint || !bellCoupling->has_cost_hint) {
            std::cerr << "Expected Explaino-family variant controls to carry sidecar cost metadata in the shipped schema-derived catalog\n";
            return 1;
        }
        if (explainoSeed->has_sensitivity_report || joyCoupling->has_sensitivity_report ||
            foldCoupling->has_sensitivity_report || bellCoupling->has_sensitivity_report ||
            momentumBeta->has_sensitivity_report || explainoMix->has_sensitivity_report) {
            std::cerr << "Expected generic Explaino-family sidecar cost metadata to remain distinct from measured sensitivity reports\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        view.fractal_type = FractalType::explaino;
        view.zoom = 10.0f;
        params.explaino_seed = 7.0;
        view.explaino_seed_drift = 0.125f;
        BindingContext stateBind = BuildBindingContext(&view, &params, &render, &lens);

        ContractMeasurementHost host;
        SidecarAutoDemoControllerPolicy policy;
        policy.enabled = true;
        policy.allow_runtime_mutation = true;

        ExplainoSidecarWindowState state;
        if (!BuildExplainoSidecarWindowState(catalog, stateBind, &host, nullptr, nullptr, nullptr, &policy, &state, &error)) {
            std::cerr << "Expected real-schema baseline Explaino sidecar state to build: " << error << "\n";
            return 1;
        }
        if (!state.has_action_recommendation) {
            std::cerr << "Expected baseline Explaino on the real schema-derived surface to expose an actionable recommendation: "
                      << state.action_error_message << "\n";
            return 1;
        }
        if (!IsBaselineExplainoCommonPath(state.action_recommendation.path)) {
            std::cerr << "Expected baseline Explaino recommendation to target a real common Explaino control after the real-schema repair, got: "
                      << state.action_recommendation.path << "\n";
            return 1;
        }
        if (state.controller_decision.status != SidecarAutoDemoControllerStatus::apply_ready ||
            !state.controller_decision.should_mutate) {
            std::cerr << "Expected baseline Explaino controller to arm a real-schema mutation-ready decision"
                      << " status=" << static_cast<int>(state.controller_decision.status)
                      << " should_mutate=" << state.controller_decision.should_mutate
                      << " reason=" << state.controller_decision.reason << "\n";
            return 1;
        }

        const double beforeValue = ReadBoundNumericValue(stateBind, state.controller_decision.path, state.controller_decision.type);
        const double beforeResidual = CaptureResidual(host, view, params, render);
        if (!ApplySidecarAutoDemoControllerDecision(state.controller_decision, stateBind, &error)) {
            std::cerr << "Expected baseline Explaino real-schema controller decision to mutate the bound state: " << error << "\n";
            return 1;
        }
        const double afterValue = ReadBoundNumericValue(stateBind, state.controller_decision.path, state.controller_decision.type);
        const double afterResidual = CaptureResidual(host, view, params, render);
        if (beforeValue == afterValue) {
            std::cerr << "Expected baseline Explaino auto-demo mutation to change the bound param value\n";
            return 1;
        }
        if (beforeResidual == afterResidual) {
            std::cerr << "Expected baseline Explaino auto-demo mutation to change sampled output residual\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        view.fractal_type = FractalType::explaino_all;
        view.zoom = 10.0f;
        params.explaino_seed = 7.0;
        view.explaino_seed_drift = 0.125f;
        BindingContext stateBind = BuildBindingContext(&view, &params, &render, &lens);

        ContractMeasurementHost host;
        SidecarAutoDemoControllerPolicy policy;
        policy.enabled = true;
        policy.allow_runtime_mutation = true;

        ExplainoSidecarWindowState state;
        if (!BuildExplainoSidecarWindowState(catalog, stateBind, &host, nullptr, nullptr, nullptr, &policy, &state, &error)) {
            std::cerr << "Expected canonical explaino_all sidecar state to build on the real schema-derived surface: " << error << "\n";
            return 1;
        }
        if (!state.error_message.empty()) {
            std::cerr << "Expected canonical explaino_all sidecar state to load without a model error, got: "
                      << state.error_message << "\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        view.fractal_type = FractalType::explaino_joy;
        view.zoom = 10.0f;
        params.explaino_seed = 7.0;
        view.explaino_seed_drift = 0.125f;
        params.joy_coupling = 0.3f;
        BindingContext stateBind = BuildBindingContext(&view, &params, &render, &lens);

        ContractMeasurementHost host;
        SidecarAutoDemoControllerPolicy policy;
        policy.enabled = true;
        policy.allow_runtime_mutation = true;

        ExplainoSidecarWindowState state;
        if (!BuildExplainoSidecarWindowState(catalog, stateBind, &host, nullptr, nullptr, nullptr, &policy, &state, &error)) {
            std::cerr << "Expected real-schema Explaino Joy sidecar state to build: " << error << "\n";
            return 1;
        }
        if (!state.has_action_recommendation) {
            std::cerr << "Expected Explaino Joy on the real schema-derived surface to expose an actionable recommendation: "
                      << state.action_error_message << "\n";
            return 1;
        }
        if (state.action_recommendation.path != "fractal.params.joy_coupling") {
            std::cerr << "Expected real-schema Explaino Joy recommendation to target joy_coupling, got: "
                      << state.action_recommendation.path << "\n";
            return 1;
        }
        if (state.controller_decision.status != SidecarAutoDemoControllerStatus::apply_ready ||
            !state.controller_decision.should_mutate) {
            std::cerr << "Expected real-schema Explaino Joy controller to arm a mutation-ready decision"
                      << " status=" << static_cast<int>(state.controller_decision.status)
                      << " should_mutate=" << state.controller_decision.should_mutate
                      << " reason=" << state.controller_decision.reason << "\n";
            return 1;
        }

        const double beforeValue = ReadBoundNumericValue(stateBind, state.controller_decision.path, state.controller_decision.type);
        const double beforeResidual = CaptureResidual(host, view, params, render);
        if (!ApplySidecarAutoDemoControllerDecision(state.controller_decision, stateBind, &error)) {
            std::cerr << "Expected Explaino Joy real-schema controller decision to mutate the bound state: " << error << "\n";
            return 1;
        }
        const double afterValue = ReadBoundNumericValue(stateBind, state.controller_decision.path, state.controller_decision.type);
        const double afterResidual = CaptureResidual(host, view, params, render);
        if (beforeValue == afterValue) {
            std::cerr << "Expected Explaino Joy auto-demo mutation to change joy_coupling\n";
            return 1;
        }
        if (beforeResidual == afterResidual) {
            std::cerr << "Expected Explaino Joy auto-demo mutation to change sampled output residual\n";
            return 1;
        }
    }

    std::cout << "test_explaino_sidecar_schema_contract: all passed\n";
    return 0;
}