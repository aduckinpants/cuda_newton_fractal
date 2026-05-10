#include "../src/runtime_walk_viewer_import.h"
#include "../src/runtime_walk_viewer_session.h"

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
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

static void CheckWithError(bool cond, const char* name, const std::string& error) {
    if (cond) {
        Check(true, name);
        return;
    }
    std::printf("    error: %s\n", error.c_str());
    Check(false, name);
}

static std::filesystem::path TempRoot(const char* name) {
    const std::filesystem::path root = std::filesystem::temp_directory_path() / "cuda_newton_runtime_walk_viewer_session_tests" / name;
    std::error_code ec;
    std::filesystem::remove_all(root, ec);
    std::filesystem::create_directories(root, ec);
    return root;
}

static void WriteText(const std::filesystem::path& path, const std::string& text) {
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    std::ofstream out(path, std::ios::out | std::ios::binary | std::ios::trunc);
    out << text;
}

static void WriteDummyFits(const std::filesystem::path& path) {
    WriteText(path, "SIMPLE  =                    T\nEND\n");
}

static void WriteOrientationInputsJson(const std::filesystem::path& path, const std::filesystem::path& fitsPath) {
    WriteText(path,
        std::string("{\n") +
        "  \"version\": 1,\n" +
        "  \"fits_path\": \"" + fitsPath.lexically_normal().generic_string() + "\",\n" +
        "  \"signals\": {\n" +
        "    \"mean\": 0.75,\n" +
        "    \"stddev\": 0.25,\n" +
        "    \"center_bias\": 0.10,\n" +
        "    \"residual_energy\": 0.55,\n" +
        "    \"edge_balance\": -0.15,\n" +
        "    \"frame_delta\": 0.65,\n" +
        "    \"x_bias\": 0.20,\n" +
        "    \"y_bias\": -0.30,\n" +
        "    \"focus_ratio\": 0.70\n" +
        "  }\n" +
        "}\n");
}

static void TestResetRuntimeWalkViewerPlaybackForNewSessionPreservesOverlayToggles() {
    RuntimeWalkViewerPlaybackState previous{};
    previous.loaded = true;
    previous.playing = false;
    previous.loop = false;
    previous.speed = 1.75;
    previous.current_t = 0.42;
    previous.nearest_tick_index = 7u;
    previous.show_raw_path = false;
    previous.show_spline_path = false;
    previous.show_closed_loop = true;
    previous.show_branch_markers = false;
    previous.show_gradient_overlay = false;

    RuntimeWalkViewerPlaybackState next{};
    ResetRuntimeWalkViewerPlaybackForNewSession(previous, &next);

    const RuntimeWalkViewerPlaybackState defaults{};
    Check(!next.loaded,
        "TestResetRuntimeWalkViewerPlaybackForNewSessionPreservesOverlayToggles_ClearsLoaded");
    Check(next.playing == defaults.playing,
        "TestResetRuntimeWalkViewerPlaybackForNewSessionPreservesOverlayToggles_ResetsPlaying");
    Check(next.loop == previous.loop,
        "TestResetRuntimeWalkViewerPlaybackForNewSessionPreservesOverlayToggles_PreservesLoop");
    Check(std::fabs(next.speed - defaults.speed) < 1.0e-9,
        "TestResetRuntimeWalkViewerPlaybackForNewSessionPreservesOverlayToggles_ResetsSpeed");
    Check(std::fabs(next.current_t - defaults.current_t) < 1.0e-9,
        "TestResetRuntimeWalkViewerPlaybackForNewSessionPreservesOverlayToggles_ResetsCurrentT");
    Check(next.nearest_tick_index == defaults.nearest_tick_index,
        "TestResetRuntimeWalkViewerPlaybackForNewSessionPreservesOverlayToggles_ResetsTickIndex");
    Check(next.show_raw_path == previous.show_raw_path,
        "TestResetRuntimeWalkViewerPlaybackForNewSessionPreservesOverlayToggles_PreservesRawPathToggle");
    Check(next.show_spline_path == previous.show_spline_path,
        "TestResetRuntimeWalkViewerPlaybackForNewSessionPreservesOverlayToggles_PreservesSplineToggle");
    Check(next.show_closed_loop == previous.show_closed_loop,
        "TestResetRuntimeWalkViewerPlaybackForNewSessionPreservesOverlayToggles_PreservesClosedLoopToggle");
    Check(next.show_branch_markers == previous.show_branch_markers,
        "TestResetRuntimeWalkViewerPlaybackForNewSessionPreservesOverlayToggles_PreservesBranchMarkersToggle");
    Check(next.show_gradient_overlay == previous.show_gradient_overlay,
        "TestResetRuntimeWalkViewerPlaybackForNewSessionPreservesOverlayToggles_PreservesGradientToggle");
}

static void TestLoadRuntimeWalkViewerSessionLoadsLiveBindingsFromSynthesizedImport() {
    const std::filesystem::path root = TempRoot("session_live_bindings");
    const std::filesystem::path fitsPath = root / "checkpoint_final.fits";
    const std::filesystem::path orientationInputsPath = root / "orientation_inputs.json";
    WriteDummyFits(fitsPath);
    WriteOrientationInputsJson(orientationInputsPath, fitsPath);

    RuntimeWalkViewerImportRequest request{};
    request.exe_dir = root.string();
    request.authority_mode = RuntimeWalkAuthorityMode::synthesized_fits_base;
    request.comparison_fits_path = fitsPath.string();
    request.orientation_inputs_json_path = orientationInputsPath.string();

    RuntimeWalkViewerImportSessionRecord record;
    std::string error;
    CheckWithError(BuildRuntimeWalkViewerImportSession(request, &record, &error),
        "TestLoadRuntimeWalkViewerSessionLoadsLiveBindingsFromSynthesizedImport_Builds",
        error);

    RuntimeWalkViewerSession session{};
    CheckWithError(LoadRuntimeWalkViewerSession(record.request_json_path, &session, &error),
        "TestLoadRuntimeWalkViewerSessionLoadsLiveBindingsFromSynthesizedImport_Loads",
        error);
    Check(session.loaded,
        "TestLoadRuntimeWalkViewerSessionLoadsLiveBindingsFromSynthesizedImport_Loaded");
    Check(session.authority_mode == RuntimeWalkAuthorityMode::synthesized_fits_base,
        "TestLoadRuntimeWalkViewerSessionLoadsLiveBindingsFromSynthesizedImport_AuthorityMode");
    Check(session.has_operator_baseline,
        "TestLoadRuntimeWalkViewerSessionLoadsLiveBindingsFromSynthesizedImport_HasOperatorBaseline");
    Check(!session.has_last_composed_state,
        "TestLoadRuntimeWalkViewerSessionLoadsLiveBindingsFromSynthesizedImport_NoLastComposedStateYet");
    Check(!session.resolved_state_json_path.empty() && std::filesystem::exists(session.resolved_state_json_path),
        "TestLoadRuntimeWalkViewerSessionLoadsLiveBindingsFromSynthesizedImport_ResolvedStateExists");
    Check(session.asset.authority.mapping_profile_json_path == record.mapping_profile_json_path,
        "TestLoadRuntimeWalkViewerSessionLoadsLiveBindingsFromSynthesizedImport_MappingProfileRecorded");
    Check(session.asset.authority.orientation_inputs_json_path == record.orientation_inputs_json_path,
        "TestLoadRuntimeWalkViewerSessionLoadsLiveBindingsFromSynthesizedImport_OrientationInputsRecorded");
    Check(session.live_bindings_loaded,
        "TestLoadRuntimeWalkViewerSessionLoadsLiveBindingsFromSynthesizedImport_LiveBindingsLoaded");
    Check(!session.live_binding_rows.empty(),
        "TestLoadRuntimeWalkViewerSessionLoadsLiveBindingsFromSynthesizedImport_LiveBindingRows");
    Check(!session.live_fits_signal_catalog.empty(),
        "TestLoadRuntimeWalkViewerSessionLoadsLiveBindingsFromSynthesizedImport_FitsSignalCatalog");
    Check(!session.live_runtime_target_catalog.empty(),
        "TestLoadRuntimeWalkViewerSessionLoadsLiveBindingsFromSynthesizedImport_RuntimeTargetCatalog");

    bool hasMeanSignal = false;
    bool hasFieldTravelerScore = false;
    for (const std::string& signal : session.live_fits_signal_catalog) {
        hasMeanSignal = hasMeanSignal || signal == "mean";
        hasFieldTravelerScore = hasFieldTravelerScore || signal == "field.traveler.score";
    }
    Check(hasMeanSignal,
        "TestLoadRuntimeWalkViewerSessionLoadsLiveBindingsFromSynthesizedImport_HasMeanSignal");
    Check(hasFieldTravelerScore,
        "TestLoadRuntimeWalkViewerSessionLoadsLiveBindingsFromSynthesizedImport_HasFieldTravelerScore");

    bool hasWarpTarget = false;
    for (const std::string& target : session.live_runtime_target_catalog) {
        hasWarpTarget = hasWarpTarget || target.find("warp") != std::string::npos;
    }
    Check(!hasWarpTarget,
        "TestLoadRuntimeWalkViewerSessionLoadsLiveBindingsFromSynthesizedImport_NoWarpTargets");
}

int main() {
    TestResetRuntimeWalkViewerPlaybackForNewSessionPreservesOverlayToggles();
    TestLoadRuntimeWalkViewerSessionLoadsLiveBindingsFromSynthesizedImport();

    std::printf("test_runtime_walk_viewer_session: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}