#include "generic_equation_pack_live.h"

#include "escape_time_coloring.h"
#include "generic_sample_core.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

namespace {

constexpr double kPi = 3.141592653589793238462643383279502884;

bool FailLiveRender(const std::string& message, std::string* outError) {
    if (outError) {
        *outError = message;
    }
    return false;
}

struct GenericLiveColor {
    unsigned char x;
    unsigned char y;
    unsigned char z;
    unsigned char w;
};

struct GenericLiveComplex {
    float x;
    float y;
};

std::uint32_t PackRgba(GenericLiveColor color) {
    return static_cast<std::uint32_t>(color.x) |
        (static_cast<std::uint32_t>(color.y) << 8) |
        (static_cast<std::uint32_t>(color.z) << 16) |
        (static_cast<std::uint32_t>(color.w) << 24);
}

int ResolvePackMaxIter(const GenericEquationPack& pack, const GenericFunctionDesc& desc) {
    int maxIter = desc.max_iterate > 0 ? desc.max_iterate : 1;
    if (!pack.iteration_param.empty()) {
        const auto it = pack.params.find(pack.iteration_param);
        if (it != pack.params.end() && std::isfinite(it->second) && it->second >= 1.0) {
            maxIter = static_cast<int>(std::lround(it->second));
        }
    }
    return (std::max)(1, maxIter);
}

bool ValidateGenericEquationPackColorState(
    const ViewState& view,
    const KernelParams& params,
    std::string* outError) {
    if (!IsColoringModeAllowedForFractal(view.fractal_type, params.coloring_mode)) {
        return FailLiveRender("selected coloring_mode is not valid for generic_equation_pack", outError);
    }
    if (!IsColorPipelineAllowedForFractal(view.fractal_type, params.color_pipeline)) {
        return FailLiveRender("selected Color Pipeline is not valid for generic_equation_pack", outError);
    }
    return true;
}

std::uint32_t ColorPipelinePixelFromResult(
    const GenericSampleResult& result,
    const KernelParams& params,
    int maxIter) {
    if (!std::isfinite(result.value_x) ||
        !std::isfinite(result.value_y) ||
        !std::isfinite(result.abs2)) {
        return PackRgba({255, 0, 255, 255});
    }

    const GenericLiveComplex z{
        static_cast<float>(result.value_x),
        static_cast<float>(result.value_y),
    };
    GenericLiveColor color = MakeEscapeTimeBaseColor<GenericLiveColor>(
        FractalType::generic_equation_pack,
        params.coloring_mode,
        result.diverged,
        (std::max)(0, result.iterations),
        (std::max)(1, maxIter),
        z,
        params);
    color = ApplyFractalColorGrading(color, params);
    return PackRgba(color);
}

std::vector<GFPoint> BuildViewportGrid(const ViewState& view, int width, int height) {
    std::vector<GFPoint> coords;
    coords.reserve(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));

    const double aspect = height > 0 ? static_cast<double>(width) / static_cast<double>(height) : 1.0;
    const double zoom = (std::max)(1.0e-300, std::exp2(view.log2_zoom));
    const double base = 2.0 / zoom;
    const double radians = static_cast<double>(view.rotation_degrees) * (kPi / 180.0);
    const double cs = std::cos(radians);
    const double sn = std::sin(radians);
    const bool rotated = view.rotation_degrees != 0.0f;

    for (int py = 0; py < height; ++py) {
        const double ny = ((static_cast<double>(py) + 0.5) / static_cast<double>(height) - 0.5) * 2.0;
        for (int px = 0; px < width; ++px) {
            const double nx = ((static_cast<double>(px) + 0.5) / static_cast<double>(width) - 0.5) * 2.0;
            double x = view.center_hp_x + nx * base * aspect;
            double y = view.center_hp_y + ny * base;
            if (rotated) {
                const double rx = (x - view.center_hp_x) * cs - (y - view.center_hp_y) * sn;
                const double ry = (x - view.center_hp_x) * sn + (y - view.center_hp_y) * cs;
                x = view.center_hp_x + rx;
                y = view.center_hp_y + ry;
            }
            coords.push_back({x, y});
        }
    }
    return coords;
}

} // namespace

bool RenderGenericEquationPackLiveFrame(
    const GenericEquationPack& pack,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    std::uint32_t* outRGBA,
    std::uint8_t* outMask,
    RenderStats* outStats,
    std::string* outError) {
    if (outError) {
        outError->clear();
    }
    if (view.fractal_type != FractalType::generic_equation_pack) {
        return FailLiveRender("generic live render requires generic_equation_pack fractal_type", outError);
    }
    if (!outRGBA) {
        return FailLiveRender("outRGBA is null", outError);
    }
    if (!ValidateGenericEquationPackColorState(view, params, outError)) {
        return false;
    }
    const int width = render.resolution.x;
    const int height = render.resolution.y;
    if (width <= 0 || height <= 0) {
        return FailLiveRender("render resolution must be positive", outError);
    }
    const std::uint64_t pixelCount64 = static_cast<std::uint64_t>(static_cast<unsigned int>(width)) *
        static_cast<std::uint64_t>(static_cast<unsigned int>(height));
    if (pixelCount64 > static_cast<std::uint64_t>((std::numeric_limits<int>::max)())) {
        return FailLiveRender("render resolution is too large for generic live render", outError);
    }

    GenericEquationLowerResult lowered = LowerGenericEquationPackToDesc(pack);
    if (!lowered.ok) {
        return FailLiveRender("AST lower error: " + lowered.error, outError);
    }

    const auto start = std::chrono::steady_clock::now();
    std::vector<GFPoint> coords = BuildViewportGrid(view, width, height);
    std::vector<GenericSampleResult> results(coords.size());
    const char* rawError = nullptr;
    if (!SampleGenericFunction(
            coords.data(),
            static_cast<int>(coords.size()),
            lowered.desc,
            pack.epsilon,
            pack.escape_radius,
            results.data(),
            &rawError)) {
        return FailLiveRender(rawError ? rawError : "CUDA generic sample execution failed", outError);
    }

    const int colorMaxIter = ResolvePackMaxIter(pack, lowered.desc);
    unsigned long long iterationSum = 0;
    for (std::size_t i = 0; i < results.size(); ++i) {
        const GenericSampleResult& result = results[i];
        iterationSum += static_cast<unsigned long long>((std::max)(0, result.iterations));
        outRGBA[i] = ColorPipelinePixelFromResult(result, params, colorMaxIter);
        if (outMask) {
            outMask[i] = result.diverged ? 0 : 255;
        }
    }

    if (outStats) {
        const auto stop = std::chrono::steady_clock::now();
        const double elapsedMs = std::chrono::duration<double, std::milli>(stop - start).count();
        const int pixelCount = ComputeRenderStatsPixelCount(width, height);
        outStats->last_render_ms = static_cast<float>(elapsedMs);
        outStats->last_iters_sum = iterationSum;
        outStats->last_iters_avg = ComputeRenderStatsIterationAverage(iterationSum, pixelCount);
        outStats->last_pixel_count = pixelCount;
        outStats->last_device_id = 0;
        outStats->resolved_eval = {NumericBackend::float64, IterationStrategy::direct};
    }
    return true;
}
