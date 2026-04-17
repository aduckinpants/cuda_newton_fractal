#include "../src/runtime_walk_viewer_import.h"

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
    const std::filesystem::path root = std::filesystem::temp_directory_path() / "cuda_newton_runtime_walk_viewer_import_tests" / name;
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

static void WriteStateJson(const std::filesystem::path& path) {
    WriteText(path, R"JSON({
  "state_version": 3,
  "fractal_type": "explaino_fp",
  "view": {
    "center_x": 0.0,
    "center_y": 0.0,
    "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0,
    "center_hp_y": 0.0,
    "log2_zoom": 0.0,
    "explaino_phase": 0.0,
    "explaino_seed_drift": 0.0,
    "explaino_seed_tween": true,
    "auto_max_iter": false,
    "auto_increment_seed": false,
    "explaino_seed_rate": 0.001,
    "explaino_phase_strength": 0.0
  },
  "params": {
    "max_iter": 500,
    "epsilon": 1.0e-6,
    "poly_kind": 2,
    "coloring_mode": "smooth_escape",
    "explaino_seed": 0.0,
    "explaino_seed_b": 0.0,
    "explaino_mix": 0.0,
    "explaino_warp_strength": 0.0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 320,
    "height": 240,
    "block_size": 256,
    "device_id": 0
  }
})JSON");
}

static void WriteBundleJson(const std::filesystem::path& path) {
    WriteText(path, R"JSON({
  "version": 1,
  "field_name": "mr_zipper_branch",
  "samples": [
    { "id": "start", "t": 0.0, "channels": [0,0,0,0,0,0,0,0,0,0,0,0,0] },
    { "id": "mid", "t": 0.5, "channels": [0.4,0.2,0.8,0.3,0.6,0.75,0.9,0.1,0.2,0.7,0.3,0.4,1.0] },
    { "id": "end", "t": 1.0, "channels": [1,1,1,1,1,1,1,1,1,1,1,1,1] }
  ],
  "branch_markers": [
    { "id": "fork_a", "label": "fork-a", "parent_id": "main", "t": 0.5, "sticky_radius": 0.1 }
  ]
})JSON");
}

static void WriteRequestJson(const std::filesystem::path& path, const std::filesystem::path& bundlePath, const std::filesystem::path& fitsPath) {
    WriteText(path,
        std::string("{\n") +
        "  \"version\": 1,\n" +
        "  \"base_state_json\": \"ignored.json\",\n" +
        "  \"bundle_json\": \"" + bundlePath.lexically_normal().string() + "\",\n" +
        "  \"out_dir\": \"runtime_walk_out\",\n" +
        "  \"ticks\": 5,\n" +
        "  \"comparison_fits\": \"" + fitsPath.lexically_normal().string() + "\"\n" +
        "}\n");
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

static void TestImportSessionRejectsMissingBaseState() {
    const std::filesystem::path root = TempRoot("missing_base_state");
    const std::filesystem::path bundlePath = root / "bundle.json";
    const std::filesystem::path fitsPath = root / "checkpoint_final.fits";
    WriteBundleJson(bundlePath);
    WriteDummyFits(fitsPath);

    RuntimeWalkViewerImportRequest request{};
    request.exe_dir = root.string();
    request.base_fractal_type = FractalType::explaino_fp;
    request.comparison_fits_path = fitsPath.string();
    request.bundle_json_path = bundlePath.string();

    RuntimeWalkViewerImportSessionRecord record;
    std::string error;
    Check(!BuildRuntimeWalkViewerImportSession(request, &record, &error),
        "TestImportSessionRejectsMissingBaseState_Fails");
    Check(error.find("loaded capture state") != std::string::npos,
        "TestImportSessionRejectsMissingBaseState_Error");
}

static void TestImportSessionRejectsWrongFamily() {
    const std::filesystem::path root = TempRoot("wrong_family");
    const std::filesystem::path statePath = root / "state.json";
    const std::filesystem::path bundlePath = root / "bundle.json";
    const std::filesystem::path fitsPath = root / "checkpoint_final.fits";
    WriteStateJson(statePath);
    WriteBundleJson(bundlePath);
    WriteDummyFits(fitsPath);

    RuntimeWalkViewerImportRequest request{};
    request.exe_dir = root.string();
    request.base_state_json_path = statePath.string();
    request.base_fractal_type = FractalType::newton;
    request.comparison_fits_path = fitsPath.string();
    request.bundle_json_path = bundlePath.string();

    RuntimeWalkViewerImportSessionRecord record;
    std::string error;
    Check(!BuildRuntimeWalkViewerImportSession(request, &record, &error),
        "TestImportSessionRejectsWrongFamily_Fails");
    Check(error.find("Explaino-family") != std::string::npos,
        "TestImportSessionRejectsWrongFamily_Error");
}

static void TestValidateImportBaseStateUsesAuthoritativeFamily() {
    std::string error;
    Check(ValidateRuntimeWalkViewerImportBaseState(RuntimeWalkAuthorityMode::loaded_base_state, "state.json", FractalType::explaino_fp, &error),
        "TestValidateImportBaseStateUsesAuthoritativeFamily_Explaino");
    Check(!ValidateRuntimeWalkViewerImportBaseState(RuntimeWalkAuthorityMode::loaded_base_state, "state.json", FractalType::mandelbrot, &error),
        "TestValidateImportBaseStateUsesAuthoritativeFamily_NonExplainoRejected");
    Check(error.find("Explaino-family") != std::string::npos,
        "TestValidateImportBaseStateUsesAuthoritativeFamily_Error");
    Check(ValidateRuntimeWalkViewerImportBaseState(RuntimeWalkAuthorityMode::synthesized_fits_base, "", FractalType::newton, &error),
        "TestValidateImportBaseStateUsesAuthoritativeFamily_SynthModeAllowsMissingState");
}

static void TestImportSessionLoadedStateGeneratesTransportWithoutBundleOrRequest() {
    const std::filesystem::path root = TempRoot("missing_bundle");
    const std::filesystem::path statePath = root / "state.json";
    const std::filesystem::path fitsPath = root / "checkpoint_final.fits";
    const std::filesystem::path orientationInputsPath = root / "orientation_inputs.json";
    WriteStateJson(statePath);
    WriteDummyFits(fitsPath);
    WriteOrientationInputsJson(orientationInputsPath, fitsPath);

    RuntimeWalkViewerImportRequest request{};
    request.exe_dir = root.string();
    request.base_state_json_path = statePath.string();
    request.base_fractal_type = FractalType::explaino_fp;
    request.comparison_fits_path = fitsPath.string();
    request.orientation_inputs_json_path = orientationInputsPath.string();

    RuntimeWalkViewerImportSessionRecord record;
    std::string error;
    CheckWithError(BuildRuntimeWalkViewerImportSession(request, &record, &error),
        "TestImportSessionLoadedStateGeneratesTransportWithoutBundleOrRequest_Builds",
        error);
    Check(record.transport_generated,
        "TestImportSessionLoadedStateGeneratesTransportWithoutBundleOrRequest_TransportGenerated");
    Check(record.discovery_source == "generated_transport",
        "TestImportSessionLoadedStateGeneratesTransportWithoutBundleOrRequest_DiscoverySource");
    Check(!record.bundle_json_path.empty() && std::filesystem::exists(record.bundle_json_path),
        "TestImportSessionLoadedStateGeneratesTransportWithoutBundleOrRequest_BundleWritten");
}

static void TestImportSessionPersistsDeterministicRecentLatest() {
    const std::filesystem::path root = TempRoot("recent_latest");
    const std::filesystem::path statePath = root / "state.json";
    const std::filesystem::path bundlePath = root / "bundle.json";
    const std::filesystem::path fitsPath = root / "checkpoint_final.fits";
    WriteStateJson(statePath);
    WriteBundleJson(bundlePath);
    WriteDummyFits(fitsPath);

    RuntimeWalkViewerImportRequest request{};
    request.exe_dir = root.string();
    request.base_state_json_path = statePath.string();
    request.base_fractal_type = FractalType::explaino_fp;
    request.comparison_fits_path = fitsPath.string();
    request.bundle_json_path = bundlePath.string();

    RuntimeWalkViewerImportSessionRecord first;
    std::string error;
    Check(BuildRuntimeWalkViewerImportSession(request, &first, &error),
        "TestImportSessionPersistsDeterministicRecentLatest_FirstBuild");
    RuntimeWalkViewerImportSessionRecord second;
    Check(BuildRuntimeWalkViewerImportSession(request, &second, &error),
        "TestImportSessionPersistsDeterministicRecentLatest_SecondBuild");
    Check(first.session_id == second.session_id,
        "TestImportSessionPersistsDeterministicRecentLatest_DeterministicId");
    Check(std::filesystem::exists(first.request_json_path),
        "TestImportSessionPersistsDeterministicRecentLatest_RequestWritten");

    RuntimeWalkViewerImportSessionRecord latest;
    Check(LoadLatestRuntimeWalkViewerImportSession(root.string(), &latest, &error),
        "TestImportSessionPersistsDeterministicRecentLatest_LoadLatest");
    Check(latest.request_json_path == first.request_json_path,
        "TestImportSessionPersistsDeterministicRecentLatest_LatestMatches");

    std::vector<RuntimeWalkViewerImportSessionRecord> recent;
    Check(LoadRecentRuntimeWalkViewerImportSessions(root.string(), &recent, &error),
        "TestImportSessionPersistsDeterministicRecentLatest_LoadRecent");
    Check(recent.size() == 1u,
        "TestImportSessionPersistsDeterministicRecentLatest_RecentCount");
    Check(recent.front().request_exists,
        "TestImportSessionPersistsDeterministicRecentLatest_RecentAvailable");
}

static void TestImportSessionCanRediscoverFromRecentMatch() {
    const std::filesystem::path root = TempRoot("recent_match");
    const std::filesystem::path statePath = root / "state.json";
    const std::filesystem::path bundlePath = root / "bundle.json";
    const std::filesystem::path fitsPath = root / "checkpoint_final.fits";
    WriteStateJson(statePath);
    WriteBundleJson(bundlePath);
    WriteDummyFits(fitsPath);

    RuntimeWalkViewerImportRequest firstRequest{};
    firstRequest.exe_dir = root.string();
    firstRequest.base_state_json_path = statePath.string();
    firstRequest.base_fractal_type = FractalType::explaino_fp;
    firstRequest.comparison_fits_path = fitsPath.string();
    firstRequest.bundle_json_path = bundlePath.string();

    RuntimeWalkViewerImportSessionRecord first;
    std::string error;
    Check(BuildRuntimeWalkViewerImportSession(firstRequest, &first, &error),
        "TestImportSessionCanRediscoverFromRecentMatch_SeedBuild");

    RuntimeWalkViewerImportRequest secondRequest{};
    secondRequest.exe_dir = root.string();
    secondRequest.base_state_json_path = statePath.string();
    secondRequest.base_fractal_type = FractalType::explaino_fp;
    secondRequest.comparison_fits_path = fitsPath.string();

    RuntimeWalkViewerImportSessionRecord second;
    Check(BuildRuntimeWalkViewerImportSession(secondRequest, &second, &error),
        "TestImportSessionCanRediscoverFromRecentMatch_Build");
    Check(second.discovery_source.find("recent_match") != std::string::npos,
        "TestImportSessionCanRediscoverFromRecentMatch_DiscoverySource");
}

static void TestLatestRejectsStaleSession() {
    const std::filesystem::path root = TempRoot("stale_latest");
    const std::filesystem::path statePath = root / "state.json";
    const std::filesystem::path bundlePath = root / "bundle.json";
    const std::filesystem::path fitsPath = root / "checkpoint_final.fits";
    WriteStateJson(statePath);
    WriteBundleJson(bundlePath);
    WriteDummyFits(fitsPath);

    RuntimeWalkViewerImportRequest request{};
    request.exe_dir = root.string();
    request.base_state_json_path = statePath.string();
    request.base_fractal_type = FractalType::explaino_fp;
    request.comparison_fits_path = fitsPath.string();
    request.bundle_json_path = bundlePath.string();

    RuntimeWalkViewerImportSessionRecord record;
    std::string error;
    Check(BuildRuntimeWalkViewerImportSession(request, &record, &error),
        "TestLatestRejectsStaleSession_SeedBuild");
    std::filesystem::remove(record.request_json_path);

    RuntimeWalkViewerImportSessionRecord latest;
    Check(!LoadLatestRuntimeWalkViewerImportSession(root.string(), &latest, &error),
        "TestLatestRejectsStaleSession_Fails");
    Check(error.find("stale") != std::string::npos,
        "TestLatestRejectsStaleSession_Error");
}

static void TestLatestRejectsMissingGeneratedBundle() {
    const std::filesystem::path root = TempRoot("stale_bundle_latest");
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
    Check(BuildRuntimeWalkViewerImportSession(request, &record, &error),
        "TestLatestRejectsMissingGeneratedBundle_SeedBuild");
    std::filesystem::remove(record.bundle_json_path);

    RuntimeWalkViewerImportSessionRecord latest;
    Check(!LoadLatestRuntimeWalkViewerImportSession(root.string(), &latest, &error),
        "TestLatestRejectsMissingGeneratedBundle_Fails");
    Check(error.find("stale") != std::string::npos,
        "TestLatestRejectsMissingGeneratedBundle_Error");
}

static void TestImportSessionSynthesizesBaseStateWithoutLoadedState() {
    const std::filesystem::path root = TempRoot("synthesized_base");
    const std::filesystem::path bundlePath = root / "bundle.json";
    const std::filesystem::path fitsPath = root / "checkpoint_final.fits";
    const std::filesystem::path orientationInputsPath = root / "orientation_inputs.json";
    WriteBundleJson(bundlePath);
    WriteDummyFits(fitsPath);
    WriteOrientationInputsJson(orientationInputsPath, fitsPath);

    RuntimeWalkViewerImportRequest request{};
    request.exe_dir = root.string();
    request.authority_mode = RuntimeWalkAuthorityMode::synthesized_fits_base;
    request.base_fractal_type = FractalType::newton;
    request.comparison_fits_path = fitsPath.string();
    request.bundle_json_path = bundlePath.string();
    request.orientation_inputs_json_path = orientationInputsPath.string();

    RuntimeWalkViewerImportSessionRecord record;
    std::string error;
    CheckWithError(BuildRuntimeWalkViewerImportSession(request, &record, &error),
        "TestImportSessionSynthesizesBaseStateWithoutLoadedState_Builds",
        error);
    Check(record.authority_mode == RuntimeWalkAuthorityMode::synthesized_fits_base,
        "TestImportSessionSynthesizesBaseStateWithoutLoadedState_AuthorityMode");
    Check(!record.synthesized_base_state_json_path.empty() && std::filesystem::exists(record.synthesized_base_state_json_path),
        "TestImportSessionSynthesizesBaseStateWithoutLoadedState_StateWritten");
    Check(!record.orientation_inputs_json_path.empty(),
        "TestImportSessionSynthesizesBaseStateWithoutLoadedState_OrientationInputsRecorded");
    Check(std::filesystem::exists(record.request_json_path),
        "TestImportSessionSynthesizesBaseStateWithoutLoadedState_RequestWritten");
}

static void TestImportSessionSynthesizedModeGeneratesTransportWithoutBundleOrRequest() {
    const std::filesystem::path root = TempRoot("synth_missing_bundle");
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
        "TestImportSessionSynthesizedModeGeneratesTransportWithoutBundleOrRequest_Builds",
        error);
    Check(record.transport_generated,
        "TestImportSessionSynthesizedModeGeneratesTransportWithoutBundleOrRequest_TransportGenerated");
    Check(record.discovery_source == "generated_transport",
        "TestImportSessionSynthesizedModeGeneratesTransportWithoutBundleOrRequest_DiscoverySource");
    Check(!record.bundle_json_path.empty() && std::filesystem::exists(record.bundle_json_path),
        "TestImportSessionSynthesizedModeGeneratesTransportWithoutBundleOrRequest_BundleWritten");
    Check(std::filesystem::exists(record.request_json_path),
        "TestImportSessionSynthesizedModeGeneratesTransportWithoutBundleOrRequest_RequestWritten");
}

static void TestImportSessionGeneratesTransportInsteadOfReusingStaleRecent() {
    const std::filesystem::path root = TempRoot("stale_recent_generation");
    const std::filesystem::path fitsPath = root / "checkpoint_final.fits";
    const std::filesystem::path orientationInputsPath = root / "orientation_inputs.json";
    const std::filesystem::path bundlePath = root / "bundle.json";
    WriteDummyFits(fitsPath);
    WriteOrientationInputsJson(orientationInputsPath, fitsPath);
    WriteBundleJson(bundlePath);

    RuntimeWalkViewerImportRequest seeded{};
    seeded.exe_dir = root.string();
    seeded.authority_mode = RuntimeWalkAuthorityMode::synthesized_fits_base;
    seeded.comparison_fits_path = fitsPath.string();
    seeded.bundle_json_path = bundlePath.string();
    seeded.orientation_inputs_json_path = orientationInputsPath.string();

    RuntimeWalkViewerImportSessionRecord first;
    std::string error;
    Check(BuildRuntimeWalkViewerImportSession(seeded, &first, &error),
        "TestImportSessionGeneratesTransportInsteadOfReusingStaleRecent_SeedBuild");
    std::filesystem::remove(first.request_json_path);
    std::filesystem::remove(first.bundle_json_path);

    RuntimeWalkViewerImportRequest regenerated{};
    regenerated.exe_dir = root.string();
    regenerated.authority_mode = RuntimeWalkAuthorityMode::synthesized_fits_base;
    regenerated.comparison_fits_path = fitsPath.string();
    regenerated.orientation_inputs_json_path = orientationInputsPath.string();

    RuntimeWalkViewerImportSessionRecord second;
    CheckWithError(BuildRuntimeWalkViewerImportSession(regenerated, &second, &error),
        "TestImportSessionGeneratesTransportInsteadOfReusingStaleRecent_Regenerates",
        error);
    Check(second.transport_generated,
        "TestImportSessionGeneratesTransportInsteadOfReusingStaleRecent_TransportGenerated");
    Check(second.discovery_source == "generated_transport",
        "TestImportSessionGeneratesTransportInsteadOfReusingStaleRecent_DiscoverySource");
}

int main() {
    TestImportSessionRejectsMissingBaseState();
    TestImportSessionRejectsWrongFamily();
    TestValidateImportBaseStateUsesAuthoritativeFamily();
    TestImportSessionLoadedStateGeneratesTransportWithoutBundleOrRequest();
    TestImportSessionSynthesizesBaseStateWithoutLoadedState();
    TestImportSessionSynthesizedModeGeneratesTransportWithoutBundleOrRequest();
    TestImportSessionGeneratesTransportInsteadOfReusingStaleRecent();
    TestImportSessionPersistsDeterministicRecentLatest();
    TestImportSessionCanRediscoverFromRecentMatch();
    TestLatestRejectsStaleSession();
    TestLatestRejectsMissingGeneratedBundle();

    std::printf("test_runtime_walk_viewer_import: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}
