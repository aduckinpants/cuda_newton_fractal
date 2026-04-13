#include "../src/explaino_exploration_advisor.h"
#include "../src/function_descriptor.h"
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

bool IsBaselineExplainoCommonPath(const std::string& path) {
    return path == "fractal.params.explaino_seed" ||
        path == "fractal.view.explaino_seed_drift" ||
        path == "fractal.view.explaino_phase" ||
        path == "fractal.view.explaino_phase_strength" ||
        path == "fractal.params.explaino_damping" ||
        path == "fractal.params.explaino_warp_strength" ||
        path == "fractal.params.explaino_root_spread";
}

} // namespace

int main() {
    ViewState loadView{};
    KernelParams loadParams{};
    RenderSettings loadRender{};
    LensSettings loadLens{};
    loadView.fractal_type = FractalType::explaino;
    BindingContext loadBind = BuildBindingContext(&loadView, &loadParams, &loadRender, &loadLens);

    EngineFunctionCatalog catalog;
    std::string error;
    if (!LoadRealCatalog(loadBind, &catalog, &error)) {
        std::cerr << "Expected real viewer schema catalog to load: " << error << "\n";
        return 1;
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
        BindingContext bind = BuildBindingContext(&view, &params, &render, &lens);

        ContractMeasurementHost host;
        ExplainoExplorationAdvisorReport report;
        if (!BuildExplainoExplorationAdvisorReport(catalog, bind, host, &report, &error)) {
            std::cerr << "Expected advisor report to build for baseline Explaino: " << error << "\n";
            return 1;
        }
        if (report.report_version != 1) {
            std::cerr << "Expected advisor report version 1\n";
            return 1;
        }
        if (report.fractal_type_id != "explaino") {
            std::cerr << "Expected advisor report fractal_type_id to be explaino\n";
            return 1;
        }
        if (!report.has_recommended_observation || report.recommendations.empty()) {
            std::cerr << "Expected advisor report to expose at least one ranked recommendation\n";
            return 1;
        }
        if (report.recommended_observation.rank != 1 || report.recommendations[0].rank != 1) {
            std::cerr << "Expected top advisor recommendation to have rank 1\n";
            return 1;
        }
        if (report.recommended_observation.path != report.recommendations[0].path) {
            std::cerr << "Expected the selected advisor recommendation to match the top ranked entry\n";
            return 1;
        }
        if (!IsBaselineExplainoCommonPath(report.recommended_observation.path)) {
            std::cerr << "Expected baseline Explaino advisor to recommend a common Explaino control, got: "
                      << report.recommended_observation.path << "\n";
            return 1;
        }
        if (report.recommendation_eligible_count <= 0 || report.available_row_count < report.recommendation_eligible_count) {
            std::cerr << "Expected advisor report to carry sane availability counts\n";
            return 1;
        }
        if (report.recommendations[0].summary != "available") {
            std::cerr << "Expected ranked advisor recommendation to carry the energy summary\n";
            return 1;
        }
        if (report.recommendations[0].reason.empty()) {
            std::cerr << "Expected ranked advisor recommendation to carry a human-reviewable reason\n";
            return 1;
        }

        const std::string json = SerializeExplainoExplorationAdvisorReportJson(report);
        json_min::ParseResult parsed = json_min::Parse(json);
        if (!parsed.error.empty() || !parsed.value.is_object()) {
            std::cerr << "Expected serialized advisor report to parse as JSON object\n";
            return 1;
        }
        const json_min::Value* recommendations = parsed.value.get("recommendations");
        if (!recommendations || !recommendations->is_array() || recommendations->as_array().empty()) {
            std::cerr << "Expected serialized advisor report to contain a non-empty recommendations array\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        view.fractal_type = FractalType::mandelbrot;
        BindingContext bind = BuildBindingContext(&view, &params, &render, &lens);

        ContractMeasurementHost host;
        ExplainoExplorationAdvisorReport report;
        if (BuildExplainoExplorationAdvisorReport(catalog, bind, host, &report, &error)) {
            std::cerr << "Expected non-Explaino advisor builds to fail fast\n";
            return 1;
        }
        if (error.find("Explaino") == std::string::npos) {
            std::cerr << "Expected non-Explaino advisor failure to mention Explaino scope\n";
            return 1;
        }
    }

    std::cout << "test_explaino_exploration_advisor: all passed\n";
    return 0;
}