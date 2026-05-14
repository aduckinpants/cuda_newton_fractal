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

float DefaultVariantStrength(FractalType fractalType) {
    switch (fractalType) {
    case FractalType::explaino_ripple:
        return 0.15f;
    case FractalType::explaino_splice:
        return 0.5f;
    case FractalType::explaino_vortex:
        return 0.3f;
    case FractalType::explaino_tension:
        return 0.02f;
    default:
        return 0.0f;
    }
}

void BuildExplainoVariantState(FractalType fractalType,
    float rippleAmplitude,
    float spliceOffset,
    float vortexStrength,
    float tensionStrength,
    ZeroAxisState* outState,
    float balanceVoid = 0.0f,
    float symmetryTension = 0.0f,
    float fieldCurvature = 0.0f) {
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
    outState->params.ripple_amplitude = rippleAmplitude;
    outState->params.splice_offset = spliceOffset;
    outState->params.vortex_strength = vortexStrength;
    outState->params.tension_strength = tensionStrength;
    outState->params.balance_void = balanceVoid;
    outState->params.symmetry_tension = symmetryTension;
    outState->params.field_curvature = fieldCurvature;

    outState->render.resolution = {kWidth, kHeight};
    outState->render.block_size = 256;
    outState->render.device_id = 0;
    outState->render.sample_tier = SampleTier::fast;

    SetPolyPreset(outState->params);
    ExplainoSeedNormalize(outState->view, outState->params);
    UpdateExplainoPolynomial(outState->view, outState->params, nullptr);
    SyncViewHpFromUi(outState->view);
}

void BuildZeroAxisState(FractalType fractalType, ZeroAxisState* outState) {
    BuildExplainoVariantState(fractalType, 0.0f, 0.0f, 0.0f, 0.0f, outState);
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

void CheckStateEquivalence(const ZeroAxisState& expected,
    const ZeroAxisState& actual,
    const char* name) {
    CheckRootSurface(expected, actual, name);

    std::vector<Double2> coords;
    BuildGridCoordinates(expected.view, &coords);

    std::vector<FractalSampleResult> expectedResults;
    std::vector<FractalSampleResult> actualResults;
    CHECK("expected sample ok", SampleGrid(expected, coords, &expectedResults));
    CHECK("actual sample ok", SampleGrid(actual, coords, &actualResults));
    if (expectedResults.size() != actualResults.size() || expectedResults.empty()) {
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

    for (size_t index = 0; index < expectedResults.size(); ++index) {
        const FractalSampleResult& expectedResult = expectedResults[index];
        const FractalSampleResult& actualResult = actualResults[index];

        if (expectedResult.converged != actualResult.converged || expectedResult.escaped != actualResult.escaped) {
            ++flagMismatches;
        }

        if (std::abs(expectedResult.iterations - actualResult.iterations) > 1) {
            ++iterationMismatches;
        }

        if (expectedResult.converged && actualResult.converged) {
            ++convergedComparisons;
            if (NearestRootIndex(expected.params, expectedResult) != NearestRootIndex(expected.params, actualResult)) {
                ++basinMismatches;
            }

            const double residualDelta = std::fabs(static_cast<double>(expectedResult.residual) - static_cast<double>(actualResult.residual));
            const double finalZDelta = Distance(
                expectedResult.final_z_x,
                expectedResult.final_z_y,
                actualResult.final_z_x,
                actualResult.final_z_y);
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

    std::printf("    %-30s conv=%5d flag=%4d basin=%4d iter=%4d residual_max=%.3e final_z_max=%.3e\n",
        name,
        convergedComparisons,
        flagMismatches,
        basinMismatches,
        iterationMismatches,
        maxResidualDelta,
        maxFinalZDelta);

    CHECK("converged comparisons available", convergedComparisons > 0);
    CHECK("state flags match", flagMismatches == 0);
    CHECK("state basin identity matches", basinMismatches == 0);
    CHECK("state iterations stay within tolerance", iterationMismatches == 0);
    CHECK("state residuals stay within tolerance", residualMismatches == 0);
    CHECK("state final_z stays within tolerance", finalZMismatches == 0);
}

void CheckSecondaryVariantReduction(FractalType primaryType,
    const char* primaryName,
    FractalType secondaryType,
    const char* secondaryName) {
    ZeroAxisState expected;
    ZeroAxisState actual;

    float rippleAmplitude = 0.0f;
    float spliceOffset = 0.0f;
    float vortexStrength = 0.0f;
    float tensionStrength = 0.0f;
    const float secondaryStrength = DefaultVariantStrength(secondaryType);

    switch (secondaryType) {
    case FractalType::explaino_ripple:
        rippleAmplitude = secondaryStrength;
        break;
    case FractalType::explaino_splice:
        spliceOffset = secondaryStrength;
        break;
    case FractalType::explaino_vortex:
        vortexStrength = secondaryStrength;
        break;
    case FractalType::explaino_tension:
        tensionStrength = secondaryStrength;
        break;
    default:
        CHECK("secondary type must be a composed variant", false);
        return;
    }

    BuildExplainoVariantState(secondaryType,
        rippleAmplitude,
        spliceOffset,
        vortexStrength,
        tensionStrength,
        &expected);
    BuildExplainoVariantState(primaryType,
        rippleAmplitude,
        spliceOffset,
        vortexStrength,
        tensionStrength,
        &actual);

    char label[96];
    std::snprintf(label, sizeof(label), "%s secondary=%s", primaryName, secondaryName);
    CheckStateEquivalence(expected, actual, label);
}

void CheckPlainExplainoIgnoresLatentComposition() {
    ZeroAxisState expected;
    ZeroAxisState actual;
    BuildZeroAxisState(FractalType::explaino, &expected);
    BuildExplainoVariantState(FractalType::explaino,
        DefaultVariantStrength(FractalType::explaino_ripple),
        DefaultVariantStrength(FractalType::explaino_splice),
        DefaultVariantStrength(FractalType::explaino_vortex),
        DefaultVariantStrength(FractalType::explaino_tension),
        &actual,
        0.35f,
        -0.2f,
        0.25f);
    CheckStateEquivalence(expected, actual, "plain explaino latent params");
}

void BuildExplainoBalanceVoidState(float balanceVoid,
    float symmetryTension,
    float fieldCurvature,
    ZeroAxisState* outState) {
    BuildExplainoVariantState(FractalType::explaino_balance_void,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        outState,
        balanceVoid,
        symmetryTension,
        fieldCurvature);
}

void CheckBalanceVoidControlPerturbation() {
    ZeroAxisState neutral;
    ZeroAxisState perturbed;
    BuildExplainoBalanceVoidState(0.0f, 0.0f, 0.0f, &neutral);
    BuildExplainoBalanceVoidState(0.35f, -0.2f, 0.25f, &perturbed);

    std::vector<Double2> coords;
    BuildGridCoordinates(neutral.view, &coords);

    std::vector<FractalSampleResult> neutralResults;
    std::vector<FractalSampleResult> perturbedResults;
    CHECK("balance-void neutral sample ok", SampleGrid(neutral, coords, &neutralResults));
    CHECK("balance-void perturbed sample ok", SampleGrid(perturbed, coords, &perturbedResults));
    if (neutralResults.size() != perturbedResults.size() || neutralResults.empty()) {
        CHECK("balance-void sample result counts match", false);
        return;
    }

    int changedFlags = 0;
    int changedIterations = 0;
    int changedBasins = 0;
    int changedFinalZ = 0;
    for (size_t index = 0; index < neutralResults.size(); ++index) {
        const FractalSampleResult& neutralResult = neutralResults[index];
        const FractalSampleResult& perturbedResult = perturbedResults[index];
        if (neutralResult.converged != perturbedResult.converged || neutralResult.escaped != perturbedResult.escaped) {
            ++changedFlags;
        }
        if (neutralResult.iterations != perturbedResult.iterations) {
            ++changedIterations;
        }
        if (neutralResult.converged && perturbedResult.converged) {
            if (NearestRootIndex(neutral.params, neutralResult) != NearestRootIndex(perturbed.params, perturbedResult)) {
                ++changedBasins;
            }
            if (Distance(neutralResult.final_z_x, neutralResult.final_z_y, perturbedResult.final_z_x, perturbedResult.final_z_y) > 1.0e-5) {
                ++changedFinalZ;
            }
        }
    }

    std::printf("    %-18s flag=%4d basin=%4d iter=%4d final_z=%4d\n",
        "balance_void_axes",
        changedFlags,
        changedBasins,
        changedIterations,
        changedFinalZ);

    CHECK("balance-void controls perturb runtime",
        changedFlags > 0 || changedIterations > 0 || changedBasins > 0 || changedFinalZ > 0);
}

void CheckComposedLabelInvariance(FractalType leftType,
    const char* leftName,
    FractalType rightType,
    const char* rightName,
    float rippleAmplitude,
    float spliceOffset,
    float vortexStrength,
    float tensionStrength) {
    ZeroAxisState left;
    ZeroAxisState right;
    BuildExplainoVariantState(leftType,
        rippleAmplitude,
        spliceOffset,
        vortexStrength,
        tensionStrength,
        &left);
    BuildExplainoVariantState(rightType,
        rippleAmplitude,
        spliceOffset,
        vortexStrength,
        tensionStrength,
        &right);

    char label[128];
    std::snprintf(label, sizeof(label), "%s vs %s composed invariance", leftName, rightName);
    CheckStateEquivalence(left, right, label);
}

} // namespace

int main() {
    std::printf("=== Explaino zero-axis equivalence ===\n");
    CheckVariantAgainstBaseline(FractalType::explaino_ripple, "explaino_ripple");
    CheckVariantAgainstBaseline(FractalType::explaino_splice, "explaino_splice");
    CheckVariantAgainstBaseline(FractalType::explaino_vortex, "explaino_vortex");
    CheckVariantAgainstBaseline(FractalType::explaino_tension, "explaino_tension");
    CheckVariantAgainstBaseline(FractalType::explaino_balance_void, "explaino_balance_void");
    std::printf("=== Explaino composed secondary reductions ===\n");
    CheckSecondaryVariantReduction(FractalType::explaino_ripple, "explaino_ripple", FractalType::explaino_vortex, "explaino_vortex");
    CheckSecondaryVariantReduction(FractalType::explaino_vortex, "explaino_vortex", FractalType::explaino_splice, "explaino_splice");
    CheckSecondaryVariantReduction(FractalType::explaino_splice, "explaino_splice", FractalType::explaino_tension, "explaino_tension");
    CheckSecondaryVariantReduction(FractalType::explaino_tension, "explaino_tension", FractalType::explaino_ripple, "explaino_ripple");
    CheckPlainExplainoIgnoresLatentComposition();
    CheckBalanceVoidControlPerturbation();
    CheckComposedLabelInvariance(
        FractalType::explaino_ripple,
        "explaino_ripple",
        FractalType::explaino_vortex,
        "explaino_vortex",
        DefaultVariantStrength(FractalType::explaino_ripple),
        0.0f,
        DefaultVariantStrength(FractalType::explaino_vortex),
        0.0f);
    CheckComposedLabelInvariance(
        FractalType::explaino_splice,
        "explaino_splice",
        FractalType::explaino_tension,
        "explaino_tension",
        0.0f,
        DefaultVariantStrength(FractalType::explaino_splice),
        0.0f,
        DefaultVariantStrength(FractalType::explaino_tension));
    std::printf("test_explaino_zero_axis_equivalence: %d passed, %d failed\n", gPass, gFail);
    return gFail == 0 ? 0 : 1;
}