#include "runtime_walk.h"

#include "explaino_seed.h"

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

static void TestParseRuntimeWalkBundleJson() {
    RuntimeWalkBundle bundle;
    std::string error;
    Check(ParseRuntimeWalkBundleJson(kValidBundleJson, &bundle, &error),
        "TestParseRuntimeWalkBundleJson_Succeeds");
    Check(error.empty(), "TestParseRuntimeWalkBundleJson_NoError");
    Check(bundle.field_name == "mr_zipper_branch", "TestParseRuntimeWalkBundleJson_FieldName");
    Check(bundle.samples.size() == 3, "TestParseRuntimeWalkBundleJson_SampleCount");
    Check(bundle.branch_markers.size() == 1, "TestParseRuntimeWalkBundleJson_BranchMarkerCount");
}

static void TestParseRuntimeWalkBundleRejectsMalformedChannelCount() {
    const char* malformed = R"JSON({
      "version": 1,
      "field_name": "mr_zipper_branch",
      "samples": [
        {
          "id": "bad",
          "t": 0.0,
          "channels": [0, 0, 0]
        }
      ]
    })JSON";
    RuntimeWalkBundle bundle;
    std::string error;
    Check(!ParseRuntimeWalkBundleJson(malformed, &bundle, &error),
        "TestParseRuntimeWalkBundleRejectsMalformedChannelCount_Fails");
    Check(!error.empty(), "TestParseRuntimeWalkBundleRejectsMalformedChannelCount_Error");
}

static void TestEvaluateRuntimeWalkSnapshotInterpolatesAndAnnotates() {
    RuntimeWalkBundle bundle;
    std::string error;
    Check(ParseRuntimeWalkBundleJson(kValidBundleJson, &bundle, &error),
        "TestEvaluateRuntimeWalkSnapshotInterpolatesAndAnnotates_Parse");

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

    RuntimeWalkSnapshot snapshot;
    Check(EvaluateRuntimeWalkSnapshot(bundle, 0.25, baseView, baseParams, render, &snapshot, &error),
        "TestEvaluateRuntimeWalkSnapshotInterpolatesAndAnnotates_Eval");
    Check(std::fabs(snapshot.channels[0] - 0.2) < 1.0e-9,
        "TestEvaluateRuntimeWalkSnapshotInterpolatesAndAnnotates_ChannelInterp");
    Check(std::fabs(snapshot.seed01 - 0.2839285714) < 1.0e-6,
        "TestEvaluateRuntimeWalkSnapshotInterpolatesAndAnnotates_Seed01");
    Check(snapshot.branch.nearest_marker_id == "fork_a",
        "TestEvaluateRuntimeWalkSnapshotInterpolatesAndAnnotates_NearestBranch");
    Check(!snapshot.branch.sticky,
        "TestEvaluateRuntimeWalkSnapshotInterpolatesAndAnnotates_NotStickyYet");

    Check(EvaluateRuntimeWalkSnapshot(bundle, 0.52, baseView, baseParams, render, &snapshot, &error),
        "TestEvaluateRuntimeWalkSnapshotInterpolatesAndAnnotates_EvalSticky");
    Check(snapshot.branch.sticky,
        "TestEvaluateRuntimeWalkSnapshotInterpolatesAndAnnotates_Sticky");
    Check(snapshot.branch.proximity > 0.0 && snapshot.branch.proximity <= 1.0,
        "TestEvaluateRuntimeWalkSnapshotInterpolatesAndAnnotates_Proximity");
}

static void TestApplyRuntimeWalkSnapshotWritesExplainoState() {
    ViewState view{};
    KernelParams params{};
    view.fractal_type = FractalType::explaino_fp;
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    view.explaino_phase = 0.0f;
    params.explaino_seed = 0.0;
    params.explaino_seed_b = 0.0;
    params.explaino_mix = 0.0f;
    params.explaino_warp_strength = 0.0f;

    RuntimeWalkSnapshot snapshot{};
    snapshot.center_hp_x = 0.25;
    snapshot.center_hp_y = -0.125;
    snapshot.log2_zoom = 0.75;
    snapshot.phase = 1.5;
    snapshot.combined_seed = 12.25;
    snapshot.seed_b = -0.5;
    snapshot.mix = 0.9;
    snapshot.warp_strength = 0.35;

    ApplyRuntimeWalkSnapshot(snapshot, &view, &params);
    Check(std::fabs(view.center_hp_x - 0.25) < 1.0e-12,
        "TestApplyRuntimeWalkSnapshotWritesExplainoState_CenterX");
    Check(std::fabs(view.center_hp_y + 0.125) < 1.0e-12,
        "TestApplyRuntimeWalkSnapshotWritesExplainoState_CenterY");
    Check(std::fabs(view.log2_zoom - 0.75) < 1.0e-12,
        "TestApplyRuntimeWalkSnapshotWritesExplainoState_Log2Zoom");
    Check(std::fabs(view.explaino_phase - 1.5) < 1.0e-6,
        "TestApplyRuntimeWalkSnapshotWritesExplainoState_Phase");
    Check(std::fabs(ExplainoSeedCombined(view, params) - 12.25) < 1.0e-6,
        "TestApplyRuntimeWalkSnapshotWritesExplainoState_CombinedSeed");
    Check(std::fabs(params.explaino_seed_b + 0.5) < 1.0e-6,
        "TestApplyRuntimeWalkSnapshotWritesExplainoState_SeedB");
    Check(std::fabs(params.explaino_mix - 0.9f) < 1.0e-6,
        "TestApplyRuntimeWalkSnapshotWritesExplainoState_Mix");
    Check(std::fabs(params.explaino_warp_strength - 0.35f) < 1.0e-6,
        "TestApplyRuntimeWalkSnapshotWritesExplainoState_Warp");
}

int main() {
    TestParseRuntimeWalkBundleJson();
    TestParseRuntimeWalkBundleRejectsMalformedChannelCount();
    TestEvaluateRuntimeWalkSnapshotInterpolatesAndAnnotates();
    TestApplyRuntimeWalkSnapshotWritesExplainoState();

    std::printf("test_runtime_walk: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}
