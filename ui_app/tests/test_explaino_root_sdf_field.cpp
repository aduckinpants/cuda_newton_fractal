#include "explaino_root_sdf_field.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <string>

static int g_pass = 0;
static int g_fail = 0;

static void Check(bool cond, const char* msg, int line) {
    if (cond) {
        ++g_pass;
    } else {
        ++g_fail;
        std::cerr << "FAIL line " << line << ": " << msg << "\n";
    }
}

#define CHECK(cond, msg) Check((cond), (msg), __LINE__)

static bool Near(float a, float b, float tol = 1.0e-5f) {
    return std::fabs(a - b) <= tol * (std::max)(1.0f, (std::max)(std::fabs(a), std::fabs(b)));
}

static KernelParams MakeRootSdfParams4() {
    KernelParams params{};
    params.explaino_root_count = 4;
    params.explaino_roots[0] = {0.0f, 0.0f};
    params.explaino_roots[1] = {0.8f, 0.0f};
    params.explaino_roots[2] = {0.0f, 0.8f};
    params.explaino_roots[3] = {0.8f, 0.8f};
    params.explaino_root_sdf_radius = 0.25f;
    params.explaino_root_sdf_bridge_width = 0.08f;
    params.explaino_root_sdf_smooth_blend = 0.0f;
    return params;
}

static KernelParams MakeRegularNgonParams(int count) {
    KernelParams params{};
    params.explaino_root_authority = ExplainoRootAuthority::generated;
    params.explaino_generated_root_layout = ExplainoGeneratedRootLayout::regular_ngon_v1;
    params.explaino_generated_root_count = count;
    params.explaino_root_spread = 0.25f;
    params.explaino_root_sdf_radius = 0.12f;
    params.explaino_root_sdf_bridge_width = 0.08f;
    params.explaino_root_sdf_smooth_blend = 0.0f;
    return params;
}

static SdfPackFieldRegion UnitRegion() {
    SdfPackFieldRegion region{};
    region.has_region = true;
    region.center_x = 0.0;
    region.center_y = 0.0;
    region.half_height = 1.0;
    return region;
}

static void TestSceneResolutionBridgeTopologyAndHashes() {
    ViewState view{};
    KernelParams params = MakeRootSdfParams4();
    ExplainoRootSdfResolvedScene scene{};
    std::string error;

    CHECK(ResolveExplainoRootSdfScene(view, params, &scene, &error), "four-root scene resolves");
    CHECK(scene.root_count == 4, "four-root scene keeps root count");
    CHECK(scene.bridge_count == 2, "four-root scene creates semantic bridges 0-1 and 2-3");
    CHECK(scene.base_root_hash == scene.effective_root_hash, "static h source keeps effective roots equal to base roots");
    CHECK(Near(scene.effective_roots[1].x, params.explaino_roots[1].x), "static root preserves coordinate order");

    params.explaino_root_count = 3;
    params.explaino_roots[2] = {0.0f, 0.8f};
    CHECK(ResolveExplainoRootSdfScene(view, params, &scene, &error), "three-root scene resolves");
    CHECK(scene.bridge_count == 1, "three-root scene creates only semantic bridge 0-1");

    params.explaino_root_sdf_bridge_width = 0.0f;
    CHECK(ResolveExplainoRootSdfScene(view, params, &scene, &error), "zero bridge-width scene resolves");
    CHECK(scene.bridge_count == 0, "zero bridge width omits bridge primitives");
}

static void TestRegularNgonSceneBridgeTopology() {
    ViewState view{};
    view.fractal_type = FractalType::explaino_root_sdf;
    ExplainoRootSdfResolvedScene scene{};
    std::string error;

    KernelParams twoRoots = MakeRegularNgonParams(2);
    CHECK(ResolveExplainoRootSdfScene(view, twoRoots, &scene, &error),
        "regular N-gon two-root scene resolves");
    CHECK(scene.root_count == 2, "regular N-gon two-root scene keeps requested count");
    CHECK(scene.bridge_count == 1, "regular N-gon two-root scene has one undirected bridge");

    KernelParams fiveRoots = MakeRegularNgonParams(5);
    CHECK(ResolveExplainoRootSdfScene(view, fiveRoots, &scene, &error),
        "regular N-gon five-root scene resolves");
    CHECK(scene.root_count == 5, "regular N-gon five-root scene keeps requested count");
    CHECK(scene.bridge_count == 5, "regular N-gon five-root scene uses ring bridges");

    KernelParams sixteenRoots = MakeRegularNgonParams(16);
    CHECK(ResolveExplainoRootSdfScene(view, sixteenRoots, &scene, &error),
        "regular N-gon sixteen-root scene resolves");
    CHECK(scene.root_count == 16, "regular N-gon sixteen-root scene keeps requested count");
    CHECK(scene.bridge_count == 16, "regular N-gon sixteen-root scene uses ring bridges");

    sixteenRoots.explaino_root_sdf_bridge_width = 0.0f;
    CHECK(ResolveExplainoRootSdfScene(view, sixteenRoots, &scene, &error),
        "regular N-gon zero bridge-width scene resolves");
    CHECK(scene.bridge_count == 0, "regular N-gon zero bridge width omits bridge primitives");
}

static void TestRegularNgonN4DiffersFromLegacy() {
    ViewState view{};
    KernelParams legacy = MakeRootSdfParams4();
    KernelParams regular = MakeRegularNgonParams(4);
    ExplainoRootSdfResolvedScene legacyScene{};
    ExplainoRootSdfResolvedScene regularScene{};
    std::string error;

    CHECK(ResolveExplainoRootSdfScene(view, legacy, &legacyScene, &error),
        "legacy four-root scene resolves for comparison");
    view.fractal_type = FractalType::explaino_root_sdf;
    CHECK(ResolveExplainoRootSdfScene(view, regular, &regularScene, &error),
        "regular four-root scene resolves for comparison");
    CHECK(legacyScene.bridge_count == 2, "legacy four-root scene keeps old two-bridge topology");
    CHECK(regularScene.bridge_count == 4, "regular four-root scene uses ring topology");
    CHECK(legacyScene.base_root_hash != regularScene.base_root_hash,
        "regular four-root layout is distinct from legacy layout");
}

static void TestSceneResolutionFailsMalformedRoots() {
    ViewState view{};
    KernelParams params = MakeRootSdfParams4();
    ExplainoRootSdfResolvedScene scene{};
    std::string error;

    params.explaino_root_count = 2;
    CHECK(!ResolveExplainoRootSdfScene(view, params, &scene, &error),
        "root-SDF rejects unsupported two-root topology");

    params = MakeRootSdfParams4();
    params.explaino_roots[2].x = std::numeric_limits<float>::quiet_NaN();
    CHECK(!ResolveExplainoRootSdfScene(view, params, &scene, &error),
        "root-SDF rejects nonfinite captured roots");
}

static void TestPhaseSineModulationIsDeterministicAndLocal() {
    ViewState view{};
    view.explaino_phase = 0.125f;
    KernelParams params = MakeRootSdfParams4();
    params.explaino_root_sdf_h_source = ExplainoRootSdfHSource::phase_sine;
    params.explaino_root_sdf_h_amplitude = 0.20f;
    params.explaino_root_sdf_h_frequency = 1.0f;

    ExplainoRootSdfResolvedScene scene{};
    std::string error;
    CHECK(ResolveExplainoRootSdfScene(view, params, &scene, &error),
        "phase-sine h source resolves");
    CHECK(scene.base_root_hash != scene.effective_root_hash,
        "phase-sine h source changes effective roots without changing base roots");
    CHECK(Near(scene.base_roots[0].x, params.explaino_roots[0].x) &&
          Near(scene.base_roots[0].y, params.explaino_roots[0].y),
        "phase-sine h source keeps base roots as captured authority");

    KernelParams duplicateRoots = params;
    duplicateRoots.explaino_roots[0] = {0.0f, 0.0f};
    duplicateRoots.explaino_roots[1] = {0.0f, 0.0f};
    duplicateRoots.explaino_roots[2] = {0.0f, 0.0f};
    duplicateRoots.explaino_roots[3] = {0.0f, 0.0f};
    CHECK(ResolveExplainoRootSdfScene(view, duplicateRoots, &scene, &error),
        "phase-sine h source handles root-at-centroid fallback directions");
    for (int index = 0; index < scene.root_count; ++index) {
        CHECK(std::isfinite(scene.effective_roots[index].x) && std::isfinite(scene.effective_roots[index].y),
            "centroid fallback effective root stays finite");
    }
}

static void TestCpuFieldProducerMetadataAndSign() {
    ViewState view{};
    KernelParams params = MakeRootSdfParams4();
    params.explaino_root_sdf_bridge_width = 0.0f;

    SdfFieldResult field;
    SdfPackFieldReport packReport;
    ExplainoRootSdfFieldReport rootReport;
    std::string error;
    CHECK(ComputeExplainoRootSdfFieldForViewport(
              view,
              params,
              UnitRegion(),
              17,
              17,
              1,
              SdfPackFieldBackend::cpu_reference,
              field,
              &packReport,
              &rootReport,
              &error),
        "CPU ExplainO root-SDF field computes");
    if (!error.empty()) {
        std::cerr << error << "\n";
    }
    CHECK(field.width == 17 && field.height == 17, "root-SDF field keeps requested dimensions at 1x downsample");
    CHECK(field.source_kind == SdfFieldSourceKind::explaino_root_sdf, "root-SDF field reports source kind");
    CHECK(packReport.used == SdfPackFieldBackend::cpu_reference, "root-SDF CPU field reports CPU backend");
    CHECK(rootReport.root_count == 4 && rootReport.bridge_count == 0, "root-SDF report includes root and bridge counts");
    CHECK(std::string(rootReport.h_source) == "none", "root-SDF report includes h source");
    CHECK(rootReport.base_root_hash == rootReport.effective_root_hash, "root-SDF report hashes static roots equally");

    bool sawNegative = false;
    bool sawPositive = false;
    bool allFinite = true;
    for (float value : field.signed_distance_px) {
        allFinite = allFinite && std::isfinite(value);
        sawNegative = sawNegative || value < 0.0f;
        sawPositive = sawPositive || value > 0.0f;
    }
    CHECK(allFinite, "root-SDF field distances are finite");
    CHECK(sawNegative, "root-SDF field has negative values inside root circles");
    CHECK(sawPositive, "root-SDF field has positive values outside root circles");

    SdfFieldResult downsampledField;
    CHECK(ComputeExplainoRootSdfFieldForViewport(
              view,
              params,
              UnitRegion(),
              17,
              17,
              4,
              SdfPackFieldBackend::cpu_reference,
              downsampledField,
              nullptr,
              nullptr,
              &error),
        "root-SDF field computes with effective downsample");
    CHECK(downsampledField.width == 5 && downsampledField.height == 5 && Near(downsampledField.pixel_scale, 4.0f),
        "root-SDF field reports downsampled dimensions and pixel scale");
}

int main() {
    TestSceneResolutionBridgeTopologyAndHashes();
    TestRegularNgonSceneBridgeTopology();
    TestRegularNgonN4DiffersFromLegacy();
    TestSceneResolutionFailsMalformedRoots();
    TestPhaseSineModulationIsDeterministicAndLocal();
    TestCpuFieldProducerMetadataAndSign();

    std::cout << "test_explaino_root_sdf_field: pass=" << g_pass << " fail=" << g_fail << "\n";
    return g_fail == 0 ? 0 : 1;
}
