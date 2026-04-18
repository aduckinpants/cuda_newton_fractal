#include "../src/runtime_walk_field_slime.h"

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

static int CountOccurrences(const std::string& text, const std::string& needle) {
    int count = 0;
    std::size_t pos = 0;
    while ((pos = text.find(needle, pos)) != std::string::npos) {
        ++count;
        pos += needle.size();
    }
    return count;
}

static int g_passed = 0;
static int g_failed = 0;

static void Check(bool cond, const char* name) {
    if (cond) {
        std::printf("  PASS: %s\n", name);
        ++g_passed;
    } else {
        std::printf("  FAIL: %s\n", name);
        ++g_failed;
    }
}

static bool NearlyEqual(double lhs, double rhs, double eps = 1.0e-9) {
    return std::fabs(lhs - rhs) <= eps;
}

class FakeFieldMeasurementHost : public SidecarMeasurementHost {
public:
    bool emit_nonfinite = false;
    double residual_scale = 1.0;

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
        for (std::size_t index = 0; index < coords.size(); ++index) {
            const Double2 coord = coords[index];
            FractalSampleResult sample{};
            sample.iterations = 20 + static_cast<int>(std::lround(10.0 * std::fabs(coord.x + coord.y)));
            sample.final_z_x = static_cast<float>(coord.x);
            sample.final_z_y = static_cast<float>(coord.y);
            sample.residual = static_cast<float>(0.5 + residual_scale * (coord.x * coord.x + 2.0 * coord.y * coord.y) + 0.1 * params.explaino_mix);
            sample.converged = sample.residual < 2.0f;
            sample.escaped = !sample.converged;
            if (emit_nonfinite && index == 0u) sample.residual = INFINITY;
            outResults->push_back(sample);
        }
        return true;
    }
};

static RuntimeWalkFieldSlimeConfig BuildConfig() {
    RuntimeWalkFieldSlimeConfig config{};
    config.min_marbles = 8;
    config.max_marbles = 32;
    config.max_steps = 2;
    config.grid_resolution = 16;
    config.gradient_sensitivity = 1.0;
    config.hysteresis = 0.35;
    config.export_cadence = 1;
    return config;
}

static void BuildRuntime(ViewState* view, KernelParams* params, RenderSettings* render) {
    *view = {};
    *params = {};
    *render = {};
    view->fractal_type = FractalType::explaino;
    view->center_hp_x = 0.0;
    view->center_hp_y = 0.0;
    view->log2_zoom = 0.0;
    params->explaino_mix = 0.5f;
    render->resolution = {320, 240};
}

static void TestMarbleSeedingIsDeterministic() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    BuildRuntime(&view, &params, &render);

    RuntimeWalkFieldSlimeState first;
    RuntimeWalkFieldSlimeState second;
    std::string error;
    Check(InitializeRuntimeWalkFieldSlime(BuildConfig(), view, params, render, 1234u, &first, &error),
        "TestMarbleSeedingIsDeterministic_First");
    Check(InitializeRuntimeWalkFieldSlime(BuildConfig(), view, params, render, 1234u, &second, &error),
        "TestMarbleSeedingIsDeterministic_Second");
    Check(first.marbles.size() == second.marbles.size() && !first.marbles.empty(),
        "TestMarbleSeedingIsDeterministic_Count");
    Check(NearlyEqual(first.marbles[3].world.x, second.marbles[3].world.x) &&
            NearlyEqual(first.marbles[3].world.y, second.marbles[3].world.y),
        "TestMarbleSeedingIsDeterministic_Position");
}


static void TestFieldStateRequestsResetWhenSessionSeedChanges() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    BuildRuntime(&view, &params, &render);

    RuntimeWalkFieldSlimeState state;
    std::string error;
    Check(InitializeRuntimeWalkFieldSlime(BuildConfig(), view, params, render, 101u, &state, &error),
        "TestFieldStateRequestsResetWhenSessionSeedChanges_Init");
    Check(!RuntimeWalkFieldSlimeNeedsResetForSeed(state, 101u),
        "TestFieldStateRequestsResetWhenSessionSeedChanges_SameSeedStable");
    Check(RuntimeWalkFieldSlimeNeedsResetForSeed(state, 202u),
        "TestFieldStateRequestsResetWhenSessionSeedChanges_NewSeedResets");
}

static void TestAdaptiveSamplingRespondsToGradientAndBounds() {
    RuntimeWalkFieldSlimeConfig config = BuildConfig();
    const int quiet = ComputeRuntimeWalkFieldSlimeAdaptiveMarbleCount(config, 0.05, 0.05, 0.05, 0.05);
    const int active = ComputeRuntimeWalkFieldSlimeAdaptiveMarbleCount(config, 3.0, 1.5, 1.0, 0.8);
    Check(quiet >= config.min_marbles && quiet <= config.max_marbles,
        "TestAdaptiveSamplingRespondsToGradientAndBounds_QuietBounded");
    Check(active >= config.min_marbles && active <= config.max_marbles,
        "TestAdaptiveSamplingRespondsToGradientAndBounds_ActiveBounded");
    Check(active > quiet,
        "TestAdaptiveSamplingRespondsToGradientAndBounds_Increases");
}

static void TestFieldStepAdaptsMarblePopulationToMeasuredGradient() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    BuildRuntime(&view, &params, &render);

    RuntimeWalkFieldSlimeConfig config = BuildConfig();
    config.gradient_sensitivity = 4.0;
    RuntimeWalkFieldSlimeState state;
    std::string error;
    Check(InitializeRuntimeWalkFieldSlime(config, view, params, render, 303u, &state, &error),
        "TestFieldStepAdaptsMarblePopulationToMeasuredGradient_Init");
    const std::size_t initialCount = state.marbles.size();

    FakeFieldMeasurementHost host;
    host.residual_scale = 120.0;
    Check(StepRuntimeWalkFieldSlime(host, config, view, params, render, 0.3, &state, &error),
        "TestFieldStepAdaptsMarblePopulationToMeasuredGradient_Step");
    Check(state.actual_sample_count == static_cast<int>(state.marbles.size()),
        "TestFieldStepAdaptsMarblePopulationToMeasuredGradient_CountRecorded");
    Check(state.marbles.size() > initialCount && state.marbles.size() <= static_cast<std::size_t>(config.max_marbles),
        "TestFieldStepAdaptsMarblePopulationToMeasuredGradient_IncreasesPopulation");
}

static void TestExportCadenceMarksDueFrames() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    BuildRuntime(&view, &params, &render);

    RuntimeWalkFieldSlimeConfig config = BuildConfig();
    config.export_cadence = 2;
    RuntimeWalkFieldSlimeState state;
    std::string error;
    Check(InitializeRuntimeWalkFieldSlime(config, view, params, render, 404u, &state, &error),
        "TestExportCadenceMarksDueFrames_Init");
    FakeFieldMeasurementHost host;
    Check(StepRuntimeWalkFieldSlime(host, config, view, params, render, 0.1, &state, &error),
        "TestExportCadenceMarksDueFrames_FirstStep");
    Check(state.export_sequence == 1 && !state.export_due,
        "TestExportCadenceMarksDueFrames_FirstNotDue");
    Check(StepRuntimeWalkFieldSlime(host, config, view, params, render, 0.2, &state, &error),
        "TestExportCadenceMarksDueFrames_SecondStep");
    Check(state.export_sequence == 2 && state.export_due,
        "TestExportCadenceMarksDueFrames_SecondDue");
}

static void TestFieldStepProducesStableTravelerAndRejectsNonfinite() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    BuildRuntime(&view, &params, &render);

    RuntimeWalkFieldSlimeState state;
    std::string error;
    Check(InitializeRuntimeWalkFieldSlime(BuildConfig(), view, params, render, 77u, &state, &error),
        "TestFieldStepProducesStableTravelerAndRejectsNonfinite_Init");

    FakeFieldMeasurementHost host;
    host.emit_nonfinite = true;
    Check(StepRuntimeWalkFieldSlime(host, BuildConfig(), view, params, render, 0.25, &state, &error),
        "TestFieldStepProducesStableTravelerAndRejectsNonfinite_Step");
    Check(state.traveler.marble_count > 0,
        "TestFieldStepProducesStableTravelerAndRejectsNonfinite_TravelerHasCluster");
    Check(std::isfinite(state.traveler.centroid_world.x) && std::isfinite(state.traveler.centroid_world.y),
        "TestFieldStepProducesStableTravelerAndRejectsNonfinite_TravelerFinite");
    bool sawNonfiniteStop = false;
    for (const RuntimeWalkFieldSlimeMarble& marble : state.marbles) {
        if (marble.stop_reason == "nonfinite_sample") sawNonfiniteStop = true;
    }
    Check(sawNonfiniteStop,
        "TestFieldStepProducesStableTravelerAndRejectsNonfinite_NonfiniteRecorded");
}

static void TestFieldCsvExportsTravelerRepresentative() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    BuildRuntime(&view, &params, &render);

    RuntimeWalkFieldSlimeState state;
    std::string error;
    Check(InitializeRuntimeWalkFieldSlime(BuildConfig(), view, params, render, 99u, &state, &error),
        "TestFieldCsvExportsTravelerRepresentative_Init");
    FakeFieldMeasurementHost host;
    Check(StepRuntimeWalkFieldSlime(host, BuildConfig(), view, params, render, 0.5, &state, &error),
        "TestFieldCsvExportsTravelerRepresentative_Step");

    const std::filesystem::path root = std::filesystem::temp_directory_path() / "cuda_newton_runtime_walk_field_slime_tests";
    std::error_code ec;
    std::filesystem::create_directories(root, ec);
    const std::filesystem::path flowPath = root / "runtime_walk_flow_lines.csv";
    const std::filesystem::path cellsPath = root / "runtime_field_cells.csv";
    std::filesystem::remove(flowPath, ec);
    std::filesystem::remove(cellsPath, ec);
    RuntimeWalkFieldSlimeExportContext exportContext{};
    exportContext.fits_frame_index = 7;
    Check(WriteRuntimeWalkFieldSlimeCsv(state, flowPath.string(), cellsPath.string(), &error, &exportContext),
        "TestFieldCsvExportsTravelerRepresentative_Write");

    std::ifstream flow(flowPath, std::ios::in | std::ios::binary);
    std::ifstream cells(cellsPath, std::ios::in | std::ios::binary);
    const std::string flowText((std::istreambuf_iterator<char>(flow)), std::istreambuf_iterator<char>());
    const std::string cellsText((std::istreambuf_iterator<char>(cells)), std::istreambuf_iterator<char>());
    Check(flowText.find("traveler_cluster_id") != std::string::npos && flowText.find("tangent_angle") != std::string::npos,
        "TestFieldCsvExportsTravelerRepresentative_FlowHeader");
    Check(flowText.find("fits_frame_index") != std::string::npos,
        "TestFieldCsvExportsTravelerRepresentative_FlowFitsContextHeader");
    Check(flowText.find("0.5,7,") != std::string::npos,
        "TestFieldCsvExportsTravelerRepresentative_FlowFitsContextValue");
    Check(cellsText.find("traveler_centroid_x") != std::string::npos && cellsText.find("cluster_confidence") != std::string::npos,
        "TestFieldCsvExportsTravelerRepresentative_CellsHeader");
    Check(cellsText.find("fits_frame_index") != std::string::npos,
        "TestFieldCsvExportsTravelerRepresentative_CellsFitsContextHeader");
    Check(cellsText.find("0.5,7,") != std::string::npos,
        "TestFieldCsvExportsTravelerRepresentative_CellsFitsContextValue");

    const std::size_t flowSizeBeforeAppend = flowText.size();
    state.t = 0.75;
    Check(WriteRuntimeWalkFieldSlimeCsv(state, flowPath.string(), cellsPath.string(), &error, &exportContext),
        "TestFieldCsvExportsTravelerRepresentative_AppendWrite");
    std::ifstream appendedFlow(flowPath, std::ios::in | std::ios::binary);
    const std::string appendedFlowText((std::istreambuf_iterator<char>(appendedFlow)), std::istreambuf_iterator<char>());
    Check(appendedFlowText.size() > flowSizeBeforeAppend,
        "TestFieldCsvExportsTravelerRepresentative_AppendsHistory");
    Check(CountOccurrences(appendedFlowText, "traveler_cluster_id") == 1,
        "TestFieldCsvExportsTravelerRepresentative_SingleHeader");
}

int main() {
    TestMarbleSeedingIsDeterministic();
    TestFieldStateRequestsResetWhenSessionSeedChanges();
    TestAdaptiveSamplingRespondsToGradientAndBounds();
    TestFieldStepAdaptsMarblePopulationToMeasuredGradient();
    TestFieldStepProducesStableTravelerAndRejectsNonfinite();
    TestFieldCsvExportsTravelerRepresentative();

    std::printf("test_runtime_walk_field_slime: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}

