#include "runtime_walk.h"

#include "diagnostics_capture.h"
#include "explaino_seed.h"
#include "explaino_sidecar_cuda_sample_host.h"
#include "explaino_sidecar_window.h"
#include "finding_state_actions.h"
#include "fractal_derived_fields.h"
#include "function_descriptor.h"
#include "lens_sdf.h"
#include "schema_binding.h"
#include "view_hp_sync.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

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

namespace {

struct StubState {
    bool request_load_ok = true;
    bool bundle_load_ok = true;
    bool state_load_ok = true;
    bool render_ok = false;
    bool evaluate_ok = true;
    bool capture_ok = true;
    bool sidecar_ok = true;

    RuntimeWalkRequest request;
    RuntimeWalkBundle bundle;
    ViewState loaded_view;
    KernelParams loaded_params;
    RenderSettings loaded_render;

    int request_load_calls = 0;
    int bundle_load_calls = 0;
    int state_load_calls = 0;
    int update_polynomial_calls = 0;
    int render_calls = 0;
    int capture_calls = 0;
    int evaluate_calls = 0;
    int apply_calls = 0;
    int sidecar_calls = 0;
    int sdf_calls = 0;

    std::string seen_request_path;
    std::string seen_bundle_path;
    std::string seen_state_path;
    RenderSettings last_render_settings;
    ViewState last_render_view;
};

StubState g_stub;

std::filesystem::path TestOutputRoot(const char* name) {
    return std::filesystem::temp_directory_path() / "cuda_newton_fractal_clone_runtime_walk_headless_tests" / name;
}

RuntimeWalkBundle MakeBundle() {
    RuntimeWalkBundle bundle;
    bundle.field_name = "mr_zipper_branch";
    bundle.samples.resize(2);
    bundle.samples[0].id = "start";
    bundle.samples[0].t = 0.0;
    bundle.samples[1].id = "end";
    bundle.samples[1].t = 1.0;
    return bundle;
}

void ResetStubs(const char* outputName) {
    g_stub = StubState{};
    g_stub.request.base_state_json_path = "state.json";
    g_stub.request.bundle_json_path = "bundle.json";
    g_stub.request.output_dir = TestOutputRoot(outputName).string();
    g_stub.request.t_values = {0.0, 1.0};
    g_stub.bundle = MakeBundle();
    g_stub.loaded_view.fractal_type = FractalType::explaino;
    g_stub.loaded_params.max_iter = 32;
    g_stub.loaded_render.resolution = {32, 24};
    g_stub.loaded_render.block_size = 128;
    g_stub.loaded_render.device_id = 2;
    g_stub.loaded_render.benchmark = true;
    std::filesystem::remove_all(TestOutputRoot(outputName));
}

int RunHeadless(LensSettings* outLens = nullptr) {
    EngineFunctionCatalog catalog;
    ViewState boundView;
    KernelParams boundParams;
    RenderSettings boundRender;
    LensSettings lens;
    BindingContext bind;
    bind.view = &boundView;
    bind.params = &boundParams;
    bind.render = &boundRender;
    bind.lens = &lens;
    CudaSidecarMeasurementHost measurementHost;
    SidecarAutoDemoControllerPolicy policy;

    const int result = RunRuntimeWalkRequest(
        "exe_dir",
        "request.json",
        catalog,
        bind,
        measurementHost,
        policy,
        lens);
    if (outLens) *outLens = lens;
    return result;
}

} // namespace

bool LoadRuntimeWalkRequestFile(const std::string& path,
    RuntimeWalkRequest* outRequest,
    std::string* outError) {
    ++g_stub.request_load_calls;
    g_stub.seen_request_path = path;
    if (outError) outError->clear();
    if (!g_stub.request_load_ok) {
        if (outError) *outError = "stub request load failure";
        return false;
    }
    if (outRequest) *outRequest = g_stub.request;
    return true;
}

bool LoadRuntimeWalkBundleFile(const std::string& path,
    RuntimeWalkBundle* outBundle,
    std::string* outError) {
    ++g_stub.bundle_load_calls;
    g_stub.seen_bundle_path = path;
    if (outError) outError->clear();
    if (!g_stub.bundle_load_ok) {
        if (outError) *outError = "stub bundle load failure";
        return false;
    }
    if (outBundle) *outBundle = g_stub.bundle;
    return true;
}

bool LoadFindingSelectionIntoRuntime(const std::string& selectedPath,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    SidecarAutoDemoMutationHistory* outMutationHistory,
    bool* outHasMutationHistory,
    std::string* outResolvedStatePath,
    std::string* outError) {
    ++g_stub.state_load_calls;
    g_stub.seen_state_path = selectedPath;
    if (outError) outError->clear();
    if (!g_stub.state_load_ok) {
        if (outError) *outError = "stub base state load failure";
        return false;
    }
    if (ioView) *ioView = g_stub.loaded_view;
    if (ioParams) *ioParams = g_stub.loaded_params;
    if (ioRender) *ioRender = g_stub.loaded_render;
    if (outOrientation) *outOrientation = SidecarOrientationVector{};
    if (outHasOrientation) *outHasOrientation = false;
    if (outControllerPolicy) *outControllerPolicy = SidecarAutoDemoControllerPolicy{};
    if (outHasControllerPolicy) *outHasControllerPolicy = false;
    if (outMutationHistory) outMutationHistory->clear();
    if (outHasMutationHistory) *outHasMutationHistory = false;
    if (outResolvedStatePath) *outResolvedStatePath = "resolved_state.json";
    return true;
}

void UpdateExplainoPolynomial(const ViewState&, KernelParams&, bool*) {
    ++g_stub.update_polynomial_calls;
}

bool RenderFractalCUDA(const ViewState& view,
    const KernelParams&,
    const RenderSettings& render,
    uint32_t* outRGBA,
    uint8_t* outMask,
    RenderStats* outStats,
    const char** outError) {
    ++g_stub.render_calls;
    g_stub.last_render_settings = render;
    g_stub.last_render_view = view;
    if (outStats) {
        outStats->last_device_id = render.device_id;
        outStats->last_iters_avg = 7;
        outStats->last_render_ms = 0.25f;
    }
    if (!g_stub.render_ok) {
        if (outError) *outError = "stub render failure";
        return false;
    }
    const std::size_t pixelCount = static_cast<std::size_t>(render.resolution.x) * static_cast<std::size_t>(render.resolution.y);
    if (outRGBA) std::fill(outRGBA, outRGBA + pixelCount, 0xFF223344u);
    if (outMask) std::fill(outMask, outMask + pixelCount, 255u);
    if (outError) *outError = nullptr;
    return true;
}

bool CaptureDiagnosticsBundleToDir(const std::string& outputDir,
    const ViewState&,
    const KernelParams&,
    const RenderSettings&,
    const RenderStats&,
    const uint32_t*,
    std::size_t,
    const SidecarOrientationVector*,
    const SidecarAutoDemoControllerPolicy*,
    const SidecarAutoDemoMutationHistory*,
    DiagnosticsCaptureResult* outResult,
    std::string* outError) {
    ++g_stub.capture_calls;
    if (outError) outError->clear();
    if (!g_stub.capture_ok) {
        if (outError) *outError = "stub capture failure";
        return false;
    }
    if (outResult) {
        outResult->output_dir = outputDir;
        outResult->frame_bmp_path = outputDir + "/frame.bmp";
        outResult->state_json_path = outputDir + "/state.json";
    }
    return true;
}

void ComputeSignedDistanceSdfChamfer(const uint8_t*,
    int width,
    int height,
    float,
    std::vector<uint32_t>& outRgba) {
    ++g_stub.sdf_calls;
    outRgba.assign(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), 0xFF000000u);
}

bool SampleSignedDistanceSdfChamfer(const uint8_t*,
    int,
    int,
    int,
    int,
    float& outSignedPx,
    bool& outInside) {
    outSignedPx = 0.0f;
    outInside = false;
    return true;
}

bool EvaluateRuntimeWalkSnapshot(const RuntimeWalkBundle&,
    double t,
    const ViewState& baseView,
    const KernelParams&,
    const RenderSettings&,
    RuntimeWalkSnapshot* outSnapshot,
    std::string* outError) {
    ++g_stub.evaluate_calls;
    if (outError) outError->clear();
    if (!g_stub.evaluate_ok) {
        if (outError) *outError = "stub snapshot failure";
        return false;
    }
    if (outSnapshot) {
        *outSnapshot = RuntimeWalkSnapshot{};
        outSnapshot->t = t;
        outSnapshot->center_hp_x = baseView.center_hp_x;
        outSnapshot->center_hp_y = baseView.center_hp_y;
        outSnapshot->log2_zoom = baseView.log2_zoom;
    }
    return true;
}

void ApplyRuntimeWalkSnapshot(const RuntimeWalkSnapshot& snapshot,
    ViewState* ioView,
    KernelParams*) {
    ++g_stub.apply_calls;
    if (ioView) {
        ioView->center_hp_x = snapshot.center_hp_x;
        ioView->center_hp_y = snapshot.center_hp_y;
        ioView->log2_zoom = snapshot.log2_zoom;
    }
}

bool BuildExplainoSidecarWindowState(const EngineFunctionCatalog&,
    const BindingContext&,
    const SidecarMeasurementHost*,
    const SidecarBudgetState*,
    const SidecarExplorationCompleteness*,
    const SidecarOrientationVector*,
    const SidecarSlimeTrace*,
    const SidecarAutoDemoControllerPolicy*,
    ExplainoSidecarWindowState* outState,
    std::string* outError) {
    ++g_stub.sidecar_calls;
    if (outError) outError->clear();
    if (!g_stub.sidecar_ok) {
        if (outError) *outError = "stub sidecar failure";
        return false;
    }
    if (outState) *outState = ExplainoSidecarWindowState{};
    return true;
}

double ExplainoSeedCombined(const ViewState& view, const KernelParams& params) {
    return params.explaino_seed + static_cast<double>(view.explaino_seed_drift);
}

double ClampD(double value, double lo, double hi) {
    return std::min(std::max(value, lo), hi);
}

double SafeZoomFromLog2(double log2Zoom) {
    return std::pow(2.0, log2Zoom);
}

bool CudaSidecarMeasurementHost::SupportsWidenedEvidence() const {
    return false;
}

bool CudaSidecarMeasurementHost::SampleEvidence(const std::vector<Double2>&,
    const ViewState&,
    const KernelParams&,
    const RenderSettings&,
    std::vector<FractalSampleEvidence>*,
    std::string* outError) const {
    if (outError) *outError = "stub sidecar measurement should not sample widened evidence in this test";
    return false;
}

bool CudaSidecarMeasurementHost::Sample(const std::vector<Double2>&,
    const ViewState&,
    const KernelParams&,
    const RenderSettings&,
    std::vector<FractalSampleResult>*,
    std::string* outError) const {
    if (outError) *outError = "stub sidecar measurement should not sample in this test";
    return false;
}

static void TestRejectsRequestLoadFailureBeforeBundleLoad() {
    ResetStubs("request_load_failure");
    g_stub.request_load_ok = false;

    Check(RunHeadless() == 1, "TestRejectsRequestLoadFailureBeforeBundleLoad_ReturnsFailure");
    Check(g_stub.request_load_calls == 1, "TestRejectsRequestLoadFailureBeforeBundleLoad_LoadsRequestOnce");
    Check(g_stub.seen_request_path == "request.json", "TestRejectsRequestLoadFailureBeforeBundleLoad_RequestPath");
    Check(g_stub.bundle_load_calls == 0, "TestRejectsRequestLoadFailureBeforeBundleLoad_NoBundleLoad");
    Check(g_stub.state_load_calls == 0, "TestRejectsRequestLoadFailureBeforeBundleLoad_NoStateLoad");
    Check(g_stub.render_calls == 0, "TestRejectsRequestLoadFailureBeforeBundleLoad_NoRender");
}

static void TestRejectsBundleLoadFailureBeforeBaseState() {
    ResetStubs("bundle_load_failure");
    g_stub.bundle_load_ok = false;

    Check(RunHeadless() == 1, "TestRejectsBundleLoadFailureBeforeBaseState_ReturnsFailure");
    Check(g_stub.request_load_calls == 1, "TestRejectsBundleLoadFailureBeforeBaseState_LoadsRequestOnce");
    Check(g_stub.bundle_load_calls == 1, "TestRejectsBundleLoadFailureBeforeBaseState_LoadsBundleOnce");
    Check(g_stub.seen_bundle_path == "bundle.json", "TestRejectsBundleLoadFailureBeforeBaseState_BundlePath");
    Check(g_stub.state_load_calls == 0, "TestRejectsBundleLoadFailureBeforeBaseState_NoStateLoad");
    Check(g_stub.render_calls == 0, "TestRejectsBundleLoadFailureBeforeBaseState_NoRender");
}

static void TestRejectsBaseStateLoadFailureBeforeRender() {
    ResetStubs("base_state_load_failure");
    g_stub.state_load_ok = false;

    Check(RunHeadless() == 1, "TestRejectsBaseStateLoadFailureBeforeRender_ReturnsFailure");
    Check(g_stub.request_load_calls == 1, "TestRejectsBaseStateLoadFailureBeforeRender_LoadsRequestOnce");
    Check(g_stub.bundle_load_calls == 1, "TestRejectsBaseStateLoadFailureBeforeRender_LoadsBundleOnce");
    Check(g_stub.state_load_calls == 1, "TestRejectsBaseStateLoadFailureBeforeRender_LoadsStateOnce");
    Check(g_stub.seen_state_path == "state.json", "TestRejectsBaseStateLoadFailureBeforeRender_StatePath");
    Check(g_stub.render_calls == 0, "TestRejectsBaseStateLoadFailureBeforeRender_NoRender");
    Check(!std::filesystem::exists(TestOutputRoot("base_state_load_failure")), "TestRejectsBaseStateLoadFailureBeforeRender_NoOutputDir");
}

static void TestRejectsNonExplainoBaseStateBeforeOutputWork() {
    ResetStubs("non_explaino_base_state");
    g_stub.loaded_view.fractal_type = FractalType::mandelbrot;
    LensSettings lens;

    Check(RunHeadless(&lens) == 1, "TestRejectsNonExplainoBaseStateBeforeOutputWork_ReturnsFailure");
    Check(g_stub.state_load_calls == 1, "TestRejectsNonExplainoBaseStateBeforeOutputWork_LoadsStateOnce");
    Check(g_stub.update_polynomial_calls == 0, "TestRejectsNonExplainoBaseStateBeforeOutputWork_NoPolynomialUpdate");
    Check(g_stub.render_calls == 0, "TestRejectsNonExplainoBaseStateBeforeOutputWork_NoRender");
    Check(!lens.enabled, "TestRejectsNonExplainoBaseStateBeforeOutputWork_DoesNotEnableLens");
    Check(!std::filesystem::exists(TestOutputRoot("non_explaino_base_state")), "TestRejectsNonExplainoBaseStateBeforeOutputWork_NoOutputDir");
}

static void TestDefaultsRenderAndEnablesLensBeforeReferenceRender() {
    ResetStubs("render_default_failure");
    g_stub.loaded_render.resolution = {0, 0};
    g_stub.loaded_render.block_size = 0;
    g_stub.loaded_render.device_id = -4;
    g_stub.loaded_render.benchmark = true;
    LensSettings lens;

    Check(RunHeadless(&lens) == 1, "TestDefaultsRenderAndEnablesLensBeforeReferenceRender_ReturnsRenderFailure");
    Check(g_stub.update_polynomial_calls == 1, "TestDefaultsRenderAndEnablesLensBeforeReferenceRender_UpdatesPolynomial");
    Check(g_stub.render_calls == 1, "TestDefaultsRenderAndEnablesLensBeforeReferenceRender_RendersReferenceOnce");
    Check(g_stub.last_render_settings.resolution.x == 1024, "TestDefaultsRenderAndEnablesLensBeforeReferenceRender_DefaultWidth");
    Check(g_stub.last_render_settings.resolution.y == 768, "TestDefaultsRenderAndEnablesLensBeforeReferenceRender_DefaultHeight");
    Check(g_stub.last_render_settings.block_size == 256, "TestDefaultsRenderAndEnablesLensBeforeReferenceRender_DefaultBlockSize");
    Check(g_stub.last_render_settings.device_id == 0, "TestDefaultsRenderAndEnablesLensBeforeReferenceRender_DefaultDevice");
    Check(!g_stub.last_render_settings.benchmark, "TestDefaultsRenderAndEnablesLensBeforeReferenceRender_DisablesBenchmark");
    Check(g_stub.last_render_view.fractal_type == FractalType::explaino, "TestDefaultsRenderAndEnablesLensBeforeReferenceRender_RendersExplaino");
    Check(lens.enabled, "TestDefaultsRenderAndEnablesLensBeforeReferenceRender_EnablesLens");
    Check(g_stub.capture_calls == 0, "TestDefaultsRenderAndEnablesLensBeforeReferenceRender_NoCaptureAfterRenderFailure");
    std::filesystem::remove_all(TestOutputRoot("render_default_failure"));
}

int main() {
    TestRejectsRequestLoadFailureBeforeBundleLoad();
    TestRejectsBundleLoadFailureBeforeBaseState();
    TestRejectsBaseStateLoadFailureBeforeRender();
    TestRejectsNonExplainoBaseStateBeforeOutputWork();
    TestDefaultsRenderAndEnablesLensBeforeReferenceRender();

    std::filesystem::remove_all(std::filesystem::temp_directory_path() / "cuda_newton_fractal_clone_runtime_walk_headless_tests");
    std::printf("test_runtime_walk_headless: passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
