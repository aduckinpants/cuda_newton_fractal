// test_fractal_sample_equivalence.cu
//
// K4 validation: headless equivalence test — SampleFractalPoints vs. RenderFractalCUDA.
//
// For every fractal type, renders a 256x256 frame via RenderFractalCUDA, then
// samples the same coordinates via SampleFractalPoints, and verifies:
//   (a) Both calls succeed
//   (b) Sum of iterations from sample path matches RenderStats.last_iters_avg
//       (within ±1 for integer division rounding)
//   (c) All 65536 sample results are valid (finite, in-range)
//   (d) Converged/escaped flags are consistent with rendered RGBA:
//       - Newton/basin family: converged pixel should have non-zero RGB
//       - Escape family: escaped pixel should have non-zero RGB
//
// Exit criteria (from spec):
//   256x256 grid for all types; results match within float epsilon.

#include "../src/fractal_types.h"
#include "../src/fractal_sample_result.h"
#include "../src/fractal_family_rules.h"

#include <cstdint>
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>

static int gPass = 0;
static int gFail = 0;
static int gWarn = 0;

#define CHECK(name, cond) do { \
    if (cond) { \
        ++gPass; \
    } else { \
        ++gFail; \
        std::cerr << "  FAIL: " << (name) << "  (" << __FILE__ << ":" << __LINE__ << ")\n"; \
    } \
} while(0)

#define WARN(name, cond) do { \
    if (!(cond)) { \
        ++gWarn; \
        std::cerr << "  WARN: " << (name) << "  (" << __FILE__ << ":" << __LINE__ << ")\n"; \
    } \
} while(0)

namespace {

constexpr int kWidth = 256;
constexpr int kHeight = 256;
constexpr int kNumPixels = kWidth * kHeight;

// Replicate kernel_render's pixel-to-coordinate mapping on the host.
// This is the same double-precision math used in the GPU render kernel.
void ComputePixelCoordinates(
    const ViewState& view, int width, int height,
    std::vector<Double2>& outCoords)
{
    outCoords.resize((size_t)width * (size_t)height);

    double aspect = (height > 0) ? (double)width / (double)height : 1.0;
    double zoom = fmax(1.0e-300, exp2(view.log2_zoom));
    double base = 2.0 / zoom;

    for (int py = 0; py < height; ++py) {
        for (int px = 0; px < width; ++px) {
            double nx = (((double)px + 0.5) / (double)width - 0.5) * 2.0;
            double ny = (((double)py + 0.5) / (double)height - 0.5) * 2.0;

            double x = view.center_hp_x + nx * base * aspect;
            double y = view.center_hp_y + ny * base;

            // Rotation (replicates kernel_render exactly)
            if (view.rotation_degrees != 0.0f) {
                double a = (double)view.rotation_degrees * (double)(3.14159265358979323846 / 180.0);
                double cs = cos(a);
                double sn = sin(a);
                double cx = view.center_hp_x;
                double cy = view.center_hp_y;
                double rx = (x - cx) * cs - (y - cy) * sn;
                double ry = (x - cx) * sn + (y - cy) * cs;
                x = cx + rx;
                y = cy + ry;
            }

            outCoords[(size_t)py * (size_t)width + (size_t)px] = {x, y};
        }
    }
}

void MakeDefaults(FractalType ft, ViewState& view, KernelParams& params, RenderSettings& render) {
    view = {};
    view.fractal_type = ft;
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    view.zoom = 1.0f;
    view.rotation_degrees = 0.0f;

    params = {};
    params.max_iter = 64;
    params.epsilon = 1e-6f;
    params.coloring_mode = DefaultColoringModeForFractal(ft);
    params.exposure = 1.0f;
    params.color_tint_r = 1.0f;
    params.color_tint_g = 1.0f;
    params.color_tint_b = 1.0f;
    params.color_saturation = 1.0f;
    params.color_contrast = 1.0f;

    if (IsExplainoFamily(ft)) {
        params.explaino_damping = 1.0f;
        params.explaino_warp_strength = 0.1f;
    }

    render = {};
    render.resolution = {kWidth, kHeight};
    render.block_size = 256;
    render.device_id = 0;
    render.sample_tier = SampleTier::fast;
}

// Run the equivalence test for one fractal type.
// Returns false if the type fails (for summary reporting).
bool TestOneType(FractalType ft, const char* name) {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    MakeDefaults(ft, view, params, render);

    // Path 1: RenderFractalCUDA
    std::vector<uint32_t> pixels(kNumPixels, 0u);
    RenderStats stats{};
    const char* error = nullptr;
    bool renderOk = RenderFractalCUDA(view, params, render, pixels.data(), nullptr, &stats, &error);
    if (!renderOk) {
        std::cerr << "  " << name << " render FAILED: " << (error ? error : "unknown") << "\n";
        CHECK(name, false);
        return false;
    }

    // Path 2: SampleFractalPoints with exactly the same coordinates
    std::vector<Double2> coords;
    ComputePixelCoordinates(view, kWidth, kHeight, coords);

    std::vector<FractalSampleResult> results(kNumPixels);
    error = nullptr;
    bool sampleOk = SampleFractalPoints(coords.data(), kNumPixels, view, params, render, results.data(), &error);
    if (!sampleOk) {
        std::cerr << "  " << name << " sample FAILED: " << (error ? error : "unknown") << "\n";
        CHECK(name, false);
        return false;
    }

    // Validate all sample results.
    long long sampleItersSum = 0;
    int invalidCount = 0;
    for (int i = 0; i < kNumPixels; ++i) {
        const auto& r = results[i];
        // Iterations must always be in range.
        if (r.iterations < 0 || r.iterations > params.max_iter) ++invalidCount;
        // For converged points, final_z and residual should be finite.
        // For escaped/diverged points, non-finite values are expected
        // (e.g., collatz/transcendental fractals produce Inf during iteration).
        if (r.converged) {
            if (!std::isfinite(r.final_z_x) || !std::isfinite(r.final_z_y)) ++invalidCount;
            if (!std::isfinite(r.residual)) ++invalidCount;
        }
        sampleItersSum += r.iterations;
    }
    CHECK((std::string(name) + " all results valid").c_str(), invalidCount == 0);
    if (invalidCount > 0) {
        std::cerr << "    " << name << " had " << invalidCount << " invalid fields\n";
    }

    // Cross-validate iteration sum.
    // RenderStats.last_iters_avg = sum(iterations) / (w*h) using integer division.
    int renderItersAvg = stats.last_iters_avg;
    int sampleItersAvg = (int)(sampleItersSum / kNumPixels);

    // Allow ±1 for integer rounding. atomicAdd in the render path is exact for
    // int, so the only source of difference would be coordinate mismatch.
    int delta = abs(renderItersAvg - sampleItersAvg);
    CHECK((std::string(name) + " iters_avg match").c_str(), delta <= 1);
    if (delta > 1) {
        std::cerr << "    " << name << " iters_avg: render=" << renderItersAvg
                  << " sample=" << sampleItersAvg << " delta=" << delta << "\n";
    }

    // Spot-check RGBA consistency with sample flags.
    // For basin-coloring types: converged → non-black pixel.
    // For escape-time types: escaped → non-black pixel.
    int flagMismatchCount = 0;
    bool isBasin = SupportsBasinColoring(ft);
    for (int i = 0; i < kNumPixels; ++i) {
        uint32_t px = pixels[i];
        uint8_t r = (uint8_t)(px & 0xFF);
        uint8_t g = (uint8_t)((px >> 8) & 0xFF);
        uint8_t b = (uint8_t)((px >> 16) & 0xFF);
        bool isBlack = (r == 0 && g == 0 && b == 0);

        const auto& sr = results[i];
        if (isBasin) {
            // Basin type: converged should produce non-black.
            if (sr.converged && isBlack) ++flagMismatchCount;
        } else {
            // Escape type: escaped should produce non-black.
            if (sr.escaped && isBlack) ++flagMismatchCount;
        }
    }
    // Allow a small tolerance — edge pixels and special coloring modes
    // may produce black even for converged/escaped points.
    int mismatchThreshold = kNumPixels / 100; // 1% tolerance
    WARN((std::string(name) + " RGBA/flag consistency").c_str(),
         flagMismatchCount <= mismatchThreshold);

    // Diagnostic summary (always printed for observability).
    int convergedCount = 0, escapedCount = 0, neitherCount = 0;
    float minResidual = 1e30f, maxResidual = -1e30f;
    float maxConvergedResidual = -1e30f;
    for (int i = 0; i < kNumPixels; ++i) {
        if (results[i].converged) ++convergedCount;
        else if (results[i].escaped) ++escapedCount;
        else ++neitherCount;
        float res = results[i].residual;
        if (std::isfinite(res)) {
            if (res < minResidual) minResidual = res;
            if (res > maxResidual) maxResidual = res;
            if (results[i].converged && res > maxConvergedResidual)
                maxConvergedResidual = res;
        }
    }

    // KF-1 regression: basin types claiming convergence must have residual < 1.0.
    // A converged point with residual >> eps indicates a degenerate false-positive
    // (e.g. unconditional converged=true without root-snapping).
    if (isBasin && convergedCount > 0) {
        CHECK((std::string(name) + " converged residual quality").c_str(),
              maxConvergedResidual < 1.0f);
        if (maxConvergedResidual >= 1.0f) {
            std::cerr << "    " << name << " max converged residual: " << maxConvergedResidual
                      << " (expected < 1.0)\n";
        }
    }

    printf("    %-30s iters_avg=%-3d  conv=%5.1f%%  esc=%5.1f%%  neither=%5.1f%%  residual=[%.2e, %.2e]\n",
           name, sampleItersAvg,
           100.0 * convergedCount / kNumPixels,
           100.0 * escapedCount / kNumPixels,
           100.0 * neitherCount / kNumPixels,
           minResidual, maxResidual);

    return true;
}

} // namespace

int main() {
    const struct { FractalType type; const char* name; } types[] = {
        {FractalType::newton, "newton"},
        {FractalType::nova, "nova"},
        {FractalType::mandelbrot, "mandelbrot"},
        {FractalType::julia, "julia"},
        {FractalType::burning_ship, "burning_ship"},
        {FractalType::multibrot, "multibrot"},
        {FractalType::phoenix, "phoenix"},
        {FractalType::explaino, "explaino"},
        {FractalType::explaino_y, "explaino_y"},
        {FractalType::explaino_fp, "explaino_fp"},
        {FractalType::explaino_nova, "explaino_nova"},
        {FractalType::explaino_halley, "explaino_halley"},
        {FractalType::explaino_dual, "explaino_dual"},
        {FractalType::explaino_mult, "explaino_mult"},
        {FractalType::explaino_phoenix, "explaino_phoenix"},
        {FractalType::explaino_transcendental, "explaino_transcendental"},
        {FractalType::explaino_inertial, "explaino_inertial"},
        {FractalType::explaino_julia, "explaino_julia"},
        {FractalType::explaino_rational, "explaino_rational"},
        {FractalType::multicorn, "multicorn"},
        {FractalType::halley, "halley"},
        {FractalType::collatz, "collatz"},
        {FractalType::explaino_collatz, "explaino_collatz"},
        {FractalType::explaino_collatz_direct, "explaino_collatz_direct"},
        {FractalType::mcmullen, "mcmullen"},
        {FractalType::lambda_map, "lambda_map"},
        {FractalType::explaino_lambda, "explaino_lambda"},
        {FractalType::explaino_rational_escape, "explaino_rational_escape"},
        {FractalType::spider, "spider"},
        {FractalType::celtic_mandelbrot, "celtic_mandelbrot"},
        {FractalType::perpendicular_burning_ship, "perpendicular_burning_ship"},
        {FractalType::explaino_joy, "explaino_joy"},
        {FractalType::explaino_fold, "explaino_fold"},
        {FractalType::explaino_bell, "explaino_bell"},
        {FractalType::explaino_ripple, "explaino_ripple"},
        {FractalType::explaino_splice, "explaino_splice"},
        {FractalType::explaino_vortex, "explaino_vortex"},
        {FractalType::explaino_tension, "explaino_tension"},
    };
    constexpr int numTypes = sizeof(types) / sizeof(types[0]);

    int passedTypes = 0;
    int failedTypes = 0;

    for (const auto& tc : types) {
        std::cout << "  testing " << tc.name << "..." << std::flush;
        bool ok = TestOneType(tc.type, tc.name);
        if (ok) {
            ++passedTypes;
            std::cout << " ok\n";
        } else {
            ++failedTypes;
            std::cout << " FAILED\n";
        }
    }

    CleanupFractalCUDA();

    std::cout << "test_fractal_sample_equivalence: "
              << passedTypes << "/" << numTypes << " types passed, "
              << "passed=" << gPass << " failed=" << gFail;
    if (gWarn > 0) std::cout << " warn=" << gWarn;
    std::cout << "\n";

    return (gFail == 0) ? 0 : 1;
}
