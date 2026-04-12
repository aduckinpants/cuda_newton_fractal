#include "../src/explaino_sidecar_window.h"
#include "../src/function_descriptor.h"
#include "../src/headless_modes.h"
#include "../src/viewer_schema_load.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

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
                std::fabs(params.explaino_seed) * 0.07 +
                std::fabs(static_cast<double>(view.explaino_seed_drift)) * 0.9 +
                std::fabs(static_cast<double>(view.explaino_phase)) * 0.05 +
                std::fabs(static_cast<double>(view.explaino_phase_strength) - 1.0) * 0.06 +
                std::fabs(static_cast<double>(params.explaino_damping) - 1.0f) * 1.4 +
                std::fabs(static_cast<double>(params.explaino_warp_strength)) * 1.1 +
                std::fabs(static_cast<double>(params.explaino_root_spread) - 0.5f) * 0.8 +
                std::fabs(static_cast<double>(params.ripple_amplitude)) * 120.0 +
                std::fabs(static_cast<double>(params.splice_offset)) * 0.5 +
                std::fabs(static_cast<double>(params.vortex_strength)) * 2.0 +
                std::fabs(static_cast<double>(params.tension_strength)) * 18.0;

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

    BindingContext bind;
    bind.view = &view;
    bind.params = &params;
    bind.render = &render;
    bind.lens = &lens;

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
        const FunctionParamDescriptor* explainoSeed = FindParam(*fractalSample, "fractal.params.explaino_seed");
        const FunctionParamDescriptor* rippleAmplitude = FindParam(*fractalSample, "fractal.params.ripple_amplitude");
        if (!explainoSeed || !rippleAmplitude) {
            std::cerr << "Expected real viewer catalog to expose both baseline and variant Explaino params\n";
            return 1;
        }
        if (explainoSeed->has_cost_hint) {
            std::cerr << "Expected baseline Explaino seed to lack cost metadata in the shipped schema-derived catalog\n";
            return 1;
        }
        if (!rippleAmplitude->has_cost_hint) {
            std::cerr << "Expected Explaino Ripple strength to retain shipped cost metadata in the schema-derived catalog\n";
            return 1;
        }
    }

    {
        ContractMeasurementHost host;
        SidecarAutoDemoControllerPolicy policy;
        policy.enabled = true;
        policy.allow_runtime_mutation = true;

        ExplainoSidecarWindowState state;
        if (!BuildExplainoSidecarWindowState(catalog, bind, &host, nullptr, nullptr, nullptr, &policy, &state, &error)) {
            std::cerr << "Expected real-schema baseline Explaino sidecar state to build: " << error << "\n";
            return 1;
        }
        if (state.has_action_recommendation) {
            std::cerr << "Expected baseline Explaino on the real schema-derived surface to have no passive recommendation yet\n";
            return 1;
        }
        if (state.action_error_message.find("no eligible cost-annotated numeric param") == std::string::npos) {
            std::cerr << "Expected baseline Explaino to report the missing-cost recommendation gap, got: "
                      << state.action_error_message << "\n";
            return 1;
        }
        if (state.controller_decision.status != SidecarAutoDemoControllerStatus::blocked_no_action ||
            state.controller_decision.should_mutate) {
            std::cerr << "Expected baseline Explaino controller to remain blocked on the real schema-derived surface\n";
            return 1;
        }
    }

    {
        ContractMeasurementHost host;
        SidecarAutoDemoControllerPolicy policy;
        policy.enabled = true;
        policy.allow_runtime_mutation = true;

        view.fractal_type = FractalType::explaino_ripple;
        params.ripple_amplitude = 0.15f;

        ExplainoSidecarWindowState state;
        if (!BuildExplainoSidecarWindowState(catalog, bind, &host, nullptr, nullptr, nullptr, &policy, &state, &error)) {
            std::cerr << "Expected real-schema Explaino Ripple sidecar state to build: " << error << "\n";
            return 1;
        }
        if (!state.has_action_recommendation) {
            std::cerr << "Expected Explaino Ripple on the real schema-derived surface to expose a passive recommendation: "
                      << state.action_error_message << "\n";
            return 1;
        }
        if (state.action_recommendation.path != "fractal.params.ripple_amplitude") {
            std::cerr << "Expected real-schema Explaino Ripple recommendation to target ripple_amplitude, got: "
                      << state.action_recommendation.path << "\n";
            return 1;
        }
        if (!state.controller_error_message.empty()) {
            std::cerr << "Expected real-schema Explaino Ripple controller evaluation to remain valid, got: "
                      << state.controller_error_message << "\n";
            return 1;
        }
    }

    std::cout << "test_explaino_sidecar_schema_contract: all passed\n";
    return 0;
}