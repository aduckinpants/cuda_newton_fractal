#include "diagnostics_capture.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

int g_failed = 0;

void Check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++g_failed;
    }
}

std::string ReadTextFile(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

bool FileStartsWithBmpSignature(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    char signature[2]{};
    file.read(signature, sizeof(signature));
    return file.gcount() == sizeof(signature) && signature[0] == 'B' && signature[1] == 'M';
}

fs::path FreshTempRoot(const char* suffix) {
    fs::path root = fs::temp_directory_path() / "viewer_host_diagnostics_capture_test" / suffix;
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::create_directories(root, ec);
    return root;
}

void PopulateState(ViewState* view, KernelParams* params, RenderSettings* render, RenderStats* stats) {
    *view = ViewState{};
    view->fractal_type = FractalType::julia;
    view->center = {-0.5f, 0.25f};
    view->zoom = 3.0f;
    view->rotation_degrees = 15.0f;
    view->auto_max_iter = true;

    *params = KernelParams{};
    params->max_iter = 321;
    params->epsilon = 0.00025f;
    params->color_pipeline = {ColorSignal::root_index, ColorPalette::joy, ColorGradingPreset::basin_default};
    params->color_shape = ColorPipelineShape::posterize;
    params->color_shape_posterize_steps = 7;
    params->color_shape_posterize_mix = 0.5f;
    params->color_root_basin_pair_count = 2;
    params->color_root_basin_pairs[0] = {ColorSignal::root_index, ColorPalette::root_classic, ColorGradingPreset::basin_default};
    params->color_root_basin_pairs[1] = {ColorSignal::root_index, ColorPalette::joy, ColorGradingPreset::basin_default};

    *render = RenderSettings{};
    render->resolution = {2, 2};
    render->interaction_debounce_ms = 123;
    render->preview_target_fps = 24.0f;
    render->preview_min_scale = 0.25f;
    render->sample_tier = SampleTier::standard;

    *stats = RenderStats{};
    stats->last_render_ms = 4.5f;
    stats->last_iters_avg = 17;
    stats->last_device_id = 3;
    stats->resolved_eval = {NumericBackend::float64, IterationStrategy::direct};
}

void TestBundleWritesFrameAndState() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    PopulateState(&view, &params, &render, &stats);

    const fs::path outputDir = FreshTempRoot("bundle") / "diagnostics_bundle";
    std::vector<uint32_t> rgba{0xff0000ffu, 0xff00ff00u, 0xffff0000u, 0xffffffffu};
    DiagnosticsCaptureResult result{};
    std::string error = "stale";

    const bool ok = CaptureDiagnosticsBundleToDir(outputDir.string(), view, params, render, stats, rgba.data(), rgba.size(), &result, &error);
    Check(ok, "CaptureDiagnosticsBundleToDir succeeds for exact pixel count");
    Check(error.empty(), "successful capture clears the error string");
    Check(fs::equivalent(fs::path(result.output_dir), outputDir.lexically_normal()), "result output_dir points at requested bundle");
    Check(FileStartsWithBmpSignature(result.frame_bmp_path), "capture writes a BMP frame");
    Check(fs::exists(result.state_json_path), "capture writes state.json");

    const std::string json = ReadTextFile(result.state_json_path);
    Check(json.find("\"fractal_type\": \"julia\"") != std::string::npos, "state persists fractal type id");
    Check(json.find("\"width\": 2") != std::string::npos, "state persists render width");
    Check(json.find("\"height\": 2") != std::string::npos, "state persists render height");
    Check(json.find("\"interaction_debounce_ms\": 123") != std::string::npos, "state persists render debounce");
    Check(json.find("\"last_render_ms\": 4.5") != std::string::npos, "state persists render stats");
    Check(json.find("\"sample_tier\": \"standard\"") != std::string::npos, "state persists requested sample tier");
    Check(json.find("\"resolved_backend\": \"float64\"") != std::string::npos, "state persists resolved numeric backend");
    Check(json.find("\"resolved_strategy\": \"direct\"") != std::string::npos, "state persists resolved iteration strategy");
    Check(json.find("\"color_palette\": \"joy\"") != std::string::npos, "state persists widened color palette id");
    Check(json.find("\"coloring_mode\": \"joy_basins\"") != std::string::npos, "state derives legacy coloring mirror from coherent root-basin pair");
}

void TestBundlePersistsCounterfactualPairFractalTypeId() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    PopulateState(&view, &params, &render, &stats);
    view.fractal_type = FractalType::counterfactual_pair;
    params.max_iter = 96;
    params.epsilon = 1.0e-6f;
    params.counterfactual_pair_root_family = CounterfactualPairRootFamily::quartic_unit_roots;
    params.counterfactual_pair_frame = CounterfactualPairFrame::view_relative;
    params.counterfactual_pair_offset_x = 0.125f;
    params.counterfactual_pair_offset_y = -0.0625f;
    params.counterfactual_pair_reconvergence_ratio = 0.25f;

    const fs::path outputDir = FreshTempRoot("counterfactual_pair") / "diagnostics_bundle";
    std::vector<uint32_t> rgba{0xff112233u, 0xff445566u, 0xff778899u, 0xffaabbccu};
    DiagnosticsCaptureResult result{};
    std::string error;

    const bool ok = CaptureDiagnosticsBundleToDir(outputDir.string(), view, params, render, stats, rgba.data(), rgba.size(), &result, &error);
    Check(ok, "CaptureDiagnosticsBundleToDir succeeds for counterfactual pair state export");
    if (!ok) {
        return;
    }

    const std::string json = ReadTextFile(result.state_json_path);
    Check(json.find("\"fractal_type\": \"counterfactual_pair\"") != std::string::npos,
        "state persists counterfactual_pair fractal type id");
    Check(json.find("\"fractal_type\": \"unknown\"") == std::string::npos,
        "state export never falls back to unknown for counterfactual_pair");
    Check(json.find("\"counterfactual_pair_root_family\": \"quartic_unit_roots\"") != std::string::npos,
        "state persists Counterfactual Pair root family id");
    Check(json.find("\"counterfactual_pair_frame\": \"view_relative\"") != std::string::npos,
        "state persists Counterfactual Pair frame id");
    Check(json.find("\"counterfactual_pair_offset_x\": 0.125") != std::string::npos,
        "state persists Counterfactual Pair offset x");
    Check(json.find("\"counterfactual_pair_offset_y\": -0.0625") != std::string::npos,
        "state persists Counterfactual Pair offset y");
    Check(json.find("\"counterfactual_pair_reconvergence_ratio\": 0.25") != std::string::npos,
        "state persists Counterfactual Pair reconvergence ratio");
}

void TestBundlePersistsExplainoCounterfactualPairFractalTypeId() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    PopulateState(&view, &params, &render, &stats);
    view.fractal_type = FractalType::explaino_counterfactual_pair;
    params.max_iter = 96;
    params.epsilon = 1.0e-6f;
    params.poly_kind = PolyKind::custom;
    params.counterfactual_pair_frame = CounterfactualPairFrame::view_relative;
    params.counterfactual_pair_offset_x = 0.1875f;
    params.counterfactual_pair_offset_y = -0.09375f;
    params.counterfactual_pair_reconvergence_ratio = 0.4f;
    params.explaino_seed = 3.5;
    params.explaino_warp_strength = 0.25f;
    params.explaino_damping = 0.75f;
    params.explaino_root_count = 4;
    params.explaino_roots[0] = {0.5f, 0.0f};
    params.explaino_roots[1] = {0.0f, 0.5f};
    params.explaino_roots[2] = {-0.5f, 0.0f};
    params.explaino_roots[3] = {0.0f, -0.5f};

    const fs::path outputDir = FreshTempRoot("explaino_counterfactual_pair") / "diagnostics_bundle";
    std::vector<uint32_t> rgba{0xff224466u, 0xff335577u, 0xff446688u, 0xff557799u};
    DiagnosticsCaptureResult result{};
    std::string error;

    const bool ok = CaptureDiagnosticsBundleToDir(outputDir.string(), view, params, render, stats, rgba.data(), rgba.size(), &result, &error);
    Check(ok, "CaptureDiagnosticsBundleToDir succeeds for explaino_counterfactual_pair state export");
    if (!ok) {
        return;
    }

    const std::string json = ReadTextFile(result.state_json_path);
    Check(json.find("\"fractal_type\": \"explaino_counterfactual_pair\"") != std::string::npos,
        "state persists explaino_counterfactual_pair fractal type id");
    Check(json.find("\"fractal_type\": \"unknown\"") == std::string::npos,
        "state export never falls back to unknown for explaino_counterfactual_pair");
    Check(json.find("\"counterfactual_pair_frame\": \"view_relative\"") != std::string::npos,
        "state persists shared Counterfactual Pair frame for explaino_counterfactual_pair");
    Check(json.find("\"explaino_seed\": 3.5") != std::string::npos &&
        json.find("\"explaino_warp_strength\": 0.25") != std::string::npos,
        "state persists Explaino-owned controls for explaino_counterfactual_pair");
}

void TestRejectsMismatchedPixelCount() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    PopulateState(&view, &params, &render, &stats);

    const fs::path outputDir = FreshTempRoot("mismatch") / "diagnostics_bundle";
    std::vector<uint32_t> rgba{0xff0000ffu, 0xff00ff00u, 0xffff0000u};
    DiagnosticsCaptureResult result{};
    std::string error;

    const bool ok = CaptureDiagnosticsBundleToDir(outputDir.string(), view, params, render, stats, rgba.data(), rgba.size(), &result, &error);
    Check(!ok, "capture rejects a buffer smaller than render width*height");
    Check(error.find("pixel count") != std::string::npos, "pixel-count rejection explains the owner error");
    Check(!fs::exists(outputDir / "state.json"), "failed capture does not write state.json");
}

void TestLastBundleAndSidecarOverloads() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    PopulateState(&view, &params, &render, &stats);

    const fs::path exeDir = FreshTempRoot("last_bundle") / "runtime";
    std::vector<uint32_t> rgba{0xff0000ffu, 0xff00ff00u, 0xffff0000u, 0xffffffffu};

    SidecarOrientationVector orientation{};
    orientation.import_signature = 42;
    orientation.pack_projection_hash = 99;
    orientation.field_embedding_stats = 1.25;

    SidecarAutoDemoControllerPolicy policy{};
    policy.enabled = true;
    policy.allow_runtime_mutation = true;
    policy.paced_loop_interval_seconds = 2.5;

    SidecarAutoDemoMutationHistory history;
    history.push_back({"Exposure", "fractal.params.exposure", "float", 1.75, 0.8});

    DiagnosticsCaptureResult result{};
    std::string error;
    const bool ok = CaptureDiagnosticsLastBundle(
        exeDir.string(),
        view,
        params,
        render,
        stats,
        rgba.data(),
        rgba.size(),
        &orientation,
        &policy,
        &history,
        &result,
        &error);

    Check(ok, "CaptureDiagnosticsLastBundle sidecar overload succeeds");
    const fs::path resultDir(result.output_dir);
    Check(resultDir.filename() == "last" && resultDir.parent_path().filename() == "diagnostics", "last-bundle overload writes under diagnostics/last");

    const std::string json = ReadTextFile(result.state_json_path);
    Check(json.find("\"sidecar_orientation\"") != std::string::npos, "state includes sidecar orientation block");
    Check(json.find("\"import_signature\": \"42\"") != std::string::npos, "state serializes orientation import signature as a string");
    Check(json.find("\"sidecar_auto_demo_policy\"") != std::string::npos, "state includes sidecar policy block");
    Check(json.find("\"allow_runtime_mutation\": true") != std::string::npos, "state serializes sidecar policy flags");
    Check(json.find("\"sidecar_mutation_history\"") != std::string::npos, "state includes sidecar mutation history");
    Check(json.find("\"path\": \"fractal.params.exposure\"") != std::string::npos, "state serializes mutation history path");
}

} // namespace

int main() {
    TestBundleWritesFrameAndState();
    TestBundlePersistsCounterfactualPairFractalTypeId();
    TestBundlePersistsExplainoCounterfactualPairFractalTypeId();
    TestRejectsMismatchedPixelCount();
    TestLastBundleAndSidecarOverloads();

    if (g_failed != 0) {
        std::cerr << "test_diagnostics_capture: " << g_failed << " failed\n";
        return 1;
    }
    std::cout << "test_diagnostics_capture: all passed\n";
    return 0;
}
