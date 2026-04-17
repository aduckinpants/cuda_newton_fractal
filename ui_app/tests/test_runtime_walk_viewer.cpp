#include "../src/runtime_walk_viewer.h"

#include <cmath>
#include <cstdio>
#include <string>

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

static const char* kValidBundleJson = R"JSON({
  "version": 1,
  "field_name": "mr_zipper_branch",
  "samples": [
    {
      "id": "start",
      "t": 0.0,
      "channels": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    },
    {
      "id": "mid",
      "t": 0.5,
      "channels": [0.4, 0.2, 0.8, 0.3, 0.6, 0.75, 0.9, 0.1, 0.2, 0.7, 0.3, 0.4, 1.0]
    },
    {
      "id": "end",
      "t": 1.0,
      "channels": [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
    }
  ],
  "branch_markers": [
    {
      "id": "fork_a",
      "label": "fork-a",
      "parent_id": "main",
      "t": 0.5,
      "sticky_radius": 0.1
    }
  ]
})JSON";

static RuntimeWalkViewerAsset BuildAsset() {
    RuntimeWalkBundle bundle;
    std::string error;
    Check(ParseRuntimeWalkBundleJson(kValidBundleJson, &bundle, &error), "BuildAsset_ParseBundle");

    RuntimeWalkRequest request{};
    request.base_state_json_path = "state.json";
    request.bundle_json_path = "bundle.json";
    request.output_dir = "artifacts/out";
    request.comparison_fits_path = "comparison.fits";
    request.rtk_manifest_json_path = "rtk_manifest.json";
    request.rtk_harvest_summary_json_path = "rtk_harvest.json";
    request.t_values = {0.0, 0.25, 0.5, 0.75, 1.0};

    ViewState baseView{};
    KernelParams baseParams{};
    RenderSettings render{};
    baseView.fractal_type = FractalType::explaino_fp;
    baseView.center_hp_x = 0.0;
    baseView.center_hp_y = 0.0;
    baseView.log2_zoom = 0.0;
    baseView.explaino_phase = 0.0f;
    baseParams.explaino_seed = 0.0;
    baseParams.explaino_seed_b = 0.0;
    baseParams.explaino_mix = 0.0f;
    baseParams.explaino_warp_strength = 0.0f;
    render.resolution = {800, 600};

    RuntimeWalkViewerAsset asset;
    Check(BuildRuntimeWalkViewerAsset(request, bundle, baseView, baseParams, render, &asset, &error),
        "BuildAsset_BuildViewerAsset");
    return asset;
}

static void TestBuildRuntimeWalkViewerAssetCarriesCompanionsAndTicks() {
    RuntimeWalkViewerAsset asset = BuildAsset();
    Check(asset.tick_snapshots.size() == 5u, "TestBuildRuntimeWalkViewerAssetCarriesCompanionsAndTicks_TickCount");
    Check(asset.companion.comparison_fits_path == "comparison.fits",
        "TestBuildRuntimeWalkViewerAssetCarriesCompanionsAndTicks_Fits");
    Check(asset.companion.rtk_manifest_json_path == "rtk_manifest.json",
        "TestBuildRuntimeWalkViewerAssetCarriesCompanionsAndTicks_RtkManifest");
    Check(asset.companion.rtk_harvest_summary_json_path == "rtk_harvest.json",
        "TestBuildRuntimeWalkViewerAssetCarriesCompanionsAndTicks_RtkHarvest");
}

static void TestRuntimeWalkViewerPlaybackMatchesHeadlessInterpolation() {
    RuntimeWalkViewerAsset asset = BuildAsset();
    RuntimeWalkViewerPlaybackState playback{};
    bool changed = false;
    Check(SeekRuntimeWalkViewerPlayback(asset, 0.25, &playback, &changed),
        "TestRuntimeWalkViewerPlaybackMatchesHeadlessInterpolation_Seek");
    Check(changed, "TestRuntimeWalkViewerPlaybackMatchesHeadlessInterpolation_Changed");

    RuntimeWalkSnapshot currentSnapshot;
    std::string error;
    Check(EvaluateRuntimeWalkViewerCurrentSnapshot(asset, playback, &currentSnapshot, &error),
        "TestRuntimeWalkViewerPlaybackMatchesHeadlessInterpolation_EvalViewer");

    RuntimeWalkSnapshot headlessSnapshot;
    Check(EvaluateRuntimeWalkSnapshot(asset.bundle, 0.25, asset.base_view, asset.base_params, asset.base_render, &headlessSnapshot, &error),
        "TestRuntimeWalkViewerPlaybackMatchesHeadlessInterpolation_EvalHeadless");
    Check(NearlyEqual(currentSnapshot.center_hp_x, headlessSnapshot.center_hp_x, 1.0e-12),
        "TestRuntimeWalkViewerPlaybackMatchesHeadlessInterpolation_CenterX");
    Check(NearlyEqual(currentSnapshot.phase, headlessSnapshot.phase, 1.0e-12),
        "TestRuntimeWalkViewerPlaybackMatchesHeadlessInterpolation_Phase");
    Check(currentSnapshot.branch.nearest_marker_id == headlessSnapshot.branch.nearest_marker_id,
        "TestRuntimeWalkViewerPlaybackMatchesHeadlessInterpolation_Branch");
}

static void TestRuntimeWalkViewerPlaybackStepAndLoop() {
    RuntimeWalkViewerAsset asset = BuildAsset();
    RuntimeWalkViewerPlaybackState playback{};
    bool changed = false;
    Check(SeekRuntimeWalkViewerPlayback(asset, 1.0, &playback, &changed),
        "TestRuntimeWalkViewerPlaybackStepAndLoop_Seek");
    playback.loop = true;
    Check(StepRuntimeWalkViewerPlayback(asset, 1, &playback, &changed),
        "TestRuntimeWalkViewerPlaybackStepAndLoop_Step");
    Check(playback.nearest_tick_index == 0u, "TestRuntimeWalkViewerPlaybackStepAndLoop_WrappedIndex");
    Check(NearlyEqual(playback.current_t, 0.0), "TestRuntimeWalkViewerPlaybackStepAndLoop_WrappedT");

    playback.playing = true;
    playback.speed = 1.25;
    Check(AdvanceRuntimeWalkViewerPlayback(asset, 1.0, &playback, &changed),
        "TestRuntimeWalkViewerPlaybackStepAndLoop_Advance");
    Check(changed, "TestRuntimeWalkViewerPlaybackStepAndLoop_AdvanceChanged");
    Check(playback.current_t >= 0.0 && playback.current_t <= 1.0,
        "TestRuntimeWalkViewerPlaybackStepAndLoop_AdvanceDomain");
}

static void TestRuntimeWalkOverlayPathBuildsDeterministicClosedLoop() {
    RuntimeWalkViewerAsset asset = BuildAsset();
    RuntimeWalkViewerPlaybackState playback{};
    bool changed = false;
    Check(SeekRuntimeWalkViewerPlayback(asset, 0.5, &playback, &changed),
        "TestRuntimeWalkOverlayPathBuildsDeterministicClosedLoop_Seek");

    RuntimeWalkOverlayPath path;
    BuildRuntimeWalkOverlayPath(asset, playback, &path);
    Check(path.raw_points.size() == asset.tick_snapshots.size(),
        "TestRuntimeWalkOverlayPathBuildsDeterministicClosedLoop_RawCount");
    Check(path.spline_points.size() > path.raw_points.size(),
        "TestRuntimeWalkOverlayPathBuildsDeterministicClosedLoop_SplineExpanded");
    Check(path.closed_loop_points.size() > path.raw_points.size(),
        "TestRuntimeWalkOverlayPathBuildsDeterministicClosedLoop_ClosedExpanded");
    Check(NearlyEqual(path.closed_loop_points.front().x, path.closed_loop_points.back().x, 1.0e-9) &&
        NearlyEqual(path.closed_loop_points.front().y, path.closed_loop_points.back().y, 1.0e-9),
        "TestRuntimeWalkOverlayPathBuildsDeterministicClosedLoop_ClosedReconnects");
    Check(path.branch_marker_points.size() == 1u,
        "TestRuntimeWalkOverlayPathBuildsDeterministicClosedLoop_BranchPoints");
}

static void TestRuntimeWalkGradientOverlayFiniteAndThresholded() {
    RuntimeWalkViewerAsset asset = BuildAsset();
    RuntimeWalkViewerPlaybackState playback{};
    bool changed = false;
    Check(SeekRuntimeWalkViewerPlayback(asset, 0.5, &playback, &changed),
        "TestRuntimeWalkGradientOverlayFiniteAndThresholded_Seek");

    RuntimeWalkOverlayProviderConfig config{};
    config.threshold = 0.02;
    RuntimeWalkOverlayProviderInputs inputs{};
    inputs.decode_stability = 0.92;
    inputs.divergence = 0.15;
    inputs.branch_proximity = 0.75;

    RuntimeWalkGradientOverlay overlay;
    std::string error;
    Check(BuildRuntimeWalkGradientOverlay(asset, playback, config, inputs, &overlay, &error),
        "TestRuntimeWalkGradientOverlayFiniteAndThresholded_Build");
    Check(!overlay.strokes.empty(),
        "TestRuntimeWalkGradientOverlayFiniteAndThresholded_HasStrokes");
    Check(overlay.strokes.size() >= 6u,
        "TestRuntimeWalkGradientOverlayFiniteAndThresholded_FlowMapDensity");
    bool finite = true;
    bool variedOrigins = false;
    Double2 firstOrigin{};
    bool haveFirstOrigin = false;
    for (const RuntimeWalkGradientOverlayGuideStroke& stroke : overlay.strokes) {
        finite = finite && stroke.points.size() >= 2u;
        if (!stroke.points.empty()) {
            if (!haveFirstOrigin) {
                firstOrigin = stroke.points.front().point;
                haveFirstOrigin = true;
            } else if (!NearlyEqual(firstOrigin.x, stroke.points.front().point.x, 1.0e-6) ||
                !NearlyEqual(firstOrigin.y, stroke.points.front().point.y, 1.0e-6)) {
                variedOrigins = true;
            }
        }
        for (const RuntimeWalkGradientOverlayGuidePoint& point : stroke.points) {
            finite = finite &&
                std::isfinite(point.point.x) &&
                std::isfinite(point.point.y) &&
                point.point.x >= 0.0 && point.point.x <= 1.0 &&
                point.point.y >= 0.0 && point.point.y <= 1.0;
        }
    }
    Check(finite, "TestRuntimeWalkGradientOverlayFiniteAndThresholded_Finite");
    Check(variedOrigins, "TestRuntimeWalkGradientOverlayFiniteAndThresholded_VariedOrigins");

    RuntimeWalkOverlayProviderConfig highThreshold = config;
    highThreshold.threshold = 10.0;
    overlay = {};
    Check(BuildRuntimeWalkGradientOverlay(asset, playback, highThreshold, inputs, &overlay, &error),
        "TestRuntimeWalkGradientOverlayFiniteAndThresholded_BuildHighThreshold");
    Check(overlay.strokes.empty(),
        "TestRuntimeWalkGradientOverlayFiniteAndThresholded_ThresholdStops");
}

int main() {
    TestBuildRuntimeWalkViewerAssetCarriesCompanionsAndTicks();
    TestRuntimeWalkViewerPlaybackMatchesHeadlessInterpolation();
    TestRuntimeWalkViewerPlaybackStepAndLoop();
    TestRuntimeWalkOverlayPathBuildsDeterministicClosedLoop();
    TestRuntimeWalkGradientOverlayFiniteAndThresholded();

    std::printf("test_runtime_walk_viewer: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}
