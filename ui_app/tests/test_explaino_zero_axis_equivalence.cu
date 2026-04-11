#include "../src/explaino_seed.h"
#include "../src/fractal_derived_fields.h"
#include "../src/fractal_sample_result.h"
#include "../src/fractal_types.h"
#include "../src/view_hp_sync.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>

namespace {

constexpr int kWidth = 256;
constexpr int kHeight = 256;
constexpr int kNumPoints = kWidth * kHeight;
constexpr float kResidualTolerance = 1.0e-4f;
constexpr float kFinalZTolerance = 1.0e-3f;
constexpr float kRootTolerance = 1.0e-5f;

struct ZeroAxisState {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
};

int gPass = 0;
int gFail = 0;

#define CHECK(name, cond) do { \
    if (cond) { \
        ++gPass; \
    } else { \
        ++gFail; \
        std::printf("  FAIL: %s (%s:%d)\n", (name), __FILE__, __LINE__); \
    } \
} while(0)

double Distance(double ax, double ay, double bx, double by) {
    const double dx = ax - bx;
    const double dy = ay - by;
    return std::sqrt(dx * dx + dy * dy);
}

void BuildZeroAxisState(FractalType fractalType, ZeroAxisState* outState) {
    outState->view = {};
    outState->params = {};
    outState->render = {};

    outState->view.fractal_type = fractalType;
    outState->view.center = {0.0f, 0.0f};
    outState->view.zoom = 1.0f;
    outState->view.rotation_degrees = 0.0f;
    outState->view.explaino_phase = 0.0f;
    outState->view.explaino_phase_strength = 1.0f;
    outState->view.explaino_seed_drift = 0.0f;

    outState->params.max_iter = 128;
    outState->params.epsilon = 1.0e-6f;
    outState->params.coloring_mode = ColoringMode::root_basin;
    outState->params.explaino_seed = 2.25;
    outState->params.explaino_warp_strength = 0.1f;
    outState->params.explaino_root_spread = 0.5f;
    outState->params.explaino_damping = 1.0f;
    outState->params.explaino_cluster_radius = 0.0f;
    outState->params.ripple_amplitude = 0.0f;
    outState->params.splice_offset = 0.0f;
    outState->params.vortex_strength = 0.0f;
    outState->params.tension_strength = 0.0f;

    outState->render.resolution = {kWidth, kHeight};
    outState->render.block_size = 256;
    outState->render.device_id = 0;
    outState->render.sample_tier = SampleTier::fast;

    SetPolyPreset(outState->params);
    ExplainoSeedNormalize(outState->view, outState->params);
    UpdateExplainoPolynomial(outState->view, outState->params, nullptr);
    SyncViewHpFromUi(outState->view);
}

void BuildGridCoordinates(const ViewState& view, std::vector<Double2>* outCoords) {
    outCoords->resize(kNumPoints);

    const double aspect = static_cast<double>(kWidth) / static_cast<double>(kHeight);
    const double zoom = std::fmax(1.0e-300, std::exp2(view.log2_zoom));
    const double base = 2.0 / zoom;

    for (int py = 0; py < kHeight; ++py) {
        for (int px = 0; px < kWidth; ++px) {
            const double nx = (((static_cast<double>(px) + 0.5) / static_cast<double>(kWidth)) - 0.5) * 2.0;
            const double ny = (((static_cast<double>(py) + 0.5) / static_cast<double>(kHeight)) - 0.5) * 2.0;
            const double x = view.center_hp_x + nx * base * aspect;
            const double y = view.center_hp_y + ny * base;
            (*outCoords)[static_cast<size_t>(py) * static_cast<size_t>(kWidth) + static_cast<size_t>(px)] = {x, y};
        }
    }
}

bool SampleGrid(const ZeroAxisState& state,
    const std::vector<Double2>& coords,
    std::vector<FractalSampleResult>* outResults) {
    outResults->assign(coords.size(), {});
    const char* error = nullptr;
    if (!SampleFractalPoints(coords.data(), static_cast<int>(coords.size()), state.view, state.params, state.render, outResults->data(), &error)) {
        std::printf("  sample failed for fractal_type=%d: %s\n", static_cast<int>(state.view.fractal_type), error ? error : "unknown");
        return false;
    }
    return true;
}

int NearestRootIndex(const KernelParams& params, const FractalSampleResult& result) {
    if (params.explaino_root_count <= 0) return -1;
    int bestIndex = 0;
    double bestDist = Distance(result.final_z_x, result.final_z_y, params.explaino_roots[0].x, params.explaino_roots[0].y);
    for (int index = 1; index < params.explaino_root_count; ++index) {
        const double dist = Distance(result.final_z_x, result.final_z_y, params.explaino_roots[index].x, params.explaino_roots[index].y);
        if (dist < bestDist) {
            bestDist = dist;
            bestIndex = index;
        }
    }
    return bestIndex;
}

void CheckRootSurface(const ZeroAxisState& baseline,
    const ZeroAxisState& variant,
    const char* name) {
    CHECK("root_count matches", baseline.params.explaino_root_count == variant.params.explaino_root_count);
    const int rootCount = baseline.params.explaino_root_count;
    int mismatches = 0;
    double maxRootDelta = 0.0;
    for (int index = 0; index < rootCount; ++index) {
        const double delta = Distance(
            baseline.params.explaino_roots[index].x,
            baseline.params.explaino_roots[index].y,
            variant.params.explaino_roots[index].x,
            variant.params.explaino_roots[index].y);
        if (delta > kRootTolerance) {
            ++mismatches;
        }
        if (delta > maxRootDelta) maxRootDelta = delta;
    }
    std::printf("    %-18s root_max_delta=%.3e\n", name, maxRootDelta);
    CHECK("zero-axis roots match baseline", mismatches == 0);
}

void CheckVariantAgainstBaseline(FractalType variantType, const char* variantName) {
    ZeroAxisState baseline;
    ZeroAxisState variant;
    BuildZeroAxisState(FractalType::explaino, &baseline);
    BuildZeroAxisState(variantType, &variant);

    CheckRootSurface(baseline, variant, variantName);

    std::vector<Double2> coords;
    BuildGridCoordinates(baseline.view, &coords);

    std::vector<FractalSampleResult> baselineResults;
    std::vector<FractalSampleResult> variantResults;
    CHECK("baseline sample ok", SampleGrid(baseline, coords, &baselineResults));
    CHECK("variant sample ok", SampleGrid(variant, coords, &variantResults));
    if (baselineResults.size() != variantResults.size() || baselineResults.empty()) {
        CHECK("sample result counts match", false);
        return;
    }

    int flagMismatches = 0;
    int basinMismatches = 0;
    int iterationMismatches = 0;
    int residualMismatches = 0;
    int finalZMismatches = 0;
    int convergedComparisons = 0;
    double maxResidualDelta = 0.0;
    double maxFinalZDelta = 0.0;

    for (size_t index = 0; index < baselineResults.size(); ++index) {
        const FractalSampleResult& baselineResult = baselineResults[index];
        const FractalSampleResult& variantResult = variantResults[index];

        if (baselineResult.converged != variantResult.converged || baselineResult.escaped != variantResult.escaped) {
            ++flagMismatches;
        }

        if (std::abs(baselineResult.iterations - variantResult.iterations) > 1) {
            ++iterationMismatches;
        }

        if (baselineResult.converged && variantResult.converged) {
            ++convergedComparisons;
            if (NearestRootIndex(baseline.params, baselineResult) != NearestRootIndex(baseline.params, variantResult)) {
                ++basinMismatches;
            }

            const double residualDelta = std::fabs(static_cast<double>(baselineResult.residual) - static_cast<double>(variantResult.residual));
            const double finalZDelta = Distance(
                baselineResult.final_z_x,
                baselineResult.final_z_y,
                variantResult.final_z_x,
                variantResult.final_z_y);
            if (residualDelta > kResidualTolerance) {
                ++residualMismatches;
            }
            if (finalZDelta > kFinalZTolerance) {
                ++finalZMismatches;
            }
            if (residualDelta > maxResidualDelta) maxResidualDelta = residualDelta;
            if (finalZDelta > maxFinalZDelta) maxFinalZDelta = finalZDelta;
        }
    }

    std::printf("    %-18s conv=%5d flag=%4d basin=%4d iter=%4d residual_max=%.3e final_z_max=%.3e\n",
        variantName,
        convergedComparisons,
        flagMismatches,
        basinMismatches,
        iterationMismatches,
        maxResidualDelta,
        maxFinalZDelta);

    CHECK("converged comparisons available", convergedComparisons > 0);
    CHECK("zero-axis flags match baseline", flagMismatches == 0);
    CHECK("zero-axis basin identity matches baseline", basinMismatches == 0);
    CHECK("zero-axis iterations stay within tolerance", iterationMismatches == 0);
    CHECK("zero-axis residuals stay within tolerance", residualMismatches == 0);
    CHECK("zero-axis final_z stays within tolerance", finalZMismatches == 0);
}

} // namespace

int main() {
    std::printf("=== Explaino zero-axis equivalence ===\n");
    CheckVariantAgainstBaseline(FractalType::explaino_ripple, "explaino_ripple");
    CheckVariantAgainstBaseline(FractalType::explaino_splice, "explaino_splice");
    CheckVariantAgainstBaseline(FractalType::explaino_vortex, "explaino_vortex");
    CheckVariantAgainstBaseline(FractalType::explaino_tension, "explaino_tension");
    std::printf("test_explaino_zero_axis_equivalence: %d passed, %d failed\n", gPass, gFail);
    return gFail == 0 ? 0 : 1;
}