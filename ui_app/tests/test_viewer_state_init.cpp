#include "../src/viewer_state_init.h"
#include "../src/viewer_cli.h"
#include "../src/fractal_types.h"
#include "../src/fractal_family_rules.h"

#include <cstdint>

// Stub: keep viewer-state-init tests lightweight while covering the
// load-state baseline outputs introduced for persisted sidecar orientation.
#include "../src/finding_state_actions.h"
static bool gLoadStateShouldSucceed = true;
static bool gLoadStateHasOrientation = false;
static SidecarOrientationVector gLoadedOrientationStub{};
static bool gLoadStateHasControllerPolicy = false;
static SidecarAutoDemoControllerPolicy gLoadedControllerPolicyStub{};
static bool gLoadStateHasMutationHistory = false;
static SidecarAutoDemoMutationHistory gLoadedMutationHistoryStub{};

bool LoadFindingSelectionIntoRuntime(const std::string&, ViewState*, KernelParams*,
    RenderSettings*, SidecarOrientationVector* outOrientation, bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy, bool* outHasControllerPolicy,
    SidecarAutoDemoMutationHistory* outMutationHistory, bool* outHasMutationHistory,
    std::string*, std::string*) {
    if (!gLoadStateShouldSucceed) return false;
    if (outOrientation) *outOrientation = gLoadedOrientationStub;
    if (outHasOrientation) *outHasOrientation = gLoadStateHasOrientation;
    if (outControllerPolicy) *outControllerPolicy = gLoadedControllerPolicyStub;
    if (outHasControllerPolicy) *outHasControllerPolicy = gLoadStateHasControllerPolicy;
    if (outMutationHistory) *outMutationHistory = gLoadedMutationHistoryStub;
    if (outHasMutationHistory) *outHasMutationHistory = gLoadStateHasMutationHistory;
    return true;
}

bool LoadFindingSelectionIntoRuntime(const std::string&, ViewState*, KernelParams*,
    RenderSettings*, std::string*, std::string*) { return gLoadStateShouldSucceed; }

#include <cmath>
#include <cstdio>
#include <cstdlib>

static int gPass = 0, gFail = 0;
#define CHECK(name, cond) do { if (cond) { ++gPass; printf("  PASS: %s\n", name); } \
    else { ++gFail; printf("  FAIL: %s  (%s:%d)\n", name, __FILE__, __LINE__); } } while(0)

static bool NearlyEqual(double a, double b, double eps = 1.0e-9) {
    return std::fabs(a - b) <= eps;
}

static bool ControllerPoliciesMatch(const SidecarAutoDemoControllerPolicy& lhs,
    const SidecarAutoDemoControllerPolicy& rhs,
    double eps = 1.0e-9) {
    return lhs.enabled == rhs.enabled &&
        lhs.allow_runtime_mutation == rhs.allow_runtime_mutation &&
        lhs.run_paced_loop == rhs.run_paced_loop &&
        NearlyEqual(lhs.paced_loop_interval_seconds, rhs.paced_loop_interval_seconds, eps) &&
        NearlyEqual(lhs.stop_demonstrated_fraction, rhs.stop_demonstrated_fraction, eps) &&
        lhs.stop_uncertain_count == rhs.stop_uncertain_count;
}

static bool MutationRecordsMatch(const SidecarAutoDemoMutationRecord& lhs,
    const SidecarAutoDemoMutationRecord& rhs,
    double eps = 1.0e-9) {
    return lhs.label == rhs.label &&
        lhs.path == rhs.path &&
        lhs.type == rhs.type &&
        NearlyEqual(lhs.target_value, rhs.target_value, eps) &&
        NearlyEqual(lhs.utility, rhs.utility, eps);
}

static bool MutationHistoriesMatch(const SidecarAutoDemoMutationHistory& lhs,
    const SidecarAutoDemoMutationHistory& rhs,
    double eps = 1.0e-9) {
    if (lhs.size() != rhs.size()) return false;
    for (size_t index = 0; index < lhs.size(); ++index) {
        if (!MutationRecordsMatch(lhs[index], rhs[index], eps)) return false;
    }
    return true;
}

// --- Defaults (no CLI overrides) ---

static void TestDefaultsNoChange() {
    ViewerCliArgs cli{};
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    render.resolution = {800, 600};
    bool dirty = false;
    int rc = ApplyCliOverrides(cli, view, params, render, &dirty);
    CHECK("DefaultsNoChange_ReturnCode", rc == 0);
    CHECK("DefaultsNoChange_FractalType", view.fractal_type == FractalType::explaino);
    CHECK("DefaultsNoChange_Width", render.resolution.x == 800);
    CHECK("DefaultsNoChange_Height", render.resolution.y == 600);
}

// --- Fractal type override ---

static void TestFractalTypeOverride() {
    ViewerCliArgs cli{};
    cli.have_fractal_type = true;
    cli.fractal_type = FractalType::mandelbrot;
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    render.resolution = {800, 600};
    bool dirty = false;
    int rc = ApplyCliOverrides(cli, view, params, render, &dirty);
    CHECK("FractalTypeOverride_ReturnCode", rc == 0);
    CHECK("FractalTypeOverride_Type", view.fractal_type == FractalType::mandelbrot);
    CHECK("FractalTypeOverride_Dirty", dirty == true);
}

// --- Explaino seed implies explaino fractal type ---

static void TestExplainoSeedImpliesExplaino() {
    ViewerCliArgs cli{};
    cli.have_explaino_seed = true;
    cli.explaino_seed = 5.0;
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    render.resolution = {800, 600};
    bool dirty = false;
    int rc = ApplyCliOverrides(cli, view, params, render, &dirty);
    CHECK("ExplainoSeedImplies_ReturnCode", rc == 0);
    CHECK("ExplainoSeedImplies_Type", view.fractal_type == FractalType::explaino);
    CHECK("ExplainoSeedImplies_Dirty", dirty == true);
}

// --- Sweep implies explaino ---

static void TestSweepImpliesExplaino() {
    ViewerCliArgs cli{};
    cli.sweep_config.enabled = true;
    cli.sweep_config.seed_start = 1.0;
    cli.sweep_config.seed_stop = 10.0;
    cli.sweep_config.seed_step = 1.0;
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    render.resolution = {800, 600};
    bool dirty = false;
    int rc = ApplyCliOverrides(cli, view, params, render, &dirty);
    CHECK("SweepImpliesExplaino_ReturnCode", rc == 0);
    CHECK("SweepImpliesExplaino_Type", IsExplainoFamily(view.fractal_type));
}

// --- Sweep with non-explaino type fails ---

static void TestSweepNonExplainoFails() {
    ViewerCliArgs cli{};
    cli.have_fractal_type = true;
    cli.fractal_type = FractalType::mandelbrot;
    cli.sweep_config.enabled = true;
    cli.sweep_config.seed_start = 1.0;
    cli.sweep_config.seed_stop = 10.0;
    cli.sweep_config.seed_step = 1.0;
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    render.resolution = {800, 600};
    bool dirty = false;
    int rc = ApplyCliOverrides(cli, view, params, render, &dirty);
    CHECK("SweepNonExplaino_Fails", rc != 0);
}

// --- Resolution overrides ---

static void TestResolutionOverrides() {
    ViewerCliArgs cli{};
    cli.have_width = true;
    cli.width = 1920;
    cli.have_height = true;
    cli.height = 1080;
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    render.resolution = {800, 600};
    bool dirty = false;
    int rc = ApplyCliOverrides(cli, view, params, render, &dirty);
    CHECK("ResolutionOverrides_ReturnCode", rc == 0);
    CHECK("ResolutionOverrides_Width", render.resolution.x == 1920);
    CHECK("ResolutionOverrides_Height", render.resolution.y == 1080);
}

// --- Explaino phase override ---

static void TestExplainoPhaseOverride() {
    ViewerCliArgs cli{};
    cli.have_fractal_type = true;
    cli.fractal_type = FractalType::explaino;
    cli.have_explaino_phase = true;
    cli.explaino_phase = 2.5;
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    render.resolution = {800, 600};
    bool dirty = false;
    int rc = ApplyCliOverrides(cli, view, params, render, &dirty);
    CHECK("ExplainoPhase_ReturnCode", rc == 0);
    CHECK("ExplainoPhase_Value", std::fabs(view.explaino_phase - 2.5f) < 0.001f);
}

// --- Explaino seed drift override ---

static void TestExplainoSeedDriftOverride() {
    ViewerCliArgs cli{};
    cli.have_fractal_type = true;
    cli.fractal_type = FractalType::explaino;
    cli.have_explaino_seed_drift = true;
    cli.explaino_seed_drift = 0.01;
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    render.resolution = {800, 600};
    bool dirty = false;
    int rc = ApplyCliOverrides(cli, view, params, render, &dirty);
    CHECK("ExplainoSeedDrift_ReturnCode", rc == 0);
    CHECK("ExplainoSeedDrift_Value", std::fabs(view.explaino_seed_drift - 0.01f) < 0.001f);
}

// --- Lambda overrides ---

static void TestLambdaOverrides() {
    ViewerCliArgs cli{};
    cli.have_fractal_type = true;
    cli.fractal_type = FractalType::lambda_map;
    cli.have_lambda_real = true;
    cli.lambda_real = 1.5;
    cli.have_lambda_imag = true;
    cli.lambda_imag = -0.3;
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    render.resolution = {800, 600};
    bool dirty = false;
    int rc = ApplyCliOverrides(cli, view, params, render, &dirty);
    CHECK("Lambda_ReturnCode", rc == 0);
    CHECK("Lambda_Real", std::fabs(params.lambda_real - 1.5f) < 0.001f);
    CHECK("Lambda_Imag", std::fabs(params.lambda_imag - (-0.3f)) < 0.001f);
}

// --- Explaino seed_b and mix overrides ---

static void TestExplainoSeedBAndMix() {
    ViewerCliArgs cli{};
    cli.have_fractal_type = true;
    cli.fractal_type = FractalType::explaino;
    cli.have_explaino_seed_b = true;
    cli.explaino_seed_b = 3.14;
    cli.have_explaino_mix = true;
    cli.explaino_mix = 0.75;
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    render.resolution = {800, 600};
    bool dirty = false;
    int rc = ApplyCliOverrides(cli, view, params, render, &dirty);
    CHECK("SeedBAndMix_ReturnCode", rc == 0);
    CHECK("SeedBAndMix_SeedB", std::fabs(params.explaino_seed_b - 3.14) < 0.001);
    CHECK("SeedBAndMix_Mix", std::fabs(params.explaino_mix - 0.75f) < 0.001f);
}

// --- Warp strength override ---

static void TestWarpStrengthOverride() {
    ViewerCliArgs cli{};
    cli.have_fractal_type = true;
    cli.fractal_type = FractalType::explaino;
    cli.have_explaino_warp_strength = true;
    cli.explaino_warp_strength = 2.0;
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    render.resolution = {800, 600};
    bool dirty = false;
    int rc = ApplyCliOverrides(cli, view, params, render, &dirty);
    CHECK("WarpStrength_ReturnCode", rc == 0);
    CHECK("WarpStrength_Value", std::fabs(params.explaino_warp_strength - 2.0f) < 0.001f);
}

// --- Explicit fractal type takes priority over explaino seed ---

static void TestFractalTypeTakesPriority() {
    ViewerCliArgs cli{};
    cli.have_fractal_type = true;
    cli.fractal_type = FractalType::explaino_halley;
    cli.have_explaino_seed = true;
    cli.explaino_seed = 5.0;
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    render.resolution = {800, 600};
    bool dirty = false;
    int rc = ApplyCliOverrides(cli, view, params, render, &dirty);
    CHECK("FractalTypePriority_ReturnCode", rc == 0);
    CHECK("FractalTypePriority_Type", view.fractal_type == FractalType::explaino_halley);
}

// --- Null dirty pointer is safe ---

static void TestNullDirtyPointer() {
    ViewerCliArgs cli{};
    cli.have_fractal_type = true;
    cli.fractal_type = FractalType::burning_ship;
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    render.resolution = {800, 600};
    int rc = ApplyCliOverrides(cli, view, params, render, nullptr);
    CHECK("NullDirtyPtr_ReturnCode", rc == 0);
    CHECK("NullDirtyPtr_Type", view.fractal_type == FractalType::burning_ship);
}

static void TestLoadStateReturnsPersistedOrientationBaseline() {
    ViewerCliArgs cli{};
    cli.have_load_state_json = true;
    cli.load_state_json = "state.json";

    gLoadStateShouldSucceed = true;
    gLoadStateHasOrientation = true;
    gLoadedOrientationStub = {};
    gLoadedOrientationStub.import_signature = 101u;
    gLoadedOrientationStub.pack_projection_hash = 202u;
    gLoadedOrientationStub.field_embedding_stats = 3.5;
    gLoadedOrientationStub.slime_energy_delta = 1.25;
    gLoadedOrientationStub.busy_beaver_metrics = 0.75;
    gLoadedOrientationStub.decode_stability = 0.5;
    gLoadedOrientationStub.diff_magnitude = 2.0;
    gLoadStateHasControllerPolicy = true;
    gLoadedControllerPolicyStub = {};
    gLoadedControllerPolicyStub.enabled = true;
    gLoadedControllerPolicyStub.allow_runtime_mutation = true;
    gLoadedControllerPolicyStub.run_paced_loop = true;
    gLoadedControllerPolicyStub.paced_loop_interval_seconds = 2.5;
    gLoadedControllerPolicyStub.stop_demonstrated_fraction = 0.75;
    gLoadedControllerPolicyStub.stop_uncertain_count = 3;
    gLoadStateHasMutationHistory = true;
    gLoadedMutationHistoryStub = {
        {"Ripple amplitude", "fractal.params.ripple_amplitude", "float", 0.15, 1.25},
        {"Seed", "fractal.params.explaino_seed", "double", 3.5, 0.75},
    };

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    SidecarAutoDemoControllerPolicy controllerPolicy{};
    render.resolution = {800, 600};
    bool dirty = false;
    SidecarOrientationVector loadedOrientation{};
    bool hasLoadedOrientation = false;
    SidecarAutoDemoMutationHistory loadedMutationHistory;
    bool hasLoadedMutationHistory = false;

    int rc = ApplyCliOverrides(
        cli,
        view,
        params,
        render,
        &controllerPolicy,
        &loadedOrientation,
        &hasLoadedOrientation,
        &loadedMutationHistory,
        &hasLoadedMutationHistory,
        nullptr,
        &dirty);
    CHECK("LoadStateBaseline_ReturnCode", rc == 0);
    CHECK("LoadStateBaseline_HasOrientation", hasLoadedOrientation == true);
    CHECK("LoadStateBaseline_ImportSignature", loadedOrientation.import_signature == 101u);
    CHECK("LoadStateBaseline_PackProjectionHash", loadedOrientation.pack_projection_hash == 202u);
    CHECK("LoadStateBaseline_ControllerPolicy", ControllerPoliciesMatch(controllerPolicy, gLoadedControllerPolicyStub));
    CHECK("LoadStateBaseline_HasMutationHistory", hasLoadedMutationHistory == true);
    CHECK("LoadStateBaseline_MutationHistory", MutationHistoriesMatch(loadedMutationHistory, gLoadedMutationHistoryStub));
    CHECK("LoadStateBaseline_Dirty", dirty == true);
}

static void TestFractalOverrideClearsLoadedOrientationBaseline() {
    ViewerCliArgs cli{};
    cli.have_load_state_json = true;
    cli.load_state_json = "state.json";
    cli.have_fractal_type = true;
    cli.fractal_type = FractalType::mandelbrot;

    gLoadStateShouldSucceed = true;
    gLoadStateHasOrientation = true;
    gLoadedOrientationStub = {};
    gLoadedOrientationStub.import_signature = 505u;
    gLoadStateHasControllerPolicy = true;
    gLoadedControllerPolicyStub = {};
    gLoadedControllerPolicyStub.enabled = true;
    gLoadedControllerPolicyStub.stop_uncertain_count = 7;
    gLoadStateHasMutationHistory = true;
    gLoadedMutationHistoryStub = {
        {"Ripple amplitude", "fractal.params.ripple_amplitude", "float", 0.15, 1.25},
    };

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    SidecarAutoDemoControllerPolicy controllerPolicy{};
    render.resolution = {800, 600};
    bool dirty = false;
    SidecarOrientationVector loadedOrientation{};
    bool hasLoadedOrientation = false;
    SidecarAutoDemoMutationHistory loadedMutationHistory;
    bool hasLoadedMutationHistory = false;

    int rc = ApplyCliOverrides(
        cli,
        view,
        params,
        render,
        &controllerPolicy,
        &loadedOrientation,
        &hasLoadedOrientation,
        &loadedMutationHistory,
        &hasLoadedMutationHistory,
        nullptr,
        &dirty);
    CHECK("LoadStateOverride_ReturnCode", rc == 0);
    CHECK("LoadStateOverride_Type", view.fractal_type == FractalType::mandelbrot);
    CHECK("LoadStateOverride_ClearsBaseline", hasLoadedOrientation == false);
    CHECK("LoadStateOverride_ClearsMutationHistory", hasLoadedMutationHistory == false);
    CHECK("LoadStateOverride_PreservesControllerPolicy", ControllerPoliciesMatch(controllerPolicy, gLoadedControllerPolicyStub));
}

static void TestLegacyLoadStateResetsControllerPolicy() {
    ViewerCliArgs cli{};
    cli.have_load_state_json = true;
    cli.load_state_json = "state.json";

    gLoadStateShouldSucceed = true;
    gLoadStateHasOrientation = false;
    gLoadedOrientationStub = {};
    gLoadStateHasControllerPolicy = false;
    gLoadedControllerPolicyStub = {};
    gLoadStateHasMutationHistory = false;
    gLoadedMutationHistoryStub = {};

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    render.resolution = {800, 600};
    SidecarAutoDemoControllerPolicy controllerPolicy{};
    controllerPolicy.enabled = true;
    controllerPolicy.allow_runtime_mutation = true;
    controllerPolicy.run_paced_loop = true;
    controllerPolicy.paced_loop_interval_seconds = 5.0;
    controllerPolicy.stop_demonstrated_fraction = 0.5;
    controllerPolicy.stop_uncertain_count = 4;
    bool dirty = false;
    SidecarOrientationVector loadedOrientation{};
    bool hasLoadedOrientation = false;
    SidecarAutoDemoMutationHistory loadedMutationHistory;
    bool hasLoadedMutationHistory = false;

    int rc = ApplyCliOverrides(
        cli,
        view,
        params,
        render,
        &controllerPolicy,
        &loadedOrientation,
        &hasLoadedOrientation,
        &loadedMutationHistory,
        &hasLoadedMutationHistory,
        nullptr,
        &dirty);
    CHECK("LegacyLoadStatePolicy_ReturnCode", rc == 0);
    CHECK("LegacyLoadStatePolicy_ClearsOrientationBaseline", hasLoadedOrientation == false);
    CHECK("LegacyLoadStatePolicy_ClearsMutationHistory", hasLoadedMutationHistory == false);
    CHECK("LegacyLoadStatePolicy_ResetsControllerPolicy", ControllerPoliciesMatch(controllerPolicy, SidecarAutoDemoControllerPolicy{}));
}

int main() {
    TestDefaultsNoChange();
    TestFractalTypeOverride();
    TestExplainoSeedImpliesExplaino();
    TestSweepImpliesExplaino();
    TestSweepNonExplainoFails();
    TestResolutionOverrides();
    TestExplainoPhaseOverride();
    TestExplainoSeedDriftOverride();
    TestLambdaOverrides();
    TestExplainoSeedBAndMix();
    TestWarpStrengthOverride();
    TestFractalTypeTakesPriority();
    TestNullDirtyPointer();
    TestLoadStateReturnsPersistedOrientationBaseline();
    TestFractalOverrideClearsLoadedOrientationBaseline();
    TestLegacyLoadStateResetsControllerPolicy();
    printf("test_viewer_state_init: %d passed, %d failed\n", gPass, gFail);
    return gFail > 0 ? 1 : 0;
}
