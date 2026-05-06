#include "../src/runtime_walk_bootstrap.h"
#include "../src/explaino_seed.h"

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



static void TestMappingCatalogRejectsWarpTargetByDefault() {
    const std::string json = R"JSON({
  "version": 1,
  "profiles": [
    {
      "id": "bad_warp",
      "target_selector": "explaino",
      "base_fractal_type": "explaino",
      "bindings": [
        {
          "target_selector": "explaino",
          "source_signal": "mean",
          "target_path": "params.explaino_warp_strength",
          "input_min": 0.0,
          "input_max": 1.0,
          "scale": 0.02,
          "offset": 0.0
        }
      ]
    }
  ]
})JSON";

    RuntimeWalkFitsMappingCatalog catalog;
    std::string error;
    Check(!ParseRuntimeWalkFitsMappingCatalogJson(json, &catalog, &error),
        "TestMappingCatalogRejectsWarpTargetByDefault_Fails");
    Check(error.find("Unsupported runtime-walk FITS mapping target path") != std::string::npos,
        "TestMappingCatalogRejectsWarpTargetByDefault_Error");
}

static void TestMappingCatalogAcceptsSchemaDerivedExplainoDampingTarget() {
    const std::string json = R"JSON({
  "version": 1,
  "profiles": [
    {
      "id": "safe_damping",
      "target_selector": "explaino",
      "base_fractal_type": "explaino",
      "bindings": [
        {
          "target_selector": "explaino",
          "source_signal": "residual_energy",
          "target_path": "params.explaino_damping",
          "input_min": 0.0,
          "input_max": 1.0,
          "scale": 0.25,
          "offset": 0.75,
          "clamp_min": 0.1,
          "clamp_max": 1.5
        }
      ]
    }
  ]
})JSON";

    RuntimeWalkFitsMappingCatalog catalog;
    std::string error;
    Check(ParseRuntimeWalkFitsMappingCatalogJson(json, &catalog, &error),
        "TestMappingCatalogAcceptsSchemaDerivedExplainoDampingTarget_Parse");
    Check(catalog.profiles.size() == 1u && catalog.profiles[0].bindings[0].target_path == "fractal.params.explaino_damping",
        "TestMappingCatalogAcceptsSchemaDerivedExplainoDampingTarget_TargetRetained");
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


static void TestOrientationInputsParseFrameTimeline() {
    const std::string json = R"JSON({
  "version": 1,
  "fits_path": "timeline.fits",
  "frame_count": 3,
  "metadata": {
    "ORIENT": "godel-test"
  },
  "signals": {
    "mean": 0.5,
    "stddev": 0.2
  },
  "frames": [
    {
      "frame_index": 0,
      "t": 0.0,
      "signals": {"mean": 0.1, "x_bias": -0.4}
    },
    {
      "frame_index": 1,
      "t": 0.5,
      "signals": {"mean": 0.6, "x_bias": 0.0}
    },
    {
      "frame_index": 2,
      "t": 1.0,
      "signals": {"mean": 0.9, "x_bias": 0.4}
    }
  ]
})JSON";

    RuntimeWalkFitsOrientationInputs inputs;
    std::string error;
    Check(ParseRuntimeWalkFitsOrientationInputsJson(json, &inputs, &error),
        "TestOrientationInputsParseFrameTimeline_Parse");
    Check(inputs.frames.size() == 3u,
        "TestOrientationInputsParseFrameTimeline_FrameCount");
    Check(inputs.frames[1].frame_index == 1 && NearlyEqual(inputs.frames[1].t, 0.5),
        "TestOrientationInputsParseFrameTimeline_FrameMetadata");
    Check(inputs.metadata.find("ORIENT") != inputs.metadata.end() && inputs.metadata["ORIENT"] == "godel-test",
        "TestOrientationInputsParseFrameTimeline_Metadata");

    double value = 0.0;
    int frameIndex = -1;
    Check(EvaluateRuntimeWalkFitsSignalAtT(inputs, "mean", 0.75, &value, &frameIndex, &error),
        "TestOrientationInputsParseFrameTimeline_Evaluate");
    Check(frameIndex == 1 && NearlyEqual(value, 0.75),
        "TestOrientationInputsParseFrameTimeline_Interpolates");
}

static void TestLiveFitsBindingsApplyOffsetsOverBaseline() {
    RuntimeWalkFitsOrientationInputs inputs;
    inputs.fits_path = "timeline.fits";
    inputs.signals = {{"mean", 0.5}};
    inputs.frames = {
        RuntimeWalkFitsSignalFrame{0, 0.0, {{"mean", 0.0}}},
        RuntimeWalkFitsSignalFrame{1, 1.0, {{"mean", 1.0}}},
    };

    RuntimeWalkFitsMappingBinding binding;
    binding.source_kind = "fits_frame";
    binding.source_path = "fits.frame.mean";
    binding.source_signal = "mean";
    binding.target_path = "fractal.params.explaino_seed";
    binding.input_min = 0.0;
    binding.input_max = 1.0;
    binding.scale = 4.0;
    binding.offset = -2.0;
    binding.weight = 1.0;
    binding.smoothing = 1.0;

    RuntimeWalkFitsMappingBinding mixBinding;
    mixBinding.source_kind = "fits_frame";
    mixBinding.source_path = "fits.frame.mean";
    mixBinding.source_signal = "mean";
    mixBinding.target_path = "fractal.params.explaino_mix";
    mixBinding.input_min = 0.0;
    mixBinding.input_max = 1.0;
    mixBinding.scale = 2.0;
    mixBinding.offset = 0.0;
    mixBinding.weight = 1.0;
    mixBinding.has_clamp = true;
    mixBinding.clamp_min = 0.0;
    mixBinding.clamp_max = 4.0;

    RuntimeWalkFitsMappingBinding phaseBinding;
    phaseBinding.source_kind = "field";
    phaseBinding.source_path = "field.traveler.confidence";
    phaseBinding.source_signal = "field.traveler.confidence";
    phaseBinding.target_path = "fractal.view.explaino_phase_strength";
    phaseBinding.input_min = 0.0;
    phaseBinding.input_max = 1.0;
    phaseBinding.scale = 0.18;
    phaseBinding.offset = -0.09;
    phaseBinding.weight = 1.0;
    phaseBinding.has_clamp = true;
    phaseBinding.clamp_min = 0.0;
    phaseBinding.clamp_max = 1.0;

    ViewState baselineView{};
    KernelParams baselineParams{};
    baselineView.fractal_type = FractalType::explaino;
    baselineView.explaino_phase_strength = 1.0f;
    baselineParams.explaino_mix = 0.75f;
    ExplainoSeedSetCombined(baselineView, baselineParams, 10.0);
    baselineParams.explaino_warp_strength = 0.37f;

    RuntimeWalkFitsFieldSignals fieldSignals{};
    fieldSignals.traveler_confidence = 0.0;
    ViewState composedView{};
    KernelParams composedParams{};
    std::vector<RuntimeWalkFitsLiveBindingResult> results;
    std::string error;
    Check(ComposeRuntimeWalkFitsBindingsOverLiveBaseline({binding, mixBinding, phaseBinding}, inputs, fieldSignals, 1.0,
            baselineView, baselineParams, &composedView, &composedParams, &results, &error),
        "TestLiveFitsBindingsApplyOffsetsOverBaseline_Compose");
    Check(NearlyEqual(ExplainoSeedCombined(composedView, composedParams), 12.0, 1.0e-6),
        "TestLiveFitsBindingsApplyOffsetsOverBaseline_SeedOffset");
    Check(NearlyEqual(composedParams.explaino_warp_strength, baselineParams.explaino_warp_strength, 1.0e-6),
        "TestLiveFitsBindingsApplyOffsetsOverBaseline_WarpPreserved");
    Check(results.size() == 3u && results[0].ok && NearlyEqual(results[0].offset_value, 2.0),
        "TestLiveFitsBindingsApplyOffsetsOverBaseline_ResultRecorded");
    Check(NearlyEqual(composedParams.explaino_mix, 1.0, 1.0e-6) && results[1].ok && results[1].clamped,
        "TestLiveFitsBindingsApplyOffsetsOverBaseline_MixTargetDomainClamped");
    Check(results[2].ok && NearlyEqual(results[2].offset_value, -0.09, 1.0e-6),
        "TestLiveFitsBindingsApplyOffsetsOverBaseline_NegativeFieldOffsetPreserved");
    Check(NearlyEqual(composedView.explaino_phase_strength, 0.91, 1.0e-6),
        "TestLiveFitsBindingsApplyOffsetsOverBaseline_FieldOffsetComposesBeforeClamp");
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
    Check(view.fractal_type == FractalType::explaino,
        "TestSynthesizedBaseStateUsesMappings_ExplainoFamily");
    Check(render.resolution.x > 0 && render.resolution.y > 0,
        "TestSynthesizedBaseStateUsesMappings_RenderDefaults");
    Check(!NearlyEqual(view.center_hp_x, 0.0) || !NearlyEqual(view.center_hp_y, 0.0),
        "TestSynthesizedBaseStateUsesMappings_ViewTransportMapped");
    Check(!NearlyEqual(params.explaino_mix, 0.0f),
        "TestSynthesizedBaseStateUsesMappings_MixMapped");
    Check(NearlyEqual(params.explaino_warp_strength, 0.0f),
        "TestSynthesizedBaseStateUsesMappings_WarpNeutralByDefault");
}

static void TestWriteSynthesizedStateJsonWritesLoadableShape() {
    const std::filesystem::path root = TempRoot("write_state");
    const std::filesystem::path statePath = root / "synthesized_state.json";

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    view.fractal_type = FractalType::explaino;
    params.color_phase_signal_offset = 1.25f;
    params.color_phase_wrap_cycles = 2.5f;
    params.color_phase_palette_offset = -0.75f;
    params.color_iteration_band_count = 5;
    params.color_iteration_band_softness = 0.8f;
    params.color_iteration_band_emphasis = 1.6f;
    params.color_iteration_band_palette_offset = 0.4f;
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
    Check(text.find("\"fractal_type\": \"explaino\"") != std::string::npos,
        "TestWriteSynthesizedStateJsonWritesLoadableShape_HasFractalType");
    Check(text.find("\"color_phase_signal_offset\": 1.25") != std::string::npos,
        "TestWriteSynthesizedStateJsonWritesLoadableShape_HasPhaseSignalOffset");
    Check(text.find("\"color_iteration_band_count\": 5") != std::string::npos,
        "TestWriteSynthesizedStateJsonWritesLoadableShape_HasBandCount");
}

static void TestSynthesizeRuntimeWalkTransportBundleBuildsPlayableShape() {
    RuntimeWalkFitsMappingCatalog catalog;
    std::string error;
    const std::filesystem::path profilePath = ResolveMappingProfilePath();
    Check(LoadRuntimeWalkFitsMappingCatalogFile(
            profilePath.string(),
            &catalog,
            &error),
        "TestSynthesizeRuntimeWalkTransportBundleBuildsPlayableShape_LoadCatalog");

    RuntimeWalkFitsOrientationInputs inputs;
    inputs.fits_path = "synthetic.fits";
    inputs.signals = {
        {"mean", 0.72},
        {"stddev", 0.35},
        {"center_bias", 0.18},
        {"residual_energy", 0.64},
        {"edge_balance", -0.22},
        {"frame_delta", 0.57},
        {"x_bias", 0.24},
        {"y_bias", -0.31},
        {"focus_ratio", 0.68},
    };

    RuntimeWalkBundle bundle;
    RuntimeWalkTransportSynthesisOptions options{};
    options.sample_count = 41u;
    options.motion_scale = 0.55;
    Check(SynthesizeRuntimeWalkTransportBundle(catalog, "explaino_default", inputs, options, &bundle, &error),
        "TestSynthesizeRuntimeWalkTransportBundleBuildsPlayableShape_Synthesizes");
    Check(bundle.field_name == "mr_zipper_branch",
        "TestSynthesizeRuntimeWalkTransportBundleBuildsPlayableShape_FieldName");
    Check(bundle.samples.size() >= 2u,
        "TestSynthesizeRuntimeWalkTransportBundleBuildsPlayableShape_HasSamples");
    Check(bundle.samples.size() == 41u,
        "TestSynthesizeRuntimeWalkTransportBundleBuildsPlayableShape_DenseSamples");
    Check(bundle.branch_markers.size() >= 1u,
        "TestSynthesizeRuntimeWalkTransportBundleBuildsPlayableShape_HasBranchMarker");
    Check(bundle.samples.front().t == 0.0 && bundle.samples.back().t == 1.0,
        "TestSynthesizeRuntimeWalkTransportBundleBuildsPlayableShape_TRange");
    Check(NearlyEqual(bundle.samples.front().channels[0], bundle.samples.back().channels[0]) &&
            NearlyEqual(bundle.samples.front().channels[6], bundle.samples.back().channels[6]),
        "TestSynthesizeRuntimeWalkTransportBundleBuildsPlayableShape_ClosedLoopEndpoints");
    Check(!NearlyEqual(bundle.samples[1].channels[0], bundle.samples.front().channels[0]) ||
            !NearlyEqual(bundle.samples[1].channels[5], bundle.samples.front().channels[5]),
        "TestSynthesizeRuntimeWalkTransportBundleBuildsPlayableShape_NotNeutralOnly");
    bool varied = false;
    for (std::size_t index = 1; index + 1u < bundle.samples.size(); ++index) {
        if (!NearlyEqual(bundle.samples[index].channels[0], bundle.samples[index - 1].channels[0]) ||
            !NearlyEqual(bundle.samples[index].channels[3], bundle.samples[index - 1].channels[3]) ||
            !NearlyEqual(bundle.samples[index].channels[6], bundle.samples[index - 1].channels[6])) {
            varied = true;
            break;
        }
    }
    Check(varied,
        "TestSynthesizeRuntimeWalkTransportBundleBuildsPlayableShape_VariedIntermediateSamples");
}

int main() {
    TestMappingCatalogRejectsUnsupportedTargetPath();
    TestMappingCatalogRejectsWarpTargetByDefault();
    TestMappingCatalogAcceptsSchemaDerivedExplainoDampingTarget();
    TestOrientationInputsRequireFitsPath();
    TestOrientationInputsParseFrameTimeline();
    TestLiveFitsBindingsApplyOffsetsOverBaseline();
    TestSynthesizedBaseStateUsesMappings();
    TestWriteSynthesizedStateJsonWritesLoadableShape();
    TestSynthesizeRuntimeWalkTransportBundleBuildsPlayableShape();

    std::printf("test_runtime_walk_bootstrap: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}
