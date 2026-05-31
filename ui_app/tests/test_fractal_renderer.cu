#include "../src/fractal_types.h"
#include "../src/fractal_family_rules.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <cmath>
#include <vector>

namespace {

int g_passed = 0;
int g_failed = 0;

void Check(bool condition, const char* message) {
    if (condition) {
        ++g_passed;
    } else {
        ++g_failed;
        std::cerr << "FAIL: " << message << "\n";
    }
}

bool ErrorContains(const char* error, const char* needle) {
    return error && std::strstr(error, needle) != nullptr;
}

int CountDistinctPixels(const std::vector<uint32_t>& pixels) {
    std::vector<uint32_t> distinct;
    distinct.reserve(pixels.size());
    for (uint32_t pixel : pixels) {
        bool seen = false;
        for (uint32_t existing : distinct) {
            if (existing == pixel) {
                seen = true;
                break;
            }
        }
        if (!seen) {
            distinct.push_back(pixel);
        }
    }
    return static_cast<int>(distinct.size());
}

int CountNonBlackPixels(const std::vector<uint32_t>& pixels) {
    int count = 0;
    for (uint32_t pixel : pixels) {
        if ((pixel & 0x00ffffffu) != 0u) ++count;
    }
    return count;
}

uint64_t HashFloatValues(const std::vector<float>& values, int offset, int count) {
    uint64_t hash = 1469598103934665603ull;
    for (int index = 0; index < count; ++index) {
        uint32_t bits = 0;
        const float value = values[static_cast<std::size_t>(offset + index)];
        std::memcpy(&bits, &value, sizeof(bits));
        hash ^= static_cast<uint64_t>(bits);
        hash *= 1099511628211ull;
    }
    return hash;
}

bool AllFiniteValues(const std::vector<float>& values, int offset, int count) {
    for (int index = 0; index < count; ++index) {
        if (!std::isfinite(values[static_cast<std::size_t>(offset + index)])) {
            return false;
        }
    }
    return true;
}

void TestNullOutputFailsBeforeCudaWork() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    const char* error = "stale";

    const bool ok = RenderFractalCUDA(view, params, render, nullptr, nullptr, &stats, &error);
    Check(!ok, "RenderFractalCUDA rejects null output buffer");
    Check(ErrorContains(error, "outRGBA is null"), "Null output error names outRGBA");
}

void TestRuntimeValidationFailsBeforeBufferAllocation() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    uint32_t pixel = 0;
    const char* error = "stale";

    params.max_iter = 0;
    const bool ok = RenderFractalCUDA(view, params, render, &pixel, nullptr, &stats, &error);
    Check(!ok, "RenderFractalCUDA rejects invalid runtime state before rendering");
    Check(ErrorContains(error, "max_iter must be > 0"), "Runtime validation error names max_iter");
}

void TestInvalidResolutionFailsBeforeCudaAllocation() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    uint32_t pixel = 0;
    const char* error = "stale";

    render.resolution = {0, 1};
    const bool ok = RenderFractalCUDA(view, params, render, &pixel, nullptr, &stats, &error);
    Check(!ok, "RenderFractalCUDA rejects invalid resolution before buffer allocation");
    Check(ErrorContains(error, "Invalid resolution"), "Invalid resolution error is reported");
}

void TestTinyRenderProducesPixelsMaskAndStats() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    const char* error = nullptr;

    view.fractal_type = FractalType::newton;
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    params.max_iter = 12;
    params.coloring_mode = ColoringMode::root_basin;
    render.resolution = {2, 2};
    render.block_size = 64;
    render.sample_tier = SampleTier::fast;

    std::vector<uint32_t> pixels(4, 0u);
    std::vector<uint8_t> mask(4, 123u);
    const bool ok = RenderFractalCUDA(view, params, render, pixels.data(), mask.data(), &stats, &error);
    Check(ok, error ? error : "RenderFractalCUDA tiny render succeeds");
    if (!ok) {
        CleanupFractalCUDA();
        return;
    }

    bool sawAlpha = true;
    for (uint32_t pixel : pixels) {
        sawAlpha = sawAlpha && ((pixel >> 24) == 255u);
    }
    Check(sawAlpha, "Tiny render writes opaque RGBA pixels");

    bool maskValuesValid = true;
    for (uint8_t value : mask) {
        maskValuesValid = maskValuesValid && (value == 0u || value == 255u);
    }
    Check(maskValuesValid, "Tiny render writes binary mask values");
    Check(stats.last_device_id >= 0, "Render stats record a non-negative device id");
    Check(stats.last_iters_avg >= 0 && stats.last_iters_avg <= params.max_iter,
        "Render stats average iterations stay within max_iter");
    Check(stats.last_pixel_count == 4, "Render stats record rendered pixel count");
    Check(stats.last_iters_sum <= static_cast<unsigned long long>(params.max_iter) * 4ull,
        "Render stats iteration sum stays within max_iter times pixel count");
    Check(stats.resolved_eval.backend == NumericBackend::float32 && stats.resolved_eval.strategy == IterationStrategy::direct,
        "Render stats record resolved fast direct mode");
    Check(stats.last_render_ms > 0.0f, "Non-benchmark live render reports elapsed milliseconds for viewport pacing");

    CleanupFractalCUDA();
    CleanupFractalCUDA();
    Check(true, "CleanupFractalCUDA is idempotent after a tiny render");
}

void TestCounterfactualPairRenderProducesMultipleClassColors() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    const char* error = nullptr;

    view.fractal_type = FractalType::counterfactual_pair;
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    params.max_iter = 96;
    params.epsilon = 1e-6f;
    params.coloring_mode = ColoringMode::root_basin;
    render.resolution = {16, 16};
    render.block_size = 64;
    render.sample_tier = SampleTier::fast;

    std::vector<uint32_t> pixels(16 * 16, 0u);
    const bool ok = RenderFractalCUDA(view, params, render, pixels.data(), nullptr, &stats, &error);
    Check(ok, error ? error : "Counterfactual Pair render succeeds");
    if (!ok) {
        CleanupFractalCUDA();
        return;
    }

    bool sawDifferentPixel = false;
    const uint32_t firstPixel = pixels.front();
    for (uint32_t pixel : pixels) {
        if (pixel != firstPixel) {
            sawDifferentPixel = true;
            break;
        }
    }
    Check(sawDifferentPixel, "Counterfactual Pair render emits more than one explicit class color");

    CleanupFractalCUDA();
}

void TestProjectionAndFlowRenderProducesMultipleClassColors() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    const char* error = nullptr;

    view.fractal_type = FractalType::projection_and_flow;
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    params.max_iter = 96;
    params.epsilon = 1e-6f;
    params.coloring_mode = ColoringMode::root_basin;
    render.resolution = {16, 16};
    render.block_size = 64;
    render.sample_tier = SampleTier::fast;

    std::vector<uint32_t> pixels(16 * 16, 0u);
    const bool ok = RenderFractalCUDA(view, params, render, pixels.data(), nullptr, &stats, &error);
    Check(ok, error ? error : "Projection-and-Flow render succeeds");
    if (!ok) {
        CleanupFractalCUDA();
        return;
    }

    Check(CountDistinctPixels(pixels) >= 6, "Projection-and-Flow unit-radius render emits several explicit class colors");

    CleanupFractalCUDA();
}

void TestProjectionAndFlowNonUnitRadiusRenderDoesNotCollapseToThreeColors() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    const char* error = nullptr;

    view.fractal_type = FractalType::projection_and_flow;
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    params.max_iter = 96;
    params.epsilon = 1e-6f;
    params.coloring_mode = ColoringMode::root_basin;
    params.projection_and_flow_target_radius = 1.75f;
    params.projection_and_flow_pressure_threshold = 1.0f;
    render.resolution = {64, 48};
    render.block_size = 64;
    render.sample_tier = SampleTier::fast;

    std::vector<uint32_t> pixels(64 * 48, 0u);
    const bool ok = RenderFractalCUDA(view, params, render, pixels.data(), nullptr, &stats, &error);
    Check(ok, error ? error : "Projection-and-Flow non-unit-radius render succeeds");
    if (!ok) {
        CleanupFractalCUDA();
        return;
    }

    Check(CountDistinctPixels(pixels) >= 6, "Projection-and-Flow non-unit-radius render does not collapse to only three public class colors");

    CleanupFractalCUDA();
}

void TestProjectionAndFlowSmoothEscapeRenderKeepsStableClassesVisible() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    const char* error = nullptr;

    view.fractal_type = FractalType::projection_and_flow;
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    params.max_iter = 96;
    params.epsilon = 1e-6f;
    params.coloring_mode = ColoringMode::smooth_escape;
    params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
    params.projection_and_flow_target_radius = 1.75f;
    params.projection_and_flow_pressure_threshold = 1.0f;
    params.color_smooth_escape_scale = 1.0f;
    params.color_smooth_escape_bias = 0.0f;
    render.resolution = {64, 48};
    render.block_size = 64;
    render.sample_tier = SampleTier::fast;

    std::vector<uint32_t> pixels(64 * 48, 0u);
    const bool ok = RenderFractalCUDA(view, params, render, pixels.data(), nullptr, &stats, &error);
    Check(ok, error ? error : "Projection-and-Flow smooth_escape render succeeds");
    if (!ok) {
        CleanupFractalCUDA();
        return;
    }

    Check(CountDistinctPixels(pixels) > 1, "Projection-and-Flow smooth_escape render should not collapse to one flat color");
    Check(CountNonBlackPixels(pixels) > 0, "Projection-and-Flow smooth_escape render should keep stable classes visible instead of rendering all black");
    CleanupFractalCUDA();
}

KernelParams BaseMagnetParams() {
    KernelParams params{};
    params.max_iter = 180;
    params.coloring_mode = ColoringMode::smooth_escape;
    params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
    params.magnet_seed_real = 0.0f;
    params.magnet_seed_imag = 0.0f;
    params.magnet_relaxation = 1.0f;
    params.magnet_bailout = 12.0f;
    return params;
}

bool RenderMagnetPixels(
    const KernelParams& params,
    std::vector<uint32_t>* outPixels,
    RenderStats* outStats,
    const char** outError) {
    ViewState view{};
    RenderSettings render{};
    view.fractal_type = FractalType::magnet;
    view.center_hp_x = -0.08;
    view.center_hp_y = 0.0;
    view.log2_zoom = 1.13750352375;
    render.resolution = {64, 48};
    render.block_size = 64;
    render.sample_tier = SampleTier::fast;

    outPixels->assign(64 * 48, 0u);
    return RenderFractalCUDA(view, params, render, outPixels->data(), nullptr, outStats, outError);
}

void CheckMagnetControlChangesPixels(const char* label, const KernelParams& baselineParams, KernelParams changedParams) {
    std::vector<uint32_t> baselinePixels;
    std::vector<uint32_t> changedPixels;
    RenderStats baselineStats{};
    RenderStats changedStats{};
    const char* baselineError = nullptr;
    const char* changedError = nullptr;

    const bool baselineOk = RenderMagnetPixels(baselineParams, &baselinePixels, &baselineStats, &baselineError);
    Check(baselineOk, baselineError ? baselineError : "Magnet baseline smooth_escape render succeeds");
    const bool changedOk = RenderMagnetPixels(changedParams, &changedPixels, &changedStats, &changedError);
    Check(changedOk, changedError ? changedError : "Magnet changed smooth_escape render succeeds");
    if (!baselineOk || !changedOk) {
        CleanupFractalCUDA();
        return;
    }

    Check(CountDistinctPixels(baselinePixels) > 1, "Magnet render should emit a non-flat smooth_escape field");
    Check(baselinePixels != changedPixels, label);
    Check(changedStats.last_iters_avg >= 0 && changedStats.last_iters_avg <= changedParams.max_iter,
        "Magnet changed stats stay within max_iter");
    Check(changedStats.last_pixel_count == 64 * 48, "Magnet changed stats record rendered pixel count");
    CleanupFractalCUDA();
}

void TestMagnetRenderRespondsToVisibleControls() {
    const KernelParams baselineParams = BaseMagnetParams();

    KernelParams seedRealParams = baselineParams;
    seedRealParams.magnet_seed_real = 0.38f;
    CheckMagnetControlChangesPixels("Magnet render should react to magnet_seed_real changes", baselineParams, seedRealParams);

    KernelParams seedImagParams = baselineParams;
    seedImagParams.magnet_seed_imag = -0.31f;
    CheckMagnetControlChangesPixels("Magnet render should react to magnet_seed_imag changes", baselineParams, seedImagParams);

    KernelParams relaxationParams = baselineParams;
    relaxationParams.magnet_relaxation = 0.42f;
    CheckMagnetControlChangesPixels("Magnet render should react to magnet_relaxation changes", baselineParams, relaxationParams);

    KernelParams bailoutParams = baselineParams;
    bailoutParams.magnet_bailout = 4.0f;
    CheckMagnetControlChangesPixels("Magnet render should react to magnet_bailout changes", baselineParams, bailoutParams);
}

bool RenderJuliaPixels(
    const KernelParams& params,
    std::vector<uint32_t>* outPixels,
    RenderStats* outStats,
    const char** outError) {
    ViewState view{};
    RenderSettings render{};
    view.fractal_type = FractalType::julia;
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    render.resolution = {64, 48};
    render.block_size = 64;
    render.sample_tier = SampleTier::fast;

    outPixels->assign(64 * 48, 0u);
    return RenderFractalCUDA(view, params, render, outPixels->data(), nullptr, outStats, outError);
}

void CheckJuliaControlChangesPixels(const char* label, const KernelParams& baselineParams, KernelParams changedParams) {
    std::vector<uint32_t> baselinePixels;
    std::vector<uint32_t> changedPixels;
    RenderStats baselineStats{};
    RenderStats changedStats{};
    const char* baselineError = nullptr;
    const char* changedError = nullptr;

    const bool baselineOk = RenderJuliaPixels(baselineParams, &baselinePixels, &baselineStats, &baselineError);
    Check(baselineOk, baselineError ? baselineError : "Julia baseline smooth_escape render succeeds");
    const bool changedOk = RenderJuliaPixels(changedParams, &changedPixels, &changedStats, &changedError);
    Check(changedOk, changedError ? changedError : "Julia changed smooth_escape render succeeds");
    if (!baselineOk || !changedOk) {
        CleanupFractalCUDA();
        return;
    }

    Check(CountDistinctPixels(baselinePixels) > 1, "Julia render should emit a non-flat smooth_escape field");
    Check(baselinePixels != changedPixels, label);
    CleanupFractalCUDA();
}

void TestJuliaRenderRespondsToVisibleControls() {
    KernelParams baselineParams{};
    baselineParams.max_iter = 180;
    baselineParams.coloring_mode = ColoringMode::smooth_escape;
    baselineParams.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
    baselineParams.julia_c_real = -0.7f;
    baselineParams.julia_c_imag = 0.27015f;

    KernelParams realParams = baselineParams;
    realParams.julia_c_real = 0.285f;
    CheckJuliaControlChangesPixels("Julia render should react to julia_c_real changes", baselineParams, realParams);

    KernelParams imagParams = baselineParams;
    imagParams.julia_c_imag = 0.01f;
    CheckJuliaControlChangesPixels("Julia render should react to julia_c_imag changes", baselineParams, imagParams);
}

KernelParams BaseNovaParams() {
    KernelParams params{};
    params.max_iter = 180;
    params.coloring_mode = ColoringMode::smooth_escape;
    params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
    params.poly_kind = PolyKind::custom;
    params.poly_coeffs[0] = -1.0f;
    params.poly_coeffs[1] = 0.0f;
    params.poly_coeffs[2] = 0.0f;
    params.poly_coeffs[3] = 1.0f;
    params.poly_coeffs[4] = 0.0f;
    params.nova_alpha = 0.50f;
    return params;
}

bool RenderNovaPixels(
    const KernelParams& params,
    std::vector<uint32_t>* outPixels,
    RenderStats* outStats,
    const char** outError) {
    ViewState view{};
    RenderSettings render{};
    view.fractal_type = FractalType::nova;
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    render.resolution = {64, 48};
    render.block_size = 64;
    render.sample_tier = SampleTier::fast;

    outPixels->assign(64 * 48, 0u);
    return RenderFractalCUDA(view, params, render, outPixels->data(), nullptr, outStats, outError);
}

void TestNovaRenderRespondsToPolyC4() {
    const KernelParams baselineParams = BaseNovaParams();
    KernelParams changedParams = baselineParams;
    changedParams.poly_coeffs[4] = 0.65f;

    std::vector<uint32_t> baselinePixels;
    std::vector<uint32_t> changedPixels;
    RenderStats baselineStats{};
    RenderStats changedStats{};
    const char* baselineError = nullptr;
    const char* changedError = nullptr;

    const bool baselineOk = RenderNovaPixels(baselineParams, &baselinePixels, &baselineStats, &baselineError);
    Check(baselineOk, baselineError ? baselineError : "Nova baseline smooth_escape render succeeds");
    const bool changedOk = RenderNovaPixels(changedParams, &changedPixels, &changedStats, &changedError);
    Check(changedOk, changedError ? changedError : "Nova changed smooth_escape render succeeds");
    if (!baselineOk || !changedOk) {
        CleanupFractalCUDA();
        return;
    }

    Check(CountDistinctPixels(baselinePixels) > 1, "Nova render should emit a non-flat smooth_escape field");
    Check(baselinePixels != changedPixels, "Nova render should react to poly_c4 changes");
    CleanupFractalCUDA();
}

KernelParams BaseMultibrotParams() {
    KernelParams params{};
    params.max_iter = 160;
    params.coloring_mode = ColoringMode::smooth_escape;
    params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
    params.multibrot_power_float = 3.0f;
    params.multibrot_power_imag = 0.0f;
    return params;
}

bool RenderMultibrotPixels(
    const KernelParams& params,
    std::vector<uint32_t>* outPixels,
    RenderStats* outStats,
    const char** outError) {
    ViewState view{};
    RenderSettings render{};
    view.fractal_type = FractalType::multibrot;
    view.center_hp_x = -0.48;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    render.resolution = {64, 48};
    render.block_size = 64;
    render.sample_tier = SampleTier::fast;

    outPixels->assign(64 * 48, 0u);
    return RenderFractalCUDA(view, params, render, outPixels->data(), nullptr, outStats, outError);
}

void CheckMultibrotControlChangesPixels(const char* label, const KernelParams& baselineParams, KernelParams changedParams) {
    std::vector<uint32_t> baselinePixels;
    std::vector<uint32_t> changedPixels;
    RenderStats baselineStats{};
    RenderStats changedStats{};
    const char* baselineError = nullptr;
    const char* changedError = nullptr;

    const bool baselineOk = RenderMultibrotPixels(baselineParams, &baselinePixels, &baselineStats, &baselineError);
    Check(baselineOk, baselineError ? baselineError : "Multibrot baseline smooth_escape render succeeds");
    const bool changedOk = RenderMultibrotPixels(changedParams, &changedPixels, &changedStats, &changedError);
    Check(changedOk, changedError ? changedError : "Multibrot changed smooth_escape render succeeds");
    if (!baselineOk || !changedOk) {
        CleanupFractalCUDA();
        return;
    }

    Check(CountDistinctPixels(baselinePixels) > 1, "Multibrot render should emit a non-flat smooth_escape field");
    Check(baselinePixels != changedPixels, label);
    CleanupFractalCUDA();
}

void TestMultibrotRenderRespondsToRealAndImaginaryPower() {
    const KernelParams baselineParams = BaseMultibrotParams();

    KernelParams lowRealParams = baselineParams;
    lowRealParams.multibrot_power_float = 1.5f;
    CheckMultibrotControlChangesPixels("Multibrot render should react to a below-two real exponent", baselineParams, lowRealParams);

    KernelParams highRealParams = baselineParams;
    highRealParams.multibrot_power_float = 16.0f;
    CheckMultibrotControlChangesPixels("Multibrot render should react to a real exponent above the old UI cap", baselineParams, highRealParams);

    KernelParams imagParams = baselineParams;
    imagParams.multibrot_power_imag = 0.75f;
    CheckMultibrotControlChangesPixels("Multibrot render should react to a nonzero imaginary exponent", baselineParams, imagParams);
}

void TestProjectionAndFlowSmoothEscapeRenderRespondsToPressureThreshold() {
    ViewState view{};
    KernelParams tightParams{};
    RenderSettings render{};
    RenderStats tightStats{};
    RenderStats looseStats{};
    const char* tightError = nullptr;
    const char* looseError = nullptr;

    view.fractal_type = FractalType::projection_and_flow;
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    tightParams.max_iter = 96;
    tightParams.epsilon = 1e-6f;
    tightParams.coloring_mode = ColoringMode::smooth_escape;
    tightParams.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
    tightParams.projection_and_flow_target_radius = 1.75f;
    tightParams.projection_and_flow_pressure_threshold = 0.25f;
    tightParams.color_smooth_escape_scale = 1.0f;
    tightParams.color_smooth_escape_bias = 0.0f;
    render.resolution = {64, 48};
    render.block_size = 64;
    render.sample_tier = SampleTier::fast;

    KernelParams looseParams = tightParams;
    looseParams.projection_and_flow_pressure_threshold = 2.0f;

    std::vector<uint32_t> tightPixels(64 * 48, 0u);
    std::vector<uint32_t> loosePixels(64 * 48, 0u);
    const bool tightOk = RenderFractalCUDA(view, tightParams, render, tightPixels.data(), nullptr, &tightStats, &tightError);
    Check(tightOk, tightError ? tightError : "Projection-and-Flow threshold-tight smooth_escape render succeeds");
    const bool looseOk = RenderFractalCUDA(view, looseParams, render, loosePixels.data(), nullptr, &looseStats, &looseError);
    Check(looseOk, looseError ? looseError : "Projection-and-Flow threshold-loose smooth_escape render succeeds");
    if (!tightOk || !looseOk) {
        CleanupFractalCUDA();
        return;
    }

    Check(tightPixels != loosePixels,
        "Projection-and-Flow smooth_escape render should react to pressure-threshold changes on the same radius lane");
    CleanupFractalCUDA();
}

void TestColorSourceSignalSidecarProducesFiniteRowOrderedSignals() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    const char* error = nullptr;

    view.fractal_type = FractalType::multibrot;
    view.center_hp_x = -0.5;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    params.max_iter = 64;
    params.coloring_mode = ColoringMode::smooth_escape;
    params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
    params.color_source_stack_count = 2;
    params.color_source_stack[0].signal = ColorSignal::smooth_escape;
    params.color_source_stack[0].params.scale = 1.0f;
    params.color_source_stack[0].params.bias = 0.0f;
    params.color_source_stack[0].params.blend_weight = 1.0f;
    params.color_source_stack[1].signal = ColorSignal::escape_magnitude;
    params.color_source_stack[1].params.scale = 1.0f;
    params.color_source_stack[1].params.bias = 0.0f;
    params.color_source_stack[1].params.blend_weight = 0.5f;
    render.resolution = {16, 12};
    render.block_size = 64;
    render.sample_tier = SampleTier::fast;

    const int pixelCount = render.resolution.x * render.resolution.y;
    std::vector<uint32_t> pixels(static_cast<std::size_t>(pixelCount), 0u);
    std::vector<float> signals(static_cast<std::size_t>(pixelCount * 2), -999.0f);
    const bool ok = RenderFractalCUDAWithColorSourceSignals(
        view,
        params,
        render,
        pixels.data(),
        nullptr,
        signals.data(),
        2,
        &stats,
        &error);
    Check(ok, error ? error : "RenderFractalCUDAWithColorSourceSignals succeeds");
    if (!ok) {
        CleanupFractalCUDA();
        return;
    }

    Check(AllFiniteValues(signals, 0, pixelCount), "Source signal sidecar row 0 is finite");
    Check(AllFiniteValues(signals, pixelCount, pixelCount), "Source signal sidecar row 1 is finite");
    Check(HashFloatValues(signals, 0, pixelCount) != HashFloatValues(signals, pixelCount, pixelCount),
        "Source signal sidecar preserves distinct row signals");
    CleanupFractalCUDA();
}

} // namespace

int main() {
    TestNullOutputFailsBeforeCudaWork();
    TestRuntimeValidationFailsBeforeBufferAllocation();
    TestInvalidResolutionFailsBeforeCudaAllocation();
    TestTinyRenderProducesPixelsMaskAndStats();
    TestCounterfactualPairRenderProducesMultipleClassColors();
    TestProjectionAndFlowRenderProducesMultipleClassColors();
    TestProjectionAndFlowNonUnitRadiusRenderDoesNotCollapseToThreeColors();
    TestProjectionAndFlowSmoothEscapeRenderKeepsStableClassesVisible();
    TestMagnetRenderRespondsToVisibleControls();
    TestJuliaRenderRespondsToVisibleControls();
    TestNovaRenderRespondsToPolyC4();
    TestMultibrotRenderRespondsToRealAndImaginaryPower();
    TestProjectionAndFlowSmoothEscapeRenderRespondsToPressureThreshold();
    TestColorSourceSignalSidecarProducesFiniteRowOrderedSignals();

    std::cout << "test_fractal_renderer: passed=" << g_passed << " failed=" << g_failed << "\n";
    return g_failed == 0 ? 0 : 1;
}
