#include "../src/fractal_types.h"
#include "../src/fractal_family_rules.h"

#include <cstdint>
#include <iostream>
#include <vector>

namespace {

const char* BackendName(NumericBackend backend) {
    switch (backend) {
    case NumericBackend::float32: return "float32";
    case NumericBackend::float64: return "float64";
    }
    return "unknown";
}

bool RenderDeepMandelbrot(SampleTier tier, std::vector<uint32_t>* outPixels, RenderStats* outStats) {
    if (!outPixels) return false;

    ViewState view{};
    view.fractal_type = FractalType::mandelbrot;
    view.center_hp_x = -0.743643887037151;
    view.center_hp_y = 0.131825904205330;
    view.log2_zoom = 30.0;
    view.center.x = static_cast<float>(view.center_hp_x);
    view.center.y = static_cast<float>(view.center_hp_y);
    view.zoom = static_cast<float>(1073741824.0);

    KernelParams params{};
    params.max_iter = 1200;
    params.coloring_mode = ColoringMode::smooth_escape;
    params.exposure = 1.0f;

    RenderSettings render{};
    render.resolution = {64, 64};
    render.block_size = 256;
    render.device_id = 0;
    render.sample_tier = tier;

    outPixels->assign(static_cast<size_t>(render.resolution.x) * static_cast<size_t>(render.resolution.y), 0u);
    const char* error = nullptr;
    RenderStats stats{};
    if (!RenderFractalCUDA(view, params, render, outPixels->data(), nullptr, &stats, &error)) {
        std::cerr << "RenderFractalCUDA failed for Mandelbrot tier test: " << (error ? error : "unknown") << "\n";
        return false;
    }
    if (outStats) *outStats = stats;
    return true;
}


bool RenderExplainoSmoothEscape(SampleTier tier, std::vector<uint32_t>* outPixels, RenderStats* outStats) {
    if (!outPixels) return false;

    ViewState view{};
    view.fractal_type = FractalType::explaino;
    view.center_hp_x = 1.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 11.0;
    view.center.x = static_cast<float>(view.center_hp_x);
    view.center.y = static_cast<float>(view.center_hp_y);
    view.zoom = static_cast<float>(2048.0);

    KernelParams params{};
    params.max_iter = 64;
    params.epsilon = 1.0e-6f;
    params.coloring_mode = ColoringMode::smooth_escape;
    params.color_pipeline = {ColorSignal::smooth_escape, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
    params.color_smooth_escape_scale = 1.0f;
    params.color_smooth_escape_bias = 0.0f;
    params.color_heatmap_cycle_scale = 0.25f;
    params.color_heatmap_saturation = 1.0f;
    params.exposure = 1.0f;

    RenderSettings render{};
    render.resolution = {64, 64};
    render.block_size = 256;
    render.device_id = 0;
    render.sample_tier = tier;

    outPixels->assign(static_cast<size_t>(render.resolution.x) * static_cast<size_t>(render.resolution.y), 0u);
    const char* error = nullptr;
    RenderStats stats{};
    if (!RenderFractalCUDA(view, params, render, outPixels->data(), nullptr, &stats, &error)) {
        std::cerr << "RenderFractalCUDA failed for ExplainO smooth_escape tier test: " << (error ? error : "unknown") << "\n";
        return false;
    }
    if (outStats) *outStats = stats;
    return true;
}

void SeedExplainoRootProximityPolynomial(KernelParams& params) {
    params.poly_kind = PolyKind::custom;
    params.explaino_root_count = 4;
    params.explaino_roots[0] = {-0.377730466f, 0.808940309f};
    params.explaino_roots[1] = {-0.377730466f, -0.808940309f};
    params.explaino_roots[2] = {-0.756559130f, 0.828689252f};
    params.explaino_roots[3] = {-0.756559130f, -0.828689252f};

    params.poly_coeffs[0] = 1.003590253f;
    params.poly_coeffs[1] = 2.157259794f;
    params.poly_coeffs[2] = 3.199274055f;
    params.poly_coeffs[3] = 2.268579193f;
    params.poly_coeffs[4] = 1.000000000f;
}

bool RenderExplainoRootProximity(SampleTier tier, std::vector<uint32_t>* outPixels, RenderStats* outStats) {
    if (!outPixels) return false;

    ViewState view{};
    view.fractal_type = FractalType::explaino;
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    view.center.x = 0.0f;
    view.center.y = 0.0f;
    view.zoom = 1.0f;

    KernelParams params{};
    params.max_iter = 500;
    params.epsilon = 1.0e-6f;
    SeedExplainoRootProximityPolynomial(params);
    params.coloring_mode = ColoringMode::smooth_escape;
    params.color_pipeline = {ColorSignal::root_proximity, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
    params.color_root_proximity_scale = 1.0f;
    params.color_root_proximity_bias = 0.0f;
    params.color_heatmap_cycle_scale = 1.0f;
    params.color_heatmap_saturation = 1.0f;
    params.exposure = 1.0f;

    RenderSettings render{};
    render.resolution = {96, 96};
    render.block_size = 256;
    render.device_id = 0;
    render.sample_tier = tier;

    outPixels->assign(static_cast<size_t>(render.resolution.x) * static_cast<size_t>(render.resolution.y), 0u);
    const char* error = nullptr;
    RenderStats stats{};
    if (!RenderFractalCUDA(view, params, render, outPixels->data(), nullptr, &stats, &error)) {
        std::cerr << "RenderFractalCUDA failed for ExplainO root_proximity tier test: " << (error ? error : "unknown") << "\n";
        return false;
    }
    if (outStats) *outStats = stats;
    return true;
}

int CountPixelDiffs(const std::vector<uint32_t>& left, const std::vector<uint32_t>& right) {
    if (left.size() != right.size()) return -1;
    int diffs = 0;
    for (size_t i = 0; i < left.size(); ++i) {
        if (left[i] != right[i]) ++diffs;
    }
    return diffs;
}

__global__ void ProbeExplainoAxisRegistryDevice(KernelParams params, FractalType carrierFractalType, int* outFlags) {
    if (!outFlags) {
        return;
    }
    outFlags[0] = HasAnyExplainoAxisRegistryPerturbation(params) ? 1 : 0;
    outFlags[1] = HasExplainoAxisRegistryPerturbationForCarrier(carrierFractalType, params) ? 1 : 0;
}

bool DeviceExplainoAxisRegistryFlags(KernelParams params, FractalType carrierFractalType, int* outAny, int* outCarrier) {
    int* deviceFlags = nullptr;
    int hostFlags[2] = {0, 0};
    if (cudaMalloc(&deviceFlags, sizeof(hostFlags)) != cudaSuccess) {
        return false;
    }
    if (cudaMemset(deviceFlags, 0, sizeof(hostFlags)) != cudaSuccess) {
        cudaFree(deviceFlags);
        return false;
    }
    ProbeExplainoAxisRegistryDevice<<<1, 1>>>(params, carrierFractalType, deviceFlags);
    const cudaError_t launchError = cudaGetLastError();
    if (launchError != cudaSuccess || cudaDeviceSynchronize() != cudaSuccess) {
        cudaFree(deviceFlags);
        return false;
    }
    if (cudaMemcpy(hostFlags, deviceFlags, sizeof(hostFlags), cudaMemcpyDeviceToHost) != cudaSuccess) {
        cudaFree(deviceFlags);
        return false;
    }
    cudaFree(deviceFlags);
    if (outAny) *outAny = hostFlags[0];
    if (outCarrier) *outCarrier = hostFlags[1];
    return true;
}

bool VerifyExplainoAxisRegistryDeviceParity() {
    KernelParams neutral{};
    int any = -1;
    int carrier = -1;
    if (!DeviceExplainoAxisRegistryFlags(neutral, FractalType::explaino_ripple, &any, &carrier)) {
        std::cerr << "CUDA Explaino axis registry device probe failed for neutral params\n";
        return false;
    }
    if (any != 0 || carrier != 0) {
        std::cerr << "Neutral params should not activate any CUDA Explaino axis registry predicate\n";
        return false;
    }

    for (const auto& axis : kExplainoAxisRegistry) {
        KernelParams params{};
        float* axisValue = ResolveExplainoAxisValue(params, axis.slot);
        if (!axisValue) {
            std::cerr << "Every registry axis should resolve before CUDA device parity probe\n";
            return false;
        }
        *axisValue = axis.default_value != 0.0f ? axis.default_value : 0.35f;
        any = 0;
        carrier = 0;
        if (!DeviceExplainoAxisRegistryFlags(params, axis.carrier_fractal_type, &any, &carrier)) {
            std::cerr << "CUDA Explaino axis registry device probe failed for " << axis.axis_id << "\n";
            return false;
        }
        if (any != 1 || carrier != 1) {
            std::cerr << "CUDA Explaino axis registry predicate drifted from host registry for " << axis.axis_id
                      << ": any=" << any << " carrier=" << carrier << "\n";
            return false;
        }
    }
    return true;
}

} // namespace

int main() {
    if (!VerifyExplainoAxisRegistryDeviceParity()) {
        CleanupFractalCUDA();
        return 1;
    }

    std::vector<uint32_t> fastPixels;
    std::vector<uint32_t> standardPixels;
    RenderStats fastStats{};
    RenderStats standardStats{};

    if (!RenderDeepMandelbrot(SampleTier::fast, &fastPixels, &fastStats)) {
        CleanupFractalCUDA();
        return 1;
    }
    if (!RenderDeepMandelbrot(SampleTier::standard, &standardPixels, &standardStats)) {
        CleanupFractalCUDA();
        return 1;
    }

    std::vector<uint32_t> explainoFastPixels;
    std::vector<uint32_t> explainoStandardPixels;
    std::vector<uint32_t> explainoAutoPixels;
    RenderStats explainoFastStats{};
    RenderStats explainoStandardStats{};
    RenderStats explainoAutoStats{};

    if (!RenderExplainoSmoothEscape(SampleTier::fast, &explainoFastPixels, &explainoFastStats)) {
        CleanupFractalCUDA();
        return 1;
    }
    if (!RenderExplainoSmoothEscape(SampleTier::standard, &explainoStandardPixels, &explainoStandardStats)) {
        CleanupFractalCUDA();
        return 1;
    }
    if (!RenderExplainoSmoothEscape(SampleTier::tier_auto, &explainoAutoPixels, &explainoAutoStats)) {
        CleanupFractalCUDA();
        return 1;
    }

    const int explainoPixelDiffs = CountPixelDiffs(explainoFastPixels, explainoStandardPixels);
    constexpr int kMaterialSmoothEscapeDiffPixels = 64;
    if (explainoPixelDiffs < kMaterialSmoothEscapeDiffPixels) {
        std::cerr << "Smooth_escape classification witness is inconclusive: fast vs standard differ by "
                  << explainoPixelDiffs << " pixels, below material threshold "
                  << kMaterialSmoothEscapeDiffPixels << "\n";
        CleanupFractalCUDA();
        return 1;
    }

    if (explainoFastStats.resolved_eval.backend != NumericBackend::float32 ||
        explainoStandardStats.resolved_eval.backend != NumericBackend::float64) {
        std::cerr << "Smooth_escape classification witness expected explicit fast=float32 and standard=float64, got fast="
                  << BackendName(explainoFastStats.resolved_eval.backend)
                  << ", standard=" << BackendName(explainoStandardStats.resolved_eval.backend) << "\n";
        CleanupFractalCUDA();
        return 1;
    }

    if (explainoAutoStats.resolved_eval.backend != NumericBackend::float64) {
        const int autoFastDiffs = CountPixelDiffs(explainoAutoPixels, explainoFastPixels);
        const int autoStandardDiffs = CountPixelDiffs(explainoAutoPixels, explainoStandardPixels);
        std::cerr << "Expected tier_auto to classify the material ExplainO smooth_escape fast/standard delta as requiring float64; "
                  << "auto backend=" << BackendName(explainoAutoStats.resolved_eval.backend)
                  << ", fast_standard_diffs=" << explainoPixelDiffs
                  << ", auto_fast_diffs=" << autoFastDiffs
                  << ", auto_standard_diffs=" << autoStandardDiffs << "\n";
        CleanupFractalCUDA();
        return 1;
    }

    std::vector<uint32_t> rootProximityFastPixels;
    std::vector<uint32_t> rootProximityStandardPixels;
    std::vector<uint32_t> rootProximityAutoPixels;
    RenderStats rootProximityFastStats{};
    RenderStats rootProximityStandardStats{};
    RenderStats rootProximityAutoStats{};

    if (!RenderExplainoRootProximity(SampleTier::fast, &rootProximityFastPixels, &rootProximityFastStats)) {
        CleanupFractalCUDA();
        return 1;
    }
    if (!RenderExplainoRootProximity(SampleTier::standard, &rootProximityStandardPixels, &rootProximityStandardStats)) {
        CleanupFractalCUDA();
        return 1;
    }
    if (!RenderExplainoRootProximity(SampleTier::tier_auto, &rootProximityAutoPixels, &rootProximityAutoStats)) {
        CleanupFractalCUDA();
        return 1;
    }

    const int rootProximityPixelDiffs = CountPixelDiffs(rootProximityFastPixels, rootProximityStandardPixels);
    constexpr int kMaterialRootProximityDiffPixels = 64;
    if (rootProximityPixelDiffs < kMaterialRootProximityDiffPixels) {
        std::cerr << "Root_proximity classification witness is inconclusive: fast vs standard differ by "
                  << rootProximityPixelDiffs << " pixels, below material threshold "
                  << kMaterialRootProximityDiffPixels << "\n";
        CleanupFractalCUDA();
        return 1;
    }

    if (rootProximityFastStats.resolved_eval.backend != NumericBackend::float32 ||
        rootProximityStandardStats.resolved_eval.backend != NumericBackend::float64) {
        std::cerr << "Root_proximity classification witness expected explicit fast=float32 and standard=float64, got fast="
                  << BackendName(rootProximityFastStats.resolved_eval.backend)
                  << ", standard=" << BackendName(rootProximityStandardStats.resolved_eval.backend) << "\n";
        CleanupFractalCUDA();
        return 1;
    }

    if (rootProximityAutoStats.resolved_eval.backend != NumericBackend::float64) {
        const int autoFastDiffs = CountPixelDiffs(rootProximityAutoPixels, rootProximityFastPixels);
        const int autoStandardDiffs = CountPixelDiffs(rootProximityAutoPixels, rootProximityStandardPixels);
        std::cerr << "Expected tier_auto to classify the material ExplainO root_proximity fast/standard delta as requiring float64; "
                  << "auto backend=" << BackendName(rootProximityAutoStats.resolved_eval.backend)
                  << ", fast_standard_diffs=" << rootProximityPixelDiffs
                  << ", auto_fast_diffs=" << autoFastDiffs
                  << ", auto_standard_diffs=" << autoStandardDiffs << "\n";
        CleanupFractalCUDA();
        return 1;
    }

    const int pixelDiffs = CountPixelDiffs(fastPixels, standardPixels);
    if (pixelDiffs <= 0) {
        std::cerr << "Expected deep Mandelbrot fast vs standard tiers to differ, but images were identical"
                  << " (fast avg iters=" << fastStats.last_iters_avg
                  << ", standard avg iters=" << standardStats.last_iters_avg << ")\n";
        CleanupFractalCUDA();
        return 1;
    }

    std::cout << "escape_time_sample_tier: " << pixelDiffs
              << " differing pixels, fast avg iters " << fastStats.last_iters_avg
              << ", standard avg iters " << standardStats.last_iters_avg << "\n";

    CleanupFractalCUDA();
    return 0;
}