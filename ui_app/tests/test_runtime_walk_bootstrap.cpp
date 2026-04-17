#include "../src/runtime_walk_bootstrap.h"

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

static bool NearlyEqual(double lhs, double rhs, double eps = 1.0e-6) {
    return std::fabs(lhs - rhs) <= eps;
}

static std::filesystem::path TempRoot(const char* name) {
    const std::filesystem::path root = std::filesystem::temp_directory_path() / "cuda_newton_runtime_walk_bootstrap_tests" / name;
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

static std::filesystem::path ResolveMappingProfilePath() {
    const std::filesystem::path cwd = std::filesystem::current_path();
    const std::filesystem::path direct = cwd / "ui" / "runtime_walk_fits_mapping_profiles_v1.json";
    if (std::filesystem::exists(direct)) return direct;
    const std::filesystem::path parent = cwd.parent_path() / "ui" / "runtime_walk_fits_mapping_profiles_v1.json";
    if (std::filesystem::exists(parent)) return parent;
    return direct;
}

static void TestMappingCatalogRejectsUnsupportedTargetPath() {
    const std::string json = R"JSON({
  "version": 1,
  "profiles": [
    {
      "id": "bad",
      "target_selector": "explaino",
      "base_fractal_type": "explaino_fp",
      "bindings": [
        {
          "target_selector": "explaino",
          "source_signal": "mean",
          "target_path": "view.not_real",
          "input_min": 0.0,
          "input_max": 1.0,
          "scale": 1.0,
          "offset": 0.0
        }
      ]
    }
  ]
})JSON";

    RuntimeWalkFitsMappingCatalog catalog;
    std::string error;
    Check(!ParseRuntimeWalkFitsMappingCatalogJson(json, &catalog, &error),
        "TestMappingCatalogRejectsUnsupportedTargetPath_Fails");
    Check(error.find("Unsupported runtime-walk FITS mapping target path") != std::string::npos,
        "TestMappingCatalogRejectsUnsupportedTargetPath_Error");
}

static void TestOrientationInputsRequireFitsPath() {
    const std::string json = R"JSON({
  "version": 1,
  "signals": {
    "mean": 0.5
  }
})JSON";

    RuntimeWalkFitsOrientationInputs inputs;
    std::string error;
    Check(!ParseRuntimeWalkFitsOrientationInputsJson(json, &inputs, &error),
        "TestOrientationInputsRequireFitsPath_Fails");
    Check(error.find("fits_path") != std::string::npos,
        "TestOrientationInputsRequireFitsPath_Error");
}

static void TestSynthesizedBaseStateUsesMappings() {
    RuntimeWalkFitsMappingCatalog catalog;
    std::string error;
    const std::filesystem::path profilePath = ResolveMappingProfilePath();
    Check(LoadRuntimeWalkFitsMappingCatalogFile(
            profilePath.string(),
            &catalog,
            &error),
        "TestSynthesizedBaseStateUsesMappings_LoadCatalog");

    RuntimeWalkFitsOrientationInputs inputs;
    inputs.fits_path = "synthetic.fits";
    inputs.signals = {
        {"mean", 0.85},
        {"stddev", 0.40},
        {"center_bias", 0.20},
        {"residual_energy", 0.55},
        {"edge_balance", -0.25},
        {"frame_delta", 0.75},
        {"x_bias", 0.10},
        {"y_bias", -0.30},
        {"focus_ratio", 0.60},
    };

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    Check(SynthesizeRuntimeWalkBaseState(catalog, "explaino_default", inputs, &view, &params, &render, &error),
        "TestSynthesizedBaseStateUsesMappings_Synthesizes");
    Check(view.fractal_type == FractalType::explaino_fp,
        "TestSynthesizedBaseStateUsesMappings_ExplainoFamily");
    Check(render.resolution.x > 0 && render.resolution.y > 0,
        "TestSynthesizedBaseStateUsesMappings_RenderDefaults");
    Check(!NearlyEqual(view.center_hp_x, 0.0) || !NearlyEqual(view.center_hp_y, 0.0),
        "TestSynthesizedBaseStateUsesMappings_ViewTransportMapped");
    Check(!NearlyEqual(params.explaino_mix, 0.0f),
        "TestSynthesizedBaseStateUsesMappings_MixMapped");
    Check(!NearlyEqual(params.explaino_warp_strength, 0.0f),
        "TestSynthesizedBaseStateUsesMappings_WarpMapped");
}

static void TestWriteSynthesizedStateJsonWritesLoadableShape() {
    const std::filesystem::path root = TempRoot("write_state");
    const std::filesystem::path statePath = root / "synthesized_state.json";

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    view.fractal_type = FractalType::explaino_fp;
    render.resolution = {320, 240};
    render.block_size = 256;
    render.device_id = 0;
    std::string error;
    Check(WriteRuntimeWalkSynthesizedStateJson(statePath.string(), view, params, render, &error),
        "TestWriteSynthesizedStateJsonWritesLoadableShape_Writes");

    const std::string text = [] (const std::filesystem::path& path) {
        std::ifstream in(path, std::ios::in | std::ios::binary);
        return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    }(statePath);
    Check(text.find("\"state_version\": 3") != std::string::npos,
        "TestWriteSynthesizedStateJsonWritesLoadableShape_HasStateVersion");
    Check(text.find("\"fractal_type\": \"explaino_fp\"") != std::string::npos,
        "TestWriteSynthesizedStateJsonWritesLoadableShape_HasFractalType");
}

int main() {
    TestMappingCatalogRejectsUnsupportedTargetPath();
    TestOrientationInputsRequireFitsPath();
    TestSynthesizedBaseStateUsesMappings();
    TestWriteSynthesizedStateJsonWritesLoadableShape();

    std::printf("test_runtime_walk_bootstrap: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}
