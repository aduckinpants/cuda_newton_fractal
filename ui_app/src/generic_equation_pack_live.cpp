#include "generic_equation_pack_live.h"

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

double Saturate(double value) {
    if (!std::isfinite(value)) {
        return 0.0;
    }
    return (std::max)(0.0, (std::min)(1.0, value));
}

std::uint8_t ToByte(double value) {
    return static_cast<std::uint8_t>(std::lround(Saturate(value) * 255.0));
}

std::uint32_t PackRgba(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255) {
    return static_cast<std::uint32_t>(r) |
        (static_cast<std::uint32_t>(g) << 8) |
        (static_cast<std::uint32_t>(b) << 16) |
        (static_cast<std::uint32_t>(a) << 24);
}

double SafeMagnitudeSignal(const GenericSampleResult& result) {
    if (std::isfinite(result.abs2) && result.abs2 >= 0.0) {
        return std::log1p(std::sqrt(result.abs2));
    }
    if (std::isfinite(result.value_x) && std::isfinite(result.value_y)) {
        return std::log1p(std::hypot(result.value_x, result.value_y));
    }
    return 0.0;
}

std::uint32_t LivePixelFromResult(const GenericSampleResult& result) {
    if (!std::isfinite(result.value_x) ||
        !std::isfinite(result.value_y) ||
        !std::isfinite(result.abs2)) {
        return PackRgba(190, 64, 220);
    }

    const double magnitude = Saturate(SafeMagnitudeSignal(result) / 5.0);
    const double iteration = Saturate(static_cast<double>((std::max)(0, result.iterations)) / 128.0);
    const double phase = Saturate((std::atan2(result.value_y, result.value_x) + kPi) / (2.0 * kPi));

    if (result.converged) {
        return PackRgba(
            ToByte(0.12 + 0.55 * phase),
            ToByte(0.45 + 0.25 * (1.0 - magnitude) + 0.25 * iteration),
            ToByte(0.22 + 0.50 * magnitude));
    }
    if (result.diverged) {
        return PackRgba(
            ToByte(0.70 + 0.24 * iteration),
            ToByte(0.24 + 0.42 * (1.0 - magnitude)),
            ToByte(0.12 + 0.30 * phase));
    }
    return PackRgba(
        ToByte(0.10 + 0.34 * magnitude + 0.20 * iteration),
        ToByte(0.36 + 0.36 * phase),
        ToByte(0.58 + 0.28 * (1.0 - magnitude)));
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

    unsigned long long iterationSum = 0;
    for (std::size_t i = 0; i < results.size(); ++i) {
        const GenericSampleResult& result = results[i];
        iterationSum += static_cast<unsigned long long>((std::max)(0, result.iterations));
        outRGBA[i] = LivePixelFromResult(result);
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
